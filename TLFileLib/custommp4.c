#include "custommp4.h"
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <time.h>
#ifndef WIN32
#include <unistd.h>
#include <stdlib.h>
#endif
#ifndef WIN32
#include "platform.h"
#if __BYTE_ORDER == __BIG_ENDIAN
#include <asm/byteorder.h>
#ifdef __le16_to_cpu
#define CF_LE_W	__le16_to_cpu
#define CF_LE_L	__le32_to_cpu
#define CT_LE_W	__cpu_to_le16
#define CT_LE_L	__cpu_to_le32
#else
#define CF_LE_W(v) ((((v) & 0xff) << 8) | (((v) >> 8) & 0xff))
#define CF_LE_L(v) (((unsigned)(v)>>24) | (((unsigned)(v)>>8)&0xff00) | \
               (((unsigned)(v)<<8)&0xff0000) | ((unsigned)(v)<<24))
#define CT_LE_W(v) CF_LE_W(v)
#define CT_LE_L(v) CF_LE_L(v)
#endif /* defined(__le16_to_cpu) */

#else
#define CF_LE_W(v) (v)
#define CF_LE_L(v) (v)
#define CT_LE_W(v) (v)
#define CT_LE_L(v) (v)
#endif /* __BIG_ENDIAN */
#else
#define CF_LE_W(v) (v)
#define CF_LE_L(v) (v)
#define CT_LE_W(v) (v)
#define CT_LE_L(v) (v)
#endif
//#define SIZE_OF_FILE_VECTOR		((s64)128*1024*1024)
#define SIZE_OF_FILE_VECTOR		((s64)256*1024*1024)
extern int SetReadDataFlag(u8 byFlag);
extern int fileflush(FILE *fp);
const GUID CUSTOMMP4_file_properties_object			= {0xABD3D211L,0xA9BA,0x11cf,{0x8E,0xE6,0x00,0xC0,0x0C,0x20,0x53,0x65}};
const GUID CUSTOMMP4_video_stream_properties_object	= {0x86D15241L,0x311D,0x11D0,{0xA3,0xA4,0x00,0xA0,0xC9,0x03,0x48,0xF6}};
const GUID CUSTOMMP4_audio_stream_properties_object	= {0x4B1ACBE3L,0x100B,0x11D0,{0xA3,0x9B,0x00,0xA0,0xC9,0x03,0x48,0xF6}};
const GUID CUSTOMMP4_mdat_vector_object				= {0x4CFEDB20L,0x75F6,0x11CF,{0x9C,0x0F,0x00,0xA0,0xC9,0x03,0x49,0xCB}};
const GUID CUSTOMMP4_mdat_segment_object			= {0x1806D474L,0xCADF,0x4509,{0xA4,0xBA,0x9A,0xAB,0xCB,0x96,0xAA,0xE8}};
#if 0
enum frame_type
{
	H264_FRAME_TYPE_NULL,
	H264_FRAME_TYPE_SPS,
	H264_FRAME_TYPE_PPS,
	H264_FRAME_TYPE_IDR,
	H264_FRAME_TYPE_I,
	H264_FRAME_TYPE_P
};
#endif
enum frame_type
{
	H264_FRAME_TYPE_P = 0,
	H264_FRAME_TYPE_SPS,
	H264_FRAME_TYPE_PPS,
	H264_FRAME_TYPE_IDR,
	H264_FRAME_TYPE_I,
	
};

BOOL is_i_frame(u8 video_type)
{
	return video_type;
}

#ifdef WIN32
int SetReadDataFlag(u8 byFlag)
{

	return 0;
}
#endif

int lock_custommp4(custommp4_t *file)
{
/*#ifndef WIN32
	return pthread_mutex_lock(&file->lock);
#endif*/
#ifdef WIN32
	WaitForSingleObject(file->lock,INFINITE);
#endif
	return 1;
}

int unlock_custommp4(custommp4_t *file)
{
/*#ifndef WIN32
	return pthread_mutex_unlock(&file->lock);
#endif*/
#ifdef WIN32
	ReleaseMutex(file->lock);
#endif
	return 1;
}

int custommp4_init(custommp4_t *file)
{
	memset(file,0,sizeof(custommp4_t));
/*#ifndef WIN32
	pthread_mutex_init(&file->lock,NULL);
#endif*/
#ifdef WIN32
	file->lock = CreateMutex(NULL, FALSE, NULL);
#endif
	return 1;
}

int custommp4_destroy(custommp4_t *file)
{
/*#ifndef WIN32
	pthread_mutex_destroy(&file->lock);
#endif*/
#ifdef WIN32
	CloseHandle(file->lock);
#endif
	if(file->p_mdat_sect_t) free(file->p_mdat_sect_t);
	if(file->p_video_table_t) free(file->p_video_table_t);
	if(file->p_audio_table_t) free(file->p_audio_table_t);
	if(file->stream)
	{
		int result = fclose(file->stream);
		if(result)
		{
			printf("custommp4_destroy:fclose error=%d,errno=%d,errstr=%s\n",result,errno,strerror(errno));
			#ifndef WIN32
// 			sleep(1);
// 			exit(1);
			#endif
		}
	}
	free(file);
	return 1;
}

int custommp4_file_properties_object_init(file_properties_object_t *pfpot)
{
	pfpot->obj.object_id = CUSTOMMP4_file_properties_object;
	pfpot->obj.object_size = sizeof(file_properties_object_t);
	pfpot->video_tracks = 0;
	pfpot->audio_tracks = 0;
	pfpot->start_time = (u32)(time(NULL));
	pfpot->end_time = pfpot->start_time;
	pfpot->file_Size = sizeof(file_properties_object_t)+sizeof(video_stream_properties_object_t)+sizeof(audio_stream_properties_object_t)+sizeof(mdat_vector_object_t);
	//printf("custommp4_file_properties_object_init:file_Size=%d\n",pfpot->file_Size);
	pfpot->reserved1 = 0;
	strncpy(pfpot->describe,CUSTOMMP4_FILE_DESCRIBE,sizeof(pfpot->describe));
	return 1;
}

int custommp4_video_stream_properties_object_init(video_stream_properties_object_t *pvspot)
{
	pvspot->obj.object_id = CUSTOMMP4_video_stream_properties_object;
	pvspot->obj.object_size = sizeof(video_stream_properties_object_t);
	pvspot->time_scale = 1000;
	pvspot->width = 352;
	pvspot->height = 288;
	pvspot->frame_rate = 25;
	pvspot->bit_rate = 512*1024;
	pvspot->compressor = str2uint("XVID");
	pvspot->depth = 0x18;
	pvspot->dpi_horizontal = 72;
	pvspot->dpi_vertical = 72;
	pvspot->flags = 0;
	pvspot->reserved1 = 0;
	return 1;
}

int custommp4_audio_stream_properties_object_init(audio_stream_properties_object_t *paspot)
{
	paspot->obj.object_id = CUSTOMMP4_audio_stream_properties_object;
	paspot->obj.object_size = sizeof(audio_stream_properties_object_t);
	paspot->time_scale = 1000;
	paspot->channels = 1;
	paspot->sample_bits = 8;
	paspot->sample_rate = 8000;
	paspot->compressor = str2uint("PCMU");
	paspot->sample_duration = 30;
	paspot->sample_size = 240;
	paspot->reserved1 = 0;
	return 1;
}

int custommp4_mdat_vector_object_init(mdat_vector_object_t *pmvot)
{
	pmvot->obj.object_id = CUSTOMMP4_mdat_vector_object;
	pmvot->obj.object_size = sizeof(mdat_vector_object_t);
	pmvot->number_of_segments = 0;
	pmvot->reserved1 = 0;
	return 1;
}

int custommp4_mdat_segment_info_table_init(custommp4_t *file,mdat_segment_info_table_t *p_mdat_sect_t)
{
	p_mdat_sect_t->video_frames = 0;
	p_mdat_sect_t->audio_frames = 0;
	p_mdat_sect_t->start_time = time(NULL);
	p_mdat_sect_t->end_time = p_mdat_sect_t->start_time;
	p_mdat_sect_t->offset = custommp4_position(file);
	if(file->fpot.video_tracks) p_mdat_sect_t->video_allocated = 3000;
	else p_mdat_sect_t->video_allocated = 0;
	if(file->fpot.audio_tracks) p_mdat_sect_t->audio_allocated = 4000;
	else p_mdat_sect_t->audio_allocated = 0;
	return 1;
}

int ReadFilePropertiesObject(custommp4_t *file)
{
	int ret;
	ret = custommp4_read_data(file, &file->fpot, sizeof(file_properties_object_t));
	fpot_endian_get(&file->fpot);
	return ret;
}
 void fpot_endian_get(file_properties_object_t *p_fpot)
 {
	p_fpot->start_time = CF_LE_L(p_fpot->start_time);	
	p_fpot->end_time = CF_LE_L(p_fpot->end_time);	
	p_fpot->file_Size = CF_LE_L(p_fpot->file_Size);	
	p_fpot->reserved1 = CF_LE_W(p_fpot->reserved1);
	base_object_endian_get(&p_fpot->obj);
 }
void base_object_endian_get(base_object_t *obj)
{
	obj->object_size = CF_LE_L(obj->object_size);	
	obj->object_id.Data1 = CF_LE_L(obj->object_id.Data1);
	obj->object_id.Data2 = CF_LE_W(obj->object_id.Data2);
	obj->object_id.Data3 = CF_LE_W(obj->object_id.Data3);
}
 void fpot_endian_put(file_properties_object_t *p_fpot)
 {
	p_fpot->start_time = CT_LE_L(p_fpot->start_time);	
	p_fpot->end_time = CT_LE_L(p_fpot->end_time);	
	p_fpot->file_Size = CT_LE_L(p_fpot->file_Size);	
	p_fpot->reserved1 = CT_LE_W(p_fpot->reserved1);
	base_object_endian_put(&p_fpot->obj);
 }
void base_object_endian_put(base_object_t *obj)
{
	obj->object_size = CT_LE_L(obj->object_size);	
	obj->object_id.Data1 = CT_LE_L(obj->object_id.Data1);
	obj->object_id.Data2 = CT_LE_W(obj->object_id.Data2);
	obj->object_id.Data3 = CT_LE_W(obj->object_id.Data3);
}
int ReadVideoStreamPropertiesObject(custommp4_t *file)
{
	int ret;
	ret = custommp4_read_data(file,&file->vspot,sizeof(video_stream_properties_object_t));
	vspot_endian_get(&file->vspot);
	return ret;
}
 void vspot_endian_get(video_stream_properties_object_t *vspot)
 {
	vspot->time_scale = CF_LE_L(vspot->time_scale);
	vspot->width = CF_LE_W(vspot->width);
	vspot->height = CF_LE_W(vspot->height);
	vspot->frame_rate = CF_LE_L(vspot->frame_rate);
	vspot->bit_rate = CF_LE_L(vspot->bit_rate);
	vspot->compressor = CF_LE_L(vspot->compressor);
	vspot->dpi_horizontal = CF_LE_L(vspot->dpi_horizontal);
	vspot->dpi_vertical = CF_LE_L(vspot->dpi_vertical);
	vspot->depth = CF_LE_W(vspot->depth);
	vspot->flags = CF_LE_W(vspot->flags);
	vspot->reserved1 = CF_LE_L(vspot->reserved1);
	base_object_endian_get(&vspot->obj);
 }
 void vspot_endian_put(video_stream_properties_object_t *vspot)
 {
	vspot->time_scale = CT_LE_L(vspot->time_scale);
	vspot->width = CT_LE_W(vspot->width);
	vspot->height = CT_LE_W(vspot->height);
	vspot->frame_rate = CT_LE_L(vspot->frame_rate);
	vspot->bit_rate = CT_LE_L(vspot->bit_rate);
	vspot->compressor = CT_LE_L(vspot->compressor);
	vspot->dpi_horizontal = CT_LE_L(vspot->dpi_horizontal);
	vspot->dpi_vertical = CT_LE_L(vspot->dpi_vertical);
	vspot->depth = CT_LE_W(vspot->depth);
	vspot->flags = CT_LE_W(vspot->flags);
	vspot->reserved1 = CT_LE_L(vspot->reserved1);
	base_object_endian_put(&vspot->obj);
 }
int ReadAudioStreamPropertiesObject(custommp4_t *file)
{
	int ret;
	ret = custommp4_read_data(file,&file->aspot,sizeof(audio_stream_properties_object_t));
	aspot_endian_get(&file->aspot);
	return ret;
}
 void aspot_endian_get(audio_stream_properties_object_t *aspot)
 {
	aspot->time_scale = CF_LE_L(aspot->time_scale);
	aspot->channels = CF_LE_W(aspot->channels);
	aspot->sample_bits = CF_LE_W(aspot->sample_bits);
	aspot->sample_rate = CF_LE_L(aspot->sample_rate);
	aspot->compressor = CF_LE_L(aspot->compressor);
	aspot->sample_size = CF_LE_L(aspot->sample_size);
	aspot->sample_duration = CF_LE_L(aspot->sample_duration);
	aspot->reserved1 = CF_LE_L(aspot->reserved1);
	base_object_endian_get(&aspot->obj);
 }
 void aspot_endian_put(audio_stream_properties_object_t *aspot)
 {
	aspot->time_scale = CT_LE_L(aspot->time_scale);
	aspot->channels = CT_LE_W(aspot->channels);
	aspot->sample_bits = CT_LE_W(aspot->sample_bits);
	aspot->sample_rate = CT_LE_L(aspot->sample_rate);
	aspot->compressor = CT_LE_L(aspot->compressor);
	aspot->sample_size = CT_LE_L(aspot->sample_size);
	aspot->sample_duration = CT_LE_L(aspot->sample_duration);
	aspot->reserved1 = CT_LE_L(aspot->reserved1);
	base_object_endian_put(&aspot->obj);
 }
int ReadmdatVectorObject(custommp4_t *file)
{
	int ret;
	ret = custommp4_read_data(file,&file->mvot,sizeof(mdat_vector_object_t));
	mvot_endian_get(&file->mvot);
	return ret;
}
 void mvot_endian_get(mdat_vector_object_t *mvot)
 {
	mvot->number_of_segments = CF_LE_L(mvot->number_of_segments);
	mvot->reserved1 = CF_LE_L(mvot->reserved1);
	base_object_endian_get(&mvot->obj);
 }
 void mvot_endian_put(mdat_vector_object_t *mvot)
 {
	mvot->number_of_segments = CT_LE_L(mvot->number_of_segments);
	mvot->reserved1 = CT_LE_L(mvot->reserved1);
	base_object_endian_put(&mvot->obj);
 }
int ReadmdatSegmentObject(custommp4_t *file,int index)
{
	mdat_segment_object_t msot;
	int ret;
	file->p_mdat_sect_t[index].offset = custommp4_position(file);
	memset(&msot,0,sizeof(msot));
	ret = custommp4_read_data(file,&msot,sizeof(mdat_segment_object_t));
	if(ret<=0)
	{
		printf("ReadmdatSegmentObject%d (offset:%d) failed 1\n",index+1,file->p_mdat_sect_t[index].offset);
		return 0;
	}
	msot_endian_get(&msot);
	if(!custommp4_object_is(&msot.obj,CUSTOMMP4_mdat_segment_object))
	{
		printf("ReadmdatSegmentObject%d (offset:%d) failed 2\n",index+1,file->p_mdat_sect_t[index].offset);
		return 0;
	}
	if(msot.obj.object_size<sizeof(mdat_segment_object_t))
	{
		printf("ReadmdatSegmentObject%d (offset:%d) failed 3\n",index+1,file->p_mdat_sect_t[index].offset);
		return 0;
	}
	/*printf("ReadmdatSegmentObject%d (offset:%d,size:%d) success\n",index+1,file->p_mdat_sect_t[index].offset,msot.obj.object_size);
	printf("ReadmdatSegmentObject%d start_time=%d,end_time=%d,video_frames=%d,audio_frames=%d,video_allocated=%d,audio_allocated=%d,sizeof(msot)=%d\n",
		index+1,
		msot.start_time,
		msot.end_time,
		msot.video_frames,
		msot.audio_frames,
		msot.video_allocated,
		msot.audio_allocated,
		sizeof(msot));*/
	file->p_mdat_sect_t[index].start_time = msot.start_time;
	file->p_mdat_sect_t[index].end_time = msot.end_time;
	file->p_mdat_sect_t[index].video_frames = msot.video_frames;
	file->p_mdat_sect_t[index].audio_frames = msot.audio_frames;
	file->p_mdat_sect_t[index].video_allocated = msot.video_allocated;
	file->p_mdat_sect_t[index].audio_allocated = msot.audio_allocated;
	ret = custommp4_set_position(file,file->p_mdat_sect_t[index].offset+msot.obj.object_size);
	if(ret <= 0)
	{
		printf("ReadmdatSegmentObject%d (offset:%d) failed 4\n",index+1,file->p_mdat_sect_t[index].offset);
		return 0;
	}
	file->total_video_frames += msot.video_frames;
	file->total_audio_frames += msot.audio_frames;
	return ret;
}
 void msot_endian_get(mdat_segment_object_t *msot)
 {
	msot->start_time = CF_LE_L(msot->start_time);
	msot->end_time = CF_LE_L(msot->end_time);
	msot->video_frames = CF_LE_L(msot->video_frames);
	msot->audio_frames = CF_LE_L(msot->audio_frames);
	msot->video_allocated = CF_LE_L(msot->video_allocated);
	msot->audio_allocated = CF_LE_L(msot->audio_allocated);
	msot->reserved1 = CF_LE_L(msot->reserved1);
	base_object_endian_get(&msot->obj);
 }
 void msot_endian_put(mdat_segment_object_t *msot)
 {
	msot->start_time = CT_LE_L(msot->start_time);
	msot->end_time = CT_LE_L(msot->end_time);
	msot->video_frames = CT_LE_L(msot->video_frames);
	msot->audio_frames = CT_LE_L(msot->audio_frames);
	msot->video_allocated = CT_LE_L(msot->video_allocated);
	msot->audio_allocated = CT_LE_L(msot->audio_allocated);
	msot->reserved1 = CT_LE_L(msot->reserved1);
	base_object_endian_put(&msot->obj);
 }

int custommp4_get_segment_of_video_frame(u32 *p_sect_offset,u32 *p_sect,custommp4_t *file,int frame)
{
	u32 i;
	u32 frame_count = 0;
	if((u32)frame>=file->total_video_frames)
	{
		return 0;
	}
	for(i=0;i<file->mvot.number_of_segments;i++)
	{
		frame_count += file->p_mdat_sect_t[i].video_frames;
		if(frame_count > (u32)frame)
		{
			break;
		}
	}
	*p_sect = i;
	*p_sect_offset = frame-(frame_count-file->p_mdat_sect_t[i].video_frames);
	return 1;
}

int custommp4_get_segment_of_audio_frame(u32 *p_sect_offset,u32 *p_sect,custommp4_t *file,int frame)
{
	u32 i;
	u32 frame_count = 0;
	if((u32)frame>=file->total_audio_frames)
	{
		return 0;
	}
	for(i=0;i<file->mvot.number_of_segments;i++)
	{
		frame_count += file->p_mdat_sect_t[i].audio_frames;
		if(frame_count > (u32)frame)
		{
			break;
		}
	}
	*p_sect = i;
	*p_sect_offset = frame-(frame_count-file->p_mdat_sect_t[i].audio_frames);
	return 1;
}

int ReadmdatIndexTable(custommp4_t *file,int index)
{
	int ret;
	u32 i;
	SetReadDataFlag(1);
	if(file->video_table_allocated < file->p_mdat_sect_t[index].video_frames)
	{
		file->video_table_allocated = file->p_mdat_sect_t[index].video_frames;
		file->p_video_table_t = realloc(file->p_video_table_t,file->video_table_allocated*sizeof(video_frame_info_table_t));
		if(file->video_table_allocated && file->p_video_table_t == NULL)
		{
			printf("read video index table failed:realloc\n");
			return 0;
		}
	}
	if(file->audio_table_allocated < file->p_mdat_sect_t[index].audio_frames)
	{
		file->audio_table_allocated = file->p_mdat_sect_t[index].audio_frames;
		file->p_audio_table_t = realloc(file->p_audio_table_t,file->audio_table_allocated*sizeof(audio_frame_info_table_t));
		if(file->audio_table_allocated && file->p_audio_table_t == NULL)
		{
			printf("read audio index table failed:realloc\n");
			return 0;
		}
	}
	if(file->p_mdat_sect_t[index].video_frames)
	{
		custommp4_set_position(file,file->p_mdat_sect_t[index].offset+sizeof(mdat_segment_object_t));
		ret = custommp4_read_data(file,file->p_video_table_t,file->p_mdat_sect_t[index].video_frames*sizeof(video_frame_info_table_t));
		if(ret<=0)
		{
			printf("read video index table failed:read\n");
			return 0;
		}
		//csp modify:相对偏移
		for(i=0;i<file->p_mdat_sect_t[index].video_frames;i++)
		{
			vframe_endian_get(&file->p_video_table_t[i]);
			file->p_video_table_t[i].offset += file->open_offset;
		}
	}
	if(file->p_mdat_sect_t[index].audio_frames)
	{
		custommp4_set_position(file,file->p_mdat_sect_t[index].offset+sizeof(mdat_segment_object_t)+file->p_mdat_sect_t[index].video_allocated*sizeof(video_frame_info_table_t));
		ret = custommp4_read_data(file,file->p_audio_table_t,file->p_mdat_sect_t[index].audio_frames*sizeof(audio_frame_info_table_t));
		if(ret<=0)
		{
			printf("read audio index table failed:read\n");
			return 0;
		}
		//csp modify:相对偏移
		for(i=0;i<file->p_mdat_sect_t[index].audio_frames;i++)
		{
			aframe_endian_get(&file->p_audio_table_t[i]);
			file->p_audio_table_t[i].offset += file->open_offset;
		}
	}
	file->current_video_sect_pos = index;
	file->current_audio_sect_pos = index;

	file->current_media_sect_pos = index;
	file->current_video_offset = 0;
	file->current_audio_offset = 0;

	return 1;
}

 void vframe_endian_get(video_frame_info_table_t *pvframe)
 {
	pvframe->offset = CF_LE_L(pvframe->offset);
	pvframe->length = CF_LE_L(pvframe->length);
	pvframe->timestamp = CF_LE_L(pvframe->timestamp);
	pvframe->reserved2 = CF_LE_W(pvframe->reserved2);
 }
 void vframe_endian_put(video_frame_info_table_t *pvframe)
 {
	pvframe->offset = CT_LE_L(pvframe->offset);
	pvframe->length = CT_LE_L(pvframe->length);
	pvframe->timestamp = CT_LE_L(pvframe->timestamp);
	pvframe->reserved2 = CT_LE_W(pvframe->reserved2);
 }
 void aframe_endian_get(audio_frame_info_table_t *paframe)
 {
	paframe->offset = CF_LE_L(paframe->offset);
	paframe->timestamp = CF_LE_L(paframe->timestamp);
	paframe->reserved = CF_LE_L(paframe->reserved);
 }
 void aframe_endian_put(audio_frame_info_table_t *paframe)
 {
	paframe->offset = CT_LE_L(paframe->offset);
	paframe->timestamp = CT_LE_L(paframe->timestamp);
	paframe->reserved = CT_LE_L(paframe->reserved);
 }
int ReadmdatVideoIndexTable(custommp4_t *file,int index)
{
	int ret;
	u32 i;
	if(file->video_table_allocated < file->p_mdat_sect_t[index].video_frames)
	{
		file->video_table_allocated = file->p_mdat_sect_t[index].video_frames;
		file->p_video_table_t = realloc(file->p_video_table_t,file->video_table_allocated*sizeof(video_frame_info_table_t));
		if(file->video_table_allocated && file->p_video_table_t == NULL)
		{
			printf("read video index table failed 1\n");
			return 0;
		}
	}
	if(file->p_mdat_sect_t[index].video_frames)
	{
		custommp4_set_position(file,file->p_mdat_sect_t[index].offset+sizeof(mdat_segment_object_t));
		ret = custommp4_read_data(file,file->p_video_table_t,file->p_mdat_sect_t[index].video_frames*sizeof(video_frame_info_table_t));
		if(ret<=0)
		{
			printf("read video index table failed 2\n");
			return 0;
		}
		//csp modify:相对偏移
		for(i=0;i<file->p_mdat_sect_t[index].video_frames;i++)
		{
			vframe_endian_get(&file->p_video_table_t[i]);
			file->p_video_table_t[i].offset += file->open_offset;
		}
	}
	file->current_video_sect_pos = index;
	return 1;
}

int ReadmdatAudioIndexTable(custommp4_t *file,int index)
{
	int ret;
	u32 i;
	if(file->audio_table_allocated < file->p_mdat_sect_t[index].audio_frames)
	{
		file->audio_table_allocated = file->p_mdat_sect_t[index].audio_frames;
		file->p_audio_table_t = realloc(file->p_audio_table_t,file->audio_table_allocated*sizeof(audio_frame_info_table_t));
		if(file->audio_table_allocated && file->p_audio_table_t == NULL)
		{
			printf("read audio index table failed 1\n");
			return 0;
		}
	}
	if(file->p_mdat_sect_t[index].audio_frames)
	{
		custommp4_set_position(file,file->p_mdat_sect_t[index].offset+sizeof(mdat_segment_object_t)+file->p_mdat_sect_t[index].video_allocated*sizeof(video_frame_info_table_t));
		ret = custommp4_read_data(file,file->p_audio_table_t,file->p_mdat_sect_t[index].audio_frames*sizeof(audio_frame_info_table_t));
		if(ret<=0)
		{
			printf("read audio index table failed 2\n");
			return 0;
		}
		//csp modify:相对偏移
		for(i=0;i<file->p_mdat_sect_t[index].audio_frames;i++)
		{
			aframe_endian_get(&file->p_audio_table_t[i]);
			file->p_audio_table_t[i].offset += file->open_offset;
		}
	}
	file->current_audio_sect_pos = index;
	return 1;
}

custommp4_t* custommp4_open(char *filename,u8 open_mode,u32 open_offset)
{
	custommp4_t *file;
	char flags[10];
	//u32 sect_offset,sect;
	
	if(open_mode == O_R)
	{
		sprintf(flags,"rb");
	}
	else if(open_mode == O_W_CREAT)
	{
		open_offset = 0;
		sprintf(flags,"wb");
	}
	else if(open_mode == O_W_APPEND)
	{
		sprintf(flags,"rb+");
	}
	else if(open_mode == O_W_AUTO)
	{
		FILE *fp = fopen(filename,"rb+");
		if(fp != NULL)
		{
			fclose(fp);
			sprintf(flags,"rb+");
		}
		else
		{
			open_offset = 0;
			sprintf(flags,"wb");
		}
	}
	else
	{
		printf("open mode error\n");
		return NULL;
	}
	
	file = (custommp4_t *)malloc(sizeof(custommp4_t));
	if(file == NULL) return NULL;
	
	custommp4_init(file);
	
	file->stream = fopen(filename,flags);
	if(file->stream == NULL)
	{
		printf("open file failed\n");
		custommp4_destroy(file);
		return NULL;
	}
	file->open_mode = open_mode;
	file->open_offset = open_offset;
	file->file_position = open_offset;
	fseek(file->stream,open_offset,SEEK_SET);
	if(open_mode == O_R)
	{
		int ret;
		u32 i;
		ret = ReadFilePropertiesObject(file);
		if(ret<=0)
		{
			printf("ReadFilePropertiesObject failed 1\n");
			custommp4_destroy(file);
			return NULL;
		}
		if(!custommp4_object_is(&file->fpot.obj,CUSTOMMP4_file_properties_object))
		{
			printf("ReadFilePropertiesObject failed 2\n");
			custommp4_destroy(file);
			return NULL;
		}
		if(file->fpot.obj.object_size != sizeof(file->fpot))
		{
			printf("ReadFilePropertiesObject failed 3\n");
			custommp4_destroy(file);
			return NULL;
		}

		ret = ReadVideoStreamPropertiesObject(file);
		if(ret<=0 || !custommp4_object_is(&file->vspot.obj,CUSTOMMP4_video_stream_properties_object) || file->vspot.obj.object_size != sizeof(file->vspot))
		{
			printf("ReadVideoStreamPropertiesObject failed\n");
			custommp4_destroy(file);
			return NULL;
		}
		ret = ReadAudioStreamPropertiesObject(file);
		if(ret<=0 || !custommp4_object_is(&file->aspot.obj,CUSTOMMP4_audio_stream_properties_object) || file->aspot.obj.object_size != sizeof(file->aspot))
		{
			printf("ReadAudioStreamPropertiesObject failed\n");
			custommp4_destroy(file);
			return NULL;
		}
		ret = ReadmdatVectorObject(file);
		if(ret<=0 || !custommp4_object_is(&file->mvot.obj,CUSTOMMP4_mdat_vector_object))
		{
			printf("ReadAudioStreamPropertiesObject failed\n");
			custommp4_destroy(file);
			return NULL;
		}
		//printf("file has %d segment%s\n",file->mvot.number_of_segments,(file->mvot.number_of_segments>1)?"s":"");
		/*if(file->mvot.number_of_segments == 0)
		{
			file->mvot.number_of_segments = 1;
			printf("change:file has %d segment%s\n",file->mvot.number_of_segments,(file->mvot.number_of_segments>1)?"s":"");
		}*/
		if(file->mvot.number_of_segments)
		{
			file->p_mdat_sect_t = malloc(file->mvot.number_of_segments*sizeof(mdat_segment_info_table_t));
			if(file->p_mdat_sect_t == NULL)
			{
				printf("alloc memory for sect table failed\n");
				custommp4_destroy(file);
				return NULL;
			}
			for(i=0;i<file->mvot.number_of_segments;i++)
			{
				if(ReadmdatSegmentObject(file,i)<=0)
				{
					#ifndef WIN32
					custommp4_destroy(file);
					return NULL;
					#else
					if(i == 0)
					{
						custommp4_destroy(file);
						return NULL;
					}
					else
					{
						break;
					}
					#endif
				}
			}
			#ifdef WIN32
			if(i != file->mvot.number_of_segments)
			{
				file->mvot.number_of_segments = i;
			}
			#endif
			if(file->total_video_frames)
			{
				file->video_table_allocated = file->total_video_frames<3000?file->total_video_frames:3000;
				file->p_video_table_t = malloc(file->video_table_allocated*sizeof(video_frame_info_table_t));
				if(file->video_table_allocated && file->p_video_table_t == NULL)
				{
					printf("alloc memory for video table failed\n");
					custommp4_destroy(file);
					return NULL;
				}
				/*if(custommp4_get_segment_of_video_frame(&sect_offset,&sect,file,0))
				{
					if(ReadmdatVideoIndexTable(file,sect)<=0)
					{
						custommp4_destroy(file);
						return NULL;
					}
					file->current_video_frame_pos = 0;
				}*/
			}
			if(file->total_audio_frames)
			{
				file->audio_table_allocated = file->total_audio_frames<4000?file->total_audio_frames:4000;
				file->p_audio_table_t = malloc(file->audio_table_allocated*sizeof(audio_frame_info_table_t));
				if(file->audio_table_allocated && file->p_audio_table_t == NULL)
				{
					printf("alloc memory for audio table failed\n");
					custommp4_destroy(file);
					return NULL;
				}
				
				/*if(custommp4_get_segment_of_audio_frame(&sect_offset,&sect,file,0))
				{
					if(ReadmdatAudioIndexTable(file,sect)<=0)
					{
						custommp4_destroy(file);
						return NULL;
					}
					file->currenr_audio_frame_pos = 0;
				}*/
			}
			if(ReadmdatIndexTable(file,0)<=0)
			{
				custommp4_destroy(file);
				return NULL;
			}
			
			//printf("custommp4_open:hehe,file_Size=%d,start_time=%d,end_time=%d,videos=%d,audios=%d\n",file->fpot.file_Size,file->fpot.start_time,file->fpot.end_time,file->total_video_frames,file->total_audio_frames);

			//printf("custommp4_open:file_Size=%d\n",file->fpot.file_Size);
			
			if(file->fpot.end_time == file->fpot.start_time)
			{
				if(file->mvot.number_of_segments)
				{
					file->fpot.end_time = file->fpot.start_time+(file->p_mdat_sect_t[file->mvot.number_of_segments-1].end_time-file->p_mdat_sect_t[0].start_time)/1000;
				}
				/*else
				{
					file->fpot.end_time = file->fpot.start_time;
				}*/
			}
			//printf("custommp4_open:file end time=%d\n",file->fpot.end_time);
		}
	}
	else
	{
		mdat_segment_object_t msot;
		file_properties_object_t tmp_fpot;
		video_stream_properties_object_t tmp_vspot;
		audio_stream_properties_object_t tmp_aspot;
		mdat_vector_object_t tmp_mvot;

		custommp4_file_properties_object_init(&file->fpot);
		custommp4_video_stream_properties_object_init(&file->vspot);
		custommp4_audio_stream_properties_object_init(&file->aspot);
		custommp4_mdat_vector_object_init(&file->mvot);
		tmp_fpot = file->fpot;
		fpot_endian_put(&tmp_fpot);
		custommp4_write_data(file, &tmp_fpot, sizeof(file->fpot));
		tmp_vspot = file->vspot;
		vspot_endian_put(&tmp_vspot);
		custommp4_write_data(file, &tmp_vspot, sizeof(file->vspot));
		tmp_aspot = file->aspot;
		aspot_endian_put(&tmp_aspot);
		custommp4_write_data(file, &tmp_aspot, sizeof(file->aspot));
		tmp_mvot = file->mvot;
		mvot_endian_put(&tmp_mvot);
		custommp4_write_data(file, &tmp_mvot, sizeof(file->mvot));
		
		file->p_mdat_sect_t = (mdat_segment_info_table_t *)malloc(sizeof(mdat_segment_info_table_t));
		custommp4_mdat_segment_info_table_init(file,file->p_mdat_sect_t);
		msot.obj.object_id = CUSTOMMP4_mdat_segment_object;
		msot.obj.object_size = sizeof(msot);
		msot.video_frames = 0;
		msot.audio_frames = 0;
		msot.reserved1 = 0;
		msot.start_time = file->p_mdat_sect_t->start_time;
		msot.end_time = file->p_mdat_sect_t->end_time;
		msot.video_allocated = file->p_mdat_sect_t->video_allocated;
		msot.audio_allocated = file->p_mdat_sect_t->audio_allocated;
		msot_endian_put(&msot);
		custommp4_write_data(file,&msot,sizeof(msot));
		
		file->video_table_allocated = 3000;
		file->audio_table_allocated = 4000;

		file->p_video_table_t = (video_frame_info_table_t *)malloc(sizeof(video_frame_info_table_t));
		memset(file->p_video_table_t,0,sizeof(video_frame_info_table_t));
		file->p_audio_table_t = (audio_frame_info_table_t *)malloc(sizeof(audio_frame_info_table_t));
		memset(file->p_audio_table_t,0,sizeof(audio_frame_info_table_t));
	}
	return file;
}

int custommp4_close(custommp4_t *file)
{
	if(file->open_mode != O_R)
	{
		if(file->p_mdat_sect_t->video_frames || file->p_mdat_sect_t->audio_frames)
		{
			u32 last_pos;
			mdat_segment_object_t msot;
			mdat_vector_object_t tmp_mvot;
			file_properties_object_t tmp_fpot;
			
			last_pos = custommp4_position(file);
			
			custommp4_set_position(file,file->p_mdat_sect_t->offset);
			msot.obj.object_id = CUSTOMMP4_mdat_segment_object;
			msot.obj.object_size = last_pos-file->p_mdat_sect_t->offset;
			msot.video_frames = file->p_mdat_sect_t->video_frames;
			msot.audio_frames = file->p_mdat_sect_t->audio_frames;
			msot.reserved1 = 0;
			msot.start_time = file->p_mdat_sect_t->start_time;
			msot.end_time = file->p_mdat_sect_t->end_time;
			msot.video_allocated = file->p_mdat_sect_t->video_allocated;
			msot.audio_allocated = file->p_mdat_sect_t->audio_allocated;
			msot_endian_put(&msot);
			custommp4_write_data(file,&msot,sizeof(msot));
			
			custommp4_set_position(file,file->open_offset+sizeof(file->fpot)+sizeof(file->vspot)+sizeof(file->aspot));
			file->mvot.number_of_segments++;
			tmp_mvot = file->mvot;
			mvot_endian_put(&tmp_mvot);
			custommp4_write_data(file, &tmp_mvot, sizeof(file->mvot));
			
			custommp4_set_position(file,file->open_offset);
			file->fpot.end_time = time(NULL);
			file->fpot.file_Size = last_pos - file->open_offset;
			//printf("custommp4_close:file_Size=%d\n",file->fpot.file_Size);
			tmp_fpot = file->fpot;
			fpot_endian_put(&tmp_fpot);
			custommp4_write_data(file, &tmp_fpot, sizeof(file->fpot));
			fileflush(file->stream);//2007-09-25 csp add
		}
	}
	custommp4_destroy(file);
	return 1;
}

int custommp4_set_video(custommp4_t *file,u32 time_scale,u16 width,u16 height,float frame_rate,u32 bit_rate,u32 compressor,u16 depth)
{
	file_properties_object_t tmp_fpot;
	video_stream_properties_object_t tmp_vspot;
	u32 last_pos = custommp4_position(file);
	file->vspot.time_scale = time_scale;
	file->vspot.width = width;
	file->vspot.height = height;
	file->vspot.frame_rate = frame_rate;
	file->vspot.bit_rate = bit_rate;
	file->vspot.compressor = compressor;
	file->vspot.depth = depth;
	file->fpot.video_tracks = 1;
	custommp4_set_position(file,file->open_offset);
	tmp_fpot = file->fpot;
	fpot_endian_put(&tmp_fpot);
	custommp4_write_data(file,&tmp_fpot,sizeof(file->fpot));
	tmp_vspot = file->vspot;
	vspot_endian_put(&tmp_vspot);
	custommp4_write_data(file, &tmp_vspot, sizeof(file->vspot));
	custommp4_set_position(file,last_pos);
	if(file->p_mdat_sect_t->video_frames == 0 && file->p_mdat_sect_t->audio_frames == 0) file->p_mdat_sect_t->video_allocated = 3000;
	return 1;
}

int custommp4_set_audio(custommp4_t *file,u32 time_scale,u16 channels,u16 bits,u32 sample_rate,u32 compressor,u32 sample_size,u32 sample_duration)
{
	file_properties_object_t tmp_fpot;
	audio_stream_properties_object_t tmp_aspot;
	u32 last_pos = custommp4_position(file);
	file->aspot.time_scale = time_scale;
	file->aspot.channels = channels;
	file->aspot.sample_bits = bits;
	file->aspot.sample_rate = sample_rate;
	file->aspot.compressor = compressor;
	file->aspot.sample_size = sample_size;
	file->aspot.sample_duration = sample_duration;
	file->fpot.audio_tracks = 1;
	custommp4_set_position(file,file->open_offset);
	tmp_fpot = file->fpot;
	fpot_endian_put(&tmp_fpot);
	custommp4_write_data(file, &tmp_fpot, sizeof(file->fpot));
	custommp4_set_position(file,file->open_offset+sizeof(file->fpot)+sizeof(file->vspot));
	tmp_aspot = file->aspot;
	aspot_endian_put(&tmp_aspot);
	custommp4_write_data(file, &tmp_aspot, sizeof(file->aspot));
	custommp4_set_position(file,last_pos);
	if(file->p_mdat_sect_t->video_frames == 0 && file->p_mdat_sect_t->audio_frames == 0) file->p_mdat_sect_t->audio_allocated = 4000;
	return 1;
}

BOOL custommp4_has_video(custommp4_t *file)
{
	return (file->total_video_frames!=0);
	//return file->fpot.video_tracks;
}

BOOL custommp4_has_audio(custommp4_t *file)
{
	return (file->total_audio_frames!=0);
	//return file->fpot.audio_tracks;
}

int custommp4_total_time(custommp4_t *file)
{
	int total_time = 0;
	if(file->mvot.number_of_segments)
	{
		total_time = file->p_mdat_sect_t[file->mvot.number_of_segments-1].end_time-file->p_mdat_sect_t[0].start_time;
	}
	return total_time;
}

int custommp4_video_length(custommp4_t *file)
{
	return file->total_video_frames;
}

int custommp4_audio_length(custommp4_t *file)
{
	return file->total_audio_frames;
}

u16 custommp4_video_width(custommp4_t *file)
{
	return file->vspot.width;
}

u16 custommp4_video_height(custommp4_t *file)
{
	return file->vspot.height;
}

u16 custommp4_video_depth(custommp4_t *file)
{
	return file->vspot.depth;
}

float custommp4_video_frame_rate(custommp4_t *file)
{
	return file->vspot.frame_rate;
}

u32 custommp4_video_bit_rate(custommp4_t *file)
{
	return file->vspot.bit_rate;
}

u32 custommp4_video_compressor(custommp4_t *file)
{
	return file->vspot.compressor;
}

u32 custommp4_video_time_scale(custommp4_t *file)
{
	return file->vspot.time_scale;
}

u16 custommp4_audio_channels(custommp4_t *file)
{
	return file->aspot.channels;
}

u16 custommp4_audio_bits(custommp4_t *file)
{
	return file->aspot.sample_bits;
}

u32 custommp4_audio_sample_rate(custommp4_t *file)
{
	return file->aspot.sample_rate;
}

u32 custommp4_audio_sample_size(custommp4_t *file)
{
	if(file->total_audio_frames)
	{
		return file->aspot.sample_size;
	}
	return 0;
}

u32 custommp4_audio_compressor(custommp4_t *file)
{
	return file->aspot.compressor;
}

u32 custommp4_audio_time_scale(custommp4_t *file)
{
	return file->aspot.time_scale;
}

int custommp4_read_one_media_frame(custommp4_t *file,u8 *meida_buffer,u32 maxBytes,u32 *start_time,u8 *key,u8 *media_type)
{
	int len = 0;
	if(file->current_media_sect_pos >= file->mvot.number_of_segments)
	{
		return 0;
	}
	if(file->current_video_offset >= file->p_mdat_sect_t[file->current_media_sect_pos].video_frames && file->current_audio_offset >= file->p_mdat_sect_t[file->current_media_sect_pos].audio_frames)
	{
		file->current_media_sect_pos++;
		if(file->current_media_sect_pos >= file->mvot.number_of_segments)
		{
			return 0;
		}
		if(ReadmdatIndexTable(file,file->current_media_sect_pos) <= 0)
		{
			printf("custommp4_read_one_media_frame:ReadmdatIndexTable error\n");
			return -1;
		}
	}
	if(file->current_video_offset >= file->p_mdat_sect_t[file->current_media_sect_pos].video_frames && file->current_audio_offset >= file->p_mdat_sect_t[file->current_media_sect_pos].audio_frames)
	{
		return 0;
	}
	if(file->current_video_offset >= file->p_mdat_sect_t[file->current_media_sect_pos].video_frames)
	{
		if(file->p_audio_table_t[file->current_audio_offset].offset > (u32)SIZE_OF_FILE_VECTOR)
		{
			printf("warning:offset error 1,sample offset:%d,sample length:%d,buffer length:%d\n",file->p_audio_table_t[file->current_audio_offset].offset,file->aspot.sample_size,maxBytes);
			return -1;
		}
		if(file->aspot.sample_size > maxBytes)
		{
			printf("warning:size error 1,sample offset:%d,sample length:%d,buffer length:%d\n",file->p_audio_table_t[file->current_audio_offset].offset,file->aspot.sample_size,maxBytes);
			return -1;
		}
		SetReadDataFlag(3);
		custommp4_set_position(file,file->p_audio_table_t[file->current_audio_offset].offset);
		len = custommp4_read_data(file, meida_buffer, file->aspot.sample_size);
		*media_type = 1;
		*key = 0;
		*start_time = file->p_audio_table_t[file->current_audio_offset].timestamp;
		//len = file->aspot.sample_size;
		file->current_audio_offset++;
	}
	else if(file->current_audio_offset >= file->p_mdat_sect_t[file->current_media_sect_pos].audio_frames)
	{
		if(file->p_video_table_t[file->current_video_offset].offset > (u32)SIZE_OF_FILE_VECTOR)
		{
			printf("warning:offset error 1,frame offset:%d,frame length:%d,buffer length:%d\n",file->p_video_table_t[file->current_video_offset].offset,file->p_video_table_t[file->current_video_offset].length,maxBytes);
			return -1;
		}
		if(file->p_video_table_t[file->current_video_offset].length > maxBytes)
		{
			printf("warning:size error 1,frame offset:%d,frame length:%d,buffer length:%d\n",file->p_video_table_t[file->current_video_offset].offset,file->p_video_table_t[file->current_video_offset].length,maxBytes);
			return -1;
		}
		SetReadDataFlag(2);
		memset(meida_buffer,0xff,4);
		custommp4_set_position(file,file->p_video_table_t[file->current_video_offset].offset);
		len = custommp4_read_data(file, meida_buffer, file->p_video_table_t[file->current_video_offset].length);
		*media_type = 0;
		*key = file->p_video_table_t[file->current_video_offset].key;
		*start_time = file->p_video_table_t[file->current_video_offset].timestamp;
		//len = file->p_video_table_t[file->current_video_offset].length;
		file->current_video_offset++;
#if 0
		if(len > 0)
		{
			if(*key == 0)
			{
				if(!(meida_buffer[0] == 0x00 && 
					meida_buffer[1] == 0x00 && 
					meida_buffer[2] == 0x01 && 
					meida_buffer[3] == 0xb6))
				{
					printf("P frame error 1\n");
					return -1;
				}
			}
			else if(*key == 1)
			{
				if(!(meida_buffer[0] == 0x00 && 
					meida_buffer[1] == 0x00 && 
					meida_buffer[2] == 0x01 && 
					meida_buffer[3] == 0xb0))
				{
					printf("I frame error 1\n");
					return -1;
				}
			}
			else
			{
				printf("video frame type error 1\n");
				return -1;
			}
		}
#endif
	}
	else
	{
		if(file->p_video_table_t[file->current_video_offset].timestamp > file->p_audio_table_t[file->current_audio_offset].timestamp)
		{
			if(file->p_audio_table_t[file->current_audio_offset].offset > (u32)SIZE_OF_FILE_VECTOR)
			{
				printf("warning:offset error 2,sample offset:%d,sample length:%d,buffer length:%d\n",file->p_audio_table_t[file->current_audio_offset].offset,file->aspot.sample_size,maxBytes);
				return -1;
			}
			if(file->aspot.sample_size > maxBytes)
			{
				printf("warning:size error 2,sample offset:%d,sample length:%d,buffer length:%d\n",file->p_audio_table_t[file->current_audio_offset].offset,file->aspot.sample_size,maxBytes);
				return -1;
			}
			SetReadDataFlag(3);
			custommp4_set_position(file,file->p_audio_table_t[file->current_audio_offset].offset);
			len = custommp4_read_data(file, meida_buffer, file->aspot.sample_size);
			*media_type = 1;
			*key = 0;
			*start_time = file->p_audio_table_t[file->current_audio_offset].timestamp;
			file->current_audio_offset++;
		}
		else
		{
			if(file->p_video_table_t[file->current_video_offset].offset > (u32)SIZE_OF_FILE_VECTOR)
			{
				printf("warning:offset error 2,frame offset:%d,frame length:%d,buffer length:%d\n",file->p_video_table_t[file->current_video_offset].offset,file->p_video_table_t[file->current_video_offset].length,maxBytes);
				return -1;
			}
			if(file->p_video_table_t[file->current_video_offset].length > maxBytes)
			{
				printf("warning:size error 2,frame offset:%d,frame length:%d,buffer length:%d\n",file->p_video_table_t[file->current_video_offset].offset,file->p_video_table_t[file->current_video_offset].length,maxBytes);
				return -1;
			}
			SetReadDataFlag(2);
			memset(meida_buffer,0xff,4);
			custommp4_set_position(file,file->p_video_table_t[file->current_video_offset].offset);
			len = custommp4_read_data(file, meida_buffer, file->p_video_table_t[file->current_video_offset].length);
			*media_type = 0;
			*key = file->p_video_table_t[file->current_video_offset].key;
			*start_time = file->p_video_table_t[file->current_video_offset].timestamp;
			file->current_video_offset++;
#if 0
		if(len > 0)
			{
				if(*key == 0)
				{
					if(!(meida_buffer[0] == 0x00 && 
						meida_buffer[1] == 0x00 && 
						meida_buffer[2] == 0x01 && 
						meida_buffer[3] == 0xb6))
					{
						printf("P frame error 2\n");
						return -1;
					}
				}
				else if(*key == 1)
				{
					if(!(meida_buffer[0] == 0x00 && 
						meida_buffer[1] == 0x00 && 
						meida_buffer[2] == 0x01 && 
						meida_buffer[3] == 0xb0))
					{
						printf("I frame error 2\n");
						return -1;
					}
				}
				else
				{
					printf("video frame type error 2\n");
					return -1;
				}	
			}
#endif
		}
	}
	return len;
}
#if 0
int custommp4_read_one_media_h264v_frame(custommp4_t *file,u8 *meida_buffer,u32 maxBytes,u32 *start_time,u8 *key,u8 *media_type,u16 *nal_size)
{
	int len = 0;
	if(file->current_media_sect_pos >= file->mvot.number_of_segments)
	{
		return 0;
	}
	if(file->current_video_offset >= file->p_mdat_sect_t[file->current_media_sect_pos].video_frames && file->current_audio_offset >= file->p_mdat_sect_t[file->current_media_sect_pos].audio_frames)
	{
		file->current_media_sect_pos++;
		if(file->current_media_sect_pos >= file->mvot.number_of_segments)
		{
			return 0;
		}
		if(ReadmdatIndexTable(file,file->current_media_sect_pos) <= 0)
		{
			printf("custommp4_read_one_media_frame:ReadmdatIndexTable error\n");
			return -1;
		}
	}
	if(file->current_video_offset >= file->p_mdat_sect_t[file->current_media_sect_pos].video_frames && file->current_audio_offset >= file->p_mdat_sect_t[file->current_media_sect_pos].audio_frames)
	{
		return 0;
	}
	if(file->current_video_offset >= file->p_mdat_sect_t[file->current_media_sect_pos].video_frames)
	{
		if(file->p_audio_table_t[file->current_audio_offset].offset > (u32)SIZE_OF_FILE_VECTOR)
		{
			printf("warning:offset error 1,sample offset:%d,sample length:%d,buffer length:%d\n",file->p_audio_table_t[file->current_audio_offset].offset,file->aspot.sample_size,maxBytes);
			return -1;
		}
		if(file->aspot.sample_size > maxBytes)
		{
			printf("warning:size error 1,sample offset:%d,sample length:%d,buffer length:%d\n",file->p_audio_table_t[file->current_audio_offset].offset,file->aspot.sample_size,maxBytes);
			return -1;
		}
		SetReadDataFlag(3);
		custommp4_set_position(file,file->p_audio_table_t[file->current_audio_offset].offset);
		len = custommp4_read_data(file, meida_buffer, file->aspot.sample_size);
		*media_type = 1;
		*key = 0;
		*start_time = file->p_audio_table_t[file->current_audio_offset].timestamp;
		//len = file->aspot.sample_size;
		file->current_audio_offset++;
	}
	else if(file->current_audio_offset >= file->p_mdat_sect_t[file->current_media_sect_pos].audio_frames)
	{
		if(file->p_video_table_t[file->current_video_offset].offset > (u32)SIZE_OF_FILE_VECTOR)
		{
			printf("warning:offset error 1,frame offset:%d,frame length:%d,buffer length:%d\n",file->p_video_table_t[file->current_video_offset].offset,file->p_video_table_t[file->current_video_offset].length,maxBytes);
			return -1;
		}
		if(file->p_video_table_t[file->current_video_offset].length > maxBytes)
		{
			printf("warning:size error 1,frame offset:%d,frame length:%d,buffer length:%d\n",file->p_video_table_t[file->current_video_offset].offset,file->p_video_table_t[file->current_video_offset].length,maxBytes);
			return -1;
		}
		SetReadDataFlag(2);
		memset(meida_buffer,0xff,4);
		custommp4_set_position(file,file->p_video_table_t[file->current_video_offset].offset);
		len = custommp4_read_data(file, meida_buffer, file->p_video_table_t[file->current_video_offset].length);
		*media_type = 0;
		*key = file->p_video_table_t[file->current_video_offset].key;
		*start_time = file->p_video_table_t[file->current_video_offset].timestamp;
		*nal_size = file->p_video_table_t[file->current_video_offset].reserved2;
		file->current_video_offset++;
	}
	else
	{
		if(file->p_video_table_t[file->current_video_offset].timestamp > file->p_audio_table_t[file->current_audio_offset].timestamp)
		{
			if(file->p_audio_table_t[file->current_audio_offset].offset > (u32)SIZE_OF_FILE_VECTOR)
			{
				printf("warning:offset error 2,sample offset:%d,sample length:%d,buffer length:%d\n",file->p_audio_table_t[file->current_audio_offset].offset,file->aspot.sample_size,maxBytes);
				return -1;
			}
			if(file->aspot.sample_size > maxBytes)
			{
				printf("warning:size error 2,sample offset:%d,sample length:%d,buffer length:%d\n",file->p_audio_table_t[file->current_audio_offset].offset,file->aspot.sample_size,maxBytes);
				return -1;
			}
			SetReadDataFlag(3);
			custommp4_set_position(file,file->p_audio_table_t[file->current_audio_offset].offset);
			len = custommp4_read_data(file, meida_buffer, file->aspot.sample_size);
			*media_type = 1;
			*key = 0;
			*start_time = file->p_audio_table_t[file->current_audio_offset].timestamp;
			file->current_audio_offset++;
		}
		else
		{
			if(file->p_video_table_t[file->current_video_offset].offset > (u32)SIZE_OF_FILE_VECTOR)
			{
				printf("warning:offset error 2,frame offset:%d,frame length:%d,buffer length:%d\n",file->p_video_table_t[file->current_video_offset].offset,file->p_video_table_t[file->current_video_offset].length,maxBytes);
				return -1;
			}
			if(file->p_video_table_t[file->current_video_offset].length > maxBytes)
			{
				printf("warning:size error 2,frame offset:%d,frame length:%d,buffer length:%d\n",file->p_video_table_t[file->current_video_offset].offset,file->p_video_table_t[file->current_video_offset].length,maxBytes);
				return -1;
			}
			SetReadDataFlag(2);
			memset(meida_buffer,0xff,4);
			custommp4_set_position(file,file->p_video_table_t[file->current_video_offset].offset);
			len = custommp4_read_data(file, meida_buffer, file->p_video_table_t[file->current_video_offset].length);
			*media_type = 0;
			*key = file->p_video_table_t[file->current_video_offset].key;
			*start_time = file->p_video_table_t[file->current_video_offset].timestamp;
			*nal_size = file->p_video_table_t[file->current_video_offset].reserved2;
			//len = file->p_video_table_t[file->current_video_offset].length;
			file->current_video_offset++;
		}
	}
	return len;
}
#endif

int custommp4_video_frame_size(custommp4_t *file,int frame)
{
	u32 sect_offset,sect;
	int len = 0;
	if(custommp4_get_segment_of_video_frame(&sect_offset,&sect,file,frame))
	{
		lock_custommp4(file);

		if(file->current_video_sect_pos != sect)
		{
			ReadmdatVideoIndexTable(file,sect);
		}
		file->current_video_frame_pos = frame;

		len = file->p_video_table_t[sect_offset].length;

		unlock_custommp4(file);
	}
	return len;
}

int custommp4_audio_frame_size(custommp4_t *file,int frame)
{
	u32 sect_offset,sect;
	int len = 0;
	if(custommp4_get_segment_of_audio_frame(&sect_offset,&sect,file,frame))
	{
		lock_custommp4(file);

		if(file->current_audio_sect_pos != sect)
		{
			ReadmdatAudioIndexTable(file,sect);
		}
		file->current_audio_frame_pos = frame;

		len = file->aspot.sample_size;

		unlock_custommp4(file);
	}
	return len;
}

int custommp4_read_video_frame(custommp4_t *file,u8 *video_buffer,u32 maxBytes,int frame,u32 *start_time,u8 *key)
{
	u32 sect_offset,sect;
	int len = 0;
	if(custommp4_get_segment_of_video_frame(&sect_offset,&sect,file,frame))
	{
		lock_custommp4(file);

		//printf("frame=%d,sect=%d,offset=%d,videos=%d,audios=%d\n",frame,sect,sect_offset,file->p_mdat_sect_t[sect].video_frames,file->p_mdat_sect_t[sect].audio_frames);
		
		if(file->current_video_sect_pos != sect)
		{
			ReadmdatVideoIndexTable(file,sect);
		}
		file->current_video_frame_pos = frame;
		custommp4_set_position(file,file->p_video_table_t[sect_offset].offset);
		custommp4_read_data(file,video_buffer,file->p_video_table_t[sect_offset].length);
		
		//printf("offset=%d,len=%d,key=%d,sizeof(video_frame_info_table_t)=%d\n",file->p_video_table_t[sect_offset].offset,file->p_video_table_t[sect_offset].length,file->p_video_table_t[sect_offset].key,sizeof(video_frame_info_table_t));
		
		*start_time = file->p_video_table_t[sect_offset].timestamp;
		*key = file->p_video_table_t[sect_offset].key;
		len = file->p_video_table_t[sect_offset].length;

		unlock_custommp4(file);
	}
	return len;
}

int custommp4_read_audio_frame(custommp4_t *file,u8 *audio_buffer,u32 maxBytes,int frame,u32 *start_time)
{
	u32 sect_offset,sect;
	int len = 0;
	if(custommp4_get_segment_of_audio_frame(&sect_offset,&sect,file,frame))
	{
		lock_custommp4(file);
		
		if(file->current_audio_sect_pos != sect)
		{
			ReadmdatAudioIndexTable(file,sect);
		}
		file->current_audio_frame_pos = frame;
		custommp4_set_position(file,file->p_audio_table_t[sect_offset].offset);
		//printf("read audio:frame%d timestamp=%d,offset=%d\n",frame,file->p_audio_table_t[sect_offset].timestamp,file->p_audio_table_t[sect_offset].offset);
		custommp4_read_data(file,audio_buffer,file->aspot.sample_size);
		
		*start_time = file->p_audio_table_t[sect_offset].timestamp;
		len = file->aspot.sample_size;

		unlock_custommp4(file);
	}
	return len;
}

int custommp4_read_one_video_frame(custommp4_t *file,u8 *video_buffer,u32 maxBytes,u32 *start_time,u32 *duration,u8 *key)
{
	u32 sect_offset,sect;
	int frame;
	int len = 0;

	lock_custommp4(file);

	frame = file->current_video_frame_pos;
	if((u32)frame >= file->total_video_frames)
	{
		unlock_custommp4(file);
		return 0;
	}
	if(custommp4_get_segment_of_video_frame(&sect_offset,&sect,file,frame))
	{
		//printf("frame=%d,sect=%d,offset=%d,videos=%d,audios=%d\n",frame,sect,sect_offset,file->p_mdat_sect_t[sect].video_frames,file->p_mdat_sect_t[sect].audio_frames);
		
		if(file->current_video_sect_pos != sect)
		{
			ReadmdatVideoIndexTable(file,sect);
		}
		custommp4_set_position(file,file->p_video_table_t[sect_offset].offset);
		custommp4_read_data(file,video_buffer,file->p_video_table_t[sect_offset].length);
		
		//printf("offset=%d,len=%d,key=%d,sizeof(video_frame_info_table_t)=%d\n",file->p_video_table_t[sect_offset].offset,file->p_video_table_t[sect_offset].length,file->p_video_table_t[sect_offset].key,sizeof(video_frame_info_table_t));
		
		*start_time = file->p_video_table_t[sect_offset].timestamp;
		*key = file->p_video_table_t[sect_offset].key;
		len = file->p_video_table_t[sect_offset].length;


		if(sect_offset < file->p_mdat_sect_t[sect].video_frames-1)
		{
			*duration = file->p_video_table_t[sect_offset+1].timestamp - *start_time;
			++frame;
		}
		else
		{
			if((u32)frame == file->total_video_frames-1)
			{
				*duration = 40;
				//frame = 0;
				++frame; //add by cj@20090928
			}
			else
			{
				++frame;
				custommp4_get_segment_of_video_frame(&sect_offset,&sect,file,frame);
				ReadmdatVideoIndexTable(file,sect);
				*duration = file->p_video_table_t[0].timestamp - *start_time;
			}
		}
		file->current_video_frame_pos = frame;
	}

	unlock_custommp4(file);

	return len;
}

int custommp4_read_one_audio_frame(custommp4_t *file,u8 *audio_buffer,u32 maxBytes,u32 *start_time,u32 *duration)
{
	u32 sect_offset,sect;
	int frame;
	int len = 0;

	lock_custommp4(file);

	frame = file->current_audio_frame_pos;
	if((u32)frame >= file->total_audio_frames)
	{
		unlock_custommp4(file);
		return 0;
	}
	if(custommp4_get_segment_of_audio_frame(&sect_offset,&sect,file,frame))
	{
		if(file->current_audio_sect_pos != sect)
		{
			ReadmdatAudioIndexTable(file,sect);
		}
		file->current_audio_frame_pos = frame;
		custommp4_set_position(file,file->p_audio_table_t[sect_offset].offset);
		custommp4_read_data(file,audio_buffer,file->aspot.sample_size);
		
		*start_time = file->p_audio_table_t[sect_offset].timestamp;
		len = file->aspot.sample_size;

		if(sect_offset < file->p_mdat_sect_t[sect].audio_frames-1)
		{
			*duration = file->p_audio_table_t[sect_offset+1].timestamp - *start_time;
			++frame;
		}
		else
		{
			if((u32)frame == file->total_audio_frames-1)
			{
				*duration = 30;
				//frame = 0;
			}
			else
			{
				++frame;
				custommp4_get_segment_of_audio_frame(&sect_offset,&sect,file,frame);
				ReadmdatAudioIndexTable(file,sect);
				*duration = file->p_audio_table_t[0].timestamp - *start_time;
			}
		}
		file->current_audio_frame_pos = frame;
	}

	unlock_custommp4(file);

	return len;
}

int custommp4_seek_to_prev_key_frame(custommp4_t *file)
{
	int i,j;
	int cur_video_pos;
	u32 start_time;
	if(file->mvot.number_of_segments == 0)
	{
		//printf("failed 1\n");
		return 0;
	}
	if(file->current_media_sect_pos > file->mvot.number_of_segments)
	{
		file->current_media_sect_pos = file->mvot.number_of_segments;
	}
	if(file->current_media_sect_pos < file->mvot.number_of_segments)
	{
		cur_video_pos = file->current_video_offset-2;
		//printf("cur_video_pos=%d\n",cur_video_pos);
		if(cur_video_pos >= (int)file->p_mdat_sect_t[file->current_media_sect_pos].video_frames)
		{
			cur_video_pos = file->p_mdat_sect_t[file->current_media_sect_pos].video_frames-1;
		}
		for(i=cur_video_pos;i>=0;i--)
		{
			if(file->p_video_table_t[i].key == 1)
			{
				file->current_video_offset = i;
				start_time = file->p_video_table_t[i].timestamp;
				//printf("seek_video_pos=%d\n",i);
				break;
			}
		}
		if(i>=0)
		{
			for(j=0;j<(int)file->p_mdat_sect_t[file->current_media_sect_pos].audio_frames;j++)
			{
				if(file->p_audio_table_t[j].timestamp > start_time)
				{
					break;
				}
			}
			file->current_audio_offset = j;
			return 1;
		}
	}

	while((int)file->current_media_sect_pos > 0)
	{
		ReadmdatIndexTable(file,file->current_media_sect_pos-1);
		//printf("last segment\n");
		cur_video_pos = file->p_mdat_sect_t[file->current_media_sect_pos].video_frames-1;
		for(i=cur_video_pos;i>=0;i--)
		{
			if(file->p_video_table_t[i].key == 1)
			{
				file->current_video_offset = i;
				start_time = file->p_video_table_t[i].timestamp;
				break;
			}
		}
		if(i>=0)
		{
			for(j=0;j<(int)file->p_mdat_sect_t[file->current_media_sect_pos].audio_frames;j++)
			{
				if(file->p_audio_table_t[j].timestamp > start_time)
				{
					break;
				}
			}
			file->current_audio_offset = j;
			return 1;
		}
	}
	//printf("failed 2\n");
	return 0;
}

int custommp4_seek_to_prev_segment(custommp4_t *file)
{
	u32 i,j;
	u32 start_time;
	if(file->mvot.number_of_segments == 0)
	{
		return 0;
	}
	if(file->current_media_sect_pos > file->mvot.number_of_segments)
	{
		file->current_media_sect_pos = file->mvot.number_of_segments;
	}
	while((int)file->current_media_sect_pos > 0)
	{	
		ReadmdatIndexTable(file,file->current_media_sect_pos-1);
		for(i=0;i<file->p_mdat_sect_t[file->current_media_sect_pos].video_frames;i++)
		{
			if(file->p_video_table_t[i].key == 1)
			{
				file->current_video_offset = i;
				start_time = file->p_video_table_t[i].timestamp;
				break;
			}
		}
		if(i<file->p_mdat_sect_t[file->current_media_sect_pos].video_frames)
		{
			for(j=0;j<file->p_mdat_sect_t[file->current_media_sect_pos].audio_frames;j++)
			{
				if(file->p_audio_table_t[j].timestamp > start_time)
				{
					break;
				}
			}
			file->current_audio_offset = j;
			return 1;
		}
	}
	return 0;
}

int custommp4_seek_to_next_segment(custommp4_t *file)
{
	u32 i,j;
	u32 start_time;
	while(file->current_media_sect_pos+1 < file->mvot.number_of_segments)
	{
		ReadmdatIndexTable(file,file->current_media_sect_pos+1);
		for(i=0;i<file->p_mdat_sect_t[file->current_media_sect_pos].video_frames;i++)
		{
			if(file->p_video_table_t[i].key == 1)
			{
				file->current_video_offset = i;
				start_time = file->p_video_table_t[i].timestamp;
				break;
			}
		}
		if(i<file->p_mdat_sect_t[file->current_media_sect_pos].video_frames)
		{
			for(j=0;j<file->p_mdat_sect_t[file->current_media_sect_pos].audio_frames;j++)
			{
				if(file->p_audio_table_t[j].timestamp > start_time)
				{
					break;
				}
			}
			file->current_audio_offset = j;
			return 1;
		}
	}
	return 0;
}

int custommp4_seek_to_time_stamp(custommp4_t *file,u32 timestamp)
{
	u32 i,j,k;
	u32 start_time;
	lock_custommp4(file);//2007-11-18
	for(i=0;i<file->mvot.number_of_segments;i++)
	{
		if(timestamp > file->p_mdat_sect_t[i].end_time) continue;
		
		#ifdef WIN32
		file->current_video_sect_pos = (u32)(-1);
		file->current_audio_sect_pos = (u32)(-1);
		#endif
		#ifndef WIN32//2007-11-18
		if(i != file->current_media_sect_pos)
		#endif//2007-11-18
		{
			if(ReadmdatIndexTable(file,i) <= 0)
			{
				unlock_custommp4(file);//2007-11-18
				printf("custommp4_seek_to_time_stamp:ReadmdatIndexTable error\n");
				return -1;//2007-11-18
			}
		}
		
		for(j=0;j<file->p_mdat_sect_t[file->current_media_sect_pos].video_frames;j++)
		{
			//if(file->p_video_table_t[j].key == 1 && file->p_video_table_t[j].timestamp/*+1500*/ >= timestamp)//2007-11-18
			if(is_i_frame(file->p_video_table_t[j].key) && file->p_video_table_t[j].timestamp+1600 >= timestamp)
			{
				file->current_video_offset = j;
				start_time = file->p_video_table_t[j].timestamp;
				break;
			}
		}
		if(j<file->p_mdat_sect_t[file->current_media_sect_pos].video_frames)
		{
			for(k=0;k<file->p_mdat_sect_t[file->current_media_sect_pos].audio_frames;k++)
			{
				if(file->p_audio_table_t[k].timestamp > start_time)
				{
					break;
				}
			}
			file->current_audio_offset = k;
			#ifdef WIN32
			{
				u32 v_frames = 0;
				u32 a_frames = 0;
				unsigned int m;
				for(m=0;m<i;m++)
				{
					v_frames += file->p_mdat_sect_t[m].video_frames;
					a_frames += file->p_mdat_sect_t[m].audio_frames;
				}
				v_frames += file->current_video_offset;
				a_frames += file->current_audio_offset;
				file->current_video_frame_pos = v_frames;
				file->current_audio_frame_pos = a_frames;
			}
			#endif
			unlock_custommp4(file);//2007-11-18
			return 1;	
		}
	}
	unlock_custommp4(file);//2007-11-18
	return 0;
}

int custommp4_seek_to_sys_time(custommp4_t *file,u32 systime)
{
	u32 timestamp;
	if(systime > file->fpot.end_time)
	{
		return 0;
	}
	if(systime < file->fpot.start_time) timestamp = 0;
	else timestamp = (systime - file->fpot.start_time)*1000;
	return custommp4_seek_to_time_stamp(file,timestamp);
}

int custommp4_write_video_frame(custommp4_t *file,u8 *video_buffer,u32 bytes,u32 timestamp,u8 isKeyFrame,u8 *update)
{
	video_frame_info_table_t tmp_vframe;
	u32 last_pos;
	
	if(update) *update = 0;
	
	if(file->p_mdat_sect_t->video_allocated == 0) return -1;
	
	file->total_video_frames++;
	file->p_mdat_sect_t->video_frames++;
	if(file->p_mdat_sect_t->video_frames == 1 && file->p_mdat_sect_t->audio_frames == 0)
	{
		file->p_mdat_sect_t->start_time = timestamp;
		custommp4_set_position(file,file->p_mdat_sect_t->offset+sizeof(mdat_segment_object_t)+file->p_mdat_sect_t->video_allocated*sizeof(video_frame_info_table_t)+file->p_mdat_sect_t->audio_allocated*sizeof(audio_frame_info_table_t));
	}
	file->p_mdat_sect_t->end_time = timestamp;
	file->p_video_table_t[0].offset = custommp4_position(file)-file->open_offset;//csp modify:相对偏移
	file->p_video_table_t[0].length = bytes;
	file->p_video_table_t[0].timestamp = timestamp;
	file->p_video_table_t[0].key = isKeyFrame;
	custommp4_write_data(file,video_buffer,bytes);
	
	last_pos = custommp4_position(file);
	
	custommp4_set_position(file,file->p_mdat_sect_t->offset+sizeof(mdat_segment_object_t)+(file->p_mdat_sect_t->video_frames-1)*sizeof(video_frame_info_table_t));
	tmp_vframe = *(file->p_video_table_t);
	vframe_endian_put(&tmp_vframe);
	custommp4_write_data(file, &tmp_vframe, sizeof(video_frame_info_table_t));
	
	if(file->p_mdat_sect_t->video_frames >= file->video_table_allocated)
	{
		mdat_segment_object_t msot;
		mdat_vector_object_t tmp_mvot;
		
		//printf("one segment finish 1,video frames:%d,audio frames:%d\n",file->p_mdat_sect_t->video_frames,file->p_mdat_sect_t->audio_frames);
		
		custommp4_set_position(file,file->p_mdat_sect_t->offset);
		msot.obj.object_id = CUSTOMMP4_mdat_segment_object;
		msot.obj.object_size = last_pos-file->p_mdat_sect_t->offset;
		msot.video_frames = file->p_mdat_sect_t->video_frames;
		msot.audio_frames = file->p_mdat_sect_t->audio_frames;
		msot.reserved1 = 0;
		msot.start_time = file->p_mdat_sect_t->start_time;
		msot.end_time = file->p_mdat_sect_t->end_time;
		msot.video_allocated = file->p_mdat_sect_t->video_allocated;
		msot.audio_allocated = file->p_mdat_sect_t->audio_allocated;
		msot_endian_put(&msot);
		custommp4_write_data(file,&msot,sizeof(msot));
		
		custommp4_set_position(file,file->open_offset+sizeof(file->fpot)+sizeof(file->vspot)+sizeof(file->aspot));
		file->mvot.number_of_segments++;
		tmp_mvot = file->mvot;
		mvot_endian_put(&tmp_mvot);
		custommp4_write_data(file, &tmp_mvot, sizeof(file->mvot));
		
		custommp4_set_position(file,last_pos);
		
		custommp4_mdat_segment_info_table_init(file,file->p_mdat_sect_t);
		
		if(update) *update = 1;
	}
	else
	{
		custommp4_set_position(file,last_pos);
	}
	
	return bytes;
}

int custommp4_write_audio_frame(custommp4_t *file,u8 *audio_buffer,u32 bytes,u32 timestamp,u8 *update)
{
	audio_frame_info_table_t tmp_aframe;
	u32 last_pos;
	
	if(update) *update = 0;
	
	if(file->p_mdat_sect_t->audio_allocated == 0) return -1;
	
	file->total_audio_frames++;
	file->p_mdat_sect_t->audio_frames++;
	if(file->p_mdat_sect_t->audio_frames == 1 && file->p_mdat_sect_t->video_frames == 0)
	{
		file->p_mdat_sect_t->start_time = timestamp;
		custommp4_set_position(file,file->p_mdat_sect_t->offset+sizeof(mdat_segment_object_t)+file->p_mdat_sect_t->video_allocated*sizeof(video_frame_info_table_t)+file->p_mdat_sect_t->audio_allocated*sizeof(audio_frame_info_table_t));
	}
	file->p_mdat_sect_t->end_time = timestamp;
	file->p_audio_table_t[0].offset = custommp4_position(file)-file->open_offset;//csp modify:相对偏移
	file->p_audio_table_t[0].timestamp = timestamp;
	custommp4_write_data(file,audio_buffer,file->aspot.sample_size);

	//printf("write audio:frame%d timestamp=%d,offset=%d\n",file->total_audio_frames-1,timestamp,file->p_audio_table_t[0].offset);

	last_pos = custommp4_position(file);

	custommp4_set_position(file,file->p_mdat_sect_t->offset+sizeof(mdat_segment_object_t)+file->p_mdat_sect_t->video_allocated*sizeof(video_frame_info_table_t)+(file->p_mdat_sect_t->audio_frames-1)*sizeof(audio_frame_info_table_t));
	tmp_aframe = *(file->p_audio_table_t);
	aframe_endian_put(&tmp_aframe);
	custommp4_write_data(file, &tmp_aframe, sizeof(audio_frame_info_table_t));

	if(file->p_mdat_sect_t->audio_frames >= file->audio_table_allocated)
	{
		mdat_segment_object_t msot;
		mdat_vector_object_t tmp_mvot;

		//printf("one segment finish 2,video frames:%d,audio frames:%d\n",file->p_mdat_sect_t->video_frames,file->p_mdat_sect_t->audio_frames);

		custommp4_set_position(file,file->p_mdat_sect_t->offset);
		msot.obj.object_id = CUSTOMMP4_mdat_segment_object;
		msot.obj.object_size = last_pos-file->p_mdat_sect_t->offset;
		msot.video_frames = file->p_mdat_sect_t->video_frames;
		msot.audio_frames = file->p_mdat_sect_t->audio_frames;
		msot.reserved1 = 0;
		msot.start_time = file->p_mdat_sect_t->start_time;
		msot.end_time = file->p_mdat_sect_t->end_time;
		msot.video_allocated = file->p_mdat_sect_t->video_allocated;
		msot.audio_allocated = file->p_mdat_sect_t->audio_allocated;
		msot_endian_put(&msot);
		custommp4_write_data(file,&msot,sizeof(msot));
		
		custommp4_set_position(file,file->open_offset+sizeof(file->fpot)+sizeof(file->vspot)+sizeof(file->aspot));
		file->mvot.number_of_segments++;
		tmp_mvot = file->mvot;
		mvot_endian_put(&tmp_mvot);
		custommp4_write_data(file, &tmp_mvot, sizeof(file->mvot));
		
		custommp4_set_position(file,last_pos);
		
		custommp4_mdat_segment_info_table_init(file,file->p_mdat_sect_t);

		if(update) *update = 1;
	}
	else
	{
		custommp4_set_position(file,last_pos);
	}
	
	return file->aspot.sample_size;
}
