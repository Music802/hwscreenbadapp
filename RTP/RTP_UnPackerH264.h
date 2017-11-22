#pragma once
#include "RTP_UnPacker.h"
#include <list>
#include <map>
#include <mutex>
namespace hwss {
#define SAVE_SEQ
#undef SAVE_SEQ
#define USE_RTP_SELF_CACHE 0
	class RTP_UnPackerH264 :
		public RTP_UnPacker
	{
	public:
		RTP_UnPackerH264(int maxRTPDelay);
		~RTP_UnPackerH264();
		virtual std::list<timedpacket> UnPackge(datapacket data) __override;
	private:
		bool ParseRTPHeader(unsigned char *data, int size,unsigned int &timestamp,unsigned char *&payloadPtr,int &payloadSize);

		std::list<datapacket> m_fu_cache;
#if USE_RTP_SELF_CACHE
		bool ParseRTPHeader(unsigned char *data, int size);
		int findNextHopeSeq();
		int m_maxRTPDelay;
		std::map<int, timedpacket> m_cachedPayload;
		int	m_nextHopeSeq = 0;
		int m_currentSeq = 0;
#endif

#ifdef SAVE_SEQ
		std::list<int> m_seqs;
		FILE *m_fp;
		FILE *m_fpLosted;
#endif // SAVE_SEQ
	};
}

