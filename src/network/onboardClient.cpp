#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/time.h>
#include <arpa/inet.h>
#include "network/network_headers.h"
#include <fcntl.h>
#include <string>
#include <iostream>
#include <signal.h>
#include <fstream>
#include <pthread.h>
#include <iostream>
#include <map>
#include <sstream>

#define PORT 4500
#define SERVERADDR "127.0.0.1"
#define MSG "Echo Test"

#define GUID_TO_SIM_PORT 4500
#define SIM_TO_GUID_PORT 4501

typedef struct thread_args {
    int sock_fd;
    int fifo_fd;
    std::string server_addr;
}; 

static que q1;

int thread_running = 1;

void handle_sigint(int sig) {
    // Note: Keep handlers brief. printf isn't strictly async-signal-safe,
    // but used here for demonstration. 
    thread_running = 0; 
}

//function to transfer data from fifo to socket
//argument 1: pointer to the socket file descriptor
//argument 2: pointer to the fifo file descriptor
void* read_from_fifo_write_to_socket(void* arg) {
    struct thread_args* args = (struct thread_args*)arg;
    int sock_fd = args->sock_fd;
    int fifo_fd = args->fifo_fd;
    char read_buffer[1024];
    int index = 0;
    int file_read_bytes, socket_write_bytes;

    int guidToSim_sock,bind_status,guid_to_sim_conn_sock,conn_status_1;

    struct sockaddr_in guidToSim_server,s_client1;
    unsigned int si_len = sizeof(s_client1);

    std::string server_addr = args->server_addr;

    TCP_SOCKET_THREAD_COMPAT(guidToSim_sock);

    guidToSim_server.sin_family = AF_INET;
    guidToSim_server.sin_port = htons(GUID_TO_SIM_PORT);

    if(!inet_aton(server_addr.data(),&guidToSim_server.sin_addr)) {
        printf("IP network format conversion failed\n");
        return NULL;
    } else {
        printf("IP network format conversion successful\n");
    }

    conn_status_1 = connect(guidToSim_sock,(struct sockaddr*)&guidToSim_server,sizeof(guidToSim_server));
    TRY_CATCH_THREAD_COMPAT(conn_status_1,"Connection to server failed in guid to sim \n");
    sock_fd = guidToSim_sock;

    while(thread_running) {
        //read from fifo
        file_read_bytes = read(fifo_fd,read_buffer,sizeof(read_buffer));
        // std::cout<<"Enter data to send to socket: ";
        // std::cin >> read_buffer; // Wait for user input
        // file_read_bytes = strlen(read_buffer);
        // std::cout << "Read bytes from fifo = " << file_read_bytes << std::endl;

        if(file_read_bytes <= 0) {
            continue;
        }

        index = 0;

        //write to socket
        socket_write_bytes = 0;
        while(index < file_read_bytes) {
            if(index > 1023) {
                std::cout << "Index exceeded buffer size" << std::endl;
                break;
            }
            int ssize = send(sock_fd,&read_buffer[index],file_read_bytes - index,0);
            std::cout << "Socket write bytes = " << ssize << std::endl;
            // std::cin >> std::ws; // Wait for user input
            if(ssize == -1) {
                perror("Error writing to socket");
                break;
            }
            index += ssize;
            socket_write_bytes += ssize;
        }
        // std::cout << "Socket write bytes = " << socket_write_bytes << std::endl;
    }
}

//function to transfer data from socket to fifo
//argument 1: pointer to the socket file descriptor
//argument 2: pointer to the fifo file descriptor
void* read_from_socket_write_to_fifo(void* arg) {
    struct thread_args* args = (struct thread_args*)arg;
    int sock_fd = args->sock_fd;
    int fifo_fd = args->fifo_fd;
    char read_buffer[1024];
    int index = 0;
    int socket_read_bytes, file_write_bytes;

    int simToGuid_sock,bind_status,guid_to_sim_conn_sock,conn_status_2;

    struct sockaddr_in simToGuid_server,s_client1;
    unsigned int si_len = sizeof(s_client1);
    std::string server_addr = args->server_addr;

    TCP_SOCKET_THREAD_COMPAT(simToGuid_sock);

    simToGuid_server.sin_family = AF_INET;
    simToGuid_server.sin_port = htons(SIM_TO_GUID_PORT);

    if(!inet_aton(server_addr.c_str(),&simToGuid_server.sin_addr)) {
        printf("IP network format conversion failed\n");
        return NULL;
    } else {
        printf("IP network format conversion successful\n");
    }

    conn_status_2 = connect(simToGuid_sock,(struct sockaddr*)&simToGuid_server,sizeof(simToGuid_server));
    TRY_CATCH_THREAD_COMPAT(conn_status_2,"Connection to server failed in sim to guid \n");

    sock_fd = simToGuid_sock;

    std::cout << "Waiting for data from socket..." << std::endl;

    while(thread_running) {
        // std::cout << "Waiting for data from socket in while loop..." << std::endl;
        //read from socket
        index = 0;
        socket_read_bytes = recv(sock_fd,&read_buffer[index],sizeof(read_buffer),0);
        // while(index < 10) {
        //     socket_read_bytes = recv(sock_fd,&read_buffer[index],sizeof(read_buffer) - index,0);
        //     // std::cout << "Socket read bytes = " << socket_read_bytes << std::endl;
        //     if(socket_read_bytes <= 0) {
        //         continue;
        //     }
        //     index += socket_read_bytes;
        // }
        // socket_read_bytes = index;
        // socket_read_bytes = recv(sock_fd,read_buffer,sizeof(read_buffer),0);
        // std::cout << "Socket read bytes = " << socket_read_bytes << std::endl;
        if(socket_read_bytes <= 0) {
            continue;
        }
        // std::cout << "Data read from socket: ";
        for(int i = 0; i < socket_read_bytes; i++) {
            std::cout << read_buffer[i];
        }
        std::cout << std::endl;
        // continue;
        // std::cin >> std::ws; // Wait for user input

        if(socket_read_bytes <= 0) {
            continue;
        }

        index = 0;

        //write to fifo
        while(index < socket_read_bytes) {
            if(index > 1023) {
                std::cout << "Index exceeded buffer size" << std::endl;
                break;
            }
            int ssize = write(fifo_fd,&read_buffer[index],socket_read_bytes - index);
            std::cout << "Write bytes to fifo = " << ssize << std::endl;
            if(ssize == -1) {
                perror("Error writing to fifo");
                break;
            }
            index += ssize;
        }
    }
}

int main(int argc, char* argv[]) {
    int simToGuid_sock,guidToSim_sock,ssize,rsize,conn_status_1,conn_status_2,n,index;
    message smsg,rmsg;

    struct sockaddr_in guidToSim_server,simToGuid_server,s_client;
    int si_len = sizeof(guidToSim_server);

    struct timeval tv;
    unsigned long before,after;

    int guidToSim_fd,simToGuid_fd;

    char read_buffer[1024],write_buffer[1024];

    int file_read_bytes, socket_read_bytes,file_write_bytes, socket_write_bytes;

    std::string server_addr;

    std::map<std::string, std::string> configMap_string;

    std::string guidToSimFName = "/home/srinidhi/Documents/Simulations/Orbital_Mechanics/CppSim/operateFiles/guidSpace/guidToSimFIFO",simToGuidFName = "/home/srinidhi/Documents/Simulations/Orbital_Mechanics/CppSim/operateFiles/guidSpace/simToGuidFIFO";

    std::string configFile;
    if(argc > 1) {
        configFile = argv[1];
    } else {
        configFile = "input/onboardClientConfig.txt";
    }

    std::ifstream file(configFile);

    if (!file.is_open()) {
        std::cerr << "Error: Could not open the file." << std::endl;
        return 1;
    }

    std::string line;

    // 2. Read the file line by line
    while (std::getline(file, line)) {
        // Skip empty lines or comments
        if (line.empty() || line[0] == '#') {
            continue;
        }

        std::stringstream ss(line);
        std::string ftype;
        std::string key;
        std::string str_value;
        double value;

        // 3. Extract key and value separated by whitespace
        // if (ss >> key >> value) {
        //     configMap[key] = value;
        // }
        ss >> ftype;
        if (ftype == "STRING"){
            ss >> key >> str_value;
            configMap_string[key] = str_value;
        } 
    }

    // 4. Close the file stream
    file.close();

    server_addr = configMap_string["server_addr"];
    simToGuidFName = configMap_string["simToGuidFName"];
    guidToSimFName = configMap_string["guidToSimFName"];

    std::cout << "Server address: " << server_addr << std::endl;

    

    guidToSim_fd = open(guidToSimFName.data(),O_RDONLY);

    simToGuid_fd = open(simToGuidFName.data(),O_WRONLY | O_NONBLOCK);
    if(simToGuid_fd == -1) {
        std::cout << "Waiting for guidance to open simToGuid" << std::endl;
    }

    //try until reader opens
    while(simToGuid_fd == -1) {
        simToGuid_fd = open(simToGuidFName.data(),O_WRONLY | O_NONBLOCK);
    }
    struct sigaction sa;
    sa.sa_handler = &handle_sigint; // Point to your handler function
    sigemptyset(&sa.sa_mask);       // Do not block other signals during execution
    sa.sa_flags = 0;                // No special flags needed

    // 3. Register the handler for SIGINT (Ctrl+C)
    if (sigaction(SIGINT, &sa, NULL) == -1) {
        perror("Error registering signal handler");
        return 1;
    }

    printf("Client started\n");
    //Creating the socket

    // TCP_SOCKET(guidToSim_sock);

    // guidToSim_server.sin_family = AF_INET;
    // guidToSim_server.sin_port = htons(GUID_TO_SIM_PORT);

    // if(!inet_aton(SERVERADDR,&guidToSim_server.sin_addr)) {
    //     printf("IP network format conversion failed\n");
    //     return 1;
    // } else {
    //     printf("IP network format conversion successful\n");
    // }

    // conn_status_1 = connect(guidToSim_sock,(struct sockaddr*)&guidToSim_server,sizeof(guidToSim_server));
    // TRY_CATCH(conn_status_1,"Connection to server failed in guid to sim \n");


    // TCP_SOCKET(simToGuid_sock);

    // simToGuid_server.sin_family = AF_INET;
    // simToGuid_server.sin_port = htons(SIM_TO_GUID_PORT);

    // if(!inet_aton(SERVERADDR,&simToGuid_server.sin_addr)) {
    //     printf("IP network format conversion failed\n");
    //     return 1;
    // } else {
    //     printf("IP network format conversion successful\n");
    // }

    // conn_status_2 = connect(simToGuid_sock,(struct sockaddr*)&simToGuid_server,sizeof(simToGuid_server));
    // TRY_CATCH(conn_status_2,"Connection to server failed in sim to guid \n");


    pthread_t read_from_socket_thread,write_to_socket_thread;
    thread_running = 1;

    // struct thread_args args1 = {guidToSim_sock, guidToSim_fd,server_addr};
    // struct thread_args args2 = {simToGuid_sock, simToGuid_fd,server_addr};

    struct thread_args args1 = {guidToSim_sock, guidToSim_fd,server_addr};
    struct thread_args args2 = {simToGuid_sock, simToGuid_fd,server_addr};

    

    pthread_create(&read_from_socket_thread,NULL,read_from_socket_write_to_fifo,(void*)&args2);
    pthread_create(&write_to_socket_thread,NULL,read_from_fifo_write_to_socket,(void*)&args1);

    std::cout << "Threads created" << std::endl;
    while(thread_running) {
    }
    // std::cout << "Press any key to terminate the server..." << std::endl;
    // std::cin >> std::ws; // Wait for user input
    std::cout << "Terminating the server..." << std::endl;
    thread_running = 0;
    pthread_join(read_from_socket_thread, NULL);
    pthread_join(write_to_socket_thread, NULL);
    close(simToGuid_sock);
    close(guidToSim_sock);
    close(simToGuid_fd);
    close(guidToSim_fd);

    // while(1) {
    //     //read from socket
    //     socket_read_bytes = recv(clisock,read_buffer,sizeof(read_buffer),0);
    //     std::cout << "Socket read bytes = " << socket_read_bytes << std::endl;
    //     //write to sim to guid fifo
    //     while(index < socket_read_bytes) {
    //         ssize = write(simToGuid_fd,&read_buffer[index],socket_read_bytes - index);
    //         if(index > 1023) {
    //             std::cout << "Index exceeded buffer size" << std::endl;
    //             break;
    //         }
    //         std::cout << "Write bytes to sim to guid fifo = " << ssize << std::endl;
    //         if(ssize == -1) {
    //             perror("Error writing to sim to guid fifo");
    //             break;
    //         }
    //         index += ssize;
    //     }

    //     //read from guid to sim fifo until fifo is empty
    //     file_read_bytes = read(guidToSim_fd,write_buffer,sizeof(write_buffer));
    //     while(file_read_bytes == -1) {
    //         file_read_bytes = read(guidToSim_fd,write_buffer,sizeof(write_buffer));
    //     }
    //     std::cout << "Read bytes from guid to sim fifo = " << file_read_bytes << std::endl;

        

    //     //write to socket
    //     index = 0;
    //     while(index < file_read_bytes) {
    //         ssize = send(clisock,&write_buffer[index],file_read_bytes - index,0);
    //         if(index > 1023) {
    //             std::cout << "Index exceeded buffer size" << std::endl;
    //             break;
    //         }
    //         std::cout << "Socket write bytes = " << ssize << std::endl;
    //         if(ssize == -1) {
    //             perror("Error writing to socket");
    //             break; 
    //         }
    //         index += ssize;
    //     }
    // }

    

    return 0;
}