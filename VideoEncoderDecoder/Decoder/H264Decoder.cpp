#include "H264Decoder.h"
#include "H264_FFMPEG_Decoder.h"


H264Decoder * H264Decoder::CreateNewDecoder(H264DecoderType decoderType)
{
	H264Decoder *decoder = nullptr;
	switch (decoderType)
	{
	case H264_Decoder_FFMPEG:
		decoder = new H264_FFMPEG_Decoder();
		break;
	default:
		break;
	}
	return decoder;
}

H264Decoder::H264Decoder()
{
}


H264Decoder::~H264Decoder()
{
}

H264_IMG_Data::H264_IMG_Data()
{
	for (int i = 0; i < H264_IMG_DATA_POINTER_NUM; i++)
	{
		data[i] = nullptr;
		lineSize[i] = 0;
	}
}

H264_IMG_Data::~H264_IMG_Data()
{
	for (int i = 0; i < H264_IMG_DATA_POINTER_NUM; i++)
	{
		if (data[i]!=nullptr)
		{
			delete[]data[i];
			data[i] = nullptr;
		}
	}
}
