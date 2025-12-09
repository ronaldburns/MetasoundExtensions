// Copyright Hitbox Games, LLC. All Rights Reserved.

#pragma once

#include "Modules/ModuleManager.h"

namespace Metasound
{
	class FMetasoundExtensionsNodesModule : public IModuleInterface
	{
	public:
		virtual void StartupModule() override;
		virtual void ShutdownModule() override;
	};
}
