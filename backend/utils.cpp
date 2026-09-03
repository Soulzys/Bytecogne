#include "utils.h"

#define INVALID_UI32 UINT32_MAX
#define INVALID_UI8 UINT8_MAX

bool
utils::dex::parse_data(const std::string& msg, void* outData, EndPoint& outDataType)
{
	size_t start = msg.find(':');
	size_t end   = msg.find(',');
	uint32 type  = str_to_ui32(msg.c_str(), start + 1, end);
	
	// >NOTE: could probably remove INVALID_UI32 check, but it may be useful for debugging
	if (type == INVALID_UI32)               return false;
	if (type > (uint32)utils::dex::EndPoint::COUNT) return false;

	switch (type)
	{
		case utils::dex::EndPoint::TOKEN_PAIRS:
		{
			
		} break;
	}

	return true;
}

utils::dex::TokenPairs 
utils::dex::parse_token_pairs(const std::string& msg)
{
	TokenPairs result = {};

	


	return result;
}


uint32 utils::str_to_ui32(const char* str, size_t start, size_t end)
{
	if (start >= end) return INVALID_UI32;

	uint32 result = 0;
	const char* strCpy = str + start;

	while (start < end)
	{
		uint8 digit = char_to_digit(*strCpy);
		if (digit == INVALID_UI8) return INVALID_UI32;

		result = result * 10 + digit;
		strCpy++;
		start++;
	}

	return result;
}


uint8 utils::char_to_digit(const char c)
{
	if (!is_digit(c)) return INVALID_UI8;

	return (uint8)(c - '0');
}


bool utils::is_digit(const char c)
{
	int val = (int)c;
	return (val >= 48 && val <= 57);
}