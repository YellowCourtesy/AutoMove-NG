#include "../PCH.h"
#include "OnMarkerChange.h"

#include "../Papyrus.h"

namespace AutoMove
{
	namespace Events
	{
		// Event Hook

		SKSE::RegistrationSet<std::uint8_t>* HKPlacePlayerMarkerCallbackFunctor::GetEvent()
		{
			static SKSE::RegistrationSet<std::uint8_t> onCustomMarkerChange("OnCustomMarkerChange");
			return &onCustomMarkerChange;
		}

		static REL::Relocation<decltype(HKPlacePlayerMarkerCallbackFunctor::RunHook)> oPlacePlayerMarker;
		void HKPlacePlayerMarkerCallbackFunctor::InstallHook()
		{
			REL::Relocation<std::uintptr_t> MarkerFunctorVTBL{ HKPlacePlayerMarkerCallbackFunctor::VTABLE[0] };
			oPlacePlayerMarker = MarkerFunctorVTBL.write_vfunc(1, HKPlacePlayerMarkerCallbackFunctor::RunHook);
		}

		void HKPlacePlayerMarkerCallbackFunctor::RunHook(RE::BSIntrusiveRefCounted* functor, std::uint8_t a_msg)
		{
			logger::info("OnCustomMarkerChange: button {}", a_msg);
			auto onCustomMarkerChange = GetEvent();
			onCustomMarkerChange->QueueEvent(a_msg);
			return oPlacePlayerMarker(functor, a_msg);
		}


		// Papyrus

		void RegisterForCustomMarkerChange(RE::TESQuest* script)
		{
			auto onCustomMarkerChange = HKPlacePlayerMarkerCallbackFunctor::GetEvent();
			onCustomMarkerChange->Register(script);
		}
		void UnregisterForCustomMarkerChange(RE::TESQuest* script)
		{
			auto onCustomMarkerChange = HKPlacePlayerMarkerCallbackFunctor::GetEvent();
			onCustomMarkerChange->Unregister(script);
		}

		void HKPlacePlayerMarkerCallbackFunctor::RegisterPapyrus(RE::BSScript::IVirtualMachine* a_vm)
		{
			a_vm->RegisterFunction("RegisterForCustomMarkerChange", "AutoMove", RegisterForCustomMarkerChange);
			a_vm->RegisterFunction("UnregisterForCustomMarkerChange", "AutoMove", UnregisterForCustomMarkerChange);
		}
	}
}


