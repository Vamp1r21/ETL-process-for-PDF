#pragma once
#include <string>
#include <map>
#include <set>
#include <vector>

class ExtractClass
{
	private:
        //PDFToTXT
        std::string RemoveSpacesBetweenChars(const std::string& input);
        std::string RemoveIndents(const std::string& content);
        bool NeedsSpaceRemoval(const std::string& content);
        //OCR
        bool IsTextMeaningful(const std::string& content);
        bool RunOCR(const std::string& pdf, const std::string& txt);
        //CSV
        std::string Trim(const std::string& s);
        void ConvertTextToCSV(const std::string& text, const std::string& csvFile, std::string baseName);
        void WriteToFile(std::ofstream& out, std::string nameFile, std::map<std::string, std::string> record);
	public:
		void ProcessFile(const std::string& pdf);
};