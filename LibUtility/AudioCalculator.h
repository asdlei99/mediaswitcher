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
#ifndef __AUDIO_CALCULATOR
#define __AUDIO_CALCULATOR

#include <strmif.h>
#include <amvideo.h>
#include <mtype.h>
#include <string>
#include <vector>
#include <streams.h>

using namespace std;

const int Analyze_Resolution = 30;//analyze 30 sample per second

class AudioCalculator
{
public:
	~AudioCalculator(void);
	static AudioCalculator& GetInstance();
private:
	AudioCalculator(void);
	static AudioCalculator* s_pCAudioCalculator;
	static void CreateInstance();

public:
	void SetWaveFormatInfo(WAVEFORMATEX* wavInfo)
	{
		memcpy(&m_WavInfo, wavInfo, sizeof(WAVEFORMATEX));

		m_AudioSampleMaxValue = (1<<m_WavInfo.wBitsPerSample)-1;

		m_AudioAnalyzeStep = m_WavInfo.nAvgBytesPerSec/Analyze_Resolution;
	}
	//send audio sample buffer
	void SetAudioSample(const unsigned char* buffer, int bufferSize);
	//to get audio sample value
	void GetAudioSampleValue(int& value);
private:
	vector<int> m_vecAudioSampleValue;
	WAVEFORMATEX m_WavInfo; 
	CCritSec m_CritSec;
	DWORD m_AudioSampleMaxValue;
	int	m_AudioAnalyzeStep;
};

#endif