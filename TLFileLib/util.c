#include "custommp4.h"
#include <string.h>
#ifndef WIN32
#include <unistd.h>
#include <stdlib.h>
#endif
#include <errno.h>

//#define SIZE_OF_FILE_VECTOR		((s64)128*1024*1024)
#define SIZE_OF_FILE_VECTOR		((s64)256*1024*1024)

int fileflush(FILE *fp)
{
#ifndef WIN32
	fflush(fp);
	fsync(fileno(fp));
#endif
	return 1;
}

int filecp(char *src,char *dst)
{
	//printf("*****filecp start*****\n");
	
	int nRet;
	FILE* fp1,*fp2;
	unsigned char byData[1024];
	
	fp1 = fopen(src,"rb");
	if(fp1 == NULL)
	{
		return 0;
	}
	
	fp2 = fopen(dst,"wb");
	if(fp2 == NULL)
	{
		printf("errno=%d,str=%s\n",errno,strerror(errno));
		fclose(fp1);
		return 0;
	}
	
	//size_t fread(void *ptr,size_t size,size_t nmemb,FILE *stream);
	//size_t fwrite(const void *ptr,size_t size,size_t nmemb,FILE *stream);
	
	nRet = fread(byData,1,sizeof(byData),fp1);
	while(nRet > 0)
	{
		fwrite(byData,nRet,1,fp2);
		nRet = fread(byData,1,sizeof(byData),fp1);
	}
	fileflush(fp2);
	
	fclose(fp1);
	fclose(fp2);
	
	//printf("*****filecp success*****\n");
	
	return 1;
}

void uint2str(unsigned char *dst, int n)
{
	dst[0] = (n)&0xff;
	dst[1] = (n>>8)&0xff;
	dst[2] = (n>>16)&0xff;
	dst[3] = (n>>24)&0xff;
}

u32 str2uint(const char *str)
{
	return ( str[0] | (str[1]<<8) | (str[2]<<16) | (str[3]<<24) );	
}

int custommp4_position(custommp4_t *file)
{
	return file->file_position;
}

int custommp4_set_position(custommp4_t *file, int position)
{
	file->file_position = position;
	return 1;
}

int custommp4_end_position(custommp4_t *file)
{
	int ret;
	if(file->open_mode == O_R)
	{
		ret = file->open_offset+file->fpot.file_Size;
	}
	else
	{
		//ret = custommp4_position(file)+file->p_mdat_sect_t->video_frames*sizeof(video_frame_info_table_t)+file->p_mdat_sect_t->audio_frames*sizeof(audio_frame_info_table_t);
		ret = custommp4_position(file);
	}
	return ret;
}

int custommp4_read_data(custommp4_t *file,void *data,int size)
{
	int result;
	if(file->file_position >= (int)SIZE_OF_FILE_VECTOR)
	{
		printf("custommp4_read_data:file_pos=%ld\n",file->file_position);
#ifndef WIN32
// 		sleep(1);
#endif
// 		exit(1);
		return 0;
	}
	if(ftell(file->stream) != file->file_position)
	{
		result = fseek(file->stream, file->file_position, SEEK_SET);
		if(result)
		{
			printf("custommp4_read_data:fseek error=%d,errno=%d,errstr=%s\n",result,errno,strerror(errno));
			result = ftell(file->stream);
			printf("custommp4_read_data:right_pos=%ld,real_pos=%d\n",file->file_position,result);
#ifndef WIN32
// 			sleep(1);
#endif
// 			exit(1);
			return 0;
		}
	}
	result = fread(data, size, 1, file->stream);
	
	file->file_position += size;
	
	return result;
}

int custommp4_write_data(custommp4_t *file,void *data,int size)
{
	int result;
	if(file->file_position >= (int)SIZE_OF_FILE_VECTOR)
	{
		printf("custommp4_write_data:file_pos=%ld\n",file->file_position);
#ifndef WIN32
// 		sleep(1);
#endif
// 		exit(1);
	}
	if(ftell(file->stream) != file->file_position)
	{
		result = fseek(file->stream, file->file_position, SEEK_SET);
		if(result)
		{
			printf("custommp4_write_data:fseek error=%d,errno=%d,errstr=%s\n",result,errno,strerror(errno));
			result = ftell(file->stream);
			printf("custommp4_write_data:right_pos=%ld,real_pos=%d\n",file->file_position,result);
#ifndef WIN32
// 			sleep(1);
#endif
// 			exit(1);
		}
	}
	result = fwrite(data, size, 1, file->stream);
	file->file_position += size;
	/*fileflush(file->stream);
	if(result != 1)
	{
		printf("custommp4_write_data failed\n");
		exit(1);
	}*/
	return result;
}

int custommp4_object_read_header(custommp4_t *file,base_object_t *pobj)
{
	int result = custommp4_read_data(file,pobj,sizeof(base_object_t));
	return !result;
}

BOOL custommp4_object_is(base_object_t *pobj,GUID type)
{
	if(!memcmp(&pobj->object_id,&type,sizeof(GUID)))
	{
		return TRUE;
	}
	return FALSE;
}
