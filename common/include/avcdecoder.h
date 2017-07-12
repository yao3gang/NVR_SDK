
#ifndef _AVCDECODER_H_
#define _AVCDECODER_H_


#define AVCD_ERROR                       -1
#define AVCD_OK                          0
#define AVCD_OK_STREAM_NOT_DECODED       1
#define AVCD_OK_FRAME_NOT_AVAILABLE      2


typedef void avcdDecoder_t;

typedef enum _AVCD_PIC_TYPE_CONST
{
	AVCD_PIC_NONE,
	AVCD_PIC_SPS,
	AVCD_PIC_PPS,
	AVCD_PIC_IDR,
	AVCD_PIC_I,
	AVCD_PIC_P,
	AVCD_PIC_B,

}AVCD_PIC_TYPE_CONST;

typedef struct _avcdYUVbuffer_s 
{
	int width;
	int height;
	int picType; 
	void *y;
	void *u;
	void *v;
} avcdYUVbuffer_s;



typedef void _stdcall AVCD_ERROR_MSG_OUT_FUN(void * userData , char * msg);



#ifdef __cplusplus
extern "C" {
#endif
	avcdDecoder_t * _stdcall avcdOpen(AVCD_ERROR_MSG_OUT_FUN * errorMsgFun , void * userData);
	int _stdcall avcdDecodeOneNal(avcdDecoder_t *dec, void *nalUnitBits, int nalUnitLen,
		avcdYUVbuffer_s *outBuf  , avcdYUVbuffer_s * refBuf);
	void _stdcall avcdClose(avcdDecoder_t *dec);
#ifdef __cplusplus
};
#endif

#endif
