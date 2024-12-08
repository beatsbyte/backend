#include <iostream>
#include <fstream>
#include <lame/lame.h>
#include <string>
#include <mpg123.h>

void changeBitrate(const std::string& inputFile, const std::string& outputFile, int bitrate) {
    mpg123_init(); // Initialize the library
    mpg123_handle* mh = mpg123_new(nullptr, nullptr); // Create a new handle
    
    std::ofstream output(outputFile, std::ios::binary);


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



    if (!output) {
        std::cerr << "Error creating files.\n";
        return;
    }

    // Initialize LAME encoders
    lame_t lame = lame_init();
    if (!lame) {
        std::cerr << "Failed to initialize LAME encoder.\n";
        return;
    }

    // Set desired bitrate
    lame_set_brate(lame, bitrate);

    // Initialize parameters
    if (lame_init_params(lame) < 0) {
        std::cerr << "Failed to initialize LAME parameters.\n";
        lame_close(lame);
        return;
    }


    // Buffer for audio processing
    const int BUFFER_SIZE = 8192;
    short int pcmBuffer[BUFFER_SIZE * 2]; // Stereo PCM buffer
    unsigned char mp3Buffer[BUFFER_SIZE];

    // Decode and encode
    size_t readSize;
    do {
        // Read PCM data (assuming decoded PCM input for simplicity)
        input.read(reinterpret_cast<char*>(pcmBuffer), sizeof(pcmBuffer));
        readSize = input.gcount() / sizeof(short int);

        if (readSize > 0) {
            int encodedSize = lame_encode_buffer_interleaved(
                lame, pcmBuffer, static_cast<int>(readSize / 2), mp3Buffer, BUFFER_SIZE);

            if (encodedSize < 0) {
                std::cerr << "Encoding error: " << encodedSize << '\n';
                break;
            }

            output.write(reinterpret_cast<char*>(mp3Buffer), encodedSize);
        }
    } while (readSize > 0);

    // Flush remaining MP3 data
    int finalEncodedSize = lame_encode_flush(lame, mp3Buffer, BUFFER_SIZE);
    if (finalEncodedSize > 0) {
        output.write(reinterpret_cast<char*>(mp3Buffer), finalEncodedSize);
    }

    // Clean up
    lame_close(lame);
    input.close();
    output.close();

    std::cout << "Bitrate changed successfully to " << bitrate << " kbps.\n";
}


int main() {
    std::string mp3;

    std::cin << mp3;
    mp3 += ".mp3";

    int bitrate;
    
    std::cin >> bitrate;
    std::string outputFile = std::to_string(bitrate) + "bit_" +"output.mp3";
    changeBitrate(mp3, outputFile, bitrate);

    return 0;
}
