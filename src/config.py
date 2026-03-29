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
        "ASAttributeValue",
        "ASBridgeState",
        "ASComponent",
        "ASComponentState",
        "ASConditionalTag",
        "ASConditionalTagHistoricalEntry",
        "ASContainer",
        "ASCooldownData",
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
        "ASEffectModifier",
        "ASEffectModifierData",
        "ASEffectRequirement",
        "ASEffectSpec",
        "ASEffectState",
        "ASEventTag",
        "ASEventTagData",
        "ASEventTagHistoricalEntry",
        "ASInspectorPlugin",
        "ASNameTag",
        "ASNameTagHistoricalEntry",
        "ASPackage",
        "ASStateCache",
        "ASStateCacheEntry",
        "ASStateSnapshot",
        "ASStateUtils",
        "ASTagBase",
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
