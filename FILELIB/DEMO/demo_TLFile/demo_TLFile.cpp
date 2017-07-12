// demo_TLFile.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "stdio.h"
#include "windows.h"


#include "TLFileLib.h"
#pragma comment(lib,"NetDvr2.lib")



int main(int argc, char* argv[])
{
	TLFILE_t hFileRead = TL_OpenFile("test.ifv", TL_FILE_READ);
	if (!hFileRead)
	{
		printf("open err !\n");
		return 0;
	}

	BOOL bHasAudio = TL_FileHasAudio(hFileRead);
	printf("Has aduio : %d\n", bHasAudio);

	int nTotalTime = TL_FileTotalTime(hFileRead);
	int nStart = TL_FileStartTime(hFileRead);
	int nEnd = TL_FileEndTime(hFileRead);
 	printf("TotalTime : %d sec. %d - %d\n", nTotalTime, nStart, nEnd);

	int width = TL_FileVideoWidth(hFileRead);
	int height = TL_FileVideoHeight(hFileRead);
	printf("width :%d, height: %d \n", width, height);

	int videonum = TL_FileVideoFrameNum(hFileRead);
	int audionum = TL_FileAudioFrameNum(hFileRead);
	printf("videonum: %d audionum: %d\n",videonum, audionum);
	BYTE buf[256*1024];
	unsigned int framestarttime = 0;
	unsigned int duration = 0;
	BYTE bykey = 0;




	printf("TL_FileReadOneVideoFrame: \n");
	int result;
	BYTE DecFrameBuf[704*576*2] = {0};
	int declen = 0;

#if 1   //读文件,解码
	for (int i=0; i< videonum; i++)
	{
		int nsize = TL_FileReadOneVideoFrame(hFileRead, buf, &framestarttime, &duration, &bykey);
		if (nsize>0)
		{
			printf("i=%d size=%d bkey=%d \n", i, nsize, bykey);

			result = TL_FileDecVideoFrame(hFileRead, bykey, buf, nsize, DecFrameBuf, &declen);
			if (result == DEC_OK)
			{
				printf("dec ok, len=%d \n", declen);

				//此处可将解码数据用来显示

			}
			else
			{
				printf("dec err\n");
			}

			Sleep(duration);

		} 
		else
		{
			printf("read err : %d \n", i);
		}

	}
#endif

	printf("TL_FileReadOneMediaFrame: \n");

	TL_FileSeekToSysTime(hFileRead, nStart);
	framestarttime = 0;

	BYTE type =0;
	int index = 0;
	unsigned int lastframetime = 0;

	TLFILE_t hFileWrite = TL_OpenFile("write.ifv", TL_FILE_CREATE);

	TL_FileSetVideo(hFileWrite, width, height, 25, TLFILE_V_H264);
	TL_FileSetAudio(hFileWrite, 1, 8, 8000, TLFILE_A_GRAW, 512, 64);

	// 另一种接口读文件,然后写文件
	while (index<(videonum+audionum))
	{
		int nsize = TL_FileReadOneMediaFrame(hFileRead, buf, &framestarttime, &bykey, &type);
		if (nsize>0)
		{
			printf("index=%d size=%d type=%d \n", index, nsize, type);
			// Sleep(framestarttime-lastframetime);
			lastframetime = framestarttime;

			if (type == 0) //video
			{
				TL_FileWriteVideoFrame(hFileWrite, buf, nsize, framestarttime, bykey);
			} 
			else //audio
			{
				TL_FileWriteAudioFrame(hFileWrite, buf, nsize, framestarttime);
			}
		} 
		else
		{
			printf("read err : %d %d %d\n", index, framestarttime, nsize);
		}
		index++;
	}
	TL_CloseFile(hFileRead);
	return 0;
}
