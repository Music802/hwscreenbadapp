
#include "RTP_Packer.h"
namespace hwss {


	RTP_Packer::RTP_Packer(int payloadType, int ssrc):
		m_iPayloadType(payloadType),m_iSSRC(ssrc), m_iSeq(0)
	{
	}

	RTP_Packer::~RTP_Packer()
	{
	}

	std::list<std::shared_ptr<RTPPacket>> RTP_Packer::GenerateH264RTPPackets(unsigned char * framedata, int frameSize, unsigned int timestamp, bool usetcp)
	{
		auto payloadSize = RTP_MTU;
		if (usetcp)
		{
			payloadSize -= 4;
		}

		std::list<std::shared_ptr<RTPPacket>> pkgs;

		payloadSize -= 13; //12 rtp header,1 f nri type
		auto nalType = framedata[0] & 0xf;
		if (frameSize<payloadSize)
		{
			m_iSeq++;
			if (m_iSeq>0xffff)
			{
				m_iSeq = 0;
			}
			std::shared_ptr<RTPPacket> pkg(new RTPPacket);
			pkg->size = 12 + frameSize;
			CreateRTPHeader(m_iPayloadType, m_iSeq, m_iSSRC, pkg->data, timestamp);
			memcpy(pkg->data + 12, framedata, frameSize);
			pkgs.push_back(pkg);
		}
		else {
			payloadSize -= 1;
			unsigned char FU_S, FU_E, FU_R, FU_Type, Nri, Type;
			Nri = (framedata[0] & 0x60) >> 5;
			Type = NAL_TYPE_FU_A;
			FU_Type = nalType;
			auto count = frameSize / payloadSize;
			if (count*payloadSize<frameSize)
			{
				count++;
			}
			auto curFrame = 0;
			auto curNalData = 1; //nal 的第一个字节的帧类型信息放到fh_header 里面
			{
				FU_S = 1;
				FU_E = 0;
				FU_R = 0;
				m_iSeq++;
				if (m_iSeq>0xffff)
				{
					m_iSeq = 0;
				}
				std::shared_ptr<RTPPacket> pkg(new RTPPacket);
				pkg->size = payloadSize + 12 + 2;
				CreateRTPHeader(m_iPayloadType, m_iSeq, m_iSSRC, pkg->data, timestamp);
				pkg->data[12] = (Nri << 5) | Type;
				pkg->data[13] = (FU_S << 7) | (FU_E << 6) | (FU_R << 5) | FU_Type;
				memcpy(pkg->data+14, framedata+ curNalData, payloadSize);
				curFrame++;
				curNalData += payloadSize;

				pkgs.push_back(pkg);
			}

			//mid
			{
				FU_S = 0;
				FU_E = 0;
				FU_R = 0;
				for (auto i = curFrame; i +1< count; i++)
				{
					m_iSeq++;
					if (m_iSeq>0xffff)
					{
						m_iSeq = 0;
					}
					std::shared_ptr<RTPPacket> pkg(new RTPPacket);
					pkg->size = payloadSize + 12 + 2;
					CreateRTPHeader(m_iPayloadType, m_iSeq, m_iSSRC, pkg->data, timestamp);
					pkg->data[12] = (Nri << 5) | Type;
					pkg->data[13] = (FU_S << 7) | (FU_E << 6) | (FU_R << 5) | FU_Type;
					memcpy(pkg->data + 14, framedata + curNalData, payloadSize);
					curFrame++;
					curNalData += payloadSize;

					pkgs.push_back(pkg);
				}
			}
			//last frame
			auto lastFrameSize = frameSize - curNalData;
			if (lastFrameSize>0)
			{
				FU_S = 0;
				FU_E = 1;
				FU_R = 0;
				m_iSeq++;
				if (m_iSeq>0xffff)
				{
					m_iSeq = 0;
				}

				std::shared_ptr<RTPPacket> pkg(new RTPPacket);
				pkg->size = lastFrameSize + 12 + 2;
				CreateRTPHeader(m_iPayloadType, m_iSeq, m_iSSRC, pkg->data, timestamp);
				pkg->data[12] = (Nri << 5) | Type;
				pkg->data[13] = (FU_S << 7) | (FU_E << 6) | (FU_R << 5) | FU_Type;

				memcpy(pkg->data + 14, framedata + curNalData, lastFrameSize);

				curNalData += lastFrameSize;
				curFrame++;

				pkgs.push_back(pkg);
			}
		}

		return pkgs;
	}


	RTPPacket::RTPPacket():size(0)
	{
	}
}


