#pragma once

constexpr int NUM_DECIMAL_PLACES_OUTPUT = 5;

void outputHelp();

bool isPositiveDouble(const char* price);

bool isNonNegativeDouble(const char* price);

bool insensitiveEquals(std::string string1, std::string string2);

bool isValidOptionType(const char* optionType);

std::filesystem::path getRootDirectory();

std::string buildPythonCommand(const std::filesystem::path& scriptPath);

void runGraphPlotter();

std::string prepareForOutput(double number);

void outputRow(std::string key, std::string value);

void outputResults(OptionResult& params);