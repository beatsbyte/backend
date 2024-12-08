#include <iostream>
#include <string>
#include <sstream>
#include <array>
#include <memory>
#include <stdexcept>

// Helper function to execute a command and capture its output
std::string executeCommand(const std::string& command) {
    std::array<char, 128> buffer;
    std::string result;
    std::unique_ptr<FILE, decltype(&pclose)> pipe(popen(command.c_str(), "r"), pclose);
    if (!pipe) {
        throw std::runtime_error("Failed to run command.");
    }
    while (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr) {
        result += buffer.data();
    }
    return result;
}

// Function to get the bitrate of an MP3 file
int getMp3Bitrate(const std::string& filePath) {
    try {
        // Try using lame to get the bitrate
        std::string lameCommand = "lame --decode --silent " + filePath + " - 2>&1";
        std::string lameOutput = executeCommand(lameCommand);
        
        // Parse lame output for bitrate
        std::istringstream lameStream(lameOutput);
        std::string line;
        while (std::getline(lameStream, line)) {
            if (line.find("kbps") != std::string::npos) {
                std::istringstream lineStream(line);
                int bitrate;
                lineStream >> bitrate;
                return bitrate;
            }
        }

        // Fallback to mpg123
        std::string mpg123Command = "mpg123 --test " + filePath + " 2>&1";
        std::string mpg123Output = executeCommand(mpg123Command);

        // Parse mpg123 output for bitrate
        std::istringstream mpg123Stream(mpg123Output);
        while (std::getline(mpg123Stream, line)) {
            if (line.find("bitrate:") != std::string::npos) {
                size_t pos = line.find("bitrate:");
                int bitrate = std::stoi(line.substr(pos + 8));
                return bitrate;
            }
        }
    } catch (const std::exception& ex) {
        std::cerr << "Error: " << ex.what() << std::endl;
    }

    return -1; // Return -1 if unable to determine bitrate
}
