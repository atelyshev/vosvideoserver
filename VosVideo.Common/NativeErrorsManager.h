#pragma once
#include <Mferror.h>
#include <WinHttp.h>
#include <unordered_map>

namespace util
{
	// Provides conversion between HRESULT and error message
	class NativeErrorsManager
	{
	public:
		static const std::wstring& ToString(int32_t hr);

	protected:
		using NativeErrorCodesMap = std::unordered_map<int32_t, std::wstring>;
		static const NativeErrorCodesMap errorCodes_;
	};
}
