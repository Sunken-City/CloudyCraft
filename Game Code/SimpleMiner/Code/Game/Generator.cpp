#include "Game/Generator.hpp"
#include "Game/Chunk.hpp"
#include "Game/World.hpp"
#include "Engine/Math/Noise.hpp"

void EarthGenerator::GenerateChunk(Block* blockArray, Chunk* chunk)
{
	const int MIN_HEIGHT = Chunk::BLOCKS_TALL_Z / 3;
	const int MAX_HEIGHT = (Chunk::BLOCKS_TALL_Z * 3) / 4;
	const int SEA_LEVEL = Chunk::BLOCKS_TALL_Z / 2;
	const float GRID_SIZE = 100.0f;
	const int NUM_OCTAVES = 5;
	const float PERSISTENCE = 0.30f;
	std::map<Vector2Int, float> heights;

	Vector2Int chunkPosInWorld = Vector2Int(chunk->m_chunkPosition.x * Chunk::BLOCKS_WIDE_X, chunk->m_chunkPosition.y * Chunk::BLOCKS_WIDE_X);
	for (int x = chunkPosInWorld.x; x < (chunkPosInWorld.x + Chunk::BLOCKS_WIDE_X); x++)
	{
		for (int y = chunkPosInWorld.y; y < (chunkPosInWorld.y + Chunk::BLOCKS_WIDE_Y); y++)
		{
			Vector2 currentColumn = Vector2(static_cast<float>(x), static_cast<float>(y));
			float delta = Compute2dPerlinNoise(currentColumn.x, currentColumn.y, GRID_SIZE, NUM_OCTAVES, PERSISTENCE);
			heights[Vector2Int(x, y)] = round(MathUtils::RangeMap(delta, -1.0f, 1.0f, static_cast<float>(MIN_HEIGHT), static_cast<float>(MAX_HEIGHT)));
		}
	}

	for (int i = 0; i < Chunk::BLOCKS_PER_CHUNK; i++)
	{
		WorldCoords globalCoords = chunk->GetWorldCoordsForBlockIndex(i);
		Vector2Int currentColumn = Vector2Int(globalCoords.x, globalCoords.y);
		float height = heights[currentColumn];
		float z = static_cast<float>(globalCoords.z);
		if (z > height)
		{
			if (z <= SEA_LEVEL)
				blockArray[i].m_type = BlockType::WATER;
			else
				blockArray[i].m_type = BlockType::AIR;
		}
		else if (z < SEA_LEVEL)
		{
			blockArray[i].m_type = BlockType::STONE;
		}
		else if (z == SEA_LEVEL)
		{
			blockArray[i].m_type = BlockType::SAND;
		}
		else if (z < height)
		{
			blockArray[i].m_type = BlockType::DIRT;
		}
		else if (z == height)
		{
			blockArray[i].m_type = BlockType::GRASS;
		}
	}
}

//-----------------------------------------------------------------------------------
void SkylandsGenerator::GenerateChunk(Block* blockArray, Chunk* chunk)
{
	const float MIN_DENSITY = 20.0f;
	const float MAX_DENSITY = 70.0f;
	const float MIN_THICKNESS_BELOW = 10.0f;
	const float MAX_THICKNESS_BELOW = 30.0f;
	const float MIN_THICKNESS_ABOVE = 0.0f;
	const float MAX_THICKNESS_ABOVE = 8.0f;
	const float VARIABLE_THICKNESS_BELOW = MAX_THICKNESS_BELOW - MIN_THICKNESS_BELOW;
	const float DENSITY_GRID_SIZE = 40.0f;
	const float DENSITY_NUM_OCTAVES = 2;
	const float DENSITY_PERSISTENCE = 0.30f;

	const float ISLAND_SUBLEVELS[] = { 25.0f, 50.0f, 75.0f, 100.0f };
	const int NUM_ISLAND_TIERS = sizeof(ISLAND_SUBLEVELS) / sizeof(ISLAND_SUBLEVELS[0]);

	float islandDensityPerColumnPerTier[Chunk::BLOCKS_PER_LAYER][NUM_ISLAND_TIERS];
	float islandThicknessAbovePerColumnPerTier[Chunk::BLOCKS_PER_LAYER][NUM_ISLAND_TIERS];
	float islandThicknessBelowPerColumnPerTier[Chunk::BLOCKS_PER_LAYER][NUM_ISLAND_TIERS];

	for (int tierIndex = 0; tierIndex < NUM_ISLAND_TIERS; ++tierIndex)
	{
		//Use different seeds for the noise functions for each tier
#pragma todo("Make these functions work off of an actual rng seed")
		unsigned int seed1 = tierIndex;
		unsigned int seed2 = seed1 + NUM_ISLAND_TIERS;
		unsigned int seed3 = seed2 + NUM_ISLAND_TIERS;

		//Determine density and the above & below thicknesses for each column in this chunk
		for (int columnIndex = 0; columnIndex < Chunk::BLOCKS_PER_LAYER; ++columnIndex)
		{
			float thicknessAbove = 0.0f;
			float thicknessBelow = 0.0f;

			//Compute 2D position to be used in various perlin noise functions; Stagger the grid by 50% each tier.
			float tierNoiseGridOffset = 0.5f * DENSITY_GRID_SIZE * (float)tierIndex;
			Vector3 blockPosition = chunk->GetWorldMinsForBlockIndex(columnIndex);
			Vector2 noisePosition = Vector2(blockPosition.x, blockPosition.y) + Vector2(tierNoiseGridOffset, tierNoiseGridOffset);

			//Compute island "density", used as a threshold to decide if an island exists on this tier, and how far inside the island each column is.
			float density = Compute2dPerlinNoise(noisePosition.x, noisePosition.y, DENSITY_GRID_SIZE, 1, 0.5, 2.0f, true, seed1);
			density = MathUtils::RangeMap(density, -1.0f, 1.0f, -MAX_DENSITY, MAX_DENSITY);
			if (density > 0.0f)
			{
				//This column is inside of an island on this tier!
				//Compute density "fraction", 0 at island edges, increasingly positive in island interiors
				float densityFraction = (density - MIN_DENSITY) / (MAX_DENSITY - MIN_DENSITY);
				densityFraction = SmoothStop(densityFraction); //Quickly (and non-linearly) ramp up "density" as we come in from the edge

				//Compute thicknessAbove (terrain variation) and deltaThicknessBelow (underbelly variation)
				thicknessAbove = fabs(Compute2dPerlinNoise(noisePosition.x, noisePosition.y, 70.0f, 6, 0.5f, 2.0f, true, seed2) * 8.0f);
				float deltaThicknessBelow = fabs(Compute2dPerlinNoise(noisePosition.x, noisePosition.y, 20.0f, 4, 0.5f, 2.0f, true, seed3) * VARIABLE_THICKNESS_BELOW);

				//Rapidly feather the terrain above down to base island level as we approach island edge
				float edgeDensityThresholdAbove = 0.05f;
				if (densityFraction < edgeDensityThresholdAbove)
				{
					float fractionInFromEdge = densityFraction / edgeDensityThresholdAbove;
					thicknessAbove *= fractionInFromEdge;
				}

				//Underbelly thickness is proportional to density, with some variation; it feathers up to base island level as we approach island edge
				thicknessBelow = (MIN_THICKNESS_BELOW + deltaThicknessBelow) * densityFraction;

				//Ensure that we're always at least 1 block thick (between terrain and underbelly)
				if (thicknessAbove + thicknessBelow < 1.0f)
				{
					thicknessBelow = 1.0f - thicknessAbove;
				}
			}
			islandDensityPerColumnPerTier[columnIndex][tierIndex] = density;
			islandThicknessAbovePerColumnPerTier[columnIndex][tierIndex] = thicknessAbove;
			islandThicknessBelowPerColumnPerTier[columnIndex][tierIndex] = thicknessBelow;
		}
	}
	
	//Populate the actual blocks based on the above data.
	for (int blockIndex = 0; blockIndex < Chunk::BLOCKS_PER_CHUNK; ++blockIndex)
	{
		WorldPosition blockMins = chunk->GetWorldMinsForBlockIndex(blockIndex);
		BlockType blockType = BlockType::AIR;
		for (int tierIndex = 0; tierIndex < NUM_ISLAND_TIERS; ++tierIndex)
		{
			int columnIndex = blockIndex & 0b11111111;
			float density = islandDensityPerColumnPerTier[columnIndex][tierIndex];
			if (density > MIN_DENSITY)
			{
				float thicknessBelow = islandThicknessBelowPerColumnPerTier[columnIndex][tierIndex];
				float thicknessAbove = islandThicknessAbovePerColumnPerTier[columnIndex][tierIndex];
				float islandBaseZ = ISLAND_SUBLEVELS[tierIndex];
				float islandMinZ = islandBaseZ - thicknessBelow;
				float islandMaxZ = islandBaseZ + thicknessAbove;
				if (blockMins.z >= islandMinZ && blockMins.z <= islandMaxZ)
				{
					if (blockMins.z + 1.0f > islandMaxZ)
					{
						blockType = BlockType::GRASS;
					}
					else
					{
						blockType = BlockType::DIRT;
					}
				}
			}
			blockArray[blockIndex].m_type = static_cast<uchar>(blockType);
		}
	}
}
