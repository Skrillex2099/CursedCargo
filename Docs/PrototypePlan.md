# Prototype Plan

## Milestone 1 — Foundation

- Unreal Engine 5.8 C++ project structure.
- Git LFS and Unreal ignore rules.
- Interaction interface.
- Replicated collectible cargo actor.
- Replicated extraction van and shared money total.

## Milestone 2 — Carry interaction

- Third-person C++ player character.
- `E` traces for a nearby interactable item.
- Server-authoritative pickup validation.
- Replicated carried-item ownership and attachment.
- `E` drops the held item in front of the player and applies a small impulse.
- Blueprint-ready carry socket, interaction distance, and tuning values.

## Milestone 3 — First playable room

- [x] Temporary visible player body using Engine primitives.
- [x] Three automatically spawned test cargo items.
- [x] Prototype van and visible extraction pad.
- [x] On-screen interaction instructions.
- [x] Crosshair and contextual pickup/drop prompt.
- [x] Team-money HUD and extraction feedback.
- [ ] Final Blueprint player mesh and animation setup.
- [x] Saved prototype map assigned as the editor, game, and server default.
- [x] Separate runtime spawn positions for multiple players.
- [ ] Complete a 2-player PIE gameplay test.

## Milestone 4 — Threat

- [x] Replicated prototype monster.
- [x] Simple patrol without requiring a NavMesh.
- [x] Hearing and chase response to dropped cargo.
- [x] Noise events from running players.
- [x] Networked sprint with draining and recovering stamina.
- [x] Impact damage, durability, and value loss for fragile cargo.
- [x] Player health, monster contact damage, and damage cooldown.
- [x] Replicated three-minute mission timer.
- [x] Mission failure from zero health or expired time.
- [x] Extraction target and success result screen.
