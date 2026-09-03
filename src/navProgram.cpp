#include "navigation.h"
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
    Navigation n1;
    navInitPkt navInit_Packet;
    // imuToNavPkt imuToNav_Packet;
    universalPkt imuToNav_Packet;
    // navToGuidPkt navToGuid_Packet;
    universalPkt navToGuid_Packet;

    std::map<std::string, std::string> configMap_string;

    std::string configFile;

    if(argc > 1) {
        configFile = argv[1];
    } else {
        configFile = "input/navProgramConfig.txt";
    }

    std::cout << "Using config file: " << configFile << std::endl;

    std::string navToGuidFName = "/home/srinidhi/Documents/Simulations/Orbital_Mechanics/CppSim/operateFiles/guidSpace/navToGuidFIFO",imuToNavFName = "/home/srinidhi/Documents/Simulations/Orbital_Mechanics/CppSim/operateFiles/guidSpace/simToNavFIFO";

    std::string navInitFile = "input/Guid_Input_Params.txt";

    int imuToNav_fd,navToGuid_fd,navInit_fd;

    struct sigaction sa;

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

    imuToNavFName = configMap_string["imuToNavFName"];
    navToGuidFName = configMap_string["navToGuidFName"];
    navInitFile = configMap_string["navInitFIFO"];

    std::cout << "simToNavFName: " << imuToNavFName << std::endl;
    std::cout << "navToGuidFName: " << navToGuidFName << std::endl;
    std::cout << "navInittFile: " << navInitFile << std::endl;
    
    sa.sa_handler = &handle_sigint; // Point to your handler function
    sigemptyset(&sa.sa_mask);       // Do not block other signals during execution
    sa.sa_flags = 0;                // No special flags needed

    // 3. Register the handler for SIGINT (Ctrl+C)
    if (sigaction(SIGINT, &sa, NULL) == -1) {
        perror("Error registering signal handler");
        return 1;
    }

    // signal(SIGPIPE, SIG_IGN); 

    auto now = std::chrono::system_clock::now();

    // 2. Convert to a legacy time_t structure
    std::time_t currentTime = std::chrono::system_clock::to_time_t(now);

    // 3. Convert to local time structure
    std::tm* localTime = std::localtime(&currentTime);

    std::ostringstream logfilename;
    logfilename <<"logs/"<< std::put_time(localTime, "%Y_%m_%d_%H_%M_%S")
             << "_guidLog.txt";

    std::cout<< "Log file name = " << logfilename.str() << std::endl;

    std::ofstream logfile(logfilename.str());
    
    int read_bytes = 0,write_bytes = 0;

    //open navInit FIFO to get init data
    navInit_fd = open(navInitFile.data(),O_RDONLY);

    //read from navInit FIFO
    while(read_bytes < sizeof(navInitPkt)) {
        read_bytes += read(navInit_fd,&navInit_Packet,sizeof(navInitPkt));
    }

    close(navInit_fd);

    double Re = navInit_Packet.Re;
    double phi_init = navInit_Packet.phi_init * PI / 180;
    double theta_init = navInit_Packet.theta_init * PI / 180;

    double sxo = Re*sin(phi_init)*cos(theta_init);
    double syo = Re*sin(phi_init)*sin(theta_init);
    double szo = Re*cos(phi_init);

    navInit_Packet.s.setXYZ(sxo, syo, szo);
    navInit_Packet.v.setXYZ(0,0,0);

    navInit_Packet.Xv.x = -cos(phi_init) * cos(theta_init);
    navInit_Packet.Xv.y = -cos(phi_init) * sin(theta_init);
    navInit_Packet.Xv.z = sin(phi_init);

    navInit_Packet.Yv.x = sin(theta_init);
    navInit_Packet.Yv.y = -cos(theta_init);

    navInit_Packet.Zv.x = sin(phi_init) * cos(theta_init);
    navInit_Packet.Zv.y = sin(phi_init) * sin(theta_init);
    navInit_Packet.Zv.z = cos(phi_init);

    navInit_Packet.body_x = navInit_Packet.Zv;
    navInit_Packet.body_y = -navInit_Packet.Yv;
    navInit_Packet.body_z = navInit_Packet.Xv;

    n1.initNav(navInit_Packet);

    //open imu to nav fifo
    imuToNav_fd = open(imuToNavFName.data(),O_RDONLY);

    navToGuid_fd = open(navToGuidFName.data(),O_WRONLY);
    
    while(keep_running) {
        //read from simulator
        read_bytes = read(imuToNav_fd,&imuToNav_Packet,sizeof(universalPkt));
        while(read_bytes == -1) {
            read_bytes = read(imuToNav_fd,&imuToNav_Packet,sizeof(universalPkt));
            // std::cout<< "Read bytes = " << read_bytes << std::endl;
        }
        std::cout<< "Read bytes = " << read_bytes << std::endl;
        n1.getNavOutput(&imuToNav_Packet,&navToGuid_Packet);
        write_bytes = write(navToGuid_fd,&navToGuid_Packet,sizeof(universalPkt));
    }

    close(imuToNav_fd);
    close(navToGuid_fd);

    // close(simToGuid_fd);
    // close(guidToSim_fd);


    

}