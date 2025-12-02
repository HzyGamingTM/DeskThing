#include "SpotifyManager.hpp"

#include <iostream>
#include <mmdeviceapi.h>

#include "await.hpp"

using namespace winrt::Windows::Media::Control;
using namespace winrt::Windows::Foundation;
using namespace winrt::Windows::Foundation::Collections;

using MediaSessionManager = GlobalSystemMediaTransportControlsSessionManager;
using MediaSession = GlobalSystemMediaTransportControlsSession;

#ifndef SAFE_RELEASE
#define SAFE_RELEASE(punk)  \
    if ((punk) != NULL)     \
    {                       \
        (punk)->Release();  \
        (punk) = NULL;      \
    }
#endif

HRESULT SpotifyMgr::GetAudioSession(IAudioSessionControl **out) {
	int pid, sessionCount = 0;
	HRESULT hr;

	// dont store these permanently in case they change over time
	ComPtr<IMMDevice> audioDevice;
	ComPtr<IAudioSessionManager2> audioSessionManager;
	ComPtr<IAudioSessionEnumerator> audioSessionEnumerator;

	// NOTE: code assumes deviceenumerator isn't null

	hr = deviceEnumerator->GetDefaultAudioEndpoint(eRender, eMultimedia, audioDevice.GetAddressOf());
	if (FAILED(hr)) return hr;

	hr = audioDevice->Activate(__uuidof(IAudioSessionManager2), CLSCTX_ALL, NULL, (void**)&audioSessionManager);
	if (FAILED(hr)) return hr;
	
	hr = audioSessionManager->GetSessionEnumerator(audioSessionEnumerator.GetAddressOf());
	if (FAILED(hr)) return hr;

	hr = audioSessionEnumerator->GetCount(&sessionCount);
	if (FAILED(hr)) return hr;

	for (int i = 0; i < sessionCount; i++) {
		ComPtr<IAudioSessionControl> audioSessionControl;
		hr = audioSessionEnumerator->GetSession(i, &audioSessionControl);
		if (FAILED(hr)) {
			printf("Failed to get an audio session, hr %ld\n", hr);
			continue;
		}

		ComPtr<IAudioSessionControl2> audioSessionControl2;
		hr = audioSessionControl.As(&audioSessionControl2);
		if (FAILED(hr)) {
			printf("Failed to get audio session v2, hr %ld\n", hr);
			continue;
		}

		wchar_t *sessionId;
		audioSessionControl2->GetSessionIdentifier(&sessionId);
		std::wstring sessionIdString(sessionId);
		CoTaskMemFree(sessionId);

		if (sessionIdString.find(L"Spotify.exe") != sessionIdString.npos) {
			audioSessionControl.CopyTo(out);
			return S_OK;
		}
	}

	return S_FALSE;
}

MediaSession SpotifyMgr::GetSpotifySession() {
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

	if (!iter.HasCurrent())
		return NULL;

	return iter.Current();
}

SpotifyMgr::SpotifyMgr() : msmgr(NULL) {
	msmgr = await GlobalSystemMediaTransportControlsSessionManager::RequestAsync();

	HRESULT hr = CoCreateInstance(
		__uuidof(MMDeviceEnumerator), NULL,
		CLSCTX_ALL, __uuidof(IMMDeviceEnumerator),
		(void**)deviceEnumerator.GetAddressOf()
	);

	if (hr != S_OK) {
		printf("Failed to create IMMDeviceEnumerator, hr %ld\n", hr);
		return;
	}
}

void SpotifyMgr::HandleMessage(WlMessage msg) {
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
		auto session = GetSpotifySession();
		if (session) session.TryTogglePlayPauseAsync();
		else std::cout << "Couldn't get session\n" << std::endl;
	}
	break;

	case 2: // Next
	{
		auto session = GetSpotifySession();
		if (session) session.TrySkipNextAsync();
		else std::cout << "Couldn't get session\n" << std::endl;
	}
	break;

	case 3: // Previous
	{
		auto session = GetSpotifySession();
		if (session) session.TrySkipPreviousAsync();
		else std::cout << "Couldn't get session\n" << std::endl;
	}
	break;
	}
}
