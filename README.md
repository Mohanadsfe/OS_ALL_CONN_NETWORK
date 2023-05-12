# OS- HW3
For Computer Science B.S.c Ariel University.

3'rd assignment in Operation Systems course. 

## Authors:
Mohanad Safi 208113381

Shahar Zaidel 211990700

# Part A

# Chat Application

## Description
this app is a chat application that allows two users to communicate with each other using TCP protocol.
the app is written in C language. 
this app uses the poll() function to handle multiple streams -
in this way, the client can send and receive messages at the same time.
client - reads from the keyboard and sends the message to the server.
also, reads from the server's port and prints the message to the screen.
server - reads from the keyboard and sends the message to the client.
also, reads from the client's port and prints the message to the screen.

notice - the server must be run before the client.
notice - the client and the server talk on the same port, received as an argument from the client and the server.
so, the client and the server must be run with the same port number, or it won't work.

## Requirements
- Full linux environment - not tested on windows or wsl
- GCC compiler

## How to run
`The client side: stnc -c IP PORT`

`The server side:stnc -s PORT`

the communication is done using IPv4 TCP protocol.

# Part B

## network performance test tool

## Description
this app is a network performance test tool:
the client generates a random 100MB chunk of data and sends it to the server.
the client can send it in 8 different types of communication styles:
- TCP IPv4
- TCP IPv6
- UDP IPv4
- UDP IPv6
- Unix Domain Socket (UDS) :stream
- Unix Domain Socket (UDS) :datagram
- Pipeline
- Mmap

notice! - run pipe option on the server side with sudo, example in the photos.  

the server receives the data and prints the time it took to receive the data.

## How to run
`The client side: stnc -c IP PORT -p <type> <param>`

for quiet mode: 

`The server side: stnc -s PORT -p -q` - only prints the time it took to receive the data.

### Client side functions: 

- void c_tcp_ipv4_channel(char* address[]);

![image](https://github.com/Mohanadsfe/OS_ALL_CONN_NETWORK/assets/117997542/8174b7f6-7013-4197-8839-9625fcaaa049)

- void c_tcp_ipv6_channel(char* address[]);

![image](https://github.com/Mohanadsfe/OS_ALL_CONN_NETWORK/assets/117997542/d8c0fd70-6738-46ff-a4b5-b3482e4f6119)

- void c_udp_ipv4_channel(char* address[]);

![image](https://github.com/Mohanadsfe/OS_ALL_CONN_NETWORK/assets/117997542/6c9e9b2e-9332-4a1e-b101-4cb898d23078)

- void c_udp_ipv6_channel(char* address[]);

![image](https://github.com/Mohanadsfe/OS_ALL_CONN_NETWORK/assets/117997542/9b392444-8bb9-448d-90ef-0b14e4637da6)
 
- void c_uds_stream_channel(char* address[]);

![image](https://github.com/Mohanadsfe/OS_ALL_CONN_NETWORK/assets/117997542/86cfac74-cc07-40ad-9282-97a1117c4af1)

- void c_uds_dgram_channel(char* address[]);

![image](https://github.com/Mohanadsfe/OS_ALL_CONN_NETWORK/assets/117997542/1bd3a82c-2b4c-47d8-8f6c-26e249835794)

- void c_pipeline_channel(char* address[]);

![image](https://github.com/Mohanadsfe/OS_ALL_CONN_NETWORK/assets/117997542/5e628a28-cc7f-4a5a-b6a3-8d5c7a18909f)

- void c_mmap_channel(char* address[]);

![image](https://github.com/Mohanadsfe/OS_ALL_CONN_NETWORK/assets/117997542/30782a76-efa0-42f0-84b5-75aeb6188e08)


# Server side functions:

- long s_tcp_ipv4_channel();

![image](https://github.com/Mohanadsfe/OS_ALL_CONN_NETWORK/assets/117997542/b80632d0-8fa3-4a5b-a5fb-1e7f8682ad6c)

- long s_tcp_ipv6_channel();

![image](https://github.com/Mohanadsfe/OS_ALL_CONN_NETWORK/assets/117997542/feff3258-af96-4ce6-b6ca-94930bec16d3)

- long s_udp_ipv4_channel();

![image](https://github.com/Mohanadsfe/OS_ALL_CONN_NETWORK/assets/117997542/d2b42557-38b9-42b8-a2a1-abeb5d151dce)

- long s_udp_ipv6_channel();

![image](https://github.com/Mohanadsfe/OS_ALL_CONN_NETWORK/assets/117997542/82dd4eb4-39ff-4efa-af0f-4ba7c894d24c)

- long s_uds_stream_channel();

![image](https://github.com/Mohanadsfe/OS_ALL_CONN_NETWORK/assets/117997542/7c02ff75-e457-4f8b-b0b1-c6309a7f1ef3)

- long s_uds_dgram_channel();

![image](https://github.com/Mohanadsfe/OS_ALL_CONN_NETWORK/assets/117997542/8130b17c-251c-4852-9eb4-ea5bc36799d5)

- long s_pipeline_channel();

![image](https://github.com/Mohanadsfe/OS_ALL_CONN_NETWORK/assets/117997542/9a3c1ce9-4b1b-4932-bc46-f78d76f94a6a)

- long s_mmap_channel();

![image](https://github.com/Mohanadsfe/OS_ALL_CONN_NETWORK/assets/117997542/932200de-f35f-46ca-8a7d-59cfbf6275b1)
