#pragma once
//-----------------------------------------------------------------------------------------------
// Based on code written by Squirrel Eiserloh
#include <string>
#include <vector>
#include "Engine/Renderer/RGBA.hpp"

//-----------------------------------------------------------------------------------------------
const std::string Stringf( const char* format, ... );
const std::string Stringf( const int maxLength, const char* format, ... );
std::vector<std::string>* SplitString(const std::string& inputString, const std::string& stringDelimiter);
std::vector<std::string>* SplitStringOnMultipleDelimiters(const std::string& inputString, int numDelimiters, ...);
std::vector<std::string>* ExtractStringsBetween(const std::string& inputString, const std::string& beginStringDelimiter, const std::string& endStringDelimiter);
RGBA GetColorFromHexString(const std::string& hexString);
void TrimBeginning(std::string& toTrim);
void TrimEnd(std::string& toTrim);
void Trim(std::string& toTrim);