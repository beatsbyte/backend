#include "inplace_converter.hpp"

namespace converter {

int getMP3Bitrate(mpg123_handle* mh) {
    mpg123_frameinfo frameInfo;
    if (mpg123_info(mh, &frameInfo) == MPG123_OK) {
        return frameInfo.bitrate;
    } else {
        std::cerr << "Failed to retrieve MP3 bitrate.\n";
        return -1;
    }
}

std::string changeBitrateDirectly(const std::string& inputMP3Data) {
    // Initialize mpg123
    mpg123_init();
    mpg123_handle* mh = mpg123_new(nullptr, nullptr);
    if (!mh) {
        std::cerr << "Failed to initialize mpg123.\n";
        mpg123_exit();
        return {};
    }

    if (mpg123_open_feed(mh) != MPG123_OK) {
        std::cerr << "Failed to open mpg123 feed.\n";
        mpg123_delete(mh);
        mpg123_exit();
        return {};
    }

    // Feed input data
    if (mpg123_feed(mh, reinterpret_cast<const unsigned char*>(inputMP3Data.data()), inputMP3Data.size()) != MPG123_OK) {
        std::cerr << "Failed to feed data into mpg123.\n";
        mpg123_delete(mh);
        mpg123_exit();
        return {};
    }

    // Get current bitrate
    int currentBitrate = getMP3Bitrate(mh);
    if (currentBitrate == -1) {
        mpg123_delete(mh);
        mpg123_exit();
        return {};
    }

    std::cout << "Current bitrate: " << currentBitrate << " kbps\n";

    // If the current bitrate is 64 or lower, return the input data unchanged
    if (currentBitrate <= 64) {
        std::cout << "Bitrate is already 64 kbps or lower. No conversion needed.\n";
        mpg123_delete(mh);
        mpg123_exit();
        return inputMP3Data;
    }

    // Get audio format
    long rate;
    int channels, encoding;
    if (mpg123_getformat(mh, &rate, &channels, &encoding) != MPG123_OK) {
        std::cerr << "Failed to get MP3 format.\n";
        mpg123_delete(mh);
        mpg123_exit();
        return {};
    }

    // Initialize LAME encoder
    lame_t lame = lame_init();
    if (!lame) {
        std::cerr << "Failed to initialize LAME encoder.\n";
        mpg123_delete(mh);
        mpg123_exit();
        return {};
    }

    lame_set_in_samplerate(lame, rate);
    lame_set_num_channels(lame, channels);
    lame_set_brate(lame, currentBitrate / 2); // Set the target bitrate
    lame_set_quality(lame, 5);                // Set encoding quality (5 = medium)

    if (lame_init_params(lame) < 0) {
        std::cerr << "Failed to initialize LAME parameters.\n";
        lame_close(lame);
        mpg123_delete(mh);
        mpg123_exit();
        return {};
    }

    // Buffers
    const size_t bufferSize = 8192;
    std::vector<unsigned char> mp3Buffer(bufferSize);
    std::vector<unsigned char> pcmBuffer(bufferSize * 2); // Large enough for stereo

    std::ostringstream outputStream; // For in-memory output
    size_t bytesRead;
    int err;

    // Decode and encode loop
    while ((err = mpg123_read(mh, pcmBuffer.data(), pcmBuffer.size(), &bytesRead)) == MPG123_OK) {
        int samples = bytesRead / (2 * channels); // 2 bytes per sample per channel
        int encodedBytes = lame_encode_buffer_interleaved(
            lame, reinterpret_cast<short int*>(pcmBuffer.data()), samples,
            mp3Buffer.data(), mp3Buffer.size());

        if (encodedBytes > 0) {
            outputStream.write(reinterpret_cast<char*>(mp3Buffer.data()), encodedBytes);
        }
    }

    if (err != MPG123_DONE) {
        std::cerr << "Decoding error: " << mpg123_strerror(mh) << '\n';
    }

    // Flush remaining MP3 data
    int finalBytes = lame_encode_flush(lame, mp3Buffer.data(), mp3Buffer.size());
    if (finalBytes > 0) {
        outputStream.write(reinterpret_cast<char*>(mp3Buffer.data()), finalBytes);
    }

    // Cleanup
    lame_close(lame);
    mpg123_delete(mh);
    mpg123_exit();

    std::cout << "Bitrate conversion complete.\n";

    // Return output as string
    return outputStream.str();
}

} // namespace converter
