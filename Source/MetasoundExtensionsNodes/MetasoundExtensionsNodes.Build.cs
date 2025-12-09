using UnrealBuildTool;

public class MetasoundExtensionsNodes : ModuleRules
{
	public MetasoundExtensionsNodes(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(
			new string[]
			{
				"AudioExtensions",
				"Core",
				"MetasoundFrontend",
				"MetasoundStandardNodes",
				"Serialization",
				"SignalProcessing",
			}
		);

		PrivateDependencyModuleNames.AddRange(
			new string[]
			{
				"CoreUObject",
				"MetasoundGraphCore",
			}
		);

		NumIncludedBytesPerUnityCPPOverride = 120 * 1024;

		PrivateDefinitions.AddRange(
			new string[]
			{
				"METASOUND_PLUGIN=Metasound",
				"METASOUND_MODULE=MetasoundExtensionNodes"
			});
	}
}