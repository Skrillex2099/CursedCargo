using UnrealBuildTool;

public class CursedCargo : ModuleRules
{
    public CursedCargo(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

        // The prototype keeps feature folders directly under the module root.
        // Expose that root so includes such as "Items/CCCollectibleItem.h"
        // resolve consistently in both handwritten and generated sources.
        PublicIncludePaths.Add(ModuleDirectory);

        PublicDependencyModuleNames.AddRange(new[]
        {
            "Core",
            "CoreUObject",
            "Engine",
            "InputCore"
        });
    }
}
