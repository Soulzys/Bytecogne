#pragma once

// Format to use StringView with prinf
#define SV_FMT "%.*s"
#define SV_ARG(s) (int)(s).size, (s).data

namespace utils
{
	namespace dex
	{
		//enum class EndPoint : uint8
		//{
		//	TOKEN_PAIRS = 0,
		//	COUNT
		//};
		//
		//struct TokenPairs
		//{
		//	std::string chain_id;
		//	std::string url;
		//	int         market_cap;
		//};
		//
		//TokenPairs parse_token_pairs(const char* msg, uint32 size);
		//uint32     retrieve_message_code(const char* msg, uint32 size);// , void* outData, EndPoint& outDataType);
	}


	namespace strv
	{
		struct StringView
		{
			const char* data;
			size_t size;
		};

		StringView create(const char* str);
		void chop_left(StringView* strv, size_t amount);
		void chop_right(StringView* strv, size_t amount);
		// Returns the chopped part
		StringView split(StringView* strv, char delim);
	}


	/**
	* @start : first digit of number
	* @end   : first char after last digit
	*/
	uint32 str_to_ui32(const char* str, uint32 start, uint32 end);
	uint32 find_char(const char* str, uint32 size, char c);
	uint8  char_to_digit(const char c);
	bool   is_digit(const char c);
}