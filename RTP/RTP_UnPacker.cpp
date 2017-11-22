#include "RTP_UnPacker.h"
#include "RTP_UnPackerH264.h"


namespace hwss {
	RTP_UnPacker * RTP_UnPacker::CreateUnPacker(RTP_PAYLOAD payload, void * params)
	{
		RTP_UnPacker *out = nullptr;
		switch (payload)
		{
		case hwss::Payload_h264:
			out = new RTP_UnPackerH264(*((int*)params));
			break;
		case hwss::Payload_MPA:
			break;
		default:
			break;
		}
		return out;
	}
	RTP_UnPacker::RTP_UnPacker()
	{
	}


	RTP_UnPacker::~RTP_UnPacker()
	{
	}
}
