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
#include "AudioCalculator.h"

AudioCalculator* AudioCalculator::s_pCAudioCalculator = NULL;

AudioCalculator::AudioCalculator(void)
{
	m_AudioSampleMaxValue = 0;
	m_AudioAnalyzeStep = 0;
}

AudioCalculator::~AudioCalculator(void)
{
}

AudioCalculator& AudioCalculator::GetInstance()
{
	if (s_pCAudioCalculator == NULL)
	{
		CreateInstance();
	}
	return *s_pCAudioCalculator;
}

void AudioCalculator::CreateInstance()
{
	static AudioCalculator audiosamleCtrl;
	s_pCAudioCalculator = &audiosamleCtrl;
}

//send audio sample buffer
void AudioCalculator::SetAudioSample(const unsigned char* buffer, int bufferSize)
{
	//calculate the audio sample's value, convert them to percentage of max value
	int size = bufferSize/m_AudioAnalyzeStep;
	int* sampleArray = (int*)malloc(size * sizeof(int));
	const unsigned char* tempBuffer = buffer;
	int i = 0;

	if(size == 0)
		size = 1;//make sure we get one value at this time;

	for(i = 0 ; i < size; i++)
	{
		int audioSampleValue = (*(tempBuffer+1)<<8)|*tempBuffer;

		sampleArray[i] = audioSampleValue*100/(m_AudioSampleMaxValue);

		tempBuffer += m_AudioAnalyzeStep;
	}
	//add them into vector
	{
		CAutoLock lock(&m_CritSec);
		for(i = 0 ; i < size; i++)
		{
			m_vecAudioSampleValue.push_back(sampleArray[i]);
		}
	}
}

//to get audio sample value
void AudioCalculator::GetAudioSampleValue(int& value)
{
	CAutoLock lock(&m_CritSec);

	if (m_vecAudioSampleValue.size() > 0)
	{
		vector<int>::iterator iter= m_vecAudioSampleValue.begin();
		value = *iter;
		m_vecAudioSampleValue.erase(iter);
	}
}