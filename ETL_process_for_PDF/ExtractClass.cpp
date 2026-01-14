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
				input[i + 1] == '.' || input[i + 1] == ',' || input[i + 1] == '(' || input[i + 1] == ')');

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

// Проверка, является ли текст читаемым
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

// OCR процесс
bool ExtractClass::runOCR(const std::string& pdf, const std::string& txt)
{
	const std::string imgPrefix = "ocr_temp_img";

	std::string pdfToImg = "pdftoppm -q -r 300 \"" + pdf + "\" " + imgPrefix;

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
		std::string ocrCmd = "tesseract \"" + pageImg + "\" stdout -l eng --dpi 300 >> \"" + txt + "\"";
		if (system(ocrCmd.c_str()) == 0)
		{
			processed = true;
		}
		fs::remove(pageImg);
	}

	return processed;
}

//To CSV
// 
// Убираем лишние пробелы
std::string ExtractClass::trim(const std::string& s) {
	size_t start = s.find_first_not_of(" \t.");
	size_t end = s.find_last_not_of(" \t.");
	if (start == std::string::npos || end == std::string::npos)
		return "";
	return s.substr(start, end - start + 1);
}

// Обработка очищенного текста
void ExtractClass::convertTextToCSV(
	const std::string& text,
	const std::string& csvFile)
{
	std::ofstream out(csvFile);
	if (!out.is_open())
	{
		std::cerr << "Не удалось создать CSV: " << csvFile << "\n";
		return;
	}

	// Шапка CSV
	out << "VesselName,Builder,Designer,OwnerOperator,Country,DeliveryDate,Length,MaxSpeed\n";

	std::istringstream stream(text);
	std::string line;
	std::map<std::string, std::string> record;
	int i = 0;
	while (std::getline(stream, line))
	{
		if (line.find(":") == std::string::npos &&
			line.find("..") == std::string::npos)
			continue;

		size_t pos = line.find(":");
		if (pos == std::string::npos)
			pos = line.find("..");

		std::string key = trim(line.substr(0, pos));
		std::string value = trim(line.substr(pos + 1));

		if (key.find("Builder") != std::string::npos ||
			key.find("Shipbuilder") != std::string::npos)
			record["Builder"] = value;

		else if (key.find("Designer") != std::string::npos ||
			key.find("Design") != std::string::npos)
			record["Designer"] = value;

		else if (key.find("Vessel") != std::string::npos)
			record["VesselName"] = value;

		else if (key.find("Owner") != std::string::npos ||
			key.find("Operator") != std::string::npos)
			record["OwnerOperator"] = value;

		else if (key.find("Country") != std::string::npos)
			record["Country"] = value;

		else if (key.find("Delivery date") != std::string::npos)
			record["DeliveryDate"] = value;

		else if (key.find("Length, oa") != std::string::npos ||
			key.find("Length oa") != std::string::npos)
			record["Length"] = value;

		else if (key.find("Max speed") != std::string::npos ||
			key.find("Maximum speed") != std::string::npos)
			record["MaxSpeed"] = value;

		if (i == 11) {
			out
				<< "\"" << record["VesselName"] << "\","
				<< "\"" << record["Builder"] << "\","
				<< "\"" << record["Designer"] << "\","
				<< "\"" << record["OwnerOperator"] << "\","
				<< "\"" << record["Country"] << "\","
				<< "\"" << record["DeliveryDate"] << "\","
				<< "\"" << record["Length"] << "\","
				<< "\"" << record["MaxSpeed"] << "\"\n";
			i = 0;
			record.clear();
		}
		if (!record["Length"].empty()) i++;
	}

	out.close();
}
void ExtractClass::processFile(const std::string& pdf)
{
	std::filesystem::path pdfPath(pdf);
	std::string baseName = pdfPath.stem().string();
	const std::string& txt = ("TXTFiles/" + baseName + ".txt");
	const std::string& csv = ("CSVFiles/" + baseName + ".csv");
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
	convertTextToCSV(cleanedContent, csv);
	outFile << cleanedContent;
	outFile.close();

	std::cout << "Успешно обработано: " << pdf << std::endl;
}