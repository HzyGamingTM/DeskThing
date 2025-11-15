#ifndef await_hpp
#define await_hpp

#include <winrt/windows.foundation.h>
#include <stdexcept>
#include <chrono>
#define await _m_await<< // Await in C++ real


static struct _m_await_class {
	template <typename T>
	T operator<<(winrt::Windows::Foundation::IAsyncOperation<T> asy) {
		if (asy.wait_for(std::chrono::seconds(5)) == winrt::Windows::Foundation::AsyncStatus::Completed)
			return asy.GetResults();

		throw std::runtime_error("bullshit happned");
	}
} _m_await;

#endif
