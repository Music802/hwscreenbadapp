#include "ffcapture.h"
#include <iostream>
#include "../CommonUtils/datapacket.h"


void ffcapture::Init()
{
	av_register_all();
	avdevice_register_all();
	return ;
}

ffcapture::ffcapture():m_fmtCtx(nullptr),
m_options(nullptr),
m_inputFmt(nullptr),
m_sws(nullptr),
m_encoder(nullptr)
{
	m_pktDesktop = (AVPacket*)av_malloc(sizeof(AVPacket));
	m_frameSrc = av_frame_alloc();
	m_frameYUV = av_frame_alloc();
#ifdef SAVE_H264
	m_fp = fopen("desktop.h264","wb");
#endif // SAVE_H264

}


ffcapture::~ffcapture()
{
	ReleaseCapturer();

	ReleaseCaler();

	
	if (m_encoder!=nullptr)
	{
		delete m_encoder;
		m_encoder = nullptr;
	}
#ifdef SAVE_H264
	if (m_fp)
	{
		fclose(m_fp);
		m_fp = nullptr;
	}
#endif // SAVE_H264

}

bool ffcapture::SetParams(int x, int y, int w, int h, int fps,int dstW,int dstH)
{
	
	if (!CreateCapturer(x,y,w,h,fps,dstW,dstH))
	{
		return false;
	}

	CreateScaler();

	H264ImgType imgtype;
	switch (m_yuvFMT)
	{
	case AV_PIX_FMT_YUV420P:
		imgtype = H264_IMG_YUV420;
		break;
	case AV_PIX_FMT_NV12:
		imgtype = H264_IMG_NV12;
		break;
	default:
		return false;
		break;
	}
	m_encoder = H264Encoder::CreateEncoder(H264_Encoder_FFMPEG,
		m_dstWidth, m_dstHeight, m_fps,
		m_dstWidth*m_dstHeight,imgtype);

	if (m_encoder==nullptr)
	{
		return false;
	}
	auto time_now = std::chrono::system_clock::now();
	m_lastTime = std::chrono::duration_cast<std::chrono::milliseconds>(time_now.time_since_epoch());
	return true;
}

int ffcapture::GetNextFrame(std::list<hwss::timedpacket> &outFrames)
{
	//get rgb,
	av_read_frame(m_fmtCtx, m_pktDesktop);
	if (m_pktDesktop->stream_index==m_videChannel)
	{
		int got;
		auto result = avcodec_decode_video2(m_codecCtx,m_frameSrc , &got, m_pktDesktop);
		if (result<=0)
		{
			return result;
		}
	}
	
	av_packet_unref(m_pktDesktop);
	//swscale to yuv
	auto result=sws_scale(m_sws, (const unsigned char* const*)m_frameSrc->data, m_frameSrc->linesize,
		0, m_codecCtx->height,
		m_frameYUV->data, m_frameYUV->linesize);
	if (result<=0)
	{
		return -1;
	}

	//encode
	auto time_now = std::chrono::system_clock::now();
	auto duration_in_ms = std::chrono::duration_cast<std::chrono::milliseconds>(time_now.time_since_epoch());
	std::cout << duration_in_ms.count() - m_lastTime.count() << std::endl;
	m_frameYUV->pts = int64_t(duration_in_ms.count() - m_lastTime.count());
	//m_lastTime = duration_in_ms;
	//m_frameYUV->pts = 0;
	//std::list<hwss::datapacket> frameOut;
	m_encoder->Encode(m_frameYUV, m_frameYUV->pts, outFrames);
	for (auto i : outFrames)
	{
#ifdef SAVE_H264
		char tmp[4];
		tmp[0] = 0;
		tmp[1] = 0;
		tmp[2] = 0;
		tmp[3] = 1;
		fwrite(tmp, 4, 1, m_fp);
		fwrite( i.data, i.size, 1, m_fp);
#endif
	}

	
	return 0;
}

bool ffcapture::GetSPSPPS(hwss::datapacket & sps, hwss::datapacket & pps)
{
	return m_encoder->GetVideoInfo(sps, pps);
}

bool ffcapture::CreateCapturer(int x, int y, int w, int h, int fps, int dstW , int dstH )
{
	m_x = x;
	m_y = y;
	m_w = w;
	m_h = h;
	m_fps = fps;
	if (m_fps<1 || m_w<1 || m_h<1)
	{
		return false;
	}

	m_dstWidth = dstW;
	m_dstHeight = dstH;

	if (dstW<1)
	{
		m_dstWidth = w;
	}
	if (dstH<1)
	{
		m_dstHeight = h;
	}



	int ret = av_dict_set(&m_options, "list_devices", "true", 0);
	char pNumber[20];
	itoa(m_x, pNumber, 10);
	ret = av_dict_set(&m_options, "offset_x", pNumber, 0);
	itoa(m_y, pNumber, 10);
	ret = av_dict_set(&m_options, "offset_y", pNumber, 0);
	itoa(m_fps, pNumber, 10);
	ret = av_dict_set(&m_options, "framerate", pNumber, 0);
	sprintf(pNumber, "%d*%d", m_w, m_h);
	ret = av_dict_set(&m_options, "video_size", pNumber, 0);
	//"gdigrab" for windows
	//"avfoundation" for mac
	//"x11grab" for linux
	m_inputFmt = av_find_input_format("gdigrab");
	if (m_inputFmt == nullptr)
	{
		return false;
	}

	ret = avformat_open_input(&m_fmtCtx, "desktop", m_inputFmt, &m_options);
	if (ret != 0)
	{
		return false;
	}

	//find video
	ret = avformat_find_stream_info(m_fmtCtx, 0);
	if (ret != 0)
	{
		return false;
	}
	int vide_channel = -1;
	for (int i = 0; i < m_fmtCtx->nb_streams; i++)
	{
		if (AVMEDIA_TYPE_VIDEO == m_fmtCtx->streams[i]->codec->codec_type)
		{
			vide_channel = i;
			break;
		}
	}

	if (vide_channel == -1)
	{
		return false;
	}


	m_codecCtx = m_fmtCtx->streams[vide_channel]->codec;
	m_codec = avcodec_find_decoder(m_codecCtx->codec_id);
	if (nullptr == m_codec)
	{
		return false;
	}
	if (avcodec_open2(m_codecCtx, m_codec, nullptr)<0)
	{
		return  false;
	}

	m_videChannel = vide_channel;

	return true;
}

void ffcapture::ReleaseCapturer()
{
	if (m_fmtCtx != nullptr)
	{
		//it free all contents
		avformat_close_input(&m_fmtCtx);
		m_fmtCtx = nullptr;
		m_options = nullptr;
		m_inputFmt = nullptr;
	}

	if (m_pktDesktop != nullptr)
	{
		av_free(m_pktDesktop);
		m_pktDesktop = nullptr;
	}

	if (m_frameSrc)
	{
		av_frame_free(&m_frameSrc);
		m_frameSrc = nullptr;
	}

}

void ffcapture::CreateScaler()
{
	m_yuvFMT = AV_PIX_FMT_YUV420P;
	int sws_flags = SWS_POINT;
	if (m_dstWidth*m_dstHeight*1.2<m_codecCtx->width*m_codecCtx->height)
	{
		sws_flags = SWS_BILINEAR;
	}
	if (m_dstWidth*m_dstHeight*1.4<m_codecCtx->width*m_codecCtx->height)
	{
		sws_flags = SWS_BICUBIC;
	}
	m_sws = sws_getContext(m_codecCtx->width, m_codecCtx->height,
		m_codecCtx->pix_fmt,
		m_dstWidth, m_dstHeight, m_yuvFMT,
		sws_flags, 0, 0, 0);

	if (m_frameYUV->data[0] != nullptr)
	{
		delete[]m_frameYUV->data[0];
	}
	if (m_frameYUV->data[1] != nullptr)
	{
		delete[]m_frameYUV->data[1];
	}
	av_freep(&m_frameYUV->data[0]);
	av_image_alloc(m_frameYUV->data, m_frameYUV->linesize,
		m_dstWidth, m_dstHeight, m_yuvFMT, 16);
	m_frameYUV->width = m_dstWidth;
	m_frameYUV->height = m_dstHeight;
	m_frameYUV->format = m_yuvFMT;
}

void ffcapture::ReleaseCaler()
{
	if (m_sws)
	{
		sws_freeContext(m_sws);
		m_sws = nullptr;
	}

	if (m_frameYUV)
	{
		av_frame_free(&m_frameYUV);
		m_frameYUV = nullptr;
	}

}
