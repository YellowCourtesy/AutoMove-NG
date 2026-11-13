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
			std::uintptr_t Base() { return REL::Module::get().base(); }

			bool IsCall(std::uintptr_t a_addr) { return *reinterpret_cast<const std::uint8_t*>(a_addr) == 0xE8; }

			std::uintptr_t CallTarget(std::uintptr_t a_addr)
			{
				const auto disp = *reinterpret_cast<const std::int32_t*>(a_addr + 1);
				return a_addr + 5 + disp;
			}

			// Address Library ID of an address, or 0 if the address is not the start of a known function.
			// Does not use Offset2ID::operator() because it stops the game on an unknown address.
			std::uint64_t FindID(const REL::Offset2ID& a_db, std::uintptr_t a_addr)
			{
				const auto offset = a_addr - Base();
				const auto it = std::lower_bound(a_db.begin(), a_db.end(), offset,
					[](const auto& a_elem, std::size_t a_off) { return a_elem.offset < a_off; });
				return (it != a_db.end() && it->offset == offset) ? it->id : 0;
			}

			// Find a call instruction in [a_fn, a_fn + a_len) whose target is a_target.
			std::uintptr_t FindCallTo(std::uintptr_t a_fn, std::size_t a_len, std::uintptr_t a_target)
			{
				for (std::size_t i = 0; i + 5 <= a_len; ++i) {
					const auto addr = a_fn + i;
					if (IsCall(addr) && CallTarget(addr) == a_target)
						return addr;
				}
				return 0;
			}

			// Write all call instructions to known functions into the log. This data lets us find
			// the correct patch offset for a new game version.
			void LogCallSites(std::uintptr_t a_fn, std::size_t a_len, std::string_view a_name)
			{
				try {
					const REL::Offset2ID db;
					logger::info("{}: function at offset {:X}, calls to known functions:", a_name, a_fn - Base());
					for (std::size_t i = 0; i + 5 <= a_len; ++i) {
						const auto addr = a_fn + i;
						if (!IsCall(addr))
							continue;
						if (const auto id = FindID(db, CallTarget(addr)); id != 0)
							logger::info("    +{} (0x{:X}) -> ID {}", i, i, id);
					}
				} catch (...) {
					logger::warn("{}: could not read the Address Library to list calls", a_name);
				}
			}

			// Write a 5 byte call hook only when the patch site holds a call (E8) instruction.
			// If a game update moves the call, the hook is skipped and logged. This prevents a crash.
			template <class F>
			std::uintptr_t SafeWriteCall(std::uintptr_t a_src, F a_dst, std::string_view a_name)
			{
				if (!IsCall(a_src)) {
					logger::error("{}: no call instruction at patch site {:X}, hook not installed", a_name, a_src - Base());
					return 0;
				}
				logger::info("{}: hook installed at {:X}", a_name, a_src - Base());
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

			const auto fn = HorseCameraState_HandleLookInput.address();
			const auto site = fn + REL::Relocate(151, 140);
			if (!IsCall(site))
				LogCallSites(fn, 0x200, __func__);
			SafeWriteCall(site, CharGetRotationXHook, __func__);
		}

		void InstallPlayerCam_subHook()
		{
			static REL::Relocation<uintptr_t> PlayerCam_sub{ RELOCATION_ID(49907, 50840) };
			// Actor::SetLooking (old name SetRotationX), the call that this hook replaces.
			static REL::Relocation<uintptr_t> Actor_SetLooking{ RELOCATION_ID(36602, 37610) };

			const auto fn = PlayerCam_sub.address();
			auto site = fn + REL::Relocate(509, 516);
			if (!IsCall(site) || CallTarget(site) != Actor_SetLooking.address()) {
				// The offset moved in a newer game version. Search the function for the call.
				if (const auto found = FindCallTo(fn, 0x600, Actor_SetLooking.address()); found != 0) {
					logger::info("{}: call to SetLooking found at +{}", __func__, found - fn);
					site = found;
				} else {
					LogCallSites(fn, 0x600, __func__);
				}
			}
			SafeWriteCall(site, CharModifyRotationXHook, __func__);
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
