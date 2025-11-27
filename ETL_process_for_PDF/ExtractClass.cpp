#include "ExtractClass.h"
#include <iostream>
#include <fstream>
#include <string>
#include <cstdio>
#include <cstdlib>
#include <algorithm>

//Удаление лишних пробелов между символами
std::string ExtractClass::removeSpacesBetweenChars(const std::string& input) 
{
    std::string result;

    for (size_t i = 0; i < input.length(); i++) 
    {
        char c = input[i];

        if (c == ' ') 
        {
            bool hasCharBefore = (i > 0) && (isalnum(input[i - 1]) || input[i - 1] == '.' || input[i - 1] == ',');
            bool hasCharAfter = (i < input.length() - 1) && (isalnum(input[i + 1]) || input[i + 1] == '.' || input[i + 1] == ',');

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

//Автоматическое определения необходимости удаления пробелов
bool ExtractClass::needsSpaceRemoval(const std::string& content)
{
    int spaceBetweenCharsCount = 0;
    int totalCharPairs = 0;

    for (size_t i = 1; i < content.length() - 1; i++) 
    {
        char prev = content[i - 1];
        char curr = content[i];
        char next = content[i + 1];

        if (curr == ' ' && isalnum(prev) && isalnum(next)) 
        {
            spaceBetweenCharsCount++;
        }
        else if (isalnum(prev) && isalnum(next)) 
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

void ExtractClass::processFile(const std::string& pdf, const std::string& txt)
{
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

    bool shouldRemoveSpaces = needsSpaceRemoval(content);

    if (shouldRemoveSpaces) 
    {
        std::string cleanedContent = removeSpacesBetweenChars(content);

        std::ofstream outFile(txt);
        if (!outFile) 
        {
            std::cerr << "Не удалось открыть файл для записи: " << txt << "\n";
            return;
        }

        outFile << cleanedContent;
        outFile.close();
    }
}
