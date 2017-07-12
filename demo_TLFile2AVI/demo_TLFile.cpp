// demo_TLFile.cpp : Defines the entry point for the console application.
//

#include "avilib.h"
#include "stdio.h"
#include "windows.h"

#include "TLFileLib.h"
#pragma comment(lib,"NetDvr2.lib")

int convert(char* fnameIN,  char* fnameOUT);

int main(int argc, char* argv[])
{

	char fnameIN[MAX_PATH] = {0}, fnameOUT[MAX_PATH] = {0};

	if (argc<= 1)
	{
		printf("Invaild param! \n");
		return 0;
	}

	if (strlen(argv[1]) < 4)
	{
		printf("Invaild filename! \n");
		return 0;
	}

	if (stricmp(argv[1], "*.ifv"))
	{
		printf("filename: %s \n", argv[1]);

		strcpy(fnameIN, argv[1]);

		strcpy(fnameOUT, fnameIN);	
		fnameOUT[strlen(fnameOUT) - 3] = '\0';
		strcat(fnameOUT, "avi");

		convert(fnameIN, fnameOUT);
	}
	else
	{
		WIN32_FIND_DATA filedata;
		HANDLE hFind = FindFirstFile(argv[1], &filedata);
		if (INVALID_HANDLE_VALUE == hFind)
		{
			printf("can not find file\n");
			return 0;
		}

		printf("filename: %s \n", filedata.cFileName);

		memset(fnameOUT, 0, sizeof(fnameOUT));
		strcpy(fnameOUT, filedata.cFileName);	
		fnameOUT[strlen(fnameOUT) - 3] = '\0';
		strcat(fnameOUT, "avi");

		convert(filedata.cFileName, fnameOUT);

		while(FindNextFile(hFind, &filedata))
		{
			printf("filename: %s \n", filedata.cFileName);

			memset(fnameOUT, 0, sizeof(fnameOUT));
			strcpy(fnameOUT, filedata.cFileName);	
			fnameOUT[strlen(fnameOUT) - 3] = '\0';
			strcat(fnameOUT, "avi");
			
			convert(filedata.cFileName, fnameOUT);
		}

		FindClose(hFind);
	}

	printf("\n========== over ==============\n");
	return 0;
}

int convert(char* fnameIN, char* fnameOUT)
{
	avi_t* pAviHandle = NULL;
	unsigned char buf[256<<10] = {0};
	UINT dwDuration = 0;
	UINT start_time;
	BYTE key;
	int vsize = 0;

	TLFILE_t hfile = TL_OpenFile(fnameIN,TL_FILE_READ);
	if(hfile == NULL)
	{
		return 0;
	}
	
	int width = TL_FileVideoWidth(hfile);
	int height = TL_FileVideoHeight(hfile);
	int total_frames = TL_FileVideoFrameNum(hfile)+TL_FileAudioFrameNum(hfile);
	
	double frame_rate = TL_FileVideoFrameNum(hfile)*1000.0/TL_FileTotalTime(hfile);
	
	pAviHandle = AVI_open_output_file(fnameOUT);
	if(!pAviHandle)
	{
		
		TL_CloseFile(hfile);
		return 0;
	}
	
	AVI_set_video(pAviHandle,width,height,frame_rate,"H264");
	
	if (TL_FileHasAudio(hfile))
	{
		AVI_set_audio(pAviHandle,  1, TL_FileAudioSampleRate(hfile), 
			TL_FileAudioBits(hfile), WAVE_FORMAT_PCM, 0);
		
	}
	else
	{
		AVI_set_audio(pAviHandle, 0, 0, 0, 0, 0);
	}
	
	
	while (total_frames > 0)
	{	
		if (1)
		{
			BYTE mediatype = 0;
			vsize = TL_FileReadOneMediaFrame(hfile,buf,&start_time,&key,&mediatype);
			if (vsize <= 0)
			{
				break;
			}
			
			if (mediatype == 0) //video
			{
				int ret = AVI_write_frame(pAviHandle,(char *)buf,vsize,key);
				if (ret != 0)
				{
					break;
				}
			}
			else //audio
			{
				unsigned char decbuf[1024] = {0};
				int declen = 0;
				TL_FILEDecAudioFrame(hfile, buf, vsize, decbuf, &declen);
				
				int ret = AVI_write_audio(pAviHandle,(char *)decbuf,declen);
				
				if (ret != 0)
				{
					break;
				}
			}
			
		}
		
		total_frames--;
	}
	
	if (total_frames > 0)
	{
		
	}
	
	AVI_close(pAviHandle);
	TL_CloseFile(hfile);
	
	printf("ok \n");
	return 0;
}


#if 0
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
#endif
