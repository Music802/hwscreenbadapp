#include "H264_FFMPEG_Decoder.h"



H264_FFMPEG_Decoder::H264_FFMPEG_Decoder():
	m_codec(nullptr),
	m_codecCtx(nullptr),
	m_codecParserCtx(nullptr),
	m_packet(nullptr),
	m_frame(nullptr)
{
	InitDecoder();
}


H264_FFMPEG_Decoder::~H264_FFMPEG_Decoder()
{
	if (m_codecParserCtx)
	{
		av_parser_close(m_codecParserCtx);
		m_codecParserCtx = nullptr;
	}
	if (m_codecCtx)
	{
		avcodec_free_context(&m_codecCtx);
		m_codecCtx = nullptr;
		m_codec = nullptr;
	}
	if (m_packet)
	{
		av_packet_free(&m_packet);
		m_packet = nullptr;
	}
	if (m_frame)
	{
		av_frame_free(&m_frame);
		m_frame = nullptr;
	}
}

std::list<std::shared_ptr<H264_IMG_Data>> H264_FFMPEG_Decoder::Decode(hwss::timedpacket frame)
{
	//may need 0x 00 00 01
	std::list<std::shared_ptr<H264_IMG_Data>> out;
	int ret = 0, cur = 0;
	unsigned char sep[] = { 0,0,0,1 };
	while (cur<4)
	{
		ret = av_parser_parse2(m_codecParserCtx, m_codecCtx,
			&m_packet->data, &m_packet->size,
			sep, 4,
			AV_NOPTS_VALUE, AV_NOPTS_VALUE, 0);
		if (ret<0)
		{
			return out;
		}
		cur += ret;
	}

	cur = 0;
	while (cur<frame.size)
	{
		ret = av_parser_parse2(m_codecParserCtx, m_codecCtx,
			&m_packet->data, &m_packet->size,
			frame.data + cur, frame.size - cur,
			AV_NOPTS_VALUE, AV_NOPTS_VALUE, 0);
		if (ret<0)
		{
			break;
		}
		cur += ret;

		if (m_packet->size>0)
		{
			if ( avcodec_send_packet(m_codecCtx, m_packet)>=0) {
				auto dataDecoded = decodeFrame();
					for (auto i: dataDecoded)
					{
						out.push_back(i);
					}
			}
		}
		av_packet_unref(m_packet);
	}
	


		
	return out;
}

bool H264_FFMPEG_Decoder::InitDecoder()
{
	bool result = false;
	do
	{
		m_codec = avcodec_find_decoder(AV_CODEC_ID_H264);
		if (m_codec==nullptr)
		{
			break;
		}
		m_codecParserCtx = av_parser_init(m_codec->id);
		if (m_codecParserCtx==nullptr)
		{
			break;
		}
		m_codecCtx = avcodec_alloc_context3(m_codec);
		if (!m_codecCtx)
		{
			break;
		}

		if (avcodec_open2(m_codecCtx, m_codec, nullptr) < 0) {
			break;
		}

		m_packet = av_packet_alloc();
		m_frame = av_frame_alloc();
		result = true;
	} while (0);
	return result;
}

std::list<std::shared_ptr<H264_IMG_Data>> H264_FFMPEG_Decoder::decodeFrame()
{
	std::list<std::shared_ptr<H264_IMG_Data>> out;
	int ret = 0;
	while (ret==0)
	{
		ret = avcodec_receive_frame(m_codecCtx, m_frame);
		if (ret!=0)
		{
			break;
		}
		std::shared_ptr<H264_IMG_Data> ptr(new H264_IMG_Data);
		for (int i = 0; i < 3; i++)
		{
			if (m_frame->linesize[i]>0)
			{
				ptr->lineSize[i] = m_frame->linesize[i];
				//int bufSize = m_frame->linesize[i] * m_frame->height;
				//ptr->data[i] = new unsigned char[bufSize];
				//memcpy(ptr->data[i], m_frame->data[i], bufSize);
				ptr->data[i] = new unsigned char[m_frame->buf[i]->size];
				memcpy(ptr->data[i], m_frame->buf[i]->data, m_frame->buf[i]->size);
			}
		}
		av_frame_unref(m_frame);
		out.push_back(ptr);
	}
	return out;
}
