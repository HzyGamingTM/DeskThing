#include <combaseapi.h>
#include <iostream>

#include <objbase.h>
#include <winrt/windows.media.control.h>
#include <winrt/windows.foundation.h>
#include <winrt/windows.foundation.collections.h>

#include "await.hpp"
#include "ListenServer.hpp"
#include "Wireblahaj.hpp"
#include "WindowManager.hpp"
#include "SpotifyManager.hpp"

#include <Windows.h>
#include <windowsx.h>

using namespace winrt::Windows::Media::Control;
using namespace winrt::Windows::Foundation;
using namespace winrt::Windows::Foundation::Collections;
using namespace std::chrono;
using namespace std;

#pragma comment(lib, "Ws2_32.lib") // Link with Ws2_32.lib
#pragma comment(lib, "iphlpapi.lib")
#pragma comment(lib, "RuntimeObject.lib")
#pragma comment(lib, "Ole32.lib")
#pragma comment(lib, "OleAut32.lib")

bool keepRunning = true;

DWORD WINAPI tread_lightly(void *ptr) {
	ListenServer &ls = *(ListenServer*)ptr;

	WindowMgr wmg("winow", 640, 480);
	wmg.InitWindow();

	keepRunning = false;
	MemoryBarrier();
	ls.interrupt();

	return 0;
}

int main() {
	cout << "Saluations Environment" << endl;

	HRESULT hres = CoInitializeEx(0, COINIT_APARTMENTTHREADED);
	if (hres != S_OK) {
		cerr << "CoInitializeEx returned " << hres << endl;
		return 1;
	}

	WlMessageReceiver receiver;
	ListenServer listenServer;
	SpotifyMgr spotifyMgr;

	// auto conn = listenServer.acceptOne();
	// SOCKET socket = conn.client;

	// receiver.sock = socket;
	
	/*
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
	*/
	
	// cout << "Connection from " << conn.clientIp << " on " << conn.serverIp << endl;

	// send(socket, "helo worl", 10, 0);

	// closesocket(socket);

	CreateThread(0, 0, tread_lightly, (void*)&listenServer, 0, 0);

	while (keepRunning) {
		auto conn = listenServer.acceptOne();
		SOCKET socket = conn.client;

		receiver.sock = socket;

		cout << (int)socket << endl;
		cout << "Connection from " << conn.clientIp << " on " << conn.serverIp << endl;

		while (keepRunning) {
			if (receiver.rcvBufEmpty()) {
				uint32_t recvSize = receiver.fillBuffer();
				if (recvSize == 0 || recvSize == -1) {
					cout << "Recv size " << recvSize << ", breaking" << endl;
					break;
				}
			}

			if (receiver.advance()) {
				if (receiver.tmpMsg.size() == 0) {
					cout << "Buffered message size 0, breaking" << endl;
					break;
				}

				spotifyMgr.HandleMessage(receiver.tmpMsg);
			}
		}

		receiver.reset();
		
		closesocket(socket);
	}

	CoUninitialize();

	return 0;
}
