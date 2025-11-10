#include <iostream>
#include <chrono>
#include <functional>
#include <vector>
#include <cstdlib>

#include <winsock2.h>
#include <ws2tcpip.h> // For getaddrinfo and related functions
#include <iphlpapi.h>
#include <Windows.h>
#include <windowsx.h>

#include <winrt/windows.media.control.h>
#include <winrt/windows.foundation.h>
#include <winrt/windows.foundation.collections.h>

#include "Wireblahaj.hpp"

using namespace winrt::Windows::Media::Control;
using namespace winrt::Windows::Foundation;
using namespace winrt::Windows::Foundation::Collections;
using namespace std::chrono;
using namespace std;

using MediaSessionManager = GlobalSystemMediaTransportControlsSessionManager;
using MediaSession = GlobalSystemMediaTransportControlsSession;

#pragma comment(lib, "Ws2_32.lib") // Link with Ws2_32.lib
#pragma comment(lib, "iphlpapi.lib")
#pragma comment(lib, "RuntimeObject.lib")

#define PORT_NUMBER 6767 // Das to tuff gng
#define await _m_await<< // Await in C++ real

struct _m_await_class {
	template <typename T>
	T operator<<(IAsyncOperation<T> asy) {
		if (asy.wait_for(5s) == AsyncStatus::Completed)
			return asy.GetResults();

		throw runtime_error("bullshit happned");
	}
} _m_await;


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
		string deviceName;
		string ipStrClient;
		string ipStrServer;
		SOCKET client;
	};

private:
	SOCKET listenSock = INVALID_SOCKET;
	bool initialized = false;
	bool countedInstance = false;

	static uint64_t instances;
	static bool wsaStarted;

	SocketError error;
	int wsaError;

public:
	Connection lastConnection;

	const SocketError& lastError = error;
	const int& lastWsaError = wsaError;
	const bool& good = initialized;

	ListenServer() {
		printf("[ListenServer@%p] Constructor called,", this);
		int res;
		if (!wsaStarted) {
			printf(" WSA not started\n");
			WSADATA wdata;
			res = WSAStartup(MAKEWORD(2, 2), &wdata);
			if (res != 0) {
				cerr << "WSAStartup failed: " << res << endl;
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
		sin.sin_port = htons(PORT_NUMBER);

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

	Connection acceptOne() {
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

	~ListenServer() {
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
};
bool ListenServer::wsaStarted = false;
uint64_t ListenServer::instances = 0;


// TODO: WinUI
class WindowMgr {
private:
	static LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp) {
		if (msg == WM_PAINT) {
			HBRUSH hbr = CreateSolidBrush(RGB(255, 0, 0));
			HBRUSH black = CreateSolidBrush(RGB(0, 0, 0));

			PAINTSTRUCT ps;
			HDC hdc = BeginPaint(hwnd, &ps);

			FillRect(hdc, &ps.rcPaint, hbr);

			HGDIOBJ oldObj = SelectObject(hdc, black);

			string serverAddress = "Server Address: ";
			string clientAddress = "Client Address: ";


			// hacky hack
			RECT drawRectangle = ps.rcPaint;
			DrawTextA(hdc, "pigga", 5, &ps.rcPaint, DT_CENTER);
			DrawTextA(hdc, serverAddress.data(), serverAddress.length(), &drawRectangle, DT_CENTER);
			drawRectangle.top += 20;
			DrawTextA(hdc, clientAddress.data(), clientAddress.length(), &drawRectangle, DT_CENTER);

			SelectObject(hdc, oldObj);

			EndPaint(hwnd, &ps);

			DeleteBrush(hbr);
			DeleteBrush(black);
		}
		else if (msg == WM_CLOSE) {
			DestroyWindow(hwnd);
		}
		else if (msg == WM_DESTROY) {
			PostQuitMessage(0);
			return 0;
		}

		return DefWindowProc(hwnd, msg, wp, lp);
	}

public:	
	LONG width, height;
	string title;

	void InitWindow() {
		HINSTANCE hinst = GetModuleHandle(0);

		WNDCLASSEXA wcex = {};
		wcex.cbSize = sizeof wcex;
		wcex.lpfnWndProc = WndProc;
		wcex.lpszClassName = "Main class";
		wcex.style = CS_HREDRAW | CS_VREDRAW;
		wcex.hInstance = hinst;

		RegisterClassExA(&wcex);

		RECT clientRect = { 0, 0, width, height };
		AdjustWindowRectEx(&clientRect, WS_OVERLAPPEDWINDOW, 0, WS_EX_OVERLAPPEDWINDOW);
		HWND hwnd = CreateWindowExA(
			WS_EX_OVERLAPPEDWINDOW, "Main class", title.data(), WS_OVERLAPPEDWINDOW,
			CW_USEDEFAULT, CW_USEDEFAULT, clientRect.right - clientRect.left, clientRect.bottom - clientRect.top,
			0, 0, hinst, 0
		);

		ShowWindow(hwnd, SW_SHOW);
	}

	WindowMgr(string title, LONG width, LONG height) {
		this->title = title;
		this->width = width;
		this->height = height;
	}
};

class SpotifyMgr {
private:
	MediaSessionManager msmgr;
	bool GetSpotifySession(function<void(const MediaSession&)> successfulCallback, function<void(void)> unsuccessfulCallback) {
		auto sessions = msmgr.GetSessions();

		auto iter = sessions.First();
		auto sessionCount = sessions.Size();

		for (int i = 0; i < sessionCount; i++) {
			auto session = *iter;
			auto props = await session.TryGetMediaPropertiesAsync();
			auto programName = winrt::to_string(session.SourceAppUserModelId());
			auto songTitle = winrt::to_string(props.Title());

			cout << songTitle << " from " << programName << endl;
			if (programName == "Spotify.exe") {
				cout << "Found spotify!" << endl;
				break;
			}

			iter.MoveNext();
		}

		if (!iter.HasCurrent()) {
			if (unsuccessfulCallback) unsuccessfulCallback();
			return false;
		}
		else {
			if (successfulCallback) successfulCallback(*iter);
			return true;
		}
	}

public:
	SpotifyMgr() : msmgr(NULL) {
		msmgr = await GlobalSystemMediaTransportControlsSessionManager::RequestAsync();
	}

	 void HandleMessage(WlMessage msg) {
		unsigned int id = msg.id(); // 4 bytes: Target
		unsigned short opcode = msg.opcode(); // 2 bytes: Instruction e.g: Play / Pause
		unsigned short size = msg.size(); // 2 bytes

		cout << (void*)id << " " << opcode << " " << size << endl;

		msg.jump(8); // Skip header cuz rey is bad

		switch (opcode) {
		case 0: // Close connection
		{
			cout << "Client requested to close connection." << endl;
		}
		break;

		case 1: // Play / Pause
		{
			GetSpotifySession(
				[](const MediaSession& session) {
					session.TryTogglePlayPauseAsync();
				},
				[]() {
					cerr << "Couldn't play/pause" << endl;
				}
			);
		}
		break;

		case 2: // Next
		{
			GetSpotifySession(
				[](const MediaSession& session) {
					session.TrySkipNextAsync();
				},
				[]() {
					cerr << "Couldn't skip next" << endl;
				}
			);
		}
		break;

		case 3: // Previous
		{
			GetSpotifySession(
				[](const MediaSession& session) {
					session.TrySkipPreviousAsync();
				},
				[]() {
					cerr << "Couldn't skip previous" << endl;
				}
			);
		}
		break;
		}
	}
};

void socketErr(SOCKET ListenSocket) {
	cerr << "bind failed with error: " << WSAGetLastError() << endl;
	closesocket(ListenSocket);
	WSACleanup();
}

string GetLocalIp() {
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
	return string(str_buffer);
}

/*
int startSocket() {
	WSADATA wsaData;
	unsigned long long iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (iResult != 0) {
		cerr << "WSAStartup failed: " << iResult << endl;
		return WSASTARTUP_FAILED;
	}

	SOCKET ListenSocket = INVALID_SOCKET;
	ListenSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

	sockaddr_in service;
	service.sin_family = AF_INET;
	service.sin_addr.s_addr = INADDR_ANY; // Listen on any available IP address
	service.sin_port = htons(PORT_NUMBER);    // Port to listen on

	iResult = bind(ListenSocket, (const SOCKADDR*)&service, (int)sizeof(service));
	if (iResult == SOCKET_ERROR) {
		socketErr(ListenSocket);
		return LISTEN_BIND_FAILED;
	}

	// Listen for incoming connections
	iResult = listen(ListenSocket, SOMAXCONN);
	if (iResult == SOCKET_ERROR) {
		socketErr(ListenSocket);
		return LISTEN_FAILED;
	}

	cout << "Server listening on port: " << ntohs(service.sin_port) << endl;

	SOCKET ClientSocket = INVALID_SOCKET;
	ClientSocket = accept(ListenSocket, NULL, NULL);
	if (ClientSocket == INVALID_SOCKET) {
		socketErr(ListenSocket);
		return CLIENT_CONNECT_FAILED;
	}
	
	SOCKADDR_IN clientAddr;
	int clientAddrSize = sizeof(clientAddr);
	char ipBuffer[INET_ADDRSTRLEN] = {};
	if (getpeername(ClientSocket, (SOCKADDR*)&clientAddr, &clientAddrSize) == 0) {
		inet_ntop(AF_INET, &clientAddr.sin_addr, ipBuffer, sizeof(ipBuffer));
	}
	
	if (ipBuffer[0] == 0) {
		strcpy_s(ipBuffer, "Unknown IP");
		cerr << "Failed to get client IP address." << endl;
	}

	currentConnection.ipAddressClient = string(ipBuffer);

	cout << "Client connected!" << currentConnection.ipAddressClient << endl;

	receiver.sock = ClientSocket;
	while (1) {
		if (receiver.rcvBufEmpty()) {
			uint32_t recvSize = receiver.fillBuffer();
			if (recvSize == 0 || recvSize == -1)
				break;
		}

		if (receiver.advance()) {
			if (receiver.tmpMsg.size() == 0)
				break;

			spotifyMgr.HandleMessage(receiver.tmpMsg);
		}
	}

	closesocket(ClientSocket);
	closesocket(ListenSocket);
	WSACleanup();

	return SUCCESS;
}
*/

SpotifyMgr spotifyMgr;

int main() {
	cout << "Saluations Environment" << endl;

	WlMessageReceiver receiver;
	ListenServer listenServer;

	ListenServer::Connection conn = listenServer.acceptOne();
	SOCKET socket = conn.client;
	receiver.sock = socket;
	
	while (1) {
		if (receiver.rcvBufEmpty()) {
			uint32_t recvSize = receiver.fillBuffer();
			if (recvSize == 0 || recvSize == -1)
				break;
		}

		if (receiver.advance()) {
			if (receiver.tmpMsg.size() == 0)
				break;

			spotifyMgr.HandleMessage(receiver.tmpMsg);
		}
	}

	return 0;
}