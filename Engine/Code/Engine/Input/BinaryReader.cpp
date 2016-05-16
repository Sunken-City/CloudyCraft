#include "Engine/Input/BinaryReader.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"

bool BinaryFileReader::Open(const char* filePath)
{
	const char* mode = "rb";

	errno_t error = fopen_s(&fileHandle, filePath, mode);
	if (error != 0)
	{
		return false;
	}
	return true;
}

void BinaryFileReader::Close()
{
	if (fileHandle != nullptr)
	{
		fclose(fileHandle);
		fileHandle = nullptr;
	}
}

void* BinaryFileReader::ReadBytes(const size_t numBytes)
{
	void* buffer = new byte[numBytes];
	fread(buffer, sizeof(byte), numBytes, fileHandle);
	return buffer;
}

IBinaryReader::EndianMode IBinaryReader::GetLocalEndianess()
{
	union {
		byte byteData[4];
		uint32_t uiData;
	} data;

	data.uiData = 0x04030201;
	return(data.byteData[0] == 0x01) ? LITTLE_ENDIAN : BIG_ENDIAN;
}

size_t IBinaryReader::ReadString(const char*& stringBuffer, size_t bufferSize)
{
    UNUSED(bufferSize);
	size_t bufferLength;
	Read<uint32_t>(bufferLength);
	if (bufferLength == 0U)
	{
		stringBuffer = nullptr;
		return bufferLength;
	}
	void* readData = ReadBytes(bufferLength);
	stringBuffer = static_cast<const char*>(readData);
	return bufferLength;
}
