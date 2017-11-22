#pragma once

#include "rtp.h"
#include <list>
#include <memory>

namespace hwss {

	struct RTPPacket
	{
		RTPPacket();
		unsigned char data[RTP_MTU];
		int size;
	};

	class RTP_Packer
	{
	public:
		RTP_Packer(int payloadType,int ssrc);
		~RTP_Packer();
		//one frame
		std::list<std::shared_ptr<RTPPacket>> GenerateH264RTPPackets(unsigned char* framedata, int frameSize, unsigned int timestamp, bool usetcp = false);

	private:
		int m_iPayloadType;
		int m_iSSRC;
		int m_iSeq;
	};
}

