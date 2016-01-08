#pragma once
//-----------------------------------------------------------------------------------------------
// Based on code written by Squirrel Eiserloh
#include <string>
#include <vector>

//-----------------------------------------------------------------------------------------------
const std::string Stringf( const char* format, ... );
const std::string Stringf( const int maxLength, const char* format, ... );
std::vector<std::string>* SplitString(const std::string& inputString, const std::string& stringDelimiter);



