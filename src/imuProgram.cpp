#include "imuClass.hpp"
#include "interface.h"
#include <fcntl.h>
#include <string>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>

volatile sig_atomic_t keep_running = 1;

// 1. Define the signal handler function
void handle_sigint(int sig) {
    // Note: Keep handlers brief. printf isn't strictly async-signal-safe,
    // but used here for demonstration. 
    keep_running = 0; 
}

int main(int argc, char* argv[]) {
    imuClass imu1;
    // imuInputPkt imuInput;
    universalPkt imuInput;
    // imuToNavPkt imuOutput;
    universalPkt imuOutput;

    std::map<std::string, std::string> configMap_string;

    std::string configFile;

    if(argc > 1) {
        configFile = argv[1];
    } else {
        configFile = "input/imuProgramConfig.txt";
    }

    std::string imuInputFName,imuOutputFName;

    struct sigaction sa;

    std::ifstream file(configFile);

    if (!file.is_open()) {
        std::cerr << "Error: Could not open the imu program config file." << std::endl;
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

    imuInputFName = configMap_string["imuInputFileName"];
    imuOutputFName = configMap_string["imuOutputFileName"];

    std::cout << "IMU input file name: " << imuInputFName << std::endl;
    std::cout <<"IMU output file name: " << imuOutputFName << std::endl;
    
    sa.sa_handler = &handle_sigint; // Point to your handler function
    sigemptyset(&sa.sa_mask);       // Do not block other signals during execution
    sa.sa_flags = 0;                // No special flags needed

    // 3. Register the handler for SIGINT (Ctrl+C)
    if (sigaction(SIGINT, &sa, NULL) == -1) {
        perror("Error registering signal handler in imu program");
        return 1;
    }

    //open guidToSim file in both simulator and guidance

    int imuInput_fd,imuOutput_fd;

    imuInput_fd = open(imuInputFName.data(), O_RDONLY);

    std::cout << "Waiting for imu output file to open" << std::endl;
    imuOutput_fd = open(imuOutputFName.data(),O_WRONLY);
    std::cout << "Opened imu output file" << std::endl;

    int read_bytes = 0,write_bytes = 0;

    //read once for init
    read_bytes = read(imuInput_fd,&imuInput,sizeof(imuInputPkt));
    while(read_bytes == -1) {
        read_bytes = read(imuInput_fd,&imuInput,sizeof(imuInputPkt));
    }

    imu1.setBodyAxes(imuInput.cmd_q);

    
    while(keep_running) {
        //read from simulator
        read_bytes = read(imuInput_fd,&imuInput,sizeof(universalPkt));
        while(read_bytes == -1) {
            read_bytes = read(imuInput_fd,&imuInput,sizeof(universalPkt));
        }
        imu1.getIMUOutput(imuInput,&imuOutput);
        write_bytes = write(imuOutput_fd,&imuOutput,sizeof(universalPkt));
    }

    close(imuInput_fd);
    close(imuOutput_fd);
}