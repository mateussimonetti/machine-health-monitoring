#include <iostream>
#include <cstdlib>
#include <chrono>
#include <ctime>
#include <thread>
#include <unistd.h>
#include <fstream>
#include <iomanip>

#define QOS 1
#define BROKER_ADDRESS "tcp://localhost:1883"

#include <fstream>
#include <sstream>
#include <string>

// Function to get the used disk percentage
double getUsedDiskPercentage() {
    double percentage = 0.0;

    // Open the "/proc/mounts" file to read disk information
    std::ifstream mounts("/proc/mounts");
    if (mounts.is_open()) {
        std::string line;
        while (std::getline(mounts, line)) {
            // Split the line by spaces
            std::istringstream iss(line);
            std::string device, mountPoint, fsType;
            iss >> device >> mountPoint >> fsType;

            // Skip non-filesystem lines
            if (fsType != "ext4" && fsType != "ext3" && fsType != "ext2") {
                continue;
            }

            // Get disk space information using the mount point
            std::ifstream statvfs(mountPoint);
            if (statvfs.is_open()) {
                // Get the total disk space and used disk space
                unsigned long long totalSpace, usedSpace;
                statvfs >> totalSpace >> usedSpace;

                // Calculate the used disk space percentage
                if (totalSpace > 0) {
                    percentage = static_cast<double>(usedSpace) / totalSpace * 100.0;
                }

                statvfs.close();
                break;  // Exit the loop after finding the first valid filesystem
            }
        }
        mounts.close();
    }

    return percentage;
}

// Function to get the used memory percentage
double getUsedMemoryPercentage() {
    double percentage = 0.0;

    // Open the "/proc/meminfo" file to read memory information
    std::ifstream meminfo("/proc/meminfo");
    if (meminfo.is_open()) {
        std::string line;
        while (std::getline(meminfo, line)) {
            if (line.substr(0, 9) == "MemTotal:") {
                // Get the total memory size
                double totalMemory;
                std::istringstream iss(line.substr(9));
                iss >> totalMemory;

                // Read the next line to get the used memory
                std::getline(meminfo, line);
                if (line.substr(0, 7) == "MemUsed:") {
                    // Get the used memory
                    double usedMemory;
                    std::istringstream iss(line.substr(7));
                    iss >> usedMemory;

                    // Calculate the used memory percentage
                    percentage = usedMemory / totalMemory * 100.0;
                }
                break;
            }
        }
        meminfo.close();
    }

    return percentage;
}