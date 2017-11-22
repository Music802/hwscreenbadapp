#include "RTP_UnPackerH264.h"
#include "CommonUtils/BitReader.h"

namespace hwss {

	RTP_UnPackerH264::RTP_UnPackerH264(int maxRTPDelay)

	{
#if USE_RTP_SELF_CACHE
		m_maxRTPDelay = maxRTPDelay;
#ifdef SAVE_SEQ
		m_fp = (nullptr);
		m_fpLosted = nullptr;
#endif // SAVE_SEQ

#endif // USE_RTP_SELF_CACHE


	}


	RTP_UnPackerH264::~RTP_UnPackerH264()
	{
		if (m_fu_cache.size() > 0)
		{
			for (auto i : m_fu_cache)
			{
				delete[]i.data;
			}
			m_fu_cache.clear();
		}
#if USE_RTP_SELF_CACHE
		for (auto i : m_cachedPayload)
		{
			delete[]i.second.data;
		}
		m_cachedPayload.clear();
#ifdef SAVE_SEQ	
		if (m_fp)
		{
			fclose(m_fp);
			m_fp = nullptr;
		}
		if (m_fpLosted)
		{
			fclose(m_fpLosted);
			m_fpLosted = nullptr;
		}
#endif //SAVE_SEQ

#endif // USE_RTP_SELF_CACJE


	}
	std::list<timedpacket> RTP_UnPackerH264::UnPackge(datapacket data)
	{
		unsigned int timestamp;
		unsigned char *payloadPtr;
		int payloadSize;
		std::list<timedpacket> out;


#if USE_RTP_SELF_CACHE
		auto result = ParseRTPHeader(data.data, data.size);
		if (!result)
		{
			//invlaid rtp packet
			return out;
		}

		auto it = m_cachedPayload.find(m_currentSeq);
		//not found
		if (it == m_cachedPayload.end())
		{
			if (m_cachedPayload.size() < m_maxRTPDelay)
			{
				return out;
			}
			else {
				m_currentSeq = findNextHopeSeq();
				it = m_cachedPayload.find(m_currentSeq);
			}
			return out;
		}


		while (result&&it != m_cachedPayload.end()) {


			result = false;
			payloadPtr = it->second.data;
			payloadSize = it->second.size;
			timestamp = it->second.timestamp & 0xffffffff;
			do
			{
				CBitReader bitReader;
				bitReader.SetBitReader(payloadPtr, payloadSize, true);
				int nal_forbidden_zero = bitReader.ReadBit();
				int nal_nri = bitReader.ReadBits(2);
				int nal_type = bitReader.ReadBits(5);
				if (nal_forbidden_zero != 0)
				{
					break;
				}

				if (nal_type >= NAL_TYPE_SINGLE_NAL_MIN&&
					nal_type <= NAL_TYPE_SINGLE_NAL_MAX)
				{
					timedpacket pkt;
					pkt.timestamp = timestamp;
					pkt.size = payloadSize;
					pkt.data = new unsigned char[pkt.size];
					memcpy(pkt.data, payloadPtr, pkt.size);
					out.push_back(pkt);
				}
				else if (nal_type == NAL_TYPE_STAP_A || nal_type == NAL_TYPE_STAP_B)
				{
					int idx = 1;
					int don = 0;
					int nalu_size = 0;
					if (nal_type == NAL_TYPE_STAP_B)
					{
						don = (int(payloadPtr[idx]) << 8) | (int(payloadPtr[idx + 1]));
						idx += 2;
					}
					while (idx < payloadSize)
					{
						nalu_size = (int(payloadPtr[idx]) << 8) | (int(payloadPtr[idx + 1]));
						idx += 2;
						if (nalu_size <= 0)
						{
							continue;
						}
						if (nalu_size + idx > payloadSize)
						{
							break;
						}
						timedpacket pkt;
						pkt.timestamp = timestamp;
						pkt.size = nalu_size;
						pkt.data = new unsigned char[nalu_size];
						memcpy(pkt.data, payloadPtr + idx, nalu_size);
						idx += nalu_size;
						out.push_back(pkt);
					}
				}

				else if (nal_type == NAL_TYPE_MTAP16 || nal_type == NAL_TYPE_MTAP24)
				{
					int idx = 1;
					int dond = 0;
					int dob = 0;
					int nalu_size = 0;
					int time_offset = 0;
					dob = (int(payloadPtr[idx]) << 8) | (int(payloadPtr[idx + 1]));
					idx += 2;
					while (idx < payloadSize)
					{
						if (nal_type == NAL_TYPE_MTAP16)
						{
							if (idx + 5 < payloadSize)
							{
								break;
							}
						}
						else
						{
							if (idx + 6 < payloadSize)
							{
								break;
							}
						}
						nalu_size = (int(payloadPtr[idx]) << 8) | (int(payloadPtr[idx + 1]));
						idx += 2;
						dond = payloadPtr[idx];
						idx++;
						if (nal_type == NAL_TYPE_MTAP16)
						{
							time_offset = (int(payloadPtr[idx]) << 8) | (int(payloadPtr[idx + 1]));
							idx += 2;
						}
						else {
							time_offset = (int(payloadPtr[idx]) << 16) | (int(payloadPtr[idx]) << 8) | (int(payloadPtr[idx]) << 0);
							idx += 3;
						}
						if (idx + nalu_size < payloadSize)
						{
							break;
						}
						timedpacket pkt;
						pkt.size = nalu_size;
						pkt.data = new unsigned char[nalu_size];
						memcpy(pkt.data, payloadPtr + idx, pkt.size);
						pkt.timestamp = timestamp + time_offset;
						out.push_back(pkt);
						idx += nalu_size;
					}

				}
				else if (nal_type == NAL_TYPE_FU_A || nal_type == NAL_TYPE_FU_B)
				{
					unsigned char h264_start_bit = payloadPtr[1] & 0x80;
					unsigned char h264_end_bit = payloadPtr[1] & 0x40;
					unsigned char h264_type = payloadPtr[1] & 0x1F;
					unsigned char h264_nri = (payloadPtr[0] & 0x60) >> 5;
					unsigned char h264_key = (h264_nri << 5) | h264_type;
					unsigned short don = 0;
					if (payloadSize < 4)
					{
						break;
					}
					int offset = 2;
					if (nal_type == NAL_TYPE_FU_B)
					{
						don = (int(payloadPtr[2]) << 8) | (int(payloadPtr[3]));
						offset += 2;
					}
					if (h264_start_bit)
					{
						for (auto i : m_fu_cache)
						{
							delete[]i.data;
						}
						m_fu_cache.clear();
						datapacket pkt;
						pkt.size = payloadSize - offset;
						pkt.size += 1;
						pkt.data = new unsigned char[pkt.size];
						pkt.data[0] = h264_type | (h264_nri << 5);
						memcpy(pkt.data + 1, payloadPtr + offset, pkt.size);
						m_fu_cache.push_back(pkt);
					}
					else
					{
						datapacket pkt;
						pkt.size = payloadSize - offset;
						pkt.data = new unsigned char[pkt.size];
						memcpy(pkt.data, payloadPtr + offset, pkt.size);
						m_fu_cache.push_back(pkt);
					}
					if (h264_end_bit)
					{
						int totalSize = 0;
						for (auto i : m_fu_cache)
						{
							totalSize += i.size;
						}
						timedpacket pkt;
						pkt.timestamp = timestamp;
						pkt.size = totalSize;
						pkt.data = new unsigned char[pkt.size];
						int cur = 0;
						for (auto i : m_fu_cache)
						{
							memcpy(pkt.data + cur, i.data, i.size);
							cur += i.size;
						}
						out.push_back(pkt);
					}
				}
				else
				{
					break;
				}

				//free current it
				delete[]it->second.data;
				m_cachedPayload.erase(it);
				//next continued seq
				m_currentSeq = m_currentSeq >= 0xffff ? 0 : m_currentSeq + 1;

				auto seqHoped = findNextHopeSeq();
				if (seqHoped != m_currentSeq)
				{
					it = m_cachedPayload.end();
					break;
				}
				else
				{
					it = m_cachedPayload.find(m_currentSeq);
				}
				result = true;
			} while (0);
		}
#else
		bool result = ParseRTPHeader(data.data, data.size, timestamp, payloadPtr, payloadSize);
		if (!result)
		{
			return out;
		}
		result = false;
		do
		{
			CBitReader bitReader;
			bitReader.SetBitReader(payloadPtr, payloadSize, true);
			int nal_forbidden_zero = bitReader.ReadBit();
			int nal_nri = bitReader.ReadBits(2);
			int nal_type = bitReader.ReadBits(5);
			if (nal_forbidden_zero != 0)
			{
				break;
			}

			if (nal_type >= NAL_TYPE_SINGLE_NAL_MIN&&
				nal_type <= NAL_TYPE_SINGLE_NAL_MAX)
			{
				timedpacket pkt;
				pkt.timestamp = timestamp;
				pkt.size = payloadSize;
				pkt.data = new unsigned char[pkt.size];
				memcpy(pkt.data, payloadPtr, pkt.size);
				out.push_back(pkt);
			}
			else if (nal_type == NAL_TYPE_STAP_A || nal_type == NAL_TYPE_STAP_B)
			{
				int idx = 1;
				int don = 0;
				int nalu_size = 0;
				if (nal_type == NAL_TYPE_STAP_B)
				{
					don = (int(payloadPtr[idx]) << 8) | (int(payloadPtr[idx + 1]));
					idx += 2;
				}
				while (idx < payloadSize)
				{
					nalu_size = (int(payloadPtr[idx]) << 8) | (int(payloadPtr[idx + 1]));
					idx += 2;
					if (nalu_size <= 0)
					{
						continue;
					}
					if (nalu_size + idx > payloadSize)
					{
						break;
					}
					timedpacket pkt;
					pkt.timestamp = timestamp;
					pkt.size = nalu_size;
					pkt.data = new unsigned char[nalu_size];
					memcpy(pkt.data, payloadPtr + idx, nalu_size);
					idx += nalu_size;
					out.push_back(pkt);
				}
			}

			else if (nal_type == NAL_TYPE_MTAP16 || nal_type == NAL_TYPE_MTAP24)
			{
				int idx = 1;
				int dond = 0;
				int dob = 0;
				int nalu_size = 0;
				int time_offset = 0;
				dob = (int(payloadPtr[idx]) << 8) | (int(payloadPtr[idx + 1]));
				idx += 2;
				while (idx < payloadSize)
				{
					if (nal_type == NAL_TYPE_MTAP16)
					{
						if (idx + 5 < payloadSize)
						{
							break;
						}
					}
					else
					{
						if (idx + 6 < payloadSize)
						{
							break;
						}
					}
					nalu_size = (int(payloadPtr[idx]) << 8) | (int(payloadPtr[idx + 1]));
					idx += 2;
					dond = payloadPtr[idx];
					idx++;
					if (nal_type == NAL_TYPE_MTAP16)
					{
						time_offset = (int(payloadPtr[idx]) << 8) | (int(payloadPtr[idx + 1]));
						idx += 2;
					}
					else {
						time_offset = (int(payloadPtr[idx]) << 16) | (int(payloadPtr[idx]) << 8) | (int(payloadPtr[idx]) << 0);
						idx += 3;
					}
					if (idx + nalu_size < payloadSize)
					{
						break;
					}
					timedpacket pkt;
					pkt.size = nalu_size;
					pkt.data = new unsigned char[nalu_size];
					memcpy(pkt.data, payloadPtr + idx, pkt.size);
					pkt.timestamp = timestamp + time_offset;
					out.push_back(pkt);
					idx += nalu_size;
				}

			}
			else if (nal_type == NAL_TYPE_FU_A || nal_type == NAL_TYPE_FU_B)
			{
				unsigned char h264_start_bit = payloadPtr[1] & 0x80;
				unsigned char h264_end_bit = payloadPtr[1] & 0x40;
				unsigned char h264_type = payloadPtr[1] & 0x1F;
				unsigned char h264_nri = (payloadPtr[0] & 0x60) >> 5;
				unsigned char h264_key = (h264_nri << 5) | h264_type;
				unsigned short don = 0;
				if (payloadSize < 4)
				{
					break;
				}
				int offset = 2;
				if (nal_type == NAL_TYPE_FU_B)
				{
					don = (int(payloadPtr[2]) << 8) | (int(payloadPtr[3]));
					offset += 2;
				}
				if (h264_start_bit)
				{
					for (auto i : m_fu_cache)
					{
						delete[]i.data;
					}
					m_fu_cache.clear();
					datapacket pkt;
					pkt.size = payloadSize - offset;
					pkt.size += 1;
					pkt.data = new unsigned char[pkt.size];
					pkt.data[0] = h264_type | (h264_nri << 5);
					memcpy(pkt.data + 1, payloadPtr + offset, pkt.size);
					m_fu_cache.push_back(pkt);
				}
				else
				{
					datapacket pkt;
					pkt.size = payloadSize - offset;
					pkt.data = new unsigned char[pkt.size];
					memcpy(pkt.data, payloadPtr + offset, pkt.size);
					m_fu_cache.push_back(pkt);
				}
				if (h264_end_bit)
				{
					int totalSize = 0;
					for (auto i : m_fu_cache)
					{
						totalSize += i.size;
					}
					timedpacket pkt;
					pkt.timestamp = timestamp;
					pkt.size = totalSize;
					pkt.data = new unsigned char[pkt.size];
					int cur = 0;
					for (auto i : m_fu_cache)
					{
						memcpy(pkt.data + cur, i.data, i.size);
						cur += i.size;
					}
					out.push_back(pkt);
				}
			}
			else
			{
				break;
			}

			//free current it
			
			result = true;
		} while (0);
#endif // RTP_CACHE



		return out;
	}
	bool RTP_UnPackerH264::ParseRTPHeader(unsigned char * data, int size, unsigned int & timestamp, unsigned char *&payloadPtr, int &payloadSize)
	{
		bool result = false;
		do
		{
			if (size < 12)
			{
				break;
			}
			hwss::CBitReader bitreader;
			bitreader.SetBitReader(data, size, true);
			auto version = bitreader.ReadBits(2);
			auto padding = bitreader.ReadBit();
			auto extension = bitreader.ReadBit();
			auto cc = bitreader.ReadBits(4);
			auto marker = bitreader.ReadBit();
			auto payload_type = bitreader.ReadBits(7);
			auto seq = bitreader.ReadBits(16);
			auto ts = bitreader.Read32Bits();
			auto ssrc = bitreader.Read32Bits();
#ifdef SAVE_SEQ
			m_seqs.push_back(seq);
			if (m_fp == nullptr)
			{
				m_fp = fopen("seq.txt", "w");
			}
			if (m_fp != nullptr)
			{
				char buf[20];
				itoa(seq, buf, 10);
				fwrite(buf, strlen(buf), 1, m_fp);
				char *dom = ",";
				fwrite(dom, strlen(dom), 1, m_fp);
			}
#endif // SAVE_SEQ

			if (version != 2)
			{
				break;
			}

			if (size - 12 < cc * 4)
			{
				break;
			}
			for (auto i = 0; i < cc; i++)
			{
				bitreader.Read32Bits();
			}

			if (size - 12 - cc * 4 < 4)
			{
				break;
			}
			if (extension)
			{
				auto profile = bitreader.ReadBits(16);
				auto length = bitreader.ReadBits(16);

				if (size - 12 - cc * 4 - 4 < length * 4)
				{
					break;
				}
				bitreader.IgnoreBytes(length * 4);
			}

			auto padsize = 0;
			if (padding)
			{
				padsize = data[size - 1];
			}
			payloadSize = size - bitreader.CurrentByteCount() - padsize;
			if (payloadSize <= 0)
			{
				break;
			}
			payloadPtr = bitreader.CurrentByteData();
			timestamp = ts;
			result = true;
		} while (0);
		return result;
	}

#if USE_RTP_SELF_CACHE

	bool RTP_UnPackerH264::ParseRTPHeader(unsigned char * data, int size)
	{
		bool result = false;
		int seq;
		timedpacket pkt;
		pkt.data = nullptr;
		do
		{
			if (size < 12)
			{
				break;
			}
			hwss::CBitReader bitreader;
			bitreader.SetBitReader(data, size, true);
			auto version = bitreader.ReadBits(2);
			auto padding = bitreader.ReadBit();
			auto extension = bitreader.ReadBit();
			auto cc = bitreader.ReadBits(4);
			auto marker = bitreader.ReadBit();
			auto payload_type = bitreader.ReadBits(7);
			seq = bitreader.ReadBits(16);
			auto ts = bitreader.Read32Bits();
			auto ssrc = bitreader.Read32Bits();

#ifdef SAVE_SEQ
			m_seqs.push_back(seq);
			if (m_fp == nullptr)
			{
				m_fp = fopen("seq.txt", "w");
			}
			if (m_fp != nullptr)
			{
				char buf[20];
				itoa(seq, buf, 10);
				fwrite(buf, strlen(buf), 1, m_fp);
				char *dom = ",";
				fwrite(dom, strlen(dom), 1, m_fp);
			}

#endif // SAVE_SEQ
			if (version != 2)
			{
				break;
			}

			if (size - 12 < cc * 4)
			{
				break;
			}
			for (auto i = 0; i < cc; i++)
			{
				bitreader.Read32Bits();
			}

			if (size - 12 - cc * 4 < 4)
			{
				break;
			}
			if (extension)
			{
				auto profile = bitreader.ReadBits(16);
				auto length = bitreader.ReadBits(16);

				if (size - 12 - cc * 4 - 4 < length * 4)
				{
					break;
				}
				bitreader.IgnoreBytes(length * 4);
			}

			auto padsize = 0;
			if (padding)
			{
				padsize = data[size - 1];
			}
			pkt.size = size - bitreader.CurrentByteCount() - padsize;
			if (pkt.size <= 0)
			{
				break;
			}
			pkt.data = new unsigned char[pkt.size];
			memcpy(pkt.data, bitreader.CurrentByteData(), pkt.size);
			pkt.timestamp = ts;
			result = true;
		} while (0);
		if (!result)
		{
			if (pkt.data)
			{
				delete[]pkt.data;
				pkt.data = nullptr;
			}
		}
		else
		{
			//make sure not exist
			auto it = m_cachedPayload.find(seq);
			if (it != m_cachedPayload.end())
			{
				delete[]it->second.data;
				m_cachedPayload.erase(it);
			}
			m_cachedPayload[seq] = pkt;
		}
		return result;
	}

	int RTP_UnPackerH264::findNextHopeSeq()
	{
#ifdef SAVE_SEQ
		int resultSeq = -1;
#endif // SAVE_SEQ

		//current seq +1
		{
			auto nextSeq = m_currentSeq >= 0xffff ? 0 : m_currentSeq + 1;
#ifdef SAVE_SEQ
			resultSeq = nextSeq;
#endif // SAVE_SEQ

			auto it = m_cachedPayload.find(m_currentSeq + 1);
			if (it != m_cachedPayload.end())
			{
				return nextSeq;
			}
		}


#ifdef SAVE_SEQ
		//m_seqs.push_back(resultSeq);
		if (m_fpLosted == nullptr)
		{
			m_fpLosted = fopen("seqLosted.txt", "w");
		}
		if (m_fpLosted != nullptr)
		{
			char buf[20];
			itoa(resultSeq, buf, 10);
			fwrite(buf, strlen(buf), 1, m_fpLosted);
			char *dom = ",";
			fwrite(dom, strlen(dom), 1, m_fpLosted);
		}

#endif // SAVE_SEQ
		//min and max
		{
			int minSeq = 0xffff;
			int	maxSeq = -1;
			for (auto i : m_cachedPayload)
			{
				minSeq = minSeq < i.first ? minSeq : i.first;
				maxSeq = maxSeq > i.first ? maxSeq : i.first;
			}
			if (maxSeq - minSeq > 0x7fff)
			{
				//max-min>0xffff>>1 new loop
				//find the min value bigger than 0x7fff
				for (auto i : m_cachedPayload)
				{
					if (i.first > 0x7fff)
					{
						maxSeq = maxSeq < i.first ? maxSeq : i.first;
					}
				}
				return maxSeq;
			}
			else
			{
				//no new loop,just this loop,use min seq
				return minSeq;
			}

			return	 0;
		}

		//error 
		return 0;
	}

#endif // USE_RTP_SELF_CACHE

}
