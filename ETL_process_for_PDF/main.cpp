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


int main() 
{
    setlocale(0, "RUS");
    ExtractClass extractClass;
    // Обрабатываем файлы
    //SSS
    extractClass.processFile("PDFFiles/SSS 1998.pdf");

    extractClass.processFile("PDFFiles/SSS 1999.pdf");

    extractClass.processFile("PDFFiles/SSS 2000.pdf");    
    
    extractClass.processFile("PDFFiles/SSS 2001.pdf");

    //extractClass.processFile("PDFFiles/SSS 2002.pdf");

    extractClass.processFile("PDFFiles/SSS 2003.pdf");

    extractClass.processFile("PDFFiles/SSS 2004.pdf");

    extractClass.processFile("PDFFiles/SSS 2005.pdf");

    extractClass.processFile("PDFFiles/SSS 2006.pdf");

    extractClass.processFile("PDFFiles/SSS 2007.pdf");

    extractClass.processFile("PDFFiles/SSS 2008.pdf");

    //extractClass.processFile("PDFFiles/SSS 2009.pdf");

    extractClass.processFile("PDFFiles/SSS 2010.pdf");

    extractClass.processFile("PDFFiles/SSS 2011.pdf");

    extractClass.processFile("PDFFiles/SSS 2012.pdf");

    extractClass.processFile("PDFFiles/SSS 2013.pdf");

    extractClass.processFile("PDFFiles/SSS 2014.pdf");

    extractClass.processFile("PDFFiles/SSS 2015.pdf");

    extractClass.processFile("PDFFiles/SSS 2016.pdf");

    extractClass.processFile("PDFFiles/SSS 2017.pdf");

    extractClass.processFile("PDFFiles/SSS 2018.pdf");

    extractClass.processFile("PDFFiles/SSS 2019.pdf");

    extractClass.processFile("PDFFiles/SSS 2020.pdf");

    extractClass.processFile("PDFFiles/SSS 2022.pdf");

    //SS
    //extractClass.processFile("PDFFiles/SS 1998.pdf");

    //extractClass.processFile("PDFFiles/SS 1999.pdf");

    //extractClass.processFile("PDFFiles/SS 2000.pdf");

    //extractClass.processFile("PDFFiles/SS 2001.pdf");

    //extractClass.processFile("PDFFiles/SS 2002.pdf");

    //extractClass.processFile("PDFFiles/SS 2003.pdf");

    //extractClass.processFile("PDFFiles/SS 2004.pdf");

    //extractClass.processFile("PDFFiles/SS 2005.pdf");

    //extractClass.processFile("PDFFiles/SS 2006.pdf");

    //extractClass.processFile("PDFFiles/SS 2007.pdf");

    extractClass.processFile("PDFFiles/SS 2008.pdf");

    extractClass.processFile("PDFFiles/SS 2009.pdf");

    extractClass.processFile("PDFFiles/SS 2010.pdf");

    extractClass.processFile("PDFFiles/SS 2011.pdf");

    extractClass.processFile("PDFFiles/SS 2012.pdf");

    extractClass.processFile("PDFFiles/SS 2013.pdf");

    extractClass.processFile("PDFFiles/SS 2014.pdf");

    extractClass.processFile("PDFFiles/SS 2015.pdf");

    extractClass.processFile("PDFFiles/SS 2016.pdf");

    extractClass.processFile("PDFFiles/SS 2018.pdf");

    extractClass.processFile("PDFFiles/SS 2019.pdf");

    extractClass.processFile("PDFFiles/SS 2020.pdf");

    extractClass.processFile("PDFFiles/SS 2021.pdf");

    extractClass.processFile("PDFFiles/SS 2022.pdf");

    std::cout << "Выполнено!";
    return 0;
}