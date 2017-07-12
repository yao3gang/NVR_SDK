#ifndef _DISK_MANAGE_H_
#define _DISK_MANAGE_H_

#include "partitionindex.h"

#define MAX_HDD_NUM			2
#define MAX_PARTITION_NUM	4

#define MAX_SEARCH_NUM		4000

#define	DISK_FULL_STOP		0
#define DISK_FULL_COVER		1

typedef struct
{
	u8 is_disk_exist;
	u8 is_partition_exist[MAX_PARTITION_NUM];
	partition_index ptn_index[MAX_PARTITION_NUM];
	long total;
	long free;
}HddInfo;

typedef struct
{
	HddInfo hinfo[MAX_HDD_NUM];
}disk_manager;

int set_policy_when_disk_full(u8 policy);
int init_disk_manager(disk_manager *hdd_manager);
int get_disk_info(disk_manager *hdd_manager,int disk_index);
int search_all_rec_file(disk_manager *hdd_manager,search_param_t *search,recfileinfo_t *fileinfo_buf,int max_nums);
partition_index* get_rec_path(disk_manager *hdd_manager,char *pPath,u32 *open_offset,int chn);
int get_rec_file_name(recfileinfo_t *fileinfo,char *filename,u32 *open_offset);

#endif
