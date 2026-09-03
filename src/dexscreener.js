const DEX_URL = "https://api.dexscreener.com";

export async function getTokenPairs(chainID, tokenAddress)
{
    const url = `${DEX_URL}/token-pairs/v1/${chainID}/${tokenAddress}`;
    const response = await fetch(url);

    if (!response.ok)
    {
        throw new Error(`DEX Screener API returned ${response.status}`);
    }

    return response.json();
}