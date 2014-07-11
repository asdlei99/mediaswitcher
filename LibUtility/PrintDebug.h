/*Copyright (C) 2010 Kevin.Wen, All Rights Reserved.

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

-----------------
*/
#ifndef __PRINT_DEBUG_H
#define __PRINT_DEBUG_H

enum LOG_LEVEL
{
	LOG_LEVEL0 = 0x01,
	LOG_LEVEL1 = (LOG_LEVEL0<<1),
	LOG_LEVEL2 = (LOG_LEVEL1<<1),
	LOG_LEVEL3 = (LOG_LEVEL2<<1),
	LOG_AVSYNC = (LOG_LEVEL3<<1)
};

void DebugMessage(LOG_LEVEL level, char* fmt, ...); 

void OpenDebugFile();
void CloseDebugFile();
void SetDebugLevel(int level);
#endif
