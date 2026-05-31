using UnrealBuildTool;

public class DudeWalksEditorTarget : TargetRules
{
    public DudeWalksEditorTarget(TargetInfo Target) : base(Target)
    {
        Type = TargetType.Editor;
        DefaultBuildSettings = BuildSettingsVersion.V5;
        IncludeOrderVersion = EngineIncludeOrderVersion.Unreal5_5;
        ExtraModuleNames.Add("DudeWalks");
    }
}
