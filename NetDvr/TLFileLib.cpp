#include "TLFileLib.h"
#include "NetDvrPrivate.h"


struct FILEHANDLE 
{
	custommp4_t* pfile;
	Decoder_Handle decoder_hdr;
	hiVOICE_ADPCM_STATE_S decaudio_hdr;
#ifdef DEC_AVCDEC
	avcdYUVbuffer_s m_recoBuf;
	avcdYUVbuffer_s m_refBuf;
#endif
	TL_FILE_OPENMODE mode;
	TL_FILE_WRITEMODE writemode;
	char *writebuf;
	unsigned int maxwritebuflen;
	unsigned int w_buf_len;
};

#define MAX_BYTE MAX_FRAME_SIZE

TLFILE_t __stdcall TL_OpenFile( char *filename , TL_FILE_OPENMODE mode, TL_FILE_WRITEMODE writemode, char *writebuf, unsigned int maxwritebuflen)
{
	FILEHANDLE * pFileHandle = (FILEHANDLE *)malloc(sizeof(FILEHANDLE));
	if (!pFileHandle)
	{
		return NULL;
	}
	memset(pFileHandle, 0, sizeof(FILEHANDLE));

	pFileHandle->mode = mode;
	pFileHandle->writemode = writemode;
	pFileHandle->writebuf = writebuf;
	pFileHandle->maxwritebuflen = maxwritebuflen;

	if (TL_FILE_WRITEWITHBUF == writemode)
	{
		if (NULL == writebuf)
		{
			free(pFileHandle);
			pFileHandle = NULL;
		}
		return NULL;
	}

	if (TL_FILE_READ == pFileHandle->mode)
	{
		HI_VOICE_DecReset(&(pFileHandle->decaudio_hdr),ADPCM_IMA);
		
		pFileHandle->pfile = custommp4_open(filename, O_R, writemode, 0);
	} 
	else if (TL_FILE_CREATE == pFileHandle->mode)
	{
		pFileHandle->pfile = custommp4_open(filename, O_W_CREAT, writemode, 0);
	}

	if (!pFileHandle->pfile)
	{
		free(pFileHandle);
		pFileHandle = NULL;
	}
	return pFileHandle;
}

void __stdcall TL_CloseFile( TLFILE_t hFile )
{
	if (!hFile)
	{
		return;
	}

	FILEHANDLE * pFileHandle = (FILEHANDLE *)hFile;
	if (pFileHandle->pfile)
	{
		if ((TL_FILE_CREATE == pFileHandle->mode) &&(pFileHandle->w_buf_len > 0))
		 {
			 custommp4_write_data(pFileHandle->pfile, pFileHandle->writebuf, pFileHandle->w_buf_len);	
			 pFileHandle->w_buf_len = 0;
		}
		custommp4_close(pFileHandle->pfile);
	}
	
	if(pFileHandle->decoder_hdr)
	{
#ifdef _NVR_
		Hi264DecDestroy(pFileHandle->decoder_hdr);
		pFileHandle->decoder_hdr = NULL;
#endif
#ifdef DEC_AVCDEC
		avcdClose(pFileHandle->decoder_hdr);
		my_aligned_free(pFileHandle->m_recoBuf.y);
		my_aligned_free(pFileHandle->m_recoBuf.u);
		my_aligned_free(pFileHandle->m_recoBuf.v);
		my_aligned_free(pFileHandle->m_refBuf.y);
		my_aligned_free(pFileHandle->m_refBuf.u);
		my_aligned_free(pFileHandle->m_refBuf.v);
#endif
	}
	
	free(pFileHandle);
	pFileHandle = NULL;
}

BOOL __stdcall TL_FileHasAudio( TLFILE_t hFile )
{
	if (!hFile)
	{
		return FALSE;
	}
	FILEHANDLE * pFileHandle = (FILEHANDLE *)hFile;
	if (pFileHandle->pfile && (TL_FILE_READ == pFileHandle->mode))
	{
		return custommp4_has_audio(pFileHandle->pfile);
	}
	return FALSE;	
}

int __stdcall TL_FileTotalTime( TLFILE_t hFile )
{
	if (!hFile)
	{
		return 0;
	}
	FILEHANDLE * pFileHandle = (FILEHANDLE *)hFile;
	if (pFileHandle->pfile && (TL_FILE_READ == pFileHandle->mode))
	{
		return custommp4_total_time(pFileHandle->pfile);
	}
	return 0;
}

int __stdcall TL_FileStartTime( TLFILE_t hFile )
{
	if (!hFile)
	{
		return 0;
	}
	FILEHANDLE * pFileHandle = (FILEHANDLE *)hFile;
	if (pFileHandle->pfile && (TL_FILE_READ == pFileHandle->mode))
	{
		return pFileHandle->pfile->fpot.start_time;
	}
	return 0;
}

int __stdcall TL_FileEndTime( TLFILE_t hFile )
{
	if (!hFile)
	{
		return 0;
	}
	FILEHANDLE * pFileHandle = (FILEHANDLE *)hFile;
	if (pFileHandle->pfile && (TL_FILE_READ == pFileHandle->mode))
	{
		return pFileHandle->pfile->fpot.end_time;
	}
	return 0;
}

int __stdcall TL_FileVideoFrameNum( TLFILE_t hFile )
{
	if (!hFile)
	{
		return 0;
	}
	FILEHANDLE * pFileHandle = (FILEHANDLE *)hFile;
	if (pFileHandle->pfile && (TL_FILE_READ == pFileHandle->mode))
	{
		return custommp4_video_length(pFileHandle->pfile);
	}
	return 0;
}

int __stdcall TL_FileAudioFrameNum( TLFILE_t hFile )
{
	if (!hFile)
	{
		return 0;
	}	
	FILEHANDLE * pFileHandle = (FILEHANDLE *)hFile;
	if (pFileHandle->pfile && (TL_FILE_READ == pFileHandle->mode))
	{
		return custommp4_audio_length(pFileHandle->pfile);
	}
	return 0;
}

int __stdcall TL_FileVideoWidth( TLFILE_t hFile )
{
	if (!hFile)
	{
		return 0;
	}	
	FILEHANDLE * pFileHandle = (FILEHANDLE *)hFile;
	if (pFileHandle->pfile && (TL_FILE_READ == pFileHandle->mode))
	{
		return custommp4_video_width(pFileHandle->pfile);
	}
	return 0;
}

int __stdcall TL_FileVideoHeight( TLFILE_t hFile )
{
	if (!hFile)
	{
		return 0;
	}
	FILEHANDLE * pFileHandle = (FILEHANDLE *)hFile;
	if (pFileHandle->pfile && (TL_FILE_READ == pFileHandle->mode))
	{
		return custommp4_video_height(pFileHandle->pfile);
	}
	return 0;
}

int __stdcall TL_FileReadOneMediaFrame( TLFILE_t hFile,BYTE *meida_buffer,unsigned int *start_time,BYTE *bkey,BYTE *media_type )
{
	if (!hFile || !meida_buffer || !start_time || !bkey || !media_type)
	{
		return 0;
	}
	FILEHANDLE * pFileHandle = (FILEHANDLE *)hFile;
	if (pFileHandle->pfile && (TL_FILE_READ == pFileHandle->mode))
	{ 
		return custommp4_read_one_media_frame(pFileHandle->pfile, meida_buffer, MAX_BYTE, start_time, bkey, media_type);
	}
	return 0;
}

int __stdcall TL_FileReadOneVideoFrame( TLFILE_t hFile,BYTE *video_buffer,unsigned int *start_time,unsigned int *duration,BYTE *bkey )
{
	if (!hFile || !video_buffer || !start_time || !bkey || !duration)
	{
		return 0;
	}
	FILEHANDLE * pFileHandle = (FILEHANDLE *)hFile;
	if (pFileHandle->pfile && (TL_FILE_READ == pFileHandle->mode))
	{
		return custommp4_read_one_video_frame(pFileHandle->pfile, video_buffer,MAX_BYTE, start_time, duration, bkey);
	}
	return 0;
}

int __stdcall TL_FileReadOneAudioFrame( TLFILE_t hFile,BYTE *audio_buffer,unsigned int *start_time,unsigned int *duration )
{
	if (!hFile || !audio_buffer || !start_time || !duration)
	{
		return 0;
	}	
	FILEHANDLE * pFileHandle = (FILEHANDLE *)hFile;
	if (pFileHandle->pfile && (TL_FILE_READ == pFileHandle->mode))
	{
		return custommp4_read_one_audio_frame(pFileHandle->pfile, audio_buffer,MAX_BYTE, start_time, duration);
	}
	return 0;
}

void __stdcall TL_FileSeekToPrevSeg( TLFILE_t hFile )
{
	if (!hFile)
	{
		return ;
	}	
	FILEHANDLE * pFileHandle = (FILEHANDLE *)hFile;
	if (pFileHandle->pfile && (TL_FILE_READ == pFileHandle->mode))
	{
		custommp4_seek_to_prev_segment(pFileHandle->pfile);
	}
}

void __stdcall TL_FileSeekToNextSeg( TLFILE_t hFile )
{
	if (!hFile)
	{
		return ;
	}	
	FILEHANDLE * pFileHandle = (FILEHANDLE *)hFile;
	if (pFileHandle->pfile && (TL_FILE_READ == pFileHandle->mode))
	{
		custommp4_seek_to_next_segment(pFileHandle->pfile);
	}
	
}

void __stdcall TL_FileSeekToSysTime( TLFILE_t hFile, unsigned int systime )
{
	if (!hFile)
	{
		return ;
	}
	FILEHANDLE * pFileHandle = (FILEHANDLE *)hFile;
	if (pFileHandle->pfile && (TL_FILE_READ == pFileHandle->mode))
	{
		custommp4_seek_to_sys_time(pFileHandle->pfile, systime);
	}
}

DECRET __stdcall TL_FileDecVideoFrame( TLFILE_t hFile, BYTE keyFlag, BYTE* encbuf, int enclen, BYTE* decbuf, int* pdeclen )
{
	if (!hFile || !encbuf || !decbuf || !pdeclen)
	{
		return DEC_ERR;
	}
	FILEHANDLE * pFileHandle = (FILEHANDLE *)hFile;
	if (!pFileHandle->pfile && (TL_FILE_READ == pFileHandle->mode))
	{
		return DEC_ERR;
	}
	
	int width = TL_FileVideoWidth(hFile);
	int height = TL_FileVideoHeight(hFile);
	
	if(!pFileHandle->decoder_hdr)
	{
#ifdef _NVR_
		H264_DEC_ATTR_S dec_attrbute;
		dec_attrbute.uBufNum        = (width>720)?2:2;	//1;//16;//reference frames number: 16
//		dec_attrbute.uPicHeightInMB = (height+15)/16;	//68;//45;//18;
//		dec_attrbute.uPicWidthInMB  = (width+15)/16;	//120;//80;//22;
		dec_attrbute.uPicHeightInMB = (height+15)/16;	//68;//45;//18;
		dec_attrbute.uPicWidthInMB  = ((width+63)/64*64)/16;//120;//80;//22;//解决长视网络摄像机bug
		dec_attrbute.uStreamInType  = 0x00;				//bitstream begin with "00 00 01" or "00 00 00 01"
		dec_attrbute.uWorkMode      = 0x01;				//0x11;//不需要deinteralce
//		dec_attrbute.uWorkMode      |= 0x20;			//多线程
		if ((pFileHandle->decoder_hdr = Hi264DecCreate(&dec_attrbute)) == NULL)
		{
			return DEC_ERR;
		}
#endif
#ifdef DEC_AVCDEC
		if( ( pFileHandle->decoder_hdr = avcdOpen(MsgOut , 0) ) == NULL )
		{
			return DEC_ERR;
		}
		int size = width * height;
		pFileHandle->m_recoBuf.width = width;
		pFileHandle->m_recoBuf.height = height;
		pFileHandle->m_refBuf.width = width;
		pFileHandle->m_refBuf.height = height;
		pFileHandle->m_recoBuf.y = my_aligned_malloc(size , 16);
		pFileHandle->m_recoBuf.u = my_aligned_malloc(size >> 2 , 16);
		pFileHandle->m_recoBuf.v = my_aligned_malloc(size >> 2 , 16);
		pFileHandle->m_refBuf.y = my_aligned_malloc(size , 16);
		pFileHandle->m_refBuf.u = my_aligned_malloc(size >> 2 , 16);
		pFileHandle->m_refBuf.v = my_aligned_malloc(size >> 2 , 16);
#endif
	}
	
#ifdef _NVR_
	int dec_succ_flg = 0;
	
	hiH264_DEC_FRAME_S dec_frame;
	HI_S32 hisiresult;
	
	hisiresult = Hi264DecFrame(pFileHandle->decoder_hdr, encbuf, enclen, 0, &dec_frame, 0);
	
	while(HI_H264DEC_NEED_MORE_BITS != hisiresult)
	{
		if(HI_H264DEC_NO_PICTURE == hisiresult)   //flush over and all the remain picture are output
		{
			break;
		}
		
		if(hisiresult == HI_H264DEC_OK)
		{
			Hi264DecImageEnhance(pFileHandle->decoder_hdr, &dec_frame, 40);
			
			const HI_U8 *pY = dec_frame.pY;
			const HI_U8 *pU = dec_frame.pU;
			const HI_U8 *pV = dec_frame.pV;
			
			memcpy(decbuf, (LPBYTE)pY, 
				width*height);
			memcpy(decbuf+width*height, 
				(LPBYTE)pV, 
				width*height/4);
			memcpy(decbuf+width*height*5/4, 
				(LPBYTE)pU, 
				width*height/4);
			
			*pdeclen = width*height*3/2;
			
			dec_succ_flg = 1;
		}
		
		hisiresult = Hi264DecFrame(pFileHandle->decoder_hdr, NULL, 0, 0, &dec_frame, 0);
	}
	
	if(dec_succ_flg)
	{
		return DEC_OK;
	}
	else
	{
		return DEC_MORE;
	}
#endif
	
#ifdef DEC_AVCDEC
	int result = My_avcdDecodeOneNal(pFileHandle->decoder_hdr, encbuf,enclen, &pFileHandle->m_recoBuf , &pFileHandle->m_refBuf, keyFlag);
	
	if(result==AVCD_ERROR)
	{
		return DEC_ERR;
	}
	else if(result==AVCD_OK)
	{
		memcpy(decbuf, (LPBYTE)pFileHandle->m_recoBuf.y, 
			width*height);
		memcpy(decbuf+width*height, 
			(LPBYTE)pFileHandle->m_recoBuf.v, 
			width*height/4);
		memcpy(decbuf+width*height*5/4, 
			(LPBYTE)pFileHandle->m_recoBuf.u, 
			width*height/4);
		
		void * tmp;
		
		tmp = pFileHandle->m_recoBuf.y;
		pFileHandle->m_recoBuf.y = pFileHandle->m_refBuf.y;
		pFileHandle->m_refBuf.y = tmp;
		
		tmp = pFileHandle->m_recoBuf.u;
		pFileHandle->m_recoBuf.u = pFileHandle->m_refBuf.u;
		pFileHandle->m_refBuf.u = tmp;
		
		tmp = pFileHandle->m_recoBuf.v;
		pFileHandle->m_recoBuf.v = pFileHandle->m_refBuf.v;
		pFileHandle->m_refBuf.v = tmp;
		
		*pdeclen = width*height*3/2;
		return DEC_OK;
	}
	else
	{
		return DEC_MORE;
	}
#endif
}

DECRET __stdcall TL_FILEDecAudioFrame( TLFILE_t hFile, BYTE* encbuf, int enclen, BYTE* decbuf, int* pdeclen )
{
	if (!hFile || !encbuf || !decbuf || !pdeclen)
	{
		return DEC_ERR;
	}
	FILEHANDLE * pFileHandle = (FILEHANDLE *)hFile;
	if (!pFileHandle->pfile && (TL_FILE_READ == pFileHandle->mode))
	{
		return DEC_ERR;
	}

	DWORD dwCpsr = custommp4_audio_compressor(pFileHandle->pfile);
	BYTE Cpsrbuf[5] = {0};
	uint2str(Cpsrbuf, dwCpsr);
	if (0 == strcmp((char *)Cpsrbuf, "GRAW"))
	{
		if (8==custommp4_audio_bits(pFileHandle->pfile))
		{
			for(int i=0;i<enclen;i++)
			{   
				decbuf[i] = encbuf[i]+0x80;   // signed 2 unsigned
			}
		}
		else
		{
			memcpy(decbuf, encbuf, enclen);
		}

		* pdeclen = enclen;
	} 
	else if (0 == strcmp((char *)Cpsrbuf, "ADPB"))
	{
		HI_S16 len=0;
		if (HI_VOICE_DecodeFrame(&(pFileHandle->decaudio_hdr),(HI_S16 *)(encbuf),(HI_S16 *)decbuf,&len)!=0)
		{
			return DEC_ERR;
		}
		* pdeclen = len*sizeof(HI_S16);
	}

	return DEC_OK;
}

unsigned short __stdcall TL_FileAudioBits( TLFILE_t hFile )
{
	if (!hFile)
	{
		return 0;
	}	
	FILEHANDLE * pFileHandle = (FILEHANDLE *)hFile;
	if (pFileHandle->pfile && (TL_FILE_READ == pFileHandle->mode))
	{
		return custommp4_audio_bits(pFileHandle->pfile);
	}
	return 0;	
}

unsigned int __stdcall TL_FileAudioSampleRate( TLFILE_t hFile )
{
	if (!hFile)
	{
		return 0;
	}	
	FILEHANDLE * pFileHandle = (FILEHANDLE *)hFile;
	if (pFileHandle->pfile && (TL_FILE_READ == pFileHandle->mode))
	{
		return custommp4_audio_sample_rate(pFileHandle->pfile);
	}
	return 0;		
}

void __stdcall TL_FileSetVideo( TLFILE_t hFile, unsigned short width, unsigned short height, float frame_rate, VIDEOCOMPRESSOR compressor )
{
	if (!hFile)
	{
		return ;
	}	
	FILEHANDLE * pFileHandle = (FILEHANDLE *)hFile;
	if (pFileHandle->pfile && (TL_FILE_CREATE == pFileHandle->mode))
	{
		char cmprs[5] = {0};
		switch (compressor)
		{
		case TLFILE_V_H264:
			strcpy(cmprs, "H264");
			break;
		}

		custommp4_set_video(pFileHandle->pfile, 
			1000,	
			width, 
			height, 
			frame_rate, 
			512 * 1024, 
			str2uint(cmprs), 
			0x18);
	}
	return ;
}

void __stdcall TL_FileSetAudio( TLFILE_t hFile, unsigned short channels, unsigned short bits, unsigned int sample_rate, AUDIOCOMPRESSOR compressor, unsigned int sample_size, unsigned int sample_duration )
{
	if (!hFile)
	{
		return ;
	}	
	FILEHANDLE * pFileHandle = (FILEHANDLE *)hFile;
	if (pFileHandle->pfile && (TL_FILE_CREATE == pFileHandle->mode))
	{
		char cmprs[5] = {0};
		switch (compressor)
		{
		case TLFILE_A_GRAW:
			strcpy(cmprs, "GRAW");
			break;
		case TLFILE_A_ADPA:
			strcpy(cmprs, "ADPA");
			break;
		case TLFILE_A_ADPB:
			strcpy(cmprs, "ADPB");
			break;
		}
		
		custommp4_set_audio(pFileHandle->pfile, 
			1000,	
			channels, 
			bits, 
			sample_rate, 
			str2uint(cmprs), 
			sample_size,
			sample_duration);
	}
	return ;	
}

int __stdcall TL_FileWriteVideoFrame( TLFILE_t hFile, BYTE *video_buffer,unsigned int bytes,unsigned int timestamp, BYTE keyflag, unsigned int frameID )
{
	if (!hFile)
	{
		return 0;
	}	
	FILEHANDLE * pFileHandle = (FILEHANDLE *)hFile;
	if (pFileHandle->pfile && (TL_FILE_CREATE == pFileHandle->mode))
	{
		u8 update = 0;
	
		if ((NULL != pFileHandle->writebuf) && (TL_FILE_WRITEWITHBUF == pFileHandle->writemode))
		{
			return custommp4_write_video_frame_with_buf(pFileHandle->pfile, 
				video_buffer, 
				bytes, 
				timestamp, 
				keyflag,
				frameID,
				0,
				&update,
				pFileHandle->writebuf, 
				&pFileHandle->w_buf_len, 
				pFileHandle->maxwritebuflen);
		}
		else
		{
			return custommp4_write_video_frame(pFileHandle->pfile, 
				video_buffer, 
				bytes, 
				timestamp, 
				keyflag,
				frameID,
				&update);
		}
	}
	return 0;	
}

int __stdcall TL_FileWriteAudioFrame( TLFILE_t hFile, BYTE *audio_buffer,unsigned int bytes,unsigned int timestamp )
{
	if (!hFile)
	{
		return 0;
	}	
	FILEHANDLE * pFileHandle = (FILEHANDLE *)hFile;
	if (pFileHandle->pfile && (TL_FILE_CREATE == pFileHandle->mode))
	{
		u8 update = 0;
		if ((NULL != pFileHandle->writebuf) && (TL_FILE_WRITEWITHBUF == pFileHandle->writemode))
		{
			return custommp4_write_audio_frame_with_buf(pFileHandle->pfile, 
				audio_buffer, 
				bytes, 
				timestamp, 
				&update,
				pFileHandle->writebuf, 
				&pFileHandle->w_buf_len, 
				pFileHandle->maxwritebuflen);
		}
		else
		{
			return custommp4_write_audio_frame(pFileHandle->pfile, 
				audio_buffer, 
				bytes, 
				timestamp, 
				&update);
		}

	}
	return 0;	
}

BOOL __stdcall SnapBMPFile(char* strbmpfilename, unsigned char* YV12Buf, unsigned short width, unsigned short height)
{
	unsigned char *bufy , *bufu , *bufv;
	short int r, c,R, G, B, y, u, v; 
	BYTE *pRGBbuf;
	long iIndex;
	
	pRGBbuf = (BYTE *)malloc(width * height * 3);
	
	if (!pRGBbuf)
	{
		return FALSE;
	}


	bufy = YV12Buf;
	bufv = YV12Buf + width * height;
	bufu = YV12Buf + width * height * 5/4;
	
	iIndex=3*width*(height-1);
	for (r = 0; r < height; r++) 
	{
		for (c = 0; c < width; c++) 
		{
			y = bufy [c];
			u = bufu [c >> 1];
			
			v = bufv [c >> 1];
			
// 			R = short(1.164 * (y - 16) + 1.159 * (v - 128));
// 			G = short(1.164 * (y - 16) - 0.38 * (u - 128) - 0.813 * (v - 128));
// 			B = short(1.164 * (y - 16) + 2.018 * (u - 128));
			R = short((1164 * (y - 16) + 1159 * (v - 128))/1000);
			G = short((1164 * (y - 16) - 380 * (u - 128) - 813 * (v - 128))/1000);
			B = short((1164 * (y - 16) + 2018 * (u - 128))/1000);
			R = max (0, min (255, R));
			G = max (0, min (255, G));
			B = max (0, min (255, B));
			
			pRGBbuf[iIndex + 3 * c] = (BYTE)B;
			pRGBbuf[iIndex + 1 + 3 * c] = (BYTE)G;
			pRGBbuf[iIndex + 2 + 3 * c] = (BYTE)R;
		}
		
		iIndex -= 3 * width;
		bufy += width;
		
		if (r % 2)
		{
			bufu += width / 2;
			bufv += width / 2;
		}
	}


	FILE* fp_rgb = fopen(strbmpfilename, "wb");
	if (fp_rgb)
	{
		BITMAPFILEHEADER hdr;
		BITMAPINFOHEADER *bih;
		bih = (BITMAPINFOHEADER *)malloc(sizeof(BITMAPINFOHEADER));
		
		bih->biSize = sizeof(BITMAPINFOHEADER);
		bih->biWidth  = width;
		bih->biHeight = height;  
		bih->biPlanes = 1;
		bih->biCompression = BI_RGB;
		bih->biBitCount = 24;
		bih->biSizeImage = width * height * 3;
		bih->biXPelsPerMeter = 0;
		bih->biYPelsPerMeter = 0;
		bih->biClrUsed = 0;
		bih->biClrImportant = 0;
		
		hdr.bfType = ((WORD)('M'<<8) | 'B');
		hdr.bfSize = width * height * 3 + sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER);
		hdr.bfReserved1 = 0;
		hdr.bfReserved2 = 0;
		hdr.bfOffBits = (DWORD)(sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER) );
		
		fwrite(&hdr, 1, sizeof(BITMAPFILEHEADER), fp_rgb);
		fflush(fp_rgb);
		fwrite(bih, 1, sizeof(BITMAPINFOHEADER), fp_rgb);
		fflush(fp_rgb);
		fwrite(pRGBbuf, 1, width * height * 3, fp_rgb);
		fflush(fp_rgb);
		free(bih);
		
		fclose(fp_rgb);
	}
	
	free(pRGBbuf);

	return TRUE;
}

BOOL __stdcall SnapJPGFile(char* strjpgfilename, unsigned char* YV12Buf, unsigned short width, unsigned short height)
{

	BYTE *sampleBuffer;
	short int m_iMaxW, m_iMaxH;
	struct jpeg_compress_struct cinfo;
	struct jpeg_error_mgr jerr;
	FILE * outfile; /* target file */
	JSAMPIMAGE buffer;
	int band, i, buf_width[3], buf_height[3];

	int counter;
	u8 *yuvdata[3];
	
	if (NULL == (outfile = fopen(strjpgfilename, "wb")))
	{
		return FALSE;
	}
	
	m_iMaxW = width;
	m_iMaxH = height;
	
	sampleBuffer = (u8 *)YV12Buf;
	
	yuvdata[0] = (u8 *)sampleBuffer;

	yuvdata[1] = (u8 *)sampleBuffer + m_iMaxW * m_iMaxH * 5 / 4;
	yuvdata[2] = (u8 *)sampleBuffer + m_iMaxW * m_iMaxH;

	
	cinfo.err = jpeg_std_error(&jerr);
	
	jpeg_create_compress(&cinfo);
	
	jpeg_stdio_dest(&cinfo, outfile);
	
	cinfo.image_width = m_iMaxW; /* image width and height, in pixels */
	cinfo.image_height = m_iMaxH;
	cinfo.input_components = 3; /* # of color components per pixel */
	cinfo.in_color_space = JCS_YCbCr;//JCS_RGB; /* colorspace of input image */
	
	jpeg_set_defaults(&cinfo);
	
	jpeg_set_quality(&cinfo, 100, TRUE );
	
	//////////////////////////////
	cinfo.raw_data_in = TRUE;
	cinfo.jpeg_color_space = JCS_YCbCr;
	cinfo.comp_info[0].h_samp_factor = 2;
	cinfo.comp_info[0].v_samp_factor = 2;
	/////////////////////////
	
	jpeg_start_compress(&cinfo, TRUE);
	
	buffer = (JSAMPIMAGE) (*cinfo.mem->alloc_small) ((j_common_ptr) &cinfo, JPOOL_IMAGE, 3 * sizeof(JSAMPARRAY)); 
	for (band = 0; band < 3; band++)
	{
		buf_width[band] = cinfo.comp_info[band].width_in_blocks * DCTSIZE;
		buf_height[band] = cinfo.comp_info[band].v_samp_factor * DCTSIZE;
		buffer[band] = (*cinfo.mem->alloc_sarray) ((j_common_ptr) &cinfo, JPOOL_IMAGE, buf_width[band], buf_height[band]);
	} 
	
	int max_line = cinfo.max_v_samp_factor*DCTSIZE; 
	for (counter = 0; cinfo.next_scanline < cinfo.image_height; counter++)
	{ 
		//buffer image copy.
		for(band=0; band < 3; band++)
		{
			int mem_size = buf_width[band];
			BYTE * pDst = (BYTE *) buffer[band][0];
			BYTE * pSrc = (BYTE *) yuvdata[band] + counter*buf_height[band] * buf_width[band];//yuv.data[band]分别表示YUV起始地址
			
			for(i=0; i<buf_height[band]; i++)
			{
				memcpy(pDst, pSrc, mem_size);
				pSrc += buf_width[band];
				pDst += buf_width[band];
			}
		}
		jpeg_write_raw_data(&cinfo, buffer, max_line);
	}
	
	jpeg_finish_compress(&cinfo);
	
	fclose(outfile);
	
	jpeg_destroy_compress(&cinfo);
	return TRUE;
}

int __stdcall TL_FileEndPosition(TLFILE_t hFile)
{
	if (!hFile)
	{
		return 0;
	}	
	FILEHANDLE * pFileHandle = (FILEHANDLE *)hFile;
	if (!pFileHandle->pfile)
	{
		return 0;
	}
	return custommp4_end_position(pFileHandle->pfile);
}
