#include <vorbis/vorbisfile.h>
#include <lame/lame.h>
#include <iostream>
#include <fstream>
#include <vector>

bool convertOggToMp3(const char* oggFile, const char* mp3File) {
    // Open the OGG file
    OggVorbis_File vf;
    if (ov_fopen(oggFile, &vf) < 0) {
        std::cerr << "Failed to open OGG file: " << oggFile << "\n";
        return false;
    }

    // Get audio information
    vorbis_info* vi = ov_info(&vf, -1);
    if (!vi) {
        std::cerr << "Failed to retrieve OGG file information.\n";
        ov_clear(&vf);
        return false;
    }

    int channels = vi->channels;
    long sampleRate = vi->rate;

    // Initialize LAME encoder
    lame_t lame = lame_init();
    if (!lame) {
        std::cerr << "Failed to initialize LAME encoder.\n";
        ov_clear(&vf);
        return false;
    }

    lame_set_in_samplerate(lame, sampleRate);
    lame_set_num_channels(lame, channels);
    lame_set_VBR(lame, vbr_default);
    if (lame_init_params(lame) < 0) {
        std::cerr << "Failed to set LAME encoder parameters.\n";
        lame_close(lame);
        ov_clear(&vf);
        return false;
    }

    // Open MP3 output file
    std::ofstream mp3Output(mp3File, std::ios::binary);
    if (!mp3Output.is_open()) {
        std::cerr << "Failed to open MP3 output file: " << mp3File << "\n";
        lame_close(lame);
        ov_clear(&vf);
        return false;
    }

    // Read and convert OGG data
    const size_t PCM_BUFFER_SIZE = 4096;
    const size_t MP3_BUFFER_SIZE = 4096;

    std::vector<short> pcmBuffer(PCM_BUFFER_SIZE * channels);
    std::vector<unsigned char> mp3Buffer(MP3_BUFFER_SIZE);

    int current_section;
    long samples_read;

    while ((samples_read = ov_read(&vf, reinterpret_cast<char*>(pcmBuffer.data()),
                                   PCM_BUFFER_SIZE * sizeof(short) * channels, 0, 2, 1, &current_section)) > 0) {
        int samples = samples_read / (sizeof(short) * channels);

        int mp3Bytes = lame_encode_buffer_interleaved(
            lame, pcmBuffer.data(), samples, mp3Buffer.data(), MP3_BUFFER_SIZE);

        if (mp3Bytes < 0) {
            std::cerr << "LAME encoding error.\n";
            break;
        }

        mp3Output.write(reinterpret_cast<char*>(mp3Buffer.data()), mp3Bytes);
    }

    // Flush remaining MP3 data
    int mp3Bytes = lame_encode_flush(lame, mp3Buffer.data(), MP3_BUFFER_SIZE);
    if (mp3Bytes > 0) {
        mp3Output.write(reinterpret_cast<char*>(mp3Buffer.data()), mp3Bytes);
    }

    // Clean up
    mp3Output.close();
    lame_close(lame);
    ov_clear(&vf);

    std::cout << "Conversion complete: " << mp3File << "\n";
    return true;
}

int main() {
    const char* oggFile = "output.ogg"; // Input OGG file
    const char* mp3File = "Slim_Ret.mp3"; // Output MP3 file

    if (convertOggToMp3(oggFile, mp3File)) {
        std::cout << "Successfully converted OGG to MP3.\n";
    } else {
        std::cerr << "Failed to convert OGG to MP3.\n";
    }

    return 0;
}
