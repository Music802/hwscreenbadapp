#include "ScreenRenderer.h"
#include "Config.h"

ScreenRenderer::ScreenRenderer(QObject *parent)
	: QObject(parent),
	m_width(0),
	m_height(0),
	m_sps(nullptr),
	m_spsLength(0),
	m_pps(nullptr),
	m_ppsLength(0),
	m_rtpUnPacker(nullptr),
	m_decoder(nullptr),
	m_sdlWidgt(nullptr),
	m_x(0),
	m_y(0)
{
#ifdef SAVE_H264
	m_fp = fopen("recved.h264", "wb");
#endif // SAVE_H264

}

ScreenRenderer::~ScreenRenderer()
{
	if (m_sps!=nullptr)
	{
		delete[]m_sps;
		m_sps = nullptr;
	}
	if (m_pps)
	{
		delete[]m_pps;
		m_pps = nullptr;
	}

	ReleaseRTPUnpacker();
	ReleaseDecoder();
	ReleaseSDL();

#ifdef SAVE_H264
	if (m_fp!=nullptr)
	{
		fclose(m_fp);
		m_fp = nullptr;
	}
#endif // SAVE_H264

}

void ScreenRenderer::SetVideoInfo(int width, int height, QString sps64, QString pps64)
{
	m_width = width;
	m_height = height;
	QByteArray ba;
	ba.append(sps64);
	ba = QByteArray::fromBase64(ba);
	m_spsLength = ba.size();
	m_sps = new unsigned char[m_spsLength];
	memcpy(m_sps, ba.data(), m_spsLength);

	ba.clear();
	ba.append(pps64);
	ba = QByteArray::fromBase64(ba);
	m_ppsLength = ba.size();
	m_pps = new unsigned char[m_ppsLength];
	memcpy(m_pps, ba.data(), m_ppsLength);

	int rtpCacheSize = Config::GetInstance().RTPMaxDelaySeq();
	m_rtpUnPacker = hwss::RTP_UnPacker::CreateUnPacker(hwss::Payload_h264, &rtpCacheSize);
	m_decoder = H264Decoder::CreateNewDecoder(H264_Decoder_FFMPEG);
	{
		//append sps pps
		hwss::timedpacket timedSps, timedPps;
		timedSps.data = m_sps;
		timedSps.size = m_spsLength;
		timedSps.timestamp = 0;

		timedPps.data = m_pps;
		timedPps.size = m_ppsLength;
		timedPps.timestamp = 0;

		m_decoder->Decode(timedSps);
		m_decoder->Decode(timedPps);
	}

	m_sdlWidgt = new SDLWidget(m_x, m_y, m_width, m_height, m_width, m_height);
#if SAVE_H264
	char sep[] = { 0,0,0,1 };
	fwrite(sep, 1, 4, m_fp);
	fwrite(m_sps, 1, m_spsLength, m_fp);
	fwrite(sep, 1, 4, m_fp);
	fwrite(m_pps, 1, m_ppsLength, m_fp);
#endif // SAVE_H264

}

void ScreenRenderer::SetLocation(int x, int y,int w,int h)
{
	m_x = x;
	m_y = y;
	if (m_sdlWidgt!=nullptr)
	{
		m_sdlWidgt->SetWindowLocation(x, y,w,h);
	}
}

void ScreenRenderer::ReleaseRTPUnpacker()
{
	if (m_rtpUnPacker!=nullptr)
	{
		delete m_rtpUnPacker;
		m_rtpUnPacker = nullptr;
	}
}

void ScreenRenderer::ReleaseDecoder()
{
	if (m_decoder!=nullptr)
	{
		delete m_decoder;
		m_decoder = nullptr;
	}
}

void ScreenRenderer::ReleaseSDL()
{
	if (m_sdlWidgt!=nullptr)
	{
		delete m_sdlWidgt;
		m_sdlWidgt = nullptr;
	}
}

void ScreenRenderer::OnNewDatagram(const char *buf, int len) {
	//generate h264 frame
	if (m_rtpUnPacker==nullptr||buf==nullptr||len<=0)
	{
		return;
	}

	hwss::datapacket rtpPacket;
	rtpPacket.size = len;
	rtpPacket.data = new unsigned char[rtpPacket.size];
	memcpy(rtpPacket.data, buf, rtpPacket.size);
	auto h264Frames = m_rtpUnPacker->UnPackge(rtpPacket);

	for (auto i:h264Frames )
	{
#if SAVE_H264
		char sep[] = { 0,0,0,1 };
		fwrite(sep, 1, 4, m_fp);
		fwrite(i.data, i.size, 1, m_fp);
#endif // SAVE_H264
		auto frames = m_decoder->Decode(i);
		for (auto img:frames)
		{
			//rend img
			if (m_sdlWidgt!=nullptr)
			{
				m_sdlWidgt->UpdateYUV(img);
			}
			//release
			continue;
		}
	}

	//clear
	delete[]rtpPacket.data;
	for (auto i : h264Frames)
	{
		delete[]i.data;
	}
}
