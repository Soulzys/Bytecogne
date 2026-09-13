namespace utils
{
	namespace dex
	{
		enum class EndPoint : uint8
		{
			TOKEN_PAIRS = 0,
			COUNT
		};

		struct TokenPairs
		{
			std::string chain_id;
			std::string url;
			int         market_cap;
		};

		TokenPairs parse_token_pairs(const char* msg, uint32 size);
		uint32     retrieve_message_code(const char* msg, uint32 size);// , void* outData, EndPoint& outDataType);
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