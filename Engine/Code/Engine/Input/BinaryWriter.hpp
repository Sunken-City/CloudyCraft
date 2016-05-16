#pragma once
#include <stdio.h>
#include <stdint.h>

typedef unsigned char byte;

class IBinaryWriter
{
public:
	IBinaryWriter() : m_endianMode(LITTLE_ENDIAN) {};
	//ENUMS//////////////////////////////////////////////////////////////////////////
	enum EndianMode
	{
		LITTLE_ENDIAN,
		BIG_ENDIAN,
		NUM_MODES
	};

	//GETTERS//////////////////////////////////////////////////////////////////////////
	EndianMode GetLocalEndianess();

	//SETTERS//////////////////////////////////////////////////////////////////////////
	inline void SetEndianess(EndianMode mode) { m_endianMode = mode; };

	//FUNCTIONS//////////////////////////////////////////////////////////////////////////
	bool WriteString(const char* string);
	//Returns the number of bytes written. This is the core implementation that subclasses
	//need to support. Writes to the appropriate buffer the bytes.
	virtual size_t WriteBytes(const void* src, const size_t numBytes) = 0;

	//-----------------------------------------------------------------------------------
	template<typename T>
	void ByteSwap(T* source, const size_t numBytes)
	{
		byte* data = (byte*)source;
		byte* start = data;
		byte* end = data + numBytes - 1;

		while (start < end)
		{
			byte temp = *start;
			*start = *end;
			*end = temp;
			++start;
			--end;
		}

		source = (T*)data;
	}

	//-----------------------------------------------------------------------------------
	template<typename T>
	bool Write(const T& data)
	{
		T copy = data;
		if (GetLocalEndianess() != m_endianMode)
		{
			ByteSwap(&copy, sizeof(T));
		}
		return WriteBytes(&copy, sizeof(T)) == sizeof(T);
	}

private:
	EndianMode m_endianMode;
};

class BinaryFileWriter : public IBinaryWriter
{
public:
	//FUNCTIONS//////////////////////////////////////////////////////////////////////////
	bool Open(const char* filename, bool append = false);
	void Close();
	virtual size_t WriteBytes(const void* src, const size_t numBytes) override;

	//MEMBER VARIABLES//////////////////////////////////////////////////////////////////////////
	FILE* fileHandle;
};