#include <iostream>
#include <cstdlib>
#include <chrono>
#include <ctime>
#include <thread>
#include <unistd.h>
#include <fstream>
#include "json.hpp" // json handling
#include "data_getters.hpp" 
#include "mqtt/client.h" // paho mqtt
#include <iomanip>

#define QOS 1
#define BROKER_ADDRESS "tcp://localhost:1883"

#include <fstream>
#include <sstream>
#include <string>

std::string now() {
    // Get the current time in ISO 8601 format
    auto now = std::chrono::system_clock::now();
    std::time_t now_c = std::chrono::system_clock::to_time_t(now);
    std::tm* now_tm = std::localtime(&now_c);
    std::stringstream ss;
    ss << std::put_time(now_tm, "%FT%TZ");
    std::string timestamp = ss.str();
    return timestamp;
}

// Function for the first thread to publish memory usage
void publishDiskUsage(const std::string& machineId, nlohmann::json config, mqtt::client& client) {
    std::clog << "publishDiskUsage" << std::endl;
    nlohmann::json j;
    while (true) {
        // Construct the JSON message

        j["timestamp"] = now();
        j["value"] = getUsedDiskPercentage();
        std::string sensorId = config["sensor_id"]; 

        // Publish the JSON message to the appropriate topic
        std::string topic = "/sensors/" + machineId + "/" + sensorId;
        mqtt::message msg(topic, j.dump(), QOS, false);
        std::clog << "Memory message published - topic: " << topic << " - message: " << j.dump() << std::endl;
        client.publish(msg);

        // Sleep for some time
        std::this_thread::sleep_for(std::chrono::milliseconds(config["exec_time_ms"]));
    }
}

// Function for the second thread to publish CPU usage
void publishCPUUsage(const std::string& machineId, nlohmann::json config, mqtt::client& client) {
    std::clog << "publishCPUUsage" << std::endl;
    nlohmann::json j;
    while (true) {
        j["timestamp"] = now();
        j["value"] = getUsedDiskPercentage();
        std::string sensorId = config["sensor_id"]; 

        // Publish the JSON message to the appropriate topic
        std::string topic = "/sensors/" + machineId + "/" + sensorId;
        mqtt::message msg(topic, j.dump(), QOS, false);
        std::clog << "CPU message published - topic: " << topic << " - message: " << j.dump() << std::endl;
        client.publish(msg);

        // Sleep for some time
        std::this_thread::sleep_for(std::chrono::milliseconds(config["exec_time_ms"]));
    }
}

int main() {
    std::string clientId = "sensor-monitor";
    mqtt::client client(BROKER_ADDRESS, clientId);

    // Connect to the MQTT broker.
    mqtt::connect_options connOpts;
    connOpts.set_keep_alive_interval(20);
    connOpts.set_clean_session(true);

    try {
        client.connect(connOpts);
    } catch (mqtt::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return EXIT_FAILURE;
    }
    std::clog << "connected to the broker" << std::endl;

    // Get the unique machine identifier, in this case, the hostname.
    char hostname[1024];
    gethostname(hostname, 1024);
    std::string machineId(hostname);

    //Get the config file
    std::ifstream f("getter_config.json");
    nlohmann::json data = nlohmann::json::parse(f);

    // Start the memory usage publishing thread
    std::thread memoryThread(publishDiskUsage, machineId, data["disk_getter"], std::ref(client));

    // Start the CPU usage publishing thread
    std::thread cpuThread(publishCPUUsage, machineId, data["CPU_getter"], std::ref(client));

    // Wait for the threads to finish (which will be never in this case, since the loops run indefinitely)
    memoryThread.join();
    cpuThread.join();

    return 0;
}
