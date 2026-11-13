#include "../PCH.h"
#include "Hooks.h"


namespace AutoMove
{
	namespace Hooks
	{
		float ActorGetSomeStateHook(RE::Actor* actor);

		static REL::Relocation<decltype(ActorGetSomeStateHook)> oActorGetSomeState;

		namespace
		{
			// Write a 5 byte call hook only when the patch site holds a call (E8) instruction.
			// If a game update moves the call, the hook is skipped and logged. This prevents a crash.
			template <class F>
			std::uintptr_t SafeWriteCall(std::uintptr_t a_src, F a_dst, std::string_view a_name)
			{
				if (*reinterpret_cast<const std::uint8_t*>(a_src) != 0xE8) {
					logger::error("{}: no call instruction at patch site {:X}, hook not installed", a_name, a_src - REL::Module::get().base());
					return 0;
				}
				return SKSE::GetTrampoline().write_call<5>(a_src, a_dst);
			}

			// Set only when a hook installed, so it is never left as zero by a skipped hook.
			void SetOriginal(std::uintptr_t a_orig)
			{
				if (a_orig != 0)
					oActorGetSomeState = a_orig;
			}
		}

		float ActorGetSomeStateHook(RE::Actor* actor)
		{
			auto pc = RE::PlayerCharacter::GetSingleton();
			// Old CommonLibSSE name: PlayerCharacter::unkBDA & 8 (offset 0xBDA, bit 3 in SE)
			if (pc->GetPlayerFlags().aiControlledPackage)
				return 0.0;
			return oActorGetSomeState(actor);
		}


		float CharGetRotationXHook(RE::Character*)
		{
			return RE::PlayerCharacter::GetSingleton()->GetAngleX();
		}

		void CharModifyRotationXHook(RE::Character* character, float rot)
		{
			auto pc = RE::PlayerCharacter::GetSingleton();
			character->As<RE::Actor>()->SetLooking(rot + pc->GetAngleX());
		}


		void InstallThirdPersonState_SetFreeLookModeHook()
		{
			static REL::Relocation<uintptr_t> ThirdPersonState_SetFreeLookMode{ RELOCATION_ID(49968, 50904) };

			SetOriginal(SafeWriteCall(ThirdPersonState_SetFreeLookMode.address() + 67, ActorGetSomeStateHook, __func__));
		}

		void InstallDragonCameraState_UpdRotationHook()
		{
			static REL::Relocation<uintptr_t> DragonCameraState_UpdRotation{ RELOCATION_ID(32371, 33120) };

			SetOriginal(SafeWriteCall(DragonCameraState_UpdRotation.address() + REL::Relocate(146, 147), ActorGetSomeStateHook, __func__));
		}

		void InstallHorseCameraState_UpdRotationHook()
		{
			static REL::Relocation<uintptr_t> HorseCameraState_UpdRotation{ RELOCATION_ID(49840, 50771) };

			SetOriginal(SafeWriteCall(HorseCameraState_UpdRotation.address() + 132, ActorGetSomeStateHook, __func__));
		}

		void InstallHorseCameraState_HandleLookInputHook()
		{
			static REL::Relocation<uintptr_t> HorseCameraState_HandleLookInput{ RELOCATION_ID(49839, 50770) };

			SafeWriteCall(HorseCameraState_HandleLookInput.address() + REL::Relocate(151, 140), CharGetRotationXHook, __func__);
		}

		void InstallPlayerCam_subHook()
		{
			static REL::Relocation<uintptr_t> PlayerCam_sub{ RELOCATION_ID(49907, 50840) };

			SafeWriteCall(PlayerCam_sub.address() + REL::Relocate(509, 516), CharModifyRotationXHook, __func__);
		}

		void InstallCameraHooks()
		{
			InstallThirdPersonState_SetFreeLookModeHook();
			InstallDragonCameraState_UpdRotationHook();
			InstallHorseCameraState_UpdRotationHook();
			InstallHorseCameraState_HandleLookInputHook();
			InstallPlayerCam_subHook();
		}
	}
}