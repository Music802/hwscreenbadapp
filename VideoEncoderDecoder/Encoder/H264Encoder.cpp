#include "H264Encoder.h"

#include "H264_FFMPEG_Encoder.h"

H264Encoder::H264Encoder()
{
}


H264Encoder * H264Encoder::CreateEncoder(H264EncoderType encoderType, int width, int height, int fps, int bitrate, H264ImgType imgType)
{
	H264Encoder *encoder = nullptr;
	switch (encoderType)
	{
	case H264_Encoder_FFMPEG:
		encoder = reinterpret_cast<H264Encoder*>(new H264_FFMPEG_Encoder(width, height, fps, bitrate, imgType));
		break;
	case H264_Encoder_X264:
		break;
	default:
		break;
	}
	return encoder;
}

H264Encoder::~H264Encoder()
{
}
