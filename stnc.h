//
// Created by super on 5/3/23.
//

#ifndef HW3_STNC_H
#define HW3_STNC_H

#include <stdio.h>
#include <netdb.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h> // read(), write(), close()
#include <arpa/inet.h>
#include <poll.h>
#include <signal.h>

/*
 * define - sizes that will be used in the program
 */
#define oneK 1024
#define chunk 65536
#define hundredMB 100 * 1024 * 1024

/*
 * S_FD_AMOUNT is the amount of file descriptors that the server will use - for poll
 * C_FD_AMOUNT is the amount of file descriptors that the client will use - for poll
 */
#define S_FD_AMOUNT 3
#define C_FD_AMOUNT 2
/*
 * define - timeout for poll
 */
#define TIMEOUT 5000

/*
 * define - all secondary channels in the program will use port 8080
 */
#define PARTB_PORT 8080

/*
 * number for each protocol
 */
#define TCP_IPV4 1
#define TCP_IPV6 2
#define UDP_IPV4 3
#define UDP_IPV6 4
#define MMAP 5
#define UDS_STREAM 6
#define UDS_DGRAM 7
#define PIPELINE 8

/*
 * paths for the unix domain sockets, pipe and mmap
 */
#define SOCK_PATH  "tpf_unix_sock.server"
#define FILE_INPUT_OUTPUT_NAME "fifo_mifo"
#define SERVER_DGRAM_PATH  "tpf_unixdgram_sock.server"
#define SERVER_PATH "tpf_unix_sock.server"
#define CLIENT_PATH "tpf_unix_sock.client"

/**
 * first message format - contains the protocol and the checksum of the file
 * it's the first message that the client sends to the server
 */
typedef struct first_message_format{
    int protocol;
    int checksum;
}first_message;

/**
 * @brief - prints the usage of the program
 */
void print_usage();

/**
 * @brief - frees the socket descriptors of the poll and exits the program
 * s_suicide_mode()- signal handling for server
 * c_suicide_mode()- signal handling for client
 */
void s_suicide_mode();
void c_suicide_mode();

/**
 * @brief - signal handling for server and client
 */
void server_sig_handler(int signo);
void client_sig_handler(int signo);

/**
 * @brief - returns the right type of struct sockaddr.
 * @return - sockaddr_in for ipv4
 * @return - sockaddr_in6 for ipv6
 */
void* get_in_addr(struct sockaddr *sa);

/**
 * @param pfds - fd array
 * @param newfd
 * @param fd_count - counter of file descriptors
 *
 * @brief adds new file descriptor to the poll array
 */
void add_to_pfds(struct pollfd* pfds[], int newfd, int *fd_count);

/**
 * @param pfds - fd array
 * @param i - index
 * @param fd_count - counter of file descriptors
 *
 * @brief remove file descriptor in index i from the poll array
 */
void del_from_pfds(struct pollfd* pfds[], int i, int *fd_count);

/**
 * @param address - arguments list from main
 *
 * @brief main server function of part a
 */
void server(char *address[]);

/**
 * @param address - arguments list from main
 *
 * @brief main client function of part a
 */
void client(char *address[]);

/**
 * @brief - returns the number of the protocol according to the input
 * @param sock_type
 * @param protocol
 *
 * @return int - number of the protocol
 */
int socktype_to_num(char* sock_type, char* protocol);

/**
 * @brief - the main server function of part b
 * @param address
 */
void partb_server(char *address[]);

/**
 * @brief - the main client function of part b
 * @param address
 */
void partb_client(char *address[]);

/**
 * @brief - server function for sending 100MB file in ipv4 tcp - part b
 *
 * @return long - time it took to send the file
 */
long s_tcp_ipv4_channel();

/**
 * @brief - server function for sending 100MB file in ipv6 tcp - part b
 *
 * @return long - time it took to send the file
 */
long s_tcp_ipv6_channel();

/**
 * @brief - server function for sending 100MB file in ipv4 udp - part b
 *
 * @return long - time it took to send the file
 */
long s_udp_ipv4_channel();

/**
 * @brief - server function for sending 100MB file in uds stream - part b
 *
 * @return long - time it took to send the file
 */
long s_uds_stream_channel();

/**
 * @brief - server function for sending 100MB file in uds dgram - part b
 *
 * @return long - time it took to send the file
 */
long s_uds_dgram_channel();

/**
 * @brief - server function for sending 100MB file in ipv6 udp - part b
 *
 * @return long - time it took to send the file
 */
long s_udp_ipv6_channel();

/**
 * @brief - server function for sending 100MB file in pipeline - part b
 *
 * @return long - time it took to send the file
 */
long s_pipeline_channel();

/**
 * @brief - server function for sending 100MB file in mmap - part b
 *
 * @return long - time it took to send the file
 */
long s_mmap_channel();

/**
 * @brief - client function for sending 100MB file in ipv4 tcp - part b
 */
void c_tcp_ipv4_channel(char* address[]);

/**
 * @brief - client function for sending 100MB file in ipv6 tcp - part b
 */
void c_tcp_ipv6_channel(char* address[]);

/**
 * @brief - client function for sending 100MB file in ipv4 udp - part b
 */
void c_udp_ipv4_channel(char* address[]);

/**
 * @brief - client function for sending 100MB file in uds stream - part b
 */
void c_uds_stream_channel(char* address[]);

/**
 * @brief - client function for sending 100MB file in uds dgram - part b
 */
void c_uds_dgram_channel(char* address[]);

/**
 * @brief - client function for sending 100MB file in ipv6 udp - part b
 */
void c_udp_ipv6_channel(char* address[]);

/**
 * @brief - client function for sending 100MB file in pipeline - part b
 */
void c_pipeline_channel(char* address[]);

/**
 * @brief - client function for sending 100MB file in mmap - part b
 */
void c_mmap_channel(char* address[]);

#endif //HW3_STNC_H

