#include "Engine/Input/InputOutputUtils.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include <windows.h>
#include <strsafe.h>
#include <fstream>

//-----------------------------------------------------------------------------------
bool LoadBufferFromBinaryFile(std::vector<unsigned char>& out_buffer, const std::string& filePath)
{
	FILE* file = nullptr;
	errno_t errorCode = fopen_s(&file, filePath.c_str(), "rb"); //returns a failure state. If failure, return false.
	if(errorCode != 0x0) 
	{ 
		return false; 
	};
	fseek(file, 0, SEEK_END);
	long size = ftell(file);
	rewind(file);
	out_buffer.resize(size);
	fread(&out_buffer[0], sizeof(unsigned char), size, file);
	fclose(file);
	return true;
}

//-----------------------------------------------------------------------------------
bool SaveBufferToBinaryFile(const std::vector<unsigned char>& buffer, const std::string& filePath)
{
	FILE* file = nullptr;
	errno_t errorCode = fopen_s(&file, filePath.c_str(), "wb"); //returns a failure state. If failure, return false.
	if (errorCode != 0x0)
	{
		return false;
	};
	fwrite(&buffer[0], sizeof(unsigned char), buffer.size(), file);
	fclose(file);
	return true;
}

//-----------------------------------------------------------------------------------
bool ReadTextFileIntoVector(std::vector<std::string>& outBuffer, const std::string& filePath)
{
	std::ifstream file(filePath);
	std::string currentLine;
	while (std::getline(file, currentLine))
	{
		outBuffer.push_back(currentLine);
	}
	return true;
}

//-----------------------------------------------------------------------------------
char* FileReadIntoNewBuffer(const std::string& filePath)
{
	FILE* file = nullptr;
	errno_t errorCode = fopen_s(&file, filePath.c_str(), "rb"); //returns a failure state. If failure, return false.
	if (errorCode != 0x0)
	{
		return false;
	};
	fseek(file, 0, SEEK_END);
	long size = ftell(file);
	rewind(file);
	char* buffer = new char[size + 1];
	fread(buffer, sizeof(unsigned char), size, file);
	fclose(file);
	buffer[size] = '\0';
	return buffer;
}

//-----------------------------------------------------------------------------------
std::vector<std::string>* GetFileNamesInFolder(const std::string& filePathSearchString)
{
	WIN32_FIND_DATA finder;
	HANDLE handleToResults = INVALID_HANDLE_VALUE;
	DWORD dwError = 0;
	TCHAR tcharFilePath[MAX_PATH];
	std::vector<std::string>* fileNames = new std::vector<std::string>();
	fileNames->reserve(20);

	//Make our search string windows-friendly.
	std::wstring wideFilePath = std::wstring(filePathSearchString.begin(), filePathSearchString.end());
	LPCWSTR wideFilePathCStr = wideFilePath.c_str();
	StringCchCopy(tcharFilePath, MAX_PATH, wideFilePathCStr);

	//Find the first file in the folder.
	handleToResults = FindFirstFile(tcharFilePath, &finder);

	if (INVALID_HANDLE_VALUE == handleToResults)
	{
		//Empty List
		return fileNames;
	}
	
	do //Add each file name in the folder to the list
	{
		std::wstring wideFileName = std::wstring(&finder.cFileName[0]);
		std::string fileName = std::string(wideFileName.begin(), wideFileName.end());
		fileNames->push_back(fileName);

	} while (FindNextFile(handleToResults, &finder) != 0);

	dwError = GetLastError();
	if (dwError != ERROR_NO_MORE_FILES)
	{
		ERROR_AND_DIE("Error while reading files in folder, code: " + dwError);
	}

	FindClose(handleToResults);
	return fileNames;
}

//-----------------------------------------------------------------------------------
//Modified from http://www.cplusplus.com/forum/general/1796/
bool FileExists(const std::string& filename)
{
	std::ifstream ifile(filename);
	return (bool)ifile;
}