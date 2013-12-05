/* This file is part of the BlizzLikeCore Project. See CREDITS and LICENSE files
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

/* ScriptData
SDName: Instance_Karazhan
SD%Complete: 70
SDComment: Instance Script for Karazhan to help in various encounters.
SDCategory: Karazhan
EndScriptData */

#include "precompiled.h"
#include "karazhan.h"

/*
0  - Attumen + Midnight (optional)
1  - Moroes
2  - Maiden of Virtue (optional)
3  - Opera Event
4  - Curator
5  - Terestian Illhoof (optional)
6  - Shade of Aran (optional)
7  - Netherspite (optional)
8  - Chess Event
9  - Prince Malchezzar
10 - Nightbane
*/

instance_karazhan::instance_karazhan(Map* pMap) : ScriptedInstance(pMap),
    m_uiOperaEvent(0),
    m_uiOzDeathCount(0),
    m_uiTeam(0),
    m_uiChessResetTimer(0),
    m_uiAllianceStalkerCount(0),
    m_uiHordeStalkerCount(0),
    m_bFriendlyGame(false)
{
    Initialize();
}

void instance_karazhan::Initialize()
{
    memset(&m_auiEncounter, 0, sizeof(m_auiEncounter));
}

bool instance_karazhan::IsEncounterInProgress() const
{
    for (uint8 i = 0; i < MAX_ENCOUNTER; ++i)
    {
        if (m_auiEncounter[i] == IN_PROGRESS)
            return true;
    }

    return false;
}

void instance_karazhan::OnPlayerEnter(Player* pPlayer)
{
    if (!m_uiTeam)                                          // very first player to enter
        m_uiTeam = pPlayer->GetTeam();

    // If the opera event is already set, return
    if (GetData(TYPE_OPERA_PERFORMANCE) != 0)
        return;

    // Set the Opera Performance type on the first player enter
    SetData(TYPE_OPERA_PERFORMANCE, urand(OPERA_EVENT_WIZARD_OZ, OPERA_EVENT_ROMULO_AND_JUL));
}

void instance_karazhan::OnCreatureCreate(Creature* pCreature)
{
    switch (pCreature->GetEntry())
    {
        case NPC_ATTUMEN:
        case NPC_MIDNIGHT:
        case NPC_MOROES:
        case NPC_BARNES:
        case NPC_NIGHTBANE:
        case NPC_NETHERSPITE:
        case NPC_JULIANNE:
        case NPC_ROMULO:
        case NPC_LADY_KEIRA_BERRYBUCK:
        case NPC_LADY_CATRIONA_VON_INDI:
        case NPC_LORD_CRISPIN_FERENCE:
        case NPC_BARON_RAFE_DREUGER:
        case NPC_BARONESS_DOROTHEA_MILLSTIPE:
        case NPC_LORD_ROBIN_DARIS:
        case NPC_IMAGE_OF_MEDIVH:
        case NPC_IMAGE_OF_ARCANAGOS:
        case NPC_ECHO_MEDIVH:
        case NPC_CHESS_VICTORY_CONTROLLER:
            m_mNpcEntryGuidStore[pCreature->GetEntry()] = pCreature->GetObjectGuid();
            break;
        case NPC_NIGHTBANE_HELPER:
            if (pCreature->GetPositionZ() < 100.0f)
                m_lNightbaneGroundTriggers.push_back(pCreature->GetObjectGuid());
            else
                m_lNightbaneAirTriggers.push_back(pCreature->GetObjectGuid());
            break;
        case NPC_INVISIBLE_STALKER:
            if (pCreature->GetPositionY() < -1870.0f)
                m_lChessHordeStalkerList.push_back(pCreature->GetObjectGuid());
            else
                m_lChessAllianceStalkerList.push_back(pCreature->GetObjectGuid());
            break;
        case NPC_CHESS_STATUS_BAR:
            if (pCreature->GetPositionY() < -1870.0f)
                m_HordeStatusGuid = pCreature->GetObjectGuid();
            else
                m_AllianceStatusGuid = pCreature->GetObjectGuid();
            break;
        case NPC_HUMAN_CHARGER:
        case NPC_HUMAN_CLERIC:
        case NPC_HUMAN_CONJURER:
        case NPC_HUMAN_FOOTMAN:
        case NPC_CONJURED_WATER_ELEMENTAL:
        case NPC_KING_LLANE:
            m_lChessPiecesAlliance.push_back(pCreature->GetObjectGuid());
            break;
        case NPC_ORC_GRUNT:
        case NPC_ORC_NECROLYTE:
        case NPC_ORC_WARLOCK:
        case NPC_ORC_WOLF:
        case NPC_SUMMONED_DAEMON:
        case NPC_WARCHIEF_BLACKHAND:
            m_lChessPiecesHorde.push_back(pCreature->GetObjectGuid());
            break;
    }
}

void instance_karazhan::OnObjectCreate(GameObject* pGo)
{
    switch (pGo->GetEntry())
    {
        case GO_STAGE_DOOR_LEFT:
        case GO_STAGE_DOOR_RIGHT:
            if (m_auiEncounter[3] == DONE)
                pGo->SetGoState(GO_STATE_ACTIVE);
            break;
        case GO_GAMESMANS_HALL_EXIT_DOOR:
            if (m_auiEncounter[8] == DONE)
                pGo->SetGoState(GO_STATE_ACTIVE);
            break;
        case GO_SIDE_ENTRANCE_DOOR:
            if (m_auiEncounter[3] == DONE)
                pGo->RemoveFlag(GAMEOBJECT_FLAGS, GO_FLAG_LOCKED);
            break;
        case GO_STAGE_CURTAIN:
        case GO_PRIVATE_LIBRARY_DOOR:
        case GO_MASSIVE_DOOR:
        case GO_GAMESMANS_HALL_DOOR:
        case GO_NETHERSPACE_DOOR:
        case GO_DUST_COVERED_CHEST:
        case GO_MASTERS_TERRACE_DOOR_1:
        case GO_MASTERS_TERRACE_DOOR_2:
            break;

            // Opera event backgrounds
        case GO_OZ_BACKDROP:
        case GO_HOOD_BACKDROP:
        case GO_HOOD_HOUSE:
        case GO_RAJ_BACKDROP:
        case GO_RAJ_MOON:
        case GO_RAJ_BALCONY:
            break;
        case GO_OZ_HAY:
            m_lOperaHayGuidList.push_back(pGo->GetObjectGuid());
            return;
        case GO_HOOD_TREE:
            m_lOperaTreeGuidList.push_back(pGo->GetObjectGuid());
            return;

        default:
            return;
    }
    m_mGoEntryGuidStore[pGo->GetEntry()] = pGo->GetObjectGuid();
}

void instance_karazhan::SetData(uint32 uiType, uint32 uiData)
{
    switch (uiType)
    {
        case TYPE_ATTUMEN:
            m_auiEncounter[uiType] = uiData;
            if (uiData == FAIL)
            {
                // Respawn Midnight on Fail
                if (Creature* pMidnight = GetSingleCreatureFromStorage(NPC_MIDNIGHT))
                {
                    if (!pMidnight->isAlive())
                        pMidnight->Respawn();
                }
            }
            break;
        case TYPE_MOROES:
        case TYPE_MAIDEN:
            m_auiEncounter[uiType] = uiData;
            break;
        case TYPE_OPERA:
            // Don't store the same data twice
            if (uiData == m_auiEncounter[uiType])
                break;
            m_auiEncounter[uiType] = uiData;
            if (uiData == IN_PROGRESS)
                m_uiOzDeathCount = 0;
            if (uiData == DONE)
            {
                DoUseDoorOrButton(GO_STAGE_DOOR_LEFT);
                DoUseDoorOrButton(GO_STAGE_DOOR_RIGHT);
                DoToggleGameObjectFlags(GO_SIDE_ENTRANCE_DOOR, GO_FLAG_LOCKED, false);
            }
            // use curtain only for event start or fail
            else
                DoUseDoorOrButton(GO_STAGE_CURTAIN);
            break;
        case TYPE_CURATOR:
        case TYPE_TERESTIAN:
            m_auiEncounter[uiType] = uiData;
            break;
        case TYPE_ARAN:
            if (uiData == FAIL || uiData == DONE)
                DoToggleGameObjectFlags(GO_PRIVATE_LIBRARY_DOOR, GO_FLAG_LOCKED, false);
            if (uiData == IN_PROGRESS)
                DoToggleGameObjectFlags(GO_PRIVATE_LIBRARY_DOOR, GO_FLAG_LOCKED, true);
            m_auiEncounter[uiType] = uiData;
            break;
        case TYPE_NETHERSPITE:
            m_auiEncounter[uiType] = uiData;
            DoUseDoorOrButton(GO_MASSIVE_DOOR);
            break;
        case TYPE_CHESS:
            if (uiData == DONE)
            {
                // doors and loot are not handled for friendly games
                if (GetData(TYPE_CHESS) != SPECIAL)
                {
                    DoUseDoorOrButton(GO_GAMESMANS_HALL_EXIT_DOOR);
                    DoRespawnGameObject(GO_DUST_COVERED_CHEST, DAY);
                    DoToggleGameObjectFlags(GO_DUST_COVERED_CHEST, GO_FLAG_NO_INTERACT, false);
                }

                // cast game end spells
                if (Creature* pMedivh = GetSingleCreatureFromStorage(NPC_ECHO_MEDIVH))
                {
                    pMedivh->CastSpell(pMedivh, SPELL_FORCE_KILL_BUNNY, true);
                    pMedivh->CastSpell(pMedivh, SPELL_GAME_OVER, true);
                    pMedivh->CastSpell(pMedivh, SPELL_CLEAR_BOARD, true);
                }
                if (Creature* pController = GetSingleCreatureFromStorage(NPC_CHESS_VICTORY_CONTROLLER))
                    pController->CastSpell(pController, SPELL_VICTORY_VISUAL, true);

                // remove silence debuff
                Map::PlayerList const& players = instance->GetPlayers();
                for (Map::PlayerList::const_iterator itr = players.begin(); itr != players.end(); ++itr)
                {
                    if (Player* pPlayer = itr->getSource())
                        pPlayer->RemoveAurasDueToSpell(SPELL_GAME_IN_SESSION);
                }

                m_bFriendlyGame = false;
                m_uiChessResetTimer = 35000;
            }
            else if (uiData == FAIL)
            {
                // clean the board for reset
                if (Creature* pMedivh = GetSingleCreatureFromStorage(NPC_ECHO_MEDIVH))
                {
                    pMedivh->CastSpell(pMedivh, SPELL_GAME_OVER, true);
                    pMedivh->CastSpell(pMedivh, SPELL_CLEAR_BOARD, true);
                }

                // remove silence debuff
                Map::PlayerList const& players = instance->GetPlayers();
                for (Map::PlayerList::const_iterator itr = players.begin(); itr != players.end(); ++itr)
                {
                    if (Player* pPlayer = itr->getSource())
                        pPlayer->RemoveAurasDueToSpell(SPELL_GAME_IN_SESSION);
                }

                m_uiChessResetTimer = 35000;
            }
            else if (uiData == IN_PROGRESS || uiData == SPECIAL)
                DoPrepareChessEvent();
            m_auiEncounter[uiType] = uiData;
            break;
        case TYPE_MALCHEZZAR:
            DoUseDoorOrButton(GO_NETHERSPACE_DOOR);
            m_auiEncounter[uiType] = uiData;
            break;
        case TYPE_NIGHTBANE:
            m_auiEncounter[uiType] = uiData;
            DoUseDoorOrButton(GO_MASTERS_TERRACE_DOOR_1);
            DoUseDoorOrButton(GO_MASTERS_TERRACE_DOOR_2);
            break;
            // Store the event type for the Opera
        case TYPE_OPERA_PERFORMANCE:
            m_uiOperaEvent = uiData;
            break;
    }

    // Also save the opera performance, once it's set
    if (uiData == DONE || uiType == TYPE_OPERA_PERFORMANCE)
    {
        OUT_SAVE_INST_DATA;

        std::ostringstream saveStream;
        saveStream << m_auiEncounter[0] << " " << m_auiEncounter[1] << " " << m_auiEncounter[2] << " "
                   << m_auiEncounter[3] << " " << m_auiEncounter[4] << " " << m_auiEncounter[5] << " "
                   << m_auiEncounter[6] << " " << m_auiEncounter[7] << " " << m_auiEncounter[8] << " "
                   << m_auiEncounter[9] << " " << m_auiEncounter[10] << " " << m_uiOperaEvent;

        m_strInstData = saveStream.str();

        SaveToDB();
        OUT_SAVE_INST_DATA_COMPLETE;
    }
}

uint32 instance_karazhan::GetData(uint32 uiType) const
{
    if (uiType < MAX_ENCOUNTER)
        return m_auiEncounter[uiType];

    if (uiType == TYPE_OPERA_PERFORMANCE)
        return m_uiOperaEvent;

    return 0;
}

void instance_karazhan::Load(const char* chrIn)
{
    if (!chrIn)
    {
        OUT_LOAD_INST_DATA_FAIL;
        return;
    }

    OUT_LOAD_INST_DATA(chrIn);

    std::istringstream loadStream(chrIn);

    loadStream >> m_auiEncounter[0] >> m_auiEncounter[1] >> m_auiEncounter[2] >> m_auiEncounter[3]
               >> m_auiEncounter[4] >> m_auiEncounter[5] >> m_auiEncounter[6] >> m_auiEncounter[7]
               >> m_auiEncounter[8] >> m_auiEncounter[9] >> m_auiEncounter[10] >> m_uiOperaEvent;

    for (uint8 i = 0; i < MAX_ENCOUNTER; ++i)
    {
        if (m_auiEncounter[i] == IN_PROGRESS)               // Do not load an encounter as "In Progress" - reset it instead.
            m_auiEncounter[i] = NOT_STARTED;
    }

    OUT_LOAD_INST_DATA_COMPLETE;
}

void instance_karazhan::OnCreatureDeath(Creature* pCreature)
{
    switch (pCreature->GetEntry())
    {
        case NPC_DOROTHEE:
        case NPC_ROAR:
        case NPC_TINHEAD:
        case NPC_STRAWMAN:
            ++m_uiOzDeathCount;
            // Summon Chrone when all 4 Oz mobs are killed
            if (m_uiOzDeathCount == MAX_OZ_OPERA_MOBS)
            {
                if (Creature* pCrone = pCreature->SummonCreature(NPC_CRONE, afChroneSpawnLoc[0], afChroneSpawnLoc[1], afChroneSpawnLoc[2], afChroneSpawnLoc[3], TEMPSUMMON_DEAD_DESPAWN, 0))
                {
                    if (pCreature->getVictim())
                        pCrone->AI()->AttackStart(pCreature->getVictim());
                }
            }
            break;
    }
}

void instance_karazhan::DoPrepareChessEvent()
{
    // Allow all the chess pieces to init start position
    for (GuidList::const_iterator itr = m_lChessPiecesAlliance.begin(); itr != m_lChessPiecesAlliance.end(); ++itr)
    {
        if (Creature* pChessPiece = instance->GetCreature(*itr))
        {
            Creature* pSquare = GetClosestCreatureWithEntry(pChessPiece, NPC_SQUARE_BLACK, 2.0f);
            if (!pSquare)
                pSquare = GetClosestCreatureWithEntry(pChessPiece, NPC_SQUARE_WHITE, 2.0f);
            if (!pSquare)
            {
                script_error_log("Instance Karazhan: ERROR Failed to properly load the Chess square for %s.", pChessPiece->GetGuidStr().c_str());
                return;
            }

            // send event which will prepare the current square
            pChessPiece->AI()->SendAIEvent(AI_EVENT_CUSTOM_B, pSquare, pChessPiece);
        }
    }

    for (GuidList::const_iterator itr = m_lChessPiecesHorde.begin(); itr != m_lChessPiecesHorde.end(); ++itr)
    {
        if (Creature* pChessPiece = instance->GetCreature(*itr))
        {
            Creature* pSquare = GetClosestCreatureWithEntry(pChessPiece, NPC_SQUARE_BLACK, 2.0f);
            if (!pSquare)
                pSquare = GetClosestCreatureWithEntry(pChessPiece, NPC_SQUARE_WHITE, 2.0f);
            if (!pSquare)
            {
                script_error_log("Instance Karazhan: ERROR Failed to properly load the Chess square for %s.", pChessPiece->GetGuidStr().c_str());
                return;
            }

            // send event which will prepare the current square
            pChessPiece->AI()->SendAIEvent(AI_EVENT_CUSTOM_B, pSquare, pChessPiece);
        }
    }

    // add silence debuff
    Map::PlayerList const& players = instance->GetPlayers();
    for (Map::PlayerList::const_iterator itr = players.begin(); itr != players.end(); ++itr)
    {
        if (Player* pPlayer = itr->getSource())
            pPlayer->CastSpell(pPlayer, SPELL_GAME_IN_SESSION, true);
    }

    m_uiAllianceStalkerCount = 0;
    m_uiHordeStalkerCount = 0;
    m_vHordeStalkers.clear();
    m_vAllianceStalkers.clear();

    // sort stalkers depending on side
    std::list<Creature*> lStalkers;
    for (GuidList::const_iterator itr = m_lChessHordeStalkerList.begin(); itr != m_lChessHordeStalkerList.end(); ++itr)
    {
        if (Creature* pTemp = instance->GetCreature(*itr))
            lStalkers.push_back(pTemp);
    }

    if (lStalkers.empty())
    {
        script_error_log("Instance Karazhan: ERROR Failed to properly load the horde side stalkers for the Chess Event.");
        return;
    }

    // get the proper statusBar npc
    Creature* pStatusBar = instance->GetCreature(m_HordeStatusGuid);
    if (!pStatusBar)
        return;

    lStalkers.sort(ObjectDistanceOrder(pStatusBar));
    for (std::list<Creature*>::const_iterator itr = lStalkers.begin(); itr != lStalkers.end(); ++itr)
        m_vHordeStalkers.push_back((*itr)->GetObjectGuid());

    lStalkers.clear();
    for (GuidList::const_iterator itr = m_lChessAllianceStalkerList.begin(); itr != m_lChessAllianceStalkerList.end(); ++itr)
    {
        if (Creature* pTemp = instance->GetCreature(*itr))
            lStalkers.push_back(pTemp);
    }

    if (lStalkers.empty())
    {
        script_error_log("Instance Karazhan: ERROR Failed to properly load the alliance side stalkers for the Chess Event.");
        return;
    }

    // get the proper statusBar npc
    pStatusBar = instance->GetCreature(m_AllianceStatusGuid);
    if (!pStatusBar)
        return;

    lStalkers.sort(ObjectDistanceOrder(pStatusBar));
    for (std::list<Creature*>::const_iterator itr = lStalkers.begin(); itr != lStalkers.end(); ++itr)
        m_vAllianceStalkers.push_back((*itr)->GetObjectGuid());
}

void instance_karazhan::DoMoveChessPieceToSides(uint32 uiSpellId, uint32 uiFaction, bool bGameEnd /*= false*/)
{
    // assign proper faction variables
    GuidVector& vStalkers = uiFaction == FACTION_ID_CHESS_ALLIANCE ? m_vAllianceStalkers : m_vHordeStalkers;
    uint32 uiCount = uiFaction == FACTION_ID_CHESS_ALLIANCE ? m_uiAllianceStalkerCount : m_uiHordeStalkerCount;

    // get the proper statusBar npc
    Creature* pStatusBar = instance->GetCreature(uiFaction == FACTION_ID_CHESS_ALLIANCE ? m_AllianceStatusGuid : m_HordeStatusGuid);
    if (!pStatusBar)
        return;

    if (vStalkers.size() < uiCount + 1)
        return;

    // handle stalker transformation
    if (Creature* pStalker = instance->GetCreature(vStalkers[uiCount]))
    {
        // need to provide specific target, in order to ensure the logic of the event
        pStatusBar->CastSpell(pStalker, uiSpellId, true);
        uiFaction == FACTION_ID_CHESS_ALLIANCE ? ++m_uiAllianceStalkerCount : ++m_uiHordeStalkerCount;
    }

    // handle emote on end game
    if (bGameEnd)
    {
        // inverse factions
        vStalkers.clear();
        vStalkers = uiFaction == FACTION_ID_CHESS_ALLIANCE ? m_vHordeStalkers : m_vAllianceStalkers;

        for (GuidVector::const_iterator itr = vStalkers.begin(); itr != vStalkers.end(); ++itr)
        {
            if (Creature* pStalker = instance->GetCreature(*itr))
                pStalker->HandleEmote(EMOTE_STATE_APPLAUD);
        }
    }
}

void instance_karazhan::DoPrepareOperaStage(Creature* pOrganizer)
{
    if (!pOrganizer)
        return;

    debug_log("SD2: Barnes Opera Event - Introduction complete - preparing encounter %d", GetData(TYPE_OPERA_PERFORMANCE));

    // summon the bosses and respawn the stage background
    switch (GetData(TYPE_OPERA_PERFORMANCE))
    {
        case OPERA_EVENT_WIZARD_OZ:
            for (uint8 i = 0; i < MAX_OZ_OPERA_MOBS; ++i)
                pOrganizer->SummonCreature(aOperaLocOz[i].uiEntry, aOperaLocOz[i].fX, aOperaLocOz[i].fY, aOperaLocOz[i].fZ, aOperaLocOz[i].fO, TEMPSUMMON_DEAD_DESPAWN, 0);
            DoRespawnGameObject(GO_OZ_BACKDROP, 12 * HOUR);
            for (GuidList::const_iterator itr = m_lOperaHayGuidList.begin(); itr != m_lOperaHayGuidList.end(); ++itr)
                DoRespawnGameObject(*itr, 12 * HOUR);
            break;
        case OPERA_EVENT_RED_RIDING_HOOD:
            pOrganizer->SummonCreature(aOperaLocWolf.uiEntry, aOperaLocWolf.fX, aOperaLocWolf.fY, aOperaLocWolf.fZ, aOperaLocWolf.fO, TEMPSUMMON_DEAD_DESPAWN, 0);
            DoRespawnGameObject(GO_HOOD_BACKDROP, 12 * HOUR);
            DoRespawnGameObject(GO_HOOD_HOUSE,    12 * HOUR);
            for (GuidList::const_iterator itr = m_lOperaTreeGuidList.begin(); itr != m_lOperaTreeGuidList.end(); ++itr)
                DoRespawnGameObject(*itr, 12 * HOUR);
            break;
        case OPERA_EVENT_ROMULO_AND_JUL:
            pOrganizer->SummonCreature(aOperaLocJul.uiEntry, aOperaLocJul.fX, aOperaLocJul.fY, aOperaLocJul.fZ, aOperaLocJul.fO, TEMPSUMMON_DEAD_DESPAWN, 0);
            DoRespawnGameObject(GO_RAJ_BACKDROP, 12 * HOUR);
            DoRespawnGameObject(GO_RAJ_MOON,     12 * HOUR);
            DoRespawnGameObject(GO_RAJ_BALCONY,  12 * HOUR);
            break;
    }

    SetData(TYPE_OPERA, IN_PROGRESS);
}

void instance_karazhan::Update(uint32 uiDiff)
{
    if (m_uiChessResetTimer)
    {
        // respawn all chess pieces and side stalkers on the original position
        if (m_uiChessResetTimer <= uiDiff)
        {
            for (GuidList::const_iterator itr = m_lChessPiecesAlliance.begin(); itr != m_lChessPiecesAlliance.end(); ++itr)
            {
                if (Creature* pTemp = instance->GetCreature(*itr))
                    pTemp->Respawn();
            }
            for (GuidList::const_iterator itr = m_lChessPiecesHorde.begin(); itr != m_lChessPiecesHorde.end(); ++itr)
            {
                if (Creature* pTemp = instance->GetCreature(*itr))
                    pTemp->Respawn();
            }

            for (GuidList::const_iterator itr = m_lChessAllianceStalkerList.begin(); itr != m_lChessAllianceStalkerList.end(); ++itr)
            {
                if (Creature* pTemp = instance->GetCreature(*itr))
                {
                    pTemp->Respawn();
                    pTemp->HandleEmote(EMOTE_STATE_NONE);
                }
            }
            for (GuidList::const_iterator itr = m_lChessHordeStalkerList.begin(); itr != m_lChessHordeStalkerList.end(); ++itr)
            {
                if (Creature* pTemp = instance->GetCreature(*itr))
                {
                    pTemp->Respawn();
                    pTemp->HandleEmote(EMOTE_STATE_NONE);
                }
            }

            if (GetData(TYPE_CHESS) == FAIL)
                SetData(TYPE_CHESS, NOT_STARTED);
            else if (GetData(TYPE_CHESS) == DONE)
                m_bFriendlyGame = true;

            m_uiChessResetTimer = 0;
        }
        else
            m_uiChessResetTimer -= uiDiff;
    }
}

InstanceData* GetInstanceData_instance_karazhan(Map* pMap)
{
    return new instance_karazhan(pMap);
}

void AddSC_instance_karazhan()
{
    Script* pNewScript;

    pNewScript = new Script;
    pNewScript->Name = "instance_karazhan";
    pNewScript->GetInstanceData = &GetInstanceData_instance_karazhan;
    pNewScript->RegisterSelf();
}