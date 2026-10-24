/*
 * BePDF: The PDF reader for Haiku.
 * 	 Copyright (C) 1997 Benoit Triquet.
 * 	 Copyright (C) 1998-2000 Hubert Figuiere.
 * 	 Copyright (C) 2000-2002 Michael Pfeiffer.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

#ifndef PAGE_THUMBNAILS_H
#define PAGE_THUMBNAILS_H

#include <Bitmap.h>
#include <ListItem.h>
#include <Messenger.h>
#include <OS.h>
#include <String.h>

class PDFDoc;

// List entry of the page list: the thumbnail of the page (once rendered)
// with the page label below it.
class PageThumbnailItem : public BListItem {
public:
	PageThumbnailItem(const char* label);
	~PageThumbnailItem();

	void DrawItem(BView* owner, BRect frame, bool complete = false);
	void Update(BView* owner, const BFont* font);

	// takes ownership of bitmap
	void SetThumbnail(BBitmap* bitmap);

	static const int32 kThumbnailWidth = 96;
	static const int32 kThumbnailHeight = 128;

private:
	BString  fLabel;
	BBitmap* fThumbnail;
	float    fFontHeight;
};


// Renders the thumbnails of all pages in a background thread and sends each
// one to the target as THUMBNAIL_MSG (int32 "index", pointer "bitmap");
// the receiver owns the bitmap.
class PageThumbnailRenderer {
public:
	enum { THUMBNAIL_MSG = 'PTrd' };

	PageThumbnailRenderer();
	~PageThumbnailRenderer();

	void Start(PDFDoc* doc, const BMessenger& target);
	// blocks until the thread has finished
	void Stop();

private:
	static int32 ThreadEntry(void* data);
	static int AbortCheck(void* data);
	void Run();

	PDFDoc*         fDoc;
	BMessenger      fTarget;
	thread_id       fThread;
	volatile bool   fStop;
};

#endif
