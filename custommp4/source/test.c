#include "custommp4.h"
#include "isoformat.h"

unsigned char buf[65536];

int main(int argc, char **argv)
{
#if 1
#if 1
#if 1
	int i;
	
	custommp4_t *file1;
	iflymp4_t *file2;
	
	int width;
	int height;
	
	int vlen;
	int alen;
	
	int vsize;
	int vtimestamp = 0;
	
	u32 start_time;
	u8  key;
	
	if(argc<4)
	{
		printf("usage:test infilename offset outfilename\n");
		exit(-1);
	}
	
	file1 = custommp4_open(argv[1],O_R,atoi(argv[2]));
	file2 = iflymp4_open(argv[3],0,1,0);
	if(file1 == NULL || file2 == NULL)
	{
		printf("open file failed\n");
		exit(-1);
	}
	
	width  = custommp4_video_width(file1);
	height = custommp4_video_height(file1);
	
	vlen = custommp4_video_length(file1);
	alen = custommp4_audio_length(file1);
	
	printf("video frames:%d\n",vlen);
	
	if(custommp4_has_video(file1))
	{
		iflymp4_set_video(file2, 1, width, height, 25, 1000, "XVID");
		
	}
	if(custommp4_has_audio(file1))
	{
		iflymp4_set_audio(file2,1,8000,8,240,1000,30,"PCMU");
	}
	
	for(i=0;i<vlen;i++)
	{
		int j;
		vsize = custommp4_read_video_frame(file1,buf,sizeof(buf),i,&start_time,&key);
		for(j=0;j<1;j++)
		{
			iflymp4_write_video_frame(file2,buf,vsize,1,key,40,0);
			vtimestamp += 40;
		}
	}
	
	custommp4_close(file1);
	iflymp4_close(file2);
#else
	int i;
	
	custommp4_t *file1 = custommp4_open("fly00004.mp4",O_R,0);
	iflymp4_t *file2 = iflymp4_open("layout.mp4",0,1,0);

	int width  = custommp4_video_width(file1);
	int height = custommp4_video_height(file1);
	
	int vlen = custommp4_video_length(file1);
	int alen = custommp4_audio_length(file1);
	
	int vsize;
	int vtimestamp = 0;
	
	u32 start_time;
	u8  key;

	printf("video frames:%d\n",vlen);
	
	if(custommp4_has_video(file1))
	{
		iflymp4_set_video(file2, 1, width, height, 25, 1000, "XVID");

	}
	if(custommp4_has_audio(file1))
	{
		iflymp4_set_audio(file2,1,8000,8,240,1000,30,"PCMU");
	}
	
	for(i=0;i<vlen;i++)
	{
		int j;
		u8  a[4];
		vsize = custommp4_read_video_frame(file1,buf,sizeof(buf),i,&start_time,&key);
		a[0] = buf[0];
		a[1] = buf[1];
		a[2] = buf[2];
		a[3] = buf[3];
		for(j=0;j<1;j++)
		{
			iflymp4_write_video_frame(file2,buf,vsize,1,key,40,0);
			vtimestamp += 40;
		}
	}
	
	custommp4_close(file1);
	iflymp4_close(file2);
#endif
#else
	int i;

	iflymp4_t *file1 = iflymp4_open("rec.mp4",1,0,0);
	custommp4_t *file2 = custommp4_open("my.mp4",O_W_CREAT,0);
	
	int width  = iflymp4_video_width(file1,1);
	int height = iflymp4_video_height(file1,1);
	
	int vlen = iflymp4_video_length(file1,1);
	int alen = iflymp4_audio_length(file1,1);

	int vsize;
	int vtimestamp = 0;

	printf("video frames:%d\n",vlen);

	if(iflymp4_has_video(file1)) custommp4_set_video(file2,1000,(u16)width,(u16)height,25,512*1024,str2uint("XVID"),0x18);
	if(iflymp4_has_audio(file1)) custommp4_set_audio(file2,1000,1,8,8000,str2uint("PCMU"),240,30);

	for(i=0;i<vlen;i++)
	{
		int j;
		vsize = iflymp4_read_frame(file1,buf,1);
		for(j=0;j<1;j++)
		{
			custommp4_write_video_frame(file2,buf,vsize,vtimestamp,1);
			vtimestamp += 40;
		}
	}

	iflymp4_close(file1);
	custommp4_close(file2);
#endif
#else
#if 1
	//FILE *fp = fopen("stream","wb");
	int i;
	int vlen;
	int vsize;
	//iflymp4_t *file1 = iflymp4_open("rec_2.mp4",0,1,0);
	custommp4_t *file2 = custommp4_open("fly00010.mp4",O_R,0);
	u32 start_time;
	u8 key;
	if(file2 == NULL) return -1;
	vlen = custommp4_video_length(file2);
	printf("video frames:%d\n\n",vlen);
	//iflymp4_set_video(file1,1,352,288,25,1000,"XVID");
	//iflymp4_set_audio(file1,1,8000,8,240,1000,30,"PCMU");
	//for(i=5025;i<5251;i++)
	//for(i=5100;i<5176;i++)
	for(i=0;i<vlen;i++)
	{
		vsize = custommp4_read_video_frame(file2,buf,sizeof(buf),i,&start_time,&key);
		//printf("#####video frame%d:size=%d\n",i+1,vsize);
		//iflymp4_write_video_frame(file1,buf,vsize,1,1,40,0);
		
		/*if(i!=5110)
		{
			fwrite(buf,vsize,1,fp);
		}
		else
		{
			//memset(buf,0xff,vsize);
			buf[0] = 0x00;
			buf[1] = 0x00;
			buf[2] = 0x01;
			buf[3] = 0xb6;
			fwrite(buf,vsize,1,fp);
		}*/

		/*//if(i==5110 || i == 5111)
		{
			char filename[64];
			FILE *tmpfp;
			sprintf(filename,"frames\\frame%d",i);
			tmpfp = fopen(filename,"wb");
			fwrite(buf,vsize,1,tmpfp);
			fclose(tmpfp);
			
			printf("frame%d size=%d\n",i-5025,vsize);
			
		}*/
	}
	//iflymp4_close(file1);
	custommp4_close(file2);
	//fclose(fp);
#else
	//FILE *fp = fopen("stream","rb");
	//FILE *fp2 = fopen("stream2","wb");
#endif
#endif
	return 0;
}
