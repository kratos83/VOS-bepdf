/*
 * BePDF: The PDF reader for VOS.
 * 	 Copyright (C) 2026	Angelo Scarnà
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

#include "PageThumbnails.h"

#include <math.h>
#include <string.h>

#include <Font.h>
#include <Message.h>
#include <View.h>

#include <PDFDoc.h>
#include <SplashOutputDev.h>
#include <SplashBitmap.h>

#include "BePDF.h"

static const float kPadding = 4;


PageThumbnailItem::PageThumbnailItem(const char* label)
	:
	BListItem(),
	fLabel(label),
	fThumbnail(NULL),
	fFontHeight(0)
{
}


PageThumbnailItem::~PageThumbnailItem()
{
	delete fThumbnail;
}


void
PageThumbnailItem::SetThumbnail(BBitmap* bitmap)
{
	delete fThumbnail;
	fThumbnail = bitmap;
}


void
PageThumbnailItem::Update(BView* owner, const BFont* font)
{
	font_height fh;
	font->GetHeight(&fh);
	fFontHeight = ceilf(fh.ascent + fh.descent + fh.leading);
	SetWidth(owner->Bounds().Width());
	SetHeight(kThumbnailHeight + fFontHeight + 3 * kPadding);
}


void
PageThumbnailItem::DrawItem(BView* owner, BRect frame, bool complete)
{
	if (IsSelected()) {
		owner->SetHighColor(ui_color(B_LIST_SELECTED_BACKGROUND_COLOR));
		owner->SetLowColor(ui_color(B_LIST_SELECTED_BACKGROUND_COLOR));
	} else {
		owner->SetHighColor(ui_color(B_LIST_BACKGROUND_COLOR));
		owner->SetLowColor(ui_color(B_LIST_BACKGROUND_COLOR));
	}
	owner->FillRect(frame);

	// thumbnail, centered in a fixed-size area
	BRect area(0, 0, kThumbnailWidth - 1, kThumbnailHeight - 1);
	area.OffsetTo(frame.left + (frame.Width() - kThumbnailWidth) / 2,
		frame.top + kPadding);
	if (fThumbnail != NULL) {
		BRect bounds = fThumbnail->Bounds();
		BRect r = bounds;
		r.OffsetTo(area.left + (kThumbnailWidth - bounds.Width() - 1) / 2,
			area.top + (kThumbnailHeight - bounds.Height() - 1) / 2);
		r.left = floorf(r.left);
		r.top = floorf(r.top);
		r.right = r.left + bounds.Width();
		r.bottom = r.top + bounds.Height();
		owner->DrawBitmap(fThumbnail, r);
		owner->SetHighColor(tint_color(ui_color(B_LIST_BACKGROUND_COLOR),
			B_DARKEN_3_TINT));
		owner->StrokeRect(r);
	} else {
		owner->SetHighColor(tint_color(ui_color(B_LIST_BACKGROUND_COLOR),
			B_DARKEN_1_TINT));
		owner->StrokeRect(area);
	}

	// page label
	rgb_color text = IsSelected() ? ui_color(B_LIST_SELECTED_ITEM_TEXT_COLOR)
		: ui_color(B_LIST_ITEM_TEXT_COLOR);
	owner->SetHighColor(text);
	font_height fh;
	owner->GetFontHeight(&fh);
	float width = owner->StringWidth(fLabel.String());
	owner->DrawString(fLabel.String(),
		BPoint(frame.left + (frame.Width() - width) / 2,
			frame.top + kThumbnailHeight + 2 * kPadding + fh.ascent));
}


//////////////////////////////////////////////////////////////////////////


PageThumbnailRenderer::PageThumbnailRenderer()
	:
	fDoc(NULL),
	fThread(-1),
	fStop(false)
{
}


PageThumbnailRenderer::~PageThumbnailRenderer()
{
	Stop();
}


void
PageThumbnailRenderer::Start(PDFDoc* doc, const BMessenger& target)
{
	Stop();
	fDoc = doc;
	fTarget = target;
	fStop = false;
	fThread = spawn_thread(ThreadEntry, "page thumbnails",
		B_LOW_PRIORITY, this);
	if (fThread >= 0)
		resume_thread(fThread);
}


void
PageThumbnailRenderer::Stop()
{
	if (fThread >= 0) {
		fStop = true;
		status_t result;
		wait_for_thread(fThread, &result);
		fThread = -1;
	}
}


int32
PageThumbnailRenderer::ThreadEntry(void* data)
{
	static_cast<PageThumbnailRenderer*>(data)->Run();
	return 0;
}


int
PageThumbnailRenderer::AbortCheck(void* data)
{
	return static_cast<PageThumbnailRenderer*>(data)->fStop;
}


void
PageThumbnailRenderer::Run()
{
	SplashColor paper;
	paper[0] = paper[1] = paper[2] = 255;
	SplashOutputDev dev(splashModeRGB8, 4, gFalse, paper);

	int pages = fDoc->getNumPages();
	for (int page = 1; page <= pages && !fStop; page++) {
		gPdfLock->Lock();
		if (fStop) {
			gPdfLock->Unlock();
			break;
		}

		double w = fDoc->getPageCropWidth(page);
		double h = fDoc->getPageCropHeight(page);
		int rotate = fDoc->getPageRotate(page);
		if (rotate == 90 || rotate == 270) {
			double t = w; w = h; h = t;
		}
		if (w < 1 || h < 1) {
			gPdfLock->Unlock();
			continue;
		}
		double dpi = 72.0 * fmin(PageThumbnailItem::kThumbnailWidth / w,
			PageThumbnailItem::kThumbnailHeight / h);

		dev.startDoc(fDoc->getXRef());
		fDoc->displayPage(&dev, page, dpi, dpi, 0, gFalse, gTrue, gFalse,
			AbortCheck, this);

		BBitmap* bitmap = NULL;
		SplashBitmap* src = dev.getBitmap();
		if (!fStop && src != NULL) {
			int width = src->getWidth();
			int height = src->getHeight();
			bitmap = new BBitmap(BRect(0, 0, width - 1, height - 1),
				B_RGB32);
			if (bitmap->InitCheck() == B_OK) {
				uint8* dst = (uint8*)bitmap->Bits();
				for (int y = 0; y < height; y++) {
					const uint8* s = src->getDataPtr() + y * src->getRowSize();
					uint8* d = dst + y * bitmap->BytesPerRow();
					for (int x = 0; x < width; x++, s += 3, d += 4) {
						d[0] = s[2];
						d[1] = s[1];
						d[2] = s[0];
						d[3] = 255;
					}
				}
			} else {
				delete bitmap;
				bitmap = NULL;
			}
		}
		gPdfLock->Unlock();

		if (bitmap != NULL) {
			BMessage msg(THUMBNAIL_MSG);
			msg.AddInt32("index", page - 1);
			msg.AddPointer("bitmap", bitmap);
			if (fTarget.SendMessage(&msg) != B_OK)
				delete bitmap;
		}
	}
}
