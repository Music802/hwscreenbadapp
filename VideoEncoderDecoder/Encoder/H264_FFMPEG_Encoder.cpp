#include "H264_FFMPEG_Encoder.h"


H264_FFMPEG_Encoder::H264_FFMPEG_Encoder(int width, int height, int fps, int bitrate, H264ImgType imgType):
	m_w(width),
	m_h(height),
	m_fps(fps),
	m_bitrate(bitrate),
	m_codec(nullptr),
	m_codecCtx(nullptr),
	m_pkt264(nullptr),
	m_inited(false),
	m_sps(nullptr),
	m_pps(nullptr),
	m_frameIdx(0)
{
	switch (imgType)
	{
	case H264_IMG_NV12:
		m_pixFMT = AV_PIX_FMT_NV12;
		break;
	case H264_IMG_YUV420:
		m_pixFMT = AV_PIX_FMT_YUV420P;
		break;
	default:
		return;
		break;
	}
	m_inited = InitEncoder();
}


H264_FFMPEG_Encoder::~H264_FFMPEG_Encoder()
{
	ReleaseEncoder();
	if (m_sps!=nullptr)
	{
		delete[]m_sps;
		m_sps = nullptr;
		m_spsSize = 0;
	}
	if (m_pps!=nullptr)
	{
		delete[]m_pps;
		m_pps = nullptr;
		m_ppsSize = 0;

	}
}

bool H264_FFMPEG_Encoder::GetVideoInfo(hwss::datapacket & sps, hwss::datapacket & pps)
{
	if (m_sps==nullptr||m_pps==nullptr)
	{
		return false;
	}

	sps.size = m_spsSize;
	pps.size = m_ppsSize;
	sps.data = new unsigned char[sps.size];
	memcpy(sps.data, m_sps, sps.size);
	pps.data = new unsigned char[pps.size];
	memcpy(pps.data, m_pps, pps.size);

	return true;
}

bool H264_FFMPEG_Encoder::Encode(void * picIn, long long timestamp, std::list<hwss::timedpacket>& outFrames)
{
	if (!m_inited||picIn==nullptr)
	{
		return false;
	}
	AVFrame *frameIn = (AVFrame*)picIn;
	//frameIn->pts = m_frameIdx++;
	frameIn->pts = timestamp;

	int ret = avcodec_send_frame(m_codecCtx, frameIn);
	if (ret<0)
	{
		return false;
	}
	static int counts = 0;
	while (ret>=0)
	{
		ret = avcodec_receive_packet(m_codecCtx, m_pkt264);
		counts++;
		if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF)
		{
			return true;
		}else if (ret<0)
		{
			return false;
		}
		

		if (m_sps==nullptr||m_pps==nullptr)
		{
			ReadSPSPPS(m_pkt264->data, m_pkt264->size);
		}

		int frameLen = 0;
		char *ptr =(char*) m_pkt264->data;
		int lastedSize = m_pkt264->size;
		do
		{
			frameLen = 0;
			hwss::timedpacket packet;
			ptr=GetFrame(ptr, lastedSize, (char**)&packet.data, frameLen);
			if (frameLen>0)
			{
				packet.size = frameLen;
				packet.timestamp = m_pkt264->pts;
				outFrames.push_back(packet);
				lastedSize = m_pkt264->size - (ptr - (char*)m_pkt264->data);
			}
		} while (frameLen>0);
		if (lastedSize>0)
		{
			//last frame
			auto ptrLast = m_pkt264->data + (m_pkt264->size - lastedSize);
			int curPos = 0;
			while ((ptrLast[curPos]==1||ptrLast[curPos]==0)&&lastedSize>0)
			{
				curPos++;
				lastedSize--;
			}
			if (lastedSize>0)
			{
				hwss::timedpacket packet;
				packet.size = lastedSize;
				packet.data = new unsigned  char[packet.size];
				memcpy(packet.data, ptrLast + curPos, packet.size);
				packet.timestamp = m_pkt264->pts;
				outFrames.push_back(packet);
			}
		}

		//qDebug() << m_pkt264->pts << "     " << m_pkt264->dts;
		
		av_packet_unref(m_pkt264);
	}
	return true;
}

bool H264_FFMPEG_Encoder::InitEncoder()
{
	bool result = false;
	do
	{
		m_codec= avcodec_find_encoder(AVCodecID::AV_CODEC_ID_H264);
		//m_codec = avcodec_find_encoder_by_name("libx264");
		if (m_codec==nullptr)
		{
			break;
		}
		m_codecCtx = avcodec_alloc_context3(m_codec);
		if (m_codecCtx==nullptr)
		{
			break;
		}
		m_codecCtx->bit_rate = m_bitrate;
		m_codecCtx->width = m_w;
		m_codecCtx->height = m_h;

		m_codecCtx->time_base.den = m_fps;
		m_codecCtx->time_base.num = 1;
		m_codecCtx->framerate.den = 1;
		m_codecCtx->framerate.den = 1000;

		m_codecCtx->me_range = 16;
		m_codecCtx->max_qdiff = 4;
		m_codecCtx->gop_size = 2;
		m_codecCtx->qmin = 10;
		m_codecCtx->qmax = 30;
		m_codecCtx->max_b_frames = 0;
		m_codecCtx->qcompress = 1;

		m_codecCtx->noise_reduction = 100;

		m_codecCtx->pix_fmt = m_pixFMT;

		av_opt_set(m_codecCtx->priv_data, "preset", "veryfast", 0);
		//for delay
		av_opt_set(m_codecCtx->priv_data, "tune", "zerolatency", 0);

		int ret = avcodec_open2(m_codecCtx, m_codec, 0);
		if (ret!=0)
		{
			break;
		}

		m_pkt264 = av_packet_alloc();

		result = true;
		m_frameIdx = 0;
	} while (0);
	return result;
}

void H264_FFMPEG_Encoder::ReleaseEncoder()
{
	avcodec_free_context(&m_codecCtx);
	m_codecCtx = nullptr;
	m_codec = nullptr;
	m_inited = false;
	av_packet_free(&m_pkt264);
	m_frameIdx = 0;
}

void H264_FFMPEG_Encoder::ReadSPSPPS(unsigned char * data, int size)
{
	//aways first two frame
	char *ptr_end = GetFrame((char*)data, size, (char**)&m_sps, m_spsSize);
	if (ptr_end == 0 || !m_sps || m_spsSize <= 0) {
		return;
	}
	if ((m_sps[0] & 0x1f) != 7) {
		delete[]m_sps;
		m_sps = nullptr;
		return;
	}
	GetFrame(ptr_end, size - (ptr_end - (char*)data), (char**)&m_pps, m_ppsSize);
	if (!m_pps||m_ppsSize<=0)
	{
		delete[]m_sps;
		m_sps = nullptr;
		return;
	}
	if ((m_pps[0] & 0x1f) != 8)
	{
		delete[]m_pps;
		m_pps = nullptr;
	}
}

char* H264_FFMPEG_Encoder::GetFrame(char * ptr_data, int buf_size, char ** ptr_frame, int & frame_len)
{
	int iRet = 0;
	int nal_start = 0;
	int nal_end = 0;
	bool	get_nal = false;
	while (true)
	{
		if (nal_start + 4 >= buf_size)
		{
			return 0;
		}
		if (ptr_data[nal_start] == 0x00 &&
			ptr_data[nal_start + 1] == 0x00)
		{
			if (ptr_data[nal_start + 2] == 0x01)
			{
				nal_start += 3;
				get_nal = true;
				break;
			}
			else if (ptr_data[nal_start + 2] == 0x00)
			{
				if (ptr_data[nal_start + 3] == 0x01)
				{
					nal_start += 4;
					get_nal = true;
					break;
				}
			}
			else
			{
				nal_start += 2;
			}
		}
		nal_start++;
	}
	if (!get_nal)
	{
		return	0;
	}
	get_nal = false;
	nal_end = nal_start;
	while (nal_end<buf_size)
	{
		if (nal_end + 4 >= buf_size)
		{
			nal_end = buf_size;
			break;
		}
		if (ptr_data[nal_end] == 0x00 &&
			ptr_data[nal_end + 1] == 0x00)
		{
			if (ptr_data[nal_end + 2] == 0x01)
			{
				get_nal = true;
				break;
			}
			else if (ptr_data[nal_end + 2] == 0x00)
			{
				if (ptr_data[nal_end + 3] == 0x01)
				{
					get_nal = true;
					break;
				}
			}
			else
			{
				nal_end += 2;
			}
		}
		nal_end++;
	}
	if (!get_nal)
	{
		return	0;
	}

	char *ptr_end = ptr_data + nal_end;
	frame_len = nal_end - nal_start;

	if (!ptr_frame)
	{
		return 0;
	}
	*ptr_frame = new char[frame_len];
	memcpy(*ptr_frame, ptr_data + nal_start, frame_len);

	return ptr_end;
}
