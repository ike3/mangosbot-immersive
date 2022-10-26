#include "immersivepch.h"
#include "SystemConfig.h"
#include "ImmersiveConfig.h"

using namespace std;

INSTANTIATE_SINGLETON_1(ImmersiveConfig);

ImmersiveConfig::ImmersiveConfig()
{
}

bool ImmersiveConfig::Initialize()
{
    sLog.outString("Initializing Immersive by ike3");

    if (!config.SetSource(SYSCONFDIR"immersive.conf"))
    {
        sLog.outString("Immersive is Disabled. Unable to open configuration file immersive.conf");
        return false;
    }

    serverPort = config.GetIntDefault("Immersive.ServerPort", 0);
    manualAttributes = config.GetBoolDefault("Immersive.ManualAttributes", true);
    manualAttributesPercent = config.GetFloatDefault("Immersive.ManualAttributesPercent", 80.0f);
    manualAttributesIncrease = config.GetFloatDefault("Immersive.ManualAttributesIncrease", 5);
    sharedXpPercent = config.GetFloatDefault("Immersive.SharedXpPercent", 90.0f);
    sharedRepPercent = config.GetFloatDefault("Immersive.SharedRepPercent", 90.0f);
    sharedQuests = config.GetBoolDefault("Immersive.SharedQuests", true);
    fishingBaubles = config.GetBoolDefault("Immersive.FishingBaubles", true);
    sharedPercentRaceRestiction = config.GetIntDefault("Immersive.SharedPercentRaceRestriction", 0);
    sharedPercentClassRestiction = config.GetIntDefault("Immersive.SharedPercentClassRestriction", 0);
    sharedPercentGuildRestiction = config.GetIntDefault("Immersive.SharedPercentGuildRestriction", 0);
    sharedPercentFactionRestiction = config.GetIntDefault("Immersive.SharedPercentFactionRestriction", 0);
    sharedPercentMinLevel = config.GetFloatDefault("Immersive.SharedPercentMinLevel", 0);
    attributeLossPerDeath = config.GetIntDefault("Immersive.AttributeLossPerDeath", 1);
    fallDamageMultiplier = config.GetFloatDefault("Immersive.FallDamageMultiplier", 1);
    sharedXpPercentLevelDiff = config.GetIntDefault("Immersive.SharedXpPercentLevelDiff", 0);
    scaleModifierWorkaround = config.GetBoolDefault("Immersive.ScaleModifierWorkaround", false);

    sLog.outString("Immersive configuration loaded");
    return true;
}
