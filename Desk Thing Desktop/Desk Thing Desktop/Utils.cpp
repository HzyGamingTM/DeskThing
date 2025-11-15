#include "Utils.hpp"

#include <winsock2.h>
#include <ws2tcpip.h>
#include <iphlpapi.h>

std::string GetLocalIp() {
	IP_ADAPTER_ADDRESSES* addresses = nullptr;
	ULONG buffer_size = 0;

	// First call to get buffer size
	GetAdaptersAddresses(AF_UNSPEC, GAA_FLAG_INCLUDE_PREFIX, NULL, addresses, &buffer_size);
	addresses = (IP_ADAPTER_ADDRESSES*)malloc(buffer_size);

	SOCKADDR_IN* lastValidIpv4Ptr = 0;

	if (GetAdaptersAddresses(AF_UNSPEC, GAA_FLAG_INCLUDE_PREFIX, NULL, addresses, &buffer_size) == NO_ERROR) {
		for (IP_ADAPTER_ADDRESSES* adapter = addresses; adapter != 0; adapter = adapter->Next) {
			if (adapter->IfType != IF_TYPE_SOFTWARE_LOOPBACK)
				continue;

			for (IP_ADAPTER_UNICAST_ADDRESS* addr = adapter->FirstUnicastAddress; addr != 0; addr = addr->Next)
				if (addr->Address.lpSockaddr->sa_family == AF_INET)
					lastValidIpv4Ptr = (SOCKADDR_IN*)addr->Address.lpSockaddr;
		}
	}

	char str_buffer[INET_ADDRSTRLEN] = {};

	if (lastValidIpv4Ptr)
		inet_ntop(AF_INET, &(lastValidIpv4Ptr->sin_addr), str_buffer, INET_ADDRSTRLEN);

	free(addresses);
	return std::string(str_buffer);
}