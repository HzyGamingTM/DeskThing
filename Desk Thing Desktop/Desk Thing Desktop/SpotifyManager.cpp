#include <functional>
#include <iostream>
#include <string>

#include <winrt/windows.media.control.h>
#include <winrt/windows.foundation.h>
#include <winrt/windows.foundation.collections.h>

#include "Wireblahaj.hpp"
#include "await.hpp"

using namespace winrt::Windows::Media::Control;
using namespace winrt::Windows::Foundation;
using namespace winrt::Windows::Foundation::Collections;

using MediaSessionManager = GlobalSystemMediaTransportControlsSessionManager;
using MediaSession = GlobalSystemMediaTransportControlsSession;

class SpotifyMgr {
private:
	MediaSessionManager msmgr;
	bool GetSpotifySession(std::function<void(const MediaSession&)> successfulCallback, std::function<void(void)> unsuccessfulCallback) {
		auto sessions = msmgr.GetSessions();

		auto iter = sessions.First();
		auto sessionCount = sessions.Size();

		for (int i = 0; i < sessionCount; i++) {
			auto session = *iter;
			auto props = await session.TryGetMediaPropertiesAsync();
			auto programName = winrt::to_string(session.SourceAppUserModelId());
			auto songTitle = winrt::to_string(props.Title());

			std::cout << songTitle << " from " << programName << std::endl;
			if (programName == "Spotify.exe") {
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

		std::cout << id << " " << opcode << " " << size << std::endl;

		msg.jump(8); // Skip header cuz rey is bad

		switch (opcode) {
		case 0: // Close connection
		{
			std::cout << "Client requested to close connection." << std::endl;
		}
		break;

		case 1: // Play / Pause
		{
			GetSpotifySession(
				[](const MediaSession& session) {
					session.TryTogglePlayPauseAsync();
				},
				[]() {
					std::cerr << "Couldn't play/pause" << std::endl;
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
					std::cerr << "Couldn't skip next" << std::endl;
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
					std::cerr << "Couldn't skip previous" << std::endl;
				}
			);
		}
		break;
		}
	}
};
