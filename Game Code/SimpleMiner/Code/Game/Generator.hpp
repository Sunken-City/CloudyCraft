#pragma once
#include "Game/GameCommon.hpp"

class Block;
class Chunk;

//-----------------------------------------------------------------------------------
class Generator
{
public:
	Generator() {};
	~Generator() {};
	virtual void GenerateChunk(Block* blockArray, Chunk* chunk) = 0;
};

//-----------------------------------------------------------------------------------
class EarthGenerator : public Generator
{
public:
	EarthGenerator() {};
	~EarthGenerator() {};
	virtual void GenerateChunk(Block* blockArray, Chunk* chunk);
};

//-----------------------------------------------------------------------------------
class SkylandsGenerator : public Generator
{
public:
	SkylandsGenerator() {};
	~SkylandsGenerator() {};
	virtual void GenerateChunk(Block* blockArray, Chunk* chunk);
};
