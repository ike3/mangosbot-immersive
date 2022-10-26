#pragma once

#include "Config.h"

class ImmersiveConfig
{
public:
    ImmersiveConfig();
    static ImmersiveConfig& instance()
    {
        static ImmersiveConfig instance;
        return instance;
    }

public:
    bool Initialize();

    int serverPort;
    bool manualAttributes;
    float manualAttributesPercent;
    int manualAttributesIncrease;
    float sharedXpPercent;
    float sharedMoneyPercent;
    float sharedRepPercent;
    bool sharedQuests;
    bool fishingBaubles;
    int sharedPercentRaceRestiction;
    int sharedPercentClassRestiction;
    int sharedPercentGuildRestiction;
    int sharedPercentFactionRestiction;
    float sharedPercentMinLevel;
    int attributeLossPerDeath;
    float fallDamageMultiplier;
    int sharedXpPercentLevelDiff;
    bool scaleModifierWorkaround;

private:
    Config config;
};

#define sImmersiveConfig MaNGOS::Singleton<ImmersiveConfig>::Instance()

