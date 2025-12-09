// Copyright Hitbox Games, LLC. All Rights Reserved.

#include "MetasoundExtensionsNodes.h"
#include "Runtime/Launch/Resources/Version.h"

#if ENGINE_MAJOR_VERSION >= 5 && ENGINE_MINOR_VERSION >= 8
#include "MetasoundFrontendModuleRegistrationMacros.h"
#else
#include "MetasoundFrontendRegistryContainer.h"
#endif

#define LOCTEXT_NAMESPACE "FMetasoundExtensionsNodesModule"

namespace Metasound
{
	void FMetasoundExtensionsNodesModule::StartupModule()
	{
#if ENGINE_MAJOR_VERSION >= 5 && ENGINE_MINOR_VERSION >= 8
		using namespace Frontend;
		METASOUND_REGISTER_ITEMS_IN_MODULE
#else
		FMetasoundFrontendRegistryContainer::Get()->RegisterPendingNodes();
#endif
	}

	void FMetasoundExtensionsNodesModule::ShutdownModule()
	{
#if ENGINE_MAJOR_VERSION >= 5 && ENGINE_MINOR_VERSION >= 8
		using namespace Frontend;
		METASOUND_UNREGISTER_ITEMS_IN_MODULE
#endif
	}
}

#undef LOCTEXT_NAMESPACE


#if ENGINE_MAJOR_VERSION >= 5 && ENGINE_MINOR_VERSION >= 8
METASOUND_IMPLEMENT_MODULE_REGISTRATION_LIST
#endif

IMPLEMENT_MODULE(Metasound::FMetasoundExtensionsNodesModule, MetasoundExtensionsNodes)
