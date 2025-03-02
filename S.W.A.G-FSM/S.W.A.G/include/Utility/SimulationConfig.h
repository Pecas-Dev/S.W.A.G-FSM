#pragma once

#include <Utility/ValueConfig.h>

#include <imgui.h>

#include <string>


class SimulationConfig
{
public:
	enum class EntityTab
	{
		Main,
		Grass,
		Sheep,
		Wolf
	};

	int initialGrassCount = ValueConfig::World::InitialGrassCount;
	int initialSheepCount = ValueConfig::World::InitialSheepCount;
	int initialWolfCount = ValueConfig::World::InitialWolfCount;

	EntityTab currentTab = EntityTab::Main;

	// Grass configuration values.
	struct GrassConfig
	{
		float timeToGrow = ValueConfig::Grass::TimeToGrow;
		float timeToSpread = ValueConfig::Grass::TimeToSpread;
		float lifetimeBeforeWilting = ValueConfig::Grass::LifetimeBeforeWilting;
		float timeToWilt = ValueConfig::Grass::TimeToWilt;
		float spreadChance = ValueConfig::Grass::SpreadChance;
	} grassConfig;


	// Sheep configuration values.
	struct SheepConfig
	{
		float maxHealth = ValueConfig::Sheep::MaxHealth;
		float maxHunger = ValueConfig::Sheep::MaxHunger;
		float maxFullness = ValueConfig::Sheep::MaxFullness;
		float hungerIncreaseRate = ValueConfig::Sheep::HungerIncreaseRate / 10.0f;
		float healthDecreaseRate = ValueConfig::Sheep::HealthDecreaseRate / 10.0f;
		float wanderSpeed = ValueConfig::Sheep::WanderSpeed;
		float fleeSpeed = ValueConfig::Sheep::FleeSpeed;
		float wolfDetectionRadius = ValueConfig::Sheep::WolfDetectionRadius;
		float grassDetectionRadius = ValueConfig::Sheep::GrassDetectionRadius;
		float groupRadius = ValueConfig::Sheep::GroupRadius;
		int maxGroupSize = ValueConfig::Sheep::MaxGroupSize;
	} sheepConfig;


	// Wolf configuration values.
	struct WolfConfig
	{
		float maxHunger = ValueConfig::Wolf::MaxHunger;
		float hungerIncreaseRate = ValueConfig::Wolf::HungerIncreaseRate / 10.0f;
		float hungerThreshold = ValueConfig::Wolf::WolfHungerThreshold;
		float huntSpeed = ValueConfig::Wolf::HuntSpeed;
		float roamSpeed = ValueConfig::Wolf::RoamSpeed;
		float attackDamage = ValueConfig::Wolf::AttackDamage;
		float sheepDetectionRadius = ValueConfig::Wolf::SheepDetectionRadius;
		float staminaMax = ValueConfig::Wolf::StaminaMax;
		float staminaDrainRate = ValueConfig::Wolf::StaminaDrainRate;
	} wolfConfig;
};


namespace RuntimeConfig
{
	extern SimulationConfig Config;

	// Grass.
	inline float GrassTimeToGrow() { return Config.grassConfig.timeToGrow; }
	inline float GrassTimeToSpread() { return Config.grassConfig.timeToSpread; }
	inline float GrassLifetimeBeforeWilting() { return Config.grassConfig.lifetimeBeforeWilting; }
	inline float GrassTimeToWilt() { return Config.grassConfig.timeToWilt; }
	inline float GrassSpreadChance() { return Config.grassConfig.spreadChance; }

	// Sheep.
	inline float SheepMaxHealth() { return Config.sheepConfig.maxHealth; }
	inline float SheepMaxHunger() { return Config.sheepConfig.maxHunger; }
	inline float SheepMaxFullness() { return Config.sheepConfig.maxFullness; }
	inline float SheepHungerIncreaseRate() { return Config.sheepConfig.hungerIncreaseRate * 10.0f; }
	inline float SheepHealthDecreaseRate() { return Config.sheepConfig.healthDecreaseRate * 10.0f; }
	inline float SheepWanderSpeed() { return Config.sheepConfig.wanderSpeed; }
	inline float SheepFleeSpeed() { return Config.sheepConfig.fleeSpeed; }
	inline float SheepWolfDetectionRadius() { return Config.sheepConfig.wolfDetectionRadius; }
	inline float SheepGrassDetectionRadius() { return Config.sheepConfig.grassDetectionRadius; }
	inline float SheepGroupRadius() { return Config.sheepConfig.groupRadius; }
	unsigned inline int SheepMaxGroupSize() { return Config.sheepConfig.maxGroupSize; }

	// Wolf.
	inline float WolfMaxHunger() { return Config.wolfConfig.maxHunger; }
	inline float WolfHungerIncreaseRate() { return Config.wolfConfig.hungerIncreaseRate * 10.0f; }
	inline float WolfHungerThreshold() { return Config.wolfConfig.hungerThreshold; }
	inline float WolfHuntSpeed() { return Config.wolfConfig.huntSpeed; }
	inline float WolfRoamSpeed() { return Config.wolfConfig.roamSpeed; }
	inline float WolfAttackDamage() { return Config.wolfConfig.attackDamage; }
	inline float WolfSheepDetectionRadius() { return Config.wolfConfig.sheepDetectionRadius; }
	inline float WolfStaminaMax() { return Config.wolfConfig.staminaMax; }
	inline float WolfStaminaDrainRate() { return Config.wolfConfig.staminaDrainRate; }

	// World.
	inline int WorldInitialGrassCount() { return Config.initialGrassCount; }
	inline int WorldInitialSheepCount() { return Config.initialSheepCount; }
	inline int WorldInitialWolfCount() { return Config.initialWolfCount; }
}