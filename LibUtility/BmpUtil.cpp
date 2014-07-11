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
#include "BmpUtil.h"
#include <math.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <io.h>
#include <windows.h>

bool writebmpfile(int width, int height, const char*fname, unsigned char* pData)
{
	int		writtenSize = 0;
	int		size = width * height*4;
	int		fd = 0;
	if ((fd = _open(fname, _O_WRONLY | _O_CREAT | _O_BINARY | _O_TRUNC, _S_IREAD | _S_IWRITE)) == -1)
	{
		return false;
	}

	BITMAPFILEHEADER	bfheader;
	BITMAPINFOHEADER	biheader;

	bfheader.bfOffBits = sizeof(BITMAPINFOHEADER) + sizeof(BITMAPFILEHEADER);
	bfheader.bfReserved1 = 0;
	bfheader.bfReserved2 = 0;
	bfheader.bfSize = size + bfheader.bfOffBits;
	bfheader.bfType = ((WORD)((BYTE)('M') << 8)) | (WORD)((BYTE)('B'));

	memset(&biheader, 0, sizeof(BITMAPINFOHEADER));
	biheader.biBitCount = 32;
	biheader.biCompression = BI_RGB;
	biheader.biHeight = height;
	biheader.biWidth = width;
	biheader.biSizeImage = height*width*4;
	biheader.biSize = sizeof(BITMAPINFOHEADER);
	biheader.biPlanes = 1;
	biheader.biClrUsed = 1;
	biheader.biClrImportant = 0;

	writtenSize += _write(fd, &bfheader, sizeof(BITMAPFILEHEADER));
	writtenSize += _write(fd, &biheader, sizeof(BITMAPINFOHEADER));

	unsigned char* tempBuffer = pData+width*(height-1)*4;
	for(int ii = 0; ii < height; ++ii)
	{
		_write(fd, tempBuffer, width*4);
		tempBuffer-=width*4;
	}

	if (fd != -1)
	{
		_commit(fd);
		_close(fd);
		fd = -1;
	}

	return true;
}