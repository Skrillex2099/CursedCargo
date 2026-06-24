using UnrealBuildTool;
using System.Collections.Generic;

public class CursedCargoEditorTarget : TargetRules
{
    public CursedCargoEditorTarget(TargetInfo Target) : base(Target)
    {
        Type = TargetType.Editor;
        DefaultBuildSettings = BuildSettingsVersion.Latest;
        IncludeOrderVersion = EngineIncludeOrderVersion.Latest;
        ExtraModuleNames.Add("CursedCargo");
    }
}

