#include <iostream>
#include <fstream>
#include <ctime>
#include <vorbis/vorbisenc.h>

bool compressAudio(const char* inputFile, const char* outputFile, int channels, long sampleRate) {
    // Open the input file (raw PCM data)
    std::ifstream input(inputFile, std::ios::binary);
    if (!input.is_open()) {
        std::cerr << "Failed to open input file: " << inputFile << "\n";
        return false;
    }

    // Open the output file
    std::ofstream output(outputFile, std::ios::binary);
    if (!output.is_open()) {
        std::cerr << "Failed to open output file: " << outputFile << "\n";
        return false;
    }

    // Initialize Vorbis encoder
    vorbis_info vi;
    vorbis_info_init(&vi);
    if (vorbis_encode_init_vbr(&vi, channels, sampleRate, 0.5)) {
        std::cerr << "Failed to initialize Vorbis encoder.\n";
        vorbis_info_clear(&vi);
        return false;
    }

    vorbis_comment vc;
    vorbis_comment_init(&vc);
    vorbis_comment_add_tag(&vc, "ENCODER", "Custom Vorbis Encoder");

    vorbis_dsp_state vd;
    vorbis_block vb;
    ogg_stream_state os;
    vorbis_analysis_init(&vd, &vi);
    vorbis_block_init(&vd, &vb);
    ogg_stream_init(&os, rand());

    // Write Vorbis header
    ogg_packet header, header_comm, header_code;
    vorbis_analysis_headerout(&vd, &vc, &header, &header_comm, &header_code);
    ogg_stream_packetin(&os, &header);
    ogg_stream_packetin(&os, &header_comm);
    ogg_stream_packetin(&os, &header_code);

    ogg_page og;
    while (ogg_stream_flush(&os, &og)) {
        output.write(reinterpret_cast<const char*>(og.header), og.header_len);
        output.write(reinterpret_cast<const char*>(og.body), og.body_len);
    }

    // Encode raw PCM data
    const int BUFFER_SIZE = 4096;
    char pcmBuffer[BUFFER_SIZE];
    while (input.read(pcmBuffer, BUFFER_SIZE) || input.gcount() > 0) {
        float** buffer = vorbis_analysis_buffer(&vd, input.gcount() / (channels * 2));
        int samples = input.gcount() / (channels * 2);
        for (int i = 0; i < samples; ++i) {
            for (int j = 0; j < channels; ++j) {
                buffer[j][i] = ((pcmBuffer[(i * channels + j) * 2 + 1] << 8) |
                                (pcmBuffer[(i * channels + j) * 2] & 0xff)) /
                               32768.f;
            }
        }

        vorbis_analysis_wrote(&vd, samples);
        while (vorbis_analysis_blockout(&vd, &vb)) {
            vorbis_analysis(&vb, nullptr);
            vorbis_bitrate_addblock(&vb);

            while (vorbis_bitrate_flushpacket(&vd, &header)) {
                ogg_stream_packetin(&os, &header);
                while (ogg_stream_pageout(&os, &og)) {
                    output.write(reinterpret_cast<const char*>(og.header), og.header_len);
                    output.write(reinterpret_cast<const char*>(og.body), og.body_len);
                }
            }
        }
    }

    // End of stream
    vorbis_analysis_wrote(&vd, 0);
    while (vorbis_analysis_blockout(&vd, &vb)) {
        vorbis_analysis(&vb, nullptr);
        vorbis_bitrate_addblock(&vb);

        while (vorbis_bitrate_flushpacket(&vd, &header)) {
            ogg_stream_packetin(&os, &header);
            while (ogg_stream_pageout(&os, &og)) {
                output.write(reinterpret_cast<const char*>(og.header), og.header_len);
                output.write(reinterpret_cast<const char*>(og.body), og.body_len);
            }
        }
    }

    // Cleanup
    ogg_stream_clear(&os);
    vorbis_block_clear(&vb);
    vorbis_dsp_clear(&vd);
    vorbis_info_clear(&vi);
    vorbis_comment_clear(&vc);

    input.close();
    output.close();

    return true;
}

int main() {
    srand(static_cast<unsigned int>(time(nullptr)));

    const char* inputFile = "output.pcm"; // Input raw PCM file
    const char* outputFile = "output.ogg"; // Output Ogg/Vorbis file
    int channels = 2; // Number of audio channels
    long sampleRate = 44100; // Sampling rate in Hz

    if (compressAudio(inputFile, outputFile, channels, sampleRate)) {
        std::cout << "Audio compressed successfully to " << outputFile << "\n";
    } else {
        std::cerr << "Audio compression failed.\n";
    }

    return 0;
}
