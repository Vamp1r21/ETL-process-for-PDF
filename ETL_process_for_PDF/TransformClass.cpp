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
    std::string r = line;
    std::replace(r.begin(), r.end(), '.', ',');
    return r;
}

//Ïğåîáğàçîâàíèå äàòû äîñòàâêè(âûïóñêà) ñóäíà
std::string TransformClass::MonthToNumber(std::string month)
{
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

std::string TransformClass::NormalizeDate(const std::string& line)
{
    std::smatch m;

    std::regex r1(R"((\d{4})[-./](\d{2})[-./](\d{2}))");
    if (std::regex_search(line, m, r1))
        return m[3].str() + "." + m[2].str() + "." + m[1].str();

    std::regex r2(R"((\d{2})[-./](\d{2})[-./](\d{4}))");
    if (std::regex_search(line, m, r2))
        return m[1].str() + "." + m[2].str() + "." + m[3].str();

    std::regex r3(R"((\d{1,2})\s+([A-Za-z]+)\s+(\d{4}))");
    if (std::regex_search(line, m, r3)) {
        std::string mm = MonthToNumber(m[2]);
        if (!mm.empty())
            return (m[1].length() == 1 ? "0" : "") + m[1].str() + "." + mm + "." + m[3].str();
    }

    std::regex r4(R"(([A-Za-z]+)\s+(\d{1,2}),?\s+(\d{4}))");
    if (std::regex_search(line, m, r4)) {
        std::string mm = MonthToNumber(m[1]);
        if (!mm.empty())
            return (m[2].length() == 1 ? "0" : "") + m[2].str() + "." + mm + "." + m[3].str();
    }

    return line;
}

//Ïğåîáğàçîâàíèå äëèíû ñóäíà
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

//Ïğåîáğàçîâàíèå ìàêñèìàëüíîé ñêîğîñòè ñóäíà
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

//Ïğåîáğàçîâàíèå âìåñòèìîñòè ñóäíà
std::string TransformClass::NormalizeGross(const std::string& line)
{
    if (Trim(line).empty()) return line;
    std::string result;
    for (char symbol : line) if (isdigit(symbol)) result += symbol;
    return result;
}

//Ïğåîáğàçîâàíèå âîäîèçìåùåíèÿ ñóäíà
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

void TransformClass::TransformCSVFile(std::string path)
{
    std::string filename = path;
    std::ifstream in(filename);

    std::vector<std::vector<std::string>> ñharacteristics;
    std::string line;
    while (getline(in, line)) ñharacteristics.push_back(ParseCSVLine(line));
    in.close();

    std::unordered_map<std::string, int> col;
    for (size_t i = 0; i < ñharacteristics[0].size(); ++i) col[ñharacteristics[0][i]] = i;

    for (size_t r = 1; r < ñharacteristics.size(); ++r) {
        auto& row = ñharacteristics[r];

        auto setNA = [&](std::string n) {
            if (col.count(n) && Trim(row[col[n]]).empty()) row[col[n]] = "N/A";
            };

        setNA("Builder");
        setNA("Designer");
        setNA("OwnerOperator");
        setNA("Country");

        if (col.count("Deliverydate")) row[col["Deliverydate"]] = NormalizeDate(row[col["Deliverydate"]]);
        if (col.count("Length")) row[col["Length"]] = NormalizeLength(row[col["Length"]]);
        if (col.count("MaxSpeed")) row[col["MaxSpeed"]] = NormalizeSpeed(row[col["MaxSpeed"]]);
        if (col.count("Gross")) row[col["Gross"]] = NormalizeGross(row[col["Gross"]]);
        if (col.count("Displacement")) row[col["Displacement"]] = NormalizeDisplacement(row[col["Displacement"]]);
    }

    std::ofstream out(filename);

    //Øàïêà òàáëèöû
    for (size_t i = 0; i < ñharacteristics[0].size(); ++i) {
        out << ñharacteristics[0][i] << (i + 1 < ñharacteristics[0].size() ? "," : "");
    }
    out << "\n";

    //Çàìåíà òî÷åê íà çàïÿòûå
    for (size_t r = 1; r < ñharacteristics.size(); ++r) {
        for (size_t c = 0; c < ñharacteristics[r].size(); ++c) {
            out << "\"" << ReplacementDotToComma(ñharacteristics[r][c]) << "\"" << (c + 1 < ñharacteristics[r].size() ? "," : "");
        }
        out << "\n";
    }

    out.close();
}

void TransformClass::TransformationCSVFiles()
{
    // Ïğîõîä ïî âñåì ôàéëàì â ïàïêå
    for (const auto& entry : std::filesystem::directory_iterator("CSVFiles")) {
        if (entry.is_regular_file()) {
            std::string fileName = entry.path().filename().string();

            std::string filePath = entry.path().string();

            // Ïğîâåğêà ğàñøèğåíèÿ ôàéëà
            if (entry.path().extension() == ".csv") {
                TransformCSVFile(filePath);
            }
        }
    }
}