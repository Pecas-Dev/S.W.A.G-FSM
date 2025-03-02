#pragma once

namespace ValueConfig
{
	// Grass Configuration.
	namespace Grass
	{
		static constexpr float TimeToGrow = 9.0f;				  // Time from seeds to fully grown.
		static constexpr float TimeToSpread = 4.0f;				  // Time between spreading seeds.
		static constexpr float LifetimeBeforeWilting = 4.5f;	  // Time before grass starts wilting.
		static constexpr float TimeToWilt = 3.0f;				  // Time it takes to wilt completely.
		static constexpr float SpreadChance = 20.0f;			  // Percentage chance to spread to each neighboring cell.
	}

	// Wolf Configuration.
	namespace Wolf
	{
		static constexpr float MaxHunger = 100.0f;				  // Maximum hunger value.
		static constexpr float HungerIncreaseRate = 275.0f;		  // Rate at which hunger increases.
		static constexpr float WolfHungerThreshold = 25.0f;		  // Hunger level that triggers hunting.
		static constexpr float HuntSpeed = 187.25f;				  // Movement speed while hunting.
		static constexpr float RoamSpeed = 95.0f;				  // Movement speed while roaming.
		static constexpr float AttackDamage = 35.0f;			  // Damage dealt to sheep.
		static constexpr float SheepDetectionRadius = 285.0f;	  // Distance at which wolves are detected.
		static constexpr float StaminaMax = 100.0f;				  // Maximum stamina for sprinting.
		static constexpr float StaminaDrainRate = 15.0f;		  // Rate at which sprinting drains stamina.

		static constexpr float TimeToRestoreHunger = 0.5f;		  // Time it takes to restore hunger while sleeping.
		static constexpr float TimeToEat = 2.0f;				  // Time it takes to eat prey.
		static constexpr float ReturnSpeed = 170.0f;			  // Speed when returning to den.
		static constexpr float AttackInterval = 0.6f;			  // Time between attacks.
		static constexpr float StaminaRecoveryRate = 300.0f;	  // Rate at which stamina recovers.
		static constexpr float TiredSpeedMultiplier = 0.7f;		  // Speed multiplier when tired.
	}

	// Sheep Configuration.
	namespace Sheep
	{
		static constexpr float MaxHealth = 75.0f;				  // Maximum health value.
		static constexpr float MaxHunger = 100.0f;				  // Maximum hunger value.
		static constexpr float MaxFullness = 100.0f;			  // Maximum fullness value.
		static constexpr float HungerIncreaseRate = 250.0f;		  // Rate at which hunger increases.
		static constexpr float HealthDecreaseRate = 300.0f;		  // Rate at which health decreases when hungry.
		static constexpr float WanderSpeed = 70.0f;				  // Movement speed while wandering.
		static constexpr float FleeSpeed = 190.0f;				  // Movement speed while fleeing.
		static constexpr float WolfDetectionRadius = 130.0f;	  // Distance at which wolves are detected.
		static constexpr float GrassDetectionRadius = 65.0f;	  // Distance at which grass is detected.
		static constexpr float GroupRadius = 100.0f;			  // Distance for considering sheep as grouped.
		static constexpr int MaxGroupSize = 5;					  // Maximum number of sheep in a group.


		static constexpr float TimeToEat = 3.5f;				  // Time it takes to eat grass.
		static constexpr float TimeToDefecate = 1.5f;			  // Time spent defecating.
		static constexpr float TimeToReproduce = 3.75f;			  // Time spent reproducing.
		static constexpr float ReproductionInterval = 10.0f;      // Time between reproduction attempts (seconds).
		static constexpr float MinGroupDistance = 40.0f;		  // Minimum distance between sheep in a group.
		static constexpr float MaxGroupDistance = 80.0f;		  // Maximum distance between sheep in a group.
		static constexpr float EdgeAvoidanceMargin = 70.0f;		  // Distance from edge to start avoidance behavior.
		static constexpr float PanicDistance = 50.0f;			  // Distance at which sheep panic and sprint.
		static constexpr float PanicSpeedMultiplier = 1.3f;		  // Speed boost when panicking.
		static constexpr float GroupCooldownAfterFleeing = 10.0f; // Time in seconds before sheep can join groups after fleeing.
		static constexpr float SheepHungerThreshold = 0.45f;	  // Hunger percentage that triggers eating (45%).
	}

	// General World Configuration.
	namespace World
	{
		static constexpr float CellSize = 32.0f;				  // Size of each cell in the grid.
		static constexpr int InitialGrassCount = 15;			  // Starting number of grass.
		static constexpr int InitialSheepCount = 12;			  // Starting number of sheep.
		static constexpr int InitialWolfCount = 2;				  // Starting number of wolves.
	}
}