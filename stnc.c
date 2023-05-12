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
#include <errno.h>
#include <sys/un.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <math.h>    
#include <time.h>  
#include <sys/mman.h>
#include <stdbool.h>
#include "stnc.h"

struct pollfd* s_pfds;
struct pollfd* c_pfds;
bool quiet_mode = false;


int main(int argc , char*argv[]){
    if(argc <= 7 && argc >= 5){
        //stnc -c IP PORT -p <type> <param>
        //stnc -s port -p (p for performance test) -q (q for quiet)
        if (strcmp(argv[1], "-c") == 0 && strcmp(argv[4], "-p") == 0) {
            partb_client(argv);
        }
        else if (strcmp(argv[1], "-s") == 0 && strcmp(argv[3], "-p") == 0) {
            if(argc>=5){
                if(strcmp(argv[4], "-q") == 0){
                    quiet_mode = true;
                }
            }
            partb_server(argv);
        }
        else {
            print_usage();
            return 0;
        }
    }
    else if(argc == 3 || argc == 4){
        if (strcmp(argv[1], "-c") == 0) {
            client(argv);
        }
        else if (strcmp(argv[1], "-s") == 0) {
            server(argv);
        }
        else {
            print_usage();
            return 0;
        }
        return 0;
    }
    else{
        print_usage();
        return 0;
    }
}

void print_usage(){
    printf("Usage: stnc -c IP PORT\n");
    printf("Usage: stnc -s PORT\n");
    printf("for test mode: \n");
    printf("Usage for client: stnc -c IP PORT -p <type> <param>\n");
    printf("Usage for server: stnc -s port -p (p for performance test) -q (q for quiet)\n");
}

void s_suicide_mode(){
    for (int i = 0; i < S_FD_AMOUNT; ++i) {
        close(s_pfds[i].fd);
    }
    free(s_pfds);
    exit(1);
}

void c_suicide_mode(){
    for (int i = 0; i < C_FD_AMOUNT; ++i) {
        close(c_pfds[i].fd);
    }
    free(c_pfds);
    exit(1);
}

void server_sig_handler(int signo){
    s_suicide_mode();
}

void client_sig_handler(int signo){
    c_suicide_mode();
}

void* get_in_addr(struct sockaddr *sa){
    if(sa->sa_family == AF_INET){
        return &(((struct sockaddr_in*)sa)->sin_addr);
    }
    return NULL;
}

// Add a new file descriptor to the set
void add_to_pfds(struct pollfd* pfds[], int newfd, int *fd_count){
    (*pfds)[*fd_count].fd = newfd;
    (*pfds)[*fd_count].events = POLLIN; // Check ready-to-read

    (*fd_count)++;
}

// Remove an index from the set
void del_from_pfds(struct pollfd* pfds[], int i, int *fd_count){
    close((*pfds)[i].fd);
    // Copy the one from the end over this one
    (*pfds)[i] = (*pfds)[*fd_count-1];
    (*fd_count)--;
}


void server(char *address[]){
    //signals for server
    signal(SIGINT, server_sig_handler);

    //address[0] = ./stnc, address[1] = "-s", address[2] = port
    int yes = 1;
    struct sockaddr_in servaddr, cliaddr; // Client address
    socklen_t addrlen = sizeof(cliaddr);

    s_pfds = (struct pollfd*)malloc(sizeof(struct pollfd)*S_FD_AMOUNT);
    if(s_pfds == NULL){
        perror("server side: malloc pfds");
        exit(4);
    }
    int fd_count = 0; // stdin, listener
    add_to_pfds(&s_pfds, STDIN_FILENO, &fd_count);

    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if(sockfd == -1){
        perror("server side: socket");
        exit(1);
    }

    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET; // IPv4
    servaddr.sin_addr.s_addr = INADDR_ANY; // Any address
    servaddr.sin_port = htons(atoi(address[2])); // Port

    if(setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1){
        perror("server side: setsockopt");
        exit(1);
    }

    if(bind(sockfd, (struct sockaddr *)&servaddr, sizeof(servaddr)) == -1){
        perror("server side: bind");
        exit(1);
    }

    add_to_pfds(&s_pfds, sockfd, &fd_count);

    while(1){
        // Listen
        if (listen(sockfd, 1) == -1) {
            perror("server side: listen");
            s_suicide_mode();
        }
        printf("Server: waiting for connections...\n");

        int newfd = accept(sockfd,(struct sockaddr *)&cliaddr ,&addrlen);
        if (newfd == -1) {
            perror("server side: accept");
            s_suicide_mode();
        }
        add_to_pfds(&s_pfds, newfd, &fd_count);

        // Main loop
        while(1){
            int poll_count = poll(s_pfds, fd_count, -1);
            if (poll_count == -1) {
                perror("server side: poll");
                s_suicide_mode();
            }
            if (s_pfds[0].revents & POLLIN) { // We got one from stdin
                int bsent = 0;
                char read_buf[oneK]; // 1K buffer
                memset(read_buf, 0, sizeof(char) * oneK);
                int bread = (int) read(s_pfds[0].fd, read_buf, sizeof(read_buf));
                if (bread == -1) {
                    perror("server side: read");
                    s_suicide_mode();
                }
                else {
                    // We got some good data from the keyboard
                    char send_buf[bread - bsent + 1];
                    memset(send_buf, 0, sizeof(char)*(bread - bsent + 1));
                    strncpy(send_buf, read_buf + bsent, bread - bsent);
                    send_buf[bread - bsent] = '\0';
                    bsent = (int)send(s_pfds[2].fd, send_buf, sizeof(send_buf), 0);
                    if (bsent == -1 || bsent == 0 || bsent < sizeof(send_buf)) {
                        perror("server side: send");
                        s_suicide_mode();
                    }
                }
            }
            else if(s_pfds[2].revents & POLLIN){
                char recv_buf[oneK];
                int brecv = (int)recv(s_pfds[2].fd, recv_buf, oneK, 0);
                if(brecv == -1){
                    perror("server side: recv");
                    s_suicide_mode();
                }
                else if(brecv == 0){
                    printf("client left the conversation\n");
                    del_from_pfds(&s_pfds, 2, &fd_count);
                    break;
                }
                else{
                    recv_buf[brecv] = '\0';
                    printf("Client: %s", recv_buf);
                }

            }
        }
    }
 }

////////////////////////////////////////////////////////////////////////

void client(char *address[]){
    //signals for client
    signal(SIGINT, client_sig_handler);
    int yes = 1;

    //address[0] = ./stnc, address[1] = "-c", address[2] = ip, address[3] = port
    c_pfds = (struct pollfd*)malloc(sizeof(struct pollfd)*C_FD_AMOUNT);
    if(c_pfds == NULL){
        perror("client side: malloc pfds");
        exit(4);
    }
    int fd_count = 0; // stdin, client socket

    add_to_pfds(&c_pfds, STDIN_FILENO, &fd_count);

    int c_sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (c_sockfd == -1) {
        perror("client side: socket");
        exit(1);
    }

    if(setsockopt(c_sockfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1){
        perror("client side: setsockopt");
        exit(1);
    }

    struct sockaddr_in c_addr;
    memset(&c_addr, 0, sizeof(struct sockaddr_in));
    c_addr.sin_family = AF_INET;
    c_addr.sin_port = htons(atoi(address[3]));
    inet_pton(AF_INET, address[2], &c_addr.sin_addr);

    if (connect(c_sockfd, (struct sockaddr *)&c_addr, sizeof(c_addr)) == -1) {
        perror("client side: connect");
        exit(1);
    }

    add_to_pfds(&c_pfds, c_sockfd, &fd_count);

    while(1){
        int poll_count = poll(c_pfds, fd_count, -1);
        if (poll_count == -1) {
            perror("client side: poll");
            c_suicide_mode();
        }
        if (c_pfds[0].revents & POLLIN) { // We got one from stdin
            int bsent = 0;
            char read_buf[oneK]; // 1K buffer
            memset(read_buf, 0, sizeof(char) * oneK);
            int bread = (int) read(c_pfds[0].fd, read_buf, sizeof(read_buf));
            if (bread == -1) {
                perror("client side: read");
                c_suicide_mode();
            }
            else {
                // We got some good data from the keyboard
                char send_buf[bread - bsent + 1];
                memset(send_buf, 0, sizeof(char)*(bread - bsent + 1));
                strncpy(send_buf, read_buf + bsent, bread - bsent);
                send_buf[bread - bsent] = '\0';
                bsent = (int)send(c_pfds[1].fd, send_buf, sizeof(send_buf), 0);
                if (bsent == -1 || bsent == 0 || bsent < bread) {
                    perror("client side: send");
                    c_suicide_mode();
                }
            }
        }
        else if(c_pfds[1].revents & POLLIN){
            char recv_buf[oneK];
            int brecv = (int)recv(c_pfds[1].fd, recv_buf, oneK, 0);
            if(brecv == -1){
                perror("client side: recv");
                c_suicide_mode();
            }
            else if(brecv == 0){
                printf("server left the conversation\n");
                c_suicide_mode();
            }
            else{
                recv_buf[brecv] = '\0';
                printf("Server: %s", recv_buf);
            }

        }
    }
}

////////////////////////////////////////////////////////////////////
//part b
int socktype_to_num(char* sock_type, char* protocol){
    if(!strcmp(sock_type, "mmap")){
        return MMAP;
    }
    if(!strcmp(sock_type, "pipe")){
        return PIPELINE;
    }
    if(!strcmp(protocol, "tcp")){
        if(!strcmp(sock_type, "ipv4")){
            return TCP_IPV4;
        }
        else return TCP_IPV6;
    }

    if(!strcmp(protocol, "udp")){
        if(!strcmp(sock_type, "ipv4")){
            return UDP_IPV4;
        }
        else return UDP_IPV6;
    }
    if(!strcmp(sock_type, "uds")&& !strcmp(protocol, "dgram")){
            return UDS_DGRAM;
        
    }
    if(!strcmp(sock_type, "uds")&& !strcmp(protocol, "stream")){
       return UDS_STREAM;
    }
}

void partb_server(char *address[]){
    //address[0] = ./stnc, address[1] = "-s", address[2] = port
    socklen_t client_addrlen = 0;
    int yes = 1;
    int time;
    struct sockaddr_in servaddr, clientaddr;
    memset(&servaddr, '\0', sizeof(struct sockaddr_in));
    memset(&clientaddr, '\0', sizeof(struct sockaddr_in));

    FILE* fp = fopen("file.txt", "w+");
    if(fp == NULL){
        perror("server side: fopen");
        exit(1);
    }

    char* chunk100MB = (char*)malloc(sizeof(char)*hundredMB);
    if(chunk100MB == NULL){
        perror("server side: malloc");
        exit(1);
    }

    for(int i = 0; i < hundredMB; i++){
        chunk100MB[i] = (char)(rand() % 256);
    }

    for(int i = 0; i < hundredMB; i++){
        fwrite(chunk100MB + i, 1, 1, fp);
    }
    fclose(fp);
    free(chunk100MB);

    // socket create and verification
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        perror("server side: socket");
        exit(1);
    }
    if(quiet_mode == false) printf("waiting for connections...\n");

    // assign IP, PORT
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servaddr.sin_port = atoi(address[2]);

    // Binding newly created socket to given IP and verification
    if ((bind(sockfd, (struct sockaddr*)&servaddr, sizeof(servaddr))) < 0){
        perror("server side: bind");
        exit(1);
    }

    if(setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1){
        perror("server side: setsockopt");
        exit(1);
    }

    // Now server is ready to listen and verification
    if ((listen(sockfd, 1)) < 0) {
        perror("server side: listen");
        exit(1);
    }

    // Accept the data packet from client and verification
    int new_fd = accept(sockfd, (struct sockaddr*)&clientaddr, &client_addrlen);
    if (new_fd < 0) {
        perror("server side: accept");
        exit(1);
    }

    if(quiet_mode == false) printf("Server: new connection from on port %s\n", address[2]);

    char recv_buf[sizeof(first_message)];
    int brecv = (int) recv(new_fd, recv_buf, sizeof(first_message), 0);
    if (brecv == -1) {
        perror("server side: recv");
        exit(1);
    } else if (brecv == 0) {
        if(quiet_mode == false) printf("client left the conversation\n");
        exit(1);
    }
    else {
        long time;
        first_message *fm = (first_message *) recv_buf;
        if (fm->protocol == TCP_IPV4) {
            time = s_tcp_ipv4_channel(); 
            if(quiet_mode == true) printf("ipv4_tcp,%ld\n",time);
        }
        else if (fm->protocol == TCP_IPV6) {
           time = s_tcp_ipv6_channel();
             if(quiet_mode == true)  printf("ipv6_tcp,%ld\n",time);
        } else if (fm->protocol == UDP_IPV4) {
            time = s_udp_ipv4_channel();
            if(quiet_mode == true)  printf("ipv4_udp,%ld\n",time);
        }
        else if (fm->protocol == UDS_STREAM) {
            time =  s_uds_stream_channel();
            if(quiet_mode == true) printf("uds_stream,%ld\n",time);
        }
        else if (fm->protocol == UDP_IPV6) {
            time = s_udp_ipv6_channel();
            if(quiet_mode == true) printf("ipv6_udp,%ld\n",time);
        }
        else if (fm->protocol == UDS_DGRAM) {
            time =  s_uds_dgram_channel();
            if(quiet_mode == true) printf("uds_dgram,%ld\n",time);
        } 
        else if (fm->protocol == MMAP) {
            time = s_mmap_channel();
            if(quiet_mode==true) printf("mmap,%ld\n",time);
        } 
        else {
            time = s_pipeline_channel();
            if(quiet_mode==true) printf("pipe,%ld\n",time);
        }
    }
}

void partb_client(char *address[]){
    struct sockaddr_in servaddr;
    int yes = 1;
    memset(&servaddr, '\0', sizeof(servaddr));
    //address[0] = ./stnc, address[1] = "-c", address[2] = ip, address[3] = port
    //address[4] = "-p", address[5] = socket type, address[6] = protocol

    // socket create and verification
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        perror("client side: socket");
        exit(1);
    }
    // assign IP, PORT
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = inet_addr(address[2]);
    servaddr.sin_port = atoi(address[3]);

    // connect the client socket to server socket
    if(connect(sockfd, (struct sockaddr*)&servaddr, sizeof(servaddr)) < 0){
        perror("client side: connect");
        exit(1);
    }

    if(setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1){
        perror("server side: setsockopt");
        exit(1);
    }

    first_message *fm = malloc(sizeof(first_message));
    if(fm == NULL){
        perror("client side: malloc first message");
        exit(1);
    }

    else fm->protocol = socktype_to_num(address[5], address[6]);
    int bsent = (int)send(sockfd, fm, sizeof(first_message), 0);
    if (bsent < 0) {
        perror("client side: send");
        exit(1);
    }
    
    if(fm->protocol == TCP_IPV4){
        sleep(1);
        c_tcp_ipv4_channel(address);
    }
    else if(fm->protocol == TCP_IPV6){
      
        c_tcp_ipv6_channel(address);
    }
    else if(fm->protocol == UDP_IPV4){
       
        c_udp_ipv4_channel(address);
    }
    else if(fm->protocol == UDS_STREAM){
        c_uds_stream_channel(address);
    }
    else if(fm->protocol == UDS_DGRAM){
        c_uds_dgram_channel(address);
    }
    else if(fm->protocol == UDP_IPV6){
        c_udp_ipv6_channel(address);
    }
    else if(fm->protocol == MMAP){
        c_mmap_channel(address);
    }
    else{
        c_pipeline_channel(address);
    }
}

long s_tcp_ipv4_channel(){
    int yes = 1;
    socklen_t client_addrlen = 0;
    struct sockaddr_in servaddr, clientaddr;
    memset(&servaddr, '\0', sizeof(servaddr)); //TODO: added this now check if it works
    memset(&clientaddr, '\0', sizeof(clientaddr));

    // socket create and verification
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        perror("server side: socket");
        exit(1);
    }

    if(quiet_mode == false) printf("Socket successfully created..\n");

    if(setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1){
        perror("server side: setsockopt");
        exit(1);
    }

    // assign IP, PORT
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servaddr.sin_port = htons(PARTB_PORT);

    // Binding newly created socket to given IP and verification
    if ((bind(sockfd, (struct sockaddr*)&servaddr, sizeof(servaddr))) < 0){
        perror("server side: bind");
        exit(1);
    }

    // Now server is ready to listen and verification
    if ((listen(sockfd, 1)) < 0) {
        perror("server side: listen");
        exit(1);
    }

    // Accept the data packet from client and verification
    int new_fd = accept(sockfd, (struct sockaddr*)&clientaddr, &client_addrlen);
    if (new_fd < 0) {
        perror("server side: accept");
        exit(1);
    }

    char *buffer = (char*)malloc(hundredMB * sizeof(char));
    if (buffer == NULL){
        perror("server side: malloc");
        exit(1);
    }

     ///// mesuare time....
    struct timeval start_time, end_time;
    gettimeofday(&start_time, NULL);
    /////////////////////////////////////////////

    long rb_counter = 0;
    if(quiet_mode == false) printf("receiving data\n");
    while(rb_counter + chunk < hundredMB) {
        int brecv = (int) recv(new_fd, buffer + rb_counter, chunk, 0);
        if (brecv == -1) {
            perror("server side: recv");
            exit(1);
        } else if (brecv == 0) {
            if(quiet_mode == false) printf("client left the conversation\n");
            exit(1);
        }
        rb_counter += brecv;
    }
    gettimeofday(&end_time, NULL);

    long start_time_micros = start_time.tv_sec * 1000000 + start_time.tv_usec;
    long end_time_micros = end_time.tv_sec * 1000000 + end_time.tv_usec;
    long time = end_time_micros - start_time_micros;

    close(new_fd);
    close(sockfd);
    free(buffer);

    return time;
}

long s_udp_ipv4_channel(){
    int yes = 1;
    struct sockaddr_in servaddr, cliaddr;
    socklen_t client_addrlen;
    char *buffer = (char*)malloc(hundredMB * sizeof(char));
    if (buffer == NULL){
        perror("server side: malloc");
        exit(1);
    }
    memset(&servaddr, '\0', sizeof(servaddr));
    memset(&cliaddr, '\0', sizeof(cliaddr));
    memset(buffer, '\0', hundredMB);

     // create socket 
    int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd == -1) {
        perror("server side: socket");
        exit(1);
    }

    if(setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes)) == -1){
        perror("server side: setsockopt");
        exit(1);
    }

    // fill info
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(PARTB_PORT);
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);


    // bind ;
    if(bind(sockfd, (struct sockaddr *)&servaddr, sizeof(struct sockaddr)) == -1) {
        perror("server side: bind");
        exit(1);
    }

    struct pollfd fds[1];
    fds[0].fd = sockfd;  
    fds[0].events = POLLIN;

    struct timeval start_time, end_time;
    gettimeofday(&start_time, NULL);   

    int rb_counter = 0;
    while (rb_counter + chunk <= hundredMB) {
        int poll_count = poll(fds, 1, -1);
        if (poll_count == -1) {
            perror("server side: poll");
            exit(1);
        }
        if (fds[0].revents & POLLIN) {
            int brecv = recvfrom(sockfd, buffer + rb_counter, chunk, 0, (struct sockaddr *)&cliaddr, &client_addrlen);
            if (brecv == -1) {
                perror("server side: recvfrom");
                exit(1);
            }
        }
        rb_counter += chunk;
    }

    gettimeofday(&end_time, NULL);


    long start_time_micros = start_time.tv_sec * 1000000 + start_time.tv_usec;
    long end_time_micros = end_time.tv_sec * 1000000 + end_time.tv_usec;
    long time = end_time_micros - start_time_micros;
    

    close(sockfd);
    return time;
}

long s_uds_stream_channel(){
    struct sockaddr_un servaddr, cliaddr;
    socklen_t client_addrlen;
    memset(&servaddr, '\0', sizeof(servaddr));
    memset(&cliaddr, '\0', sizeof(cliaddr));
  
    int sockfd = socket(AF_UNIX, SOCK_STREAM, 0);
    if(sockfd == -1){
        perror("server side: socket");
        exit(1);
    }

    sleep(1); 

    servaddr.sun_family = AF_UNIX;
    strcpy(servaddr.sun_path, SOCK_PATH);
    unlink(SOCK_PATH);

    if(bind(sockfd, (struct sockaddr *)&servaddr, sizeof(servaddr)) == -1){
        close(sockfd);
        perror("server side: bind");
        exit(1);
    }

    if(listen(sockfd, 1) == -1){
        close(sockfd);
        perror("server side: listen");
        exit(1);
    }
    if(quiet_mode == false) printf("socket listening...\n");

    int newfd = accept(sockfd, (struct sockaddr *)&cliaddr, &client_addrlen);
    if(newfd == -1){
        perror("server side: accept");
        close(sockfd);
        close(newfd);
        exit(1);
    }
    if(quiet_mode==false) printf("socket accepted");

    //TODO: why use cliaddrlen twice? strange
    if(getpeername(newfd, (struct sockaddr *)&cliaddr, &client_addrlen) == -1){
        perror("server side: getpeername");
        close(sockfd);
        close(newfd);
        exit(1);
    }

    if(quiet_mode == false) printf("Client socket filepath: %s\n", cliaddr.sun_path);

    char* buffer = (char*)malloc(hundredMB * sizeof(char));
    if (buffer == NULL){
        perror("server side: malloc");
        close(sockfd);
        close(newfd);
        exit(1);
    }
    memset(buffer, 0, hundredMB);
    int rb_counter = 0;

    ///// mesuare time....
    struct timeval start_time, end_time;
    gettimeofday(&start_time, NULL);
    /////////////////////////////////////////////

    while(rb_counter + chunk <= hundredMB) {
        int brecv = (int)recv(newfd, buffer + rb_counter, chunk, 0);
        if (brecv == -1) {
            perror("server side: recv");
            close(sockfd);
            close(newfd);
            exit(1);
        } else if (brecv == 0) {
            if(quiet_mode == false) printf("client left the conversation\n");
            close(sockfd);
            close(newfd);
            break; 
        }
        rb_counter += brecv;
    }
    gettimeofday(&end_time, NULL);

    long start_time_micros = start_time.tv_sec * 1000000 + start_time.tv_usec;
    long end_time_micros = end_time.tv_sec * 1000000 + end_time.tv_usec;
    long time = end_time_micros - start_time_micros; 

    free(buffer);
    close(sockfd);
    close(newfd);

    return time;
}

long s_udp_ipv6_channel(){
    int yes = 1; 
    struct sockaddr_in6 servaddr, cliaddr;
    socklen_t client_addrlen;
    char* data = (char*)malloc(hundredMB * sizeof(char));
    if (data == NULL){
        perror("server side: malloc");
        exit(1);
    }
    memset(data, '\0', hundredMB);
    memset(&servaddr, '\0', sizeof(servaddr));
    memset(&cliaddr, '\0', sizeof(cliaddr));


    /// create socket
    int sockfd = socket(AF_INET6, SOCK_DGRAM, 0);
    if (sockfd == -1) {
        perror("server side: socket");
        exit(1);
    }

    if(setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes)) == -1){
        perror("server side: setsockopt");
        exit(1);
    }

    // fill info.
    servaddr.sin6_family = AF_INET6;
    servaddr.sin6_port = htons(PARTB_PORT);
    servaddr.sin6_addr = in6addr_any;


    if(bind(sockfd, (struct sockaddr *)&servaddr, sizeof(struct sockaddr_in6)) == -1) {
        perror("server side: bind");
        exit(1);
    }

    struct pollfd fds[1]; // there 1 envents max
    fds[0].fd = sockfd;  
    fds[0].events = POLLIN;
    struct timeval start_time, end_time;
    gettimeofday(&start_time, NULL);    

    int rb_counter = 0;
    while (rb_counter + chunk <= hundredMB) {
        int poll_count = poll(fds, 1, -1);
        if (poll_count == -1) {
            perror("server side: poll");
            exit(1);
        }
        if (fds[0].revents & POLLIN) {
            int brecv = recvfrom(sockfd, data + rb_counter, chunk, 0, (struct sockaddr *)&cliaddr, &client_addrlen);
            if (brecv == -1) {
                perror("server side: recvfrom");
                exit(1);
            }
        }
        rb_counter += chunk;
    }

    gettimeofday(&end_time, NULL);

    long start_time_micros = start_time.tv_sec * 1000000 + start_time.tv_usec;
    long end_time_micros = end_time.tv_sec * 1000000 + end_time.tv_usec;
    long time = end_time_micros - start_time_micros;

    close(sockfd);
    return time;
}

long s_uds_dgram_channel(){
    struct sockaddr_un servaddr;
    socklen_t client_addrlen;
    int yes = 1;
    memset(&servaddr, '\0', sizeof(servaddr));

    int sockfd = socket(AF_UNIX, SOCK_DGRAM, 0);
    if (sockfd == -1){
        perror("server side: socket");
        exit(1);
    }

    if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes))) {
        perror("server side: setsockopt");
        exit(1);
    }//TODO: check if needed

    servaddr.sun_family = AF_UNIX;
    strncpy(servaddr.sun_path, SERVER_DGRAM_PATH,sizeof(servaddr.sun_path) - 1);
    unlink(SERVER_DGRAM_PATH);

    if (bind(sockfd, (struct sockaddr *)&servaddr, sizeof(servaddr)) == -1){
        perror("server side: bind");
        close(sockfd);
        exit(1);
    }

    char* buffer = (char*)malloc(hundredMB * sizeof(char));
    if (buffer == NULL){
        close(sockfd);
        exit(1);
    }

    memset(buffer, 0, hundredMB);
    int rb_counter = 0;

    ///// mesuare time....
    struct timeval start_time, end_time;
    gettimeofday(&start_time, NULL);
    /////////////////////////////////////////////

    while(rb_counter + chunk <= hundredMB){
        int brecv = (int)recvfrom(sockfd, buffer + rb_counter, chunk,
                                  0, (struct sockaddr *)&servaddr, &client_addrlen);
        if (brecv == -1){
            perror("server side: recv");
            close(sockfd);
            exit(1);
        } else if (brecv == 0) {
            if(quiet_mode == false) printf("client left the conversation\n");
            close(sockfd);
            exit(1);
        }
        rb_counter += brecv;
    }

    gettimeofday(&end_time, NULL);

    long start_time_micros = start_time.tv_sec * 1000000 + start_time.tv_usec;
    long end_time_micros = end_time.tv_sec * 1000000 + end_time.tv_usec;
    long time = end_time_micros - start_time_micros;

    free(buffer);
    close(sockfd);

    return time;
}

long s_pipeline_channel(){
   
    int file;
    char buffer[oneK];
    ssize_t num_bytes_read;

    mkfifo(FILE_INPUT_OUTPUT_NAME, 0666);
    file = open(FILE_INPUT_OUTPUT_NAME, O_RDONLY|O_CREAT);
    if (file == -1) {
        perror("server side: open");
        exit(EXIT_FAILURE);
    }
    FILE* file_path = fopen("file_rcv_pipe.txt", "w+");
    if (file_path == NULL) {
        perror("server side: fopen");
        exit(EXIT_FAILURE);
    }

    struct timeval start_time, end_time;
    gettimeofday(&start_time, NULL);

    while ((num_bytes_read = read(file, buffer, oneK)) > 0) {
        fwrite(buffer, 1, num_bytes_read, file_path);
    }
    
    gettimeofday(&end_time, NULL);
   
   
    long start_time_micros = start_time.tv_sec * 1000000 + start_time.tv_usec;
    long end_time_micros = end_time.tv_sec * 1000000 + end_time.tv_usec;
    long time = end_time_micros - start_time_micros;

    fclose(file_path);
    close(file);
    unlink(FILE_INPUT_OUTPUT_NAME);
    return time;
}

long s_mmap_channel(){
    struct sockaddr_in servaddr, cliaddr;
    memset((char *)&servaddr, 0, sizeof(servaddr));
    memset((char *)&cliaddr, 0, sizeof(cliaddr));
    char buffer[oneK];
    struct pollfd fds[1];
    int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0) {
        perror("server side: socket");
      
    }
    
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = INADDR_ANY;
    servaddr.sin_port = htons(PARTB_PORT);
    if (bind(sockfd, (struct sockaddr *)&servaddr, sizeof(servaddr)) < 0) {
        perror("server side: bind_mmap");
        close(sockfd);
    }

    fds[0].fd = sockfd;
    fds[0].events = POLLIN;

    ////// mesuare time.....
    struct timeval start_time, end_time;
    gettimeofday(&start_time, NULL);
    //////////////////////////////////////////////////

    while (1) {
        int poll_count = poll(fds, 1, TIMEOUT);
        if (poll_count == -1) {
            perror("server side: poll_mmap"); 
            close(sockfd);
            
        } else if (poll_count == 0) {
            break;
        } else {
            if (fds[0].revents & POLLIN) {
                memset(buffer,oneK ,sizeof(buffer));
                socklen_t clientlen = sizeof(cliaddr);

                int rcv_bytes = recvfrom(sockfd, buffer, oneK - 1, 0, (struct sockaddr *)&cliaddr, &clientlen);
                if (rcv_bytes < 0) {
                    perror("server side: recvfrom_mmap");
                   
                }
            }
        }
    }

    gettimeofday(&end_time, NULL);

    long start_time_micros = start_time.tv_sec * 1000000 + start_time.tv_usec;
    long end_time_micros = end_time.tv_sec * 1000000 + end_time.tv_usec;
    long time = end_time_micros - start_time_micros;

    close(sockfd);
    return time;
}

long s_tcp_ipv6_channel(){
    int yes = 1;
    socklen_t client_addrlen;
    struct sockaddr_in6 servaddr, cliaddr;
    memset(&servaddr, 0, sizeof(servaddr));
    memset(&cliaddr, 0, sizeof(cliaddr));

    int sockfd = socket(AF_INET6, SOCK_STREAM, 0);
    // socket create and verification
    if (sockfd < 0) {
        perror("server side: socket failed");
        exit(1);
    }
    else{
       if(quiet_mode==false) printf("Socket successfully created..\n");
    }

    if(setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes))){
        perror("server side: setsockopt");
        exit(1);
    }

    // assign IP, PORT
    servaddr.sin6_family = AF_INET6;
    servaddr.sin6_addr = in6addr_any;
    servaddr.sin6_port = htons(PARTB_PORT);

    // Binding newly created socket to given IP and verification
    if ((bind(sockfd, (struct sockaddr*)&servaddr, sizeof(servaddr))) < 0){
        perror("server side: bind");
        exit(1);
    }

    // Now server is ready to listen and verification
    if ((listen(sockfd, 3)) < 0) {
        perror("server side: listen");
        exit(1);
    }

    // Accept the data packet from client and verification
    int new_fd = accept(sockfd, (struct sockaddr*)&servaddr, &client_addrlen);
    if (new_fd < 0) {
        perror("server side: accept");
        exit(1);
    }

    char *buffer = (char * )malloc(hundredMB * sizeof(char));

    if (buffer == NULL)
    {
        perror("server side: malloc");
        exit(1);
    }

    ///// mesuare time....
    struct timeval start_time, end_time;
    gettimeofday(&start_time, NULL);
    /////////////////////////////////////////////

    long rb_counter = 0;
    while(rb_counter + chunk < hundredMB) {
            int brecv = (int) recv(new_fd, buffer + rb_counter, chunk, 0);
            if (brecv == -1) {
                perror("server side: recv");
                exit(1);
            } else if (brecv == 0) {
                if(quiet_mode == false) printf("client left the conversation\n");
                exit(1); 
            }
            rb_counter += brecv;
    }

    gettimeofday(&end_time, NULL);

    long start_time_micros = start_time.tv_sec * 1000000 + start_time.tv_usec;
    long end_time_micros = end_time.tv_sec * 1000000 + end_time.tv_usec;
    long time = end_time_micros - start_time_micros;

    if(quiet_mode==false) printf("\nTime get to recv (tcp_ipv6):- %ld\n\n",time);

    free(buffer);
    return time;
}

void c_tcp_ipv4_channel(char* address[]){
    struct sockaddr_in servaddr;
    int yes = 1;
    memset(&servaddr, '\0', sizeof(servaddr));

    // socket create and verification
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        perror("client side: socket");
        exit(1);
    }
    // assign IP, PORT
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = inet_addr(address[2]);
    servaddr.sin_port = htons(PARTB_PORT);

    if(setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1){
        perror("server side: setsockopt");
        exit(1);
    }

    // connect the client socket to server socket
    if(connect(sockfd, (struct sockaddr*)&servaddr, sizeof(servaddr)) < 0){
        perror("client side: connect");
        exit(1);
    }

    int sb_counter = 0;

    char* chunk100MB = malloc(sizeof(char)*hundredMB);
    if(chunk100MB == NULL){
        perror("generate 100MB: malloc");
        exit(1);
    }
    memset(chunk100MB, '\0', sizeof(char)*hundredMB);
    for (int i = 0; i < hundredMB; i++) {
        chunk100MB[i] = (char)(rand() % 256);
    }

    while(sb_counter + chunk < hundredMB){
        if (send(sockfd, chunk100MB + sb_counter, chunk, 0) < 0){
            perror("client side: send");
            exit(1);
        }
        sb_counter += chunk;
    }

    close(sockfd);
    free(chunk100MB);
}

void c_tcp_ipv6_channel(char* address[]){
    struct sockaddr_in6 servaddr;
    memset(&servaddr,'\0',sizeof(servaddr));
    int yes = 1;

    // socket create and verification
    int sockfd = socket(AF_INET6, SOCK_STREAM, 0);
    if (sockfd < 0) {
        perror("client side: socket");
        exit(1);
    }

    // assign IP, PORT
    servaddr.sin6_family = AF_INET6;
    servaddr.sin6_port = htons(PARTB_PORT);

    if (inet_pton(AF_INET6, "::1", &servaddr.sin6_addr) <= 0) {
        perror("client side: inet_pton");
        exit(1);
    }
    
    sleep(5);
    // connect the client socket to server socket
    if(connect(sockfd, (struct sockaddr*)&servaddr, sizeof(servaddr)) < 0){
        perror("client side: connect");
        exit(1);
    }

    int sb_counter = 0;

    char* chunk100MB = malloc(sizeof(char)*hundredMB);
    if(chunk100MB == NULL){
        perror("generate 100MB: malloc");
        exit(1);
    }
    memset(chunk100MB, '\0', sizeof(char)*hundredMB);
    for (int i = 0; i < hundredMB; i++) {
        chunk100MB[i] = (char)(rand() % 256);
    }

    while(sb_counter + chunk <= hundredMB){
        if (send(sockfd, chunk100MB + sb_counter, chunk, 0) < 0){
            perror("client side: send");
            exit(1); 
        }
        sb_counter += chunk;

    }
    
    free(chunk100MB);
    close(sockfd);
}

void c_udp_ipv4_channel(char* address[]){
    int yes = 1; 
    struct sockaddr_in servaddr;
    memset(&servaddr,'\0',sizeof(servaddr));
    char data[oneK];
    FILE *filepath;
    int bsent; 

    // create socket
    int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd == -1) {
        perror("client side: socket");
        exit(1);
    }

    if(setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes)) == -1){
        perror("server side: setsockopt");
        exit(1);
    }

    // file info
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(PARTB_PORT);
    servaddr.sin_addr.s_addr = inet_addr("127.0.0.1");
    filepath = fopen("file.txt", "r+");
    if (filepath == NULL) {
        perror("client side: fopen");
        exit(1);
    }

    struct pollfd filedis[2];
    filedis[0].fd = sockfd;  
    filedis[0].events = POLLOUT;    
    filedis[1].fd = fileno(filepath);     
    filedis[1].events = POLLIN;     

    while (1) {
        int ret = poll(filedis, 2, -1);
        if (ret == -1) {
            perror("client side: poll");
            exit(1);
        }
        if (filedis[0].revents & POLLOUT){
            if (fgets(data, sizeof(data), filepath) != NULL) {
                bsent = sendto(sockfd, data, strlen(data), 0, (struct sockaddr *)&servaddr, sizeof(struct sockaddr));
                if (bsent == -1) {
                    perror("client side: sendto");
                    exit(1);
                }
            }
            else{
                break;
            }
        }
    }
    close(sockfd);
}

void c_uds_stream_channel(char* address[]){
    struct sockaddr_un servaddr, cliaddr;
    memset(&servaddr, 0, sizeof(servaddr));
    memset(&cliaddr, 0, sizeof(cliaddr));

    int sockfd = socket(AF_UNIX, SOCK_STREAM, 0);
    if (sockfd == -1) {
        perror("client side: socket");
        exit(1);
    }

    cliaddr.sun_family = AF_UNIX;
    strcpy(cliaddr.sun_path, CLIENT_PATH);

    unlink(CLIENT_PATH);
    if (bind(sockfd, (struct sockaddr *) &cliaddr, sizeof(cliaddr)) == -1){
        perror("client side: bind");
        close(sockfd);
        exit(1);
    }

    servaddr.sun_family = AF_UNIX;
    strcpy(servaddr.sun_path, SERVER_PATH);
    if(connect(sockfd, (struct sockaddr *)&servaddr, sizeof(servaddr)) == -1){
        perror("client side: connect");
        close(sockfd);
        exit(1);
    }

    int sb_counter = 0;
    char* chunk100MB = malloc(sizeof(char)*hundredMB);
    if(chunk100MB == NULL){
        perror("generate 100MB: malloc");
        exit(1);
    }

    memset(chunk100MB, '\0', sizeof(char)*hundredMB);
    for (int i = 0; i < hundredMB; i++) {
        chunk100MB[i] = (char)(rand() % 256);
    }
    
    while(sb_counter + chunk < hundredMB){
        if(send(sockfd, chunk100MB + sb_counter, chunk, 0) == -1){
            perror("client side: send");
            close(sockfd);
            exit(1);
        }
        sb_counter += chunk;
    }
    close(sockfd);
    free(chunk100MB);
}

void c_udp_ipv6_channel(char* address[]){
    int yes = 1;
    struct sockaddr_in6 servaddr;
    memset(&servaddr,'\0',sizeof(servaddr));
    FILE *filepath;
    char data[oneK];

    // create socket
    int sockfd = socket(AF_INET6, SOCK_DGRAM, 0);
    if (sockfd == -1) {
        perror("client side: socket");
        exit(1);
    }

    if(setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes)) == -1){
        perror("server side: setsockopt");
        exit(1);
    }

    // file info
    servaddr.sin6_family = AF_INET6;
    servaddr.sin6_port = htons(PARTB_PORT);
    inet_pton(AF_INET6, "::1", &(servaddr.sin6_addr));

    filepath = fopen("file.txt", "r+");
    if (filepath == NULL) {
        perror("client side: fopen");
        exit(1);
    }


    struct pollfd fds[2]; // max 2 events
    fds[0].fd = sockfd;  
    fds[0].events = POLLOUT;   

    while (1) {
        int poll_count = poll(fds, 2, -1);
        if (poll_count == -1) {
            perror("client side: poll");
            exit(1);
        }

        if(fds[0].revents & POLLOUT){
            memset(data, 0, sizeof(data));
            if (fgets(data, sizeof(data), filepath) != NULL) {
                int bsent = sendto(sockfd, data, strlen(data), 0, (struct sockaddr *)&servaddr, sizeof(struct sockaddr_in6));
                if (bsent == -1) {
                    perror("client side: sendto");
                    exit(1);
                }
            } else {
                break;
            }
        }
    }
    fclose(filepath);
    close(sockfd);

}

void c_uds_dgram_channel(char* address[]){
    struct sockaddr_un servaddr;
    int yes = 1;
    memset(&servaddr, '\0', sizeof(servaddr));

    int sockfd = socket(AF_UNIX, SOCK_DGRAM, 0);
    if (sockfd == -1) {
        perror("client side: socket");
        exit(1);
    }

    if(setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1){
        perror("server side: setsockopt");
        exit(1);
    }

    servaddr.sun_family = AF_UNIX;
    strncpy(servaddr.sun_path, SERVER_DGRAM_PATH,sizeof(servaddr.sun_path) - 1);
    unlink(SERVER_DGRAM_PATH);

    int sb_counter = 0;

    char* chunk100MB = malloc(sizeof(char)*hundredMB);
    if(chunk100MB == NULL){
        perror("generate 100MB: malloc");
        exit(1);
    }

    memset(chunk100MB, '\0', sizeof(char)*hundredMB);
    for (int i = 0; i < hundredMB; i++) {
        chunk100MB[i] = (char)(rand() % 256);
    }
    
   while(sb_counter + chunk < hundredMB){
        if(sendto(sockfd, chunk100MB + sb_counter, chunk, 0,
                  (struct sockaddr*)&servaddr, sizeof(servaddr)) == -1){
            perror("client side: send");
            close(sockfd);
            exit(1);
        }
        sb_counter += chunk;
   }
    if(sendto(sockfd, chunk100MB + sb_counter, hundredMB - sb_counter,
              0,(struct sockaddr *)&servaddr, sizeof(servaddr)) == -1){
        perror("client side: send");
        close(sockfd);
        exit(1);
    }

    close(sockfd);
    free(chunk100MB);
}

void c_pipeline_channel(char* address[]){
    int fd;
    char buffer[oneK];
    ssize_t bytes_read;
    FILE* file_path;
    file_path = fopen("file.txt", "r+");
    if (file_path == NULL) {
        perror("client side: fopen");
        exit(EXIT_FAILURE);
    }
    fd = open(FILE_INPUT_OUTPUT_NAME, O_WRONLY|O_CREAT);
    if (fd == -1) {
        perror("client side: open");
        exit(EXIT_FAILURE);
    }
    while ((bytes_read = fread(buffer, 1, oneK, file_path)) > 0) {
        write(fd, buffer, bytes_read);
    }
    fclose(file_path);
    close(fd);
}

void c_mmap_channel(char* address[]){
    struct sockaddr_in servaddr;
    struct stat stat;
    int fd = open("file.txt", O_RDONLY);
    if (fd < 0) {
        perror("client side: open");
      
    }
    if (fstat(fd, &stat) < 0) {
        perror("client side: fstat");
        close(fd);
        
    }

    off_t fileSIZE = stat.st_size;
    char *fileDATA = mmap(NULL, fileSIZE, PROT_READ, MAP_PRIVATE, fd, 0);
    if (fileDATA == MAP_FAILED) {
        perror("client side: mmap");
        close(fd);
       
    }

    int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0) {
        perror("client side: socket_mmap");
        munmap(fileDATA, fileSIZE);
        close(fd);
    }

    ///////////////////////////////////////// fill that data of server....
    memset((char *)&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(PARTB_PORT);
    servaddr.sin_addr.s_addr = INADDR_ANY;
    /////////////////////////////////////

    off_t offset = 0;
    ssize_t bytes_notSentYet = fileSIZE;
    ssize_t bytes_SuccessSent;

    while (bytes_notSentYet > 0) {
        size_t chunk_size_DATA = (bytes_notSentYet > oneK) ? oneK : bytes_notSentYet;
        bytes_SuccessSent = sendto(sockfd, fileDATA + offset, chunk_size_DATA, 0, (struct sockaddr *)&servaddr, sizeof(servaddr));
        if (bytes_SuccessSent < 0) {
            perror("client side: sendto_mmap");
            munmap(fileDATA, fileSIZE);
            close(fd);
          
        }
        offset += bytes_SuccessSent;
        bytes_notSentYet -= bytes_SuccessSent;
    }

    munmap(fileDATA, fileSIZE); // Unmapping the file.
    close(fd);
    close(sockfd);
    sleep(3); 
}
