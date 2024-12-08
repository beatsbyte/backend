#include <iostream>
#include <cstdlib>
#include <string>

void decompressAudio(const std::string& zipFile, const std::string& outputFile) {
    // Распаковываем ZIP-архив
    std::string unzipCommand = "unzip \"" + zipFile + "\"";
    int result = std::system(unzipCommand.c_str());
    
    if (result == 0) {
        std::cout << "Архив успешно распакован: " << zipFile << std::endl;

        // Извлекаем имя MP3-файла из имени архива
        std::string mp3File = zipFile.substr(0, zipFile.size() - 4); // Удаляем ".zip"

        // Декодируем MP3 в WAV с помощью FFmpeg
        std::string decodeCommand = "ffmpeg -i \"" + mp3File + "\" \"" + outputFile + ".mp3\"";
        result = std::system(decodeCommand.c_str());

        if (result == 0) {
            std::cout << "Аудиофайл успешно декодирован в WAV: " << outputFile << ".mp3" << std::endl;
        } else {
            std::cerr << "Ошибка при декодировании MP3-файла." << std::endl;
        }

    } else {
        std::cerr << "Ошибка при распаковке ZIP-архива." << std::endl;
    }
}

int main() {
    std::string zipFile;
    std::cin >> zipFile;
    std::cout << zipFile;
    //zipFile += ".zip";
    std::string outputFile = "encoded_" + zipFile;

    decompressAudio(zipFile, outputFile);

    return 0;
}
