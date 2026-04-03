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

// ---------------- LENGTH ----------------
double TransformClass::TransformationToMeters(double number, const std::string& type)
{
    if (type == "ft") return number * 0.3048;
    return number;
}

std::string TransformClass::NormalizeLength(const std::string& line)
{
    std::regex r(R"((\d+(\.\d+)?)\s*(m|ft))", std::regex::icase);
    std::smatch m;
    if (std::regex_search(line, m, r)) {
        double number = stod(m[1]);
        std::string type = ToLower(m[3]);
        std::ostringstream o;
        o << std::fixed << std::setprecision(2) << TransformationToMeters(number, type);
        return o.str();
    }
    return ExtractNumber(line);
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
    return ExtractNumber(line);
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

        if (col.count("ImoNumber"))
            row[col["ImoNumber"]] = ExtractNumber(row[col["ImoNumber"]]);

        if (col.count("NumberOfEngines")) {
            std::string v = ExtractNumber(row[col["NumberOfEngines"]]);
            row[col["NumberOfEngines"]] = v.empty() ? "1" : v;
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