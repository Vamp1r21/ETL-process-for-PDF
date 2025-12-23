#pragma once
#include <fstream>
#include <string>
#include <cstdio>
#include <cstdlib>
#include <algorithm>

class ExtractClass
{
	private:
		bool needsSpaceRemoval(const std::string& content);
		std::string removeSpacesBetweenChars(const std::string& input);
		std::string removeIndents(const std::string& content);
		bool runOCR(const std::string& pdf, const std::string& txt);
		bool isTextMeaningful(const std::string& content);
	public:
		void processFile(const std::string& pdf, const std::string& txt);
};