// AudioPlay.cpp: implementation of the CAudioPlay class.
//
//////////////////////////////////////////////////////////////////////

#include "AudioPlay.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CAudioPlay::CAudioPlay()
{
	hWaveOut=NULL;
	m_bAudioMute =FALSE;
	m_wVolume = 0xffff;
	for(int i=0;i<PLAYING_BUFFER_NUM;i++)
	{
		PlayingWaveHdr[i].lpData = new char[AUDIO_BUFFER_MAX_LENGTH];
		PlayingWaveHdr[i].dwBufferLength = AUDIO_BUFFER_MAX_LENGTH;
	}
}

CAudioPlay::~CAudioPlay()
{
	Release();

	for(int i=0;i<PLAYING_BUFFER_NUM;i++)
	{
		delete [] PlayingWaveHdr[i].lpData;
	}
}

bool CAudioPlay::Init( int bit,int sampleRate )
{
// 	int numDevsIn = waveInGetNumDevs();
// 	if(numDevsIn == 0)
// 		return false;	
	Release();

	int numDevsOut = waveOutGetNumDevs();
	if(numDevsOut == 0)
		return false;
	
	WAVEFORMATEX format;
	
    format.wFormatTag = WAVE_FORMAT_PCM;
    format.nChannels = 1;         
    format.wBitsPerSample = bit; 
    format.nSamplesPerSec = sampleRate;//hardcoded for now 48k sample rate 
    format.nBlockAlign = format.wBitsPerSample * format.nChannels / 8;
    format.nAvgBytesPerSec = format.nSamplesPerSec * format.nBlockAlign;//* 2 channel * 2bytes per channel
    format.cbSize = 0;

	if (MMSYSERR_NOERROR != waveOutOpen(&hWaveOut, WAVE_MAPPER, &format,0,0,CALLBACK_NULL))
		return false;

	m_bQuit = FALSE;
	for(int i=0;i<PLAYING_BUFFER_NUM;i++)
	{
		PlayingWaveHdr[i].dwFlags = 0;
		PlayingWaveHdr[i].dwUser = 0;
		PlayingWaveHdr[i].dwLoops = 0;	
		waveOutPrepareHeader(hWaveOut, &(PlayingWaveHdr[i]), sizeof(WAVEHDR));
		PlayingWaveHdr[i].dwBufferLength = 0;
	}	

	PlayingBufferIndex = 0;
    return true;	
}

void CAudioPlay::Release()
{
	if (hWaveOut == NULL)
		return;

	m_bQuit = TRUE;

	Sleep(400);

	for(int i=0;i<PLAYING_BUFFER_NUM;i++)
	{
		waveOutUnprepareHeader(hWaveOut, &(PlayingWaveHdr[i]), sizeof(WAVEHDR));
	}
	waveOutClose(hWaveOut);
	hWaveOut = NULL;	
}

void CAudioPlay::FillAudioFrame( BYTE* pData, DWORD dwDatalen)
{
	if (hWaveOut == NULL)
		return;
	
	if(pData == NULL)
	{
		return;
	}
	
	if (m_bQuit)
	{
		return;
	}
	
	//csp modify 20131001
	//::MessageBox(NULL, "NetDvrSDK Ao", NULL, MB_OK);
	
	memcpy(PlayingWaveHdr[PlayingBufferIndex].lpData, pData, dwDatalen);
	
	PlayingWaveHdr[PlayingBufferIndex].dwBufferLength = dwDatalen;			
	waveOutWrite(hWaveOut, &(PlayingWaveHdr[PlayingBufferIndex]), sizeof(WAVEHDR));
	if ((PlayingWaveHdr[PlayingBufferIndex].dwFlags&WHDR_DONE) != WHDR_DONE )
		Sleep(0);
	PlayingBufferIndex++;
	PlayingBufferIndex %= PLAYING_BUFFER_NUM;	
}

void CAudioPlay::SetMute( BOOL bMute )
{
	if (hWaveOut == NULL)
		return;


	if(bMute){
		waveOutSetVolume(hWaveOut,MAKELONG(0, 0));
		m_bAudioMute=TRUE;
	}else{
		waveOutSetVolume(hWaveOut,MAKELONG(m_wVolume, m_wVolume));
		m_bAudioMute=FALSE;
	}
}

void CAudioPlay::SetVolume( WORD volume )
{
	if (hWaveOut == NULL)
		return;

	m_wVolume = volume;
	
	waveOutSetVolume(hWaveOut,MAKELONG(m_wVolume, m_wVolume));
}