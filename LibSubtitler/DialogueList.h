/*	Subtitler - Sub Station Alpha v2.x/4.x subtitling plugin for VirtualDub
	Copyright (C) 2000 Avery Lee, All Rights Reserved.

	This program is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation; either version 2 of the License, or
	(at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with this program; if not, write to the Free Software
	Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

	-----------------

	DialogueList.h: Keeps a sorted list of dialogue entries.
*/
#ifndef f_DIALOGUELIST_H
#define f_DIALOGUELIST_H

#include <windows.h>

#include "List.h"

class DialogueStyle;
class FontDescriptor;

enum DialogueAlignment {
	DALIGN_DEFAULT	= 0,

	DALIGN_LEFT		= 1,
	DALIGN_CENTER	= 2,
	DALIGN_RIGHT	= 3,

	DALIGN_TOP		= 16,
	DALIGN_MIDDLE	= 32,
	DALIGN_BOTTOM	= 48,
};

struct DialogueStyleDefinition {
	const char *		name;
	const char *		fontname;
	int					nHeight;
	long				rgbColors[3];
	long				rgbShadowColor;
	DWORD				dwCharSet;
	bool				fItalics;
	bool				fBold;
	bool				fUnderline;
	bool				fOpaqueBox;
	int					nShadowDepth;
	int					nBorderWidth;
	int					daPos;
	int					nLeftMargin;
	int					nRightMargin;
	int					nBottomMargin;
	int					nAlpha;
};

class DialogueItem : public ListNode2<DialogueItem> {
public:
	char	*text;
	long	x, y;
	long	start_ms, duration_ms;
	int		order;

	DialogueStyle		*pStyle;
	DialogueStyleDefinition *pDSD;

	int					nLeftMargin;
	int					nRightMargin;
	int					nBottomMargin;

	// Non-standard

	int					nXScrollRate;		// 1/256th pixels per second
	int					nYScrollRate;		// 1/256th pixels per second
	int					nYWindowPos;		// pixels
	int					nYWindowSize;		// pixels

	DialogueItem();
	~DialogueItem();
};

class DialogueList {
private:
	DialogueStyle *FindStyle(const char *style);
//////////////////////////////////////////////////////////////////////////
private:
	DialogueStyle*	m_CurStyles;//current select style
	DialogueItem*	m_CurDialogueItem;//current select style
public:
	void AddCurStyle(const DialogueStyleDefinition* style);//specify current display style 
	DialogueStyle* GetCurStyle();//get current display style
	void  AddCurDialogueItem(DialogueItem *pdi);
	DialogueItem * GetCurDialogueItem();//get current display style
//////////////////////////////////////////////////////////////////////////
public:
	List2<DialogueItem>	inactive_items;
	List2<DialogueItem>	active_items;
	List2<DialogueItem>	new_items;
	List2<DialogueItem>	expired_items;
	List2<DialogueStyle>	styles;
	int		item_count;
	DialogueItem *cur_item;

	DialogueList();
	~DialogueList();

	void			Clear();
	void			AddStyle(const DialogueStyleDefinition *dsd);
	void			Add(DialogueItem *pdi, const char *style);
	bool			Sort();
	void			Start();
	void			Stop();
	void			Restart();
	void			AdvanceClock(long ms);
	void			AdvanceClock2(long ms);
	DialogueItem *	getNextExpired();
	DialogueItem *	getNextNew();
	DialogueItem *	getFirstActive();
	DialogueItem *	getNextActive(DialogueItem *);
};

#endif
