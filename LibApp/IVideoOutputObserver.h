/*Copyright (C) 2010 Kevin.Wen, All Rights Reserved.

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
*/
#ifndef __IVIDEOOUTPUT_OBSERVER_H
#define __IVIDEOOUTPUT_OBSERVER_H

class ISampleReceiver;
class IVideoOutputObserver
{
public:
	virtual ~IVideoOutputObserver(){}
	virtual bool RegisterOutPutReceiver(ISampleReceiver* pReveiver, DWORD id) = 0;
	virtual bool UnRegisterOutPutReceiver() = 0;
};
#endif