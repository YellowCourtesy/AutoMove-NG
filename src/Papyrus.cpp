#include "PCH.h"
#include "Papyrus.h"
#include "CustomDestination.h"
#include "Events/Events.h"


namespace AutoMove
{
	namespace Papyrus
	{


		bool ForceRefToAlias(RE::TESQuest* script, unsigned int aliasID, RE::TESObjectREFR* ref)
		{
			static REL::Relocation<decltype(ForceRefToAlias)> fn{ RELOCATION_ID(24523,25052) };
			return fn(script, aliasID, ref);
		}


		void ForceDestinationMarkerIntoAliasID(RE::TESQuest* script, unsigned int aliasID)
		{
			RE::NiPointer<RE::TESObjectREFR> handleptr;
			const auto handle = GetPlayerCustomDestinationRef();
			logger::info("ForceDestinationMarkerIntoAliasID: quest {:08X}, alias {}, marker handle {:X}",
				script ? script->GetFormID() : 0, aliasID, handle.native_handle());
			if (!handle || !script)
				return;
			RE::LookupReferenceByHandle(handle.native_handle(), handleptr);
			if (!handleptr) {
				logger::warn("ForceDestinationMarkerIntoAliasID: marker handle does not resolve to a reference");
				return;
			}
			const auto pos = handleptr->GetPosition();
			logger::info("ForceDestinationMarkerIntoAliasID: marker ref {:08X} at ({:.0f}, {:.0f}, {:.0f})",
				handleptr->GetFormID(), pos.x, pos.y, pos.z);
			ForceRefToAlias(script, aliasID, handleptr.get());
			RE::NiPointer<RE::TESObjectREFR> filled;
			RE::LookupReferenceByHandle(script->GetAliasedRef(aliasID).native_handle(), filled);
			logger::info("ForceDestinationMarkerIntoAliasID: alias {} now holds {:08X}", aliasID, filled ? filled->GetFormID() : 0);
		}

		bool IsCustomDestinationActive(RE::StaticFunctionTag*)
		{
			const bool active = static_cast<bool>(GetPlayerCustomDestinationRef());
			logger::info("IsCustomDestinationActive: {}", active);
			return active;
		}



		// Using the GetMount new to NG
		RE::Actor* GetCurrentMount(RE::StaticFunctionTag*, RE::Actor* a)
		{
			RE::ActorPtr mount_ptr;
			if (!a->GetMount(mount_ptr))
				return nullptr;
			return mount_ptr.get();
		}


		// A faithful recreation of the original AutoMove version of GetCurrentMount.
		RE::Actor* GetCurrentMount_Original(RE::StaticFunctionTag*, RE::Actor* a)
		{
			RE::ExtraInteraction* extdata = a->extraList.GetByType<RE::ExtraInteraction>();
			RE::NiPointer<RE::TESObjectREFR> handleptr;
			RE::LookupReferenceByHandle(extdata->interaction->actor.native_handle(), handleptr);
			if (!handleptr)
				return nullptr;
			return handleptr->As<RE::Actor>();
		}

		bool RegisterFuncs(RE::BSScript::IVirtualMachine* a_vm)
		{
			a_vm->RegisterFunction("ForceDestinationMarkerIntoAliasID", "AutoMove", ForceDestinationMarkerIntoAliasID);
			a_vm->RegisterFunction("IsCustomDestinationActive", "AutoMove", IsCustomDestinationActive);
			a_vm->RegisterFunction("GetCurrentMount", "AutoMove", GetCurrentMount_Original);

			Events::RegisterPapyrus(a_vm);
			return true;
		}
	}
}