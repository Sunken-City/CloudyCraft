#include "Engine/Core/StringUtils.hpp"
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

//Modified from http://stackoverflow.com/a/325000/2619871
//Returns a new vector with the tokenized string pieces.
//-----------------------------------------------------------------------------------------------
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


