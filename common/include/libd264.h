#ifndef _LIBD264_H
#define _LIBD264_H

typedef struct
{
	int     i_plane;
	int     i_stride[4];
	unsigned char *plane[4];
} d264_image_t;

typedef struct
{
	int i_width;
	int i_height;

	d264_image_t img;
} d264_picture_t;

typedef struct
{
	int i_ref_idc;  /* nal_priority_e */
	int i_type;     /* nal_unit_type_e */

	/* This data are raw payload */
	int     i_payload;
	unsigned char *p_payload;
} d264_nal_t;


extern "C"
{
	void * _cdecl d264_decoder_open   ();
	void    _cdecl d264_decoder_close  ( void *h );
	int     _cdecl d264_decoder_decode( void *h ,d264_picture_t **pp_pic, d264_nal_t *nal );
	int _cdecl d264_nal_decode( d264_nal_t *nal, void *p_data, int i_data );
};


#endif