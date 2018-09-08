#include "immersivepch.h"
#include "immersive.h"
#include "SharedDefines.h"
#include "ImmersiveConfig.h"

#ifdef ENABLE_PLAYERBOTS
#include "../Bots/playerbot/PlayerbotAIConfig.h"
#include "../Bots/playerbot/PlayerbotAI.h"
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

    sImmersiveConfig.Initialize();
}

void Immersive::GetPlayerLevelInfo(Player *player, PlayerLevelInfo* info)
{
    if (!sImmersiveConfig.manualAttributes) return;

#ifdef ENABLE_PLAYERBOTS
    uint32 account = sObjectMgr.GetPlayerAccountIdByGUID(player->GetObjectGuid());
    if (sPlayerbotAIConfig.IsInRandomAccountList(account))
        return;
#endif

    PlayerInfo const* playerInfo = sObjectMgr.GetPlayerInfo(player->getRace(), player->getClass());
    *info = playerInfo->levelInfo[0];

    uint32 owner = player->GetObjectGuid().GetRawValue();
    for (int i = STAT_STRENGTH; i < MAX_STATS; ++i)
        info->stats[i] += GetValue(owner, Immersive::statValues[(Stats)i]);
}

void Immersive::OnGossipSelect(Player *player, uint32 gossipListId, GossipMenuItemData *menuData)
{
    switch (menuData->m_gAction_poi)
    {
    case 0: // Help
        PrintHelp(player, true);
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
    }
}

float Immersive::GetFallDamage(float zdist)
{
    return 0.0075 * zdist * zdist;
}

void Immersive::OnDeath(Player *player)
{
    uint32 owner = player->GetObjectGuid().GetRawValue();

    map<Stats, int> loss;
    for (int j = STAT_STRENGTH; j < MAX_STATS; ++j)
        loss[(Stats)j] = 0;

    int totalLoss = 0;
    int lossPerDeath = 3;
    for (uint32 i = 0; i < lossPerDeath && totalLoss < lossPerDeath; i++)
    {
        for (int type = STAT_STRENGTH; type < MAX_STATS && totalLoss < lossPerDeath; ++type)
        {
            uint32 value = GetValue(owner, statValues[(Stats)type]);
            if (value)
            {
                SetValue(owner, statValues[(Stats)type], value - 1);
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

void Immersive::PrintHelp(Player *player, bool detailed)
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

    if (detailed)
    {
        ostringstream out;
        out << "|cffa0a0ffUsed: ";
        bool first = true;
        bool used = false;
        for (int i = STAT_STRENGTH; i < MAX_STATS; ++i)
        {
            uint32 value = GetValue(owner, statValues[(Stats)i]);
            if (!value) continue;
            if (!first) out << ", "; else first = false;
            out << "|cff00ff00+" << value << "|cffa0a0ff " << statNames[(Stats)i];
            used = true;
        }
        if (used)
            SendMessage(player, out.str().c_str());
    }
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

    uint32 value = GetValue(owner, statValues[(Stats)type]);
    SetValue(owner, statValues[(Stats)type], value + 1);

    usedStats = GetUsedStats(player);
    totalStats = GetTotalStats(player);
    cost = GetStatCost(player);
    ostringstream out;
    out << "|cffa0a0ffYou have gained |cff00ff00+1|cffa0a0ff " << statNames[(Stats)type].c_str() <<
            ", |cff00ff00" << (totalStats - usedStats) << "|cffa0a0ff points left (|cffffff00" << formatMoney(cost) << "|cffa0a0ff per use)";
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

uint32 Immersive::GetTotalStats(Player *player)
{
    return (player->getLevel() - 1) * 3;
}

uint32 Immersive::GetUsedStats(Player *player)
{
    uint32 owner = player->GetObjectGuid().GetRawValue();

    uint32 usedStats = 0;
    for (int i = STAT_STRENGTH; i < MAX_STATS; ++i)
        usedStats += GetValue(owner, statValues[(Stats)i]);

    return usedStats;
}

uint32 Immersive::GetStatCost(Player *player)
{
    uint32 usedLevels = GetUsedStats(player) / 3;
    return 10 * (usedLevels * usedLevels + 1);
}

uint32 Immersive::GetValue(uint32 owner, string type)
{
    uint32 value = 0;

    QueryResult* results = CharacterDatabase.PQuery(
            "select `value` from immersive_values where owner = '%u' and `type` = '%s'",
            owner, type.c_str());

    if (results)
    {
        Field* fields = results->Fetch();
        value = fields[0].GetUInt32();
    }

    delete results;
    return value;
}

uint32 Immersive::SetValue(uint32 owner, string type, uint32 value)
{
    CharacterDatabase.DirectPExecute("delete from immersive_values where owner = '%u' and `type` = '%s'",
            owner, type.c_str());
    if (value)
    {
        CharacterDatabase.DirectPExecute(
                "insert into immersive_values (owner, `type`, `value`) values ('%u', '%s', '%u')",
                owner, type.c_str(), value);
    }

    return value;
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


void Immersive::RunAction(Player* player, ImmersiveAction* action)
{
    bool first = true, needMsg = false;
    ostringstream out; out << "|cffa0a0ff";
#ifdef ENABLE_PLAYERBOTS
    for (PlayerBotMap::const_iterator i = player->GetPlayerbotMgr()->GetPlayerBotsBegin(); i != player->GetPlayerbotMgr()->GetPlayerBotsEnd(); ++i)
    {
        Player *bot = i->second;
        if (!bot->GetGroup() && action->Run(player, bot))
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

class OnGiveXPAction : public ImmersiveAction
{
public:
    OnGiveXPAction(int32 value) : ImmersiveAction(), value(value) {}

    virtual bool Run(Player* player, Player* bot)
    {
        if ((int)player->getLevel() - (int)bot->getLevel() > 1)
        {
            bot->GiveXP(value, NULL);
            Pet *pet = bot->GetPet();
            if (pet && pet->getPetType() == HUNTER_PET)
                pet->GivePetXP(value);
            return true;
        }
        return false;
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

class OnReputationChangeAction : public ImmersiveAction
{
public:
    OnReputationChangeAction(FactionEntry const* factionEntry, int32 value) : ImmersiveAction(), factionEntry(factionEntry), value(value) {}

    virtual bool Run(Player* player, Player* bot)
    {
        bot->GetReputationMgr().ModifyReputation(factionEntry, value);
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

INSTANTIATE_SINGLETON_1( immersive::Immersive );
