#include <iostream>
#include <fstream>
#include <string>
#include <cstdio>
#include <cstdlib>
#include <algorithm>
#include "ExtractClass.h"
#include "TransformClass.h"

#ifdef _WIN32
#define popen  _popen
#define pclose _pclose
#endif


int main()
{
    setlocale(0, "RUS");
    std::ofstream out("CSVFiles/allShip.csv");
    out << "FileName,VesselName,Builder,Designer,OwnerOperator,Country,DeliveryDate,Length(m),MaxSpeed(knots),ImoNumber,Gross,SisterShip,Displacement(kg),MainEngineDesigner,MainEngineModel,NumberOfEngines\n";
    out.close();
    ExtractClass extractClass;
    TransformClass transformClass;
    // Обрабатываем файлы
    //SSS
    //extractClass.ProcessFile("PDFFiles/SSS 1998.pdf");

    //extractClass.ProcessFile("PDFFiles/SSS 1999.pdf");

    //extractClass.ProcessFile("PDFFiles/SSS 2000.pdf");    

    //extractClass.ProcessFile("PDFFiles/SSS 2001.pdf");

    //extractClass.ProcessFile("PDFFiles/SSS 2002.pdf");

    //extractClass.ProcessFile("PDFFiles/SSS 2003.pdf");

    //extractClass.ProcessFile("PDFFiles/SSS 2004.pdf");

    //extractClass.ProcessFile("PDFFiles/SSS 2005.pdf");

    extractClass.ProcessFile("PDFFiles/SSS 2006.pdf");

    extractClass.ProcessFile("PDFFiles/SSS 2007.pdf");

    extractClass.ProcessFile("PDFFiles/SSS 2008.pdf");

    extractClass.ProcessFile("PDFFiles/SSS 2009.pdf");

    extractClass.ProcessFile("PDFFiles/SSS 2010.pdf");

    extractClass.ProcessFile("PDFFiles/SSS 2011.pdf");

    extractClass.ProcessFile("PDFFiles/SSS 2012.pdf");

    extractClass.ProcessFile("PDFFiles/SSS 2013.pdf");

    extractClass.ProcessFile("PDFFiles/SSS 2014.pdf");

    extractClass.ProcessFile("PDFFiles/SSS 2015.pdf");

    extractClass.ProcessFile("PDFFiles/SSS 2016.pdf");

    extractClass.ProcessFile("PDFFiles/SSS 2017.pdf");

    extractClass.ProcessFile("PDFFiles/SSS 2018.pdf");

    extractClass.ProcessFile("PDFFiles/SSS 2019.pdf");

    extractClass.ProcessFile("PDFFiles/SSS 2020.pdf");

    extractClass.ProcessFile("PDFFiles/SSS 2022.pdf");

    //SS
    //extractClass.ProcessFile("PDFFiles/SS 1998.pdf");

    //extractClass.ProcessFile("PDFFiles/SS 1999.pdf");

    //extractClass.ProcessFile("PDFFiles/SS 2000.pdf");

    //extractClass.ProcessFile("PDFFiles/SS 2001.pdf");

    //extractClass.ProcessFile("PDFFiles/SS 2002.pdf");

    //extractClass.ProcessFile("PDFFiles/SS 2003.pdf");

    //extractClass.ProcessFile("PDFFiles/SS 2004.pdf");

    //extractClass.ProcessFile("PDFFiles/SS 2005.pdf");

    extractClass.ProcessFile("PDFFiles/SS 2006.pdf");

    //extractClass.ProcessFile("PDFFiles/SS 2007.pdf");

    extractClass.ProcessFile("PDFFiles/SS 2008.pdf");

    extractClass.ProcessFile("PDFFiles/SS 2009.pdf");

    extractClass.ProcessFile("PDFFiles/SS 2010.pdf");

    extractClass.ProcessFile("PDFFiles/SS 2012.pdf");

    extractClass.ProcessFile("PDFFiles/SS 2013.pdf");

    extractClass.ProcessFile("PDFFiles/SS 2014.pdf");

    extractClass.ProcessFile("PDFFiles/SS 2015.pdf");

    extractClass.ProcessFile("PDFFiles/SS 2016.pdf");

    extractClass.ProcessFile("PDFFiles/SS 2018.pdf");

    extractClass.ProcessFile("PDFFiles/SS 2019.pdf");

    extractClass.ProcessFile("PDFFiles/SS 2020.pdf");

    extractClass.ProcessFile("PDFFiles/SS 2021.pdf");

    extractClass.ProcessFile("PDFFiles/SS 2022.pdf");

    std::cout << "Извлечение выполнено!" << "\n";

    transformClass.TransformCSVFile();

    std::cout << "Преобразование CSV файлов выполнено";
    return 0;
}