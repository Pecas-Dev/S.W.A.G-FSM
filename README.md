# S.W.A.G-FSM <br> | Sheep, Wolves and Grass: Finite State Machine Simulation |

<br>

---

## Overview

This project implements a dynamic ecosystem simulation featuring three entities (**Sheep, Wolves and Grass**) using **Finite State Machines (FSM)** for behavior modeling. Built in **C++** with [**Raylib**](https://github.com/raysan5/raylib) for graphics rendering and [**ImGui**](https://github.com/ocornut/imgui) for the user interface, the simulation demonstrates complex agent behaviors and interactions within a controlled environment.

The ecosystem models predator-prey relationships and natural life cycles, with each entity having distinct states and behaviors:

- **Sheep** wander, eat grass, form groups, reproduce, and flee from wolves.
- **Wolves** hunt sheep, rest in dens, and manage their hunger cycles.
- **Grass** grows, spreads, and eventually wilts.

### Features

- **Complete FSM Implementation**: Each entity has a dedicated state machine with multiple states and transitions.
- **Configurable Simulation**: Adjust entity parameters through a user-friendly interface.
- **Visual Feedback**: Color-coded entities for easy state identification.
- **Real-time Statistics**: Monitor entity counts and states during simulation.
- **Sense-Decide-Act Pattern**: Entities use this pattern for realistic decision-making.
- **Dynamic Ecosystem**: Watch how population balances shift over time.
- **Group Behaviors**: Sheep exhibit flocking and social behaviors.
- **Health/Hunger/Stamina Systems**: Entities track vital statistics.

<br>

---

## Simulation Details

### Sheep States

- **Wandering Alone**: Individual exploration.
- **Wandering in Group**: Social movement with other sheep.
- **Eating**: Consuming grass for sustenance.
- **Defecating**: After reaching fullness.
- **Running Away**: When wolves are detected.
- **Reproducing**: Creating new sheep when conditions are right.

### Wolf States

- **Sleeping**: Resting in den.
- **Roaming**: Searching for prey.
- **Hunting**: Pursuing targeted sheep.
- **Eating**: Consuming captured prey.
- **Returning to Den**: Moving back home after feeding.

### Grass States

- **Seeds Planted**: Initial growth stage.
- **Fully Grown**: Mature state where seeds can be spread.
- **Wilting**: Final state before removal.

<br>

---

## User Interface

The simulation provides a comprehensive UI that allows users to:

- Configure initial entity counts.
- Adjust entity-specific parameters:
  - **Sheep**: Health, hunger, movement speed, perception radius, etc.
  - **Wolves**: Hunger thresholds, hunting speed, attack damage, etc.
  - **Grass**: Growth time, spreading chance, lifetime, etc.
- Monitor entity states in real-time.
- View console output for significant events.
- Toggle visualization of detection radii.
- Adjust display scale to match screen size.

<br>
  
---------------------------------------------

## Architecture

The project follows a clean, modular architecture with clear separation of concerns:

### Core Components

- **State Machines**: Handle entity behavior transitions.
- **Entity Base Classes**: Provide common functionality for world entities.
- **World Class**: Manages entity creation, updates, and interactions.
- **Simulation Class**: Controls the overall flow and UI rendering.

### Design Patterns

- **State Pattern**: For FSM implementation.
- **Sense-Decide-Act**: Entities perceive the environment, evaluate options, then act.
- **Component-Based Design**: Entities have distinct components for different aspects of behavior.

### Code Organization

- Consistent naming conventions (camelCase for variables, PascalCase for functions).
- Clear separation between header (.h) and implementation (.cpp) files.
- Well-documented code with focused explanatory comments.
- Proper memory management with RAII principles.

<br>

---

## Installation & Running

<br>

**Clone the repository:**

```bash
git clone https://github.com/Pecas-Dev/S.W.A.G-FSM.git
```

### Pre-compiled Executables

The easiest way to run the simulation is using the pre-compiled executables:

1. Navigate to the `Release` folder.
2. Choose the appropriate version for your system (`x64` or `86`).
3. Run the `S.W.A.G x64.bat` or `S.W.A.G x86.bat`or file.

### Building from Source

To build the project from source:

1. Open `S.W.A.G-FSM.sln` in Visual Studio.
2. Choose your configuration (Debug/Release) and platform (x64/x86).
3. Build the solution.
4. Run the program.

### System Requirements

- Windows operating system.
- Visual Studio 2022 (to build from source).
- After Testing: Display resolution of at least 1366x768 for optimal viewing.

<br>

---

<br>

## Project Structure

```
Project Root
├── S.W.A.G-FSM.sln                        // Visual Studio solution file
├── .gitignore
├── bin                                    // Binary files
├── Release                                // Pre-compiled executables
│   ├── S.W.A.G x64.bat                    // Run x64 S.W.A.G
│   ├── S.W.A.G x86.bat                    // Run x86 S.W.A.G
│   └── Platforms
│       ├── x64                            // 64-bit builds
│       └── Win32                          // 32-bit builds
│
├── Dependencies                           // External dependencies
│   └── raylib                             // Raylib graphics library files
│
├── S.W.A.G-FSM                            // Source files and project configurations
│   ├── Assets                             // Simulation assets
│   │   ├── Wolf                           // Wolf entity sprites
│   │   │   └── Wolf.png
│   │   │
│   │   ├── Sheep                          // Sheep entity sprites
│   │   │   └── Sheep.png
│   │   │
│   │   └── Grass                          // Grass entity sprites in various states
│   │       ├── FullyGrown
│   │       │   └── Grass7.png
│   │       │
│   │       ├── Wilting
│   │       │   └── Grass8.png
│   │       │
│   │       └── Growing                    // Growth animation frames
│   │           ├── Grass1.png
│   │           ├── Grass2.png
│   │           ├── Grass3.png
│   │           ├── Grass4.png
│   │           ├── Grass5.png
│   │           └── Grass6.png
│   │
│   ├── S.W.A.G                            // Source code of the project
│   │   ├── include                        // Header files
│   │   │   ├── Entities                   // Entity definitions
│   │   │   │    ├── Grass                 // Grass entity and states
│   │   │   │    ├── Sheep                 // Sheep entity and states
│   │   │   │    └── Wolf                  // Wolf entity and states
│   │   │   │
│   │   │   ├── Simulation                 // Simulation control and UI
│   │   │   ├── Utility                    // Helper classes and functions
│   │   │   ├── vendor                     // Third-party libraries
│   │   │   └── World                      // World management
│   │   │
│   │   ├── source                         // Implementation files
│   │   │   ├── Entities                   // Entity implementations
│   │   │   │    ├── Grass                 // Grass behaviors
│   │   │   │    ├── Sheep                 // Sheep behaviors
│   │   │   │    └── Wolf                  // Wolf behaviors
│   │   │   │
│   │   │   ├── Simulation                 // Simulation logic
│   │   │   ├── Utility                    // Helper implementations
│   │   │   ├── vendor                     // Third-party code
│   │   │   └── World                      // World class implementation
│   │   │
│   │   └── main.cpp                       // Entry point
│   │
│   └── Project Files                      // Visual Studio project configuration files
└── README.md                              // This documentation file
```

<br>

---

## Controls and Interaction

Once the simulation is running:

- **Press R**: Toggle detection radius visualization.
- Use the UI panels to monitor entity states.
- Watch the ecosystem evolve over time.
- Check the console tab for detailed event logs.

---

## Development

This project was developed as an exploration of agent-based modeling and finite state machines. The implementation focused on creating a visually engaging simulation that demonstrates complex entity interactions while maintaining clean architecture principles. The project showcases my interest in game AI, interactive simulations, and **UI/UX design**, combining **technical programming skills** with **creative system design**.

---

## Acknowledgments

- [**Raylib**](https://github.com/raysan5/raylib) library for rendering.
- [**Dear ImGui**](https://github.com/ocornut/imgui) for the user interface components.

---

## Credits

This project was created by **_PecasDev_**.
