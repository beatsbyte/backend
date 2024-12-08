#include <mpg123.h>
#include <iostream>
#include <fstream>
#include <vector>

bool convertMp3ToPcm(const char* mp3File, const char* pcmFile) {
    mpg123_handle* mh = nullptr;
    int err;

    // Initialize mpg123 library
    if (mpg123_init() != MPG123_OK) {
        std::cerr << "Failed to initialize mpg123 library.\n";
        return false;
    }

    // Create a new mpg123 handle
    mh = mpg123_new(nullptr, &err);
    if (mh == nullptr) {
        std::cerr << "Failed to create mpg123 handle: " << mpg123_plain_strerror(err) << "\n";
        mpg123_exit();
        return false;
    }

    // Open the MP3 file
    if (mpg123_open(mh, mp3File) != MPG123_OK) {
        std::cerr << "Failed to open MP3 file: " << mp3File << "\n";
        mpg123_delete(mh);
        mpg123_exit();
        return false;
    }

    // Get audio format information
    long rate;
    int channels, encoding;
    if (mpg123_getformat(mh, &rate, &channels, &encoding) != MPG123_OK) {
        std::cerr << "Failed to get audio format from MP3 file.\n";
        mpg123_close(mh);
        mpg123_delete(mh);
        mpg123_exit();
        return false;
    }

    // Set the output format to raw PCM
    mpg123_format_none(mh);
    mpg123_format(mh, rate, channels, MPG123_ENC_SIGNED_16);

    // Prepare to read and write PCM data
    std::ofstream pcmOutput(pcmFile, std::ios::binary);
    if (!pcmOutput.is_open()) {
        std::cerr << "Failed to open PCM output file: " << pcmFile << "\n";
        mpg123_close(mh);
        mpg123_delete(mh);
        mpg123_exit();
        return false;
    }

    const size_t buffer_size = 4096;
    std::vector<unsigned char> buffer(buffer_size);
    size_t done;

    // Decode MP3 and write PCM data
    while (mpg123_read(mh, buffer.data(), buffer_size, &done) == MPG123_OK) {
        pcmOutput.write(reinterpret_cast<char*>(buffer.data()), done);
    }

    // Clean up
    pcmOutput.close();
    mpg123_close(mh);
    mpg123_delete(mh);
    mpg123_exit();

    std::cout << "Conversion complete: " << pcmFile << "\n";
    return true;
}

int main() {
    const char* mp3File = "Real_Slim_Shady.mp3"; // Input MP3 file
    const char* pcmFile = "output.pcm"; // Output PCM file

    if (convertMp3ToPcm(mp3File, pcmFile)) {
        std::cout << "Successfully converted MP3 to PCM.\n";
    } else {
        std::cerr << "Failed to convert MP3 to PCM.\n";
    }

    return 0;
}
