#include "rtp.h"
namespace hwss {

	bool CreateRTPHeader(int payloadType, int seq, int ssrc, unsigned char * header, unsigned int timestamp, char marker)
	{
		if (header == nullptr)
		{
			return false;
		}
		unsigned char tmp8;
		tmp8 = 0;//cc
		header[0] = (2 << 6) | tmp8;
		header[1] = (marker << 7) | payloadType;
		header[2] = (seq >> 8) & 0xff;
		header[3] = seq & 0xff;
		header[4] = (timestamp >> 24) & 0xff;
		header[5] = (timestamp >> 16) & 0xff;
		header[6] = (timestamp >> 8) & 0xff;
		header[7] = (timestamp >> 0) & 0xff;

		header[8] = (ssrc >> 24) & 0xff;
		header[9] = (ssrc >> 24) & 0xff;
		header[10] = (ssrc >> 24) & 0xff;
		header[11] = (ssrc >> 24) & 0xff;


		return true;
	}

	datapacket CreateRTPHeader(int payloadType, int seq, int ssrc, int * csrc, int csrcCount, unsigned int timestamp, char marker)
	{
		if (csrcCount>15)
		{
			csrcCount = 15;
		}
		datapacket header;
		header.size = 12 + 4 * csrcCount;
		header.data = new unsigned char[header.size];

		int cur = 0;
		header.data[cur++] = (2 << 6) | csrcCount;
		header.data[cur++] = (marker << 7) | payloadType;
		header.data[cur++] = (seq >> 8) & 0xff;
		header.data[cur++] = seq & 0xff;

		header.data[cur++] = (timestamp >> 24) & 0xff;
		header.data[cur++] = (timestamp >> 16) & 0xff;
		header.data[cur++] = (timestamp >> 8) & 0xff;
		header.data[cur++] = (timestamp >> 0) & 0xff;

		for (auto i = 0; i < csrcCount; i++)
		{
			header.data[cur++] = (csrc[i] >> 24) & 0xff;
			header.data[cur++] = (csrc[i] >> 16) & 0xff;
			header.data[cur++] = (csrc[i] >> 8) & 0xff;
			header.data[cur++] = (csrc[i] >> 0) & 0xff;
		}

		return header;
	}

}
