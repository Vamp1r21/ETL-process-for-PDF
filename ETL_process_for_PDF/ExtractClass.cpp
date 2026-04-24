#include <iostream>
#include <fstream>
#include <string>
#include <cstdio>
#include <cstdlib>
#include <algorithm>
#include <cctype>
#include <sstream>
#include <filesystem>
#include <vector>
#include <regex>
#include "ExtractClass.h"


#ifdef _WIN32
#define popen  _popen
#define pclose _pclose
#endif


namespace fs = std::filesystem;

// Удаление лишних пробелов между символами
std::string ExtractClass::RemoveSpacesBetweenChars(const std::string& input)
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

// Удаление отступов
std::string ExtractClass::RemoveIndents(const std::string& content)
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
bool ExtractClass::NeedsSpaceRemoval(const std::string& content)
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


// OCR процесс

// Проверка, является ли текст читаемым
bool ExtractClass::IsTextMeaningful(const std::string& content)
{
	int alphaCount = 0;
	for (char c : content)
	{
		if (isalpha(static_cast<unsigned char>(c)))
			alphaCount++;
	}
	return alphaCount > 20000;
}

bool ExtractClass::RunOCR(const std::string& pdf, const std::string& txt)
{
	const std::string imgPrefix = "ocr_temp_pg";
	const std::string tempDir = "ocr_workdir";

	fs::create_directories(tempDir);

	std::string pdfToImg = "pdftoppm -q -r 200 -gray \"" + pdf + "\" " + tempDir + "/" + imgPrefix;

	if (system(pdfToImg.c_str()) != 0) {
		return false;
	}

	std::vector<std::string> pages;
	for (const auto& entry : fs::directory_iterator(tempDir)) {
		if (entry.path().extension() == ".pgm" || entry.path().extension() == ".ppm") {
			pages.push_back(entry.path().string());
		}
	}

	std::sort(pages.begin(), pages.end());

	std::ofstream out(txt, std::ios::trunc);
	bool processed = false;

	for (const std::string& pageImg : pages) {

		std::string ocrCmd = "tesseract \"" + pageImg + "\" stdout -l eng --psm 3 --oem 3 quiet";

		FILE* pipe = popen(ocrCmd.c_str(), "r");
		if (pipe) {
			char buffer[1024];
			std::string pageContent;
			while (fgets(buffer, sizeof(buffer), pipe) != nullptr) {
				pageContent += buffer;
			}
			pclose(pipe);

			pageContent = std::regex_replace(pageContent, std::regex("[ce!\\.\\-]{4,}"), " ");

			out << pageContent << "\n\f\n";
			processed = true;
		}
		fs::remove(pageImg);
	}

	fs::remove_all(tempDir);
	return processed;
}

//CSV

// Убираем лишние пробелы
std::string ExtractClass::Trim(const std::string& s) {
	size_t start = s.find_first_not_of(" \t.");
	size_t end = s.find_last_not_of(" \t.");
	if (start == std::string::npos || end == std::string::npos)
		return "";
	return s.substr(start, end - start + 1);
}

//Добавление в файл
void ExtractClass::WriteToFile(std::ofstream& out, std::string nameFile, std::map<std::string, std::string> record)
{
	out
		<< "\"" << nameFile << "\","
		<< "\"" << record["VesselName"] << "\","
		<< "\"" << record["Builder"] << "\","
		<< "\"" << record["Designer"] << "\","
		<< "\"" << record["OwnerOperator"] << "\","
		<< "\"" << record["Country"] << "\","
		<< "\"" << record["DeliveryDate"] << "\","
		<< "\"" << record["Length"] << "\","
		<< "\"" << record["MaxSpeed"] << "\","
		<< "\"" << record["IMONumber"] << "\","
		<< "\"" << record["Gross"] << "\","
		<< "\"" << record["SisterShip"] << "\","
		<< "\"" << record["Displacement"] << "\","
		<< "\"" << record["MainEngineDesign"] << "\","
		<< "\"" << record["MainEngineModel"] << "\","
		<< "\"" << record["NumberOfEngines"] << "\"\n";
}


// Обработка очищенного текста
void ExtractClass::ConvertTextToCSV(
	const std::string& text,
	const std::string& csvFile,
	std::string baseName)
{
	std::ofstream out1("CSVFiles/allShip.csv", std::ios::app);
	std::ofstream out(csvFile);
	if (!out.is_open())
	{
		std::cerr << "Не удалось создать CSV: " << csvFile << "\n";
		return;
	}

	out << "FileName,VesselName,Builder,Designer,OwnerOperator,Country,DeliveryDate,Length(m),MaxSpeed(knots),ImoNumber,Gross,SisterShip,Displacement(kg),MainEngineDesigner,MainEngineModel,NumberOfEngines\n";

	std::istringstream stream(text);
	std::string line;
	std::map<std::string, std::string> record;
	int designer = 0;
	int i = 0;
	int mainEngine = 0;
	int vessel = 0;
	int engine = 0;
	int model = 0;
	int number = 0;
	while (std::getline(stream, line))
	{
		if (line.find("Main engine") != std::string::npos || line.find("Mainengine") != std::string::npos)
		{
			mainEngine++;
		}

		if (line.find(":") == std::string::npos &&
			line.find("..") == std::string::npos)
			continue;

		size_t pos = line.find(":");
		if (pos == std::string::npos)
			pos = line.find("..");

		if (pos == std::string::npos)
			pos = line.find(",,,");

		std::string key = Trim(line.substr(0, pos));
		std::string value = Trim(line.substr(pos + 1));

		//Название судна
		if (key.find("Vessel") != std::string::npos && vessel == 0)
		{
			vessel++;
			record["VesselName"] = value;
		}

		//Производитель судна
		else if (key.find("Builder") != std::string::npos ||
			key.find("Shipbuilder") != std::string::npos ||
			key.find("Builder’s name") != std::string::npos)
		{
			// если уже есть данные — сохраняем ПРЕДЫДУЩЕЕ судно
			if (!record.empty() && !record["Builder"].empty())
			{
				// сбрасываем состояния
				vessel = 0;
				mainEngine = 0;
				designer = 0;
				engine = 0;
				model = 0;
				number = 0;
				i = 0;
				if (record["VesselName"].empty())
				{
					record.clear();
					record["Builder"] = value;
					continue;
				}
				WriteToFile(out, baseName, record);
				WriteToFile(out1, baseName, record);
				record.clear();
			}

			// начинаем новое судно
			record["Builder"] = value;

			continue;
			//record["Builder"] = value;
		}

		//Проектировщик судна
		else if ((key.find("Designer") != std::string::npos ||
			key.find("Design") != std::string::npos) && designer <= 0)
		{
			record["Designer"] = value;
			designer++;
		}

		//Владелец судна
		else if (key.find("Owner") != std::string::npos ||
			key.find("Operator") != std::string::npos)
			record["OwnerOperator"] = value;

		//Страна
		else if (key.find("Country") != std::string::npos)
			record["Country"] = value;

		//Дата доставки(выпуска)
		else if (key.find("Delivery date") != std::string::npos ||
			key.find("Deliverydate") != std::string::npos)
			record["DeliveryDate"] = value;

		//Длина судна
		else if (key.find("Length, oa") != std::string::npos ||
			key.find("Length oa") != std::string::npos ||
			key.find("Length,oa") != std::string::npos ||
			key.find("Lengthoa") != std::string::npos)
		{
			// если уже есть данные — сохраняем ПРЕДЫДУЩЕЕ судно
			if (!record.empty() && !record["Length"].empty())
			{
				// сбрасываем состояния
				mainEngine = 0;
				designer = -100;
				i = 0;
				engine = 0;
				model = 0;
				vessel = 0;
				number = 0;
				if (record["VesselName"].empty())
				{
					record.clear();
					record["Length"] = value;
					continue;
				}
				WriteToFile(out, baseName, record);
				WriteToFile(out1, baseName, record);
				record.clear();
			}

			// начинаем новое судно
			record["Length"] = value;

			continue;
		}

		//Максимальная скорость судна
		else if (key.find("Max speed") != std::string::npos ||
			key.find("Maximum speed") != std::string::npos ||
			key.find("Speed service 2") != std::string::npos ||
			key.find("Speed, service") != std::string::npos ||
			key.find("Speed,service") != std::string::npos ||
			key.find("Maxspeed") != std::string::npos ||
			key.find("Maximumspeed") != std::string::npos ||
			key.find("Speedservice 2") != std::string::npos ||
			key.find("Speed (light load)") != std::string::npos)
			record["MaxSpeed"] = value;

		else if (key.find("IMO number") != std::string::npos)
			record["IMONumber"] = value;

		//Вместимость
		else if (key.find("Gross") != std::string::npos)
			record["Gross"] = value;

		//Число кораблей сестер
		else if (key.find("Total number of sister ships already completed") != std::string::npos)
			record["SisterShip"] = value;

		//Водоизмещение судна
		else if (key.find("Displacement") != std::string::npos)
			record["Displacement"] = value;

		//Производитель двигателя судна
		else if ((key.find("Design") != std::string::npos ||
			key.find("Make") != std::string::npos) && mainEngine > 0 && engine == 0)
		{
			engine++;
			record["MainEngineDesign"] = value;
		}

		//Модель двигателя судна
		else if (key.find("Model") != std::string::npos && mainEngine > 0 && model == 0)
		{
			model++;
			record["MainEngineModel"] = value;
		}

		else if ((key.find("Number of engines") != std::string::npos ||
			key.find("Number") != std::string::npos) && mainEngine > 0 && number == 0)
		{
			number++;
			record["NumberOfEngines"] = value;
		}

		/*if (!record["MainEngineModel"].empty() || i > 5)
		{
			if (record["VesselName"].empty()) continue;
			WriteToFile(out, baseName, record);
			WriteToFile(out1, baseName, record);
			mainEngine = 0;
			designer = 0;
			vessel = 0;
			i = 0;
			record.clear();
		}*/
		if (mainEngine > 0)
		{
			i++;
		}
	}
	if (record["VesselName"].empty())
	{
		record.clear();
	}

	if (!record.empty())
	{
		WriteToFile(out, baseName, record);
		WriteToFile(out1, baseName, record);
		record.clear();
	}

	out.close();
	out1.close();
}

// Специальная обработка для дублирующихся букв
// Исправление дублирования букв (не трогает точки)
std::string ExtractClass::FixDuplicateLetters(const std::string& input)
{
	std::string result;
	std::istringstream stream(input);
	std::string line;

	while (std::getline(stream, line)) {
		std::string fixedLine;

		for (size_t i = 0; i < line.length(); i++) {
			// Пропускаем последовательности точек (оставляем как есть)
			if (line[i] == '.') {
				fixedLine += line[i];
				continue;
			}

			// Если текущий символ - буква или цифра
			if (isalnum(static_cast<unsigned char>(line[i]))) {
				// Проверяем паттерн "X x" (буква пробел буква)
				if (i + 2 < line.length() &&
					line[i + 1] == ' ' &&
					line[i + 2] == line[i]) {
					// Добавляем только первую букву, пропускаем пробел и дубликат
					fixedLine += line[i];
					i += 2; // пропускаем пробел и дубликат
					continue;
				}

				// Проверяем паттерн "X X" (буква пробел буква - для разных букв)
				if (i + 2 < line.length() &&
					line[i + 1] == ' ' &&
					isalnum(static_cast<unsigned char>(line[i + 2]))) {
					// Добавляем букву, пробел пропускаем
					fixedLine += line[i];
					i += 1; // пропускаем только пробел, следующую букву обработаем на следующей итерации
					continue;
				}

				// Проверяем дублирование букв подряд (aa, bb, cc и т.д.)
				if (i + 1 < line.length() &&
					line[i + 1] == line[i] &&
					i + 2 < line.length() &&
					line[i + 2] == line[i]) {
					// Три одинаковых подряд - оставляем два
					fixedLine += line[i];
					fixedLine += line[i];
					i += 2;
					continue;
				}
			}

			fixedLine += line[i];
		}

		// Дополнительная очистка: убираем множественные пробелы, но не трогаем точки
		std::string cleanedLine;
		bool lastWasSpace = false;
		for (char c : fixedLine) {
			if (c == ' ') {
				if (!lastWasSpace) {
					cleanedLine += c;
					lastWasSpace = true;
				}
			}
			else {
				cleanedLine += c;
				lastWasSpace = false;
			}
		}

		if (!result.empty()) {
			result += "\n";
		}
		result += cleanedLine;
	}

	return result;
}

bool ExtractClass::HasDuplicateLettersProblem(const std::string& content)
{
	// Ищем слова, где каждая буква разделена пробелом (минимум 4 буквы)
	std::regex pattern(R"(\b([A-Za-z]\s){3,}[A-Za-z]\b)");

	int count = 0;
	auto begin = std::sregex_iterator(content.begin(), content.end(), pattern);
	auto end = std::sregex_iterator();

	for (auto i = begin; i != end; ++i)
	{
		count++;
		if (count >= 2) // минимум 2 таких слова
			return true;
	}

	return false;
}

// Объединение разорванных строк на основе структуры документа
std::string ExtractClass::MergeBrokenFieldLines(const std::string& input)
{
	std::istringstream stream(input);
	std::string line;
	std::string currentField;
	std::string result;

	result.reserve(input.size());

	static const std::vector<std::string> fieldMarkers = {
		"Shipbuilder", "Builder", "Vessel", "Deliverydate",
		"IMO number", "Owner", "Operator", "Designer"
	};

	auto startsWithMarker = [&](const std::string& line) {
		for (const auto& marker : fieldMarkers) {
			if (line.rfind(marker, 0) == 0) // строго с начала строки
				return true;
		}
		return false;
		};

	auto isNewFieldLine = [&](const std::string& line) {
		// Поле вида "Name: value"
		auto colonPos = line.find(':');
		if (colonPos != std::string::npos && colonPos < 25)
			return true;

		// Линии с точками-разделителями
		if (line.find("....") != std::string::npos)
			return true;

		// Явные маркеры
		if (startsWithMarker(line))
			return true;

		return false;
		};

	auto isIndented = [](const std::string& line) {
		return !line.empty() && std::isspace(static_cast<unsigned char>(line[0]));
		};

	auto endsLikeCompleteSentence = [](const std::string& line) {
		if (line.empty()) return true;
		char last = line.back();
		return last == '.' || last == ':' || last == ';';
		};

	auto shouldMerge = [&](const std::string& prev, const std::string& current) {
		// 1. Если строка с отступом — почти точно continuation
		if (isIndented(current))
			return true;

		// 2. Если предыдущая строка НЕ закончена — возможно перенос
		if (!endsLikeCompleteSentence(prev))
			return true;

		return false;
		};

	while (std::getline(stream, line)) {
		if (line.empty()) {
			if (!currentField.empty()) {
				result += currentField + '\n';
				currentField.clear();
			}
			result += '\n';
			continue;
		}

		if (isNewFieldLine(line)) {
			if (!currentField.empty()) {
				result += currentField + '\n';
			}
			currentField = line;
		}
		else {
			if (!currentField.empty() && shouldMerge(currentField, line)) {
				currentField += ' ';
				currentField += line;
			}
			else {
				// ❗ НЕ склеиваем — сохраняем структуру
				if (!currentField.empty()) {
					result += currentField + '\n';
				}
				currentField = line;
			}
		}
	}

	if (!currentField.empty()) {
		if (!result.empty()) result += '\n';
		result += currentField;
	}

	return result;
}

void ExtractClass::ProcessFile(const std::string& pdf)
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
	if (!IsTextMeaningful(content))
	{
		std::cout << "PDF looks like a scan, starting OCR (English)..." << std::endl;
		if (!RunOCR(pdf, txt))
		{
			std::cerr << "Ошибка OCR для файла: " << pdf << "\n";
			return;
		}

		std::ifstream ocrFile(txt);
		content.assign((std::istreambuf_iterator<char>(ocrFile)), std::istreambuf_iterator<char>());
		ocrFile.close();
	}

	if (content.empty())
	{
		std::cerr << "Предупреждение: Текст не обнаружен даже после OCR.\n";
		return;
	}

	//bool shouldRemoveSpaces = NeedsSpaceRemoval(content);
	std::string cleanedContent = RemoveIndents(content);

	if (NeedsSpaceRemoval(cleanedContent))
	{
		cleanedContent = RemoveSpacesBetweenChars(cleanedContent);
	}
	else if (HasDuplicateLettersProblem(cleanedContent))
	{
		cleanedContent = MergeBrokenFieldLines(cleanedContent);
		cleanedContent = FixDuplicateLetters(cleanedContent);
	}

	std::ofstream outFile(txt);
	if (!outFile)
	{
		std::cerr << "Не удалось открыть файл для записи: " << txt << "\n";
		return;
	}
	ConvertTextToCSV(cleanedContent, csv, baseName);
	outFile << cleanedContent;
	outFile.close();

	std::cout << "Успешно обработано: " << pdf << std::endl;
}