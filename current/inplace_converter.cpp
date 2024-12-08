#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <mpg123.h>
#include <lame/lame.h>

int getMP3Bitrate(mpg123_handle* mh) {
    mpg123_frameinfo frameInfo;
    if (mpg123_info(mh, &frameInfo) == MPG123_OK) {
        return frameInfo.bitrate;
    } else {
        std::cerr << "Failed to retrieve MP3 bitrate.\n";
        return -1;
    }
}

void changeBitrateDirectly(const std::string& inputFile, const std::string& outputFile, int compressionLevel) {
    // Initialize mpg123
    mpg123_init();
    mpg123_handle* mh = mpg123_new(nullptr, nullptr);
    if (mpg123_open(mh, inputFile.c_str()) != MPG123_OK) {
        std::cerr << "Failed to open input MP3 file.\n";
        mpg123_delete(mh);
        mpg123_exit();
        return;
    }

    // Get current bitrate
    int currentBitrate = getMP3Bitrate(mh);
    if (currentBitrate == -1) {
        mpg123_close(mh);
        mpg123_delete(mh);
        mpg123_exit();
        return;
    }

    std::cout << "Current bitrate: " << currentBitrate << " kbps\n";

    // If the current bitrate is 64 or lower, do nothing
    if (currentBitrate <= 64) {
        std::cout << "Bitrate is already 64 kbps or lower. No conversion needed.\n";
        mpg123_close(mh);
        mpg123_delete(mh);
        mpg123_exit();
        return;
    }

    // Get audio format
    long rate;
    int channels, encoding;
    if (mpg123_getformat(mh, &rate, &channels, &encoding) != MPG123_OK) {
        std::cerr << "Failed to get MP3 format.\n";
        mpg123_close(mh);
        mpg123_delete(mh);
        mpg123_exit();
        return;
    }

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

    if(compressionLevel == 0){
    lame_set_brate(lame, 2*currentBitrate/3); // Set the target bitrate
    }
    else if (compressionLevel == 1) {
    lame_set_brate(lame, currentBitrate/2); // Set the target bitrate
    }
    else if (compressionLevel == 2){
    lame_set_brate(lame, currentBitrate/4); // Set the target bitrate
    }

    lame_set_quality(lame, 5);           // Set encoding quality (5 = medium)

    if (lame_init_params(lame) < 0) {
        std::cerr << "Failed to initialize LAME parameters.\n";
        lame_close(lame);
        mpg123_close(mh);
        mpg123_delete(mh);
        mpg123_exit();
        return;
    }

    // Open output MP3 file
    std::ofstream output(outputFile, std::ios::binary);
    if (!output.is_open()) {
        std::cerr << "Failed to open output file.\n";
        lame_close(lame);
        mpg123_close(mh);
        mpg123_delete(mh);
        mpg123_exit();
        return;
    }

    // Buffer setup
    const size_t bufferSize = 8192;
    std::vector<unsigned char> mp3Buffer(bufferSize);
    std::vector<unsigned char> pcmBuffer(bufferSize * 2); // Large enough for stereo

    size_t bytesRead;
    int err;

    // Decode and encode loop
    while ((err = mpg123_read(mh, pcmBuffer.data(), pcmBuffer.size(), &bytesRead)) == MPG123_OK) {
        int samples = bytesRead / (2 * channels); // 2 bytes per sample per channel
        int encodedBytes = lame_encode_buffer_interleaved(
            lame, reinterpret_cast<short int*>(pcmBuffer.data()), samples, 
            mp3Buffer.data(), mp3Buffer.size());

        if (encodedBytes > 0) {
            output.write(reinterpret_cast<char*>(mp3Buffer.data()), encodedBytes);
        }
    }

    if (err != MPG123_DONE) {
        std::cerr << "Decoding error: " << mpg123_strerror(mh) << '\n';
    }

    // Flush remaining MP3 data
    int finalBytes = lame_encode_flush(lame, mp3Buffer.data(), mp3Buffer.size());
    if (finalBytes > 0) {
        output.write(reinterpret_cast<char*>(mp3Buffer.data()), finalBytes);
    }

    // Cleanup
    output.close();
    lame_close(lame);
    mpg123_close(mh);
    mpg123_delete(mh);
    mpg123_exit();

    std::cout << "Bitrate conversion complete. File saved to " << outputFile << '\n';
}

int main() {
    std::string inputFile;

    std::cin >> inputFile;
    std::string outputFile = inputFile + "output";
    inputFile += ".mp3";
    std::cout << "Choose the level of compression: ";
    int compressionLevel;
    std::cin >> compressionLevel;


    if(compressionLevel == 0){
    outputFile += "Low";
    }
    else if (compressionLevel == 1) {
    outputFile += "Medium";
    }
    else if (compressionLevel == 2){
        outputFile += "High";
    }


    outputFile += ".mp3";

    changeBitrateDirectly(inputFile, outputFile, compressionLevel);

    return 0;
}
