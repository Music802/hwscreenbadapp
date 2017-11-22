#pragma once
extern "C"
{
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libswscale/swscale.h>
#include <libavdevice/avdevice.h>
#include <libavutil/opt.h>
#include <libavutil/imgutils.h>
}

#pragma comment(lib,"avcodec.lib")
#pragma comment(lib,"avdevice.lib")
#pragma comment(lib,"avformat.lib")
#pragma comment(lib,"swscale.lib")
#pragma comment(lib,"avutil.lib")


enum H264ImgType
{
	H264_IMG_YUV420,
	H264_IMG_NV12
};


enum H264DecoderType
{
	H264_Decoder_FFMPEG
};
static const int H264_IMG_DATA_POINTER_NUM = 8;
struct H264_IMG_Data
{
	H264_IMG_Data();
	~H264_IMG_Data();
	H264ImgType imgType;
	unsigned char *data[H264_IMG_DATA_POINTER_NUM];
	int lineSize[H264_IMG_DATA_POINTER_NUM];
};


void FFMPEG_Init();
