#include "hooks.h"

// include minhook for epic hookage
#include "../../ext/minhook/minhook.h"

#include <intrin.h>

#include "../hacks/misc.h"

void hooks::Setup() noexcept
{
	MH_Initialize();

	// AllocKeyValuesMemory hook
	MH_CreateHook(
		memory::Get(interfaces::keyValuesSystem, 1),
		&AllocKeyValuesMemory,
		reinterpret_cast<void**>(&AllocKeyValuesMemoryOriginal)
	);

	// CreateMove hook
	MH_CreateHook(
		memory::Get(interfaces::clientMode, 24),
		&CreateMove,
		reinterpret_cast<void**>(&CreateMoveOriginal)
	);
	// DoPostScreenSpaceEffects hook
	MH_CreateHook(
		memory::Get(interfaces::clientMode, 44),//is @ index 44
		&DoPostScreenSpaceEffects, // our hook
		reinterpret_cast<void**>(&DoPostScreenSpaceEffectsOriginal) // original func
	);

	MH_EnableHook(MH_ALL_HOOKS);
}

void hooks::Destroy() noexcept
{
	// restore hooks
	MH_DisableHook(MH_ALL_HOOKS);
	MH_RemoveHook(MH_ALL_HOOKS);

	// uninit minhook
	MH_Uninitialize();
}

void* __stdcall hooks::AllocKeyValuesMemory(const std::int32_t size) noexcept
{
	// if function is returning to speficied addresses, return nullptr to "bypass"
	if (const std::uint32_t address = reinterpret_cast<std::uint32_t>(_ReturnAddress());
		address == reinterpret_cast<std::uint32_t>(memory::allocKeyValuesEngine) ||
		address == reinterpret_cast<std::uint32_t>(memory::allocKeyValuesClient)) 
		return nullptr;

	// return original
	return AllocKeyValuesMemoryOriginal(interfaces::keyValuesSystem, size);
}

bool __stdcall hooks::CreateMove(float frameTime, CUserCmd* cmd) noexcept
{
	// make sure this function is being called from CInput::CreateMove
	if (!cmd->commandNumber)
		return CreateMoveOriginal(interfaces::clientMode, frameTime, cmd);

	// this would be done anyway by returning true
	if (CreateMoveOriginal(interfaces::clientMode, frameTime, cmd))
		interfaces::engine->SetViewAngles(cmd->viewAngles);

	// get our local player here
	globals::UpdateLocalPlayer();

	if (globals::localPlayer && globals::localPlayer->IsAlive())
	{
		// example bhop
		hacks::RunBunnyHop(cmd);
	}

	return false;
}


//esp
void __stdcall hooks::DoPostScreenSpaceEffects(const void* viewSetup) noexcept {

	// sure localplayer is valid and in game
	if (globals::localPlayer && interfaces::engine->IsInGame()) {

		for (int beratloop = 0; beratloop < interfaces::glow->glowObjects.size; beratloop++) {

			IGlowManager::CGlowObject& glowObject = interfaces::glow->glowObjects[beratloop];

			//used and valid
			if (glowObject.IsUnused()) continue;
			if (!glowObject.entity) continue;

			switch (glowObject.entity->GetClientClass()->classID)
			{
				case CClientClass::CCSPlayer:
					if (!glowObject.entity->IsAlive()) break;


					glowObject.SetColor(0.f, 0.f, 1.f);
					if (glowObject.entity->GetTeam() != globals::localPlayer->GetTeam()) {

						glowObject.SetColor(1.f, 0.f, 0.f);
					}

					break;
			
				default:
					break;
			}

		}

	}

}

	
		