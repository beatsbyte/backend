#include <iostream>
#include <fstream>
#include <string>
#include <zlib.h>

void compressFile(const std::string& inputFilePath, const std::string& outputFilePath) {
    // Open the input file in binary mode
    std::ifstream inputFile(inputFilePath, std::ios::binary);
    if (!inputFile) {
        std::cerr << "Error: Could not open input file for reading!" << std::endl;
        return;
    }

    // Create the output file stream for the compressed file
    gzFile outputFile = gzopen(outputFilePath.c_str(), "wb");
    if (!outputFile) {
        std::cerr << "Error: Could not open output file for writing!" << std::endl;
        return;
    }

    // Buffer for reading and writing
    const size_t bufferSize = 1024;
    char buffer[bufferSize];

    // Read from the input file and write to the compressed file
    while (!inputFile.eof()) {
        inputFile.read(buffer, bufferSize);
        gzwrite(outputFile, buffer, inputFile.gcount());
    }

    // Close the files
    inputFile.close();
    gzclose(outputFile);

    std::cout << "File compressed successfully!" << std::endl;
}

int main() {
    std::string inputFilePath = "chopin_nocturne_9.mp3";      // Input file path
    std::string outputFilePath = "output.gz";       // Compressed file path

    compressFile(inputFilePath, outputFilePath);
    return 0;
}
