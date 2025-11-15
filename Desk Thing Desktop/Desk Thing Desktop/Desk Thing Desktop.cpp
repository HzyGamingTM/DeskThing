
/*
#include <iostream>
using namespace std;
#include "Wireblahaj.hpp"
#include "ListenServer.hpp"

#include "await.hpp"

#include <iostream>
#include <functional>
#include <vector>
#include <cstdlib>

#include <Windows.h>
#include <windowsx.h>

#include <winrt/windows.media.control.h>
#include <winrt/windows.foundation.h>
#include <winrt/windows.foundation.collections.h>

using namespace winrt::Windows::Media::Control;
using namespace winrt::Windows::Foundation;
using namespace winrt::Windows::Foundation::Collections;
using namespace std::chrono;
using namespace std;

using MediaSessionManager = GlobalSystemMediaTransportControlsSessionManager;
using MediaSession = GlobalSystemMediaTransportControlsSession;
*/

	/*
#pragma comment(lib, "Ws2_32.lib") // Link with Ws2_32.lib
#pragma comment(lib, "iphlpapi.lib")
#pragma comment(lib, "RuntimeObject.lib")
#pragma comment(lib, "Ole32.lib")
#pragma comment(lib, "OleAut32.lib")

int main() {
	cout << "Saluations Environment" << endl;

	WlMessageReceiver receiver;
	ListenServer listenServer;
	// SpotifyMgr spotifyMgr;

	auto conn = listenServer.acceptOne();
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
	

	cout << "Connection from " << conn.clientIp << " on " << conn.serverIp << endl;

	send(socket, "helo worl", 10, 0);

	closesocket(socket);

	return 0;
}
	*/
int main() {}
