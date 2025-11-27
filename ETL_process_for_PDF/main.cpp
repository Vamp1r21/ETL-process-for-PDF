#include <iostream>
#include <fstream>
#include <string>
#include <cstdio>
#include <cstdlib>
#include <algorithm>
#include "ExtractClass.h"

#ifdef _WIN32
#define popen  _popen
#define pclose _pclose
#endif


int main() {
    setlocale(0, "RUS");
    ExtractClass extractClass;
    // Обрабатываем файлы
    extractClass.processFile("../PDFFiles/SSS 1998.pdf",
        "../TXTFiles/SSS 1998.txt");

    extractClass.processFile("../PDFFiles/SSS 2022.pdf",
        "../TXTFiles/SSS 2022.txt");
    std::cout << "Выполнено!";
    return 0;
}