#include <mpg123.h>
#include <iostream>
#include <fstream>
#include <vector>

void decodeMP3ToPCM(const std::string& inputFile, const std::string& pcmFile) {
    mpg123_init(); // Initialize the library
    mpg123_handle* mh = mpg123_new(nullptr, nullptr); // Create a new handle

    if (mpg123_open(mh, inputFile.c_str()) != MPG123_OK) {
        std::cerr << "Failed to open MP3 file: " << inputFile << "\n";
        mpg123_delete(mh);
        mpg123_exit();
        return;
    }

    // Get audio format (sampling rate, channels, etc.)
    long rate;
    int channels, encoding;
    mpg123_getformat(mh, &rate, &channels, &encoding);

    std::cout << "MP3 Info: Rate=" << rate << ", Channels=" << channels << "\n";


    // Calculate and display bitrate
    double secondsPerFrame = mpg123_tpf(mh); // Time per frame in seconds
    int avgBitrate = static_cast<int>((1152 * 8 * channels) / secondsPerFrame / 1000); // Approximation for MP3 files
    std::cout << "Estimated Bitrate: " << avgBitrate << " kbps\n";

    if(avgBitrate <= 64){

    // Open output PCM file
    std::ofstream pcmOutput(pcmFile, std::ios::binary);
    if (!pcmOutput.is_open()) {
        std::cerr << "Failed to open output file: " << pcmFile << "\n";
        mpg123_close(mh);
        mpg123_delete(mh);
        mpg123_exit();
        return;
    }

    // Buffer for decoded audio
    const size_t bufferSize = 8192;
    std::vector<unsigned char> buffer(bufferSize);

    size_t bytesRead;
    int err;

    // Read and decode MP3 data
    while ((err = mpg123_read(mh, buffer.data(), bufferSize, &bytesRead)) == MPG123_OK) {
        pcmOutput.write(reinterpret_cast<char*>(buffer.data()), bytesRead);
    }

    if (err != MPG123_DONE) {
        std::cerr << "Decoding error: " << mpg123_strerror(mh) << "\n";
    }

    pcmOutput.close();
    std::cout << "Decoding complete. PCM file saved at: " << pcmFile << "\n";

    }

    else{
        std::cout << "Your bitrate is already 64bit";
    }

    mpg123_close(mh);
    mpg123_delete(mh);
    mpg123_exit();

}


int main() {
    std::string inputFile = "input.mp3";
    std::string pcmFile = "decoded.pcm";

    // Step 1: Decode MP3 to PCM
    decodeMP3ToPCM(inputFile, pcmFile);

    return 0;
}
