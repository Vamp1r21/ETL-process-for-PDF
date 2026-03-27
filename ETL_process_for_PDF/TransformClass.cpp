#include "TransformClass.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <unordered_map>
#include <regex>
#include <iomanip>
#include <algorithm>
#include <filesystem>

//CSV
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

std::string TransformClass::Trim(const std::string& line)
{
    size_t b = line.find_first_not_of(" \t\r\n");
    size_t e = line.find_last_not_of(" \t\r\n");
    return (b == std::string::npos) ? "" : line.substr(b, e - b + 1);
}

std::string TransformClass::ReplacementDotToComma(const std::string& line)
{
    // Проверяем, является ли значение датой (формат DD.MM.YYYY)
    // Простая проверка по формату
    if (line.length() == 10 && line[2] == '.' && line[5] == '.') {
        return line; // Возвращаем дату без изменений
    }

    // Для чисел: меняем точку на запятую
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

std::string TransformClass::NormalizeDate(const std::string& line) {
    if (line.empty() || line == "N/A") return "";

    std::smatch m;

    // 1. Полная дата: YYYY-MM-DD или DD.MM.YYYY
    std::regex r1(R"((\d{4})[-./](\d{2})[-./](\d{2}))");
    if (std::regex_search(line, m, r1))
        return m[3].str() + "." + m[2].str() + "." + m[1].str();

    std::regex r2(R"((\d{2})[-./](\d{2})[-./](\d{4}))");
    if (std::regex_search(line, m, r2))
        return m[1].str() + "." + m[2].str() + "." + m[3].str();

    // 2. Формат: "25 June, 2009" или "June 25 2009"
    std::regex r3(R"((\d{1,2})\s+([A-Za-z]+),?\s+(\d{4}))");
    std::regex r4(R"(([A-Za-z]+)\s+(\d{1,2}),?\s+(\d{4}))");

    if (std::regex_search(line, m, r3)) {
        std::string mm = MonthToNumber(m[2]);
        if (!mm.empty()) {
            std::string dd = (m[1].length() == 1 ? "0" : "") + m[1].str();
            return dd + "." + mm + "." + m[3].str();
        }
    }
    if (std::regex_search(line, m, r4)) {
        std::string mm = MonthToNumber(m[1]);
        if (!mm.empty()) {
            std::string dd = (m[2].length() == 1 ? "0" : "") + m[2].str();
            return dd + "." + mm + "." + m[3].str();
        }
    }

    // --- ОБРАБОТКА КВАРТАЛОВ И СПЕЦ. СЛУЧАЕВ ---

    // 3. Кварталы текстовые: "3rd quarter 2008"
    std::regex r_q_text(R"((\d)(?:st|nd|rd|th)\s+quarter\s+(\d{4}))", std::regex::icase);
    if (std::regex_search(line, m, r_q_text)) {
        std::string q = m[1].str();
        std::string year = m[2].str();
        if (q == "1") return "01.01." + year;
        if (q == "2") return "01.04." + year;
        if (q == "3") return "01.07." + year;
        if (q == "4") return "01.10." + year;
    }

    // 4. Кварталы краткие: "Q1 2019"
    std::regex r_q_short(R"(Q([1-4])\s+(\d{4}))", std::regex::icase);
    if (std::regex_search(line, m, r_q_short)) {
        std::string q = m[1].str();
        std::string year = m[2].str();
        if (q == "1") return "01.01." + year;
        if (q == "2") return "01.04." + year;
        if (q == "3") return "01.07." + year;
        if (q == "4") return "01.10." + year;
    }

    // 5. Середина года: "Mid-2006"
    std::regex r_mid(R"(Mid-(\d{4}))", std::regex::icase);
    if (std::regex_search(line, m, r_mid)) {
        return "01.07." + m[1].str();
    }

    // 6. Только месяц и год: "November 2006"
    std::regex r_month_year(R"(([A-Za-z]+)\s+(\d{4}))");
    if (std::regex_search(line, m, r_month_year)) {
        std::string mm = MonthToNumber(m[1]);
        if (!mm.empty()) return "01." + mm + "." + m[2].str();
    }

    // 7. Только год: "2006"
    std::regex r_year(R"(^\s*(\d{4})\s*$)");
    if (std::regex_search(line, m, r_year)) {
        return "01.01." + m[1].str();
    }

    return line;
}

//Преобразование длины судна
double TransformClass::TransformationToMeters(double number, const std::string& type)
{
    if (type == "ft") return number * 0.3048;
    else return number;
}

std::string TransformClass::NormalizeLength(const std::string& line)
{
    std::regex r(R"((\d+(\.\d+)?)\s*(m|ft))", std::regex::icase);
    std::smatch m;
    if (std::regex_search(line, m, r)) {
        double number = stod(m[1]);
        std::string type = m[3];
        transform(type.begin(), type.end(), type.begin(), ::tolower);
        std::ostringstream o;
        o << std::fixed << std::setprecision(2) << TransformationToMeters(number, type);
        return o.str();
    }
    else return line;
}

//Преобразование максимальной скорости судна
double TransformClass::TransformationToKnots(double number, const std::string& type)
{
    if (type == "km/h") return number * 0.539957;
    else if (type == "mph") return number * 0.868976;
    else return number;
}

std::string TransformClass::NormalizeSpeed(const std::string& line)
{
    std::regex r(R"((\d+(\.\d+)?)\s*(kn|kts|km\/h|mph))", std::regex::icase);
    std::smatch m;
    if (std::regex_search(line, m, r)) {
        double number = stod(m[1]);
        std::string type = m[3];
        transform(type.begin(), type.end(), type.begin(), ::tolower);
        std::ostringstream o;
        o << std::fixed << std::setprecision(2) << TransformationToKnots(number, type);
        return o.str();
    }
    return line;
}

//Преобразование вместимости судна
std::string TransformClass::NormalizeGross(const std::string& line)
{
    if (Trim(line).empty()) return line;
    std::string result;
    for (char symbol : line) if (isdigit(symbol)) result += symbol;
    return result;
}

//Преобразование водоизмещения судна
double TransformClass::TransformationToKg(double number, const std::string& type)
{
    if (type == "kg") return number;
    else if (type == "t" || type == "tonne" || type == "tonnes") return number * 1000.0;
    else if (type == "long ton" || type == "long tons") return number * 1016.0;
    else if (type == "short ton" || type == "short tons") return number * 907.185;
}

std::string TransformClass::NormalizeDisplacement(const std::string& line)
{
    if (Trim(line).empty()) return line;

    std::regex r(R"((\d+(\.\d+)?)\s*(kg|t|tonne|tonnes|long\s+ton[s]?|short\s+ton[s]?)?)",
        std::regex::icase);
    std::smatch m;
    if (std::regex_search(line, m, r)) {
        double number = stod(m[1]);
        std::string type = m[3].matched ? m[3].str() : "t";
        transform(type.begin(), type.end(), type.begin(), ::tolower);
        std::ostringstream o;
        o << std::fixed << std::setprecision(0) << TransformationToKg(number, type);
        return o.str();
    }
    return line;
}

void TransformClass::TransformCSVFile()
{
    std::string filename = "CSVFiles/allShip.csv";;
    std::ifstream in(filename);

    std::vector<std::vector<std::string>> сharacteristics;
    std::string line;
    while (getline(in, line)) сharacteristics.push_back(ParseCSVLine(line));
    in.close();

    std::unordered_map<std::string, int> col;
    for (size_t i = 0; i < сharacteristics[0].size(); ++i) col[сharacteristics[0][i]] = i;

    for (size_t r = 1; r < сharacteristics.size(); ++r) {


        auto& row = сharacteristics[r];

        if (row[col["VesselName"]] == "")
        {
            continue;
        }

        auto setNA = [&](std::string n) {
            if (col.count(n) && Trim(row[col[n]]).empty()) row[col[n]] = "N/A";
            };

        setNA("Builder");
        setNA("Designer");
        setNA("OwnerOperator");
        setNA("Country");

        if (col.count("DeliveryDate")) row[col["DeliveryDate"]] = NormalizeDate(row[col["DeliveryDate"]]);
        if (col.count("Length(m)")) row[col["Length(m)"]] = NormalizeLength(row[col["Length(m)"]]);
        if (col.count("MaxSpeed(knots)")) row[col["MaxSpeed(knots)"]] = NormalizeSpeed(row[col["MaxSpeed(knots)"]]);
        if (col.count("Gross")) row[col["Gross"]] = NormalizeGross(row[col["Gross"]]);
        if (col.count("Displacement(kg)")) row[col["Displacement(kg)"]] = NormalizeDisplacement(row[col["Displacement(kg)"]]);
    }

    std::ofstream out(filename);

    //Шапка таблицы
    for (size_t i = 0; i < сharacteristics[0].size(); ++i) {
        out << сharacteristics[0][i] << (i + 1 < сharacteristics[0].size() ? "," : "");
    }
    out << "\n";

    //Замена точек на запятые
    for (size_t r = 1; r < сharacteristics.size(); ++r) {
        for (size_t c = 0; c < сharacteristics[r].size(); ++c) {
            out << "\"" << ReplacementDotToComma(сharacteristics[r][c]) << "\"" << (c + 1 < сharacteristics[r].size() ? "," : "");
        }
        out << "\n";
    }

    out.close();
}
