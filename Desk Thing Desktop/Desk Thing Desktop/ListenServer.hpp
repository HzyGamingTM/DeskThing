#ifndef ListenServer_hpp
#define ListenServer_hpp

#include <string>
#include <WS2tcpip.h>

class ListenServer {
public:
	enum SocketError {
		SUCCESS = 0,
		WSASTARTUP_FAILED,
		LISTEN_BIND_FAILED,
		LISTEN_FAILED,
		CLIENT_CONNECT_FAILED,
	};

	struct Connection {
		std::string deviceName;
		std::string clientIp;
		std::string serverIp;
		SOCKET client;
	};

private:
	SOCKET listenSock;
	bool initialized;
	bool countedInstance;

	static uint64_t instances;
	static bool wsaStarted;

	SocketError error;
	int wsaError;

public:
	Connection lastConnection;

	const SocketError& lastError;
	const int& lastWsaError;
	const bool& good;

	ListenServer(unsigned short port = 6767);

	Connection acceptOne();

	~ListenServer();
};

#endif
