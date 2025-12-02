#pragma once

#include <functional>

#include "Wireblahaj.hpp"

#include <winrt/windows.media.control.h>
#include <winrt/windows.foundation.h>
#include <winrt/windows.foundation.collections.h>

#include <mmdeviceapi.h>
#include <audiopolicy.h>

#include <wrl/client.h>

class SpotifyMgr {
private:
	winrt::Windows::Media::Control::GlobalSystemMediaTransportControlsSessionManager msmgr;

	// Audio Session
	Microsoft::WRL::ComPtr<IMMDeviceEnumerator> deviceEnumerator;

	winrt::Windows::Media::Control::GlobalSystemMediaTransportControlsSession GetSpotifySession();
	HRESULT GetAudioSession(IAudioSessionControl2 **out);

public:
	SpotifyMgr();
	void HandleMessage(WlMessage msg);
};
