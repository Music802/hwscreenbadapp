#pragma once

#include "CommonUtils/datapacket.h"
namespace hwss {

	//rtp packet type
	const int NAL_TYPE_SINGLE_NAL_MIN = 1;
	const int NAL_TYPE_SINGLE_NAL_MAX = 23;
	const int NAL_TYPE_STAP_A = 24;
	const int NAL_TYPE_STAP_B = 25;
	const int NAL_TYPE_MTAP16 = 26;
	const int NAL_TYPE_MTAP24 = 27;
	const int NAL_TYPE_FU_A = 28;
	const int NAL_TYPE_FU_B = 29;
	enum RTP_PAYLOAD
	{
		Payload_h264 = 96,
		Payload_MPA=14
	};
	//const int Payload_h264 = 96;
	//const int Payload_MPA = 14; //mp3 freq 90000
	const int RTP_H264_freq = 90000;

	//#define RTP_MTU 1500
	const int RTP_MTU = 50000;

	//no csrc,const size 12 byte
	bool CreateRTPHeader(int payloadType, int seq, int ssrc, unsigned char *header, unsigned int timestamp, char marker = 0);
	//csrc ,dynamic size
	datapacket CreateRTPHeader(int payloadType, int seq, int ssrc, int *csrc, int csrcCount, unsigned int timestamp, char marker = 0);

}
