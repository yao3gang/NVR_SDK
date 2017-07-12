#include "TLFileLib.h"
#include "custommp4.h"

#define DEC_HISI
//#define DEC_AVCDEC

#ifdef DEC_HISI
#include "hi_config.h"
#include "hi_h264api.h"
#pragma comment(lib,"hi_h264dec_w.lib")
#endif

#ifdef DEC_AVCDEC
#include "avcdecoder.h" 
#pragma comment(lib,"avcdec.lib")
#endif

#include "hi_voice_api.h"
#pragma comment(lib,"lib_VoiceEngine_static.lib")

typedef void * Decoder_Handle;

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
};

#ifdef DEC_AVCDEC
void _stdcall MsgOut(void *user , char * msg)
{
	
}

void *my_aligned_malloc(int size, int alignment)
{
	void* ptr = malloc(size + alignment);
	
	if(ptr)
    {
		void* aligned = (void*)(((long)ptr + alignment) & ~(alignment - 1));
		((void**)aligned)[-1] = ptr;
		
		return aligned;
    }
	else
		return NULL;
}

void *my_aligned_free(void *paligned)
{
	//delete [ ]paligned;
	
	void* ptr = ((void**)paligned)[-1];
	free(ptr);
	return ptr;
}

typedef struct _byteStream_s 
{
	long bytesRead;               /* The number of bytes read from the file */
	unsigned char *bitbuf;        /* Buffer for stream bits */
	int bitbufLen;                /* Size of the bit buffer in bytes */
	int bitbufDataPos;
	int bitbufDataLen;            /* Length of all the data in the bit buffer in bytes */
	
	unsigned char *bitbufNalunit; /* Pointer to first NAL unit in the bitbuffer */
								  int bitbufNalunitLen;         /* Length of the first NAL unit in the bit buffer in bytes,
																including variable size start code, nal head byte and the
								  RBSP payload. It will help to find the RBSP length. */
} byteStream_s;

static int findStartCode(byteStream_s *str)
{
	int numZeros;
	int startCodeFound;
	int i;
	int currByte;
	
	numZeros       = 0;
	startCodeFound = 0;
	
	i = str->bitbufDataPos;
	
	/* Find sequence of 0x00 ... 0x00 0x01 */
	
	while (i < str->bitbufDataLen) {
		currByte = str->bitbuf[i];
		i++;
		
		if (currByte > 1) /* If current byte is > 1, it cannot be part of a start code */
			numZeros = 0;
		else if (currByte == 0)  /* If current byte is 0, it could be part of a start code */
			numZeros++;
		else {                    /* currByte == 1 */
			if (numZeros > 1) {  /* currByte == 1. If numZeros > 1, we found a start code */
				startCodeFound = 1;
				return i - 3;
			}
			numZeros = 0;
		}
	}
	return -1;
	
}

enum
{
	PARSE_STATE_INIT=0,
		PARSE_STATE_A,
		PARSE_STATE_B
};

int _stdcall My_avcdDecodeOneNal( avcdDecoder_t *dec, unsigned char *encbuf, int encdatalen, avcdYUVbuffer_s *outBuf , avcdYUVbuffer_s * refBuf, unsigned char keyflag )
{
	int result = -1;
	//if(nalUnitLen >30 && keyflag)
	if(3 == keyflag || 5 == keyflag)
	{
#if 1
		int index = 0;
		byteStream_s strByte;
		int nalPos , nalIndex = 0;
		int nalPosTab[5] = {0};

		strByte.bitbuf				= encbuf;
		strByte.bytesRead           = encdatalen;
		strByte.bitbufLen           = encdatalen;
		strByte.bitbufDataLen       = strByte.bytesRead;
		strByte.bitbufDataPos       = 0;
		strByte.bitbufNalunitLen    = 0;

		for(;;)
		{
			nalPos = findStartCode(&strByte);
			if(nalPos < 0)
			{
				nalPosTab[nalIndex++] = strByte.bitbufDataLen;
				break;
			}
			nalPosTab[nalIndex++] = nalPos;
			strByte.bitbufDataPos = nalPos + 3;
		}

		while (1) 
		{
			if (result != AVCD_OK_STREAM_NOT_DECODED) 
			{
				strByte.bitbufNalunit    = strByte.bitbuf + nalPosTab[index];
				strByte.bitbufNalunitLen = nalPosTab[index + 1] - nalPosTab[index];
				index++;
				if(strByte.bitbufNalunitLen <= 0)
				{
					strByte.bitbufNalunit = 0;
					strByte.bitbufNalunitLen = 0;
					break;
				}
				
				if(index > 4)break;
			}
			else
			{
				// printf("!!!!!!!!!!!!!!!!!!!!!!!AVCD_OK_STREAM_NOT_DECODED\n");
			}
			
			/* Decode one NAL unit */
			result = avcdDecodeOneNal(dec, strByte.bitbufNalunit,
				strByte.bitbufNalunitLen, outBuf , refBuf);

		
			
			if(outBuf->picType == AVCD_PIC_SPS)
			{
				// printf("hehe0.1\n");
			}
			
			if(outBuf->picType == AVCD_PIC_PPS)
			{
				// printf("hehe0.2\n");
			}
			
			//printf("hehe1\n");
			if(result == AVCD_OK)
			{
				return result;
			}
		}
#else		
	int nal_size = GetNalheadSize(encbuf);
	int i_data_size=encdatalen;
	int i_cur=nal_size;
	int i_cliplen=0;
	int decoffset = 0;
	int b_decode_finish=0;
	int i_parse_state;
	int b_find_nal, i_start_code_size;
	//[2] main loop
	while (!b_decode_finish)
	{
		i_parse_state=PARSE_STATE_INIT;
		
		//[2.2] extract a nal
		b_find_nal=0;
		
		while (i_cur<i_data_size && !b_find_nal)
		{
			switch(i_parse_state) 
			{
			case PARSE_STATE_INIT:
					if (0==encbuf[i_cur])
					{
						i_parse_state=PARSE_STATE_A;
						//printf("*****kakaka\n");
					}
					else
						i_parse_state=PARSE_STATE_INIT;
					break;
				case PARSE_STATE_A:
					if (0==encbuf[i_cur])
					{
						i_start_code_size=2;
						i_parse_state=PARSE_STATE_B;
					}
					else
						i_parse_state=PARSE_STATE_INIT;
					break;
				case PARSE_STATE_B:
					if (0==encbuf[i_cur])
					{
						i_start_code_size++;
						//printf("kakaka\n");
						i_parse_state=PARSE_STATE_B;
					}
					else if (1==encbuf[i_cur])
					{
						i_start_code_size++;
						b_find_nal=1;
					}
					else
					{
						//printf("hehe\n");
						i_parse_state=PARSE_STATE_INIT;
					}
					break;
				default:
					//printf("*****hehe\n");
					//can't jump here
					break;
				}
				i_cur++;//!!
			}//end of while (i_cur<i_data_size && !b_find_nal)
			
			
			
			//judge decode finish
			if (i_cur>=i_data_size)
			{
				b_decode_finish=1;
			}
			
			
			//update the data index
			i_cliplen=(int)i_cur-nal_size-decoffset;
			
			// printf("i_nal_start: %d\n ", i_cliplen);
			if (dec)
			{
				result = avcdDecodeOneNal(dec, encbuf+decoffset,i_cliplen, outBuf, refBuf);
			}
			
			decoffset += i_cliplen;
			
			// printf("resualt %d m_recoBuf.picType : %d\n ", result, m_recoBuf.picType);
		}
#endif
	}
	else
	{
		if (dec)
		{
			result = avcdDecodeOneNal(dec, encbuf, encdatalen, outBuf, refBuf);
		}
	}
	return result;
}
#endif

//csp modify 20121206
//#define MAX_BYTE 256*1024
#define MAX_BYTE 512*1024

TLFILE_t TL_OpenFile( char *filename , TL_FILE_OPENMODE mode)
{
	FILEHANDLE * pFileHandle = (FILEHANDLE *)malloc(sizeof(FILEHANDLE));
	if (!pFileHandle)
	{
		return NULL;
	}
	memset(pFileHandle, 0, sizeof(FILEHANDLE));

	pFileHandle->mode = mode;

	if (TL_FILE_READ == pFileHandle->mode)
	{
		HI_VOICE_DecReset(&(pFileHandle->decaudio_hdr),ADPCM_IMA);
		
		pFileHandle->pfile = custommp4_open(filename, O_R, 0);
	} 
	else if (TL_FILE_CREATE == pFileHandle->mode)
	{
		pFileHandle->pfile = custommp4_open(filename, O_W_CREAT, 0);
	}

	if (!pFileHandle->pfile)
	{
		free(pFileHandle);
		pFileHandle = NULL;
	}
	return pFileHandle;
}

void TL_CloseFile( TLFILE_t hFile )
{
	if (!hFile)
	{
		return;
	}

	FILEHANDLE * pFileHandle = (FILEHANDLE *)hFile;
	if (pFileHandle->pfile)
	{
		custommp4_close(pFileHandle->pfile);
	}
	
	if (pFileHandle->decoder_hdr)
	{
#ifdef DEC_HISI
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

BOOL TL_FileHasAudio( TLFILE_t hFile )
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

int TL_FileTotalTime( TLFILE_t hFile )
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

int TL_FileStartTime( TLFILE_t hFile )
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

int TL_FileEndTime( TLFILE_t hFile )
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

int TL_FileVideoFrameNum( TLFILE_t hFile )
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

int TL_FileAudioFrameNum( TLFILE_t hFile )
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

int TL_FileVideoWidth( TLFILE_t hFile )
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

int TL_FileVideoHeight( TLFILE_t hFile )
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

int TL_FileReadOneMediaFrame( TLFILE_t hFile,BYTE *meida_buffer,unsigned int *start_time,BYTE *bkey,BYTE *media_type )
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

int TL_FileReadOneVideoFrame( TLFILE_t hFile,BYTE *video_buffer,unsigned int *start_time,unsigned int *duration,BYTE *bkey )
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

int TL_FileReadOneAudioFrame( TLFILE_t hFile,BYTE *audio_buffer,unsigned int *start_time,unsigned int *duration )
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

void TL_FileSeekToPrevSeg( TLFILE_t hFile )
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

void TL_FileSeekToNextSeg( TLFILE_t hFile )
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

void TL_FileSeekToSysTime( TLFILE_t hFile, unsigned int systime )
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

DECRET TL_FileDecVideoFrame( TLFILE_t hFile, BYTE keyFlag, BYTE* encbuf, int enclen, BYTE* decbuf, int* pdeclen )
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

	if (!pFileHandle->decoder_hdr)
	{
#ifdef DEC_HISI
		H264_DEC_ATTR_S dec_attrbute;
		dec_attrbute.uBufNum        = (width>720)?1:2;	//1;//16;//reference frames number: 16
		dec_attrbute.uPicHeightInMB = (height+15)/16;	//68;//45;//18;
		dec_attrbute.uPicWidthInMB  = (width+15)/16;	//120;//80;//22;
		dec_attrbute.uStreamInType  = 0x00;				// bitstream begin with "00 00 01" or "00 00 00 01"
		dec_attrbute.uWorkMode = 0x11;
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
	
#ifdef DEC_HISI
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
	
	if (result==AVCD_ERROR)
	{
		return DEC_ERR;
	}
	else if (result==AVCD_OK)
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

DECRET TL_FILEDecAudioFrame( TLFILE_t hFile, BYTE* encbuf, int enclen, BYTE* decbuf, int* pdeclen )
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
		for(int i=0;i<enclen;i++)
		{   
			decbuf[i] = encbuf[i]+0x80;   // signed 2 unsigned
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

unsigned short TL_FileAudioBits( TLFILE_t hFile )
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

unsigned int TL_FileAudioSampleRate( TLFILE_t hFile )
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

void TL_FileSetVideo( TLFILE_t hFile, unsigned short width, unsigned short height, float frame_rate, VIDEOCOMPRESSOR compressor )
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

void TL_FileSetAudio( TLFILE_t hFile, unsigned short channels, unsigned short bits, unsigned int sample_rate, AUDIOCOMPRESSOR compressor, unsigned int sample_size, unsigned int sample_duration )
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

int TL_FileWriteVideoFrame( TLFILE_t hFile, BYTE *video_buffer,unsigned int bytes,unsigned int timestamp,BYTE keyflag )
{
	if (!hFile)
	{
		return 0;
	}	
	FILEHANDLE * pFileHandle = (FILEHANDLE *)hFile;
	if (pFileHandle->pfile && (TL_FILE_CREATE == pFileHandle->mode))
	{
		u8 update = 0;
	
		return custommp4_write_video_frame(pFileHandle->pfile, 
			video_buffer, 
			bytes, 
			timestamp, 
			keyflag, 
			&update);
	}
	return 0;	
}

int TL_FileWriteAudioFrame( TLFILE_t hFile, BYTE *audio_buffer,unsigned int bytes,unsigned int timestamp )
{
	if (!hFile)
	{
		return 0;
	}	
	FILEHANDLE * pFileHandle = (FILEHANDLE *)hFile;
	if (pFileHandle->pfile && (TL_FILE_CREATE == pFileHandle->mode))
	{
		u8 update = 0;
		
		return custommp4_write_audio_frame(pFileHandle->pfile, 
			audio_buffer, 
			bytes, 
			timestamp, 
 			&update);
	}
	return 0;	
}
