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
#ifndef __IDEVICEINTERFACE_H
#define __IDEVICEINTERFACE_H
#include "../LibDevice/DSDevice.h"

class IDeviceInterface
{
public:
	virtual BOOL GetAudioDevice(DEVICE_LIST& audioDeviceList) = 0;
	virtual BOOL GetVideoDevice(DEVICE_LIST& videoDeviceList) = 0;
	virtual BOOL FindDevice(const char * inDeviceName, DSDevice& outDevice) = 0;
};
#endif