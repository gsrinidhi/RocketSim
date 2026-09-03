#include "cguidance.h"
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
    CGuidance g1;
    guidInitPkt guidInitPacket;
    // navToGuidPkt navToGuid_Packet;
    universalPkt navToGuid_Packet;
    // simToGuidPkt simToGuid_Packet;
    universalPkt simToGuid_Packet;
    // guidToSimPkt guidToSim_Packet;
    universalPkt guidToSim_Packet;

    std::map<std::string, std::string> configMap_string;

    std::string configFile;

    std::string navToGuidFName;

    int read_bytes = 0,write_bytes = 0;

    if(argc > 1) {
        configFile = argv[1];
    } else {
        configFile = "input/guidProgramConfig.txt";
    }

    std::cout << "Using config file: " << configFile << std::endl;

    std::string guidToSimFName = "/home/srinidhi/Documents/Simulations/Orbital_Mechanics/CppSim/operateFiles/guidSpace/guidToSimFIFO",simToGuidFName = "/home/srinidhi/Documents/Simulations/Orbital_Mechanics/CppSim/operateFiles/guidSpace/simToGuidFIFO";

    std::string guidInitFile = "input/Guid_Input_Params.txt";

    int simToGuid_fd,guidToSim_fd,guidInit_fd,navToGuid_fd;

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

    navToGuidFName = configMap_string["navToGuidFName"];
    guidToSimFName = configMap_string["guidToSimFName"];
    guidInitFile = configMap_string["guidInitFile"];

    std::cout << "simToGuidFName: " << simToGuidFName << std::endl;
    std::cout << "guidToSimFName: " << guidToSimFName << std::endl;
    std::cout << "guidInitFile: " << guidInitFile << std::endl;
    
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

    std::ostringstream logfilename,dataLogFileName;
    logfilename <<"logs/"<< std::put_time(localTime, "%Y_%m_%d_%H_%M_%S")
             << "_guidLog.txt";

    dataLogFileName <<"logs/"<< std::put_time(localTime, "%Y_%m_%d_%H_%M_%S")
             << "_guidDataLog.txt";

    std::cout<< "Log file name = " << logfilename.str() << std::endl;
    std::cout<< "Data Log file name = " << dataLogFileName.str() << std::endl;

    std::ofstream logfile(logfilename.str()),datalogFile(dataLogFileName.str());

    guidInit_fd = open(guidInitFile.data(),O_RDONLY);

    //read from guidInit FIFO
    while(read_bytes < sizeof(guidInitPkt)) {
        read_bytes += read(guidInit_fd,&guidInitPacket,sizeof(guidInitPkt));
    }

    close(guidInit_fd);

    //open guidToSim file in both simulator and guidance
    guidToSim_fd = open(guidToSimFName.data(),O_WRONLY | O_NONBLOCK);
    if(guidToSim_fd == -1) {
        std::cout << "Waiting for guid to sim file to open" << std::endl;
    }

    while(guidToSim_fd == -1) {
        guidToSim_fd = open(guidToSimFName.data(),O_WRONLY | O_NONBLOCK);
    }

    // simToGuid_fd = open(simToGuidFName.data(),O_RDONLY);
    navToGuid_fd = open(navToGuidFName.data(),O_RDONLY);

    //read init packet

    // while(read(simToGuid_fd,&simToGuid_Init_Packet,sizeof(simToGuidInitPkt)) == -1) {

    // }
    // std::cout<< "Init parameters received" << std::endl;
    // std::cout << "Mass of earth: " << simToGuid_Init_Packet.Me << std::endl;
    // std::cout << "Radius of earth: " << simToGuid_Init_Packet.Re << std::endl;
    // std::cout << "Position vector: " << simToGuid_Init_Packet.s.x << " " << simToGuid_Init_Packet.s.y << " " << simToGuid_Init_Packet.s.z << std::endl;
    // std::cout << "Phi init: " << simToGuid_Init_Packet.phi_init << std::endl;
    // std::cout << "Theta init: " << simToGuid_Init_Packet.theta_init << std::endl;
    // std::cout << "Inclination: " << simToGuid_Init_Packet.inclination << std::endl;


    // g1.setEarthSpecs(simToGuid_Init_Packet.Me,simToGuid_Init_Packet.Re);
    g1.guidInit(guidInitPacket);
    g1.setAltitudeThresholds(50e3,50e3);
    // g1.guidInitFile(guidInitFile);
    g1.setLogFile(&logfile);
    g1.setDataLogFile(&datalogFile);
    // g1.guidInit(simToGuid_Init_Packet.s,simToGuid_Init_Packet.phi_init,simToGuid_Init_Packet.theta_init,simToGuid_Init_Packet.inclination);

    

    
    while(keep_running) {
        //read from simulator
        read_bytes = read(navToGuid_fd,&navToGuid_Packet,sizeof(universalPkt));
        while(read_bytes == -1) {
            read_bytes = read(navToGuid_fd,&navToGuid_Packet,sizeof(universalPkt));
            // std::cout<< "Read bytes = " << read_bytes << std::endl;
        }
        std::cout<< "Read bytes = " << read_bytes << std::endl;
        g1.getHeading(simToGuid_Packet.s,simToGuid_Packet.Xv,&guidToSim_Packet.heading_angle);
        simToGuid_Packet.heading_angle = guidToSim_Packet.heading_angle;
        simToGuid_Packet.Xv = navToGuid_Packet.Xv;
        simToGuid_Packet.Yv = navToGuid_Packet.Yv;
        simToGuid_Packet.Zv = navToGuid_Packet.Zv;
        simToGuid_Packet.s = navToGuid_Packet.s;
        simToGuid_Packet.v = navToGuid_Packet.v;
        simToGuid_Packet.local_FPA = navToGuid_Packet.local_FPA;
        simToGuid_Packet.time = navToGuid_Packet.time;
        g1.getGuidanceOutputQuat(simToGuid_Packet.s,simToGuid_Packet.v,simToGuid_Packet.local_FPA,simToGuid_Packet.time,simToGuid_Packet.heading_angle,simToGuid_Packet.Xv,simToGuid_Packet.Yv,simToGuid_Packet.Zv,(guidToSim_Packet.cmd_q),&guidToSim_Packet.isThrusting);
        g1.getApoapsisPeriapsis(&guidToSim_Packet.apogee,&guidToSim_Packet.perigee);
        write_bytes = write(guidToSim_fd,&guidToSim_Packet,sizeof(universalPkt));
        // if(write_bytes == -1) {
        //     std::cout << "Write fifo closed" << std::endl;
        //     keep_running = 0;
        // }
    }

    close(navToGuid_fd);
    close(guidToSim_fd);


    

}