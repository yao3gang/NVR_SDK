#ifndef _TLFILE_H_
#define _TLFILE_H_

#ifdef __cplusplus
extern "C" {
#endif

typedef void * TLFILE_t; 
typedef int BOOL;
typedef unsigned char BYTE;

enum TL_FILE_OPENMODE{TL_FILE_READ = 0, TL_FILE_CREATE = 1};

//////////////////////////////////////////////////////////////////////////
//	打开关闭文件
//////////////////////////////////////////////////////////////////////////

//TL_OpenFile: 打开文件,返回文件句柄
//TL_CloseFile: 关闭文件
TLFILE_t TL_OpenFile(char *filename, TL_FILE_OPENMODE mode = TL_FILE_READ);
void TL_CloseFile(TLFILE_t hFile);

//////////////////////////////////////////////////////////////////////////
//	读文件操作接口
//////////////////////////////////////////////////////////////////////////

//TL_FileHasAudio: 判断文件是否有音频
BOOL TL_FileHasAudio(TLFILE_t hFile);

//TL_FileTotalTime:文件总时间,单位-秒
//TL_FileStartTime: 文件起始时间 time_t时间
//TL_FileEndTime:文件结束时间 time_t时间
int TL_FileTotalTime(TLFILE_t hFile);
int TL_FileStartTime(TLFILE_t hFile);
int TL_FileEndTime(TLFILE_t hFile);

//TL_FileVideoFrameNum:文件含视频总帧数
//TL_FileAudioFrameNum:文件含音频总帧数
int TL_FileVideoFrameNum(TLFILE_t hFile);
int TL_FileAudioFrameNum(TLFILE_t hFile);

//TL_FileVideoWidth: 视频宽
//TL_FileVideoHeight: 视频高
int TL_FileVideoWidth(TLFILE_t hFile);
int TL_FileVideoHeight(TLFILE_t hFile);

//TL_FileAudioBits: 音频位率
//TL_FileAudioSampleRate: 音频帧采样率
unsigned short TL_FileAudioBits(TLFILE_t hFile);
unsigned int TL_FileAudioSampleRate(TLFILE_t hFile);

//TL_FileReadOneMediaFrame: 按所有帧的顺序依次读文件,每次读一帧
//参数:meida_buffer 存放读取数据的buf; start_time 返回时间戳,距离文件开头的毫秒数; 
//keyflag:帧标志; media_type:帧的类型, 0-video  1-audio
//返回值:读取字节数
int TL_FileReadOneMediaFrame(TLFILE_t hFile,BYTE *meida_buffer,unsigned int *start_time,BYTE *keyflag,BYTE *media_type);

//TL_FileReadOneVideoFrame: 按视频帧顺序依次读文件,每次读一帧视频
//参数: video_buffer 存放视频的buf, duration 返回这一帧的持续时间,单位-毫秒; 其余参数同TL_FileReadOneMediaFrame
//返回值:读取字节数
int TL_FileReadOneVideoFrame(TLFILE_t hFile,BYTE *video_buffer,unsigned int *start_time,unsigned int *duration,BYTE *keyFlag);
//TL_FileReadOneAudioFrame: 按音频帧顺序依次读文件,每次读一帧音频
int TL_FileReadOneAudioFrame(TLFILE_t hFile,BYTE *audio_buffer,unsigned int *start_time,unsigned int *duration);

//TL_FileSeekToPrevSeg: 定位到上一段
//TL_FileSeekToNextSeg: 定位到下一段
void TL_FileSeekToPrevSeg(TLFILE_t hFile);
void TL_FileSeekToNextSeg(TLFILE_t hFile);

//TL_FileSeekToSysTime: 定位到systime (time_t格式, 秒数)
void TL_FileSeekToSysTime(TLFILE_t hFile, unsigned int systime);

//TL_FileDecVideoFrame: 解视频帧
//TL_FILEDecAudioFrame: 解音频帧
enum DECRET{DEC_ERR=-1, DEC_OK=0, DEC_MORE=1};
DECRET TL_FileDecVideoFrame(TLFILE_t hFile, BYTE keyFlag, BYTE* encbuf, int enclen, BYTE* decbuf, int* pdeclen);
DECRET TL_FILEDecAudioFrame(TLFILE_t hFile, BYTE* encbuf, int enclen, BYTE* decbuf, int* pdeclen);

//////////////////////////////////////////////////////////////////////////
//	写文件操作接口
//////////////////////////////////////////////////////////////////////////
enum VIDEOCOMPRESSOR{TLFILE_V_H264 = 0};
enum AUDIOCOMPRESSOR{TLFILE_A_GRAW = 0, TLFILE_A_ADPA, TLFILE_A_ADPB};

//TL_FileSetVideo: 设置视频属性
//TL_FileSetAudio: 设置音频属性
void TL_FileSetVideo(TLFILE_t hFile, unsigned short width, unsigned short height, float frame_rate, VIDEOCOMPRESSOR compressor);
void TL_FileSetAudio(TLFILE_t hFile, unsigned short channels, unsigned short bits, unsigned int sample_rate, AUDIOCOMPRESSOR compressor, unsigned int sample_size, unsigned int sample_duration);

//TL_FileWriteVideoFrame: 写视频帧
//TL_FileWriteAudioFrame: 写音频帧
int TL_FileWriteVideoFrame(TLFILE_t hFile, BYTE *video_buffer,unsigned int bytes,unsigned int timestamp,BYTE keyflag);
int TL_FileWriteAudioFrame(TLFILE_t hFile, BYTE *audio_buffer,unsigned int bytes,unsigned int timestamp);

#ifdef __cplusplus
}
#endif

#endif