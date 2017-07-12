#ifndef _MEDIA_COMMON_H_
#define _MEDIA_COMMON_H_

#include "common.h"

#include <sys/mman.h>
#include <sys/select.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <linux/fb.h>
#include <linux/videodev2.h>
#include <media/davinci_vpfe.h>

#define FB_OUTTYPE_CVBS			0
#define FB_OUTTYPE_VGA			1

/*图像格式*/
#define VID_CIF_FORMAT		3
#define VID_4CIF_FORMAT		4

/*码率模式*/
#define VID_CONST_BITRATE	0	/*定码率*/
#define VID_VAR_BITRATE		1	/*变码率*/

/*图像质量*/
#define VID_SPEED_FIRST		0	/*速度优先*/
#define VID_QUALITY_FIRST	1	/*质量优先*/

/*字节序*/
#define B_ENDIAN			0	/*大头序*/
#define S_ENDIAN			1	/*小头序*/
#define DEFAULT_ENDIAN		B_ENDIAN	/*默认字节序为大头序*/

/* Video window to show diagram on */
#define TW2834_DEVICE       "/dev/tw2834"
#define V4L2_DEVICE			"/dev/video0"
#define SOUND_DEVICE		"/dev/dsp"
#define FBVID_DEVICE        "/dev/fb/3"
#define OSD_DEVICE          "/dev/fb/0"
#define ATTR_DEVICE         "/dev/fb/2"

/* Describes a capture frame buffer */
typedef struct VideoBuffer
{
    void   *start;
    size_t  length;
}VideoBuffer;

struct Zoom_Params
{
    u_int32_t WindowID;
    u_int32_t Zoom_H;
    u_int32_t Zoom_V;
};

/* Custom Davinci FBDEV defines (should be in device driver header) */
#define VID0_INDEX 0
#define VID1_INDEX 1
#define ZOOM_1X    0
#define ZOOM_2X    1
#define ZOOM_4X    2

#define FBIO_SETZOOM			_IOW('F', 0x24, struct Zoom_Params)
#define FBIO_WAITFORVSYNC		_IOW('F', 0x20, u_int32_t)
#define FBIO_GETSTD				_IOR('F', 0x25, u_int32_t)

/* Scaling factors for the video standards */
#define NOSTD					0
#define PAL						12
#define NTSC					10

/* Screen dimensions */
#define SCREEN_BPP				16

/* Black color in UYVY format */
#define UYVY_BLACK				0x10801080

/* The 0-7 transparency value to use for the OSD */
#define OSD_TRANSPARENCY		0x77
#define MIX_TRANSPARENCY		0x55
#define IMAGE_TRANSPARENCY		0x00

/* Video display is triple buffered */
#define OSD_BUFS				2

/* Video display is triple buffered */
#define DISPLAY_BUFS			3

/* Video capture is triple buffered */
#define CAP_BUFS				3

/* Screen dimensions */
#define D1_WIDTH                720
#define D1_HEIGHT               576
#define D1_LINE_WIDTH           D1_WIDTH * SCREEN_BPP / 8
#define D1_FRAME_SIZE           D1_LINE_WIDTH * D1_HEIGHT

#define CAP_FIELD_INTERLACED	0
#define CAP_FIELD_SEQ_TB		1

#ifdef __cplusplus
extern "C" {
#endif

int initCaptureDevice(VideoBuffer **vidBufsPtr,
					  int *numVidBufsPtr, int *captureSizePtr,int fieldMode);
void cleanupCaptureDevice(int fd, VideoBuffer *vidBufs, int numVidBufs);
inline int waitForFrame(int fd);

int initDisplayDevice(char *displays[]);
void cleanupDisplayDevice(int fd, char *displays[]);
int flipDisplayBuffers(int fd, int displayIdx);

extern int fb_show_type;

#ifdef __cplusplus
}
#endif

#endif
