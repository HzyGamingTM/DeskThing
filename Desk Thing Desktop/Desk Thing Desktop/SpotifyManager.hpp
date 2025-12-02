#pragma once

#include <functional>

#include "Wireblahaj.hpp"

#include <winrt/windows.media.control.h>
#include <winrt/windows.foundation.h>
#include <winrt/windows.foundation.collections.h>

#include <mmdeviceapi.h>
#include <audiopolicy.h>

#include <wrl/client.h>

using namespace winrt::Windows::Media::Control;
using namespace winrt::Windows::Foundation;
using namespace winrt::Windows::Foundation::Collections;

template <typename T>
using ComPtr = Microsoft::WRL::ComPtr<T>;

class SpotifyMgr {
private:
	GlobalSystemMediaTransportControlsSessionManager msmgr;

	// Audio Session
	ComPtr<IMMDeviceEnumerator> deviceEnumerator;

	HRESULT GetAudioSession(IAudioSessionControl **out);
	GlobalSystemMediaTransportControlsSession GetSpotifySession();

public:
	SpotifyMgr();
	void HandleMessage(WlMessage msg);
};
