# Cursed Cargo

Cooperative third-person cargo extraction game built with Unreal Engine 5.8.

## Current milestone

Milestone 2 provides a network-ready C++ foundation:

- third-person movement and camera;
- `E` to pick up a targeted cargo item;
- `E` again to drop/throw the held item;
- server-side distance and line-of-sight validation;
- replicated cargo ownership, physics state, and attachment;
- extraction van with a replicated team-money total.

## Open the project

1. Install the Unreal Engine 5.8 C++ prerequisites (Visual Studio 2022 with **Desktop development with C++** and **Game development with C++**).
2. Right-click `CursedCargo.uproject` and choose **Generate Visual Studio project files**.
3. Build the `CursedCargoEditor` target in Development Editor / Win64.
4. Open `CursedCargo.uproject`.

On Windows, `BuildProject.bat` performs step 3 automatically when Unreal Engine is installed in its default location.

## First editor setup

Create Blueprint children of:

- `CCursedCargoCharacter` — assign the player skeletal mesh and animation Blueprint;
- `CCCollectibleItem` — assign a static mesh and tune value, weight, and durability;
- `CCExtractionVan` — assign a van mesh and resize the extraction zone.

For a hand carry pose, add a `hand_rSocket` socket to the character skeleton. Without that socket, cargo uses the safe fallback anchor in front of the capsule.

See [Docs/PrototypePlan.md](Docs/PrototypePlan.md) for the next milestone.
