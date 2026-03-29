def can_build(env, platform):
    return True


def configure(env):
    env.Append(CPPDEFINES=["ABILITY_SYSTEM_MODULE"])


def get_doc_classes():
    return [
        "AbilitySystem",
        "ASAbility",
        "ASAbilityPhase",
        "ASAbilitySpec",
        "ASAttribute",
        "ASAttributeSet",
        "ASBridgeState",
        "ASComponent",
        "ASContainer",
        "ASCue",
        "ASCueAnimation",
        "ASCueAudio",
        "ASCueSpec",
        "ASDelivery",
        "ASEditorPlugin",
        "ASEditorPropertyName",
        "ASEditorPropertySelector",
        "ASEditorPropertyTagSelector",
        "ASEffect",
        "ASEffectSpec",
        "ASInspectorPlugin",
        "ASPackage",
        "ASStateCache",
        "ASStateSnapshot",
        "ASStateUtils",
        "ASTagSpec",
        "ASTagsPanel",
        "ASTagUtils",
        "BTActionAS_ActivateAbility",
        "BTActionAS_DispatchEvent",
        "BTActionAS_WaitForEvent",
        "BTConditionAS_CanActivate",
        "BTConditionAS_EventOccurred",
        "BTConditionAS_HasTag",
    ]


def get_doc_path():
    return "doc_classes"


def get_icon_path():
    return "icons"
