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
    float sharedXpPercent;
    float sharedRepPercent;
    bool sharedQuests;
    bool fishingBaubles;
    int sharedPercentRaceRestiction;
    int sharedPercentClassRestiction;
    float sharedPercentMinLevel;
    int attributeLossPerDeath;
    float fallDamageMultiplier;
    int sharedXpPercentLevelDiff;

private:
    Config config;
};

#define sImmersiveConfig MaNGOS::Singleton<ImmersiveConfig>::Instance()

