#pragma once
#include <vector>
#include <string>

bool LoadBufferFromBinaryFile(std::vector<unsigned char>& out_buffer, const std::string& filePath);
bool SaveBufferToBinaryFile(const std::vector<unsigned char>& buffer, const std::string& filePath);
bool ReadTextFileIntoVector(std::vector<std::string>& outBuffer, const std::string& filePath);
char* FileReadIntoNewBuffer(const std::string& filePath);
std::vector<std::string>* GetFileNamesInFolder(const std::string& filePathSearchString);
bool FileExists(const std::string& filename);