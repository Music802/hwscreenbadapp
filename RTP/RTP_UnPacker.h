#pragma once
#include "rtp.h"
#include <list>
namespace hwss {

	class RTP_UnPacker
	{
	public:
		static RTP_UnPacker *CreateUnPacker(RTP_PAYLOAD payload, void *params = nullptr);
		RTP_UnPacker();
		virtual ~RTP_UnPacker();
		virtual std::list<timedpacket> UnPackge(datapacket data) = 0;
	};
}

