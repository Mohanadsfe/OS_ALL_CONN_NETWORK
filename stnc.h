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

#define oneK 1000
#define S_FD_AMOUNT 3
#define C_FD_AMOUNT 2
#define chunk 64000
#define hundredMB 100 * 1024 * 1024
#define PARTB_PORT 8080

#define TCP_IPV4 1
#define TCP_IPV6 2
#define UDP_IPV4 3
#define UDP_IPV6 4
#define MMAP 5
#define UDS_STREAM 6
#define UDS_DGRAM 7
#define PIPELINE 8

#define TIMEOUT 5000
#define SOCK_PATH  "tpf_unix_sock.server"
#define FILE_INPUT_OUTPUT_NAME "fifo_mifo"
#define SERVER_DGRAM_PATH  "tpf_unixdgram_sock.server"
#define SERVER_PATH "tpf_unix_sock.server"
#define CLIENT_PATH "tpf_unix_sock.client"

typedef struct first_message_format{
    int protocol;
    int checksum;
}first_message;

void debug(char* message);
void debug_num(char* message, int num);
void print_usage();

void s_suicide_mode();
void c_suicide_mode();

void server_sig_handler(int signo);
void client_sig_handler(int signo);

void* get_in_addr(struct sockaddr *sa);
void add_to_pfds(struct pollfd* pfds[], int newfd, int *fd_count);
void del_from_pfds(struct pollfd* pfds[], int i, int *fd_count);

void server(char *address[]);
void client(char *address[]);

int socktype_to_num(char* sock_type, char* protocol);

void partb_server(char *address[]);
void partb_client(char *address[]);

long s_tcp_ipv4_channel();
long s_tcp_ipv6_channel();
long s_udp_ipv4_channel();
long s_uds_stream_channel();
long s_uds_dgram_channel();
long s_udp_ipv6_channel();
long s_pipeline_channel();
long s_mmap_channel();

void c_tcp_ipv4_channel(char* address[]);
void c_tcp_ipv6_channel(char* address[]);
void c_udp_ipv4_channel(char* address[]);
void c_uds_stream_channel(char* address[]);
void c_uds_dgram_channel(char* address[]);
void c_udp_ipv6_channel(char* address[]);
void c_pipeline_channel(char* address[]);
void c_mmap_channel(char* address[]);

#endif //HW3_STNC_H
