#include "ExtractClass.h"
#include <iostream>
#include <fstream>
#include <string>
#include <cstdio>
#include <cstdlib>
#include <algorithm>
#include <cctype>
#include <sstream>
#include <filesystem> // Используем для поиска временных файлов картинок
#include <vector>

namespace fs = std::filesystem;

// Удаление лишних пробелов между символами
std::string ExtractClass::removeSpacesBetweenChars(const std::string& input)
{
    std::string result;

    for (size_t i = 0; i < input.length(); i++)
    {
        char c = input[i];

        if (c == ' ')
        {
            bool hasCharBefore = (i > 0) && (isalnum(static_cast<unsigned char>(input[i - 1])) || input[i - 1] == '.' || 
                input[i - 1] == ',' || input[i - 1] == '(' || input[i - 1] == ')' || input[i - 1] == '/');
            bool hasCharAfter = (i < input.length() - 1) && (isalnum(static_cast<unsigned char>(input[i + 1])) || 
                input[i + 1] == '.' || input[i + 1] == ',' || input[i + 1] == '(' || input[i + 1] == ')' || input[i + 1] == ')');

            if (hasCharBefore && hasCharAfter)
            {
                continue;
            }
            else
            {
                result += c;
            }
        }
        else
        {
            result += c;
        }
    }

    return result;
}

// Удаление отступов согласно правилам
std::string ExtractClass::removeIndents(const std::string& content)
{
    std::string result;
    std::istringstream stream(content);
    std::string line;
    std::string prevLine;

    while (std::getline(stream, line))
    {
        bool shouldRemoveNewline = false;

        if (!prevLine.empty())
        {
            char lastChar = prevLine.back();
            if (lastChar == ',' || lastChar == '/')
            {
                shouldRemoveNewline = true;
            }
        }

        if (!line.empty() && !shouldRemoveNewline)
        {
            char firstChar = line.front();
            if (!std::isupper(static_cast<unsigned char>(firstChar)))
            {
                shouldRemoveNewline = true;
            }
        }

        if (shouldRemoveNewline)
        {
            if (!result.empty() && result.back() != ' ')
            {
                result += ' ';
            }
            result += line;
        }
        else
        {
            if (!result.empty())
            {
                result += '\n';
            }
            result += line;
        }

        prevLine = line;
    }

    return result;
}

// Автоматическое определения необходимости удаления пробелов
bool ExtractClass::needsSpaceRemoval(const std::string& content)
{
    int spaceBetweenCharsCount = 0;
    int totalCharPairs = 0;

    for (size_t i = 1; i < content.length() - 1; i++)
    {
        char prev = content[i - 1];
        char curr = content[i];
        char next = content[i + 1];

        if (curr == ' ' && isalnum(static_cast<unsigned char>(prev)) && isalnum(static_cast<unsigned char>(next)))
        {
            spaceBetweenCharsCount++;
        }
        else if (isalnum(static_cast<unsigned char>(prev)) && isalnum(static_cast<unsigned char>(next)))
        {
            totalCharPairs++;
        }
    }

    if (totalCharPairs > 0)
    {
        double ratio = static_cast<double>(spaceBetweenCharsCount) / totalCharPairs;
        return ratio > 0.3;
    }

    return false;
}

// Проверка, является ли текст читаемым (для English)
bool ExtractClass::isTextMeaningful(const std::string& content)
{
    int alphaCount = 0;
    for (char c : content)
    {
        if (isalpha(static_cast<unsigned char>(c)))
            alphaCount++;
    }
    return alphaCount > 50; // Если меньше 50 букв, значит это скан или пустой файл
}

// OCR процесс (English)
bool ExtractClass::runOCR(const std::string& pdf, const std::string& txt)
{
    const std::string imgPrefix = "ocr_temp_img";

    // Используем синтаксис Xpdf 4.06 (без флага -sep)
    std::string pdfToImg = "pdftoppm -q -r 1200 \"" + pdf + "\" " + imgPrefix;

    if (system(pdfToImg.c_str()) != 0)
    {
        return false;
    }

    // Очищаем txt файл перед записью результатов OCR
    {
        std::ofstream clear(txt, std::ios::trunc);
    }

    std::vector<std::string> pages;
    for (const auto& entry : fs::directory_iterator("."))
    {
        std::string filename = entry.path().filename().string();
        if (filename.find(imgPrefix) == 0 && filename.find(".ppm") != std::string::npos)
        {
            pages.push_back(filename);
        }
    }
    std::sort(pages.begin(), pages.end());

    bool processed = false;
    for (const std::string& pageImg : pages)
    {
        // Английский язык (-l eng)
        std::string ocrCmd = "tesseract \"" + pageImg + "\" stdout -l eng --dpi 1200 >> \"" + txt + "\"";
        if (system(ocrCmd.c_str()) == 0)
        {
            processed = true;
        }
        fs::remove(pageImg);
    }

    return processed;
}

void ExtractClass::processFile(const std::string& pdf, const std::string& txt)
{
    // Попытка обычного извлечения текста
    std::string convertCmd = "pdftotext -raw -nopgbrk -enc UTF-8 \"" + pdf + "\" \"" + txt + "\"";

    int status = system(convertCmd.c_str());
    if (status != 0)
    {
        std::cerr << "Ошибка: pdftotext завершился с ошибкой для файла " << pdf << "\n";
        return;
    }

    std::ifstream inFile(txt);
    if (!inFile)
    {
        std::cerr << "Не удалось открыть файл для чтения: " << txt << "\n";
        return;
    }

    std::string content((std::istreambuf_iterator<char>(inFile)),
        std::istreambuf_iterator<char>());
    inFile.close();

    // Если текста нет или он не "осмысленный" (скан), запускаем OCR
    if (!isTextMeaningful(content))
    {
        std::cout << "PDF looks like a scan, starting OCR (English)..." << std::endl;
        if (!runOCR(pdf, txt))
        {
            std::cerr << "Ошибка OCR для файла: " << pdf << "\n";
            return;
        }

        // Загружаем новый контент, полученный через Tesseract
        std::ifstream ocrFile(txt);
        content.assign((std::istreambuf_iterator<char>(ocrFile)), std::istreambuf_iterator<char>());
        ocrFile.close();
    }

    if (content.empty())
    {
        std::cerr << "Предупреждение: Текст не обнаружен даже после OCR.\n";
        return;
    }

    // Обработка текста функциями из твоего примера
    bool shouldRemoveSpaces = needsSpaceRemoval(content);
    std::string cleanedContent = removeIndents(content);

    if (shouldRemoveSpaces)
    {
        cleanedContent = removeSpacesBetweenChars(cleanedContent);
    }

    std::ofstream outFile(txt);
    if (!outFile)
    {
        std::cerr << "Не удалось открыть файл для записи: " << txt << "\n";
        return;
    }

    outFile << cleanedContent;
    outFile.close();

    std::cout << "Успешно обработано: " << pdf << std::endl;
}