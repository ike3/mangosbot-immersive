#pragma once
#include "GossipDef.h"

using namespace std;

namespace immersive
{
    class ImmersiveAction
    {
    public:
        ImmersiveAction() {}

    public:
        virtual bool Run(Player* player, Player* bot) = 0;
        virtual string GetMessage() = 0;
    };

    class Immersive
    {
    public:
        Immersive();

    public:
        void GetPlayerLevelInfo(Player *player, PlayerLevelInfo* info);
        void OnGossipSelect(Player *player, uint32 gossipListId, GossipMenuItemData *menuData);
        float GetFallDamage(float zdist);
        void OnDeath(Player *player);
        void OnGiveXP(Player *player, uint32 xp, Unit* victim);
        void OnReputationChange(Player* player, FactionEntry const* factionEntry, int32& standing, bool incremental);
        void OnRewardQuest(Player* player, Quest const* quest);

    private:
        void PrintHelp(Player *player, bool detailed = false);
        void IncreaseStat(Player *player, uint32 type);
        void ResetStats(Player *player);
        void SendMessage(Player *player, string message);

    private:
        uint32 GetTotalStats(Player *player);
        uint32 GetUsedStats(Player *player);
        uint32 GetStatCost(Player *player);

    private:
        void RunAction(Player* player, ImmersiveAction* action);

    private:
        uint32 GetValue(uint32 owner, string type);
        uint32 SetValue(uint32 owner, string type, uint32 value);

    private:
        static map<Stats, string> statValues;
        static map<Stats, string> statNames;
    };
}


#define sImmersive MaNGOS::Singleton<immersive::Immersive>::Instance()
