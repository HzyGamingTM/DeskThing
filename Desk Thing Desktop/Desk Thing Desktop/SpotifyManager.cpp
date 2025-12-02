#include "SpotifyManager.hpp"

#include <iostream>
#include <mmdeviceapi.h>

#include "await.hpp"

template <typename T>
using ComPtr = Microsoft::WRL::ComPtr<T>;

using MediaSessionManager = winrt::Windows::Media::Control::GlobalSystemMediaTransportControlsSessionManager;
using MediaSession = winrt::Windows::Media::Control::GlobalSystemMediaTransportControlsSession;

#ifndef SAFE_RELEASE
#define SAFE_RELEASE(punk)  \
	if ((punk) != NULL)     \
	{                       \
		(punk)->Release();  \
		(punk) = NULL;      \
	}
#endif

HRESULT SpotifyMgr::GetAudioSession(IAudioSessionControl2 **out) {
	int pid, sessionCount = 0;
	HRESULT hr;

	// dont store these permanently in case they change over time
	ComPtr<IMMDevice> audioDevice;
	ComPtr<IAudioSessionManager2> audioSessionManager;
	ComPtr<IAudioSessionEnumerator> audioSessionEnumerator;

	// NOTE: code assumes deviceenumerator isn't null

	hr = deviceEnumerator->GetDefaultAudioEndpoint(eRender, eMultimedia, audioDevice.GetAddressOf());
	if (FAILED(hr)) return hr;

	hr = audioDevice->Activate(__uuidof(IAudioSessionManager2), CLSCTX_ALL, NULL, (void**)audioSessionManager.GetAddressOf());
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
			audioSessionControl2.CopyTo(out);
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
	msmgr = await msmgr.RequestAsync();

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

	std::cout << "Received message: " << id << " " << opcode << " " << size << std::endl;

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

	case 4: // Set volume (passed as u32 0-100)
	{
		uint32_t volumeFromMessage = msg.u32();
		float newVolume = volumeFromMessage * 0.01f;
		if (newVolume > 1)
			newVolume = 1;

		ComPtr<IAudioSessionControl2> audioSession;
		HRESULT hr = GetAudioSession(audioSession.GetAddressOf());
		if (hr != S_OK) {
			std::cout << "Couldn't get audio session, " << hr << std::endl;
			break;
		}

		ComPtr<ISimpleAudioVolume> volumeControl;
		hr = audioSession->QueryInterface(volumeControl.GetAddressOf());
		if (hr != S_OK) {
			std::cout << "Couldn't get volume control for session, " << hr << std::endl;
			break;
		}

		hr = volumeControl->SetMasterVolume(newVolume, 0);
		if (hr != S_OK) {
			std::cout << "Couldn't set mute state, " << hr << std::endl;
			break;
		}
	}
	break;

	case 5: // Mute / Unmute
	{
		ComPtr<IAudioSessionControl2> audioSession;
		HRESULT hr = GetAudioSession(audioSession.GetAddressOf());
		if (hr != S_OK) {
			std::cout << "Couldn't get audio session, " << hr << std::endl;
			break;
		}

		ComPtr<ISimpleAudioVolume> volumeControl;
		hr = audioSession->QueryInterface(volumeControl.GetAddressOf());
		if (hr != S_OK) {
			std::cout << "Couldn't get volume control for session, " << hr << std::endl;
			break;
		}

		BOOL currentlyMuted;
		hr = volumeControl->GetMute(&currentlyMuted);
		if (hr != S_OK) {
			std::cout << "Couldn't get mute state, " << hr << std::endl;
			break;
		}

		hr = volumeControl->SetMute(!currentlyMuted, 0);
		if (hr != S_OK) {
			std::cout << "Couldn't set mute state, " << hr << std::endl;
			break;
		}
	}
	break;
	}
}
