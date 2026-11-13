#pragma once

namespace AutoMove
{
	namespace Events
	{
		class HKPlacePlayerMarkerCallbackFunctor : public RE::BSIntrusiveRefCounted
		{
		public:
			inline static constexpr auto VTABLE = RE::VTABLE___PlacePlayerMarkerCallbackFunctor;

			static void RunHook(RE::BSIntrusiveRefCounted* functor, std::uint8_t a_msg);

			static void InstallHook();
			static void RegisterPapyrus(RE::BSScript::IVirtualMachine* a_vm);

			static SKSE::RegistrationSet<std::uint8_t>* GetEvent();
		};

	}
}