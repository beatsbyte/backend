#include <iostream>
#include <cstdlib>
#include <string>

void compressAudio(const std::string& inputFile, const std::string& outputFile, int bitrate) {
    // Формируем команду для FFmpeg
    std::string command = "ffmpeg -i \"" + inputFile + "\" -b:a " + std::to_string(bitrate) + "k \"" + outputFile + ".mp3\"";  // Добавлено расширение .mp3
    int result = std::system(command.c_str());
    
    if (result == 0) {
        std::cout << "Аудиофайл успешно сжат: " << outputFile << ".mp3" << std::endl;

        // Сжимаем аудиофайл с помощью zip
        std::string zipCommand = "zip -9 \"" + outputFile + ".mp3.zip\" \"" + outputFile + ".mp3\"";
        result = std::system(zipCommand.c_str());
        
        if (result == 0) {
            std::cout << "Аудиофайл успешно сжат в архив: " << outputFile << ".mp3.zip" << std::endl;
        } else {
            std::cerr << "Ошибка при сжатии файла в архив." << std::endl;
        }

    } else {
        std::cerr << "Ошибка при сжатии аудиофайла." << std::endl;
    }
}

int main() {
    std::string inputFile;
    std::cin >> inputFile;
    inputFile += ".mp3";
    
    int bitrate;

    std::cout << "Введите путь к исходному аудиофайлу: ";
    // std::getline(std::cin, inputFile);

    std::cout << "Введите путь для сохранения сжатого файла: ";
    // std::getline(std::cin, outputFile);

    std::cout << "Введите желаемый битрейт (например, 128 для 128 kbps): ";
    std::cin >> bitrate;
    std::string outputFile = "enc_-9z_"+ inputFile + std::to_string(bitrate);
    compressAudio(inputFile, outputFile, bitrate);

    return 0;
}

