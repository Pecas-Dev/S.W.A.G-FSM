#include <Utility/SimulationConfig.h>
#include <Simulation/Simulation.h>
#include <Utility/TestingConfig.h>
#include <Utility/ValueConfig.h>
#include <Utility/WindowUtils.h>

#include <rlImGui.h>
#include <raylib.h>
#include <imgui.h>

#include <string>


// Creates a new simulation with default window and cell size settings.
Simulation::Simulation() : screenWidth(1850), screenHeight(900), cellSize(ValueConfig::World::CellSize), initialGrassCount(RuntimeConfig::WorldInitialGrassCount()), initialSheepCount(RuntimeConfig::WorldInitialSheepCount()), initialWolfCount(RuntimeConfig::WorldInitialWolfCount()), currentState(SimulationState::Setup)
{
	_monitorWidth = 1920;
	_monitorHeight = 1080;

	screenScaleFactor = 1.0f;
	currentScaleIndex = 2;

	const float titleBarHeight = 20.0f;
	const float mainAreaWidth = screenWidth * 3.0f / 4.0f;
	const float simulationHeight = screenHeight * 2.0f / 3.0f;

	simulationViewport =
	{
		0.0f,
		titleBarHeight,
		mainAreaWidth,
		simulationHeight - titleBarHeight
	};


	UpdateValidScalingOptions();
}

// Cleans up simulation resources by closing the window.
Simulation::~Simulation()
{
	if (currentState == SimulationState::Running && world)
	{
		GrassStateMachine::CleanupTextures();

		world.reset();
	}

	ShutdownImGui();
	CloseWindow();
}

// Sets up the simulation environment, window, and initial world state.
void Simulation::Initialize()
{
	InitWindow(screenWidth, screenHeight, "Juan F. Gutierrez FSM Assignment");
	SetTargetFPS(60);

	int monitorIdx = GetCurrentMonitor();
	_monitorWidth = GetMonitorWidth(monitorIdx);
	_monitorHeight = GetMonitorHeight(monitorIdx);

	if (_monitorWidth <= 0 || _monitorHeight <= 0)
	{
		printf("WARNING: Failed to detect monitor with GLFW, trying system APIs\n");

		_monitorWidth = GetScreenWidth();
		_monitorHeight = GetScreenHeight();

		if (_monitorWidth <= 0 || _monitorHeight <= 0)
		{
			printf("WARNING: Still failed to detect monitor, using fallback resolution\n");
			_monitorWidth = 1920;
			_monitorHeight = 1080;
		}
	}

	printf("Monitor dimensions detected: %dx%d\n", _monitorWidth, _monitorHeight);

	screenScaleFactor = GetHighestValidScaleFactor();

	printf("Selected scale factor: %.2f\n", screenScaleFactor);

	if (screenScaleFactor == 0.5f)
	{
		currentScaleIndex = 0;
	}
	else if (screenScaleFactor == 0.75f)
	{
		currentScaleIndex = 1;
	}
	else if (screenScaleFactor == 1.0f)
	{
		currentScaleIndex = 2;
	}
	else if (screenScaleFactor == 1.25f)
	{
		currentScaleIndex = 3;
	}
	else if (screenScaleFactor == 1.5f)
	{
		currentScaleIndex = 4;
	}

	CalculateWindowDimensions(screenScaleFactor, screenWidth, screenHeight);

	SetWindowSize(screenWidth, screenHeight);

	const float titleBarHeight = 20.0f * screenScaleFactor;
	const float mainAreaWidth = screenWidth * 3.0f / 4.0f;
	const float simulationHeight = screenHeight * 2.0f / 3.0f;

	simulationViewport =
	{
		0.0f,
		titleBarHeight,
		mainAreaWidth,
		simulationHeight - titleBarHeight
	};

	UpdateValidScalingOptions();

	CenterWindow(screenWidth, screenHeight);

	MakeWindowNonMovableByHandle(GetWindowHandle());

	InitializeImGui();
	ImGui::GetIO().IniFilename = "GrassSheepWolves-FSM/include/vendor/imgui.ini";

#ifdef _WIN32
	FILE* dummyFile;
	freopen_s(&dummyFile, "CONOUT$", "w", stdout);
	setvbuf(stdout, nullptr, _IONBF, 0);
#else
	setvbuf(stdout, nullptr, _IONBF, 0);
#endif
}

// Initializes ImGui context and style settings.
void Simulation::InitializeImGui()
{
	rlImGuiSetup(true);
}

// Shuts down ImGui context and resources.
void Simulation::ShutdownImGui()
{
	rlImGuiShutdown();
}

// Processes world updates each frame using the current frame time.
void Simulation::Update()
{
	if (IsKeyPressed(KEY_R))
	{
		showDetectionRadii = !showDetectionRadii;
	}

	if (currentState == SimulationState::Running && world)
	{
		world->Update(GetFrameTime());
	}
}

// Calculates window dimensions for a given scale factor.
void Simulation::CalculateWindowDimensions(float scaleFactor, int& outWidth, int& outHeight) const
{
	outWidth = static_cast<int>(BaseWindowWidth * scaleFactor);
	outHeight = static_cast<int>(BaseWindowHeight * scaleFactor);
}

// Gets the highest valid scale factor for the current monitor
float Simulation::GetHighestValidScaleFactor() const
{
#ifdef SKIP_SCALING_VALIDATION
	return 1.0f;
#else


	const float scaleValues[5] = { 0.5f, 0.75f, 1.0f, 1.25f, 1.5f };

	printf("Finding highest valid scale factor for monitor: %d × %d\n", _monitorWidth, _monitorHeight);

	if (_monitorWidth >= 1366 && _monitorWidth < 1600 && _monitorHeight >= 768)
	{
		printf("Detected common laptop resolution (%dx%d), allowing 0.75x scale\n", _monitorWidth, _monitorHeight);

		int scaledWidth075, scaledHeight075;
		CalculateWindowDimensions(0.75f, scaledWidth075, scaledHeight075);

		int heightMargin = static_cast<int>(scaledHeight075 * 0.015f);
		heightMargin = std::max(10, heightMargin);

		bool heightFits = (scaledHeight075 + heightMargin <= _monitorHeight);

		if (heightFits)
		{
			printf("Selected scale factor: 0.75 (optimized for this resolution)\n");
			return 0.75f;
		}
	}

	for (int i = 4; i >= 0; i--)
	{
		int scaledWidth, scaledHeight;

		CalculateWindowDimensions(scaleValues[i], scaledWidth, scaledHeight);

		int widthMargin = static_cast<int>(scaledWidth * 0.015f);
		int heightMargin = static_cast<int>(scaledHeight * 0.015f);

		widthMargin = std::max(10, widthMargin);
		heightMargin = std::max(10, heightMargin);

		bool widthFits = (scaledWidth + widthMargin <= _monitorWidth);
		bool heightFits = (scaledHeight + heightMargin <= _monitorHeight);
		bool fits = widthFits && heightFits;

		printf("Testing scale %.2f (%s):\n", scaleValues[i], scaleOptions[i]);
		printf("  Window: %d×%d + margins %d,%d = %d×%d\n", scaledWidth, scaledHeight, widthMargin, heightMargin, scaledWidth + widthMargin, scaledHeight + heightMargin);
		printf("  Monitor: %d×%d\n", _monitorWidth, _monitorHeight);
		printf("  Width fits: %s, Height fits: %s\n", widthFits ? "yes" : "NO", heightFits ? "yes" : "NO");

		if (fits)
		{
			printf("Selected scale factor: %.2f\n", scaleValues[i]);
			return scaleValues[i];
		}
	}

	printf("No scale fits, falling back to 0.5x\n");
	return 0.5f;
#endif
}

// Updates which scaling options are valid.
void Simulation::UpdateValidScalingOptions()
{
#ifdef ENABLE_ALL_SCALING_OPTIONS
	for (int i = 0; i < 5; i++)
	{
		scaleOptionEnabled[i] = true;

#ifdef PRINT_DEBUG_INFO
		printf("[TESTING] Scale %s: Force-enabled for testing\n", scaleOptions[i]);
#endif
	}
#else

	const float scaleValues[5] = { 0.5f, 0.75f, 1.0f, 1.25f, 1.5f };

	bool force075xEnabled = (_monitorWidth >= 1366 && _monitorWidth < 1600 && _monitorHeight >= 768);

	for (int i = 0; i < 5; i++)
	{
		if (i == 1 && force075xEnabled)
		{
			scaleOptionEnabled[i] = true;

			printf("Scale %s: Force-enabled for this monitor resolution\n", scaleOptions[i]);

			continue;
		}

		int scaledWidth, scaledHeight;

		CalculateWindowDimensions(scaleValues[i], scaledWidth, scaledHeight);

		int widthMargin = static_cast<int>(scaledWidth * 0.015f);
		int heightMargin = static_cast<int>(scaledHeight * 0.015f);

		widthMargin = std::max(10, widthMargin);
		heightMargin = std::max(10, heightMargin);

		bool widthFits = (scaledWidth + widthMargin <= _monitorWidth);
		bool heightFits = (scaledHeight + heightMargin <= _monitorHeight);

		scaleOptionEnabled[i] = widthFits && heightFits;

		printf("Scale %s: Width=%d+%d=%d, Height=%d+%d=%d, Fits=%s\n", scaleOptions[i], scaledWidth, widthMargin, scaledWidth + widthMargin, scaledHeight, heightMargin, scaledHeight + heightMargin, scaleOptionEnabled[i] ? "yes" : "no");
	}

	bool anyEnabled = false;

	for (int i = 0; i < 5; i++)
	{
		if (scaleOptionEnabled[i])
		{
			anyEnabled = true;
			break;
		}
	}

	if (!anyEnabled)
	{
		scaleOptionEnabled[0] = true;
	}
#endif
}

// Helper Function to Check if a scale option is valid.
bool Simulation::IsScaleOptionValid(int scaleIndex) const
{
	if (scaleIndex >= 0 && scaleIndex < 5)
	{
		return scaleOptionEnabled[scaleIndex];
	}

	return false;
}

// Renders the current state of the world and debug information.
void Simulation::Draw()
{
	BeginDrawing();
	ClearBackground(RAYWHITE);

	rlImGuiBegin();

	if (currentState == SimulationState::Setup)
	{
		DrawSetupUI();
	}
	else if (currentState == SimulationState::Running && world)
	{
		world->Draw();

		DrawSimulationLayout();
	}

	rlImGuiEnd();
	EndDrawing();
}

// Draws the setup UI for configuring the initial world state.
void Simulation::DrawSetupUI()
{
	float windowWidth = 600.0f;
	float windowHeight = 500.0f;
	float windowPosX = static_cast<float>(screenWidth / 2 - windowWidth / 2);
	float windowPosY = static_cast<float>(screenHeight / 2 - windowHeight / 2);

	if (screenScaleFactor <= 0.5f)
	{
		windowWidth = 480.0f;
		windowHeight = 400.0f;

		windowPosX = static_cast<float>(screenWidth / 2 - windowWidth / 2);
		windowPosY = static_cast<float>(screenHeight / 2 - windowHeight / 2);

		windowPosY = std::max(windowPosY, 30.0f);
	}

	ImGui::SetNextWindowPos(ImVec2(windowPosX, windowPosY), ImGuiCond_Always);
	ImGui::SetNextWindowSize(ImVec2(windowWidth, windowHeight), ImGuiCond_Always);

	ImGui::Begin("JFG Simulation Setup", nullptr, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize);

	ImGui::PushFont(ImGui::GetIO().Fonts->Fonts[0]);
	ImGui::SetCursorPosX((ImGui::GetWindowWidth() - ImGui::CalcTextSize("SIMULATION").x) * 0.5f);
	ImGui::Text("SIMULATION");
	ImGui::PopFont();
	ImGui::Spacing();
	ImGui::Separator();
	ImGui::Spacing();

	ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 8.0f);
	ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(8, 8));

	const float buttonWidth = (ImGui::GetWindowWidth() - 60) / 3.0f;

	ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.0f, 0.7f, 0.0f, 0.8f));
	ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.0f, 0.9f, 0.0f, 0.9f));
	ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.0f, 0.5f, 0.0f, 1.0f));

	if (ImGui::Button("Grass Settings", ImVec2(buttonWidth, 40)))
	{
		RuntimeConfig::Config.currentTab = SimulationConfig::EntityTab::Grass;
	}
	ImGui::PopStyleColor(3);

	ImGui::SameLine();


	ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.0f, 0.4f, 0.8f, 0.8f));
	ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.2f, 0.5f, 0.9f, 0.9f));
	ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.0f, 0.3f, 0.7f, 1.0f));

	if (ImGui::Button("Sheep Settings", ImVec2(buttonWidth, 40)))
	{
		RuntimeConfig::Config.currentTab = SimulationConfig::EntityTab::Sheep;
	}
	ImGui::PopStyleColor(3);

	ImGui::SameLine();


	ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.8f, 0.0f, 0.0f, 0.8f));
	ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.9f, 0.2f, 0.2f, 0.9f));
	ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.7f, 0.0f, 0.0f, 1.0f));

	if (ImGui::Button("Wolf Settings", ImVec2(buttonWidth, 40)))
	{
		RuntimeConfig::Config.currentTab = SimulationConfig::EntityTab::Wolf;
	}
	ImGui::PopStyleColor(3);

	ImGui::PopStyleVar(2);

	ImGui::Spacing();
	ImGui::Separator();
	ImGui::Spacing();

	if (RuntimeConfig::Config.currentTab == SimulationConfig::EntityTab::Main)
	{
		ImGui::Spacing();
		ImGui::Spacing();
		ImGui::Text("Configure your ecosystem parameters:");
		ImGui::Spacing();

		ImGui::SliderInt("Initial Grass Count", &RuntimeConfig::Config.initialGrassCount, 0, 100, "%d");
		ImGui::SliderInt("Initial Sheep Count", &RuntimeConfig::Config.initialSheepCount, 1, 30, "%d");
		ImGui::SliderInt("Initial Wolf Count", &RuntimeConfig::Config.initialWolfCount, 0, 10, "%d");


		for (int i = 0; i < 7; i++)
		{
			ImGui::Spacing();
		}

		ImGui::TextWrapped("Note: Configure individual entity settings using the buttons above! (;");
	}
	else if (RuntimeConfig::Config.currentTab == SimulationConfig::EntityTab::Grass)
	{
		RenderGrassSettings();
	}
	else if (RuntimeConfig::Config.currentTab == SimulationConfig::EntityTab::Sheep)
	{
		RenderSheepSettings();
	}
	else if (RuntimeConfig::Config.currentTab == SimulationConfig::EntityTab::Wolf)
	{
		RenderWolfSettings();
	}

	for (int i = 0; i < 3; i++)
	{
		ImGui::Spacing();
	}

	ImGui::Separator();

	for (int i = 0; i < 3; i++)
	{
		ImGui::Spacing();
	}

	if (RuntimeConfig::Config.currentTab != SimulationConfig::EntityTab::Main)
	{
		if (ImGui::Button("Back to Main Settings", ImVec2(200, 30)))
		{
			RuntimeConfig::Config.currentTab = SimulationConfig::EntityTab::Main;
		}

		ImGui::SameLine();
	}

	ImGui::SetCursorPosX(ImGui::GetWindowWidth() - 220);
	ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.2f, 0.6f, 0.2f, 1.0f));
	ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.3f, 0.7f, 0.3f, 1.0f));
	ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.1f, 0.5f, 0.1f, 1.0f));



	for (int i = 0; i < 3; i++)
	{
		ImGui::Spacing();
	}
	ImGui::Separator();
	for (int i = 0; i < 5; i++)
	{
		ImGui::Spacing();
	}

	ImGui::Text("Display Settings:");
	ImGui::Spacing();

	if (ImGui::BeginCombo("Screen Size", scaleOptions[currentScaleIndex]))
	{
		for (int i = 0; i < 5; i++)
		{
			bool optionDisabled = !scaleOptionEnabled[i];

			if (optionDisabled)
			{
				ImGui::BeginDisabled();
			}

			bool isSelected = (currentScaleIndex == i);

			if (ImGui::Selectable(scaleOptions[i], isSelected))
			{
				currentScaleIndex = i;

				switch (i)
				{
				case 0: screenScaleFactor = 0.5f; break;
				case 1: screenScaleFactor = 0.75f; break;
				case 2: screenScaleFactor = 1.0f; break;
				case 3: screenScaleFactor = 1.25f; break;
				case 4: screenScaleFactor = 1.5f; break;
				}

				CalculateWindowDimensions(screenScaleFactor, screenWidth, screenHeight);

				pendingResize = true;
			}

			if (isSelected)
			{
				ImGui::SetItemDefaultFocus();
			}

			if (optionDisabled)
			{
				ImGui::EndDisabled();
			}
		}

		ImGui::EndCombo();
	}

	ImGui::SameLine();
	ImGui::TextDisabled("(?)");
	if (ImGui::IsItemHovered())
	{
		ImGui::BeginTooltip();
		ImGui::Text("Select window size based on your screen resolution.");
		ImGui::Text("Current monitor: %dx%d", _monitorWidth, _monitorHeight);
		ImGui::Text("Window dimensions:");
		ImGui::Text("0.5x = %dx%d", static_cast<int>(BaseWindowWidth * 0.5f), static_cast<int>(BaseWindowHeight * 0.5f));
		ImGui::Text("0.75x = %dx%d", static_cast<int>(BaseWindowWidth * 0.75f), static_cast<int>(BaseWindowHeight * 0.75f));
		ImGui::Text("1.0x = %dx%d", BaseWindowWidth, BaseWindowHeight);
		ImGui::Text("1.25x = %dx%d", static_cast<int>(BaseWindowWidth * 1.25f), static_cast<int>(BaseWindowHeight * 1.25f));
		ImGui::Text("1.5x = %dx%d", static_cast<int>(BaseWindowWidth * 1.5f), static_cast<int>(BaseWindowHeight * 1.5f));

		bool hasDisabledOptions = false;

		for (int i = 0; i < 5; i++)
		{
			if (!scaleOptionEnabled[i])
			{
				hasDisabledOptions = true;
				break;
			}
		}

		if (hasDisabledOptions)
		{
			ImGui::Separator();
			ImGui::Text("Grayed out options exceed your screen resolution.");
		}

		ImGui::EndTooltip();
	}

	ImGui::Spacing();

	if (pendingResize)
	{
		int newWidth = static_cast<int>(1850.0f * screenScaleFactor);
		int newHeight = static_cast<int>(900.0f * screenScaleFactor);

		SetWindowSize(newWidth, newHeight);

		CenterWindow(newWidth, newHeight);

		screenWidth = newWidth;
		screenHeight = newHeight;

		const float titleBarHeight = 20.0f * screenScaleFactor;
		const float mainAreaWidth = newWidth * 3.0f / 4.0f;
		const float simulationHeight = newHeight * 2.0f / 3.0f;

		simulationViewport = { 0.0f,titleBarHeight,mainAreaWidth,simulationHeight - titleBarHeight };

		pendingResize = false;
	}


	if (screenScaleFactor <= 0.5f)
	{
		for (int i = 0; i < 6; i++)
		{
			ImGui::Spacing();
		}

		ImGui::Separator();

		float buttonX = (ImGui::GetWindowWidth() - 200.0f) / 2.0f;

		ImGui::SetCursorPosX(buttonX);

		if (ImGui::Button("Start Simulation", ImVec2(200, 35)))
		{
			initialGrassCount = RuntimeConfig::Config.initialGrassCount;
			initialSheepCount = RuntimeConfig::Config.initialSheepCount;
			initialWolfCount = RuntimeConfig::Config.initialWolfCount;

			StartSimulation();
		}

		ImGui::Separator();
	}
	else
	{
		for (int i = 0; i < 6; i++)
		{
			ImGui::Spacing();
		}

		ImGui::Separator();

		float buttonX = (ImGui::GetWindowWidth() - 200.0f) / 2.0f;

		ImGui::SetCursorPosX(buttonX);

		if (ImGui::Button("Start Simulation", ImVec2(200, 40)))
		{
			initialGrassCount = RuntimeConfig::Config.initialGrassCount;
			initialSheepCount = RuntimeConfig::Config.initialSheepCount;
			initialWolfCount = RuntimeConfig::Config.initialWolfCount;

			StartSimulation();
		}

		ImGui::Separator();
	}

#ifdef ENABLE_TESTING_FEATURES
	ImGui::Separator();
	ImGui::Spacing();

	ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 0.3f, 0.3f, 1.0f));
	ImGui::TextWrapped("TESTING MODE ENABLED - All scaling options are available");
	ImGui::PopStyleColor();
#endif

	ImGui::PopStyleColor(3);

	ImGui::End();
	}

// Renders the grass settings UI for configuring grass entity parameters.
void Simulation::RenderGrassSettings()
{
	ImGui::Text("Grass Growth Parameters");
	ImGui::Spacing();

	if (screenScaleFactor <= 0.5f)
	{
		ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4(0.0f, 0.3f, 0.0f, 0.5f));

		ImGui::SliderFloat("Time Grow (sec)", &RuntimeConfig::Config.grassConfig.timeToGrow, 1.0f, 15.0f, "%.1f");
		ImGui::SliderFloat("Time Spr Seeds (sec)", &RuntimeConfig::Config.grassConfig.timeToSpread, 2.0f, 15.0f, "%.1f");
		ImGui::SliderFloat("Lifetime (sec)", &RuntimeConfig::Config.grassConfig.lifetimeBeforeWilting, 4.5f, 20.0f, "%.1f");
		ImGui::SliderFloat("Time to Wilt (sec)", &RuntimeConfig::Config.grassConfig.timeToWilt, 1.0f, 10.0f, "%.1f");
		ImGui::SliderFloat("Spread Chance (%)", &RuntimeConfig::Config.grassConfig.spreadChance, 10.0f, 50.0f, "%.1f");

		ImGui::PopStyleColor();
	}
	else
	{
		ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4(0.0f, 0.3f, 0.0f, 0.5f));

		ImGui::SliderFloat("Time to Grow (sec)", &RuntimeConfig::Config.grassConfig.timeToGrow, 1.0f, 15.0f, "%.1f");
		ImGui::SliderFloat("Time to Spread Seeds (sec)", &RuntimeConfig::Config.grassConfig.timeToSpread, 2.0f, 15.0f, "%.1f");
		ImGui::SliderFloat("Lifetime Before Wilt (sec)", &RuntimeConfig::Config.grassConfig.lifetimeBeforeWilting, 4.5f, 20.0f, "%.1f");
		ImGui::SliderFloat("Time to Wilt (sec)", &RuntimeConfig::Config.grassConfig.timeToWilt, 1.0f, 10.0f, "%.1f");
		ImGui::SliderFloat("Spread Chance (%)", &RuntimeConfig::Config.grassConfig.spreadChance, 5.0f, 50.0f, "%.1f");

		ImGui::PopStyleColor();
	}

	for (int i = 0; i < 10; i++)
	{
		ImGui::Spacing();
	}

	ImGui::TextWrapped("These parameters control the growth and spread of grass in the ecosystem.");
	ImGui::Spacing();
	ImGui::TextWrapped("Grass is the foundation of the food chain and provides energy for sheep.");
}

// Renders the sheep settings UI for configuring sheep entity parameters.
void Simulation::RenderSheepSettings()
{
	ImGui::Text("Sheep Parameters");
	ImGui::Spacing();

	ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4(0.0f, 0.2f, 0.4f, 0.5f));

	ImGui::Text("Health & Hunger:");

	if (screenScaleFactor <= 0.5f)
	{
		ImGui::SliderFloat("Max Health", &RuntimeConfig::Config.sheepConfig.maxHealth, 50.0f, 200.0f, "%.0f");
		ImGui::SliderFloat("Max Hunger", &RuntimeConfig::Config.sheepConfig.maxHunger, 50.0f, 200.0f, "%.0f");
		ImGui::SliderFloat("Max Fullness", &RuntimeConfig::Config.sheepConfig.maxFullness, 50.0f, 200.0f, "%.0f");
		ImGui::SliderFloat("Hunger Inc Rate", &RuntimeConfig::Config.sheepConfig.hungerIncreaseRate, 5.0f, 50.0f, "%.1f");
		ImGui::SliderFloat("Health Dec Rate", &RuntimeConfig::Config.sheepConfig.healthDecreaseRate, 5.0f, 40.0f, "%.1f");
	}
	else
	{
		ImGui::SliderFloat("Max Health", &RuntimeConfig::Config.sheepConfig.maxHealth, 50.0f, 200.0f, "%.0f");
		ImGui::SliderFloat("Max Hunger", &RuntimeConfig::Config.sheepConfig.maxHunger, 50.0f, 200.0f, "%.0f");
		ImGui::SliderFloat("Max Fullness", &RuntimeConfig::Config.sheepConfig.maxFullness, 50.0f, 200.0f, "%.0f");
		ImGui::SliderFloat("Hunger Increase Rate", &RuntimeConfig::Config.sheepConfig.hungerIncreaseRate, 5.0f, 50.0f, "%.1f");
		ImGui::SliderFloat("Health Decrease Rate", &RuntimeConfig::Config.sheepConfig.healthDecreaseRate, 5.0f, 40.0f, "%.1f");
	}

	for (int i = 0; i < 6; i++)
	{
		ImGui::Spacing();
	}

	ImGui::Text("Movement:");
	ImGui::SliderFloat("Wander Speed", &RuntimeConfig::Config.sheepConfig.wanderSpeed, 50.0f, 150.0f, "%.0f");
	ImGui::SliderFloat("Flee Speed", &RuntimeConfig::Config.sheepConfig.fleeSpeed, 100.0f, 300.0f, "%.0f");

	for (int i = 0; i < 6; i++)
	{
		ImGui::Spacing();
	}

	ImGui::Text("Perception:");

	if (screenScaleFactor <= 0.5f)
	{
		ImGui::SliderFloat("Wolf Detect Radius", &RuntimeConfig::Config.sheepConfig.wolfDetectionRadius, 100.0f, 300.0f, "%.0f");
		ImGui::SliderFloat("Grass Detect Radius", &RuntimeConfig::Config.sheepConfig.grassDetectionRadius, 40.0f, 200.0f, "%.0f");
	}
	else
	{
		ImGui::SliderFloat("Wolf Detection Radius", &RuntimeConfig::Config.sheepConfig.wolfDetectionRadius, 100.0f, 300.0f, "%.0f");
		ImGui::SliderFloat("Grass Detection Radius", &RuntimeConfig::Config.sheepConfig.grassDetectionRadius, 40.0f, 200.0f, "%.0f");
	}

	for (int i = 0; i < 6; i++)
	{
		ImGui::Spacing();
	}

	ImGui::Text("Group Behavior:");
	ImGui::SliderFloat("Group Radius", &RuntimeConfig::Config.sheepConfig.groupRadius, 50.0f, 200.0f, "%.0f");
	ImGui::SliderInt("Max Group Size", &RuntimeConfig::Config.sheepConfig.maxGroupSize, 2, 10);

	ImGui::PopStyleColor();

	for (int i = 0; i < 10; i++)
	{
		ImGui::Spacing();
	}

	ImGui::TextWrapped("These parameters control sheep behavior, including their vitals,");
	ImGui::Spacing();
	ImGui::TextWrapped("movement abilities, perception, and social behavior.");
}

// Renders the wolf settings UI for configuring wolf entity parameters.
void Simulation::RenderWolfSettings()
{
	ImGui::Text("Wolf Parameters");
	ImGui::Spacing();

	ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4(0.4f, 0.0f, 0.0f, 0.5f));

	ImGui::Text("Hunger & Stamina:");
	

	if (screenScaleFactor <= 0.5f)
	{
		ImGui::SliderFloat("Max Hunger", &RuntimeConfig::Config.wolfConfig.maxHunger, 50.0f, 200.0f, "%.0f");
		ImGui::SliderFloat("Hunger Inc Rate", &RuntimeConfig::Config.wolfConfig.hungerIncreaseRate, 5.0f, 50.0f, "%.1f");
		ImGui::SliderFloat("Hunger Thresh", &RuntimeConfig::Config.wolfConfig.hungerThreshold, 10.0f, 50.0f, "%.0f");
		ImGui::SliderFloat("Max Stamina", &RuntimeConfig::Config.wolfConfig.staminaMax, 50.0f, 200.0f, "%.0f");
		ImGui::SliderFloat("Stamina Drain", &RuntimeConfig::Config.wolfConfig.staminaDrainRate, 5.0f, 40.0f, "%.1f");
	}
	else
	{
		ImGui::SliderFloat("Max Hunger", &RuntimeConfig::Config.wolfConfig.maxHunger, 50.0f, 200.0f, "%.0f");
		ImGui::SliderFloat("Hunger Increase Rate", &RuntimeConfig::Config.wolfConfig.hungerIncreaseRate, 5.0f, 50.0f, "%.1f");
		ImGui::SliderFloat("Hunger Threshold", &RuntimeConfig::Config.wolfConfig.hungerThreshold, 10.0f, 50.0f, "%.0f");
		ImGui::SliderFloat("Max Stamina", &RuntimeConfig::Config.wolfConfig.staminaMax, 50.0f, 200.0f, "%.0f");
		ImGui::SliderFloat("Stamina Drain Rate", &RuntimeConfig::Config.wolfConfig.staminaDrainRate, 5.0f, 40.0f, "%.1f");
	}

	for (int i = 0; i < 6; i++)
	{
		ImGui::Spacing();
	}

	ImGui::Text("Movement:");
	ImGui::SliderFloat("Hunt Speed", &RuntimeConfig::Config.wolfConfig.huntSpeed, 100.0f, 300.0f, "%.0f");
	ImGui::SliderFloat("Roam Speed", &RuntimeConfig::Config.wolfConfig.roamSpeed, 50.0f, 150.0f, "%.0f");

	for (int i = 0; i < 6; i++)
	{
		ImGui::Spacing();
	}

	ImGui::Text("Combat:");

	if (screenScaleFactor <= 0.5f)
	{
		ImGui::SliderFloat("Attack Damage", &RuntimeConfig::Config.wolfConfig.attackDamage, 10.0f, 50.0f, "%.0f");
		ImGui::SliderFloat("Sheep Detect Radius", &RuntimeConfig::Config.wolfConfig.sheepDetectionRadius, 100.0f, 350.0f, "%.0f");
	}
	else
	{
		ImGui::SliderFloat("Attack Damage", &RuntimeConfig::Config.wolfConfig.attackDamage, 10.0f, 75.0f, "%.0f");
		ImGui::SliderFloat("Sheep Detection Radius", &RuntimeConfig::Config.wolfConfig.sheepDetectionRadius, 100.0f, 350.0f, "%.0f");
	}

	ImGui::PopStyleColor();

	for (int i = 0; i < 10; i++)
	{
		ImGui::Spacing();
	}

	ImGui::TextWrapped("These parameters control wolf behavior, including their hunger,");
	ImGui::Spacing();
	ImGui::TextWrapped("stamina, movement abilities, and combat effectiveness.");
}

// Draws the entity status window showing the current state of all entities.
void Simulation::DrawEntityStatusWindow()
{
	if (ImGui::Begin("Entity Status", nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_HorizontalScrollbar))
	{
		DrawTabBar();

		if (showEntityStatus)
		{
			ImGui::Columns(3, "EntityColumns", true);

			ImGui::Text("Grass Entities"); ImGui::NextColumn();
			ImGui::Text("Sheep Entities"); ImGui::NextColumn();
			ImGui::Text("Wolf Entities"); ImGui::NextColumn();

			ImGui::Separator();


			const auto& grasses = world->GetGrasses();
			int grassCount = 0;

			for (const auto& grass : grasses)
			{
				std::string stateStr;
				switch (grass->GetCurrentState())
				{
				case GrassStateMachine::GrassState::SeedsPlanted:
					stateStr = "Seeds Planted";
					break;
				case GrassStateMachine::GrassState::FullyGrown:
					stateStr = "Fully Grown";
					break;
				case GrassStateMachine::GrassState::Wilting:
					stateStr = "Wilting";
					break;
				default:
					stateStr = "Unknown";
					break;
				}

				std::string statusText = "Grass #" + std::to_string(++grassCount) + ": " + stateStr;

				if (grass->IsBeingEaten())
				{
					statusText += " (Being Eaten)";
				}

				ImGui::Text("%s", statusText.c_str());
			}
			ImGui::NextColumn();



			const auto& sheeps = world->GetSheep();
			int sheepCount = 0;

			for (const auto& sheep : sheeps)
			{
				std::string stateStr;

				switch (sheep->GetCurrentState())
				{
				case SheepStateMachine::SheepState::WanderingAlone:
					stateStr = "Wandering Alone";
					break;
				case SheepStateMachine::SheepState::WanderingInGroup:
					stateStr = "Wandering in Group";
					break;
				case SheepStateMachine::SheepState::Eating:
					stateStr = "Eating";
					break;
				case SheepStateMachine::SheepState::Defecating:
					stateStr = "Defecating";
					break;
				case SheepStateMachine::SheepState::RunningAway:
					stateStr = "Running Away";
					break;
				case SheepStateMachine::SheepState::Reproducing:
					stateStr = "Reproducing";
					break;
				default:
					stateStr = "Unknown";
					break;
				}

				std::string groupStatus;

				if (sheep->IsInGroup())
				{
					if (sheep->IsGroupLeader())
					{
						groupStatus = " (Group Leader)";
					}
					else
					{
						groupStatus = " (In Group)";
					}
				}
				else
				{
					groupStatus = " (No Group)";
				}

				std::string healthInfo = " H:" + std::to_string(static_cast<int>(sheep->GetHealth())) + " F:" + std::to_string(static_cast<int>(sheep->GetHunger()));

				ImGui::Text("Sheep #%d: %s%s%s", ++sheepCount, stateStr.c_str(), groupStatus.c_str(), healthInfo.c_str());
			}
			ImGui::NextColumn();



			const auto& wolves = world->GetWolves();
			int wolfCount = 0;

			for (const auto& wolf : wolves)
			{
				std::string stateStr;
				switch (wolf->GetCurrentState())
				{
				case WolfStateMachine::WolfState::Sleeping:
					stateStr = "Sleeping";
					break;
				case WolfStateMachine::WolfState::Roaming:
					stateStr = "Roaming";
					break;
				case WolfStateMachine::WolfState::Hunting:
					stateStr = "Hunting";
					break;
				case WolfStateMachine::WolfState::Eating:
					stateStr = "Eating";
					break;
				case WolfStateMachine::WolfState::ReturnToDen:
					stateStr = "Returning to Den";
					break;
				default:
					stateStr = "Unknown";
					break;
				}

				std::string targetInfo;

				if (wolf->GetTargetSheep() != nullptr)
				{
					targetInfo = " (Targeting Sheep)";
				}

				std::string hungerInfo = " H:" + std::to_string(static_cast<int>(wolf->GetHunger())) + " S:" + std::to_string(static_cast<int>(wolf->GetStamina()));

				ImGui::Text("Wolf #%d: %s%s%s", ++wolfCount, stateStr.c_str(), targetInfo.c_str(), hungerInfo.c_str());
			}

			ImGui::Columns(1);
			ImGui::Separator();
		}
		else
		{
			DrawConsoleWindow();
		}
	}
	ImGui::End();
}

// Draws the simulation layout with the main world view, legend, and entity status window.
void Simulation::DrawSimulationLayout()
{
	const float simulationHeight = screenHeight * 2.0f / 3.0f;
	const float consoleHeight = screenHeight / 3.0f;
	const float mainAreaWidth = screenWidth * 3.0f / 4.0f;
	const float legendWidth = screenWidth / 4.0f;
	const float titleBarHeight = 20.0f * screenScaleFactor;

	ImGui::SetNextWindowPos(ImVec2(0, 0));
	ImGui::SetNextWindowSize(ImVec2(mainAreaWidth, simulationHeight));

	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
	ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.0f, 0.0f, 0.0f, 0.0f));

	std::string windowTitle = "Simulation View - FPS: " + std::to_string(GetFPS());

	if (ImGui::Begin(windowTitle.c_str(), nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse | ImGuiWindowFlags_NoBringToFrontOnFocus))
	{
	}
	ImGui::End();

	ImGui::PopStyleColor();
	ImGui::PopStyleVar();

	ImGui::SetNextWindowPos(ImVec2(mainAreaWidth, 0));
	ImGui::SetNextWindowSize(ImVec2(legendWidth, simulationHeight));
	DrawLegendPanel();

	ImGui::SetNextWindowPos(ImVec2(0, simulationHeight));
	ImGui::SetNextWindowSize(ImVec2(static_cast<float>(screenWidth), consoleHeight));
	DrawEntityStatusWindow();

	simulationViewport.y = titleBarHeight;
	simulationViewport.width = mainAreaWidth;
	simulationViewport.height = simulationHeight - titleBarHeight;

	RenderSimulationInViewport();
}

// Renders the simulation world within the viewport area.
void Simulation::RenderSimulationInViewport()
{
	BeginScissorMode((int)simulationViewport.x, (int)simulationViewport.y, (int)simulationViewport.width, (int)simulationViewport.height);

	DrawRectangle((int)simulationViewport.x, (int)simulationViewport.y, (int)simulationViewport.width, (int)simulationViewport.height, RAYWHITE);

	world->Draw();

	DrawRectangleLines((int)simulationViewport.x, (int)simulationViewport.y, (int)simulationViewport.width, (int)simulationViewport.height, RED);

	EndScissorMode();
}

// Draws the legend panel showing the color codes for each entity state.
void Simulation::DrawLegendPanel()
{
	if (ImGui::Begin("Legend", nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse))
	{
		ImGui::Text("Entity State Legend");
		ImGui::Separator();

		ImGui::Text("Grass States:");

		ImGui::ColorButton("##SeedsPlanted", ImVec4(0.0f, 1.0f, 0.4f, 1.0f), 0, ImVec2(20, 20)); 
		ImGui::SameLine();
		ImGui::Text("Seeds Planted/Growing");

		ImGui::ColorButton("##FullyGrown", ImVec4(0.0f, 0.39f, 0.0f, 1.0f), 0, ImVec2(20, 20)); 
		ImGui::SameLine();
		ImGui::Text("Fully Grown");

		ImGui::ColorButton("##Wilting", ImVec4(0.5f, 0.3f, 0.0f, 1.0f), 0, ImVec2(20, 20));
		ImGui::SameLine();
		ImGui::Text("Wilting");

		for (int i = 0; i < 5; i++)
		{
			ImGui::Spacing();
		}

		ImGui::Separator();
		ImGui::Text("Sheep States:");

		ImGui::ColorButton("##WanderingAlone", ImVec4(0.53f, 0.81f, 0.98f, 1.0f), 0, ImVec2(20, 20)); 
		ImGui::SameLine();
		ImGui::Text("Wandering Alone");

		ImGui::ColorButton("##WanderingInGroup", ImVec4(0.1f, 0.1f, 0.6f, 1.0f), 0, ImVec2(20, 20)); 
		ImGui::SameLine();
		ImGui::Text("Wandering in Group");

		ImGui::ColorButton("##SheepEating", ImVec4(0.4f, 0.2f, 0.6f, 1.0f), 0, ImVec2(20, 20)); 
		ImGui::SameLine();
		ImGui::Text("Eating");

		ImGui::ColorButton("##Defecating", ImVec4(1.0f, 0.5f, 0.0f, 1.0f), 0, ImVec2(20, 20)); 
		ImGui::SameLine();
		ImGui::Text("Defecating");

		ImGui::ColorButton("##RunningAway", ImVec4(1.0f, 0.0f, 0.2f, 1.0f), 0, ImVec2(20, 20)); 
		ImGui::SameLine();
		ImGui::Text("Running Away");

		ImGui::ColorButton("##Reproducing", ImVec4(1.0f, 0.4f, 0.7f, 1.0f), 0, ImVec2(20, 20));
		ImGui::SameLine();
		ImGui::Text("Reproducing");


		for (int i = 0; i < 5; i++)
		{
			ImGui::Spacing();
		}


		ImGui::Separator();
		ImGui::Text("Wolf States:");

		ImGui::ColorButton("##Sleeping", ImVec4(0.0f, 0.0f, 0.0f, 1.0f), 0, ImVec2(20, 20)); 
		ImGui::SameLine();
		ImGui::Text("Sleeping");

		ImGui::ColorButton("##Roaming", ImVec4(1.0f, 1.0f, 0.0f, 1.0f), 0, ImVec2(20, 20)); 
		ImGui::SameLine();
		ImGui::Text("Roaming");

		ImGui::ColorButton("##Hunting", ImVec4(0.5f, 0.0f, 0.0f, 1.0f), 0, ImVec2(20, 20)); 
		ImGui::SameLine();
		ImGui::Text("Hunting");

		ImGui::ColorButton("##WolfEating", ImVec4(0.016f, 0.4f, 0.392f, 1.0f), 0, ImVec2(20, 20)); 
		ImGui::SameLine();
		ImGui::Text("Eating");

		ImGui::ColorButton("##ReturnToDen", ImVec4(0.5f, 0.75f, 0.2f, 1.0f), 0, ImVec2(20, 20)); 
		ImGui::SameLine();
		ImGui::Text("Returning to Den");

		for (int i = 0; i < 10; i++)
		{
			ImGui::Spacing();
		}
		ImGui::Checkbox("Show Detection Radii (R)", &showDetectionRadii);
		ImGui::Text("Toggle visibility of detection ranges");
	}
	ImGui::End();
}

// Draws the tab bar for switching between Entity Status and Console.
void Simulation::DrawTabBar()
{
	if (ImGui::BeginTabBar("SimulationTabs"))
	{
		if (ImGui::BeginTabItem("Entity Status"))
		{
			showEntityStatus = true;
			ImGui::EndTabItem();
		}

		if (ImGui::BeginTabItem("Console"))
		{
			showEntityStatus = false;
			ImGui::EndTabItem();
		}

		ImGui::EndTabBar();
	}
}

// Draws the console window with captured printf output.
void Simulation::DrawConsoleWindow()
{
	ImGui::SetCursorPos(ImVec2(ImGui::GetStyle().WindowPadding.x, ImGui::GetCursorPosY()));

	if (ImGui::Button("Clear Console"))
	{
		consoleMessages.clear();
	}

	ImGui::SameLine();

	static bool autoScroll = true;

	ImGui::Checkbox("Auto-scroll", &autoScroll);

	ImGui::BeginChild("ConsoleScrollRegion", ImVec2(0, 0), true, ImGuiWindowFlags_HorizontalScrollbar);

	for (const auto& message : consoleMessages)
	{
		if (message.empty())
		{
			ImGui::Text(" ");
			continue;
		}

		ImVec4 textColor = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);

		if (message.find("ERROR") != std::string::npos)
		{
			textColor = ImVec4(1.0f, 0.4f, 0.4f, 1.0f);
		}
		else if (message.find("WARNING") != std::string::npos)
		{
			textColor = ImVec4(1.0f, 0.8f, 0.0f, 1.0f);
		}
		else if (message.find("Wolf") != std::string::npos)
		{
			textColor = ImVec4(0.8f, 0.3f, 0.8f, 1.0f);
		}
		else if (message.find("Sheep") != std::string::npos)
		{
			textColor = ImVec4(0.4f, 0.8f, 1.0f, 1.0f);
		}
		else if (message.find("INFO:") != std::string::npos)
		{
			textColor = ImVec4(0.5f, 0.8f, 0.5f, 1.0f);
		}

		ImGui::TextColored(textColor, "%s", message.c_str());
	}

	if (autoScroll && ImGui::GetScrollY() < ImGui::GetScrollMaxY())
	{
		ImGui::SetScrollHereY(1.0f);
	}

	ImGui::EndChild();
}

// Starts the simulation with the current setup parameters.
void Simulation::StartSimulation()
{
	float scaledCellSize = ValueConfig::World::CellSize * screenScaleFactor;

	int worldWidth = static_cast<int>(simulationViewport.width / scaledCellSize);
	int worldHeight = static_cast<int>(simulationViewport.height / scaledCellSize);

	world = std::make_unique<World>(worldWidth, worldHeight, ValueConfig::World::CellSize, simulationViewport.y, screenScaleFactor, this);
	world->Initialize(initialGrassCount, initialSheepCount, initialWolfCount);

	currentState = SimulationState::Running;
}

// Helper Function that Redirects printf to the console.
void Simulation::ConsoleOutputCallback(const char* text, void* userData)
{
	if (!text || !userData)
	{
		return;
	}

	Simulation* _simulation = static_cast<Simulation*>(userData);
	_simulation->consoleMessages.push_back(text);

	if (_simulation->consoleMessages.size() > 1000)
	{
		_simulation->consoleMessages.erase(_simulation->consoleMessages.begin());
	}
}

// Centers the window on the screen based on the given width and height.
void Simulation::CenterWindow(int width, int height)
{
	int monitorWidth = GetMonitorWidth(GetCurrentMonitor());
	int monitorHeight = GetMonitorHeight(GetCurrentMonitor());

	int posX = (monitorWidth - width) / 2;
	int posY = (monitorHeight - height) / 2;

	SetWindowPosition(posX, posY);
}

// Manages the main simulation loop until the window is closed.
void Simulation::Run()
{
	Initialize();

	while (!WindowShouldClose())
	{
		Update();
		Draw();
	}
}