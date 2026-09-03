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

		TokenPairs parse_token_pairs(const std::string& msg);
		bool parse_data(const std::string& msg, void* outData, EndPoint& outDataType);
	}


	/**
	* @start : first digit of number
	* @end   : first char after last digit
	*/
	uint32 str_to_ui32(const char* str, size_t start, size_t end);
	uint8  char_to_digit(const char c);
	bool   is_digit(const char c);
}