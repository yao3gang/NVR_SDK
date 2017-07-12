#ifndef _CUSTOM_MP4_COMMON_H_
#define _CUSTOM_MP4_COMMON_H_

#include <stdio.h>
#include "iflytype.h"


#ifdef __cplusplus
extern "C" {
#endif
	
#ifdef WIN32
#include <wtypes.h>
#else
typedef struct _GUID
{
    unsigned long Data1;
    unsigned short Data2;
    unsigned short Data3;
    unsigned char Data4[8];
}GUID;
#include <pthread.h>
#endif

extern const GUID CUSTOMMP4_file_properties_object;
extern const GUID CUSTOMMP4_video_stream_properties_object;
extern const GUID CUSTOMMP4_audio_stream_properties_object;
extern const GUID CUSTOMMP4_mdat_vector_object;
extern const GUID CUSTOMMP4_mdat_segment_object;

#define CUSTOMMP4_FILE_DESCRIBE	"This is DVR record file ver 2"

#define O_R				0
#define O_W_CREAT		1
#define O_W_APPEND		2
#define O_W_AUTO		3

#pragma pack( push, 1 )

typedef struct
{
	GUID object_id;
	u32	 object_size;
}base_object_t;

typedef struct
{
	base_object_t obj;
	char describe[32];
	u32  start_time;
	u32  end_time;
	u32  file_Size;
	u8   video_tracks;
	u8   audio_tracks;
	u16  reserved1;
}file_properties_object_t;

typedef struct
{
	base_object_t obj;
	u32 time_scale;
	u16 width;
	u16 height;
	float frame_rate;
	u32 bit_rate;
	u32 compressor;
	u32 dpi_horizontal;
	u32 dpi_vertical;
	u16 depth;
	u16 flags;
	u32 reserved1;
}video_stream_properties_object_t;

typedef struct
{
	base_object_t obj;
	u32 time_scale;
	u16 channels;
	u16 sample_bits;
	u32 sample_rate;
	u32 compressor;
	u32 sample_size;
	u32 sample_duration;
	u32 reserved1;
}audio_stream_properties_object_t;

typedef struct
{
	base_object_t obj;
	u32 number_of_segments;
	u32 reserved1;
}mdat_vector_object_t;

typedef struct
{
	base_object_t obj;
	u32 start_time;
	u32 end_time;
	u32 video_frames;
	u32 audio_frames;
	u32 video_allocated;
	u32 audio_allocated;
	u32 reserved1;
}mdat_segment_object_t;

typedef struct
{
	u32	offset;
	u32 start_time;
	u32 end_time;
	u32 video_frames;
	u32 audio_frames;
	u32 video_allocated;
	u32 audio_allocated;
}mdat_segment_info_table_t;

typedef struct
{
	u32 offset;
	u32 length;
	u64 pts;
	u32 timestamp;
	u8  key;
	u8  reserved1;  
	u16 reserved2;
	u32 reserved3;
}video_frame_info_table_t;

typedef struct
{
	u32 offset;
	u32 length;
	u64 pts;
	u32 timestamp;
	u32 reserved;
}audio_frame_info_table_t;

#pragma pack( pop )

typedef struct
{
	FILE *stream;
	u8 open_mode;
	long open_offset;
	long file_position;
	
	file_properties_object_t fpot;
	video_stream_properties_object_t vspot;
	audio_stream_properties_object_t aspot;
	mdat_vector_object_t mvot;

	mdat_segment_info_table_t *p_mdat_sect_t;
	
	video_frame_info_table_t *p_video_table_t;
	audio_frame_info_table_t *p_audio_table_t;
	
	u32 video_table_allocated;
	u32 audio_table_allocated;
	
	u32 total_video_frames;
	u32 total_audio_frames;
	
	u32 current_media_sect_pos;
	u32 current_video_offset;
	u32 current_audio_offset;

	u32 current_video_sect_pos;//for video read
	u32 current_audio_sect_pos;//for audio read
	u32 current_video_frame_pos;//for video read
	u32 current_audio_frame_pos;//for audio read

/*#ifndef WIN32
	pthread_mutex_t lock;
#endif*/
#ifdef WIN32
	HANDLE lock;
#endif
}custommp4_t;

void uint2str(unsigned char *dst, int n);
u32 str2uint(const char *str);

custommp4_t* custommp4_open(char *filename,u8 open_mode,u32 open_offset);
int custommp4_close(custommp4_t *file);

int custommp4_set_video(custommp4_t *file,u32 time_scale,u16 width,u16 height,float frame_rate,u32 bit_rate,u32 compressor,u16 depth);
int custommp4_set_audio(custommp4_t *file,u32 time_scale,u16 channels,u16 bits,u32 sample_rate,u32 compressor,u32 sample_size,u32 sample_duration);

BOOL custommp4_has_video(custommp4_t *file);
BOOL custommp4_has_audio(custommp4_t *file);

int custommp4_total_time(custommp4_t *file);

int custommp4_video_length(custommp4_t *file);
int custommp4_audio_length(custommp4_t *file);

u16 custommp4_video_width(custommp4_t *file);
u16 custommp4_video_height(custommp4_t *file);
u16 custommp4_video_depth(custommp4_t *file);
float custommp4_video_frame_rate(custommp4_t *file);
u32 custommp4_video_bit_rate(custommp4_t *file);
u32 custommp4_video_compressor(custommp4_t *file);
u32 custommp4_video_time_scale(custommp4_t *file);

u16 custommp4_audio_channels(custommp4_t *file);
u16 custommp4_audio_bits(custommp4_t *file);
u32 custommp4_audio_sample_rate(custommp4_t *file);
u32 custommp4_audio_sample_size(custommp4_t *file);
u32 custommp4_audio_compressor(custommp4_t *file);
u32 custommp4_audio_time_scale(custommp4_t *file);

int custommp4_read_one_media_frame(custommp4_t *file,u8 *meida_buffer,u32 maxBytes,u32 *start_time,u8 *key,u8 *media_type);

int custommp4_video_frame_size(custommp4_t *file,int frame);
int custommp4_audio_frame_size(custommp4_t *file,int frame);

int custommp4_read_video_frame(custommp4_t *file,u8 *video_buffer,u32 maxBytes,int frame,u32 *start_time,u8 *key);
int custommp4_read_audio_frame(custommp4_t *file,u8 *audio_buffer,u32 maxBytes,int frame,u32 *start_time);

int custommp4_read_one_video_frame(custommp4_t *file,u8 *video_buffer,u32 maxBytes,u32 *start_time,u32 *duration,u8 *key);
int custommp4_read_one_audio_frame(custommp4_t *file,u8 *audio_buffer,u32 maxBytes,u32 *start_time,u32 *duration);

int custommp4_seek_to_prev_key_frame(custommp4_t *file);

int custommp4_seek_to_prev_segment(custommp4_t *file);
int custommp4_seek_to_next_segment(custommp4_t *file);

int custommp4_seek_to_time_stamp(custommp4_t *file,u32 timestamp);
int custommp4_seek_to_sys_time(custommp4_t *file,u32 systime);

int custommp4_write_video_frame(custommp4_t *file,u8 *video_buffer,u32 bytes,u32 timestamp,u8 isKeyFrame,u8 *update);
int custommp4_write_audio_frame(custommp4_t *file,u8 *audio_buffer,u32 bytes,u32 timestamp,u8 *update);

BOOL custommp4_object_is(base_object_t *pobj,GUID type);

int custommp4_position(custommp4_t *file);
int custommp4_set_position(custommp4_t *file, int position);
int custommp4_end_position(custommp4_t *file);

int custommp4_read_data(custommp4_t *file,void *data,int size);
int custommp4_write_data(custommp4_t *file,void *data,int size);

 void fpot_endian_get(file_properties_object_t *p_fpot);
 void fpot_endian_put(file_properties_object_t *p_fpot);
 void base_object_endian_get(base_object_t *obj);
 void base_object_endian_put(base_object_t *obj);
 void vspot_endian_get(video_stream_properties_object_t *vspot);
 void vspot_endian_put(video_stream_properties_object_t *vspot);
 void aspot_endian_get(audio_stream_properties_object_t *aspot);
 void aspot_endian_put(audio_stream_properties_object_t *aspot);
 void mvot_endian_get(mdat_vector_object_t *mvot);
 void mvot_endian_put(mdat_vector_object_t *mvot);
 void msot_endian_get(mdat_segment_object_t *msot);
 void msot_endian_put(mdat_segment_object_t *msot);
 void vframe_endian_get(video_frame_info_table_t *pvframe);
 void vframe_endian_put(video_frame_info_table_t *pvframe);
 void aframe_endian_get(audio_frame_info_table_t *paframe);
 void aframe_endian_put(audio_frame_info_table_t *paframe);
#ifdef __cplusplus
}
#endif

#endif
