const DEX_URL = "https://api.dexscreener.com";

function check_and_throw(response)
{
    if (!response.ok)
    {
        throw new Error(`DEX Screener API returned ${response.status}`);
    }
}

export async function get_latest_token_profiles()
{
    const url = `${DEX_URL}/token-profiles/latest/v1`;
    const response = await fetch(url);

    check_and_throw(response);

    return response.json();
}

export async function get_recently_updated_token_profiles()
{
    const url = `${DEX_URL}/token-profiles/recent-updates/v1`;
    const response = await fetch(url);

    check_and_throw(response);

    return response.json();
}

export async function get_latest_token_community_takeovers()
{
    const url = `${DEX_URL}/community-takeovers/latest/v1`;
    const response = await fetch(url);

    check_and_throw(response);

    return response.json();
}

export async function get_latest_ads()
{
    const url = `${DEX_URL}/ads/latest/v1`;
    const response = await fetch(url);

    check_and_throw(response);

    return response.json();
}

export async function get_latest_boosted_tokens()
{
    const url = `${DEX_URL}/token-boosts/latest/v1`;
    const response = await fetch(url);
    
    check_and_throw(response);

    return response.json();
}

export async function get_tokens_most_active_boosts()
{
    const url = `${DEX_URL}/token-boosts/top/v1`;
    const response = await fetch(url);

    check_and_throw(response);

    return response.json();
}

export async function get_paid_order(chain_id, token_address)
{
    const url = `${DEX_URL}/orders/v1/${chain_id}/${token_address}`;
    const response = await fetch(url);

    check_and_throw(response);

    return response.json();
}

export async function get_pairs(chain_id, pair_id)
{
    const url = `${DEX_URL}/latest/dex/pairs/${chain_id}/${pair_id}`;
    const response = await fetch(url);

    check_and_throw(response);

    return response.json();
}

export async function search_for_pairs(query)
{
    const url = `${DEX_URL}/latest/dex/search?q=${query}`;
    const response = await fetch(url);

    check_and_throw(response);

    return response.json();
}

export async function get_token_pools(chain_id, token_address)
{
    const url = `${DEX_URL}/token-pairs/v1/${chain_id}/${token_address}`;
    const response = await fetch(url);

    check_and_throw(response);

    return response.json();
}

export async function get_token_pairs(chain_id, token_address)
{
    const url = `${DEX_URL}/tokens/v1/${chain_id}/${token_address}`;
    const response = await fetch(url);

    check_and_throw(response);

    return response.json();
}