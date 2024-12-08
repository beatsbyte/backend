#include <mpg123.h>
#include <vorbis/vorbisenc.h>
#include <iostream>
#include <fstream>
#include <vector>

void convertMP3ToOGG(const std::string& mp3File, const std::string& oggFile) {
    // Initialize mpg123
    mpg123_init();
    mpg123_handle* mh = mpg123_new(nullptr, nullptr);
    if (mpg123_open(mh, mp3File.c_str()) != MPG123_OK) {
        std::cerr << "Failed to open MP3 file: " << mp3File << "\n";
        mpg123_delete(mh);
        mpg123_exit();
        return;
    }

    // Get MP3 format information
    long rate;
    int channels, encoding;
    mpg123_getformat(mh, &rate, &channels, &encoding);

    std::cout << "MP3 Info: Rate=" << rate << ", Channels=" << channels << "\n";

    // Initialize Vorbis encoder
    vorbis_info vi;
    vorbis_info_init(&vi);
    if (vorbis_encode_init_vbr(&vi, channels, rate, 0.4) != 0) {
        std::cerr << "Failed to initialize Vorbis encoder.\n";
        mpg123_close(mh);
        mpg123_delete(mh);
        mpg123_exit();
        return;
    }

    vorbis_comment vc;
    vorbis_comment_init(&vc);
    vorbis_comment_add_tag(&vc, "ENCODER", "MP3 to OGG Converter");

    vorbis_dsp_state vd;
    vorbis_analysis_init(&vd, &vi);

    vorbis_block vb;
    vorbis_block_init(&vd, &vb);

    ogg_stream_state os;
    ogg_stream_init(&os, rand()); // Random serial number

    // Write OGG headers
    ogg_packet header, header_comm, header_code;
    vorbis_analysis_headerout(&vd, &vc, &header, &header_comm, &header_code);
    ogg_stream_packetin(&os, &header);
    ogg_stream_packetin(&os, &header_comm);
    ogg_stream_packetin(&os, &header_code);

    ogg_page og;
    std::ofstream oggOutput(oggFile, std::ios::binary);
    while (ogg_stream_flush(&os, &og)) {
        oggOutput.write(reinterpret_cast<char*>(og.header), og.header_len);
        oggOutput.write(reinterpret_cast<char*>(og.body), og.body_len);
    }

    // Decode MP3 and encode to OGG
    const size_t bufferSize = 4096;
    std::vector<unsigned char> mp3Buffer(bufferSize);
    unsigned char* rawBuffer = new unsigned char[bufferSize];
    size_t bytesRead;

    while (mpg123_read(mh, rawBuffer, bufferSize, &bytesRead) == MPG123_OK) {
        // Decode raw PCM data
        float** vorbisBuffer = vorbis_analysis_buffer(&vd, bytesRead / (channels * 2));
        for (size_t i = 0; i < bytesRead / (channels * 2); ++i) {
            for (int c = 0; c < channels; ++c) {
                vorbisBuffer[c][i] = ((rawBuffer[i * channels * 2 + c * 2 + 1] << 8) |
                                      (0x00FF & rawBuffer[i * channels * 2 + c * 2])) /
                                     32768.f;
            }
        }

        vorbis_analysis_wrote(&vd, bytesRead / (channels * 2));

        // Process Vorbis blocks
        while (vorbis_analysis_blockout(&vd, &vb) == 1) {
            vorbis_analysis(&vb, nullptr);
            vorbis_bitrate_addblock(&vb);

            ogg_packet op;
            while (vorbis_bitrate_flushpacket(&vd, &op)) {
                ogg_stream_packetin(&os, &op);

                while (ogg_stream_pageout(&os, &og)) {
                    oggOutput.write(reinterpret_cast<char*>(og.header), og.header_len);
                    oggOutput.write(reinterpret_cast<char*>(og.body), og.body_len);
                }
            }
        }
    }

    // Finish encoding
    vorbis_analysis_wrote(&vd, 0);
    while (vorbis_analysis_blockout(&vd, &vb) == 1) {
        vorbis_analysis(&vb, nullptr);
        vorbis_bitrate_addblock(&vb);

        ogg_packet op;
        while (vorbis_bitrate_flushpacket(&vd, &op)) {
            ogg_stream_packetin(&os, &op);

            while (ogg_stream_pageout(&os, &og)) {
                oggOutput.write(reinterpret_cast<char*>(og.header), og.header_len);
                oggOutput.write(reinterpret_cast<char*>(og.body), og.body_len);
            }
        }
    }

    // Cleanup
    oggOutput.close();
    delete[] rawBuffer;

    ogg_stream_clear(&os);
    vorbis_block_clear(&vb);
    vorbis_dsp_clear(&vd);
    vorbis_comment_clear(&vc);
    vorbis_info_clear(&vi);

    mpg123_close(mh);
    mpg123_delete(mh);
    mpg123_exit();

    std::cout << "MP3 to OGG conversion complete: " << oggFile << "\n";
}
int main(){
    std::string mp3;
    std::cin >> mp3;

    std::string ogg = "output.ogg";

    convertMP3ToOGG(mp3,ogg);


}