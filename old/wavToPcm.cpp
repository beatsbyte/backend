#include <iostream>
#include <fstream>
#include <cstdint>

struct WAVHeader {
    char riff[4];              // "RIFF"
    uint32_t fileSize;         // File size - 8 bytes
    char wave[4];              // "WAVE"
    char fmt[4];               // "fmt "
    uint32_t fmtSize;          // Size of the fmt chunk
    uint16_t audioFormat;      // Audio format (1 = PCM)
    uint16_t numChannels;      // Number of channels
    uint32_t sampleRate;       // Sampling rate
    uint32_t byteRate;         // Byte rate
    uint16_t blockAlign;       // Block align
    uint16_t bitsPerSample;    // Bits per sample
    char data[4];              // "data"
    uint32_t dataSize;         // Size of the data chunk
};

void convertWAVToPCM(const std::string& wavFile, const std::string& pcmFile) {
    std::ifstream input(wavFile, std::ios::binary);
    if (!input.is_open()) {
        std::cerr << "Failed to open WAV file: " << wavFile << "\n";
        return;
    }

    std::ofstream output(pcmFile, std::ios::binary);
    if (!output.is_open()) {
        std::cerr << "Failed to create PCM file: " << pcmFile << "\n";
        input.close();
        return;
    }

    // Read WAV header
    WAVHeader header;
    input.read(reinterpret_cast<char*>(&header), sizeof(WAVHeader));

    // Validate WAV file
    if (std::string(header.riff, 4) != "RIFF" || std::string(header.wave, 4) != "WAVE") {
        std::cerr << "Invalid WAV file format.\n";
        input.close();
        output.close();
        return;
    }

    std::cout << "WAV Info:\n";
    std::cout << "  Channels: " << header.numChannels << "\n";
    std::cout << "  Sample Rate: " << header.sampleRate << " Hz\n";
    std::cout << "  Bits Per Sample: " << header.bitsPerSample << "\n";

    // Write raw PCM data to the output file
    input.seekg(sizeof(WAVHeader), std::ios::beg); // Skip the header
    char buffer[4096];
    while (!input.eof()) {
        input.read(buffer, sizeof(buffer));
        output.write(buffer, input.gcount());
    }

    input.close();
    output.close();

    std::cout << "PCM data written to: " << pcmFile << "\n";
}

int main() {
    std::string wavFile = "input.wav";
    std::cin << wavFile;

    wavFile += ".wav";
    std::string pcmFile = wavFile + "_output.pcm";

    convertWAVToPCM(wavFile, pcmFile);

    return 0;
}
