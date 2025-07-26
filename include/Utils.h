#pragma once

constexpr int NUM_DECIMAL_PLACES_OUTPUT = 5;

std::filesystem::path getRootDirectory();

std::string buildPythonCommand(const std::filesystem::path& scriptPath);

void runGraphPlotter();

std::string prepareForOutput(double number);

void outputRow(std::string key, std::string value, bool rewriteLine);

void outputResults(OptionResult& params);