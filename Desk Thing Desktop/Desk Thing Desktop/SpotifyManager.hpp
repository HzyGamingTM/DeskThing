#pragma once

#include <functional>
#include <iostream>
#include <string>

#include <winrt/windows.media.control.h>
#include <winrt/windows.foundation.h>
#include <winrt/windows.foundation.collections.h>

#include "await.hpp"
#include "Wireblahaj.hpp"

using namespace winrt::Windows::Media::Control;
using namespace winrt::Windows::Foundation;
using namespace winrt::Windows::Foundation::Collections;

using MediaSessionManager = GlobalSystemMediaTransportControlsSessionManager;
using MediaSession = GlobalSystemMediaTransportControlsSession;

class SpotifyMgr {
private:
	MediaSessionManager msmgr;
	bool GetSpotifySession(std::function<void(const MediaSession&)> successfulCallback, std::function<void(void)> unsuccessfulCallback);

public:
	SpotifyMgr();
	void HandleMessage(WlMessage msg);
};