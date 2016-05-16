#include "Engine/Core/StringUtils.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include <stdarg.h>

//-----------------------------------------------------------------------------------------------
// Based on code written by Squirrel Eiserloh

//-----------------------------------------------------------------------------------------------
const int STRINGF_STACK_LOCAL_TEMP_LENGTH = 2048;


//-----------------------------------------------------------------------------------------------
const std::string Stringf( const char* format, ... )
{
	char textLiteral[ STRINGF_STACK_LOCAL_TEMP_LENGTH ];
	va_list variableArgumentList;
	va_start( variableArgumentList, format );
	vsnprintf_s( textLiteral, STRINGF_STACK_LOCAL_TEMP_LENGTH, _TRUNCATE, format, variableArgumentList );	
	va_end( variableArgumentList );
	textLiteral[ STRINGF_STACK_LOCAL_TEMP_LENGTH - 1 ] = '\0'; // In case vsnprintf overran (doesn't auto-terminate)

	return std::string( textLiteral );
}


//-----------------------------------------------------------------------------------------------
const std::string Stringf( const int maxLength, const char* format, ... )
{
	char textLiteralSmall[ STRINGF_STACK_LOCAL_TEMP_LENGTH ];
	char* textLiteral = textLiteralSmall;
	if( maxLength > STRINGF_STACK_LOCAL_TEMP_LENGTH )
		textLiteral = new char[ maxLength ];

	va_list variableArgumentList;
	va_start( variableArgumentList, format );
	vsnprintf_s( textLiteral, maxLength, _TRUNCATE, format, variableArgumentList );	
	va_end( variableArgumentList );
	textLiteral[ maxLength - 1 ] = '\0'; // In case vsnprintf overran (doesn't auto-terminate)

	std::string returnValue( textLiteral );
	if( maxLength > STRINGF_STACK_LOCAL_TEMP_LENGTH )
		delete[] textLiteral;

	return returnValue;
}

//-----------------------------------------------------------------------------------------------
//Modified from http://stackoverflow.com/a/325000/2619871
//Returns a new vector with the tokenized string pieces.
std::vector<std::string>* SplitString(const std::string& inputString, const std::string& stringDelimiter)
{
	size_t  start = 0, end = 0;
	std::vector<std::string>* stringPieces = new std::vector<std::string>();

	while (end != std::string::npos)
	{
		end = inputString.find(stringDelimiter, start);

		// If at end, use length = maxLength.  Else use length = end - start.
		stringPieces->push_back(inputString.substr(start,
			(end == std::string::npos) ? std::string::npos : end - start));

		// If at end, use start = maxSize.  Else use start = end + delimiter.
		start = ((end > (std::string::npos - stringDelimiter.size()))
			? std::string::npos : end + stringDelimiter.size());
	}

	return stringPieces;
}

//-----------------------------------------------------------------------------------------------
std::vector<std::string>* SplitStringOnMultipleDelimiters(const std::string& inputString, int numDelimiters, ...)
{
	va_list delimiterList;
	va_start(delimiterList, numDelimiters);
	std::vector<std::string> delimiters;
	for (int i = 0; i < numDelimiters; i++)
	{
		delimiters.push_back(va_arg(delimiterList, const char*));
	}
	va_end(delimiterList);
	std::vector<std::string>* stringPieces = new std::vector<std::string>();

	size_t start = 0, end = 0;

	while (end != std::string::npos)
	{
		size_t shortestEnd = std::string::npos;
		std::string bestDelimiter = "";
		for (const std::string& delim : delimiters)
		{
			end = inputString.find(delim, start);
			if (end < shortestEnd)
			{
				shortestEnd = end;
				bestDelimiter = delim;
			}
		}
		end = shortestEnd;
		stringPieces->push_back(inputString.substr(start, 
			(end == std::string::npos) ? std::string::npos : end - start));

		start = ((end >(std::string::npos - bestDelimiter.size()))
			? std::string::npos : end + bestDelimiter.size());
	}

	return stringPieces;
}

//-----------------------------------------------------------------------------------------------
//Returns a new vector with the tokenized string pieces found in between both delimiters
std::vector<std::string>* ExtractStringsBetween(const std::string& inputString, const std::string& beginStringDelimiter, const std::string& endStringDelimiter)
{
	size_t  start = 0, end = 0;
	std::vector<std::string>* stringPieces = new std::vector<std::string>();
	while (end != std::string::npos)
	{
		start = inputString.find(beginStringDelimiter, start) + beginStringDelimiter.size() - 1;
		if (start == std::string::npos)
		{
			return stringPieces; //We couldn't find the delimiter
		}
		else
		{
			start += 1; //Move up to the first desired character to grab.
		}
		end = inputString.find(endStringDelimiter, start);

		// If at end, use length = maxLength.  Else use length = end - start.
		stringPieces->push_back(inputString.substr(start,
			(end == std::string::npos) ? std::string::npos : end - start));

		// If at end, use start = maxSize.  Else use start = end + delimiter.
		start = ((end > (std::string::npos - endStringDelimiter.size()))
			? std::string::npos : end + endStringDelimiter.size());
	}

	return stringPieces;
}

//-----------------------------------------------------------------------------------------------
RGBA GetColorFromHexString(const std::string& hexString)
{
	std::string workingString = hexString;
	std::string errString = "Invalid hex string, must be in FFFFFF or 0xFFFFFF format";
	size_t hexLength = workingString.size();
	GUARANTEE_OR_DIE(hexLength == 6 || hexLength == 8, errString);

	if (hexLength == 8)
	{
		GUARANTEE_OR_DIE(workingString.substr(0, 2) == "0x", errString);
		workingString = workingString.substr(2);
	}
	try
	{
		unsigned int rgba = std::stoi(workingString, 0, 16);
		rgba <<= 8;
		rgba += 0xFF;
		return RGBA(rgba);
	}
	catch (const std::exception&)
	{
		ERROR_AND_DIE(errString);
	}
}


//-----------------------------------------------------------------------------------------------
void TrimBeginning(std::string& toTrim)
{
	while (!toTrim.empty())
	{
		char firstChar = toTrim[0];
		if (firstChar != ' ')
		{
			return;
		}
		else
		{
			toTrim = toTrim.substr(1);
		}
	}
}


//-----------------------------------------------------------------------------------------------
void TrimEnd(std::string& toTrim)
{
	while (!toTrim.empty())
	{
		char lastChar = toTrim[toTrim.size() - 1];
		if (lastChar != ' ')
		{
			return;
		}
		else
		{
			toTrim = toTrim.substr(0, toTrim.size() - 1);
		}
	}
}


//-----------------------------------------------------------------------------------------------
void Trim(std::string& toTrim)
{
	TrimBeginning(toTrim);
	TrimEnd(toTrim);
}