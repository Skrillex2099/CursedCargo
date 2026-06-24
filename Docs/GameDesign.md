# Cursed Cargo — Game Design

## High concept

**Cursed Cargo** is a cooperative third-person extraction game for 2–6 players. A crew enters dangerous locations, recovers valuable and often fragile cargo, and loads it into an extraction van while creatures react to the noise the crew makes.

## Core loop

1. Enter a location with limited time.
2. Search for cargo and judge its value, weight, and fragility.
3. Carry valuable objects back to the van without breaking them.
4. Avoid or distract monsters that investigate footsteps and dropped cargo.
5. Extract before time runs out and spend the team's earnings on equipment and progression.

## Cargo

Every collectible can define:

- monetary value;
- carry weight;
- durability and fragility;
- noise generated when dropped;
- a mesh and presentation supplied by a Blueprint child.

Only one player can carry an item at a time. Pickup, drop, physics state, and extraction value are authoritative on the server.

## Prototype scope

- third-person movement and camera;
- multiplayer replication for 2–6 players;
- pickup, carrying, throwing, and dropping cargo;
- extraction van that converts cargo into team money;
- simple monster patrol/chase behavior driven by noise;
- mission timer and money HUD.

