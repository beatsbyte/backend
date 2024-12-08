#include <iostream>
#include <fstream>
#include <bitset>
#include <string>

void mp3ToBinary(const std::string& inputFilePath, const std::string& outputFilePath) {
    std::ifstream inputFile(inputFilePath, std::ios::binary);
    std::ofstream outputFile(outputFilePath);

    if (!inputFile) {
        std::cerr << "Error: Could not open input file." << std::endl;
        return;
    }

    if (!outputFile) {
        std::cerr << "Error: Could not open output file." << std::endl;
        return;
    }

    char byte;
    while (inputFile.get(byte)) {
        // Convert byte to binary and write to the output file
        outputFile << std::bitset<8>(static_cast<unsigned char>(byte));
    }

    inputFile.close();
    outputFile.close();
    std::cout << "Binary data has been written to " << outputFilePath << std::endl;
}

int main() {
    std::string inputFilePath = "chopin_nocturne_9.mp3";  // Path to your MP3 file
    std::string outputFilePath = "output_binary.txt";  // Path to the output text file

    mp3ToBinary(inputFilePath, outputFilePath);
    return 0;
}
