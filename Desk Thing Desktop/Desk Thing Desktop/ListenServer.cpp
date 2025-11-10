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
	SOCKET client = accept(listenSock, NULL, NULL);
	conn.client = client;

	// TODO: get the client's ip and store in a class variable
	if (client != INVALID_SOCKET) {
		conn.ipStrClient = "TODO";
		conn.ipStrServer = "TODO";
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
