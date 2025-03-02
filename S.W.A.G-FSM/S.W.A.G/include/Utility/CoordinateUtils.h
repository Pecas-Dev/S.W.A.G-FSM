#pragma once

#include <Utility/ValueConfig.h>

#include <raylib.h>


namespace CoordinateUtils
{
	// Converts grid coordinates to world space coordinates
	inline Vector2 GridToWorldPosition(Vector2 gridPos, float scaleFactor = 1.0f)
	{
		return { gridPos.x * ValueConfig::World::CellSize * scaleFactor, gridPos.y * ValueConfig::World::CellSize * scaleFactor };
	}

	// Converts world space coordinates to grid coordinates
	inline Vector2 WorldToGridPosition(Vector2 worldPos, float scaleFactor = 1.0f)
	{
		return { worldPos.x / ValueConfig::World::CellSize * scaleFactor, worldPos.y / ValueConfig::World::CellSize * scaleFactor };
	}
}