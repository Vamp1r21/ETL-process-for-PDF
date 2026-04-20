#pragma once
#include<vector>
#include<string>

class TransformClass
{
	private:
		int FixYear(int year);
		std::string FixDuplicateDelimiters(const std::string& line);
		bool IsNumericOrMeasurement(const std::string& line);
		std::string NormalizeTextField(const std::string& line);
		std::string NormalizeCountryField(const std::string& line);
		int ExtractFirstNumber(const std::string& line);
		//CSV
		std::vector<std::string> ParseCSVLine(const std::string& line);
		std::string Trim(const std::string& line);
		std::string ReplacementDotToComma(const std::string& line);
		//Преобразование даты доставки(выпуска) судна
		std::string MonthToNumber(std::string month);
		std::string NormalizeDate(const std::string& line);
		//Преобразование длины судна
		double TransformationToMeters(double number, const std::string& type);
		std::string NormalizeLength(const std::string& line);
		//Преобразование максимальной скорости судна
		double TransformationToKnots(double number, const std::string& type);
		std::string NormalizeSpeed(const std::string& line);
		//Преобразование вместимости судна
		std::string NormalizeGross(const std::string& line);
		//Преобразование водоизмещения судна
		double TransformationToKg(double number, const std::string& type);
		std::string NormalizeDisplacement(const std::string& line);
		
		std::string ExtractNumber(const std::string& line);
		std::string DetectEngineDesigner(const std::string& model);
		std::string CleanEngineModel(std::string model, const std::string& designer);
		void FixEngineFields(std::string& designer, std::string& model);

	public:
		void TransformCSVFile();
};

