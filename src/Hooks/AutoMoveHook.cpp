#include "../PCH.h"
#include "Hooks.h"
#include "../Papyrus.h"
#include "../CustomDestination.h"


namespace AutoMove
{
	namespace Hooks
	{
		// AutoMoveHandler::CanProcess
		// SSE 1.5.97 ID: 263061

		namespace
		{
			bool AutoMove_CanProcessHook(RE::AutoMoveHandler* thisptr, RE::InputEvent* evt);
			static REL::Relocation<decltype(AutoMove_CanProcessHook)> oAutoMove_CanProcess;
			bool AutoMove_CanProcessHook(RE::AutoMoveHandler* thisptr, RE::InputEvent* evt)
			{
				const bool vanilla = oAutoMove_CanProcess(thisptr, evt);
				const bool hasMarker = static_cast<bool>(GetPlayerCustomDestinationRef());
				// Log only the key events that the game would handle, to keep the log short.
				if (vanilla)
					logger::info("AutoMove key: custom marker {}, vanilla auto move {}", hasMarker, hasMarker ? "blocked" : "allowed");
				return vanilla && !hasMarker;
			}
		}


		void InstallAutoMoveCanProcessHook()
		{
			static REL::Relocation<std::uintptr_t> AutoMoveHandlerVTBL{ RE::VTABLE_AutoMoveHandler[0] };
			oAutoMove_CanProcess = AutoMoveHandlerVTBL.write_vfunc(1, AutoMove_CanProcessHook);
		}
	}
}