#include "Engine/Input/BinaryWriter.hpp"
#include <string.h>

//-----------------------------------------------------------------------------------
IBinaryWriter::EndianMode IBinaryWriter::GetLocalEndianess()
{
	union {
		byte byteData[4];
		uint32_t uiData;
	} data;

	data.uiData = 0x04030201;
	return(data.byteData[0] == 0x01) ? LITTLE_ENDIAN : BIG_ENDIAN;
}

//-----------------------------------------------------------------------------------
//Want to be able to write
//-nullptr
//-empty strings
//-normal strings
bool IBinaryWriter::WriteString(const char* string)
{
	//Write length of the bufer
	if (nullptr == string)
	{
		Write<uint32_t>(0U);
		return true;
	}

	size_t length = strlen(string);
	size_t bufferLength = length + 1;
	return Write<uint32_t>(bufferLength) && (WriteBytes(string, bufferLength) == bufferLength);
}

bool BinaryFileWriter::Open(const char* filename, bool append /*= false*/)
{
	const char* mode;
	if (append)
	{
		mode = "ab"; //Append binary mode
	}
	else
	{
		mode = "wb";
	}

	errno_t error = fopen_s(&fileHandle, filename, mode);
	if (error != 0)
	{
		return false;
	}
	return true;
}

void BinaryFileWriter::Close()
{
	if (fileHandle != nullptr)
	{
		fclose(fileHandle);
		fileHandle = nullptr;
	}
}

size_t BinaryFileWriter::WriteBytes(const void* src, const size_t numBytes)
{
	return fwrite(src, sizeof(byte), numBytes, fileHandle);
}
