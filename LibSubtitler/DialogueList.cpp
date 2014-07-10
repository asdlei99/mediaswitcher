/*	Subtitler - Sub Station Alpha v2.x/4.x subtitling plugin for VirtualDub
	Copyright (C) 2000-2002 Avery Lee, All Rights Reserved.

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

	DialogueList.cpp: Maintains a sorted list of dialogue lines.
*/

#include <stdlib.h>
#include <crtdbg.h>

//#define new new(_NORMAL_BLOCK, __FILE__, __LINE__)

#include "DialogueList.h"
#include "FontCache.h"

///////////////////////////////////////////////////////////////////////////

DialogueItem::DialogueItem() {
	text = NULL;
}

DialogueItem::~DialogueItem() {
#ifndef TEST_SUBTITLE
	free(text);
#endif
}

///////////////////////////////////////////////////////////////////////////

class DialogueStyle : public ListNode2<DialogueStyle> {
public:
	DialogueStyleDefinition dsd;

	DialogueStyle(const DialogueStyleDefinition *dsd);
	~DialogueStyle();
};

DialogueStyle::DialogueStyle(const DialogueStyleDefinition *pdsd) {
	dsd = *pdsd;

	dsd.name = strdup(dsd.name);
	dsd.fontname = strdup(dsd.fontname);
}

DialogueStyle::~DialogueStyle() {
	free((void *)dsd.name);
	free((void *)dsd.fontname);
}

///////////////////////////////////////////////////////////////////////////

DialogueList::DialogueList() {
	item_count = 0;
	m_CurStyles = NULL;
	m_CurDialogueItem = NULL;
}

DialogueList::~DialogueList() {
	Clear();
}

void DialogueList::Clear() {
	DialogueItem *pdi;
	DialogueStyle *pds;

	while(pdi = (DialogueItem *)inactive_items.RemoveHead())
		delete pdi;

	while(pdi = (DialogueItem *)active_items.RemoveHead())
		delete pdi;

	while(pdi = (DialogueItem *)new_items.RemoveHead())
		delete pdi;

	while(pdi = (DialogueItem *)expired_items.RemoveHead())
		delete pdi;

	while(pds = (DialogueStyle *)styles.RemoveHead())
		delete pds;

	item_count = 0;
}

DialogueStyle *DialogueList::FindStyle(const char *style) {
	DialogueStyle *pds = (DialogueStyle *)styles.AtHead(), *pds_next;

	while(pds_next = (DialogueStyle *)pds->NextFromHead()) {
		if (!strcmp(style, pds->dsd.name))
			return pds;

		pds = pds_next;
	}

	return NULL;
}

void DialogueList::AddStyle(const DialogueStyleDefinition *dsd) {
	DialogueStyle *pds;
	DialogueStyleDefinition dsd2 = *dsd;

	if (pds = FindStyle(dsd->name)) {
		pds->Remove();
		delete pds;
	}

	pds = new DialogueStyle(&dsd2);

	styles.AddHead(pds);
}

void DialogueList::AddCurStyle(const DialogueStyleDefinition* style)//specify current display style 
{
	if(m_CurStyles)
 		delete m_CurStyles;

	m_CurStyles = new DialogueStyle(style);
}

DialogueStyle* DialogueList::GetCurStyle()//get current display style
{
	return m_CurStyles;
}

void  DialogueList::AddCurDialogueItem(DialogueItem *pdi)
{
// 	if(m_CurDialogueItem && (m_CurDialogueItem->text))
// 		delete [] (pdi->text);
	if(m_CurDialogueItem)
		delete m_CurDialogueItem;

	m_CurDialogueItem = pdi;
}

DialogueItem * DialogueList::GetCurDialogueItem()
{
	return m_CurDialogueItem;
}

void DialogueList::Add(DialogueItem *pdi, const char *style) {
	DialogueStyle *pds;

	if (!(pds = FindStyle(style)))
		if (!(pds = FindStyle("*Default")))
			pds = (DialogueStyle *)styles.AtTail();

	pdi->pStyle = pds;

	pdi->pDSD = &pds->dsd;

	if (!pdi->nLeftMargin)		pdi->nLeftMargin = pds->dsd.nLeftMargin;
	if (!pdi->nRightMargin)		pdi->nRightMargin = pds->dsd.nRightMargin;
	if (!pdi->nBottomMargin)	pdi->nBottomMargin = pds->dsd.nBottomMargin;

	inactive_items.AddTail(pdi);
	++item_count;
}

static int sort_comparator(const void *ppv1, const void *ppv2) {
	const DialogueItem *pdi1 = *(DialogueItem **)ppv1;
	const DialogueItem *pdi2 = *(DialogueItem **)ppv2;

/*	return pdi1->start_ms < pdi2->start_ms ? -1
		: pdi1->start_ms > pdi2->start_ms ? 1
		: 0;*/

	if (pdi1->start_ms != pdi2->start_ms)
		return pdi1->start_ms - pdi2->start_ms;
	else
		return pdi1->order - pdi2->order;
}

bool DialogueList::Sort() {
	if (!item_count) return true;

	DialogueItem **ppdi_list = new DialogueItem *[item_count], **ppdi;
	DialogueItem *pdi, *pdi_next;
	int i;

	//////////////////

	if (!ppdi_list)
		return false;

	pdi = (DialogueItem *)inactive_items.AtHead();
	ppdi = ppdi_list;
	while(pdi_next = (DialogueItem *)pdi->NextFromHead()) {
		*ppdi++ = pdi;

		pdi = pdi_next;
	}

	qsort(ppdi_list, item_count, sizeof(DialogueItem *), sort_comparator);

	inactive_items.Init();
	for(i=0; i<item_count; i++)
		inactive_items.AddTail(ppdi_list[i]);

	delete[] ppdi_list;

	return true;
}

void DialogueList::Start() {
	if (!item_count)
		cur_item = NULL;
	else
		cur_item = (DialogueItem *)inactive_items.AtHead();
}

void DialogueList::Stop() {
	DialogueItem *pdi, *pdi_next;

	// Move any remaining items from active list to inactive list.

	pdi = (DialogueItem *)active_items.AtHead();

	while(pdi_next = (DialogueItem *)pdi->NextFromHead()) {
		pdi->Remove();
		inactive_items.AddTail(pdi);

		pdi = pdi_next;
	}
}

void DialogueList::Restart() {
	DialogueItem *pdi;

	// Move all active text back to the inactive list.

	while(pdi = active_items.RemoveTail())
		inactive_items.AddHead(pdi);

	// Move all expired text back to the active list.

	while(pdi = expired_items.RemoveTail())
		inactive_items.AddHead(pdi);

	// Resort.

	Sort();
	Start();
}

void DialogueList::AdvanceClock(long ms) {
	DialogueItem *pdi, *pdi_next;

	// Expire all text on the active list that has passed.

	pdi = (DialogueItem *)active_items.AtHead();
	while(pdi_next = (DialogueItem *)pdi->NextFromHead()) {
		if (ms >= pdi->start_ms + pdi->duration_ms) {
			pdi->Remove();
			inactive_items.AddHead(pdi);
		}

		pdi = pdi_next;
	}

	// Activate text on the inactive list that has arrived.

	while(cur_item && (pdi_next = (DialogueItem *)cur_item->NextFromHead())) {
		if (cur_item->start_ms > ms)
			break;

		cur_item->Remove();
		active_items.AddTail(cur_item);

		cur_item->x = cur_item->y = -1;

		cur_item = pdi_next;
	}
}

void DialogueList::AdvanceClock2(long ms) {
	DialogueItem *pdi, *pdi_next;

	// Expire all text on the active list that has passed.

	pdi = (DialogueItem *)active_items.AtHead();
	while(pdi_next = (DialogueItem *)pdi->NextFromHead()) {
		if (ms >= pdi->start_ms + pdi->duration_ms) {
			pdi->Remove();
			expired_items.AddHead(pdi);
		}

		pdi = pdi_next;
	}

	// Activate text on the inactive list that has arrived.

	while(cur_item && (pdi_next = (DialogueItem *)cur_item->NextFromHead())) {
		if (cur_item->start_ms > ms)
			break;

		cur_item->Remove();

		if (cur_item->start_ms + cur_item->duration_ms <= ms)
			expired_items.AddHead(cur_item);
		else
			new_items.AddTail(cur_item);

		cur_item->x = cur_item->y = -1;

		cur_item = pdi_next;
	}
}

DialogueItem *DialogueList::getNextExpired() {
	DialogueItem *pdi;

	if (pdi = expired_items.RemoveHead())
		inactive_items.AddHead(pdi);

	return pdi;
}

DialogueItem *DialogueList::getNextNew() {
	DialogueItem *pdi;

	if (pdi = new_items.RemoveHead())
		active_items.AddTail(pdi);

	return pdi;
}

DialogueItem *DialogueList::getFirstActive() {
	if (!active_items.IsEmpty())
		return (DialogueItem *)active_items.AtHead();
	else
		return NULL;
}

DialogueItem *DialogueList::getNextActive(DialogueItem *pdi) {
	pdi = (DialogueItem *)pdi->NextFromHead();

	if (pdi->NextFromHead())
		return pdi;
	else
		return NULL;
}
