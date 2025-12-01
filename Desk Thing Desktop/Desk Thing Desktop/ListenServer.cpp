#include "ListenServer.hpp"
#include <stdio.h>

bool ListenServer::wsaStarted = false;
uint64_t ListenServer::instances = 0;

ListenServer::ListenServer(unsigned short port) :
		listenSock(INVALID_SOCKET), initialized(false), countedInstance(false), 
		error(SUCCESS), wsaError(0), lastError(error), lastWsaError(wsaError),
		good(initialized) {

	printf("[ListenServer@%p] Constructor called,", this);

	int res;

	if (!wsaStarted) {
		printf(" WSA not started\n");
		WSADATA wdata;
		res = WSAStartup(MAKEWORD(2, 2), &wdata);
		if (res != 0) {
			wsaError = res;
			initialized = false;
			return;
		}
		wsaStarted = true;
	} else {
		printf(" WSA started\n");
	}

	countedInstance = true;
	instances++;

	printf("[ListenServer@%p] instance count now: %llu\n", this, instances);

	listenSock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	sockaddr_in sin = {};
	sin.sin_family = AF_INET;
	sin.sin_addr.s_addr = INADDR_ANY; 
	sin.sin_port = htons(port);

	res = bind(listenSock, (const SOCKADDR*)&sin, (int)sizeof(sin));
	if (res == SOCKET_ERROR) {
		printf("[ListenServer@%p] Error binding\n", this);
		return;
	}

	printf("[ListenServer@%p] Binding\n", this);

	// Listen for incoming connections
	res = listen(listenSock, SOMAXCONN);
	if (res == SOCKET_ERROR) {
		printf("[ListenServer@%p] Error listening\n", this);
		return;
	}

	printf("[ListenServer@%p] Listening\n", this);

	initialized = true;
}

ListenServer::Connection ListenServer::acceptOne() {
	Connection conn = {};
	printf("ListenServer: Accepting one\n");
	SOCKET client = accept(listenSock, NULL, NULL);
	printf("ListenServer: Accepted one\n");
	conn.client = client;

	if (client != INVALID_SOCKET) {
		// Client IP
		SOCKADDR_IN clientAddr;
		int sizeofClientAddr = sizeof clientAddr;
		char ipStrBuffer[INET_ADDRSTRLEN] = {};
		if (getpeername(client, (SOCKADDR*)&clientAddr, &sizeofClientAddr) == 0) {
			inet_ntop(AF_INET, &clientAddr.sin_addr, ipStrBuffer, sizeof ipStrBuffer);
		}

		conn.clientIp = ipStrBuffer;

		// Server IP
		SOCKADDR_IN serverAddr;
		int sizeofServerAddr = sizeof serverAddr;
		if (getsockname(client, (SOCKADDR*)&serverAddr, &sizeofServerAddr) == 0) {
			inet_ntop(AF_INET, &serverAddr.sin_addr, ipStrBuffer, sizeof ipStrBuffer);
		}
		conn.serverIp = ipStrBuffer;
	}

	return conn;
}

ListenServer::~ListenServer() {
	printf("[ListenServer@%p] Destructor called\n", this);

	if (listenSock != INVALID_SOCKET) {
		printf("[ListenServer@%p] Closing listening socket\n", this);
		closesocket(listenSock);
	}

	if (countedInstance && instances > 0) {
		instances--;
	}
	countedInstance = false;
	initialized = false;

	printf("[ListenServer@%p] Instance count now: %llu\n", this, instances);

	if (wsaStarted && instances <= 0) {
		WSACleanup();

		printf("[ListenServer@%p] cleaning up\n", this);
	}
}

void ListenServer::interrupt() {
	printf("[ListenServer@%p] Trying to interrupt server\n", this);
	SOCKET sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

	sockaddr_in sin = {};
	sin.sin_family = AF_INET;
	sin.sin_addr.s_addr = 0x0100007f; 
	sin.sin_port = htons(6767);

	SOCKET otherSock = connect(sock, (sockaddr*)&sin, sizeof sin);
	if (otherSock != INVALID_SOCKET) {
		int sentBytes = send(otherSock, "please kill yourself", 21, 0);
		if (sentBytes != 21) {
			printf("WARNING: Sent bytes not equal to 21, errno %d %d\n", errno, WSAGetLastError());
		}
		closesocket(otherSock);
		printf("[ListenServer@%p] Sent off message\n", this);
	} else {
		printf("[ListenServer@%p] Couldnt connect to socket, %d\n", this, WSAGetLastError());
	}
}