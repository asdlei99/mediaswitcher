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
#ifndef __GlobalDefine_H
#define __GlobalDefine_H

#define SAFE_DELETE(p)       { if(p) { delete (p);     (p)=NULL; } }
#define SAFE_DELETE_ARRAY(p) { if(p) { delete[] (p);   (p)=NULL; } }
#define SAFE_RELEASE(p)      { if(p) { (p)->Release(); (p)=NULL; } }
typedef enum
{
	DVSD = 0,
	RGB24,
	YUY2,
	PCM
}MEDIA_Type;

typedef enum
{
	DT_DV = 0,
	DT_Analog_WDM,
	DT_Analog_VFW,
	DT_Analog_Audio,
	DT_Unknown
} Device_Type;

typedef enum 
{
	MD_Idle,
	MD_Preview, 
	MD_Capture,
	MD_Capture_Second
} Work_Mode;

typedef enum 
{
	SR_NTSC, 
	SR_PAL,
	SR_SECAM,
	SR_UNKNOWN
} Signal_Resolution;

typedef enum 
{
	ET_AVI, 
	ET_DIVX
} Encoding_Type;

typedef enum 
{
	CTRL_IDLE,
	CTRL_PAUSE,
	CTRL_RUNNING,
	CTLR_EOS,
}CtrlStatus;

const long cVideoSourceChanged			= 'lcap' + 1;
const long cAudioSourceChanged			= 'lcap' + 2;
const long cVideoConnectorChanged		= 'lcap' + 3;
const long cAudioConnectorChanged		= 'lcap' + 4;
const long cVideoResolutionChanged		= 'lcap' + 5;
const long cAudioMixLevelChanged		= 'lcap' + 6;

const long msg_PnPDeviceAdded	= 88888;
const long msg_PnPDeviceRemoved	= 88889;

const long msg_FilterGraphError	= 88890;
const long msg_DShowDeviceLost  = 88891;

const long msg_FilterGraphDestroy	= 88892;

const int outputWidth = 720;
const int outputHeight = 576;

const int unit_one_second = 1000;
const int unit_one_minute = unit_one_second*60;
const int unit_one_hour = unit_one_minute*60;
const int BREAKPOINT_FIXTIME = 1000;

enum GraphType
{
	NONE_TYPE = -1,
	AUDIO_ONLY,
	VIDEO_ONLY,
	AUDIO_VIDEO_BOTH,
	SAVE_FILE_GRAPH
};

enum OutputType
{
	AVI_FILE_OUTPUT_TYPE,
	WMV_FILE_OUTPUT_TYPE,
	WMV_Streaming_OUTPUT_TYPE
};

// Filter graph notification to the specified window
#define WM_GRAPHNOTIFY  (WM_USER+20)

#endif // __H_GlobalDefs__