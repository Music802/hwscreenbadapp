#pragma once
namespace hwss {
#ifndef BYTE_ORDER

#define LITTLE_ENDIAN	1234
#define BIG_ENDIAN	4321

#if defined(sun) || defined(__BIG_ENDIAN) || defined(NET_ENDIAN)
#define BYTE_ORDER	BIG_ENDIAN
#else
#define BYTE_ORDER	LITTLE_ENDIAN
#endif
#endif
	struct datapacket
	{
		int size;
		unsigned char* data;
	};

	struct timedpacket
	{
		int size;
		unsigned char* data;
		long long timestamp;
	};
}
