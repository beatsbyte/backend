#include <iostream>
#include <fstream>
#include <lame/lame.h>
#include <mpg123.h>
#include <vector>
#include <string>

void changeBitrate(const std::string& inputFile, const std::string& outputFile, int bitrate) {
    // Initialize mpg123 and LAME
    mpg123_init();
    mpg123_handle* mh = mpg123_new(nullptr, nullptr);
    if (!mh) {
        std::cerr << "Failed to initialize mpg123.\n";
        return;
    }


    

    if (mpg123_open(mh, inputFile.c_str()) != MPG123_OK) {
        std::cerr << "Failed to open MP3 file: " << inputFile << "\n";
        mpg123_delete(mh);
        mpg123_exit();
        return;
    }

    // Get MP3 format details
    long rate;
    int channels, encoding;
    mpg123_getformat(mh, &rate, &channels, &encoding);
    std::cout << "MP3 Info: Rate = " << rate << " Hz, Channels = " << channels << "\n";

    // Initialize LAME encoder
    lame_t lame = lame_init();
    if (!lame) {
        std::cerr << "Failed to initialize LAME encoder.\n";
        mpg123_close(mh);
        mpg123_delete(mh);
        mpg123_exit();
        return;
    }

    lame_set_in_samplerate(lame, rate);
    lame_set_num_channels(lame, channels);
    lame_set_brate(lame, bitrate);
    lame_set_quality(lame, 2); // High-quality encoding

    if (lame_init_params(lame) < 0) {
        std::cerr << "Failed to initialize LAME parameters.\n";
        lame_close(lame);
        mpg123_close(mh);
        mpg123_delete(mh);
        mpg123_exit();
        return;
    }

    // Open output file
    std::ofstream output(outputFile, std::ios::binary);
    if (!output.is_open()) {
        std::cerr << "Failed to create output file: " << outputFile << "\n";
        lame_close(lame);
        mpg123_close(mh);
        mpg123_delete(mh);
        mpg123_exit();
        return;
    }

    // Buffers for decoding and encoding
    const size_t bufferSize = 8192;
    std::vector<unsigned char> inputBuffer(bufferSize);
    unsigned char mp3Buffer[bufferSize];
    size_t bytesRead;

    // Decoding and encoding loop
    while (mpg123_read(mh, inputBuffer.data(), bufferSize, &bytesRead) == MPG123_OK) {
        int encodedSize = lame_encode_buffer_interleaved(
            lame, reinterpret_cast<short int*>(inputBuffer.data()),
            bytesRead / (channels * 2), // Convert bytes to samples
            mp3Buffer, bufferSize);

        if (encodedSize < 0) {
            std::cerr << "Encoding error: " << encodedSize << '\n';
            break;
        }

        output.write(reinterpret_cast<char*>(mp3Buffer), encodedSize);
    }

    // Flush remaining MP3 data
    int finalEncodedSize = lame_encode_flush(lame, mp3Buffer, bufferSize);
    if (finalEncodedSize > 0) {
        output.write(reinterpret_cast<char*>(mp3Buffer), finalEncodedSize);
    }

    // Clean up
    lame_close(lame);
    mpg123_close(mh);
    mpg123_delete(mh);
    mpg123_exit();
    output.close();

    std::cout << "Bitrate changed successfully to " << bitrate << " kbps. Output: " << outputFile << "\n";
}

int main() {
    std::string inputFile;
    int bitrate;

    // std::cout << "Enter input MP3 file (with extension): ";
    // std::cin >> inputFile;
    inputFile = "Winter.mp3";

    // std::cout << "Enter desired bitrate (kbps): ";
    // std::cin >> bitrate;
    bitrate = 64;

    std::string outputFile = std::to_string(bitrate) + "bit_output.mp3";
    changeBitrate(inputFile, outputFile, bitrate);

    return 0;
}
