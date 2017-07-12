#ifndef _PARTITION_INDEX_H_
#define _PARTITION_INDEX_H_

#include <stdio.h>
#include "iflytype.h"
#ifndef WIN32
#include <pthread.h>
#endif

//#define SIZE_OF_FILE_VECTOR		((s64)128*1024*1024)
#define SIZE_OF_FILE_VECTOR		((s64)256*1024*1024)
#define SIZE_OF_RESERVED		((s32)393216)

#define RECTYPE_MASK_TIMER	0x1
#define RECTYPE_MASK_MD		0x2
#define RECTYPE_MASK_ALARM	0x4
#define RECTYPE_MASK_HAND	0x8
#define RECTYPE_MASK_ALL	0x10

#pragma pack( push, 1 )

typedef struct
{
	u16 file_no;
	u16 sect_nums;
	u32 start_time;
	u32 end_time;
}chn_last_use_file;

typedef struct
{
	u32 op_nums;
	u32 reserved1;
	u32 version;
	u32 reserved2;
	u32 total_file_nums;
	u32 recorded_file_nums;
	u32 full_file_nums;
	u32 full_file_offset;
	chn_last_use_file chn_info[33];
	u8  reserved3[80];
	u32 verify;
}partition_index_header;

typedef struct
{
	u32 file_no;
	u8  chn_no;
	u8  busy;
	u16 sect_nums;
	u32 start_time;
	u32 end_time;
}file_use_info;

typedef struct
{
	u8  type;
	u8  image_format;
	u8  stream_flag;
	u8  video_compressor;
	u8  audio_compressor;
	u8  reserved1[11];
	u32 start_time;
	u32 end_time;
	u32 start_position;
	u32 end_position;
}segment_use_info;

typedef struct
{
	u8   channel_no;
	u8   type;
	u32  start_time;
	u32  end_time;
	u32  card_no;
	u8   mask;
}search_param_t;

typedef struct
{
	u8   channel_no;
	u8   type;
	u32  start_time;
	u32  end_time;
	u8   image_format;//3:cif;4:4cif
	u8   stream_flag;//0:ÊÓÆµÁ÷;1:ÒôÆµÁ÷
	u32	 size;
	u32  offset;
	u8   disk_no;
	u8   ptn_no;
	u16  file_no;
}recfileinfo_t;

#pragma pack( pop )

typedef struct
{
	FILE *index1;
	FILE *index2;
	partition_index_header header;
	u8 valid;
#ifndef WIN32
	pthread_mutex_t lock;
#endif
}partition_index;

int fileflush(FILE *fp);
int filecp(char *src,char *dst);
int seek_to_segment(partition_index *index,int file_no,int sect_no);
int init_partition_index(partition_index *index,char *path);
int destroy_partition_index(partition_index *index);
s64 get_partition_total_space(partition_index *index);
s64 get_partition_free_space(partition_index *index);
int get_chn_next_segment(partition_index *index,int chn,int *file_no,int *sect_offset);
int get_first_full_file_end_time(partition_index *index,u32 *end_time);
int get_chn_next_segment_force(partition_index *index,int chn,int *file_no,int *sect_offset);
int update_chn_cur_segment(partition_index *index,int chn,segment_use_info *p_s_u_info,u8 finished);
int search_rec_file(partition_index *index,search_param_t *search,recfileinfo_t *fileinfo_buf,int max_nums,u8 disk_no,u8 ptn_no);

#endif
