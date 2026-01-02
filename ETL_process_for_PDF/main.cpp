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
    extractClass.processFile("PDFFiles/SSS 1998.pdf",
        "TXTFiles/SSS 1998.txt");

    extractClass.processFile("PDFFiles/SSS 1999.pdf",
        "TXTFiles/SSS 1999.txt");

    extractClass.processFile("PDFFiles/SSS 2000.pdf",
        "TXTFiles/SSS 2000.txt");    
    
    extractClass.processFile("PDFFiles/SSS 2001.pdf",
            "TXTFiles/SSS 2001.txt");

    extractClass.processFile("PDFFiles/SSS 2002.pdf",
        "TXTFiles/SSS 2002.txt");

    extractClass.processFile("PDFFiles/SSS 2003.pdf",
        "TXTFiles/SSS 2003.txt");

    extractClass.processFile("PDFFiles/SSS 2004.pdf",
        "TXTFiles/SSS 2004.txt");

    extractClass.processFile("PDFFiles/SSS 2005.pdf",
        "TXTFiles/SSS 2005.txt");

    extractClass.processFile("PDFFiles/SSS 2006.pdf",
        "TXTFiles/SSS 2006.txt");

    extractClass.processFile("PDFFiles/SSS 2007.pdf",
        "TXTFiles/SSS 2007.txt");

    extractClass.processFile("PDFFiles/SSS 2008.pdf",
        "TXTFiles/SSS 2008.txt");

    extractClass.processFile("PDFFiles/SSS 2009.pdf",
        "TXTFiles/SSS 2009.txt");

    extractClass.processFile("PDFFiles/SSS 2010.pdf",
        "TXTFiles/SSS 2010.txt");

    extractClass.processFile("PDFFiles/SSS 2011.pdf",
        "TXTFiles/SSS 2011.txt");

    extractClass.processFile("PDFFiles/SSS 2012.pdf",
        "TXTFiles/SSS 2012.txt");

    extractClass.processFile("PDFFiles/SSS 2013.pdf",
        "TXTFiles/SSS 2013.txt");

    extractClass.processFile("PDFFiles/SSS 2014.pdf",
        "TXTFiles/SSS 2014.txt");

    extractClass.processFile("PDFFiles/SSS 2015.pdf",
        "TXTFiles/SSS 2015.txt");

    extractClass.processFile("PDFFiles/SSS 2016.pdf",
        "TXTFiles/SSS 2016.txt");

    extractClass.processFile("PDFFiles/SSS 2017.pdf",
        "TXTFiles/SSS 2017.txt");

    extractClass.processFile("PDFFiles/SSS 2018.pdf",
        "TXTFiles/SSS 2018.txt");

    extractClass.processFile("PDFFiles/SSS 2019.pdf",
        "TXTFiles/SSS 2019.txt");

    extractClass.processFile("PDFFiles/SSS 2020.pdf",
        "TXTFiles/SSS 2020.txt");

    extractClass.processFile("PDFFiles/SSS 2022.pdf",
        "TXTFiles/SSS 2022.txt");

    //SS
    //extractClass.processFile("PDFFiles/SS 1998.pdf",
    //    "TXTFiles/SS 1998.txt");

    //extractClass.processFile("PDFFiles/SS 1999.pdf",
    //    "TXTFiles/SS 1999.txt");

    //extractClass.processFile("PDFFiles/SS 2000.pdf",
    //    "TXTFiles/SS 2000.txt");

    //extractClass.processFile("PDFFiles/SS 2001.pdf",
    //    "TXTFiles/SS 2001.txt");

    //extractClass.processFile("PDFFiles/SS 2002.pdf",
    //    "TXTFiles/SS 2002.txt");

    //extractClass.processFile("PDFFiles/SS 2003.pdf",
    //    "TXTFiles/SS 2003.txt");

    //extractClass.processFile("PDFFiles/SS 2004.pdf",
    //    "TXTFiles/SS 2004.txt");

    //extractClass.processFile("PDFFiles/SS 2005.pdf",
    //    "TXTFiles/SS 2005.txt");

    //extractClass.processFile("PDFFiles/SS 2006.pdf",
    //    "TXTFiles/SS 2006.txt");

    extractClass.processFile("PDFFiles/SS 2007.pdf",
        "TXTFiles/SS 2007.txt");

    extractClass.processFile("PDFFiles/SS 2008.pdf",
        "TXTFiles/SS 2008.txt");

    extractClass.processFile("PDFFiles/SS 2009.pdf",
        "TXTFiles/SS 2009.txt");

    extractClass.processFile("PDFFiles/SS 2010.pdf",
        "TXTFiles/SS 2010.txt");

    extractClass.processFile("PDFFiles/SS 2011.pdf",
        "TXTFiles/SS 2011.txt");

    extractClass.processFile("PDFFiles/SS 2012.pdf",
        "TXTFiles/SS 2012.txt");

    extractClass.processFile("PDFFiles/SS 2013.pdf",
        "TXTFiles/SS 2013.txt");

    extractClass.processFile("PDFFiles/SS 2014.pdf",
        "TXTFiles/SS 2014.txt");

    extractClass.processFile("PDFFiles/SS 2015.pdf",
        "TXTFiles/SS 2015.txt");

    extractClass.processFile("PDFFiles/SS 2016.pdf",
        "TXTFiles/SS 2016.txt");

    extractClass.processFile("PDFFiles/SS 2018.pdf",
        "TXTFiles/SS 2018.txt");

    extractClass.processFile("PDFFiles/SS 2019.pdf",
        "TXTFiles/SS 2019.txt");

    extractClass.processFile("PDFFiles/SS 2020.pdf",
        "TXTFiles/SS 2020.txt");

    extractClass.processFile("PDFFiles/SS 2021.pdf",
        "TXTFiles/SS 2021.txt");

    extractClass.processFile("PDFFiles/SS 2022.pdf",
        "TXTFiles/SS 2022.txt");

    std::cout << "Выполнено!";
    return 0;
}