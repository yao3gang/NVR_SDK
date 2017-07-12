// AudioPlay.h: interface for the CAudioPlay class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_AUDIOPLAY_H__F703427E_3DC8_4469_9456_4548682B43A5__INCLUDED_)
#define AFX_AUDIOPLAY_H__F703427E_3DC8_4469_9456_4548682B43A5__INCLUDED_

#ifdef WIN32
#include <Windows.h>
#include <mmsystem.h>
#pragma comment(lib,"WINMM.LIB")
#endif

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000



#define	AUDIO_BUFFER_MAX_LENGTH		1024
#define PLAYING_BUFFER_NUM	6

class CAudioPlay  
{
public:
	CAudioPlay();
	virtual ~CAudioPlay();
	bool Init(int bit,int sampleRate);
	void Release();
	void FillAudioFrame(BYTE* pData, DWORD dwDatalen);
	void SetMute(BOOL bMute);
	void SetVolume(WORD volume);
private:
	int		    PlayingBufferIndex;
	WAVEHDR     PlayingWaveHdr[PLAYING_BUFFER_NUM];
	HWAVEOUT    hWaveOut;
	BOOL		m_bAudioMute;
	WORD		m_wVolume;
	BOOL		m_bQuit;
};

#endif // !defined(AFX_AUDIOPLAY_H__F703427E_3DC8_4469_9456_4548682B43A5__INCLUDED_)
