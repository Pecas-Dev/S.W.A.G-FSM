#pragma once

#include <Utility/StateMachine.h>
#include <Utility/ValueConfig.h>
#include <Entities/Entity.h>

#include <raylib.h>


class World;


class GrassStateMachine : public Entity, public StateMachine
{
public:
    enum class GrassState
    {
        SeedsPlanted,
        FullyGrown,
        Wilting
    };

    GrassStateMachine(World* world, Vector2 position, float scaleFactor = 1.0f);
    ~GrassStateMachine();

    void Initialize();

    void Sense(float deltaTime) override;
    void Decide(float deltaTime) override;
    void Act(float deltaTime) override;
    void Draw() override;

    void SpreadSeeds();
    void Die();


    GrassState GetCurrentState() const { return currentGrassState; }
    Vector2 GetPosition() const { return position; }

    void SetCurrentState(GrassState state) { currentGrassState = state; }
    void SetBeingEaten(bool value) { isBeingEaten = value; }

    bool IsBeingEaten() const { return isBeingEaten; }
    bool IsAlive() const { return isAlive; }


    static void CleanupTextures();

private:
    static void LoadTextures();
    static void UnloadTextures();


    GrassState currentGrassState;
    World* world;


    Vector2 position;


    static Texture2D seedsPlantedTextures[6]; 
    static Texture2D fullyGrownTexture;       
    static Texture2D wiltingTexture;


    float scaleFactor;

    bool isAlive;
    bool isBeingEaten = false;

    static bool texturesLoaded;


    static constexpr float cellSize = ValueConfig::World::CellSize;
};
