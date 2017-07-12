#ifndef WIN32

#include "mediastream.h"
#include <stdlib.h>
#include <string.h>

#include <sys/types.h>
//#include <linux/unistd.h>
#include <errno.h>
#include <asm/unistd.h>
_syscall0(pid_t, gettid);

int main()
{
	ifly_mediasnd_t *g_pMediaSnd = NULL;
	FRAMEHDR tFRAMEHDR;
	TNetSndParam tNetSndParam;
	TNetSndParam tNetSndParam2;
	u8  byBuf[256<<10];
	u16 wRet;
	int i,j;
	
	mediastreamStartup();
	
	tNetSndParam.m_byNum = 1;
	tNetSndParam.m_tLocalNet.m_wRTPPort   = 4008;
	tNetSndParam.m_tLocalNet.m_dwRTPAddr  = 0;
	tNetSndParam.m_tLocalNet.m_wRTCPPort  = 4009;
	tNetSndParam.m_tLocalNet.m_dwRTCPAddr = 0;
	tNetSndParam.m_tRemoteNet[0].m_wRTPPort  = 64040;
	tNetSndParam.m_tRemoteNet[0].m_dwRTPAddr = inet_addr("192.168.1.30");
	
	tNetSndParam2.m_byNum = 5;
	tNetSndParam2.m_tLocalNet.m_wRTPPort   = 4012;
	tNetSndParam2.m_tLocalNet.m_dwRTPAddr  = 0;
	tNetSndParam2.m_tLocalNet.m_wRTCPPort  = 4013;
	tNetSndParam2.m_tLocalNet.m_dwRTCPAddr = 0;
	tNetSndParam2.m_tRemoteNet[0].m_wRTPPort  = 64044;
	tNetSndParam2.m_tRemoteNet[0].m_dwRTPAddr = inet_addr("192.168.1.30");
	tNetSndParam2.m_tRemoteNet[1].m_wRTPPort  = 64048;
	tNetSndParam2.m_tRemoteNet[1].m_dwRTPAddr = inet_addr("192.168.1.30");
	tNetSndParam2.m_tRemoteNet[2].m_wRTPPort  = 64052;
	tNetSndParam2.m_tRemoteNet[2].m_dwRTPAddr = inet_addr("192.168.1.30");
	tNetSndParam2.m_tRemoteNet[3].m_wRTPPort  = 64056;
	tNetSndParam2.m_tRemoteNet[3].m_dwRTPAddr = inet_addr("192.168.1.30");
	tNetSndParam2.m_tRemoteNet[4].m_wRTPPort  = 64060;
	tNetSndParam2.m_tRemoteNet[4].m_dwRTPAddr = inet_addr("192.168.1.30");
	
	memset(byBuf,0,sizeof(byBuf));
	tFRAMEHDR.m_tVideoParam.m_wVideoWidth  = 352;
	tFRAMEHDR.m_tVideoParam.m_wVideoHeight = 288;
	//printf("width=%d,height=%d\n",tFRAMEHDR.m_tVideoParam.m_wVideoWidth,tFRAMEHDR.m_tVideoParam.m_wVideoHeight);
	tFRAMEHDR.m_dwPreBufSize = 0;
	tFRAMEHDR.m_byMediaType  = MEDIA_TYPE_MP4;
	tFRAMEHDR.m_byFrameRate  = 25;
	tFRAMEHDR.m_tVideoParam.m_bKeyFrame	= 1;
	
	i = 0;
	j = 0;
	while(1)
	{
		//if(g_pMediaSnd == NULL)
		{
			g_pMediaSnd = CreateMediaSnd(MAX_FRAME_SIZE,80<<20,25,MEDIA_TYPE_MP4,0);
			if(g_pMediaSnd == NULL)
			{
				printf("CreateMediaSnd failed\n");
				exit(-1);
			}
		}
		TNetSndParam tmpNetSndParam;
		if(i%2 == 0)
		{
			tmpNetSndParam = tNetSndParam;
		}
		else
		{
			tmpNetSndParam = tNetSndParam2;
		}
		//printf("addr num:%d\n",tmpNetSndParam.m_byNum);
		wRet = SetMediaSndNetParam(g_pMediaSnd,tmpNetSndParam);
		if(wRet != MEDIASTREAM_NO_ERROR)
		{
			printf("error,wRet=%d\n",wRet);
			exit(-1);
		}
		tFRAMEHDR.m_pData = byBuf;
		tFRAMEHDR.m_dwDataSize = 10240;
		wRet = SendMediaFrame(g_pMediaSnd,&tFRAMEHDR,0);
		if(wRet != MEDIASTREAM_NO_ERROR)
		{
			printf("send failed,wRet=%d\n",wRet);
			exit(-1);
		}
		DestroyMediaSnd(g_pMediaSnd);
		if(i%1000 == 0)
		{
			j = i/1000;
			printf("\n%dst:\n",j);
			system("free");
			printf("\n");
		}
		i++;
	}
	
	return 0;
}

#else

#include "mediastream.h"
#include "isoformat.h"
#include "custommp4.h"

#pragma comment( lib, "Ws2_32.lib" )

ifly_mediasnd_t *g_pMediaSnd = NULL;

int main()
{
	int i=0;
	u16 wRet;
	FRAMEHDR tFRAMEHDR;
	TNetSndParam tNetSndParam;
	u8 byBuf[256<<10];
//	iflymp4_t *file;
	custommp4_t *file;
	int vlen,vsize;
	
	u32 start_time = 0;
	u8 key;
	u16 nal_size;
	
	tNetSndParam.m_byNum = 1;
	tNetSndParam.m_tLocalNet.m_wRTPPort   = 4008;
	tNetSndParam.m_tLocalNet.m_dwRTPAddr  = 0;
	tNetSndParam.m_tLocalNet.m_wRTCPPort  = 4009;
	tNetSndParam.m_tLocalNet.m_dwRTCPAddr = 0;
	
	tNetSndParam.m_tRemoteNet[0].m_wRTPPort  = 64000;
	tNetSndParam.m_tRemoteNet[0].m_dwRTPAddr = inet_addr("192.168.1.20");
	
	mediastreamStartup();
	
	memset(byBuf,0,sizeof(byBuf));
	
#if 1
//	file = iflymp4_open("snd.mp4",1,0,0);
	file = custommp4_open("myh264.mp4",O_R,0);
	if(file == NULL)
	{
		printf("file is not exist\n");
		return -1;
	}
	
//	vlen = iflymp4_video_length(file,1);
	vlen = custommp4_video_length(file);
	printf("vlen=%d\n",vlen);
	
//	tFRAMEHDR.m_tVideoParam.m_wVideoWidth  = iflymp4_video_width(file,1);
//	tFRAMEHDR.m_tVideoParam.m_wVideoHeight = iflymp4_video_height(file,1);
	tFRAMEHDR.m_tVideoParam.m_wVideoWidth  = custommp4_video_width(file);
	tFRAMEHDR.m_tVideoParam.m_wVideoHeight = custommp4_video_height(file);
	printf("width=%d,height=%d\n",tFRAMEHDR.m_tVideoParam.m_wVideoWidth,tFRAMEHDR.m_tVideoParam.m_wVideoHeight);
	tFRAMEHDR.m_dwPreBufSize = 0;
//	tFRAMEHDR.m_byMediaType  = MEDIA_TYPE_MP4;
	tFRAMEHDR.m_byMediaType  = MEDIA_TYPE_H264;
	tFRAMEHDR.m_byFrameRate  = 25;
	tFRAMEHDR.m_tVideoParam.m_bKeyFrame    = 1;
	tFRAMEHDR.m_tVideoParam.m_wVideoWidth  = 704;
	tFRAMEHDR.m_tVideoParam.m_wVideoHeight = 576;
	tFRAMEHDR.m_dwTimeStamp = 0;
	
//	g_pMediaSnd = CreateMediaSnd(MAX_FRAME_SIZE,80<<20,25,MEDIA_TYPE_MP4,0);
	g_pMediaSnd = CreateMediaSnd(MAX_FRAME_SIZE,80<<20,25,MEDIA_TYPE_H264,0);
	wRet = SetMediaSndNetParam(g_pMediaSnd,tNetSndParam);
	if(wRet != MEDIASTREAM_NO_ERROR)
	{
		printf("error,wRet=%d\n",wRet);
		return -1;
	}
	
	while(1)
	{
		for(i=0;i<vlen;i++)
		{
			//vsize = iflymp4_frame_size(file,i,1);
			//iflymp4_read_frame(file,byBuf,1);
			vsize = custommp4_read_h264_frame(file,byBuf,sizeof(byBuf),i,&start_time,&key,&nal_size);
			printf("frame%d size:%d\n",i+1,vsize);
			tFRAMEHDR.m_pData = byBuf;
			tFRAMEHDR.m_dwDataSize = vsize;
			wRet = SendMediaFrame(g_pMediaSnd,&tFRAMEHDR,0);
			if(wRet != MEDIASTREAM_NO_ERROR)
			{
				printf("send failed,wRet=%d\n",wRet);
			}
			else
			{
				//printf("send success\n");
			}
			Sleep(40);
		}
	}
	
//	iflymp4_close(file);
	custommp4_close(file);
#else
#if 0
	vlen = 0;
	file = NULL;
	tFRAMEHDR.m_tVideoParam.m_wVideoWidth  = 352;
	tFRAMEHDR.m_tVideoParam.m_wVideoHeight = 288;
	printf("width=%d,height=%d\n",tFRAMEHDR.m_tVideoParam.m_wVideoWidth,tFRAMEHDR.m_tVideoParam.m_wVideoHeight);
	tFRAMEHDR.m_dwPreBufSize = 0;
	tFRAMEHDR.m_byMediaType  = MEDIA_TYPE_MJPEG;
	tFRAMEHDR.m_byFrameRate  = 25;
	tFRAMEHDR.m_tVideoParam.m_bKeyFrame    = 1;
	
	g_pMediaSnd = CreateMediaSnd(MAX_FRAME_SIZE,80<<20,25,MEDIA_TYPE_MJPEG,0);
	wRet = SetMediaSndNetParam(g_pMediaSnd,tNetSndParam);
	if(wRet != MEDIASTREAM_NO_ERROR)
	{
		printf("error,wRet=%d\n",wRet);
		return -1;
	}
	while(1)
	{
		for(i=0;i<8;i++)
		{
			FILE *fp;
			char filename[32];
			sprintf(filename,"jpg\\bootloader%d.jpg",i+1);
			fp = fopen(filename,"rb");
			if(fp == NULL)
			{
				Sleep(40);
				continue;
			}
			fseek(fp,0,SEEK_END);
			vsize = ftell(fp);
			fseek(fp,0,SEEK_SET);
			fread(byBuf,1,vsize,fp);
			fclose(fp);
			tFRAMEHDR.m_pData = byBuf;
			tFRAMEHDR.m_dwDataSize = vsize;
			wRet = SendMediaFrame(g_pMediaSnd,&tFRAMEHDR);
			if(wRet != MEDIASTREAM_NO_ERROR)
			{
				printf("send failed,wRet=%d\n",wRet);
			}
			else
			{
				printf("send success\n");
			}
			Sleep(40);
		}
	}
#else
	vlen = 0;
	file = NULL;
	vsize = 0;
	
	tFRAMEHDR.m_byAudioMode  = 8;
	tFRAMEHDR.m_dwPreBufSize = 0;
	tFRAMEHDR.m_byFrameRate  = 0;
	tFRAMEHDR.m_byMediaType  = MEDIA_TYPE_RAWAUDIO;
	
	g_pMediaSnd = CreateMediaSnd(10000,128<<10,0,MEDIA_TYPE_RAWAUDIO,0);
	wRet = SetMediaSndNetParam(g_pMediaSnd,tNetSndParam);
	if(wRet != MEDIASTREAM_NO_ERROR)
	{
		printf("error,wRet=%d\n",wRet);
		return -1;
	}

	if(1)
	{
		FILE *fp;
		char filename[32];
		sprintf(filename,"raw.pcm");
		fp = fopen(filename,"rb");
		if(fp != NULL)
		{
			while(1)
			{
				u8 buf[240];
				int ret = fread(buf,1,sizeof(buf),fp);
				//printf("ret=%d\n",ret);
				if(ret<=0)
				{
					fseek(fp,0,SEEK_SET);
					continue;
				}
				tFRAMEHDR.m_pData = buf;
				tFRAMEHDR.m_dwDataSize = ret;
				wRet = SendMediaFrame(g_pMediaSnd,&tFRAMEHDR);
				if(wRet != MEDIASTREAM_NO_ERROR)
				{
					printf("send failed,wRet=%d\n",wRet);
				}
				else
				{
					printf("send success\n");
				}
				Sleep(ret/8);
			}
			fclose(fp);
		}
	}
#endif
#endif

	DestroyMediaSnd(g_pMediaSnd);

	mediastreamCleanup();

	return 0;
}
#endif
