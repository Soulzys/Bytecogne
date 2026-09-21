import { getTokenPairs } from "./dexscreener.js";
import { DATA } from "./utils.js";

const chain_name = "solana";
const token_address = "JUPyiwrYJFskUPiHa7hkeR8VUtAeFoSYbKedZNsDvCN";

const pairs = await getTokenPairs(chain_name, token_address);

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

    while ((newline_index = buffer.indexOf('\n')) !== -1)
    {
        const message = buffer.slice(0, newline_index);
        buffer = buffer.slice(newline_index + 1);
        console.log("C++ requested: ", message);
        console.log("newline_index: ", newline_index);

        //if (message == "TOKEN_PAIRS")
        //{}

    }
    run_app();
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