#pragma once

#include <Windows.h>
#include <stdexcept>

class HRException : std::exception
{
private:
	const HRESULT m_hr;

public:
	HRException(HRESULT hr) : m_hr(hr)
	{
	}

	HRESULT GetResult() const
	{
		return m_hr;
	}

	static void ThrowHR(HRESULT hr)
	{
		if (FAILED(hr))
		{
			throw HRException(hr);
		}
	}
};
