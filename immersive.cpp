#include "immersivepch.h"
#include "immersive.h"

#include "SharedDefines.h"
#include "ImmersiveConfig.h"


#ifdef ENABLE_PLAYERBOTS
#include "../Bots/playerbot/ServerFacade.h"
#include "../Bots/playerbot/PlayerbotAIConfig.h"
#include "../Bots/playerbot/PlayerbotAI.h"
#include "../Bots/playerbot/ChatHelper.h"
#endif

using namespace immersive;

map<Stats, string> Immersive::statValues;
map<Stats, string> Immersive::statNames;

string formatMoney(uint32 copper)
{
    ostringstream out;
    if (!copper)
    {
        out << "0";
        return out.str();
    }

    uint32 gold = uint32(copper / 10000);
    copper -= (gold * 10000);
    uint32 silver = uint32(copper / 100);
    copper -= (silver * 100);

    bool space = false;
    if (gold > 0)
    {
        out << gold <<  "g";
        space = true;
    }

    if (silver > 0 && gold < 50)
    {
        if (space) out << " ";
        out << silver <<  "s";
        space = true;
    }

    if (copper > 0 && gold < 10)
    {
        if (space) out << " ";
        out << copper <<  "c";
    }

    return out.str();
}
Immersive::Immersive()
{
    statValues[STAT_STRENGTH] = "Strength";
    statValues[STAT_AGILITY] = "Agility";
    statValues[STAT_STAMINA] = "Stamina";
    statValues[STAT_INTELLECT] = "Intellect";
    statValues[STAT_SPIRIT] = "Spirit";

    statNames[STAT_STRENGTH] = "STR";
    statNames[STAT_AGILITY] = "AGI";
    statNames[STAT_STAMINA] = "STA";
    statNames[STAT_INTELLECT] = "INT";
    statNames[STAT_SPIRIT] = "SPI";
}

PlayerInfo extraPlayerInfo[MAX_RACES][MAX_CLASSES];

PlayerInfo const* Immersive::GetPlayerInfo(uint32 race, uint32 class_)
{
    if (class_ == CLASS_SHAMAN && race == RACE_NIGHTELF)
    {
        PlayerInfo const* piSh = sObjectMgr.GetPlayerInfo(RACE_DRAENEI, class_);
        PlayerInfo *result = &extraPlayerInfo[race][class_];
        memcpy(result, piSh, sizeof(PlayerInfo));

        PlayerInfo const* piDr = sObjectMgr.GetPlayerInfo(race, CLASS_DRUID);
        result->displayId_f = piDr->displayId_f;
        result->displayId_m = piDr->displayId_m;

        return result;
    }

    return sObjectMgr.GetPlayerInfo(race, class_);
}

void Immersive::GetPlayerLevelInfo(Player *player, PlayerLevelInfo* info)
{
    if (!sImmersiveConfig.manualAttributes) return;

#ifdef ENABLE_PLAYERBOTS
    if (!sPlayerbotAIConfig.randomBotImmersive)
    {
        uint32 account = sObjectMgr.GetPlayerAccountIdByGUID(player->GetObjectGuid());
        if (sPlayerbotAIConfig.IsInRandomAccountList(account))
            return;
    }
#endif

    PlayerInfo const* playerInfo = GetPlayerInfo(player->getRace(), player->getClass());
    *info = playerInfo->levelInfo[0];

    uint32 owner = player->GetObjectGuid().GetRawValue();
    int modifier = GetModifierValue(owner);
    for (int i = STAT_STRENGTH; i < MAX_STATS; ++i)
    {
        info->stats[i] += GetStatsValue(owner, (Stats)i) * modifier / 100;
    }
}

void Immersive::OnGossipSelect(Player *player, uint32 gossipListId, GossipMenuItemData *menuData)
{
    switch (menuData->m_gAction_poi)
    {
    case 0: // Help
        PrintHelp(player, true, true);
        break;
    case 1:
    case 2:
    case 3:
    case 4:
    case 5:
        IncreaseStat(player, menuData->m_gAction_poi - 1);
        break;
    case 6:
        ResetStats(player);
        break;
    case 11:
    case 12:
    case 13:
    case 14:
    case 15:
    case 16:
    case 17:
    case 18:
    case 19:
    case 20:
        ChangeModifier(player, menuData->m_gAction_poi - 11);
        break;
    case 40:
        CastPortal(player);
        break;
    case 41:
        CastPortal(player, true);
        break;
    }
}

float Immersive::GetFallDamage(float zdist)
{
    return 0.0055 * zdist * zdist * sImmersiveConfig.fallDamageMultiplier;
}

void Immersive::OnDeath(Player *player)
{
    uint32 owner = player->GetObjectGuid().GetRawValue();

    map<Stats, int> loss;
    for (int j = STAT_STRENGTH; j < MAX_STATS; ++j)
        loss[(Stats)j] = 0;

    int totalLoss = 0;
    int lossPerDeath = sImmersiveConfig.attributeLossPerDeath;
    if (lossPerDeath < 1) return;

    for (int i = 0; i < lossPerDeath && totalLoss < lossPerDeath; i++)
    {
        for (int type = STAT_STRENGTH; type < MAX_STATS && totalLoss < lossPerDeath; ++type)
        {
            uint32 value = GetStatsValue(owner, (Stats)type);
            if (value)
            {
                SetStatsValue(owner, (Stats)type, value - 1);
                loss[(Stats)type]++;
                totalLoss++;
            }
        }
    }

    ostringstream out;
    out << "|cffa0a0ffYou have lost these attributes: ";
    bool first = true;
    bool used = false;
    for (int i = STAT_STRENGTH; i < MAX_STATS; ++i)
    {
        uint32 value = loss[(Stats)i];
        if (!value) continue;
        if (!first) out << ", "; else first = false;
        out << "|cffffa0a0-" << value << "|cffa0a0ff " << statNames[(Stats)i];
        used = true;
    }

    if (used)
    {
        SendMessage(player, out.str());
    }

    player->InitStatsForLevel(true);
    player->UpdateAllStats();
}

string percent(Player *player)
{
    return player->GetPlayerbotAI() ? "%" : "%%";
}

void Immersive::PrintHelp(Player *player, bool detailed, bool help)
{
    uint32 owner = player->GetObjectGuid().GetRawValue();

    uint32 usedStats = GetUsedStats(player);
    uint32 totalStats = GetTotalStats(player);
    uint32 cost = GetStatCost(player);

    SendMessage(player, "|cffa0a0ff== Attribute Points ==");
    ostringstream out;
    out << "|cffa0a0ffAvailable: |cff00ff00" << (totalStats - usedStats) <<
            "|cffa0a0ff (|cffffff00" << formatMoney(cost) << "|cffa0a0ff per use)";
    SendMessage(player, out.str());

    /*for (int8 level = 1; level <= 80; level++)
    {
        uint32 nextLvlTotalStats = GetTotalStats(player, level);
        uint32 nextLvlCost = GetStatCost(player, level, nextLvlTotalStats);
        ostringstream out;
        out << "|cffa0a0ffLevel: |cff00ff00" << (uint32)level <<
                "|cffa0a0ffAvailable: |cff00ff00" << (uint32)nextLvlTotalStats <<
                "|cffa0a0ff (|cffffff00" << formatMoney(nextLvlCost) << "|cffa0a0ff per use)";
        SendMessage(player, out.str());
    }*/

    if (detailed)
    {
        ostringstream out;
        out << "|cffa0a0ffUsed: ";
        bool first = true;
        bool used = false;
        uint32 modifier = GetModifierValue(owner);
        for (int i = STAT_STRENGTH; i < MAX_STATS; ++i)
        {
            uint32 value = GetStatsValue(owner, (Stats)i) * modifier / 100;
            if (!value) continue;
            if (!first) out << ", "; else first = false;
            out << "|cff00ff00+" << value << "|cffa0a0ff " << statNames[(Stats)i];
            used = true;
        }

        if (modifier != 100) out << " (|cff00ff00" << modifier << percent(player) << "|cffa0a0ff modifier)";
        if (used)
            SendMessage(player, out.str().c_str());
    }

    if (help)
    {
        ostringstream out;
        out << "|cffa0a0ffDefault: ";
        PlayerInfo const* info = GetPlayerInfo(player->getRace(), player->getClass());
        PlayerLevelInfo level1Info = info->levelInfo[0];
        uint8 level = player->getLevel();
        PlayerLevelInfo levelCInfo = info->levelInfo[level - 1];
        bool first = true;
        bool used = false;
        for (int i = STAT_STRENGTH; i < MAX_STATS; ++i)
        {
            uint32 value = ((int)levelCInfo.stats[i] - (int)level1Info.stats[i]);
            value = (uint32)floor(value * sImmersiveConfig.manualAttributesPercent / 100.0f);
            if (!value) continue;
            if (!first) out << ", "; else first = false;
            out << "|cff00ff00+" << value << "|cffa0a0ff " << statNames[(Stats)i];
            used = true;
        }
        if (used)
            SendMessage(player, out.str().c_str());
    }
}

void Immersive::ChangeModifier(Player *player, uint32 type)
{
    uint32 owner = player->GetObjectGuid().GetRawValue();
    uint32 value = type * 10;
    SetValue(owner, "modifier", value);

    ostringstream out;
    out << "|cffa0a0ffYou have changed your attribute modifier to |cff00ff00" << value << percent(player) << "|cffa0a0ff";
    if (!value || value == 100) out << " (disabled)";
    SendMessage(player, out.str());

    player->InitStatsForLevel(true);
    player->UpdateAllStats();
}

void Immersive::IncreaseStat(Player *player, uint32 type)
{
    uint32 owner = player->GetObjectGuid().GetRawValue();

    uint32 usedStats = GetUsedStats(player);
    uint32 totalStats = GetTotalStats(player);
    uint32 cost = GetStatCost(player);

    if (usedStats >= totalStats)
    {
        SendMessage(player, "|cffffa0a0You have no attribute points left");
        return;
    }

    if (player->GetMoney() < cost)
    {
        SendMessage(player, "|cffffa0a0You have not enough gold");
        return;
    }

    uint32 value = GetStatsValue(owner, (Stats)type);
    SetStatsValue(owner, (Stats)type, value + sImmersiveConfig.manualAttributesIncrease);

    usedStats = GetUsedStats(player);
    totalStats = GetTotalStats(player);
    uint32 nextCost = GetStatCost(player);
    ostringstream out;
    out << "|cffa0a0ffYou have gained |cff00ff00+" << sImmersiveConfig.manualAttributesIncrease << "|cffa0a0ff " << statNames[(Stats)type].c_str() <<
            ", |cff00ff00" << (totalStats - usedStats) << "|cffa0a0ff points left (|cffffff00" << formatMoney(nextCost) << "|cffa0a0ff per use)";
    SendMessage(player, out.str());

    player->InitStatsForLevel(true);
    player->UpdateAllStats();
    player->ModifyMoney(-(int32)cost);
    player->SaveGoldToDB();
}

void Immersive::ResetStats(Player *player)
{
    uint32 owner = player->GetObjectGuid().GetRawValue();

    for (int i = STAT_STRENGTH; i < MAX_STATS; ++i)
        SetValue(owner, statValues[(Stats)i], 0);

    uint32 usedStats = GetUsedStats(player);
    uint32 totalStats = GetTotalStats(player);
    uint32 cost = GetStatCost(player);
    ostringstream out;
    out << "|cffa0a0ffYour attributes have been reset, |cff00ff00" << (totalStats - usedStats) <<
            "|cffa0a0ff points available (|cffffff00" << formatMoney(cost) << "|cffa0a0ff per use)";
    SendMessage(player, out.str());
    player->InitStatsForLevel(true);
    player->UpdateAllStats();
}

uint32 Immersive::GetTotalStats(Player *player, uint8 level)
{
    PlayerInfo const* info = GetPlayerInfo(player->getRace(), player->getClass());
    PlayerLevelInfo level1Info = info->levelInfo[0];
    if (!level) level = player->getLevel();
    PlayerLevelInfo levelCInfo = info->levelInfo[level - 1];
    int total = 0;
    for (int i = STAT_STRENGTH; i < MAX_STATS; ++i)
    {
        total += ((int)levelCInfo.stats[i] - (int)level1Info.stats[i]);
    }
    uint32 byPercent = (uint32)floor(total * sImmersiveConfig.manualAttributesPercent / 100.0f);
    return byPercent / sImmersiveConfig.manualAttributesIncrease * sImmersiveConfig.manualAttributesIncrease;
}

uint32 Immersive::GetUsedStats(Player *player)
{
    uint32 owner = player->GetObjectGuid().GetRawValue();

    uint32 usedStats = 0;
    for (int i = STAT_STRENGTH; i < MAX_STATS; ++i)
        usedStats += GetStatsValue(owner, (Stats)i);

    return usedStats;
}

uint32 Immersive::GetStatCost(Player *player, uint8 level, uint32 usedStats)
{
    if (!level) level = player->getLevel();
    if (!usedStats) usedStats = GetUsedStats(player);
    uint32 usedLevels = usedStats / 5;
    for (int8 i = level; i >= 1; i--)
    {
        uint32 forLevel = GetTotalStats(player, i);
        if (usedStats > forLevel) {
            usedLevels = i - 1;
            break;
        }
    }
    return 10 * (usedLevels * usedLevels + 1) * sImmersiveConfig.manualAttributesIncrease;
}

uint32 Immersive::GetValue(uint32 owner, string type)
{
    uint32 value = valueCache[owner][type];

    if (!value)
    {
        QueryResult* results = CharacterDatabase.PQuery(
                "select `value` from immersive_values where owner = '%u' and `type` = '%s'",
                owner, type.c_str());

        if (results)
        {
            Field* fields = results->Fetch();
            value = fields[0].GetUInt32();
            valueCache[owner][type] = value;
        }

        delete results;
    }
    return value;
}

void Immersive::SetValue(uint32 owner, string type, uint32 value)
{
    valueCache[owner][type] = value;
    CharacterDatabase.DirectPExecute("delete from immersive_values where owner = '%u' and `type` = '%s'",
            owner, type.c_str());
    if (value)
    {
        CharacterDatabase.DirectPExecute(
                "insert into immersive_values (owner, `type`, `value`) values ('%u', '%s', '%u')",
                owner, type.c_str(), value);
    }
}

uint32 Immersive::GetStatsValue(uint32 owner, Stats type)
{
    return GetValue(owner, Immersive::statValues[type]);
}

void Immersive::SetStatsValue(uint32 owner, Stats type, uint32 value)
{
    SetValue(owner, Immersive::statValues[type], value);
}

uint32 Immersive::GetModifierValue(uint32 owner)
{
    int modifier = GetValue(owner, "modifier");
    if (!modifier) modifier = 100;
    return modifier;
}

void Immersive::SendMessage(Player *player, string message)
{
#ifdef ENABLE_PLAYERBOTS
    if (player->GetPlayerbotAI())
    {
        player->GetPlayerbotAI()->TellMaster(message);
        return;
    }
#endif
    ChatHandler &chat = ChatHandler(player->GetSession());
    chat.PSendSysMessage(message.c_str());
}

bool ImmersiveAction::CheckSharedPercentReqs(Player* player, Player* bot)
{
    Group *group = player->GetGroup();
    if (!group) return CheckSharedPercentReqsSingle(player, bot);

    for (GroupReference *gr = group->GetFirstMember(); gr; gr = gr->next())
    {
        Player *member = gr->getSource();
        if (CheckSharedPercentReqsSingle(member, bot)) return true;
    }
    return false;
}

bool ImmersiveAction::CheckSharedPercentReqsSingle(Player* player, Player* bot)
{
    if (sImmersiveConfig.sharedPercentMinLevel && (int)player->getLevel() < sImmersiveConfig.sharedPercentMinLevel)
        return false;

    uint8 race1 = player->getRace();
    uint8 cls1 = player->getClass();
    uint8 race2 = bot->getRace();
    uint8 cls2 = bot->getClass();

    if (sImmersiveConfig.sharedPercentGuildRestiction && player->GetGuildId() != bot->GetGuildId())
        return false;

    if (sImmersiveConfig.sharedPercentFactionRestiction && (IsAlliance(player->getRace()) ^ IsAlliance(bot->getRace())))
        return false;

    if (sImmersiveConfig.sharedPercentRaceRestiction == 2)
    {
        if (race1 == RACE_TROLL) race1 = RACE_ORC;
        if (race1 == RACE_DWARF) race1 = RACE_GNOME;

        if (race2 == RACE_TROLL) race2 = RACE_ORC;
        if (race2 == RACE_DWARF) race2 = RACE_GNOME;
    }

    if (sImmersiveConfig.sharedPercentClassRestiction == 2)
    {
        if (cls1 == CLASS_PALADIN || cls1 == CLASS_SHAMAN) cls1 = CLASS_WARRIOR;
        if (cls1 == CLASS_HUNTER || cls1 == CLASS_ROGUE) cls1 = CLASS_DRUID;
        if (cls1 == CLASS_PRIEST || cls1 == CLASS_WARLOCK) cls1 = CLASS_MAGE;

        if (cls2 == CLASS_PALADIN || cls2 == CLASS_SHAMAN) cls2 = CLASS_WARRIOR;
        if (cls2 == CLASS_HUNTER || cls2 == CLASS_ROGUE) cls2 = CLASS_DRUID;
        if (cls2 == CLASS_PRIEST || cls2 == CLASS_WARLOCK) cls2 = CLASS_MAGE;
    }

    if (sImmersiveConfig.sharedPercentRaceRestiction && race1 != race2)
        return false;

    if (sImmersiveConfig.sharedPercentClassRestiction && cls1 != cls2)
        return false;

    return true;
}

void Immersive::RunAction(Player* player, ImmersiveAction* action)
{
    bool first = true, needMsg = false;
    ostringstream out; out << "|cffa0a0ff";
#ifdef ENABLE_PLAYERBOTS
    for (PlayerBotMap::const_iterator i = player->GetPlayerbotMgr()->GetPlayerBotsBegin(); i != player->GetPlayerbotMgr()->GetPlayerBotsEnd(); ++i)
    {
        Player *bot = i->second;
        if (bot->GetGroup() && bot->GetGroup() == player->GetGroup()) continue;
        if (action->Run(player, bot))
        {
            if (!first)  out << ", "; else first = false;
            out << bot->GetName();
            needMsg = true;
        }
    }
#endif

    if (!needMsg) return;
    out << "|cffa0a0ff: " << action->GetMessage();
    SendMessage(player, out.str());
}

uint32 ApplyRandomPercent(uint32 value)
{
    if (!sImmersiveConfig.sharedRandomPercent) return value;

    float percent = (float) urand(0, sImmersiveConfig.sharedRandomPercent) - (float) sImmersiveConfig.sharedRandomPercent / 2;
    return value + (uint32) (value * percent / 100.0f);
}

class OnGiveXPAction : public ImmersiveAction
{
public:
    OnGiveXPAction(int32 value) : ImmersiveAction(), value(value) {}

    virtual bool Run(Player* player, Player* bot)
    {
        if ((int)player->getLevel() - (int)bot->getLevel() < sImmersiveConfig.sharedXpPercentLevelDiff)
            return false;

        if (!CheckSharedPercentReqs(player, bot))
            return false;

        bot->GiveXP(ApplyRandomPercent(value), NULL);
        Pet *pet = bot->GetPet();
        if (pet && pet->getPetType() == HUNTER_PET)
            pet->GivePetXP(ApplyRandomPercent(value));
        return true;
    }

    virtual string GetMessage()
    {
        ostringstream out;
        out << value << " experience gained";
        return out.str();
    }

private:
    int32 value;
};

void Immersive::OnGiveXP(Player *player, uint32 xp, Unit* victim)
{
    if (sImmersiveConfig.sharedXpPercent < 0.01f || !player->GetPlayerbotMgr()) return;

    uint32 bonus_xp = xp + (victim ? player->GetXPRestBonus(xp) : 0);
    uint32 botXp = (uint32) (bonus_xp * sImmersiveConfig.sharedXpPercent / 100.0f);
    if (botXp < 1) return;

    OnGiveXPAction action(botXp);
    RunAction(player, &action);
}

class OnGiveMoneyAction : public ImmersiveAction
{
public:
    OnGiveMoneyAction(int32 value) : ImmersiveAction(), value(value) {}

    virtual bool Run(Player* player, Player* bot)
    {
        if ((int)player->getLevel() - (int)bot->getLevel() < sImmersiveConfig.sharedXpPercentLevelDiff)
            return false;

        if (!CheckSharedPercentReqs(player, bot))
            return false;

        bot->ModifyMoney(ApplyRandomPercent(value));
        return true;
    }

    virtual string GetMessage()
    {
        ostringstream out;
        out <<
#ifdef ENABLE_PLAYERBOTS
            ai::ChatHelper::formatMoney(value)
#else
            value << "c"
#endif
            << " gained";
        return out.str();
    }

private:
    int32 value;
};

void Immersive::OnModifyMoney(Player *player, int32 delta)
{
    if (delta < 1) return;
    if (sImmersiveConfig.sharedMoneyPercent < 0.01f || !player->GetPlayerbotMgr()) return;

    int32 botMoney = (int32) (delta * sImmersiveConfig.sharedMoneyPercent / 100.0f);
    if (botMoney < 1) return;

    OnGiveMoneyAction action(botMoney);
    RunAction(player, &action);
}

class OnReputationChangeAction : public ImmersiveAction
{
public:
    OnReputationChangeAction(FactionEntry const* factionEntry, int32 value) : ImmersiveAction(), factionEntry(factionEntry), value(value) {}

    virtual bool Run(Player* player, Player* bot)
    {
        if (!CheckSharedPercentReqs(player, bot))
            return false;

        bot->GetReputationMgr().ModifyReputation(factionEntry, ApplyRandomPercent(value));
        return true;
    }

    virtual string GetMessage()
    {
        ostringstream out;
        out << value << " reputation gained";
        return out.str();
    }

private:
    FactionEntry const* factionEntry;
    int32 value;
};

void Immersive::OnReputationChange(Player* player, FactionEntry const* factionEntry, int32& standing, bool incremental)
{
    if (sImmersiveConfig.sharedRepPercent < 0.01f || !player->GetPlayerbotMgr() || !incremental) return;

    int32 value = (uint32) (standing * sImmersiveConfig.sharedRepPercent / 100.0f);
    if (value < 1) return;

    OnReputationChangeAction action(factionEntry, value);
    RunAction(player, &action);
}

class OnRewardQuestAction : public ImmersiveAction
{
public:
    OnRewardQuestAction(Quest const* quest) : ImmersiveAction(), quest(quest) {}

    virtual bool Run(Player* player, Player* bot)
    {
        if (quest->GetRequiredClasses())
            return false;

        if (!CheckSharedPercentReqs(player, bot))
            return false;

        uint32 questId = quest->GetQuestId();
        if (bot->GetQuestStatus(questId) != QUEST_STATUS_NONE)
            return false;

        bot->SetQuestStatus(questId, QUEST_STATUS_COMPLETE);
        QuestStatusData& sd = bot->getQuestStatusMap()[questId];
        sd.m_explored = true;
        sd.m_rewarded = true;
        sd.uState = (sd.uState != QUEST_NEW) ? QUEST_CHANGED : QUEST_NEW;
        return true;
    }

    virtual string GetMessage()
    {
        ostringstream out;
        out << quest->GetTitle().c_str() << " completed";
        return out.str();
    }

private:
    Quest const* quest;
};

void Immersive::OnRewardQuest(Player* player, Quest const* quest)
{
    if (!sImmersiveConfig.sharedQuests || !player->GetPlayerbotMgr()) return;
    if (!quest || quest->IsRepeatable()) return;

    OnRewardQuestAction action(quest);
    RunAction(player, &action);
}

bool Immersive::OnFishing(Player* player, bool success)
{
    if (!success || !sImmersiveConfig.fishingBaubles || !player->GetPlayerbotMgr()) return success;

    Item* const item = player->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_MAINHAND);
    if (!item) return false;

    uint32 eId = item->GetEnchantmentId(TEMP_ENCHANTMENT_SLOT);
    uint32 eDuration = item->GetEnchantmentDuration(TEMP_ENCHANTMENT_SLOT);
    if (!eDuration) return false;

    SpellItemEnchantmentEntry const* pEnchant = sSpellItemEnchantmentStore.LookupEntry(eId);
    if (!pEnchant)
        return false;

    for (int s = 0; s < 3; ++s)
    {
        uint32 spellId = pEnchant->spellid[s];
        if (pEnchant->type[s] == ITEM_ENCHANTMENT_TYPE_EQUIP_SPELL && spellId)
        {
            SpellEntry const *entry = sServerFacade.LookupSpellInfo(spellId);
            if (entry)
            {
                for (int i = 0; i < MAX_EFFECT_INDEX; ++i)
                {
                    if (entry->Effect[i] == SPELL_EFFECT_APPLY_AURA && entry->EffectMiscValue[i] == SKILL_FISHING)
                        return true;
                }
            }
        }
    }

    return false;
}

int32 Immersive::CalculateEffectiveChance(int32 difference, const Unit* attacker, const Unit* victim, ImmersiveEffectiveChance type)
{
    if (!sImmersiveConfig.manualAttributes) return 0;

    int32 attackerDelta = CalculateEffectiveChanceDelta(attacker);
    int32 victimDelta = CalculateEffectiveChanceDelta(victim);

    int32 multiplier = 5;
    if (type == IMMERSIVE_EFFECTIVE_CHANCE_SPELL_MISS || type == IMMERSIVE_EFFECTIVE_ATTACK_DISTANCE)
        multiplier = 1;

    switch (type)
    {
    case IMMERSIVE_EFFECTIVE_ATTACK_DISTANCE:
        // victim level - attacker level
        return - victimDelta * multiplier
                + attackerDelta * multiplier;
        break;
    case IMMERSIVE_EFFECTIVE_CHANCE_MISS:
    case IMMERSIVE_EFFECTIVE_CHANCE_SPELL_MISS:
        // victim defense - attacker offense
        return - victimDelta * multiplier
                + attackerDelta * multiplier;
    case IMMERSIVE_EFFECTIVE_CHANCE_DODGE:
    case IMMERSIVE_EFFECTIVE_CHANCE_PARRY:
    case IMMERSIVE_EFFECTIVE_CHANCE_BLOCK:
        // attacker defense - victim offense
        return - attackerDelta * multiplier
                + victimDelta * multiplier;
    case IMMERSIVE_EFFECTIVE_CHANCE_CRIT:
        // attacker offence - victim defense
        return - attackerDelta * multiplier
                + victimDelta * multiplier;
    }

    return 0;
}

uint32 Immersive::CalculateEffectiveChanceDelta(const Unit* unit)
{
    if (unit->GetObjectGuid().IsPlayer())
    {
        int modifier = GetModifierValue(unit->GetObjectGuid().GetCounter());
#ifdef ENABLE_PLAYERBOTS
        if (sPlayerbotAIConfig.IsInRandomAccountList(sObjectMgr.GetPlayerAccountIdByGUID(unit->GetObjectGuid())))
            return 0;
#endif
        return unit->getLevel() * (100 - modifier) / 100;
    }

    return 0;
}

void Immersive::CastPortal(Player *player, bool meetingStone)
{
    Group *group = player->GetGroup();
    if (!group)
    {
        SendMessage(player, "|cffffa0a0You are not in a group");
        return;
    }

    uint32 spellId =
#ifdef MANGOSBOT_ZERO
            23598; // meeting stone summoning
#endif
#ifdef MANGOSBOT_ONE
            23598; // meeting stone summoning
#endif
#ifdef MANGOSBOT_TWO
            61994; // meeting stone summoning
#endif
    uint32 reagent = 17032; // rune of portals

    if (!meetingStone)
    {
        if (!player->HasItemCount(reagent, 1))
        {
            SendMessage(player, "|cffffa0a0You do not have any runes of portals");
            return;
        }

        player->DestroyItemCount(reagent, 1, true);
    }

    player->CastSpell(player, spellId, false);
}

void Immersive::OnGoUse(Player *player, GameObject* obj)
{
    if (obj && obj->GetGoType() == GAMEOBJECT_TYPE_MEETINGSTONE)
    {
        CastPortal(player, true);
    }
}

void Immersive::OnGossipHello(Player* player, Creature* creature)
{
#if MAX_EXPANSION == 1
    GossipMenu& menu = player->PlayerTalkClass->GetGossipMenu();
    if (creature)
    {
        uint32 textId = player->GetGossipTextId(menu.GetMenuId(), creature);
        GossipText const* text = sObjectMgr.GetGossipText(textId);
        if (text)
        {
            for (int i = 0; i < MAX_GOSSIP_TEXT_OPTIONS; i++)
            {
                string text0 = text->Options[i].Text_0;
                if (!text0.empty()) creature->MonsterSay(text0.c_str(), 0, player);
                string text1 = text->Options[i].Text_1;
                if (!text1.empty() && text0 != text1) creature->MonsterSay(text1.c_str(), 0, player);
            }
        }
    }

#endif
}

map<uint8,float> scale;
void Immersive::CheckScaleChange(Player* player)
{
    if (!sImmersiveConfig.scaleModifierWorkaround) return;

    uint8 race = player->getRace();
    if (scale.empty())
    {
        scale[RACE_TROLL] = 0.85f;
        scale[RACE_NIGHTELF] = 0.85f;
        scale[RACE_ORC] = 0.95f;
        scale[RACE_TAUREN] = 0.95f;
        scale[RACE_HUMAN] = 1.0f;
        scale[RACE_DWARF] = 0.85f;
        scale[RACE_GNOME] = 1.15f;
    }

    if (scale.find(race) != scale.end())
    {
        player->SetObjectScale(scale[race] - (player->getGender() == GENDER_MALE ? 0.1f : 0));
    }
}

INSTANTIATE_SINGLETON_1( immersive::Immersive );
