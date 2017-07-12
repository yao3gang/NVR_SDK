#include "ctrlprotocol.h"
#include <stdio.h>
#include "loopbuf.h"

#pragma comment( lib, "Ws2_32.lib" )

#include "TLFileLib.h"
#pragma comment(lib,"TLFileLib.lib")

u16 DealCommand(CPHandle cph,u16 event,u8 *pbyMsgBuf,int msgLen,u8 *pbyAckBuf,int *pAckLen,void* pContext)
{
// 	ifly_monitor_param_t mp;
// 	printf("hehe,recv event:%d from(0x%08x,%d)\n",event,cph->ip,cph->port);
// 	if(pAckLen) *pAckLen = 0;
// 	switch(event)
// 	{
// 	case CTRL_CMD_STARTMONITOR:
// 		memcpy(&mp,pbyMsgBuf,sizeof(ifly_monitor_param_t));
// 		printf("start mp:(0x%08x,%d,%d)\n",mp.dwIp,mp.wPort,mp.byChn);
// 		return CTRL_FAILED_OUTOFMEMORY;
// 		break;
// 	case CTRL_CMD_STOPMONITOR:
// 		memcpy(&mp,pbyMsgBuf,sizeof(ifly_monitor_param_t));
// 		printf("stop mp:(0x%08x,%d,%d)\n",mp.dwIp,mp.wPort,mp.byChn);
// 		return CTRL_FAILED_RESOURCE;
// 		break;
// 	}
	
	return CTRL_SUCCESS;
}


typedef enum									//播放速率枚举
{
	PLAYRATE_1 = 1,								//正常播放
		PLAYRATE_2 = 2,								//2x快放
		PLAYRATE_4 = 4,								//4x快放
		PLAYRATE_MAX = 8,							//8x快放
		PLAYRATE_1_2 = -2,							//1/2x慢放
		PLAYRATE_1_4 = -4,							//1/4x慢放
		PLAYRATE_MIN = -8,							//1/8x慢放
		PLAYRATE_SINGLE = 0,						//帧进
		PLAYRATE_BACK_1 = -1						//1x退放
}em_play_rate;

typedef struct
{
	SOCKHANDLE	sockfd;
	u8	bStart;
	u32 linkid;
	ifly_TCP_Stream_Req req;
	union
	{
		struct  
		{
			u32 frameid;
		}Monitor_t;
		
		struct
		{
			CPHandle cph;
			u8   byFlag;//0:按文件播放;1:按时间回放
			u8   byUse;
			u8   byStop;
			u8   byPause;
			u8   byMute;
			u8   byStatus;
			u8   bySkip;
			em_play_rate rate;
			u32  curPos;
			u32  totalTime;
			u32  refTime;
			u32  seekPos;
		}PlayBack_t;
		
		struct
		{
			u8   byUse;
			u8   byStop;
		}DonwLoad_t;
		
		struct  
		{
			CPHandle cph;
		}Update_t;
	};
	
}ifly_stearmsnd_t;

ifly_stearmsnd_t g_snd[STREAM_LINK_MAXNUM];

u16 GetNewIdNum()
{
	static u32 dwIDNum = 0;

	if (++dwIDNum == 0)
	{
		dwIDNum = 1;
	}
	return dwIDNum;
}

int AddStreamTCPLink(SOCKHANDLE hSock, ifly_TCP_Stream_Req req, void* pContext)
{

	int i=0;
	printf("AddStreamTCPLink \n");
	for (i=0; i<STREAM_LINK_MAXNUM; i++ )
	{
		if(g_snd[i].sockfd == INVALID_SOCKET)
		{
			g_snd[i].sockfd = hSock;
			memcpy(&g_snd[i].req, &req,sizeof(ifly_TCP_Stream_Req));
			g_snd[i].bStart =1;
			return 0;
		}

	}
	return -1;
}


static ifly_msgQ_t netsndMsgQ;

void* netsndthread(void *pParam)
{
	u8   *pBuf;
	u32  dwReadLen;
	int	 i;
	while(1)
	{
		for (i=0; i<STREAM_LINK_MAXNUM; i++ )
		{
			if((g_snd[i].sockfd != INVALID_SOCKET) && (g_snd[i].bStart))
			{
				if (g_snd[i].req.command == 0) //video monitor
				{
					if (g_snd[i].linkid == 0)
					{
						ifly_TCP_Stream_Ack ack;
						g_snd[i].linkid = GetNewIdNum();
						ack.ackid = htonl(g_snd[i].linkid);
						ack.errcode = htonl(0);
						send(g_snd[i].sockfd, (char *)&ack, sizeof(ack), 0);

					} 
					else
					{
						if (FALSE == GetMsgQReadInfo(&netsndMsgQ,&pBuf,&dwReadLen))
						{

							Sleep(0);
							continue;
						}

						g_snd[i].Monitor_t.frameid++;

						{
							u32 remain = dwReadLen;
							u32 sendlen = 0;
							int ret = 0;
							while (remain > 0)
							{
								//			printf("s4:%u:%u\n", sendlen, remain);
								ret = send(g_snd[i].sockfd, pBuf + sendlen, remain, 0);
								if(ret < 0)
								{
									//printf("socket send failed ret = %d errno = %d, err msg = %s\n", ret, errno, strerror(errno));
									break;
								}
								else if(ret == 0)
								{
									//printf("socket send ret = 0\n");
									break;
								}
								sendlen += ret;
								remain -= ret;
							}	
						}

						 printf("%d\n",GetTickCount());
						skipReadMsgQ(&netsndMsgQ);

					}
				}


			}
			else
			{
				Sleep(0);
				continue;
			}
			
		}
	}


	return 0;
}

#define  MEDIA_TYPE_H264	 (u8)98
void* testCapStreamThread(void *pParam)
{
	TLFILE_t hFile;
	BOOL bHasAudio;
	int nTotalTime;
	int nStart, nEnd;
	int width, height;
	int videonum, audionum;
	BYTE buf[256*1024];
	unsigned int framestarttime = 0;
	unsigned int duration = 0;
	BYTE bkey = 0;
	u32 frameid = 0;
	int i;
	hFile = TL_OpenFile("test.ifv");
	if (!hFile)
	{
		printf("open err !\n");
		return 0;
	}

	bHasAudio = TL_FileHasAudio(hFile);
	printf("Has aduio : %d\n", bHasAudio);

	nTotalTime = TL_FileTotalTime(hFile);
	nStart = TL_FileStartTime(hFile);
	nEnd = TL_FileEndTime(hFile);
 	printf("TotalTime : %d sec. %d - %d\n", nTotalTime, nStart, nEnd);

	width = TL_FileVideoWidth(hFile);
	height = TL_FileVideoHeight(hFile);
	printf("width :%d, height: %d \n", width, height);

	videonum = TL_FileVideoFrameNum(hFile);
	audionum = TL_FileAudioFrameNum(hFile);
	printf("videonum: %d audionum: %d\n",videonum, audionum);


	printf("TL_FileReadOneVideoFrame: \n");

	for ( i=0; i< videonum; i++)
	{
		int starttick;
		int sleeptime;
		int nsize;
		starttick= GetTickCount();
		nsize = TL_FileReadOneVideoFrame(hFile, buf, &framestarttime, &duration, &bkey);
		if (nsize>0)
		{
			u8 *pBuf = NULL;
	
			ifly_MediaFRAMEHDR_t mfheader;
			u32 dwLen = nsize + sizeof(mfheader);
			
			frameid ++;
			// printf("i=%d size=%d bkey=%d \n", i, nsize, bkey);
			if (GetMsgQWriteInfo(&netsndMsgQ,&pBuf,&dwLen))
			{

				mfheader.m_byMediaType = MEDIA_TYPE_H264;
				mfheader.m_byFrameRate = 25;
				mfheader.m_bKeyFrame = 25;
				mfheader.m_nVideoWidth = htonl(width);
				mfheader.m_nVideoHeight = htonl(height);
				mfheader.m_dwDataSize = htonl(nsize);
				mfheader.m_dwFrameID = htonl(frameid);
				mfheader.m_dwTimeStamp = htonl(framestarttime);

				memcpy(pBuf,&mfheader,sizeof(mfheader));
				memcpy(pBuf+sizeof(mfheader),buf,nsize);
				
				skipWriteMsgQ(&netsndMsgQ);
			}
			else
			{
				printf("GetMsgQWriteInfo faild \n");
			}
			sleeptime = duration-(GetTickCount()-starttick);
			if (sleeptime <0)
			{
				sleeptime = 40;
			} 
			
			// printf("dur = %d\n", sleeptime);
			Sleep(sleeptime);
			

		} 
		else
		{
			printf("read err : %d \n", i);
		}

		if (i == videonum-1)
		{
			TL_FileSeekToSysTime(hFile, nStart);
			i = 0;
		}

	}

	TL_CloseFile(hFile);


	return 0;
}

#ifndef MAX_FRAME_SIZE
#define MAX_FRAME_SIZE				(INT32)256*1024
#endif
int main()
{

	int i=0;
	ifly_DeviceInfo_t info;
	HANDLE hThreadCap =NULL, hThreadSnd = NULL;
	info.deviceIP = inet_addr("192.168.1.23");
	info.devicePort = htons(6630);
	strcpy(info.device_name, "visualdecvice");
	strcpy(info.device_mode, "R9016");
	info.maxChnNum = 16;
	info.maxAduioNum = 16;
	info.maxSubstreamNum = 16;
	info.maxPlaybackNum = 4;
	info.maxAlarmInNum = 16;
	info.maxAlarmOutNum = 4;
	info.maxHddNum = 8;
	SetDeviceInfo(info);
	
	SetMsgCallBack(DealCommand,NULL);

	memset(g_snd,0,sizeof(g_snd));
	for (i=0; i<STREAM_LINK_MAXNUM; i++ )
	{
		g_snd[i].sockfd = INVALID_SOCKET;
	}

	SetAddStreamLinkCB(AddStreamTCPLink, 0);

	CPLibInit(CTRL_PROTOCOL_SERVERPORT);

	initMsgQ(&netsndMsgQ, (VIDEO_CHANNELS) >> 1, MAX_FRAME_SIZE + sizeof(ifly_MediaFRAMEHDR_t));

	hThreadCap = CreateThread(NULL,1<<20,(LPTHREAD_START_ROUTINE)testCapStreamThread,NULL,0,NULL);

	hThreadSnd = CreateThread(NULL,1<<20,(LPTHREAD_START_ROUTINE)netsndthread,NULL,0,NULL);

	while(1)
	{
		Sleep(1000);
	}

	if (hThreadCap)
	{
		CloseHandle(hThreadCap);
	}
	if (hThreadSnd)
	{
		CloseHandle(hThreadSnd);
	}
	destroyMsgQ(&netsndMsgQ);
	CPLibCleanup(TRUE);

	return 0;
}
