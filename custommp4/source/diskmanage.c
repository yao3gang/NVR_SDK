#include "diskmanage.h"
#include "hddcmd.h"
#include "common.h"
#include <string.h>
#ifdef WIN32
#include <fcntl.h>
#include <io.h>
#include <direct.h>
#else
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#endif

static u8 disk_full_policy = DISK_FULL_COVER;

int init_disk_manager(disk_manager *hdd_manager)
{
	int  ret;
	DiskInfo dinfo;
	int  i;
	int  fd;
	char devname[16];
	char mountname[16];
	memset(hdd_manager,0,sizeof(disk_manager));
	ret = ifly_diskinfo("/dev/hda",&dinfo);
	if(ret == 0)
	{
		printf("检测到主盘\n");
		hdd_manager->hinfo[0].is_disk_exist = 1;
		hdd_manager->hinfo[0].total = 0;
		hdd_manager->hinfo[0].free  = 0;
		for(i=0;i<MAX_PARTITION_NUM;i++)
		{
			sprintf(devname,"/dev/hda%d",i+1);
			fd = open(devname,O_RDONLY);
			if(fd != -1)
			{
				close(fd);
				hdd_manager->hinfo[0].is_partition_exist[i] = 1;
				sprintf(mountname,"rec/a%d",i+1);
#ifdef WIN32
				mkdir(mountname);
#else
				mkdir(mountname,1);
#endif
				mount_user(devname,mountname);

				init_partition_index(&hdd_manager->hinfo[0].ptn_index[i],mountname);
				hdd_manager->hinfo[0].total += (long)(get_partition_total_space(&hdd_manager->hinfo[0].ptn_index[i])/1024);
				hdd_manager->hinfo[0].free += (long)(get_partition_free_space(&hdd_manager->hinfo[0].ptn_index[i])/1024);
			}
			else break;
		}
	}
	ret = ifly_diskinfo("/dev/hdb",&dinfo);
	if(ret == 0)
	{
		printf("检测到从盘\n");
		hdd_manager->hinfo[1].is_disk_exist = 1;
		hdd_manager->hinfo[1].total = 0;
		hdd_manager->hinfo[1].free  = 0;
		for(i=0;i<MAX_PARTITION_NUM;i++)
		{
			sprintf(devname,"/dev/hdb%d",i+1);
			fd = open(devname,O_RDONLY);
			if(fd != -1)
			{
				close(fd);
				hdd_manager->hinfo[1].is_partition_exist[i] = 1;
				sprintf(mountname,"rec/b%d",i+1);
#ifdef WIN32
				mkdir(mountname);
#else
				mkdir(mountname,1);
#endif
				mount_user(devname,mountname);
				init_partition_index(&hdd_manager->hinfo[1].ptn_index[i],mountname);
				hdd_manager->hinfo[1].total += (long)(get_partition_total_space(&hdd_manager->hinfo[1].ptn_index[i])/1024);
				hdd_manager->hinfo[1].free += (long)(get_partition_free_space(&hdd_manager->hinfo[1].ptn_index[i])/1024);
			}
			else break;
		}
	}
	return 1;
}

int get_disk_info(disk_manager *hdd_manager,int disk_index)
{
	int i;
	HddInfo *phinfo = &hdd_manager->hinfo[disk_index];
	if(phinfo->is_disk_exist)
	{
		phinfo->total = 0;
		phinfo->free = 0;
		for(i=0;i<MAX_PARTITION_NUM;i++)
		{
			phinfo->total += (long)(get_partition_total_space(&phinfo->ptn_index[i])/1024);
			phinfo->free += (long)(get_partition_free_space(&phinfo->ptn_index[i])/1024);
			printf("disk%d:total=%ld,free=%ld\n",disk_index,phinfo->total,phinfo->free);
		}
	}
	return 1;
}

partition_index* get_rec_path(disk_manager *hdd_manager,char *pPath,u32 *open_offset,int chn)
{
	int i,j;
	int file_no,sect_offset;
	HddInfo *phinfo;
	u32 min_end_time = (u32)(-1);
	u32 end_time;
	int cover_disk = -1;
	int cover_ptn = -1;
	for(i=0;i<MAX_HDD_NUM;i++)
	{
		phinfo = &hdd_manager->hinfo[i];
		if(phinfo->is_disk_exist)
		{
			for(j=0;j<MAX_PARTITION_NUM;j++)
			{
				if(phinfo->is_partition_exist[j])
				{
					//printf("before:get_chn_next_segment\n");
					if(get_chn_next_segment(&phinfo->ptn_index[j],chn,&file_no,&sect_offset))
					{
						//printf("after1:get_chn_next_segment\n");
						//printf("disk partition=hd%c%d,file_no=%d,sect_offset=%d,chn=%d\n",'a'+i,j+1,file_no,sect_offset,chn);
						sprintf(pPath,"rec/%c%d/fly%05d.mp4",'a'+i,j+1,file_no);
						*open_offset = sect_offset;
						//printf("new record file name:%s,open_offset=%d,chn=%d\n",pPath,*open_offset,chn);
						return &phinfo->ptn_index[j];
					}
					//printf("after2:get_chn_next_segment\n");
				}
			}
		}
	}
	if(disk_full_policy == DISK_FULL_COVER)
	{
		for(i=0;i<MAX_HDD_NUM;i++)
		{
			phinfo = &hdd_manager->hinfo[i];
			if(phinfo->is_disk_exist)
			{
				for(j=0;j<MAX_PARTITION_NUM;j++)
				{
					if(phinfo->is_partition_exist[j])
					{
						if(get_first_full_file_end_time(&phinfo->ptn_index[j],&end_time))
						{
							if(end_time < min_end_time)
							{
								min_end_time = end_time;
								cover_disk = i;
								cover_ptn = j;
							}
						}
					}
				}
			}
		}
	}
	if(cover_disk != -1)
	{
		phinfo = &hdd_manager->hinfo[cover_disk];
		if(get_chn_next_segment_force(&phinfo->ptn_index[cover_ptn],chn,&file_no,&sect_offset))
		{
			printf("disk partition=hd%c%d,file_no=%d,sect_offset=%d,chn=%d\n",'a'+cover_disk,cover_ptn+1,file_no,sect_offset,chn);
			sprintf(pPath,"rec/%c%d/fly%05d.mp4",'a'+cover_disk,cover_ptn+1,file_no);
			*open_offset = sect_offset;
			printf("new record file name:%s,open_offset=%d,chn=%d\n",pPath,*open_offset,chn);
			return &phinfo->ptn_index[cover_ptn];
		}
	}
	return NULL;
}

int set_policy_when_disk_full(u8 policy)
{
	if(policy > DISK_FULL_COVER) policy = DISK_FULL_COVER;
	disk_full_policy = policy;
	return 1;
}

int sort_file_with_start_time(recfileinfo_t *fileinfo_buf,int nums)
{
	int i,j;
	recfileinfo_t tmp;
	for(i=0;i<nums-1;i++)
	{
		for(j=0;j<nums-1-i;j++)
		{
			if(fileinfo_buf[j].start_time > fileinfo_buf[j+1].start_time)
			{
				tmp = fileinfo_buf[j];
				fileinfo_buf[j] = fileinfo_buf[j+1];
				fileinfo_buf[j+1] = tmp;
			}
		}
	}
	return 1;
}

#ifndef WIN32
#include <sys/time.h>
#define PRINT_SEARCH_TIME
#endif

int search_all_rec_file(disk_manager *hdd_manager,search_param_t *search,recfileinfo_t *fileinfo_buf,int max_nums)
{
	u8 i,j;
	HddInfo *phinfo;
	int search_count = 0;
	int ret = 0;

	#ifdef PRINT_SEARCH_TIME
	struct timeval start,end;
	long span;
	#endif

	for(i=0;i<MAX_HDD_NUM;i++)
	{
		phinfo = &hdd_manager->hinfo[i];
		if(phinfo->is_disk_exist)
		{
			for(j=0;j<MAX_PARTITION_NUM;j++)
			{
				if(phinfo->is_partition_exist[j])
				{
					#ifdef PRINT_SEARCH_TIME
					gettimeofday(&start,NULL);
					#endif

					ret = search_rec_file(&phinfo->ptn_index[j],search,fileinfo_buf+search_count,max_nums-search_count,i,j);
					
					#ifdef PRINT_SEARCH_TIME
					gettimeofday(&end,NULL);
					span = (end.tv_sec-start.tv_sec)*1000000+(end.tv_usec-start.tv_usec);
					printf("*******disk%c ptn%d search %d files,span:%ld\n",'a'+i,j+1,ret,span/1000);
					#endif

					if(ret < 0)
					{
						return -1;
					}
					search_count += ret;
				}
			}
		}
	}
	if(search_count)
	{
		#ifdef PRINT_SEARCH_TIME
		gettimeofday(&start,NULL);
		#endif

		sort_file_with_start_time(fileinfo_buf,search_count);

		#ifdef PRINT_SEARCH_TIME
		gettimeofday(&end,NULL);
		span = (end.tv_sec-start.tv_sec)*1000000+(end.tv_usec-start.tv_usec);
		printf("sort %d files,span:%ld\n",search_count,span/1000);
		#endif
	}
	return search_count;
}

int get_rec_file_name(recfileinfo_t *fileinfo,char *filename,u32 *open_offset)
{
	*open_offset = fileinfo->offset;
	sprintf(filename,"rec/%c%d/fly%05d.mp4",'a'+fileinfo->disk_no,fileinfo->ptn_no+1,fileinfo->file_no);
	return 1;
}
