#pragma once
#include <memory>
#include <list>
#include "../ffmpeg_header.h"
#include "../../CommonUtils/datapacket.h"


class H264Decoder
{
public:
	static H264Decoder *CreateNewDecoder(H264DecoderType decoderType);
	virtual std::list<std::shared_ptr<H264_IMG_Data>> Decode(hwss::timedpacket frame) = 0;
	H264Decoder();
	virtual ~H264Decoder();
};
