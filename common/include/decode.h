#ifndef _6441DECODE_H_
#define _6441DECODE_H_

#ifdef __cplusplus
extern "C" {
#endif


void * dec_init();
	
void dec_setoutfomat(void *dec_handle, const char* outfomat);
	
int dec_oneframe(void *dec_handle,
						unsigned char *istream,
						unsigned char *ostream,
						int istream_size,
						int inwidth,
						int inheight
						);

int dec_stop(void *dec_handle);

#ifdef __cplusplus
}
#endif

#endif