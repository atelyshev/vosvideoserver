#pragma once
#include <vector>

namespace util
{
	class StringUtil
	{
	public:
		static std::wstring ToWstring(const std::string& str);
		static std::string ToString(const std::wstring& wstr);
		static std::string IntToHex(int32_t i);

		template<typename T>
		static void Tokenize(const T& str, std::vector<T>& tokens, const T& delimiters)
		{
			// Skip delimiters at beginning.
			T::size_type lastPos = str.find_first_not_of(delimiters, 0);
			// Find first "non-delimiter".
			T::size_type pos     = str.find_first_of(delimiters, lastPos);

			while (string::npos != pos || string::npos != lastPos)
			{
				// Found a token, add it to the vector.
				tokens.push_back(str.substr(lastPos, pos - lastPos));
				// Skip delimiters.  Note the "not_of"
				lastPos = str.find_first_not_of(delimiters, pos);
				// Find next "non-delimiter"
				pos = str.find_first_of(delimiters, lastPos);
			}
		}
	};
}
