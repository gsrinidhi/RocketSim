#include "network/network_headers.h"
#include<fcntl.h>
#include <unistd.h>
#include <string>
#include <iostream>
#include <pthread.h>
#include <signal.h>

#define GUID_TO_SIM_PORT 4500
#define SIM_TO_GUID_PORT 4501

typedef struct thread_args {
    int sock_fd;
    int fifo_fd;
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

    int sim_to_guid_sock,bind_status,sim_to_guid_conn_sock;

    struct sockaddr_in simToGuid_server,s_client2;
    unsigned int si_len = sizeof(s_client2);

    TCP_SOCKET_THREAD_COMPAT(sim_to_guid_sock);
    simToGuid_server.sin_family = AF_INET;
    simToGuid_server.sin_port = htons(SIM_TO_GUID_PORT);
    simToGuid_server.sin_addr.s_addr = htonl(INADDR_ANY);
    BIND_THREAD_COMPAT(bind_status,sim_to_guid_sock,simToGuid_server);
    listen(sim_to_guid_sock,5);
    sim_to_guid_conn_sock = accept(sim_to_guid_sock,(struct sockaddr*)&s_client2,&si_len);
    sock_fd = sim_to_guid_conn_sock;

    while(thread_running) {
        //read from fifo
        file_read_bytes = read(fifo_fd,read_buffer,sizeof(read_buffer));
        // std::cout<<"Enter data to send to socket: ";
        // std::cin >> read_buffer; // Wait for user input
        // file_read_bytes = strlen(read_buffer);
        std::cout << "Read bytes from fifo = " << file_read_bytes << std::endl;

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
            ssize_t ssize = send(sock_fd,&read_buffer[index],file_read_bytes - index,0);
            std::cout << "Socket write bytes = " << ssize << std::endl;
            // std::cin >> std::ws; // Wait for user input
            if(ssize == -1) {
                perror("Error writing to socket");
                break;
            }
            index += ssize;
            socket_write_bytes += ssize;
        }
        std::cout << "Socket write bytes = " << socket_write_bytes << std::endl;
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

    int guid_to_sim_sock,bind_status,guid_to_sim_conn_sock;

    struct sockaddr_in guidToSim_server,s_client1;
    unsigned int si_len = sizeof(s_client1);

    TCP_SOCKET_THREAD_COMPAT(guid_to_sim_sock);

    guidToSim_server.sin_family = AF_INET;
    guidToSim_server.sin_port = htons(GUID_TO_SIM_PORT);
    guidToSim_server.sin_addr.s_addr = htonl(INADDR_ANY);

    BIND_THREAD_COMPAT(bind_status,guid_to_sim_sock,guidToSim_server);
    listen(guid_to_sim_sock,5);
    guid_to_sim_conn_sock = accept(guid_to_sim_sock,(struct sockaddr*)&s_client1,&si_len);
    sock_fd = guid_to_sim_conn_sock;

    std::cout << "Waiting for data from socket..." << std::endl;

    while(thread_running) {
        //read from socket
        index = 0;
        socket_read_bytes = recv(sock_fd,&read_buffer[index],sizeof(read_buffer),0);
        // while(index < sizeof(gui)) {
        //     socket_read_bytes = recv(sock_fd,&read_buffer[index],sizeof(read_buffer) - index,0);
        //     // std::cout << "Socket read bytes = " << socket_read_bytes << std::endl;
        //     if(socket_read_bytes <= 0) {
        //         continue;
        //     }
        //     index += socket_read_bytes;
        // }
        // socket_read_bytes = index;
        // socket_read_bytes = recv(sock_fd,read_buffer,sizeof(read_buffer),0);
        std::cout << "Socket read bytes = " << socket_read_bytes << std::endl;
        if(socket_read_bytes <= 0) {
            continue;
        }
        std::cout << "Data read from socket: ";
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

    std::string guidToSimFName = "/home/srinidhi/Documents/Simulations/Orbital_Mechanics/CppSim/operateFiles/simSpace/guidToSimFIFO",simToGuidFName = "/home/srinidhi/Documents/Simulations/Orbital_Mechanics/CppSim/operateFiles/simSpace/simToGuidFIFO";

    int simToGuid_fd,guidToSim_fd;

    int guid_to_sim_sock, sim_to_guid_sock,guid_to_sim_conn_sock,sim_to_guid_conn_sock,ssize,rsize,bind_status,index;
    //message smsg, rmsg;
    struct sockaddr_in simToGuid_server,guidToSim_server,s_client1,s_client2;
    unsigned int si_len = sizeof(s_client1);

    struct sigaction sa;
    sa.sa_handler = &handle_sigint; // Point to your handler function
    sigemptyset(&sa.sa_mask);       // Do not block other signals during execution
    sa.sa_flags = 0;                // No special flags needed

    // 3. Register the handler for SIGINT (Ctrl+C)
    if (sigaction(SIGINT, &sa, NULL) == -1) {
        perror("Error registering signal handler");
        return 1;
    }

    char read_buffer[1024],write_buffer[1024];
    size_t file_read_bytes, socket_read_bytes,file_write_bytes, socket_write_bytes;

    pthread_t read_from_socket_thread,write_to_socket_thread;
    //open guidToSim file in both simulator and guidance
    guidToSim_fd = open(guidToSimFName.data(),O_WRONLY | O_NONBLOCK);
    if(guidToSim_fd == -1) {
        std::cout << "Waiting for guid to sim file to open" << std::endl;
    }

    while(guidToSim_fd == -1) {
        guidToSim_fd = open(guidToSimFName.data(),O_WRONLY | O_NONBLOCK);
    }

    simToGuid_fd = open(simToGuidFName.data(),O_RDONLY);

    init_q(&q1);
    

    printf("Server started\n");

    //Creating the socket
    // TCP_SOCKET(guid_to_sim_sock);

    // guidToSim_server.sin_family = AF_INET;
    // guidToSim_server.sin_port = htons(GUID_TO_SIM_PORT);
    // guidToSim_server.sin_addr.s_addr = htonl(INADDR_ANY);

    // BIND(bind_status,guid_to_sim_sock,guidToSim_server);
    // listen(guid_to_sim_sock,5);
    // guid_to_sim_conn_sock = accept(guid_to_sim_sock,(struct sockaddr*)&s_client1,&si_len);

    // TCP_SOCKET(sim_to_guid_sock);
    // simToGuid_server.sin_family = AF_INET;
    // simToGuid_server.sin_port = htons(SIM_TO_GUID_PORT);
    // simToGuid_server.sin_addr.s_addr = htonl(INADDR_ANY);
    // BIND(bind_status,sim_to_guid_sock,simToGuid_server);
    // listen(sim_to_guid_sock,5);
    // sim_to_guid_conn_sock = accept(sim_to_guid_sock,(struct sockaddr*)&s_client2,&si_len);
    thread_running = 1;
    //create thread to read from socket and write to fifo
    struct thread_args args1 = {guid_to_sim_conn_sock, guidToSim_fd};
    pthread_create(&read_from_socket_thread,NULL,read_from_socket_write_to_fifo,(void*)&args1);
    //create thread to read from fifo and write to socket
    struct thread_args args2 = {sim_to_guid_conn_sock, simToGuid_fd};
    pthread_create(&write_to_socket_thread,NULL,read_from_fifo_write_to_socket,(void*)&args2);
    std::cout << "Threads created" << std::endl;
    // std::cout << "Press any key to terminate the server..." << std::endl;
    // std::cin >> std::ws; // Wait for user input
    while(thread_running) {
    }
    std::cout << "Terminating the server..." << std::endl;
    thread_running = 0;
    pthread_join(read_from_socket_thread, NULL);
    pthread_join(write_to_socket_thread, NULL);
    close(sim_to_guid_sock);
    close(guid_to_sim_sock);
    close(simToGuid_fd);
    close(guidToSim_fd);
    // while(1) {

    //     //read from sim to guid fifo
    //     file_read_bytes = read(simToGuid_fd,read_buffer,sizeof(read_buffer));
    //     std::cout << "Read bytes from sim to guid fifo = " << file_read_bytes << std::endl;

    //     index = 0;
    //     //write to socket
    //     while(index < file_read_bytes) {
    //         if(index > 1023) {
    //             std::cout << "Index exceeded buffer size" << std::endl;
    //             break;
    //         }
    //         ssize = send(guid_to_sim_conn_sock,&read_buffer[index],file_read_bytes - index,0);
    //         std::cout << "Socket write bytes = " << ssize << std::endl;
    //         if(ssize == -1) {
    //             perror("Error writing to socket");
    //             break;
    //         }
    //         index += ssize;
    //     }
    //     // socket_write_bytes = write(guid_to_sim_conn_sock,read_buffer,file_read_bytes);

    //     //read from socket
    //     socket_read_bytes = recv(guid_to_sim_conn_sock,write_buffer,sizeof(write_buffer),0);
    //     std::cout << "Socket read bytes = " << socket_read_bytes << std::endl;
    //     //write to guid to sim fifo
    //     index = 0;
    //     while(index < socket_read_bytes) {
    //         if(index > 1023) {
    //             std::cout << "Index exceeded buffer size" << std::endl;
    //             break;
    //         }
    //         ssize = write(guidToSim_fd,&write_buffer[index],socket_read_bytes - index);
    //         std::cout << "Write bytes to guid to sim fifo = " << ssize << std::endl;
    //         if(ssize == -1) {
    //             perror("Error writing to guid to sim fifo");
    //             break;
    //         }
    //         index += ssize;
    //     }
    //     // file_write_bytes = write(guidToSim_fd,write_buffer,socket_read_bytes);

    // }
    return 0;
}