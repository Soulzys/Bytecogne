import { get_latest_token_profiles            } from "./dexscreener.js";
import { get_recently_updated_token_profiles  } from "./dexscreener.js";
import { get_latest_token_community_takeovers } from "./dexscreener.js";
import { get_latest_ads                       } from "./dexscreener.js";
import { get_latest_boosted_tokens            } from "./dexscreener.js";
import { get_tokens_most_active_boosts        } from "./dexscreener.js";
import { get_paid_order                       } from "./dexscreener.js";
import { get_pairs                            } from "./dexscreener.js";
import { search_for_pairs                     } from "./dexscreener.js";
import { get_token_pools                      } from "./dexscreener.js";
import { get_token_pairs                      } from "./dexscreener.js";

import { DATA } from "./utils.js";

const chain_id = "solana";
const token_address = "JUPyiwrYJFskUPiHa7hkeR8VUtAeFoSYbKedZNsDvCN";

const pairs = await get_token_pairs(chain_id, token_address);
const order = await get_paid_order(chain_id, token_address);

const first_pair = pairs[0];

console.log(first_pair);
console.log("~~~~~~~~~~~~~~~~~~~~~~~~~~~~");
console.log(first_pair.chainId);

const test_object = {
    chain_id: first_pair.chainId,
    url: first_pair.url,
    marketCap: first_pair.marketCap,
};

// _c stands for "code", i.e. the type of the data we're sending
//const real_test_object = {_c: DATA.DEX.TOKEN_PAIRS, ...test_object};
const real_test_object = {_c: DATA.DEX.TOKEN_PAIRS, ...test_object};
console.log("test_object: ", real_test_object);




console.log("--------------- My paid order: ", order);


import net from "node:net"
const socket = net.createConnection({
    host: "127.0.0.1",
    port: 5000
});



let connected = false;



socket.on("connect", () =>
{
    connected = true;
    console.log("Connected to C++");

    //run_app();
});

socket.on("close", () =>
{
    connected = false;
    console.log("C++ disconnected !");
    process.exit(1);
});

socket.on("error", err =>
{
    console.log("Socket error: ", err.message);
});


let buffer = '';
socket.on('data', (data) => {
    buffer += data.toString();
    let newline_index = -1;

    var message = "";

    while ((newline_index = buffer.indexOf('\n')) !== -1)
    {
        const message_chunk = buffer.slice(0, newline_index);
        buffer = buffer.slice(newline_index + 1);
        message += message_chunk + "\n";

        //console.log("C++ requested: ", message);
        //console.log("newline_index: ", newline_index);

        //if (message == "TOKEN_PAIRS")
        //{}


    }

    try
    {
        const obj = JSON.parse(message);
        console.log("Received: ", obj);
    }
    catch (err)
    {
        console.log("Invalid JSON: ", message, err);
    }
    
    //run_app();
});


const app_args = process.argv.slice(2); // The first 2 args are useless
if (app_args[0] == "--shutdown")
{
    process.exit(0);
}

function run_app()
{
    console.log("App started !");

    // JS -> C++
    const interval = setInterval(() => {
        if (!connected) return;

        const message = {value: Math.floor(Math.random() * 100)};
        socket.write(JSON.stringify(real_test_object) + "\n");
        console.log("Sent to C++: ", real_test_object);
    }, 3000);
}