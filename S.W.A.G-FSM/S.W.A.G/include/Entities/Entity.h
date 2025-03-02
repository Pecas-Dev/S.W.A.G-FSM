#pragma once

#include <raylib.h>


class World;


class Entity
{
public:
	Entity(World* world, Vector2 position);
	virtual ~Entity() = default;

	virtual void Update(float deltaTime);
	virtual void Draw() = 0;

	// Sense-Decide-Act Pattern
	virtual void Sense(float deltaTime) = 0;
	virtual void Decide(float deltaTime) = 0;
	virtual void Act(float deltaTime) = 0;


	Vector2 GetPosition() const { return position; }


	bool IsAlive() const { return isAlive; }

protected:
	World* world;
	Vector2 position;

	bool isAlive;

	float senseTimer;
	float decideTimer;

	static constexpr float senseInterval = 0.5f;
	static constexpr float decideInterval = 1.0f;
};