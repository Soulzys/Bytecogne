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
const real_test_object = {_c: 77, ...test_object};
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

    run_app();
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