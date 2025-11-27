#include "ExtractClass.h"
#include <iostream>
#include <fstream>
#include <string>
#include <cstdio>
#include <cstdlib>
#include <algorithm>

#ifdef _WIN32
#define popen  _popen
#define pclose _pclose
#endif

// Функция для удаления лишних пробелов между символами
std::string ExtractClass::removeSpacesBetweenChars(const std::string& input) {
    std::string result;

    for (size_t i = 0; i < input.length(); i++) {
        char c = input[i];

        // Если это пробел
        if (c == ' ') {
            // Проверяем, есть ли буквы/цифры до и после пробела
            bool hasCharBefore = (i > 0) && (isalnum(input[i - 1]) || input[i - 1] == '.' || input[i - 1] == ',');
            bool hasCharAfter = (i < input.length() - 1) && (isalnum(input[i + 1]) || input[i + 1] == '.' || input[i + 1] == ',');

            // Если пробел между символами - удаляем его
            if (hasCharBefore && hasCharAfter) {
                continue;
            }
            // Сохраняем обычные пробелы (между словами, в начале/конце строки)
            else {
                result += c;
            }
        }
        // Сохраняем все остальные символы
        else {
            result += c;
        }
    }

    return result;
}

// Функция для автоматического определения необходимости удаления пробелов
bool ExtractClass::needsSpaceRemoval(const std::string& content) {
    int spaceBetweenCharsCount = 0;
    int totalCharPairs = 0;

    for (size_t i = 1; i < content.length() - 1; i++) {
        char prev = content[i - 1];
        char curr = content[i];
        char next = content[i + 1];

        // Если текущий символ - пробел между двумя буквами/цифрами
        if (curr == ' ' && isalnum(prev) && isalnum(next)) {
            spaceBetweenCharsCount++;
        }
        // Считаем общее количество пар символов, разделенных пробелом
        else if (isalnum(prev) && isalnum(next)) {
            totalCharPairs++;
        }
    }

    // Если более 10% пар символов разделены пробелами, считаем что нужно удалять
    if (totalCharPairs > 0) {
        double ratio = static_cast<double>(spaceBetweenCharsCount) / totalCharPairs;
        return ratio > 0.3; // Порог 10%
    }

    return false;
}

void ExtractClass::processFile(const std::string& pdf, const std::string& txt) {
    // Конвертируем PDF → TXT
    std::string convertCmd = "pdftotext -raw -nopgbrk -enc UTF-8 \"" + pdf + "\" \"" + txt + "\"";

    int status = system(convertCmd.c_str());
    if (status != 0) {
        std::cerr << "Ошибка: pdftotext завершился с ошибкой для файла " << pdf << "\n";
        return;
    }

    // Читаем конвертированный текст
    std::ifstream inFile(txt);
    if (!inFile) {
        std::cerr << "Не удалось открыть файл для чтения: " << txt << "\n";
        return;
    }

    std::string content((std::istreambuf_iterator<char>(inFile)),
        std::istreambuf_iterator<char>());
    inFile.close();

    // Автоматически определяем, нужно ли удалять пробелы
    bool shouldRemoveSpaces = needsSpaceRemoval(content);

    if (shouldRemoveSpaces) {
        // Удаляем лишние пробелы
        std::string cleanedContent = removeSpacesBetweenChars(content);

        // Записываем обратно
        std::ofstream outFile(txt);
        if (!outFile) {
            std::cerr << "Не удалось открыть файл для записи: " << txt << "\n";
            return;
        }

        outFile << cleanedContent;
        outFile.close();
    }
}
