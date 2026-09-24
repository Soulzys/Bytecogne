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


let test_paid_order;

let buffer = '';
// Data can be called several times to send one complete
socket.on('data', async (data) => {
    buffer += data.toString();
    let newline_index = -1;

    while ((newline_index = buffer.indexOf('\n')) !== -1)
    {
        const message = buffer.slice(0, newline_index);
        buffer = buffer.slice(newline_index + 1);
    
        try
        {
            const obj = JSON.parse(message);
            console.log("Received: ", obj);

            // >TODO: 
            if (obj["dt"] == "paid_order")
            {
                console.log("Fetching PaidOrder from DEX Screener...");
                test_paid_order = await get_paid_order(chain_id, token_address);

                console.log("Received PaidOrder from DEX Screener...");
                console.log(test_paid_order);

                console.log("Preparing PaidOrder command for backend...");

                const response_obj = {};
                response_obj.obj_type = "paid_order";
                if (obj["type"] == 1)
                {
                    // Can also get properties via ["<property_name>"], e.g.
                    // test_paid_order.orders[0]["type"];
                    response_obj.type = test_paid_order.orders[0].type;
                }
                if (obj["status"] == 1)
                {
                    response_obj.type = test_paid_order.orders[0].status;
                }
                if (obj["payment_timestamp"] == 1)
                {
                    response_obj.payment_timestamp = test_paid_order.orders[0].paymentTimestamp;
                }
                
                console.log("Sending to backend...");
                console.log(response_obj);
                socket.write(JSON.stringify(response_obj) + "\n");
            }
        }
        catch (err)
        {
            console.log("Invalid JSON: ", message, err);
            // >TODO: Return something to C++
            return;
        }
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