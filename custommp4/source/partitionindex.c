#include "partitionindex.h"
#include <string.h>
#include <time.h>
#ifndef WIN32
#include <stdlib.h>
#endif

#include <assert.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

int lock_partition_index(partition_index *index)
{
#ifndef WIN32
	return pthread_mutex_lock(&index->lock);
#endif
	return 1;
}

int unlock_partition_index(partition_index *index)
{
#ifndef WIN32
	return pthread_mutex_unlock(&index->lock);
#endif
	return 1;
}

BOOL is_partition_index_valid(FILE *fp)
{
	partition_index_header header;
	u8  *p = (u8 *)&header;
	u32 verify = 0;
	u32 file_len = 0;
	int i = 0;
	int nRet = fread(&header,sizeof(header),1,fp);
	if(nRet<=0)
	{
		printf("read file error\n");
		return FALSE;
	}
	if(header.full_file_nums>header.total_file_nums || header.recorded_file_nums>header.total_file_nums || header.full_file_offset>=header.total_file_nums)
	{
		printf("count file error\n");
		return FALSE;
	}
	fseek(fp,0,SEEK_END);
	file_len = ftell(fp);
	if(file_len != sizeof(partition_index_header)+header.total_file_nums*(sizeof(file_use_info)+8192))
	{
		printf("file len error\n");
		return FALSE;
	}
	for(i=0;i<sizeof(partition_index_header)-4;i++)
	{
		verify += p[i];
	}
	if(verify != header.verify)
	{
		printf("verify error\n");
		return FALSE;
	}
	return TRUE;
}

int update_partition_index(partition_index *index)
{
	if(index->valid)
	{
		int i;
		int readlen;
		int readnums;
		int remain;
		u8  buf[1024];
		u8  *p = index->mapdst1;
		
		memcpy(&index->header,index->mapdst1,sizeof(index->header));
		
		readlen = index->header.total_file_nums*(sizeof(file_use_info)+8192);
		readnums = readlen/sizeof(buf);
		remain = readlen%sizeof(buf);
		
		for(i=0;i<readnums;i++)
		{
			memcpy(buf,p,sizeof(buf));
			p += sizeof(buf);
		}
		if(remain)
		{
			memcpy(buf,p,remain);
		}

		return 1;
	}
	
	return 0;
}

int write_partition_index_header(partition_index *index,partition_index_header *pHeader)
{
	u8  *p = (u8 *)pHeader;
	u32 verify = 0;
	int i = 0;

	pHeader->op_nums++;
	for(i=0;i<sizeof(partition_index_header)-4;i++)
	{
		verify += p[i];
	}
	pHeader->verify = verify;
	
	memcpy(index->mapdst1,pHeader,sizeof(partition_index_header));
	msync(index->mapdst1,sizeof(partition_index_header),MS_SYNC);
	
	memcpy(index->mapdst2,pHeader,sizeof(partition_index_header));
	msync(index->mapdst2,sizeof(partition_index_header),MS_SYNC);
	
	return 1;
}

int proofread_partition_index(partition_index *index)
{
	BOOL bUpdate = FALSE;
	int  i = 0;
	for(i=1;i<33;i++)
	{
		chn_last_use_file *p_chn_info = &index->header.chn_info[i];
		if(p_chn_info->file_no < index->header.total_file_nums)
		{
			if(p_chn_info->sect_nums >= 255 || p_chn_info->end_time > (u32)time(NULL))
			{
				file_use_info f_u_info;
				long position;
				
				f_u_info.start_time = p_chn_info->start_time;
				f_u_info.end_time = p_chn_info->end_time;
				f_u_info.file_no = p_chn_info->file_no;
				f_u_info.busy = 0;
				f_u_info.chn_no = i;
				f_u_info.sect_nums = p_chn_info->sect_nums;
				position = sizeof(index->header)+sizeof(file_use_info)*((index->header.full_file_offset+index->header.full_file_nums)%index->header.total_file_nums);	
				
				memcpy(index->mapdst1+position,&f_u_info,sizeof(f_u_info));
				msync(index->mapdst1+position,sizeof(f_u_info),MS_SYNC);

				memcpy(index->mapdst2+position,&f_u_info,sizeof(f_u_info));
				msync(index->mapdst2+position,sizeof(f_u_info),MS_SYNC);

				index->header.full_file_nums++;
				p_chn_info->file_no = 0xffff;
				p_chn_info->start_time = 0;
				p_chn_info->end_time = 0;
				p_chn_info->sect_nums = 0;
				
				bUpdate = TRUE;
			}
		}
	}
	if(bUpdate)
	{
		write_partition_index_header(index,&index->header);
	}
	return 1;
}

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

int init_partition_index(partition_index *index,char *path)
{
	BOOL bValid = FALSE;
	BOOL bBackupValid = FALSE;
	char filename[64];
	char backup_filename[64];

	sprintf(filename,"%s/index00.bin",path);
	sprintf(backup_filename,"%s/index01.bin",path);

	memset(index,0,sizeof(partition_index));
	
	index->index1 = fopen(filename,"rb+");
	if(index->index1 != NULL)
	{
		if(is_partition_index_valid(index->index1))
		{
			//printf("index file:%s is valid\n",filename);
			bValid = TRUE;
		}
		else
		{
			printf("index file:%s is not valid\n",filename);
		}
		fclose(index->index1);
		index->index1 = NULL;
	}

	index->index2 = fopen(backup_filename,"rb+");
	if(index->index2 != NULL)
	{
		if(is_partition_index_valid(index->index2))
		{
			//printf("backup index file:%s is valid\n",backup_filename);
			bBackupValid = TRUE;
		}
		else
		{
			printf("backup index file:%s is not valid\n",backup_filename);
		}
		fclose(index->index2);
		index->index2 = NULL;
	}

	if(bValid)
	{
		filecp(filename,backup_filename);
	}
	else if(bBackupValid)
	{
		filecp(backup_filename,filename);
	}
	else
	{
		return 0;
	}
	
#ifndef WIN32
	pthread_mutex_init(&index->lock,NULL);
#endif
	
	index->index1 = fopen(filename,"rb+");
	index->index2 = fopen(backup_filename,"rb+");
	
	struct stat statbuf;
	
	if(fstat(fileno(index->index1),&statbuf) < 0)
    {
        perror("fstat file 1");
		fclose(index->index1);
		index->index1 = NULL;
		fclose(index->index2);
		index->index2 = NULL;
		return 0;
    }
	index->mapdst1 = (u8 *)mmap(NULL,statbuf.st_size,PROT_READ|PROT_WRITE,MAP_SHARED,fileno(index->index1),0);
	if(index->mapdst1 == NULL)
	{
		perror("map file 1");
		fclose(index->index1);
		index->index1 = NULL;
		fclose(index->index2);
		index->index2 = NULL;
		return 0;
	}
	index->length1 = statbuf.st_size;
	
	if(fstat(fileno(index->index2),&statbuf) < 0)
    {
        perror("fstat file 2");
		munmap(index->mapdst1,index->length1);
		index->mapdst1 = NULL;
		index->length1 = 0;
		fclose(index->index1);
		index->index1 = NULL;
		fclose(index->index2);
		index->index2 = NULL;
		return 0;
    }
	index->mapdst2 = (u8 *)mmap(NULL,statbuf.st_size,PROT_READ|PROT_WRITE,MAP_SHARED,fileno(index->index2),0);
	if(index->mapdst2 == NULL)
	{
		perror("map file 2");
		munmap(index->mapdst1,index->length1);
		index->mapdst1 = NULL;
		index->length1 = 0;
		fclose(index->index1);
		index->index1 = NULL;
		fclose(index->index2);
		index->index2 = NULL;
		return 0;
	}
	index->length2 = statbuf.st_size;
	
	index->valid = 1;
	
	update_partition_index(index);
	
	return 1;
}

int destroy_partition_index(partition_index *index)
{
	if(index->valid)
	{
		index->valid = 0;
#ifndef WIN32
		pthread_mutex_destroy(&index->lock);
#endif
		if(index->mapdst1)
		{
			munmap(index->mapdst1,index->length1);
			index->mapdst1 = NULL;
			index->length1 = 0;
		}
		if(index->mapdst2)
		{
			munmap(index->mapdst2,index->length2);
			index->mapdst2 = NULL;
			index->length2 = 0;
		}
		if(index->index1)
		{
			printf("fclose 1 with fsync\n");
			fclose(index->index1);
			index->index1 = NULL;
		}
		if(index->index2)
		{
			printf("fclose 2 with fsync\n");
			fclose(index->index2);
			index->index2 = NULL;
		}
	}

	return 1;
}

s64 get_partition_total_space(partition_index *index)
{
	if(index->valid)
	{
		printf("total_file_nums=%d\n",index->header.total_file_nums);
		return index->header.total_file_nums*SIZE_OF_FILE_VECTOR;
	}
	return 0;
}

s64 get_partition_free_space(partition_index *index)
{
	if(index->valid)
	{
		printf("free_file_nums=%d\n",index->header.total_file_nums-index->header.recorded_file_nums);
		return (index->header.total_file_nums-index->header.recorded_file_nums)*SIZE_OF_FILE_VECTOR;
	}
	return 0;
}

long seek_to_segment(partition_index *index,partition_index_header *pHeader,int file_no,int sect_no)
{
	if(index->valid)
	{
		long position = sizeof(partition_index_header)+pHeader->total_file_nums*sizeof(file_use_info)+8192*file_no+32*sect_no;
		return position;
	}
	return 0;
}

int get_chn_next_segment(partition_index *index,int chn,int *file_no,int *sect_offset)
{
	if(index->valid)
	{
		lock_partition_index(index);
		
		if(index->header.full_file_nums < index->header.total_file_nums)
		{
			chn_last_use_file *p_chn_info = &index->header.chn_info[chn+1];
			segment_use_info s_u_info;
			file_use_info f_u_info;
			long position;
			if(p_chn_info->file_no < index->header.total_file_nums)
			{
				if(p_chn_info->sect_nums < 255 && p_chn_info->end_time <= (u32)time(NULL))//缺少开始时间的判断
				{
					position = seek_to_segment(index,&index->header,p_chn_info->file_no,p_chn_info->sect_nums);
					memcpy(&s_u_info,index->mapdst1+position,sizeof(s_u_info));
					*file_no = p_chn_info->file_no;
					*sect_offset = (s_u_info.end_position/512+1)*512;
					p_chn_info->sect_nums++;
					
					//printf("#####chn%d:file_no=%d,sect_offset=%d,sect_nums=%d\n",chn,*file_no,*sect_offset,p_chn_info->sect_nums+1);
					
					memset(&s_u_info,0,sizeof(s_u_info));
					s_u_info.type = 0;
					s_u_info.start_time = time(NULL);
					s_u_info.end_time = s_u_info.start_time;
					s_u_info.start_position = *sect_offset;
					s_u_info.end_position = s_u_info.start_position;
					position = seek_to_segment(index,&index->header,p_chn_info->file_no,p_chn_info->sect_nums);
					
					memcpy(index->mapdst1+position,&s_u_info,sizeof(s_u_info));
					msync(index->mapdst1+position,sizeof(s_u_info),MS_SYNC);

					memcpy(index->mapdst2+position,&s_u_info,sizeof(s_u_info));
					msync(index->mapdst2+position,sizeof(s_u_info),MS_SYNC);

					unlock_partition_index(index);
					
					return 1;
				}
				else
				{
					f_u_info.start_time = p_chn_info->start_time;
					f_u_info.end_time = p_chn_info->end_time;
					f_u_info.file_no = p_chn_info->file_no;
					f_u_info.busy = 0;
					f_u_info.chn_no = chn+1;
					f_u_info.sect_nums = p_chn_info->sect_nums;
					
					position = sizeof(index->header)+sizeof(file_use_info)*((index->header.full_file_offset+index->header.full_file_nums)%index->header.total_file_nums);
					
					memcpy(index->mapdst1+position,&f_u_info,sizeof(f_u_info));
					msync(index->mapdst1+position,sizeof(f_u_info),MS_SYNC);

					memcpy(index->mapdst2+position,&f_u_info,sizeof(f_u_info));
					msync(index->mapdst2+position,sizeof(f_u_info),MS_SYNC);

					index->header.full_file_nums++;
					p_chn_info->file_no = 0xffff;
					p_chn_info->start_time = 0;
					p_chn_info->end_time = 0;
					p_chn_info->sect_nums = 0;
					
					write_partition_index_header(index,&index->header);
				}
			}
			if(index->header.recorded_file_nums < index->header.total_file_nums)
			{
				*file_no = index->header.recorded_file_nums;//此处可能有问题
				*sect_offset = 0;
				
				p_chn_info->start_time = time(NULL);
				p_chn_info->end_time = p_chn_info->start_time;
				p_chn_info->sect_nums = 0;
				p_chn_info->file_no = *file_no;
				
				index->header.recorded_file_nums++;
				
				//printf("*****chn%d:file_no=%d,sect_offset=%d,sect_nums=%d\n",chn,*file_no,*sect_offset,p_chn_info->sect_nums+1);
				
				memset(&s_u_info,0,sizeof(s_u_info));
				s_u_info.type = 0;
				s_u_info.start_time = p_chn_info->start_time;
				s_u_info.end_time = s_u_info.start_time;
				s_u_info.start_position = *sect_offset;
				s_u_info.end_position = s_u_info.start_position;
				
				position = seek_to_segment(index,&index->header,p_chn_info->file_no,p_chn_info->sect_nums);
				
				memcpy(index->mapdst1+position,&s_u_info,sizeof(s_u_info));
				msync(index->mapdst1+position,sizeof(s_u_info),MS_SYNC);

				memcpy(index->mapdst2+position,&s_u_info,sizeof(s_u_info));
				msync(index->mapdst2+position,sizeof(s_u_info),MS_SYNC);

				unlock_partition_index(index);
				
				return 1;
			}
		}
		
		unlock_partition_index(index);
		
		return 0;
	}
	
	return 0;
}

int get_first_full_file_end_time(partition_index *index,u32 *end_time)
{
	if(index->valid)
	{
		file_use_info f_u_info;
		long position;
		
		lock_partition_index(index);
		
		position = sizeof(index->header)+index->header.full_file_offset*sizeof(file_use_info);
		
		memcpy(&f_u_info,index->mapdst1+position,sizeof(f_u_info));
		
		unlock_partition_index(index);
		
		*end_time = f_u_info.end_time;
		
		return 1;
	}

	return 0;
}

int get_chn_next_segment_force(partition_index *index,int chn,int *file_no,int *sect_offset)
{
	if(index->valid)
	{
		file_use_info f_u_info;
		segment_use_info s_u_info;
		long position;
		
		lock_partition_index(index);
		
		position = sizeof(index->header)+index->header.full_file_offset*sizeof(file_use_info);
		
		memcpy(&f_u_info,index->mapdst1+position,sizeof(f_u_info));
		
		index->header.full_file_offset = (index->header.full_file_offset+1)%index->header.total_file_nums;
		index->header.full_file_nums--;
		//index->header.recorded_file_nums--;
		//write_partition_index_header(index,&index->header);
		
		index->header.chn_info[chn+1].file_no = f_u_info.file_no;
		index->header.chn_info[chn+1].sect_nums = 0;
		index->header.chn_info[chn+1].start_time = time(NULL);
		index->header.chn_info[chn+1].end_time = index->header.chn_info[chn+1].start_time;
		write_partition_index_header(index,&index->header);
		*file_no = f_u_info.file_no;
		*sect_offset = 0;
		
		memset(&s_u_info,0,sizeof(s_u_info));
		s_u_info.type = 0;
		s_u_info.start_time = index->header.chn_info[chn+1].start_time;
		s_u_info.end_time = s_u_info.start_time;
		s_u_info.start_position = 0;
		s_u_info.end_position = s_u_info.start_position;	
		
		position = seek_to_segment(index,&index->header,f_u_info.file_no,0);
		
		memcpy(index->mapdst1+position,&s_u_info,sizeof(s_u_info));
		msync(index->mapdst1+position,sizeof(s_u_info),MS_SYNC);

		memcpy(index->mapdst2+position,&s_u_info,sizeof(s_u_info));
		msync(index->mapdst2+position,sizeof(s_u_info),MS_SYNC);

		unlock_partition_index(index);
		
		return 1;
	}

	return 0;
}

int update_chn_cur_segment(partition_index *index,int chn,segment_use_info *p_s_u_info,u8 finished)
{
	//printf("chn%d update_chn_cur_segment:hehe1\n",chn);

	if(index->valid)
	{
		chn_last_use_file *p_chn_info;
		long position;

		//printf("chn%d update_chn_cur_segment:hehe2\n",chn);

		lock_partition_index(index);
		
		//printf("chn%d update_chn_cur_segment:hehe2.1\n",chn);
		
		p_chn_info = &index->header.chn_info[chn+1];
		p_chn_info->end_time = p_s_u_info->end_time;
		
		//printf("chn%d update_chn_cur_segment:hehe2.2\n",chn);
		
		if(p_s_u_info->type == 0)
		{
			printf("##################warning:update chn%d record type is 0\n",chn);
		}
		
		//printf("chn%d update_chn_cur_segment:file_no=%d,sect_nums=%d,hehe2.3\n",chn,p_chn_info->file_no,p_chn_info->sect_nums);
		
		position = seek_to_segment(index,&index->header,p_chn_info->file_no,p_chn_info->sect_nums);
		
		//printf("chn%d update_chn_cur_segment:position=%ld,length=%d,hehe2.4\n",chn,position,index->length1);
		
		memcpy(index->mapdst1+position,p_s_u_info,sizeof(segment_use_info));
		msync(index->mapdst1+position,sizeof(segment_use_info),MS_SYNC);
		
		//printf("chn%d update_chn_cur_segment:hehe2.5\n",chn);

		memcpy(index->mapdst2+position,p_s_u_info,sizeof(segment_use_info));
		msync(index->mapdst2+position,sizeof(segment_use_info),MS_SYNC);
		
		//printf("chn%d update_chn_cur_segment:hehe3\n",chn);
		
		if(finished)
		{
			if(p_chn_info->sect_nums >= 255 || p_s_u_info->end_position > (SIZE_OF_FILE_VECTOR-2*SIZE_OF_RESERVED) || p_chn_info->end_time > (u32)time(NULL))
			{
				file_use_info f_u_info;
				f_u_info.start_time = p_chn_info->start_time;
				f_u_info.end_time = p_chn_info->end_time;
				f_u_info.file_no = p_chn_info->file_no;
				f_u_info.busy = 0;
				f_u_info.chn_no = chn+1;
				f_u_info.sect_nums = p_chn_info->sect_nums;
				position = sizeof(index->header)+sizeof(file_use_info)*((index->header.full_file_offset+index->header.full_file_nums)%index->header.total_file_nums);
				
				memcpy(index->mapdst1+position,&f_u_info,sizeof(f_u_info));
				msync(index->mapdst1+position,sizeof(f_u_info),MS_SYNC);

				memcpy(index->mapdst2+position,&f_u_info,sizeof(f_u_info));
				msync(index->mapdst2+position,sizeof(f_u_info),MS_SYNC);

				index->header.full_file_nums++;
				p_chn_info->file_no = 0xffff;
				p_chn_info->start_time = 0;
				p_chn_info->end_time = 0;
				p_chn_info->sect_nums = 0;
			}
		}
		
		write_partition_index_header(index,&index->header);
		
		unlock_partition_index(index);
		
		//printf("chn%d update_chn_cur_segment:hehe4\n",chn);
		
		return 1;
	}
	
	//printf("chn%d update_chn_cur_segment:hehe5\n",chn);
	
	return 0;
}

int is_type_matching(u8 type,u8 mask)
{
	type = type & 0x0f;
	if(!type) return 0;
	
	mask = mask & 0x1f;
	if(!mask) return 0;
	
	if(mask & RECTYPE_MASK_ALL)
	{
		return 1;
	}
	else
	{
		//if(mask == (type & mask))
		if(type & mask)
		{
			return 1;
		}
	}
	
	return 0;
}

int search_rec_file(partition_index *index,search_param_t *search,recfileinfo_t *fileinfo_buf,int max_nums,u8 disk_no,u8 ptn_no)
{
	if(index->valid)
	{
		file_use_info f_u_info;
		segment_use_info s_u_info;
		chn_last_use_file *p_chn_info;
		long position;
		u16  i,j;
		int  ret = 0;
		
		lock_partition_index(index);
		
		//printf("search_rec_file:disk_no=%d,ptn_no=%d,full=%d\n",disk_no,ptn_no,index->header.full_file_nums);
		
		for(i=0;i<index->header.full_file_nums;i++)
		{
			position = sizeof(index->header)+((index->header.full_file_offset+i)%index->header.total_file_nums)*sizeof(file_use_info);
			memcpy(&f_u_info,index->mapdst1+position,sizeof(f_u_info));
			
			//printf("hehe1,chn=%d,i=%d,position=%ld,ret=%d,disk_no=%d,ptn_no=%d,index=0x%x\n",search->channel_no,i,position,ret,disk_no,ptn_no,(int)index);
			
			if(f_u_info.chn_no == 1)
			{
				//printf("hehe1,file_no=%d,chn=%d,sect_nums=%d,start_time=%d,end_time=%d\n",f_u_info.file_no,f_u_info.chn_no,f_u_info.sect_nums+1,f_u_info.start_time,f_u_info.end_time);
				if(f_u_info.end_time <= f_u_info.start_time)
				{
					printf("!!!!!!!!!!!!!!!!end<=start(%d,%d,%d,%d,%d)!!!!!!!!!!!!!!!!\n",f_u_info.chn_no,f_u_info.start_time,f_u_info.end_time,f_u_info.file_no,f_u_info.sect_nums);
				}
			}

#if 1
			if(f_u_info.chn_no != search->channel_no+1 || f_u_info.start_time >= search->end_time || f_u_info.end_time <= search->start_time)
			{
				continue;
			}
#endif
			
			//if(f_u_info.chn_no == 1) printf("hehe2,file_no=%d,chn=%d,sect_nums=%d,start_time=%d,end_time=%d\n",f_u_info.file_no,f_u_info.chn_no,f_u_info.sect_nums+1,f_u_info.start_time,f_u_info.end_time);
			int cc = 0;

			//printf("hehe2,chn=%d,i=%d,position=%ld,ret=%d,disk_no=%d,ptn_no=%d,index=0x%x\n",search->channel_no,i,position,ret,disk_no,ptn_no,(int)index);
			
			for(j=0;j<f_u_info.sect_nums+1;j++)
			{
				position = seek_to_segment(index,&index->header,f_u_info.file_no,j);
				memcpy(&s_u_info,index->mapdst1+position,sizeof(s_u_info));
				
#if 1
				if(s_u_info.start_time >= search->end_time)
				{
					if(f_u_info.chn_no == 1 && f_u_info.file_no == 0)
					{
						printf("break1\n");
					}
					break;
				}
				if(s_u_info.end_time <= search->start_time)
				{
					if(f_u_info.chn_no == 1 && f_u_info.file_no == 0)
					{
						printf("break2,search->start_time=%d,s_u_info.start_time=%d,s_u_info.end_time=%d,pos:(%d,%d)\n",search->start_time,s_u_info.start_time,s_u_info.end_time,s_u_info.start_position,s_u_info.end_position);
					}
					continue;
				}
				else
				{
					printf("no break,search->start_time=%d,s_u_info.start_time=%d,s_u_info.end_time=%d,pos:(%d,%d)\n",search->start_time,s_u_info.start_time,s_u_info.end_time,s_u_info.start_position,s_u_info.end_position);
				}
				if(s_u_info.end_time <= s_u_info.start_time/* || s_u_info.end_position <= s_u_info.start_position*/)
				{
					if(f_u_info.chn_no == 1 && f_u_info.file_no == 0)
					{
						printf("break3\n");
					}
					continue;
				}
				
				if(s_u_info.end_position <= s_u_info.start_position)
				{
					if(f_u_info.chn_no == 1 && f_u_info.file_no == 0)
					{
						printf("break4\n");
					}
					continue;
				}
				
#endif
				if(is_type_matching(s_u_info.type,search->type))
				{
					if(ret+1>max_nums)
					{
						unlock_partition_index(index);
						return -1;
					}
					fileinfo_buf[ret].disk_no = disk_no;
					fileinfo_buf[ret].ptn_no = ptn_no;
					fileinfo_buf[ret].channel_no = search->channel_no;
					fileinfo_buf[ret].start_time = s_u_info.start_time;
					fileinfo_buf[ret].end_time = s_u_info.end_time;
					fileinfo_buf[ret].file_no = f_u_info.file_no;
					fileinfo_buf[ret].offset = s_u_info.start_position;
					fileinfo_buf[ret].size = s_u_info.end_position - s_u_info.start_position;
					fileinfo_buf[ret].type = s_u_info.type;
					fileinfo_buf[ret].image_format = s_u_info.image_format;
					fileinfo_buf[ret].stream_flag = s_u_info.stream_flag;
					++ret;

					++cc;
				}
			}
			
			if(f_u_info.chn_no == 1) printf("add %d files\n",cc);
			
			//printf("hehe3,chn=%d,i=%d,position=%ld,ret=%d,disk_no=%d,ptn_no=%d,index=0x%x\n",search->channel_no,i,position,ret,disk_no,ptn_no,(int)index);
		}
		p_chn_info = &index->header.chn_info[search->channel_no+1];
		if(p_chn_info->file_no < index->header.total_file_nums)
		{
			printf("no full file=%d,sect_nums=%d\n",p_chn_info->file_no,p_chn_info->sect_nums+1);
			printf("chn_info:(%d,%d),search:(%d,%d)\n",p_chn_info->start_time,p_chn_info->end_time,search->start_time,search->end_time);
			//if(!(p_chn_info->start_time >= search->end_time || p_chn_info->end_time <= search->start_time))
			{
				printf("haha1\n");
				for(j=0;j<p_chn_info->sect_nums+1;j++)
				{
					position = seek_to_segment(index,&index->header,p_chn_info->file_no,j);
					memcpy(&s_u_info,index->mapdst1+position,sizeof(s_u_info));
					printf("haha2,(%d,%d)\n",s_u_info.start_time,s_u_info.end_time);
					if(s_u_info.start_time >= search->end_time)
					{
						//break;
						continue;
					}
					printf("haha3\n");
					if(s_u_info.end_time <= search->start_time)
					{
						continue;
					}
					printf("haha4\n");
					if(s_u_info.end_time <= s_u_info.start_time || s_u_info.end_position <= s_u_info.start_position)
					{
						continue;
					}
					printf("haha5\n");
					if(is_type_matching(s_u_info.type,search->type))
					{
						if(ret+1>max_nums)
						{
							unlock_partition_index(index);
							return -1;
						}
						fileinfo_buf[ret].disk_no = disk_no;
						fileinfo_buf[ret].ptn_no = ptn_no;
						fileinfo_buf[ret].channel_no = search->channel_no;
						fileinfo_buf[ret].start_time = s_u_info.start_time;
						fileinfo_buf[ret].end_time = s_u_info.end_time;
						fileinfo_buf[ret].file_no = p_chn_info->file_no;
						fileinfo_buf[ret].offset = s_u_info.start_position;
						fileinfo_buf[ret].size = s_u_info.end_position - s_u_info.start_position;
						fileinfo_buf[ret].type = s_u_info.type;
						fileinfo_buf[ret].image_format = s_u_info.image_format;
						fileinfo_buf[ret].stream_flag = s_u_info.stream_flag;
						++ret;
					}
				}
			}
		}
		
		unlock_partition_index(index);
		
		//printf("search_rec_file:disk_no=%d,ptn_no=%d,total=%d\n",disk_no,ptn_no,ret);
		
		return ret;
	}
	
	return 0;
}
