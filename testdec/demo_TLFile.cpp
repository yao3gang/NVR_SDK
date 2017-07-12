// demo_TLFile.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "stdio.h"
#include "windows.h"


#include "TLFileLib.h"
#pragma comment(lib,"NetDvr2.lib")


int DecByAVCDEC( char* filename );

int DecByHISIDEC( char* filename );

int main(int argc, char* argv[])
{
	if (argc >1)
	{
		printf("\\\\\\\\\\\\\\\\\\\\\\\n");
		
		DecByAVCDEC(argv[1]);
		
		printf("\\\\\\\\\\\\\\\\\\\\\\\n");
		
		DecByHISIDEC(argv[1]);
	}




	return 0;
}





int DecByAVCDEC( char* filename )
{
	TLFILE_t hFileRead = TL_OpenFile(filename, TL_FILE_READ);
	if (!hFileRead)
	{
		printf("open err !\n");
		return 0;
	}
	
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
	
	printf("DecByAVCDEC: \n");
	int result;
	BYTE DecFrameBuf[704*576*2] = {0};
	int declen = 0;
	
  //读文件,解码

	DWORD dwStart = GetTickCount();
	__int64 llTotalsize = 0;
	for (int i=0; i< videonum; i++)
	{
		int nsize = TL_FileReadOneVideoFrame(hFileRead, buf, &framestarttime, &duration, &bykey);
		if (nsize>0)
		{
			llTotalsize += nsize;
			//printf("i=%d t=%ld size=%d bkey=%d \n", i, framestarttime, nsize, bykey);
			
			result = TL_FileDecVideoFrame(hFileRead, bykey, buf, nsize, DecFrameBuf, &declen);
			if (result == DEC_OK)
			{
				//printf("dec ok, len=%d \n", declen);
				
				//此处可将解码数据用来显示
			}
			else
			{
				printf("dec err\n");
			}
			
		} 
		else
		{
			printf("read err : %d \n", i);
		}
		
	}

	DWORD dwEnd = GetTickCount();
	printf("DecByAVCDEC: dur = %d, mspf: %f \n", dwEnd - dwStart, (dwEnd - dwStart)*1.0/videonum);
	
	llTotalsize /= 1024;
	nTotalTime /=1000;
	printf("DecByAVCDEC: KBps: %f \n", llTotalsize*1.0/nTotalTime);

	TL_CloseFile(hFileRead);
	return 0;	
}


#include "hi_config.h"
#include "hi_h264api.h"
#pragma comment(lib,"hi_h264dec_w.lib")

int DecByHISIDEC( char* filename )
{
	TLFILE_t hFileRead = TL_OpenFile(filename, TL_FILE_READ);
	if (!hFileRead)
	{
		printf("open err !\n");
		return 0;
	}
	
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
	
	printf("DecByHISIDEC: \n");
	int result;
	BYTE DecFrameBuf[704*576*2] = {0};
	int declen = 0;
	
	HI_HDL hHiDec = NULL;
	H264_DEC_ATTR_S   dec_attrbute;
	dec_attrbute.uBufNum        = 3;//16;				// reference frames number: 16
	dec_attrbute.uPicHeightInMB = height/16;//18;     
	dec_attrbute.uPicWidthInMB  = width/16;	//22;
    dec_attrbute.uStreamInType  = 0x00;					// bitstream begin with "00 00 01" or "00 00 00 01"
	dec_attrbute.uWorkMode = 0x11;
	hHiDec = Hi264DecCreate(&dec_attrbute);
    if(NULL ==  hHiDec)
    {
		
		return -1;
    }

	//读文件,解码
	
	DWORD dwStart = GetTickCount();
	__int64 llTotalsize = 0;
	for (int i=0; i< videonum; i++)
	{
		int nsize = TL_FileReadOneVideoFrame(hFileRead, buf, &framestarttime, &duration, &bykey);
		if (nsize>0)
		{
			llTotalsize += nsize;
			//printf("i=%d t=%ld size=%d bkey=%d \n", i, framestarttime, nsize, bykey);
			H264_DEC_FRAME_S  dec_frame;
			result = Hi264DecAU(hHiDec, (HI_U8 *)buf, nsize, 0, &dec_frame, 0);
			//		HI_S32 result = Hi264DecFrame(hHiDec, (HI_U8 *)buf, vsize, 0, &dec_frame, 0);
		
			if (result != HI_H264DEC_OK)
			{
				printf("dec err\n");
				//break;
			}
			else 
			{
				Hi264DecImageEnhance(hHiDec, &dec_frame, 40);
				const HI_U8 *pY = dec_frame.pY;
				const HI_U8 *pU = dec_frame.pU;
				const HI_U8 *pV = dec_frame.pV;
				HI_U32 width    = dec_frame.uWidth;
				HI_U32 height   = dec_frame.uHeight;
				HI_U32 yStride  = dec_frame.uYStride;
				HI_U32 uvStride = dec_frame.uUVStride;
				
				memcpy(DecFrameBuf, (LPBYTE)pY, width*height);
				memcpy(DecFrameBuf+width*height, (LPBYTE)pV, width*height/4);
				memcpy(DecFrameBuf+width*height*5/4, (LPBYTE)pU, width*height/4);
				
			}

		} 
		else
		{
			printf("read err : %d \n", i);
		}
		
	}
	
	DWORD dwEnd = GetTickCount();
	printf("DecByHISIDEC: dur = %d, mspf: %f \n", dwEnd - dwStart, (dwEnd - dwStart)*1.0/videonum);
	
	llTotalsize /= 1024;
	nTotalTime /=1000;
	printf("DecByHISIDEC: KBps: %f \n", llTotalsize*1.0/nTotalTime);
	
	if (hHiDec)
	{
		Hi264DecDestroy(hHiDec);
		hHiDec = NULL;
	}
	TL_CloseFile(hFileRead);
	return 0;		
}


