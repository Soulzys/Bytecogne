#include "utils.h"

#define INVALID_UI32 UINT32_MAX
#define INVALID_UI8 UINT8_MAX

uint32
utils::dex::retrieve_message_code(const char* msg, uint32 size)
{
	uint32 start = find_char(msg, size, ':'); // This should be at a constant position specified by the format, so no need to look for it (in theory)
	uint32 end   = find_char(msg, size, ','); 

	if (start == INVALID_UI32) return false;
	if (end   == INVALID_UI32) return false;

	return str_to_ui32(msg, start + 1, end);
}

utils::dex::TokenPairs 
utils::dex::parse_token_pairs(const char* msg, uint32 size)
{
	TokenPairs result = {};

	


	return result;
}


uint32 utils::str_to_ui32(const char* str, uint32 start, uint32 end)
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

uint32 utils::find_char(const char* str, uint32 size, char c)
{
	for (uint32 i = 0; i < size; i++)
	{
		if (str[i] == c)
		{
			return i;
		}
	}

	return INVALID_UI32;
}