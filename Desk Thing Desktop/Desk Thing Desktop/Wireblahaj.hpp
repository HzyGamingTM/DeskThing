#pragma once

#include <iostream>
#include <string>
#include <inttypes.h>

#include <winsock2.h>
#include <ws2tcpip.h> // For getaddrinfo and related functions

struct WlMessage {
	const unsigned char *data;
	uint16_t pos = 0;

	constexpr inline uint32_t id() {
		return *(uint32_t*)data;
	}

	constexpr inline uint16_t opcode() {
		return *(uint16_t*)(data + 4);
	}

	constexpr inline uint16_t size() {
		return *(uint16_t*)(data + 6);
	}

	inline uint16_t jump(int amount) {
		return pos += amount;
	}

	inline uint16_t seek(uint16_t newpos) {
		return pos = newpos;
	}

	inline uint8_t u8() {
		return data[pos++];
	}

	inline int8_t i8() {
		return ((int8_t*)data)[pos++];
	}

	inline int16_t u16() {
		if ((pos & 1) == 0) {
			uint16_t ret = ((uint16_t*)data)[pos >> 1];
			pos += 2;
			return *(int16_t*)&ret;
		} else {
			uint16_t ret = u8();
			ret += (uint16_t)u8() << 8;
			return *(int16_t*)&ret;
		}
	}

	inline int32_t u32() {
		if ((pos & 3) == 0) {
			uint32_t ret = ((uint32_t*)data)[pos >> 2];
			pos += 4;
			return *(int32_t*)&ret;
		} else {
			uint32_t ret = u8();
			ret += (uint32_t)u8() << 8;
			ret += (uint32_t)u8() << 16;
			ret += (uint32_t)u8() << 24;
			return *(int32_t*)&ret;
		}
	}

	inline const char *str() {
		uint32_t strln = u32();
		strln = (strln + 3) & ~3;
		pos += strln;
		return (const char*)(data + pos - strln);
	}
};

struct WlMessageBuilder {
	unsigned char *data;

	int sock;

	uint16_t size;

	inline WlMessageBuilder() : size(0) {
		data = new unsigned char[65536];
	}

	inline WlMessageBuilder &seek(uint16_t pos) {
		size = pos;
		return *this;
	}

	inline WlMessageBuilder &jump(int32_t j) {
		size += j;
		return *this;
	}

	inline WlMessageBuilder &u8(uint8_t u) {
		data[size++] = u;
		return *this;
	}

	inline WlMessageBuilder &u16(uint16_t u) {
		if ((size & 1) == 0) {
			*(uint16_t*)(data + size) = u;
			size += 2;
		} else {
			u8(uint8_t(u));
			u8(uint8_t(u >> 8));
		}
		return *this;
	}

	inline WlMessageBuilder &u32(uint32_t u) {
		if ((size & 3) == 0) {
			*(uint32_t*)(data + size) = u;
			size += 4;
		} else {
			u8(uint8_t(u));
			u8(uint8_t(u >> 8));
			u8(uint8_t(u >> 16));
			u8(uint8_t(u >> 24));
		}
		return *this;
	}

	inline WlMessageBuilder &str(const char *s, size_t len_with_nul = -1) {
		if (len_with_nul == -1) len_with_nul = strlen(s) + 1;
		uint32_t roundup_len = (len_with_nul + 3) & ~3;
		u32(len_with_nul);
		memcpy(data + size, s, len_with_nul);
		size += roundup_len;
		return *this;
	}

	inline WlMessageBuilder &newId(char *interfaceName, uint32_t version, uint32_t id) {
		if (interfaceName) {
			str(interfaceName);
			u32(version);
		}

		u32(id);
		return *this;
	}
	
	inline WlMessageBuilder &header(uint32_t id, uint16_t opcode) {
		u32(id);
		u16(opcode);
		u16(8);
		return *this;
	}

	inline WlMessageBuilder &writeSize() {
		*(uint16_t*)(data + 6) = size;
		return *this;
	}

	inline WlMessageBuilder &reset() {
		seek(0);
		return *this;
	}

	inline WlMessageBuilder &sendAndReset(int flags = 0) {
		writeSize();

		send(sock, (char*)data, size, 0);

		reset();
		return *this;
	}

	inline ~WlMessageBuilder() {
		if (data) {
			delete[] data;
			data = 0;
		}

		size = 0;
	}
};

class WlMessageReceiver {
	private:
		unsigned char *rcvBuf;
		const uint32_t rcvBufMax;  // TODO: allow resizing of buffer if empty
		uint32_t rcvBufSize;
		uint32_t rcvBufPos = 0;

		unsigned char *msgBuf;   // for partial messages
		uint32_t msgBufPos;
		bool msgComplete = false;

	public:
		SOCKET sock;
		WlMessage tmpMsg;

		inline WlMessageReceiver(unsigned int rcvBufMax = 65536) : rcvBufMax(rcvBufMax) {
			rcvBuf = new unsigned char[rcvBufMax];
			rcvBufPos = 0;
			msgBuf = new unsigned char[65536];
			msgBufPos = 0;
		}

		constexpr inline bool rcvBufEmpty() {
			return rcvBufPos >= rcvBufSize;
		}

		constexpr inline bool pending() {
			return msgComplete;
		}

		inline uint32_t fillBuffer() {
			if (!rcvBufEmpty()) {
				std::cerr 
					<< "WARNING: fillBuffer called on non-empty receive buffer! "
					<< "Messages will be discarded." 
					<< std::endl;
			}
			rcvBufSize = recv(sock, (char*)rcvBuf, rcvBufMax, 0);
			rcvBufPos = 0;
			return rcvBufSize;
		}

		inline bool advance() {
			if (rcvBufEmpty()) {
				// No data left to process in receive buffer.
				return false;
			}

			if (msgComplete) {
				msgBufPos = 0;
				msgComplete = false;
			}

			// guaranteed rcvBufSize > rcvBufSize
			uint32_t availableRcv = rcvBufSize - rcvBufPos;
			unsigned char *currentRcvBytes = rcvBuf + rcvBufPos;
			if (msgBufPos == 0) {
				if (availableRcv >= 8) {
					uint16_t msgSize = *(uint16_t*)(currentRcvBytes + 6);
					if (availableRcv >= msgSize) {
						tmpMsg.pos = 0;
						tmpMsg.data = currentRcvBytes;

						rcvBufPos += msgSize;
						msgComplete = true;
						return msgComplete;
					} else {
						memcpy(msgBuf, currentRcvBytes, availableRcv);

						msgBufPos += availableRcv;
						rcvBufPos += availableRcv;
						msgComplete = false;
						return msgComplete;
					}
				} else {
					memcpy(msgBuf, currentRcvBytes, availableRcv);

					msgBufPos += availableRcv;
					rcvBufPos += availableRcv;
					msgComplete = false;
					return msgComplete;
				}
			} else {
				if (msgBufPos < 8) {
					uint16_t neededForHeader = 8 - msgBufPos;
					if (availableRcv >= neededForHeader) {
						memcpy(msgBuf + msgBufPos, currentRcvBytes, neededForHeader);

						rcvBufPos += neededForHeader;
						msgBufPos += neededForHeader;

						// These are needed because the function does not
						// return in this branch and later code needs it
						currentRcvBytes += neededForHeader;
						availableRcv -= neededForHeader;
					} else {
						memcpy(msgBuf + msgBufPos, currentRcvBytes, availableRcv);

						rcvBufPos += availableRcv;
						msgBufPos += availableRcv;
						msgComplete = false;
						return msgComplete;
					}
				}

				// guaranteed msgBufPos >= 8, size from header can be read
				uint16_t msgSize = *(uint16_t*)(msgBuf + 6);
				uint16_t neededForMsg = msgSize - msgBufPos;

				if (availableRcv >= neededForMsg) {
					memcpy(msgBuf + msgBufPos, currentRcvBytes, neededForMsg);
					tmpMsg.pos = 0;
					tmpMsg.data = msgBuf;

					msgBufPos += neededForMsg;
					rcvBufPos += neededForMsg;
					msgComplete = true;
					return msgComplete;
				} else {
					memcpy(msgBuf + msgBufPos, currentRcvBytes, availableRcv);
					
					msgBufPos += availableRcv;
					rcvBufPos += availableRcv;
					msgComplete = false;
					return msgComplete;
				}
			}
		}

		inline ~WlMessageReceiver() {
			delete[] rcvBuf;
			delete[] msgBuf;
		}
};
