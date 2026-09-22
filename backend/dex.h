#pragma once


enum class EndPoint : enum_type
{
	PaidOrder, 
	TokenPair,
};

struct Token
{
	const char* address;
	const char* name;
	const char* symbol;
};

struct TokenPair
{
	struct Liquidity
	{
		real32 usd;
		real32 base;
		real32 quote;
	};

	struct Info
	{
		const char* image_url;
		const char* websites[5];
		const char* socials[5];
	};

	const char* chain_id;
	const char* dex_id;
	const char* url;
	const char* pair_address;
	const char* labels[5];
	Token       base_token;
	Token       quote_token;
	const char* price_native;
	const char* price_usd;
	// txns
	// volume
	// price change
	real32 fdv;
	real32 market_cap;
	int32  pair_credit_at;
	// boosts
};


struct PaidOrder
{
	enum class Properties : flag_type
	{
		TYPE              = 1 << 0,
		STATUS            = 1 << 1,
		PAYMENT_TIMESTAMP = 1 << 2
	};

	enum class Type : enum_type
	{
		TOKEN_PROFILE,
		COMMUNITY_TAKEOVER,
		TOKEN_AD,
		TRENDING_BAR_AD
	};

	enum class Status : enum_type
	{
		PROCESSING,
		CANCELLED,
		ON_HOLD,
		APPROVED,
		REJECTED
	};


	// Outputs
	Type   type;
	Status status;
	int32  payment_timestamp;

	// Inputs
	char chain_id      [API_PARAM_BSIZE];
	char token_address [API_PARAM_BSIZE];

	Properties internal_properties;
};

template<typename E>
bool has_flag(E flags, E flag)
{
	return ((flag_type)(flags) & (flag_type)flag) != 0;
}

struct Dex
{
	PaidOrder paid_order;
};

std::string stringify(const PaidOrder& data);
std::string stringify(EndPoint e);
std::string stringify(PaidOrder::Type e);
std::string stringify(PaidOrder::Status e);
std::string stringify(int32 data);
std::string stringify(uint32 flags, const std::string flags_name[FLAGS_COUNT], const std::string& prefix = "");

std::string format_to_node(const PaidOrder& data);

void dex_paid_order_popup(PaidOrder& data);
