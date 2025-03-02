#include <Entities/Grass/GrassStateMachine.h>
#include <Entities/Grass/States/GrassStates.h>
#include <World/World.h>


Texture2D GrassStateMachine::seedsPlantedTextures[6] = {};
Texture2D GrassStateMachine::fullyGrownTexture = {};
Texture2D GrassStateMachine::wiltingTexture = {};

bool GrassStateMachine::texturesLoaded = false;


// Initializes a new grass instance with its world reference and position.
GrassStateMachine::GrassStateMachine(World* world, Vector2 position, float scaleFactor) : Entity(world, position), StateMachine(), world(world), position(position), isAlive(true), scaleFactor(scaleFactor)
{
	LoadTextures();
}

GrassStateMachine::~GrassStateMachine()
{
}

// Sets up the initial state for the grass and its starting visual state.
void GrassStateMachine::Initialize()
{
	SwitchState(std::make_unique<SeedsPlantedState>(this));
	currentGrassState = GrassState::SeedsPlanted;
}

void GrassStateMachine::Sense(float deltaTime)
{
	(void)deltaTime;
}

void GrassStateMachine::Decide(float deltaTime)
{
	(void)deltaTime;
}

// Executes the current state's behavior each frame.
void GrassStateMachine::Act(float deltaTime)
{
	StateMachine::Update(deltaTime);
}

// Renders the grass on screen with a color based on its current state.
void GrassStateMachine::Draw()
{
    Color stateColor;
	Texture2D textureToUse = {};

    switch (currentGrassState)
    {
    case GrassState::SeedsPlanted:
        stateColor = { 0, 255, 102, 255 };

        if (currentState)
        {
            SeedsPlantedState* seedsState = dynamic_cast<SeedsPlantedState*>(currentState.get());

            if (seedsState)
            {
                float growthProgress = seedsState->GetGrowthTimer() / RuntimeConfig::GrassTimeToGrow();
                int textureIndex = static_cast<int>(growthProgress * 5.99f); 
                textureIndex = std::min(5, std::max(0, textureIndex)); 
                textureToUse = seedsPlantedTextures[textureIndex];
            }
            else
            {
                textureToUse = seedsPlantedTextures[0]; 
            }
        }
        else
        {
            textureToUse = seedsPlantedTextures[0]; 
        }
        break;

    case GrassState::FullyGrown:
        stateColor = { 0, 100, 0, 255 };
        textureToUse = fullyGrownTexture;
        break;

    case GrassState::Wilting:
        stateColor = BROWN;
        textureToUse = wiltingTexture;
        break;

    default:
        stateColor = WHITE;
        textureToUse = seedsPlantedTextures[0]; 
        break;
    }

    float minY = world->GetMinY();
    float scaledCellSize = cellSize * scaleFactor;

    float drawX = position.x * scaledCellSize;
    float drawY = position.y * scaledCellSize + minY;

    if (scaleFactor <= 0.5f && drawY < minY)
    {
        drawY = minY;
    }

    if (textureToUse.id != 0) 
    {
        Rectangle sourceRec = { 0.0f, 0.0f, static_cast<float>(textureToUse.width), static_cast<float>(textureToUse.height) };
        Rectangle destRec = { drawX, drawY, scaledCellSize, scaledCellSize };
        Vector2 origin = { 0.0f, 0.0f };

        DrawTexturePro(textureToUse, sourceRec, destRec, origin, 0.0f, WHITE);
    }

    float squareOffset = 5.0f * scaleFactor;
    float squareScale = 0.5f;
    float squareSize = scaledCellSize / 2 * squareScale;

    Vector2 center = { drawX + scaledCellSize / 2, drawY + scaledCellSize / 2 };
    Vector2 squarePosition = { center.x - squareSize / 2, drawY + scaledCellSize + squareOffset };

    DrawRectangle(static_cast<int>(squarePosition.x),static_cast<int>(squarePosition.y), static_cast<int>(squareSize),static_cast<int>(squareSize),stateColor);
}

// Triggers seed spreading behavior in neighboring cells through the world.
void GrassStateMachine::SpreadSeeds()
{
	world->SpreadSeeds(position);
}

// Marks the grass as dead, triggering its removal from the world.
void GrassStateMachine::Die()
{
	isAlive = false;
}

// Loads all grass textures.
void GrassStateMachine::LoadTextures()
{
	if (!texturesLoaded)
	{
		seedsPlantedTextures[0] = LoadTexture("Assets/Grass/Growing/TestGrass1.png");
		seedsPlantedTextures[1] = LoadTexture("Assets/Grass/Growing/TestGrass2.png");
		seedsPlantedTextures[2] = LoadTexture("Assets/Grass/Growing/TestGrass3.png");
		seedsPlantedTextures[3] = LoadTexture("Assets/Grass/Growing/TestGrass4.png");
		seedsPlantedTextures[4] = LoadTexture("Assets/Grass/Growing/TestGrass5.png");
		seedsPlantedTextures[5] = LoadTexture("Assets/Grass/Growing/TestGrass6.png");

		fullyGrownTexture = LoadTexture("Assets/Grass/FullyGrown/TestGrass7.png");

		wiltingTexture = LoadTexture("Assets/Grass/Wilting/TestGrass8.png");

		texturesLoaded = true;
	}
}

// Unloads all grass textures.
void GrassStateMachine::UnloadTextures()
{
	if (texturesLoaded)
	{
		for (int i = 0; i < 6; i++)
		{
			UnloadTexture(seedsPlantedTextures[i]);
		}

		UnloadTexture(fullyGrownTexture);

		UnloadTexture(wiltingTexture);

		texturesLoaded = false;
	}
}

// Helper Function that Unloads all grass textures from memory.
void GrassStateMachine::CleanupTextures()
{
	UnloadTextures();
}