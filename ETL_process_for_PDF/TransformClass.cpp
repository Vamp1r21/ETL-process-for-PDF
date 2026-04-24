#include "TransformClass.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <unordered_map>
#include <regex>
#include <iomanip>
#include <algorithm>

// ---------------- ENGINE BRANDS ----------------
static std::vector<std::string> ENGINE_BRANDS = {
    "MAN B&W", "MAN", "Wärtsilä", "Wartsila",
    "Sulzer", "WinGD", "Rolls-Royce",
    "MTU", "Caterpillar", "Cummins",
    "Deutz", "Yanmar", "Hyundai"
};

// ---------------- UTILS ----------------
std::string ToLower(const std::string& s)
{
    std::string r = s;
    std::transform(r.begin(), r.end(), r.begin(), ::tolower);
    return r;
}

std::string TransformClass::Trim(const std::string& line)
{
    size_t b = line.find_first_not_of(" \t\r\n");
    size_t e = line.find_last_not_of(" \t\r\n");
    return (b == std::string::npos) ? "" : line.substr(b, e - b + 1);
}

// ---------------- CSV ----------------
std::vector<std::string> TransformClass::ParseCSVLine(const std::string& line)
{
    std::vector<std::string> result;
    std::string field;
    bool inQuotes = false;

    for (char c : line) {
        if (c == '"') inQuotes = !inQuotes;
        else if (c == ',' && !inQuotes) {
            result.push_back(field);
            field.clear();
        }
        else field += c;
    }
    result.push_back(field);
    return result;
}

// ---------------- NUMBER CLEAN ----------------
std::string TransformClass::ExtractNumber(const std::string& line)
{
    std::string result;
    for (char c : line)
        if (isdigit(c) || c == '.') result += c;
    return result;
}

// ---------------- DOT → COMMA ----------------
std::string TransformClass::ReplacementDotToComma(const std::string& line)
{
    if (line.length() == 10 && line[2] == '.' && line[5] == '.')
        return line;

    std::string result = line;
    replace(result.begin(), result.end(), '.', ',');
    return result;
}

//Преобразование даты доставки(выпуска) судна
std::string TransformClass::MonthToNumber(std::string month) {
    std::transform(month.begin(), month.end(), month.begin(), ::tolower);
    static std::unordered_map<std::string, std::string> mm = {
        {"january","01"},{"jan","01"},
        {"february","02"},{"feb","02"},
        {"march","03"},{"mar","03"},
        {"april","04"},{"apr","04"},
        {"may","05"},
        {"june","06"},{"jun","06"},
        {"july","07"},{"jul","07"},
        {"august","08"},{"aug","08"},
        {"september","09"},{"sep","09"},
        {"october","10"},{"oct","10"},
        {"november","11"},{"nov","11"},
        {"december","12"},{"dec","12"}
    };
    return mm.count(month) ? mm[month] : "";
}

// ---------------- FIX YEAR ----------------
int TransformClass::FixYear(int year)
{
    int currentYear = 2026;

    // если год вне диапазона → берём первые 2 цифры
    if (year < 1998 || year > currentYear) {
        std::string y = std::to_string(year);

        if (y.length() >= 2) {
            int firstTwo = std::stoi(y.substr(0, 2));
            return 2000 + firstTwo;
        }
    }

    return year;
}

//Преобразование даты доставки(выпуска) судна
std::string TransformClass::NormalizeDate(const std::string& line) {
    if (line.empty() || line == "N/A") return "";

    std::string cleaned;

    // убрать лишние слова
    cleaned = std::regex_replace(line,
        std::regex(R"(completed|delivered|built)", std::regex::icase), "");

    // исправление "15thJanuary2010"
    cleaned = std::regex_replace(cleaned,
        std::regex(R"((\d{1,2})(st|nd|rd|th))"), "$1");

    // фикс отсутствия пробелов
    cleaned = std::regex_replace(cleaned, std::regex(R"(([A-Za-z]+)(\d{4}))"), "$1 $2");
    cleaned = std::regex_replace(cleaned, std::regex(R"((\d{1,2})([A-Za-z]+))"), "$1 $2");
    cleaned = std::regex_replace(cleaned, std::regex(R"((\d{4})([A-Za-z]+))"), "$2 $1");

    // MayandAugust2009 → May 2009
    cleaned = std::regex_replace(cleaned,
        std::regex(R"(([A-Za-z]+)and([A-Za-z]+)\s*(\d{4}))"),
        "$1 $3");

    std::smatch m;

    // YYYY-MM-DD
    std::regex r1(R"((\d{4})[-./](\d{2})[-./](\d{2}))");
    if (std::regex_search(cleaned, m, r1)) {
        int year = FixYear(stoi(m[1]));
        return m[3].str() + "." + m[2].str() + "." + std::to_string(year);
    }

    // DD.MM.YYYY
    std::regex r2(R"((\d{2})[-./](\d{2})[-./](\d{4}))");
    if (std::regex_search(cleaned, m, r2)) {
        int year = FixYear(stoi(m[3]));
        return m[1].str() + "." + m[2].str() + "." + std::to_string(year);
    }

    // 15 January 2010
    std::regex r3(R"((\d{1,2})\s+([A-Za-z]+)\s+(\d{4}))");
    if (std::regex_search(cleaned, m, r3)) {
        int year = FixYear(stoi(m[3]));
        std::string mm = MonthToNumber(m[2]);

        if (!mm.empty()) {
            std::string dd = (m[1].length() == 1 ? "0" : "") + m[1].str();
            return dd + "." + mm + "." + std::to_string(year);
        }
    }

    // January 15 2010
    std::regex r4(R"(([A-Za-z]+)\s+(\d{1,2})\s+(\d{4}))");
    if (std::regex_search(cleaned, m, r4)) {
        int year = FixYear(stoi(m[3]));
        std::string mm = MonthToNumber(m[1]);

        if (!mm.empty()) {
            std::string dd = (m[2].length() == 1 ? "0" : "") + m[2].str();
            return dd + "." + mm + "." + std::to_string(year);
        }
    }

    // Late2007
    std::regex r_late(R"(late\s*(\d{4}))", std::regex::icase);
    if (std::regex_search(cleaned, m, r_late)) {
        int year = FixYear(stoi(m[1]));
        return "01.10." + std::to_string(year);
    }

    // Month Year
    std::regex r_month_year(R"(([A-Za-z]+)\s+(\d{4}))");
    if (std::regex_search(cleaned, m, r_month_year)) {
        int year = FixYear(stoi(m[2]));
        std::string mm = MonthToNumber(m[1]);

        if (!mm.empty())
            return "01." + mm + "." + std::to_string(year);
    }

    // Year only
    std::regex r_year(R"(^\s*(\d{4})\s*$)");
    if (std::regex_search(cleaned, m, r_year)) {
        int year = FixYear(stoi(m[1]));
        return "01.01." + std::to_string(year);
    }

    return "";
}

// ---------------- LENGTH ----------------
double TransformClass::TransformationToMeters(double number, const std::string& type)
{
    if (type == "ft") return number * 0.3048;
    return number;
}

std::string TransformClass::NormalizeLength(const std::string& line)
{
    std::string cleaned = line;

    // фикс ",229,4"
    cleaned = std::regex_replace(cleaned, std::regex(R"(,(\d+))"), "$1");
    cleaned = std::regex_replace(cleaned, std::regex(R"((\d+),(\d+))"), "$1.$2");

    std::regex r(R"((\d+(\.\d+)?)\s*(m|ft))", std::regex::icase);
    std::smatch m;

    if (std::regex_search(cleaned, m, r)) {
        double number = stod(m[1]);
        std::string type = ToLower(m[3]);

        std::ostringstream o;
        o << std::fixed << std::setprecision(2)
            << TransformationToMeters(number, type);

        return o.str();
    }

    return ExtractNumber(cleaned);
}

// ---------------- SPEED ----------------
double TransformClass::TransformationToKnots(double number, const std::string& type)
{
    if (type == "km/h") return number * 0.539957;
    if (type == "mph") return number * 0.868976;
    return number;
}

std::string TransformClass::NormalizeSpeed(const std::string& line)
{
    std::regex r(R"((\d+(\.\d+)?)\s*(kt|kn|kts|km\/h|mph))", std::regex::icase);
    std::smatch m;
    if (std::regex_search(line, m, r)) {
        double number = stod(m[1]);
        std::string type = ToLower(m[3]);
        std::ostringstream o;
        o << std::fixed << std::setprecision(2) << TransformationToKnots(number, type);
        return o.str();
    }
    return ExtractNumber(line);
}

// ---------------- GROSS ----------------
std::string TransformClass::NormalizeGross(const std::string& line)
{
    std::vector<std::string> nums;
    std::regex r(R"(\d+)");
    auto begin = std::sregex_iterator(line.begin(), line.end(), r);
    auto end = std::sregex_iterator();

    for (auto i = begin; i != end; ++i)
        nums.push_back(i->str());

    if (nums.empty()) return "";

    // если есть несколько чисел → берем среднее
    if (nums.size() >= 3)
        return nums[1];

    return nums[0];
}

// ---------------- DISPLACEMENT ----------------
double TransformClass::TransformationToKg(double number, const std::string& type)
{
    if (type == "kg") return number;
    if (type == "t" || type == "tonne" || type == "tonnes") return number * 1000.0;
    return number;
}

std::string TransformClass::NormalizeDisplacement(const std::string& line)
{
    std::regex r(R"((\d+(\.\d+)?)\s*(kg|t|tonne|tonnes)?)", std::regex::icase);
    std::smatch m;
    if (std::regex_search(line, m, r)) {
        double number = stod(m[1]);
        std::string type = m[3].matched ? ToLower(m[3]) : "t";
        std::ostringstream o;
        o << std::fixed << std::setprecision(0) << TransformationToKg(number, type);
        return o.str();
    }
    return ExtractNumber(line);
}

// ---------------- ENGINE ----------------
std::string TransformClass::DetectEngineDesigner(const std::string& model)
{
    std::string lower = ToLower(model);

    for (const auto& brand : ENGINE_BRANDS) {
        if (lower.find(ToLower(brand)) != std::string::npos)
            return brand;
    }

    return "MAN B&W"; // default
}

std::string TransformClass::CleanEngineModel(std::string model, const std::string& designer)
{
    std::string lowerModel = ToLower(model);
    std::string lowerDesigner = ToLower(designer);

    size_t pos = lowerModel.find(lowerDesigner);
    if (pos != std::string::npos)
        model.erase(pos, designer.length());

    return Trim(model);
}

void TransformClass::FixEngineFields(std::string& designer, std::string& model)
{
    if (Trim(model).empty()) {
        designer = "MAN B&W";
        model = "Generic";
        return;
    }

    if (Trim(designer).empty() || designer == "N/A")
        designer = DetectEngineDesigner(model);

    model = CleanEngineModel(model, designer);

    if (model.empty())
        model = "Generic";
}

std::string TransformClass::FixDuplicateDelimiters(const std::string& line)
{
    std::string result = line;

    result = std::regex_replace(result, std::regex(R"([\.…]{2,})"), " ");
    result = std::regex_replace(result, std::regex(R"([,;:/]{2,})"), ",");
    result = std::regex_replace(result, std::regex(R"(\s+)"), " ");

    return result;
}


// Новая функция для проверки, является ли строка числовым значением или измерением
bool TransformClass::IsNumericOrMeasurement(const std::string& line)
{
    if (line.empty()) return true;

    std::string trimmed = Trim(line);
    std::string lower = ToLower(trimmed);

    // Паттерны, которые нужно заменить на N/A
    std::vector<std::string> patterns = {
        // 4-strokeengine, 4- strokeengine, 4stroke, 4-stroke
        R"(^\s*\d+\s*[-–]?\s*(stroke|cylinder|engine|cyl|strokes?)\s*$)",
        R"(^\s*\d+\s*(stroke|cylinder|engine|cyl|strokes?)\s*$)",

        // 5,5m, 5.5m, 5,5 m, 5.5 m, 5.5meters
        R"(^\s*\d+(?:[.,]\d+)?\s*(?:m|ft|mm|cm|km|meters?|metres?|feet?)\s*$)",

        // 12,5 x 4,3, 12.5 x 4.3, 12,5x4,3
        R"(^\s*\d+(?:[.,]\d+)?\s*[xх]\s*\d+(?:[.,]\d+)?(?:\s*[xх]\s*\d+(?:[.,]\d+)?)?\s*$)",

        // 2364tonnes, 2364 tonnes, 2364 t, 2364t
        R"(^\s*\d+(?:[.,]\d+)?\s*(?:tonnes?|t|kg|mt|ton)\s*$)",

        // 702/dpcc, 702/dpcc/, 702/dpcc something
        R"(^\s*\d+\s*[/]\s*[a-z]+\s*$)",
        R"(^\s*\d+\s*[/]\s*[a-z]+\s*\d*\s*$)",

        // 3, Maj Brodogradiliste - начинается с цифры и запятой или пробела
        R"(^\s*\d+\s*[,;]\s*[A-Za-z])",
        R"(^\s*\d+\s+[A-Za-z].*\d*\s*$)",

        // 20-30, 20 - 30
        R"(^\s*\d+(?:[.,]\d+)?\s*[-–]\s*\d+(?:[.,]\d+)?\s*$)",

        // 1,2,3 или 1;2;3
        R"(^\s*\d+(?:[.,;]\s*\d+)+\s*)",

        // Только цифры (возможно с запятыми/точками)
        R"(^\s*[\d\.,]+\s*$)",

        // Смешанные паттерны типа "12,5 x 4,3 14,5 x 2,0"
        R"(^\s*\d+(?:[.,]\d+)?\s*[xх]\s*\d+(?:[.,]\d+)?(?:\s+\d+(?:[.,]\d+)?\s*[xх]\s*\d+(?:[.,]\d+)?)+\s*$)",

        // Паттерн для "3, Maj Brodogradiliste" - цифра, запятая, пробел, слово
        R"(^\s*\d+\s*,\s+[A-Za-z])",

        // 702/dpcc - цифры, слеш, буквы
        R"(^\s*\d+\s*/\s*[a-z]+\s*\d*\s*$)",
    };

    for (const auto& pattern : patterns) {
        std::regex r(pattern, std::regex::icase);
        if (std::regex_match(trimmed, r)) {
            return true;
        }
    }

    // Дополнительная проверка: если строка начинается с цифры и содержит мало букв
    if (!trimmed.empty() && std::isdigit(trimmed[0])) {
        // Считаем буквы
        int letterCount = 0;
        bool hasDigit = false;
        bool hasSlash = false;

        for (char c : trimmed) {
            if (std::isalpha(c)) letterCount++;
            if (std::isdigit(c)) hasDigit = true;
            if (c == '/') hasSlash = true;
        }

        // Если есть слеш и есть буквы - скорее всего 702/dpcc
        if (hasSlash && letterCount > 0) {
            return true;
        }

        // Если букв меньше 3 и есть цифры - вероятно измерение
        if (letterCount <= 3 && hasDigit) {
            return true;
        }

        // Если строка содержит 'x' или 'х' и цифры - размеры
        if ((trimmed.find('x') != std::string::npos || trimmed.find('х') != std::string::npos) && hasDigit) {
            return true;
        }
    }

    return false;
}

// Очистка текстовых полей (VesselName, Builder, Designer, OwnerOperator)
std::string TransformClass::NormalizeTextField(const std::string& line)
{
    if (line.empty()) return "N/A";

    std::string result = CleanGarbage(line);

    if (result.find_first_not_of("- \t") == std::string::npos)
        return "N/A";

    result = std::regex_replace(result, std::regex(R"(-{2,})"), "-");
    result = std::regex_replace(result, std::regex(R"(\([^)]*\))"), " ");

    result = std::regex_replace(result, std::regex(R"([/\\])"), " ");
    result = std::regex_replace(result, std::regex(R"(,)"), " ");

    result = std::regex_replace(result, std::regex(R"(\s+)"), " ");

    result = Trim(result);

    if (result.empty()) return "N/A";

    return result;
}

// Обработка поля Country - берем первое слово, обозначающее страну
std::string TransformClass::NormalizeCountryField(const std::string& line)
{
    if (line.empty()) return "N/A";

    std::string result = line;

    // Если значение "-" или "--"
    if (result.find_first_not_of("- \t") == std::string::npos) {
        return "N/A";
    }

    // Убираем точки в начале
    result = std::regex_replace(result, std::regex(R"(^\.+)"), "");

    // Убираем многоточия
    result = std::regex_replace(result, std::regex(R"(\.{2,})"), " ");

    // Заменяем "--" на "-"
    result = std::regex_replace(result, std::regex(R"(-{2,})"), "-");

    // Убираем содержимое в скобках
    result = std::regex_replace(result, std::regex(R"(\s*\(\([^)]*\)\)\s*)"), " ");
    result = std::regex_replace(result, std::regex(R"(\s*\([^)]*\)\s*)"), " ");

    // Убираем дублирующиеся разделители
    result = std::regex_replace(result, std::regex(R"(;\s*;)"), ";");
    result = std::regex_replace(result, std::regex(R"(,\s*,)"), ",");

    // Убираем лишние пробелы
    result = std::regex_replace(result, std::regex(R"(\s+)"), " ");
    result = Trim(result);

    // Берем первое слово до пробела, запятой, точки с запятой
    size_t pos = result.find_first_of(" ,;");
    if (pos != std::string::npos) {
        result = result.substr(0, pos);
    }

    // Убираем слово "Flag" если оно есть
    result = std::regex_replace(result, std::regex(R"(\s*Flag\s*)", std::regex::icase), "");

    result = Trim(result);

    return result.empty() ? "N/A" : result;
}

int TransformClass::ExtractFirstNumber(const std::string& line)
{
    std::smatch m;
    std::regex r(R"(\d+)");

    if (std::regex_search(line, m, r))
        return std::stoi(m[0]);

    return -1;
}


std::string TransformClass::CleanGarbage(const std::string& input)
{
    std::string result = input;

    // убираем многоточия и мусор
    result = std::regex_replace(result, std::regex(R"([\.…]{2,})"), " ");

    // убираем повторяющиеся разделители
    result = std::regex_replace(result, std::regex(R"([,;:/&]{2,})"), ",");

    // убираем "висячие" закрывающие скобки
    int open = std::count(result.begin(), result.end(), '(');
    int close = std::count(result.begin(), result.end(), ')');
    if (close > open)
        result = std::regex_replace(result, std::regex(R"(\))"), "");

    // нормализация пробелов
    result = std::regex_replace(result, std::regex(R"(\s+)"), " ");

    return Trim(result);
}

// ---------------- MAIN ----------------
void TransformClass::TransformCSVFile()
{
    std::string filename = "CSVFiles/allShip.csv";
    std::ifstream in(filename);

    std::vector<std::vector<std::string>> data;
    std::string line;

    while (getline(in, line))
        data.push_back(ParseCSVLine(line));

    in.close();

    std::unordered_map<std::string, int> col;
    for (size_t i = 0; i < data[0].size(); ++i)
        col[data[0][i]] = i;

    for (size_t r = 1; r < data.size(); ++r) {

        auto& row = data[r];

        // НОВЫЕ ОБРАБОТКИ для текстовых полей
        if (col.count("VesselName"))
            row[col["VesselName"]] = NormalizeTextField(row[col["VesselName"]]);

        if (col.count("Builder"))
            row[col["Builder"]] = NormalizeTextField(row[col["Builder"]]);

        if (col.count("Designer")) {
            std::string designer = row[col["Designer"]];

            // Сначала проверяем, является ли значение "N/A" или прочерком
            if (designer.empty() || designer == "-" || designer == "--" || designer == "---") {
                row[col["Designer"]] = "N/A";
            }
            // Проверяем, является ли значение числовым/измерением
            else if (IsNumericOrMeasurement(designer)) {
                row[col["Designer"]] = "N/A";
            }
            else {
                std::string cleaned = NormalizeTextField(designer);

                // Дополнительная проверка после очистки
                if (cleaned == "N/A" || IsNumericOrMeasurement(cleaned)) {
                    row[col["Designer"]] = "N/A";
                }
                else {
                    row[col["Designer"]] = cleaned;
                }
            }
        }

        if (col.count("OwnerOperator"))
            row[col["OwnerOperator"]] = NormalizeTextField(row[col["OwnerOperator"]]);

        if (col.count("Country"))
            row[col["Country"]] = NormalizeCountryField(row[col["Country"]]);

        // Остальные обработки остаются без изменений
        if (col.count("DeliveryDate"))
            row[col["DeliveryDate"]] = NormalizeDate(row[col["DeliveryDate"]]);

        if (col.count("Length(m)"))
            row[col["Length(m)"]] = NormalizeLength(row[col["Length(m)"]]);

        if (col.count("MaxSpeed(knots)"))
            row[col["MaxSpeed(knots)"]] = NormalizeSpeed(row[col["MaxSpeed(knots)"]]);

        if (col.count("Gross"))
            row[col["Gross"]] = NormalizeGross(row[col["Gross"]]);

        if (col.count("Displacement(kg)"))
            row[col["Displacement(kg)"]] = NormalizeDisplacement(row[col["Displacement(kg)"]]);

        if (col.count("ImoNumber")) {
            std::smatch m;
            std::regex r(R"(\b\d{7}\b)");

            if (std::regex_search(row[col["ImoNumber"]], m, r))
                row[col["ImoNumber"]] = m[0];
            else
                row[col["ImoNumber"]] = "";
        }

        if (col.count("NumberOfEngines")) {
            int num = ExtractFirstNumber(row[col["NumberOfEngines"]]);

            if (num <= 0 || num > 20)
                row[col["NumberOfEngines"]] = "1";
            else
                row[col["NumberOfEngines"]] = std::to_string(num);
        }

        if (col.count("SisterShip")) {
            std::string v = ExtractNumber(row[col["SisterShip"]]);
            row[col["SisterShip"]] = v.empty() ? "0" : v;
        }

        if (col.count("MainEngineDesigner") && col.count("MainEngineModel")) {
            FixEngineFields(
                row[col["MainEngineDesigner"]],
                row[col["MainEngineModel"]]
            );
        }
    }

    std::ofstream out(filename);

    for (size_t i = 0; i < data[0].size(); ++i)
        out << data[0][i] << (i + 1 < data[0].size() ? "," : "");

    out << "\n";

    for (size_t r = 1; r < data.size(); ++r) {
        for (size_t c = 0; c < data[r].size(); ++c) {
            out << "\"" << ReplacementDotToComma(data[r][c]) << "\""
                << (c + 1 < data[r].size() ? "," : "");
        }
        out << "\n";
    }

    out.close();
}