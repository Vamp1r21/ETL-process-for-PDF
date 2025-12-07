#include "ExtractClass.h"
#include <iostream>
#include <fstream>
#include <string>
#include <cstdio>
#include <cstdlib>
#include <algorithm>
#include <cctype>
#include <sstream>  // Добавляем для std::istringstream

//Удаление лишних пробелов между символами
std::string ExtractClass::removeSpacesBetweenChars(const std::string& input)
{
    std::string result;

    for (size_t i = 0; i < input.length(); i++)
    {
        char c = input[i];

        if (c == ' ')
        {
            bool hasCharBefore = (i > 0) && (isalnum(input[i - 1]) || input[i - 1] == '.' || input[i - 1] == ',' || input[i - 1] == '(' || input[i - 1] == ')');
            bool hasCharAfter = (i < input.length() - 1) && (isalnum(input[i + 1]) || input[i + 1] == '.' || input[i + 1] == ',' || input[i + 1] == '(' || input[i + 1] == ')');

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

    // Обрабатываем остальные строки
    while (std::getline(stream, line))
    {
        // Проверяем правила для удаления перевода строки
        bool shouldRemoveNewline = false;

        // Правило 1: удалять отступ если в конце предыдущей строки есть символы "," или "/"
        if (!prevLine.empty())
        {
            char lastChar = prevLine.back();
            if (lastChar == ',' || lastChar == '/' )
            {
                shouldRemoveNewline = true;
            }
        }

        // Правило 2: удалять отступ если текущая строка не начинается с заглавной буквы
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
            // Удаляем перевод строки, добавляем пробел для разделения
            if (!result.empty() && result.back() != ' ')
            {
                result += ' ';
            }
            result += line;
        }
        else
        {
            // Сохраняем перевод строки
            result += '\n' + line;
        }

        prevLine = line;
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
}