#pragma once
#include <string>
#include <map>
#include <set>

class ExtractClass
{
	private:
        std::string removeSpacesBetweenChars(const std::string& input);
        std::string removeIndents(const std::string& content);
        bool needsSpaceRemoval(const std::string& content);
        bool isTextMeaningful(const std::string& content);
        bool runOCR(const std::string& pdf, const std::string& txt);
        std::string trim(const std::string& s);
        void convertTextToCSV(const std::string& text, const std::string& csvFile);
	public:
		void processFile(const std::string& pdf);
};