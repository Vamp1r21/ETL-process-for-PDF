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
	public:
		void processFile(const std::string& pdf, const std::string& txt);
};