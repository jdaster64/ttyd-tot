#include "patches_battle.h"

#include "common_functions.h"
#include "evt_cmd.h"
#include "mod.h"
#include "patch.h"
#include "patches_mario_move.h"
#include "patches_options.h"
#include "tot_custom_rel.h"
#include "tot_generate_enemy.h"
#include "tot_gsw.h"
#include "tot_manager_achievements.h"
#include "tot_manager_cosmetics.h"
#include "tot_manager_move.h"

#include <ttyd/_core_language_libs.h>
#include <ttyd/ac_button_down.h>
#include <ttyd/battle.h>
#include <ttyd/battle_ac.h>
#include <ttyd/battle_actrecord.h>
#include <ttyd/battle_audience.h>
#include <ttyd/battle_audience_kind_data.h>
#include <ttyd/battle_break_slot.h>
#include <ttyd/battle_database_common.h>
#include <ttyd/battle_damage.h>
#include <ttyd/battle_disp.h>
#include <ttyd/battle_event_cmd.h>
#include <ttyd/battle_item_data.h>
#include <ttyd/battle_message.h>
#include <ttyd/battle_pad.h>
#include <ttyd/battle_seq_command.h>
#include <ttyd/battle_status_effect.h>
#include <ttyd/battle_sub.h>
#include <ttyd/battle_unit.h>
#include <ttyd/eff_gonbaba_breath.h>
#include <ttyd/eff_stamp_n64.h>
#include <ttyd/effdrv.h>
#include <ttyd/evtmgr.h>
#include <ttyd/evtmgr_cmd.h>
#include <ttyd/item_data.h>
#include <ttyd/mario_pouch.h>
#include <ttyd/pmario_sound.h>
#include <ttyd/system.h>

#include <cstdint>
#include <cstdio>
#include <cstring>

// Assembly patch functions.
extern "C" {
    // action_command_patches.s
    void StartButtonDownChooseButtons();
    void BranchBackButtonDownChooseButtons();
    void StartButtonDownWrongButton();
    void BranchBackButtonDownWrongButton();
    void ConditionalBranchButtonDownWrongButton();
    void StartButtonDownCheckComplete();
    void BranchBackButtonDownCheckComplete();
    void ConditionalBranchButtonDownCheckComplete();
    void StartGetGuardDifficulty();
    void BranchBackGetGuardDifficulty();
    // action_menu_patches.s
    void StartSpendFpOnSwitchPartner();
    void BranchBackSpendFpOnSwitchPartner();
    // action_seq_patches.s
    void StartSetConfuseProcRate();
    void BranchBackSetConfuseProcRate();
    // attack_fx_patches.s
    void StartCheckPlayAttackFX();
    void BranchBackCheckPlayAttackFX();
    void ConditionalBranchCheckPlayAttackFX();
    // audience_level_patches.s
    void StartSetTargetAudienceCount();
    void BranchBackSetTargetAudienceCount();
    // permanent_status_patches.s
    void StartCheckRecoveryStatus();
    void BranchBackCheckRecoveryStatus();
    void ConditionalBranchCheckRecoveryStatus();
    void StartStatusIconDisplay();
    void BranchBackStatusIconDisplay();
    void ConditionalBranchStatusIconDisplay();
    // status_effect_patches.s
    void StartCalculateCounterDamage();
    void BranchBackCalculateCounterDamage();
    void StartCheckExplosiveKO();
    void BranchBackCheckExplosiveKO();
    void ConditionalBranchCheckExplosiveKO();
    void StartToggleScopedAndCheckFreezeBreak();
    void BranchBackToggleScopedAndCheckFreezeBreak();
    void StartTrackPoisonDamage();
    void BranchBackTrackPoisonDamage();
    // target_selection_patches.s
    void StartCommandHandleSideSelection();
    void BranchBackCommandHandleSideSelection();
    void StartCommandDisplaySideSelection();
    void BranchBackCommandDisplaySideSelection();
    void ConditionalBranchCommandDisplaySideSelection();
    
    int32_t getActionCommandDifficulty() {
        return mod::g_Mod->state_.GetOption(mod::tot::OPT_AC_DIFFICULTY);
    }        
    void signalPlayerInitiatedPartySwitch(ttyd::battle_unit::BattleWorkUnit* unit) {
        mod::tot::patch::battle::SignalPlayerInitiatedPartySwitch();
    }
    void setTargetAudienceCount() {
        mod::tot::patch::battle::SetTargetAudienceAmount();
    }
    double applySpRegenMultiplier(double base_regen) {
        return mod::tot::patch::battle::ApplySpRegenMultiplier(base_regen);
    }
    void calculateCounterDamage(
        ttyd::battle_damage::CounterattackWork* cw,
        ttyd::battle_unit::BattleWorkUnit* attacker,
        ttyd::battle_unit::BattleWorkUnit* target,
        ttyd::battle_unit::BattleWorkUnitPart* part, int32_t damage_dealt) {
        mod::tot::patch::battle::CalculateCounterDamage(
            cw, attacker, target, part, damage_dealt);
    }
    void toggleOffScopedStatus(
        ttyd::battle_unit::BattleWorkUnit* attacker,
        ttyd::battle_unit::BattleWorkUnit* target,
        ttyd::battle_database_common::BattleWeapon* weapon) {
        mod::tot::patch::battle::ToggleScopedStatus(attacker, target, weapon);
    }
    void commandHandleSideSelection() {
        mod::tot::patch::battle::HandleSideSelection();
    }
    bool checkOnSelectedSide(int32_t target_idx) {
        return mod::tot::patch::battle::CheckOnSelectedSide(target_idx);
    }
    bool checkPlayAttackFx(uint32_t flags, gc::vec3* position) {
        return mod::tot::patch::battle::CheckPlayAttackFx(flags, position);
    }
}

namespace mod::tot::patch {

namespace {

// For convenience.
using namespace ::ttyd::battle_database_common;
using namespace ::ttyd::battle_unit;

using ::ttyd::battle::BattleWork;
using ::ttyd::battle::BattleWorkAudience;
using ::ttyd::battle::BattleWorkCommand;
using ::ttyd::battle::SpBonusInfo;
using ::ttyd::battle_damage::CounterattackWork;
using ::ttyd::eff_gonbaba_breath::EffGonbabaBreathWork;
using ::ttyd::evtmgr::EvtEntry;
using ::ttyd::evtmgr_cmd::evtGetValue;
using ::ttyd::evtmgr_cmd::evtSetValue;

namespace AttackTargetClass_Flags = 
    ::ttyd::battle_database_common::AttackTargetClass_Flags;
namespace ItemType = ::ttyd::item_data::ItemType;

using WeaponDamageFn = uint32_t (*) (
    BattleWorkUnit*, BattleWeapon*, BattleWorkUnit*, BattleWorkUnitPart*);

}  // namespace

// Function hooks.
extern void (*g_BattleStoreExp_trampoline)(BattleWork*, int32_t);
extern void (*g__EquipItem_trampoline)(BattleWorkUnit*, uint32_t, int32_t);
extern void (*g_BattleActionCommandSetDifficulty_trampoline)(
    BattleWork*, BattleWorkUnit*, int32_t);
extern int32_t (*g_BattleActionCommandCheckDefence_trampoline)(
    BattleWorkUnit*, BattleWeapon*);
extern int32_t (*g_BattleCheckDamage_trampoline)(
    BattleWorkUnit*, BattleWorkUnit*, BattleWorkUnitPart*,
    BattleWeapon*, uint32_t);
extern int32_t (*g_BattlePreCheckDamage_trampoline)(
    BattleWorkUnit*, BattleWorkUnit*, BattleWorkUnitPart*,
    BattleWeapon*, uint32_t);
extern uint32_t (*g_BattleSetStatusDamageFromWeapon_trampoline)(
    BattleWorkUnit*, BattleWorkUnit*, BattleWorkUnitPart*,
    BattleWeapon*, uint32_t);
extern int32_t (*g_BattleCalculateDamage_trampoline)(
    BattleWorkUnit*, BattleWorkUnit*, BattleWorkUnitPart*, BattleWeapon*,
    uint32_t*, uint32_t);
extern int32_t (*g_BattleCalculateFpDamage_trampoline)(
    BattleWorkUnit*, BattleWorkUnit*, BattleWorkUnitPart*, BattleWeapon*,
    uint32_t*, uint32_t);
extern void (*g_BattleCheckPikkyoro_trampoline)(BattleWeapon*, uint32_t*);
extern void (*g_BattleDamageDirect_trampoline)(
    int32_t, BattleWorkUnit*, BattleWorkUnitPart*, int32_t, int32_t,
    uint32_t, uint32_t, uint32_t);
extern int32_t (*g_btlevtcmd_CommandGetWeaponAddress_trampoline)(EvtEntry*, bool);
extern int32_t (*g_btlevtcmd_GetSelectEnemy_trampoline)(EvtEntry*, bool);
extern int32_t (*g_btlevtcmd_ChangeParty_trampoline)(EvtEntry*, bool);
extern void* (*g_BattleSetConfuseAct_trampoline)(BattleWork*, BattleWorkUnit*);
extern BattleWeapon* (*g_BattleGetSelectWeapon_trampoline)(BattleWork*);
extern void (*g__btlcmd_SetAttackEvent_trampoline)(BattleWorkUnit*, BattleWorkCommand*);
extern uint32_t (*g_BtlUnit_CheckRecoveryStatus_trampoline)(
    BattleWorkUnit*, int8_t);
extern void (*g_BtlUnit_ClearStatus_trampoline)(BattleWorkUnit*);
extern void (*g_BattleAudience_Case_Appeal_trampoline)(BattleWorkUnit*);
extern void (*g_BattleAudience_Case_ActionCommandBad_trampoline)(BattleWeapon*);
extern void (*g_BattleAudience_ApRecoveryBuild_trampoline)(SpBonusInfo*);
extern int32_t (*g_btlevtcmd_AnnounceMessage_trampoline)(EvtEntry*, bool);
extern uint32_t (*g_battleAcMain_ButtonDown_trampoline)(BattleWork*);
extern void (*g_init_breath_trampoline)(EffGonbabaBreathWork*, int32_t, int32_t);
// Patch addresses.
extern const int32_t g_BtlUnit_EquipItem_Patch_ReviseHpFp;
extern const int32_t g_BattleActionCommandCheckDefence_GetDifficulty_BH;
extern const int32_t g_BattleActionCommandCheckDefence_GetDifficulty_EH;
extern const int32_t g_BattleCheckDamage_AlwaysFreezeBreak_BH;
extern const int32_t g_BattleCheckDamage_CheckExplosiveKO_BH;
extern const int32_t g_BattleCheckDamage_CheckExplosiveKO_EH;
extern const int32_t g_BattleCheckDamage_CheckExplosiveKO_CH1;
extern const int32_t g_BattleCheckDamage_CalculateCounterDamage_BH;
extern const int32_t g_BattleCheckDamage_CalculateCounterDamage_EH;
extern const int32_t g_BattleSetStatusDamage_Patch_FeelingFineYesCase;
extern const int32_t g_BattleSetStatusDamage_Patch_FeelingFineNoCase;
extern const int32_t g_BattleSetStatusDamage_Patch_SkipHugeTinyArrows;
extern const int32_t g_BattleDamageDirect_CheckPlayAttackFX_BH;
extern const int32_t g_BattleDamageDirect_CheckPlayAttackFX_EH;
extern const int32_t g_BattleDamageDirect_CheckPlayAttackFX_CH1;
extern const int32_t g_btlSeqAct_SetConfuseProcRate_BH;
extern const int32_t g_btlseqTurn_TrackPoisonDamage_BH;
extern const int32_t g__btlcmd_SetAttackEvent_SwitchPartnerCost_BH;
extern const int32_t g_BattleCommandDisplay_HandleSelectSide_BH;
extern const int32_t g_BattleCommandDisplay_HandleSelectSide_EH;
extern const int32_t g_BattleCommandDisplay_HandleSelectSide_CH1;
extern const int32_t g_BattleCommandInput_HandleSelectSide_BH;
extern const int32_t g__btlcmd_MakeOperationTable_Patch_NoSuperCharge;
extern const int32_t g_BattleAudience_SetTargetAmount_BH;
extern const int32_t g_battleAcMain_ButtonDown_ChooseButtons_BH;
extern const int32_t g_battleAcMain_ButtonDown_ChooseButtons_EH;
extern const int32_t g_battleAcMain_ButtonDown_WrongButton_BH;
extern const int32_t g_battleAcMain_ButtonDown_WrongButton_EH;
extern const int32_t g_battleAcMain_ButtonDown_WrongButton_CH1;
extern const int32_t g_battleAcMain_ButtonDown_CheckComplete_BH;
extern const int32_t g_battleAcMain_ButtonDown_CheckComplete_EH;
extern const int32_t g_battleAcMain_ButtonDown_CheckComplete_CH1;
extern const int32_t g_battle_status_icon_SkipIconForPermanentStatus_BH;
extern const int32_t g_battle_status_icon_SkipIconForPermanentStatus_EH;
extern const int32_t g_battle_status_icon_SkipIconForPermanentStatus_CH1;
extern const int32_t g_btlseqEnd_Patch_CheckDisableExpLevel;
extern const int32_t g_effStarPointDisp_Patch_SetIconId;
extern const int32_t g_BattleSetStatusDamage_FeelingFine_SwitchTable;

int32_t g_PartySwitchNextFpCost = 1;
int32_t g_PartySwitchPlayerInitiated = false;
int32_t g_GonbabaBreathDir = 0;

namespace battle {

// Fetches the base parameters for a given status from a weapon.
void GetStatusParams(
    BattleWorkUnit* attacker, BattleWorkUnit* target, BattleWeapon* weapon,
    int32_t status_type, int32_t& chance, int32_t& turns, int32_t& strength) {
    chance = 0;
    turns = 0;
    strength = 0;
    
    switch (status_type) {
        case StatusEffectType::ALLERGIC:
            chance = weapon->allergic_chance;
            turns = weapon->allergic_time;
            break;
        case StatusEffectType::SLEEP:
            chance = weapon->sleep_chance;
            turns = weapon->sleep_time;
            break;
        case StatusEffectType::STOP:
            chance = weapon->stop_chance;
            turns = weapon->stop_time;
            break;
        case StatusEffectType::DIZZY:
            chance = weapon->dizzy_chance;
            turns = weapon->dizzy_time;
            break;
        case StatusEffectType::POISON:
            chance = weapon->poison_chance;
            turns = weapon->poison_time;
            strength = weapon->poison_strength;
            break;
        case StatusEffectType::CONFUSE:
            chance = weapon->confuse_chance;
            turns = weapon->confuse_time;
            break;
        case StatusEffectType::ELECTRIC:
            chance = weapon->electric_chance;
            turns = weapon->electric_time;
            break;
        case StatusEffectType::DODGY:
            chance = weapon->dodgy_chance;
            turns = weapon->dodgy_time;
            break;
        case StatusEffectType::BURN:
            chance = weapon->burn_chance;
            turns = weapon->burn_time;
            break;
        case StatusEffectType::FREEZE:
            chance = weapon->freeze_chance;
            turns = weapon->freeze_time;
            break;
        case StatusEffectType::HUGE:
            if (weapon->size_change_strength > 0) {
                chance = weapon->size_change_chance;
                turns = weapon->size_change_time;
                strength = weapon->size_change_strength;
            }
            break;
        case StatusEffectType::TINY:
            if (weapon->size_change_strength <= 0) {
                chance = weapon->size_change_chance;
                turns = weapon->size_change_time;
                strength = weapon->size_change_strength;
            }
            break;
        case StatusEffectType::ATTACK_UP:
            if (weapon->atk_change_strength > 0) {
                chance = weapon->atk_change_chance;
                turns = weapon->atk_change_time;
                strength = weapon->atk_change_strength;
            }
            break;
        case StatusEffectType::ATTACK_DOWN:
            if (weapon->atk_change_strength <= 0) {
                chance = weapon->atk_change_chance;
                turns = weapon->atk_change_time;
                strength = weapon->atk_change_strength;
            }
            break;
        case StatusEffectType::DEFENSE_UP:
            if (weapon->def_change_strength > 0) {
                chance = weapon->def_change_chance;
                turns = weapon->def_change_time;
                strength = weapon->def_change_strength;
            }
            break;
        case StatusEffectType::DEFENSE_DOWN:
            if (weapon->def_change_strength <= 0) {
                chance = weapon->def_change_chance;
                turns = weapon->def_change_time;
                strength = weapon->def_change_strength;
            }
            break;
        case StatusEffectType::CHARGE:
            chance = weapon->charge_strength ? 100 : 0;
            strength = weapon->charge_strength;
            break;
        case StatusEffectType::INVISIBLE:
            chance = weapon->invisible_chance;
            turns = weapon->invisible_time;
            break;
        case StatusEffectType::FAST:
            chance = weapon->fast_chance;
            turns = weapon->fast_time;
            break;
        case StatusEffectType::SLOW:
            chance = weapon->slow_chance;
            turns = weapon->slow_time;
            break;
        case StatusEffectType::PAYBACK:
            chance = weapon->payback_time ? 100 : 0;
            turns = weapon->payback_time;
            break;
        case StatusEffectType::HOLD_FAST:
            chance = weapon->hold_fast_time ? 100 : 0;
            turns = weapon->hold_fast_time;
            break;
        case StatusEffectType::HP_REGEN:
            chance = weapon->hp_regen_time ? 100 : 0;
            turns = weapon->hp_regen_time;
            strength = weapon->hp_regen_strength;
            break;
        case StatusEffectType::FP_REGEN:
            chance = weapon->fp_regen_time ? 100 : 0;
            turns = weapon->fp_regen_time;
            strength = weapon->fp_regen_strength;
            break;
        case StatusEffectType::FRIGHT:
            chance = weapon->fright_chance;
            break;
        case StatusEffectType::GALE_FORCE:
            chance = weapon->gale_force_chance;
            break;
        case StatusEffectType::OHKO:
            chance = weapon->ohko_chance;
            break;
    }
    
    switch (weapon->item_id) {
        case ItemType::SLEEPY_STOMP:
            if (status_type == StatusEffectType::SLEEP) {
                turns += MoveManager::GetSelectedLevel(
                    MoveType::JUMP_SLEEPY_STOMP) * 2 - 2;
            }
            break;
        case ItemType::HEAD_RATTLE:
            if (status_type == StatusEffectType::TINY) {
                turns += MoveManager::GetSelectedLevel(
                    MoveType::HAMMER_SHRINK_SMASH) * 1 - 1;
            }
            break;
        case ItemType::ICE_SMASH:
            if (status_type == StatusEffectType::FREEZE) {
                turns += MoveManager::GetSelectedLevel(
                    MoveType::HAMMER_ICE_SMASH) * 1 - 1;
            }
            break;
        case ItemType::TORNADO_JUMP:
            if (status_type == StatusEffectType::DIZZY) {
                // If not a floating enemy, don't inflict Dizzy status.
                if (!(target->attribute_flags & 
                      BattleUnitAttribute_Flags::UNQUAKEABLE)) {
                    turns = 0;
                    chance = 0;
                }
            }
            break;
        case ItemType::CHARGE:
        case ItemType::CHARGE_P:
            if (status_type == StatusEffectType::CHARGE) {
                strength += MoveManager::GetSelectedLevel(
                    MoveType::BADGE_MOVE_CHARGE +
                    (attacker->current_kind == BattleUnitType::MARIO ? 0 : 1))
                     - 1;
            }
            break;
        case ItemType::TOT_TOUGHEN_UP:
        case ItemType::TOT_TOUGHEN_UP_P:
            if (status_type == StatusEffectType::DEFENSE_UP) {
                strength += MoveManager::GetSelectedLevel(
                    MoveType::BADGE_MOVE_SUPER_CHARGE +
                    (attacker->current_kind == BattleUnitType::MARIO ? 0 : 1))
                     - 1;
            }
            break;
    }

    if (status_type == StatusEffectType::CHARGE) {
        // Scale Charge attacks in the same way as ATK and FP damage.
        if (strength > 0 && !weapon->item_id &&
            attacker->current_kind <= BattleUnitType::BONETAIL) {
            int32_t altered_charge;
            GetEnemyStats(
                attacker->current_kind, nullptr, &altered_charge,
                nullptr, nullptr, nullptr, strength);
            strength = altered_charge;
        }
        // Cap max Charge at 99.
        if (strength > 99) strength = 99;
    }
}

// Processes all statuses given a weapon.
uint32_t GetStatusDamageFromWeapon(
    BattleWorkUnit* attacker, BattleWorkUnit* target, BattleWorkUnitPart* part,
    BattleWeapon* weapon, uint32_t extra_params) {
    uint32_t result = 0;
    if ((extra_params & 0x2000'0000) == 0) {
        for (int32_t type = 0; type < StatusEffectType::STATUS_MAX; ++type) {
            int32_t chance, turns, strength;
            GetStatusParams(
                attacker, target, weapon, type, chance, turns, strength);
                
            uint32_t special_properties = weapon->special_property_flags;

            bool always_update = true;
            switch (type) {
                case StatusEffectType::GALE_FORCE: {
                    // Modify rate based on enemy HP and Huge/Tiny status.
                    if (target->size_change_turns > 0) {
                        if (target->size_change_strength > 0) {
                            chance = 0;
                        } else {
                            chance *= 1.5;
                        }
                    } else {
                        // Rate ranges from 70% at max health to 100% at half.
                        float factor =
                            1.3f - 0.6f * target->current_hp / target->max_hp;
                        if (factor > 1.0f) factor = 1.0f;
                        chance *= factor;
                    }
                    break;
                }
                case StatusEffectType::POISON:
                    // Each new infliction increases power by 1.
                    if (target->poison_turns > 0 && strength > 0) {
                        strength = target->poison_strength + 1;
                    }
                    if (strength > 99) strength = 99;
                    break;
                case StatusEffectType::HUGE:
                case StatusEffectType::ATTACK_UP:
                case StatusEffectType::DEFENSE_UP:
                    // Should not update status if turns and strength = 0
                    // (e.g. a Tasty Tonic was used).
                    always_update = false;
                    break;
            }

            // Make midbosses less susceptible to most negative statuses,
            // unless the move is meant to guarantee that they go through.
            if ((turns != 0 || strength != 0) &&
                (target->status_flags & BattleUnitStatus_Flags::MIDBOSS)) {

                if (type == StatusEffectType::HUGE) {
                    // Always immune to non-permanent Huge status.
                    chance = 0;
                }

                // Only reduce other vulnerabilities for non-guaranteed status.
                if (!(weapon->special_property_flags 
                    & AttackSpecialProperty_Flags::IGNORES_STATUS_CHANCE)) {
                    switch (type) {
                        case StatusEffectType::SLEEP:
                        case StatusEffectType::STOP:
                        case StatusEffectType::DIZZY:
                        case StatusEffectType::CONFUSE:
                        case StatusEffectType::FREEZE:
                        case StatusEffectType::ATTACK_DOWN:
                        case StatusEffectType::DEFENSE_DOWN:
                        case StatusEffectType::SLOW:
                            chance = chance * 75 / 100;
                            break;
                        case StatusEffectType::TINY:
                        case StatusEffectType::FRIGHT:
                        case StatusEffectType::OHKO:
                            chance /= 2;
                            break;
                        case StatusEffectType::GALE_FORCE:
                            // Effectively 50% if shrunk, 0% otherwise.
                            chance /= 3;
                            break;
                    }
                }
            }
            
            // Non-KO statuses are guaranteed to land if target is Scoped+.
            if (target->status_flags & BattleUnitStatus_Flags::SCOPED_PLUS) {
                switch (type) {
                    case StatusEffectType::FRIGHT:
                    case StatusEffectType::GALE_FORCE:
                    case StatusEffectType::OHKO:
                        break;
                    default:
                        if (chance > 0) chance = 100;
                        special_properties |=
                            AttackSpecialProperty_Flags::IGNORES_STATUS_CHANCE;
                        break;
                }
            }

            // Make midbosses and bosses receive Slow status in place of Stop.
            int32_t actual_type = type;
            if (type == StatusEffectType::STOP) {
                switch (target->current_kind) {
                    case BattleUnitType::HOOKTAIL:
                    case BattleUnitType::GLOOMTAIL:
                    case BattleUnitType::BONETAIL:
                    case BattleUnitType::ATOMIC_BOO:
                    case BattleUnitType::TOT_COSMIC_BOO:
                        actual_type = StatusEffectType::SLOW;
                        break;
                }
                if (target->status_flags & BattleUnitStatus_Flags::MIDBOSS) {
                    actual_type = StatusEffectType::SLOW;
                }
            }

            if (always_update || turns != 0 || strength != 0) {
                int32_t damage_result = 
                    ttyd::battle_damage::BattleSetStatusDamage(
                        &result, target, part, special_properties, actual_type,
                        chance, /* gale_factor */ 0, turns, strength);

                if (damage_result && type == StatusEffectType::INVISIBLE) {
                    ttyd::battle_disp::btlDispPoseAnime(part);
                }

                // Use unk_136 as "proc rate" for failed actions from Confusion.
                if (damage_result && type == StatusEffectType::CONFUSE) {
                    // By default, 50/50.
                    target->unk_136 = 50;
                    if (weapon->pad_ae == MoveType::MOWZ_TEASE) {
                        // 60 / 75 / 90% failed action chance, based on level.
                        target->unk_136 = 
                            45 + 15 * MoveManager::GetSelectedLevel(
                                MoveType::MOWZ_TEASE);
                    }
                }

                // Special case: for Tiny cure on midboss, go back to perma-Huge.
                if (damage_result && type == StatusEffectType::TINY &&
                    (target->status_flags & BattleUnitStatus_Flags::MIDBOSS) &&
                    turns == 0 && strength == 0) {
                    BtlUnit_SetStatus(target, StatusEffectType::HUGE, 100, 1);
                }

                // Achievement for Charge status failing on an enemy.
                if (!damage_result && type == StatusEffectType::CHARGE &&
                    strength > 0 && target->true_kind <= BattleUnitType::BONETAIL) {
                    AchievementsManager::MarkCompleted(
                        AchievementId::V2_MISC_ALLERGIC);
                }

                // Achievement for getting 5+ negative status on a single enemy.
                if (damage_result && target->true_kind <= BattleUnitType::BONETAIL) {
                    int32_t statuses = 0;
                    if (target->sleep_turns)                    ++statuses;
                    if (target->stop_turns)                     ++statuses;
                    if (target->dizzy_turns)                    ++statuses;
                    if (target->confusion_turns)                ++statuses;
                    if (target->burn_turns)                     ++statuses;
                    if (target->freeze_turns)                   ++statuses;
                    if (target->allergic_turns)                 ++statuses;
                    if (target->slow_turns)                     ++statuses;
                    if (target->poison_strength > 0)            ++statuses;
                    if (target->attack_change_strength < 0)     ++statuses;
                    if (target->defense_change_strength < 0)    ++statuses;
                    if (target->size_change_strength < 0)       ++statuses;
                    if (statuses >= 5) {
                        AchievementsManager::MarkCompleted(
                            AchievementId::V2_MISC_STATUS_5);
                    }
                }
            }
        }
    }
    return result;
}

// Ticks down the turn count for a given status, and returns whether it expired.
uint32_t StatusEffectTick(BattleWorkUnit* unit, int8_t status_type) {
    uint32_t result = false;
    
    if (unit == nullptr)
        return result;
    if (ttyd::battle_unit::BtlUnit_CheckStatus(unit, StatusEffectType::OHKO))
        return result;
    if (unit->current_hp < 1)
        return result;
    
    int8_t turns, strength;
    ttyd::battle_unit::BtlUnit_GetStatus(unit, status_type, &turns, &strength);
    
    if (turns <= 0) return result;
    
    // 100+ turn (permanent) statuses should not tick down.
    if (turns <= 99) --turns;
    
    if (turns == 0) {
        // If expired, reset the effect strength to 0.
        strength = 0;
        result = 1;
        
        // Special case: for Tiny ending on midboss, go back to perma-Huge.
        if (status_type == StatusEffectType::TINY &&
            (unit->status_flags & BattleUnitStatus_Flags::MIDBOSS)) {
            status_type = StatusEffectType::HUGE;
            strength = 1;
            turns = 100;
        }
    } else if (status_type == StatusEffectType::POISON && strength < 99) {
        // Poison strengthens every turn it remains active.
        ++strength;
    }
    
    BtlUnit_SetStatus(unit, status_type, turns, strength);
    
    return result;
}

// Handles Lucky chance from evasion badges.
bool CheckEvasionBadges(BattleWorkUnit* unit) {
    bool cap_badge_evasion = false;
    if (cap_badge_evasion) {
        float hit_chance = 100.f;
        for (int32_t i = 0; i < unit->badges_equipped.pretty_lucky; ++i) {
            hit_chance *= 0.875f;
        }
        for (int32_t i = 0; i < unit->badges_equipped.lucky_day; ++i) {
            hit_chance *= 0.75f;
        }
        // Will not activate if the actor is at 1/1 (max) HP.
        if (unit->current_hp < unit->max_hp &&
            unit->current_hp <= options::GetPinchThresholdForMaxHp(
                unit->max_hp, /* peril = */ false)) {
            for (int32_t i = 0; i < unit->badges_equipped.close_call; ++i) {
                hit_chance *= 0.67f;
            }
        }
        if (hit_chance < 20.f) hit_chance = 20.f;
        return ttyd::system::irand(100) >= hit_chance;
    } else {
        for (int32_t i = 0; i < unit->badges_equipped.pretty_lucky; ++i) {
            if (ttyd::system::irand(100) < 10) return true;
        }
        for (int32_t i = 0; i < unit->badges_equipped.lucky_day; ++i) {
            if (ttyd::system::irand(100) < 25) return true;
        }
        // Will not activate if the actor is at 1/1 (max) HP.
        if (unit->current_hp < unit->max_hp &&
            unit->current_hp <= options::GetPinchThresholdForMaxHp(
                unit->max_hp, /* peril = */ false)) {
            for (int32_t i = 0; i < unit->badges_equipped.close_call; ++i) {
                if (ttyd::system::irand(100) < 33) return true;
            }
        }
    }
    return false;
}

// Checks whether an attack should connect, or what caused it to miss.
int32_t PreCheckDamage(
    BattleWorkUnit* attacker, BattleWorkUnit* target, BattleWorkUnitPart* part,
    BattleWeapon* weapon, uint32_t extra_params) {
        
    if (!target || !part) return 1;
    
    // Expend charge for chargeable attacks, whether attack lands or not.
    if (weapon->special_property_flags & 
        AttackSpecialProperty_Flags::CHARGE_BUFFABLE) {
        attacker->token_flags |= BattleUnitToken_Flags::CHARGE_EXPENDED;
    }
    
    if ((part->part_attribute_flags & PartsAttribute_Flags::UNK_MISS_4000) &&
        !(weapon->special_property_flags & 
          AttackSpecialProperty_Flags::UNKNOWN_0x40000)) {
        return 2;
    }
    
    // Scoped status guarantees a hit.
    if ((target->status_flags & BattleUnitStatus_Flags::SCOPED) ||
        (target->status_flags & BattleUnitStatus_Flags::SCOPED_PLUS)) {
        return 1;
    }
    if (weapon->special_property_flags & 
        AttackSpecialProperty_Flags::CANNOT_MISS) {
        return 1;
    }
    
    if (BtlUnit_CheckStatus(target, StatusEffectType::INVISIBLE) ||
        (target->attribute_flags & BattleUnitAttribute_Flags::UNK_8) ||
        (target->attribute_flags & BattleUnitAttribute_Flags::VEILED)) {
        return 4;
    }
    if ((part->counter_attribute_flags & 
         PartsCounterAttribute_Flags::PREEMPTIVE_SPIKY) &&
        !((weapon->counter_resistance_flags & 
           AttackCounterResistance_Flags::PREEMPTIVE_SPIKY) ||
          attacker->badges_equipped.spike_shield)) {
        return 5;
    }
    if (part->part_attribute_flags & PartsAttribute_Flags::UNK_40) {
        return 4;
    }
    if (extra_params & 0x10'0000) {
        return 1;
    }
    if (CheckEvasionBadges(target)) {
        return 3;
    }
    
    if (BtlUnit_CheckStatus(target, StatusEffectType::DODGY) &&
        !BtlUnit_CheckStatus(target, StatusEffectType::SLEEP) &&
        !BtlUnit_CheckStatus(target, StatusEffectType::STOP) &&
        ttyd::system::irand(100) < 50) {
        return 6;
    }
    
    float accuracy = weapon->base_accuracy;
    if (BtlUnit_CheckStatus(attacker, StatusEffectType::DIZZY)) accuracy /= 2;
    if (ttyd::battle::g_BattleWork->stage_hazard_work.fog_active) accuracy /= 2;
    return ttyd::system::irand(100) < accuracy ? 1 : 2;
}

// Calculates an actor's current ATK given base power + special property flags.
int32_t CalculateAtkImpl(
    BattleWorkUnit* attacker, int32_t atk, int32_t sp_flags,
    bool enemy_scale, bool ice_power_bonus) {
    
    // Modify enemy ATK based on current scaling.
    if (enemy_scale) {
        int32_t altered_atk = atk;
        GetEnemyStats(
            attacker->current_kind, nullptr, &altered_atk, nullptr, 
            nullptr, nullptr, atk);
        atk = Clamp(altered_atk, 1, 99);
    }
    
    // Positive attack modifiers (badges and statuses).
    if (sp_flags & AttackSpecialProperty_Flags::BADGE_BUFFABLE) {
        atk += attacker->badges_equipped.all_or_nothing;
        atk += attacker->badges_equipped.power_plus;
        atk += attacker->badges_equipped.p_up_d_down;
        
        // Perfect Power.
        if (attacker->current_hp == attacker->max_hp) {
            atk += attacker->badges_equipped.unk_03;
        }
        
        // Danger / Peril badges (weaker than in original).
        // Will not activate if the actor is at 1/1 (max) HP.
        if (attacker->current_hp < attacker->max_hp &&
            attacker->current_hp <= options::GetPinchThresholdForMaxHp(
                attacker->max_hp, /* peril = */ false)) {
            atk += 1 * attacker->badges_equipped.power_rush;
        }
        if (attacker->current_hp < attacker->max_hp &&
            attacker->current_hp <= options::GetPinchThresholdForMaxHp(
                attacker->max_hp, /* peril = */ true)) {
            atk += 3 * attacker->badges_equipped.mega_rush;
        }
        if (ice_power_bonus) {
            atk += attacker->badges_equipped.ice_power;
        }
    }
    if (sp_flags & AttackSpecialProperty_Flags::STATUS_BUFFABLE) {
        int8_t strength = 0;
        BtlUnit_GetStatus(
            attacker, StatusEffectType::ATTACK_UP, nullptr, &strength);
        atk += strength;
        // ATK spell activated (doesn't check if Mario is attacker!)
        if (ttyd::battle::g_BattleWork->impending_merlee_spell_type == 1) {
            atk += 3;
        }
    }
    if (sp_flags & AttackSpecialProperty_Flags::CHARGE_BUFFABLE) {
        if (BtlUnit_CheckStatus(attacker, StatusEffectType::CHARGE)) {
            int8_t strength = 0;
            BtlUnit_GetStatus(
                attacker, StatusEffectType::CHARGE, nullptr, &strength);
            atk += strength;
            attacker->token_flags |= BattleUnitToken_Flags::CHARGE_EXPENDED;
        }
    }
    
    // Negative attack modifiers (badges and statuses).
    if (sp_flags & AttackSpecialProperty_Flags::BADGE_BUFFABLE) {
        atk -= attacker->badges_equipped.p_down_d_up;
        // Only drop by 1, regardless of number equipped.
        atk -= attacker->badges_equipped.hp_drain ? 1 : 0;
        atk -= attacker->badges_equipped.fp_drain ? 1 : 0;
    }
    if (sp_flags & AttackSpecialProperty_Flags::STATUS_BUFFABLE) {
        int8_t strength = 0;
        BtlUnit_GetStatus(
            attacker, StatusEffectType::ATTACK_DOWN, nullptr, &strength);
        atk += strength;
        
        if (BtlUnit_CheckStatus(attacker, StatusEffectType::BURN)) {
            atk -= 1;
        }
    }
    if (atk < 0) atk = 0;
    
    // Attack multiplier statuses.
    if (sp_flags & AttackSpecialProperty_Flags::STATUS_BUFFABLE) {
        if (BtlUnit_CheckStatus(attacker, StatusEffectType::HUGE)) {
            // 1.5x, rounded up.
            atk = ((atk * 3) + 1) / 2;
        } else if (atk > 1 &&
            BtlUnit_CheckStatus(attacker, StatusEffectType::TINY)) {
            // 0.5x, rounded down (unless base is 1).
            atk /= 2;
        }
    }
    
    return atk;
}

// Calculates an actor's current default DEF power.
int32_t CalculateDefImpl(
    BattleWorkUnit* target, BattleWorkUnitPart* part, int32_t element) {
    int32_t def = part->defense[element];
    
    // Modify enemy DEF based on current scaling.
    if (target->current_kind <= BattleUnitType::BONETAIL && def > 0 && def < 99) {
        int32_t altered_def = def;
        GetEnemyStats(
            target->current_kind, nullptr, nullptr, &altered_def, 
            nullptr, nullptr);
        def = Clamp(altered_def, 1, 99);
    }
    
    // Defense modifiers.
    def += target->badges_equipped.defend_plus;
    
    int8_t strength = 0;
    BtlUnit_GetStatus(target, StatusEffectType::DEFENSE_UP, nullptr, &strength);
    def += strength;
    BtlUnit_GetStatus(target, StatusEffectType::DEFENSE_DOWN, nullptr, &strength);
    def += strength;
    if (def < 0) def = 0;
    
    if (part->part_attribute_flags & PartsAttribute_Flags::WEAK_TO_ATTACK_FX_R) {
        def += target->unit_work[1];
        if (def < 0) def = 0;
    }
    
    return def;
}

// Replaces the original core damage function used by TTYD.
int32_t CalculateBaseDamage(
    BattleWorkUnit* attacker, BattleWorkUnit* target, BattleWorkUnitPart* part,
    BattleWeapon* weapon, uint32_t* unk0, uint32_t unk1) {
    auto* battleWork = ttyd::battle::g_BattleWork;
    // For shorter / more readable code lines, lol.
    const auto& sp = weapon->special_property_flags;
    
    if (target->token_flags & BattleUnitToken_Flags::UNK_2) {
        unk1 |= 0x1000;
    }
    
    int32_t element = weapon->element;
    if (attacker &&
        (sp & AttackSpecialProperty_Flags::IGNITES_IF_BURNED) &&
        BtlUnit_CheckStatus(attacker, StatusEffectType::BURN)) {
        element = AttackElement::FIRE;
    }
    *unk0 = unk1 | 0x1e;
    
    if (!weapon->damage_function) {
        return 0;
    }
    if (attacker &&
        (sp & AttackSpecialProperty_Flags::BADGE_BUFFABLE) &&
        attacker->badges_equipped.all_or_nothing > 0 &&
        !(battleWork->ac_manager_work.ac_result & 2) &&
        !(unk1 & 0x20000)) {
        return 0;
    }
    // Immune to damage/status.
    if (part->part_attribute_flags & PartsAttribute_Flags::UNK_2000_0000) {
        return 0;
    }
    
    int32_t atk = weapon->damage_function_params[0];
    auto weapon_fn = (WeaponDamageFn)weapon->damage_function;
    if (weapon_fn) atk = weapon_fn(attacker, weapon, target, part);
    
    int32_t should_scale_atk =
        attacker->current_kind <= BattleUnitType::BONETAIL
        && !(attacker->current_kind >= BattleUnitType::DOOPLISS_CH_8_FAKE_MARIO &&
             attacker->current_kind <= BattleUnitType::DOOPLISS_CH_8_MS_MOWZ)
        && !(weapon->target_property_flags & AttackTargetProperty_Flags::RECOIL_DAMAGE)
        && !weapon->item_id
        && atk > 0;
    bool ice_power_bonus = 
        part->part_attribute_flags & PartsAttribute_Flags::WEAK_TO_ICE_POWER;
    
    atk = CalculateAtkImpl(attacker, atk, sp, should_scale_atk, ice_power_bonus);
    
    // Special case handled outside base ATK function.
    
    // If an enemy is attacking the player, and not during the player attack
    // phase, queue pity Star Power restoration based on enemy's ATK.
    if (attacker->current_kind <= BattleUnitType::BONETAIL &&
        (target->current_kind == BattleUnitType::MARIO ||
         target->current_kind >= BattleUnitType::GOOMBELLA)) {
        if (atk > 0 && ttyd::battle::BattleGetSeq(battleWork, 4) != 0x400'0002) {
            auto& audience_work = battleWork->audience_work;
            int32_t current_audience = audience_work.current_audience_count_int;
                
            // Regen 10% of a unit SP per damage, up to 1% per audience member
            // (100 max); then increase the result by 0.25x per Pity Star (P).
            int32_t sp_regen = Min(atk * 10, Min(current_audience, 100));
            sp_regen = sp_regen * (target->badges_equipped.simplifier + 4) / 4;
            
            audience_work.impending_star_power += sp_regen;
        }
    }
    
    int32_t def = CalculateDefImpl(target, part, element);
    int32_t def_attr = part->defense_attr[element];
        
    if (battleWork->impending_merlee_spell_type == 2 &&
        target->current_kind == BattleUnitType::MARIO) {
        def += 3;
    }
    
    // If defense-piercing or elemental healing, set DEF to 0.
    if (sp & AttackSpecialProperty_Flags::DEFENSE_PIERCING or def_attr == 3) {
        def = 0;
    }
    
    switch (element) {
        case AttackElement::NORMAL:
            *unk0 = unk1 | 0x17;
            break;
        case AttackElement::FIRE:
            *unk0 = unk1 | 0x18;
            break;
        case AttackElement::ICE:
            *unk0 = unk1 | 0x1a;
            break;
        case AttackElement::EXPLOSION:
            *unk0 = unk1 | 0x19;
            break;
        case AttackElement::ELECTRIC:
            *unk0 = unk1 | 0x1b;
            break;
    }
    
    int32_t damage = atk - def;
    
    damage += target->badges_equipped.p_up_d_down;
    if (damage > 0) {
        if (sp & AttackSpecialProperty_Flags::TOT_INCREASING_BY_TARGET) {
            damage += attacker->hits_dealt_this_attack;
        }
        if (sp & AttackSpecialProperty_Flags::DIMINISHING_BY_HIT) {
            damage -= target->hp_damaging_hits_dealt;
        }
        if (sp & AttackSpecialProperty_Flags::DIMINISHING_BY_TARGET) {
            damage -= attacker->hits_dealt_this_attack;
        }
        if (damage < 1) damage = 1;
    }

    // Defending now reduces damage taken directly, rather than adding DEF.
    if (target->status_flags & BattleUnitStatus_Flags::DEFENDING) { 
        // Toughen Up buffs the strength of the "Defend" action.
        damage -= (1 + target->badges_equipped.super_charge);
    }
    // Guarding also reduces damage taken directly (as well as Damage Dodge).
    if (unk1 & 0x40000) {  // guarding
        damage -= (1 + target->badges_equipped.damage_dodge);
    }

    if (element == AttackElement::FIRE) {
        damage -= target->badges_equipped.ice_power;
    }
    damage -= target->badges_equipped.p_down_d_up;
    damage *= (target->badges_equipped.double_pain + 1);
    
    int32_t last_stands = target->badges_equipped.last_stand;
    // Will not activate if the actor is at 1/1 (max) HP.
    if (target->current_hp < target->max_hp &&
        target->current_hp <= options::GetPinchThresholdForMaxHp(
            target->max_hp, /* peril = */ false) &&
        last_stands > 0) {
        damage = (damage + last_stands) / (last_stands + 1);
    }
    
    bool freeze_broken = false;
    // Double the damage of a hit if it will break Freeze status.
    if (BtlUnit_CheckStatus(target, StatusEffectType::FREEZE) &&
        element != AttackElement::ICE && weapon->freeze_time == 0) {
        damage *= 2;

        freeze_broken = true;
    }
    
    switch (def_attr) {
        case 1:
            damage += 1;
            break;
        case 2:
            damage = 0;
            break;
        case 4:
            // New Iron Clefts never take more than one damage.
            if (damage > 1) damage = 1;
            break;
    }
    
    if (def_attr == 3) {
        damage *= -1;
        *unk0 |= 0x2000;
    } else if (damage < 1) {
        damage = 0;
        *unk0 = unk1 | 0x1e;
    } else if (
        (unk1 & 0x100) &&
        (sp & AttackSpecialProperty_Flags::UNKNOWN_0x8000 ||
         !(target->attribute_flags & BattleUnitAttribute_Flags::UNK_100))) {
        *unk0 |= 0x10000;
    }

    if (freeze_broken && damage >= 20) {
        AchievementsManager::MarkCompleted(AchievementId::MISC_FROZEN_20);
    }

    // "Critical hit" effects ignore all damage calculation; deal half of
    // current HP if guarded, or 1 damage if guarded.
    if (sp & AttackSpecialProperty_Flags::TOT_CRITICAL_HIT) {
        if (unk1 & 0x40000) {
            damage = 1;
        } else {
            damage = (target->current_hp + 1) / 2;
        }
    }
    
    return damage;
}

// Replaces the original core FP damage function used by TTYD.
int32_t CalculateBaseFpDamage(
    BattleWorkUnit* attacker, BattleWorkUnit* target, BattleWorkUnitPart* part,
    BattleWeapon* weapon, uint32_t* unk0, uint32_t unk1) {
    auto* battleWork = ttyd::battle::g_BattleWork;
    // For shorter / more readable code lines, lol.
    const auto& sp = weapon->special_property_flags;
    int32_t result = 0;
    
    if (!weapon->fp_damage_function) {
        return 0;
    }
    if (attacker &&
        (sp & AttackSpecialProperty_Flags::BADGE_BUFFABLE) &&
        attacker->badges_equipped.all_or_nothing > 0 &&
        !(battleWork->ac_manager_work.ac_result & 2) &&
        !(unk1 & 0x20000)) {
        return 0;
    }
    // Immune to damage/status.
    if (part->part_attribute_flags & PartsAttribute_Flags::UNK_2000_0000) {
        return 0;
    }
    
    if (!(*unk0 & 0x2000)) {
        auto weapon_fn = (WeaponDamageFn)weapon->fp_damage_function;
        if (weapon_fn) result = weapon_fn(attacker, weapon, target, part);
        
        // Modify base FP damage for enemies based on current scaling.
        if (attacker->current_kind <= BattleUnitType::BONETAIL
            && !weapon->item_id && result > 0) {
            int32_t altered_atk = result;
            GetEnemyStats(
                attacker->current_kind, nullptr, &altered_atk, nullptr,
                nullptr, nullptr, result);
            result = Clamp(altered_atk, 1, 99);
        }
        
        // If guarded, reduce damage by one.
        if (unk1 & 0x40000) --result;
        
        if (result < 1) result = 0;
        if (result > 1 && ((*unk0 & 0xff) == 0x1e)) {
            *unk0 = (*unk0 & 0xffffff00U) | 0x17;
        }
    }
    return result;
}

void CalculateCounterDamage(
    CounterattackWork* cw, BattleWorkUnit* attacker, BattleWorkUnit* target,
    BattleWorkUnitPart* part, int32_t damage_dealt) {
    // For Payback, Hold Fast and Return Postage, return the full damage.
    if (cw->payback_or_poison_countered) cw->total_damage += damage_dealt;
    if (cw->hold_fast_countered) cw->total_damage += damage_dealt;
    if (cw->return_postage_countered) cw->total_damage += damage_dealt;
    
    // For Bob-omb counters, deal the damage the explosion would have dealt.
    // (Note that if the Bob-omb has Payback status that will take precedent).
    if (cw->counter_type_1 == 0x31 || cw->counter_type_1 == 0x32) {
        int32_t part_id = BtlUnit_GetBodyPartsId(attacker);
        auto* attacker_part = BtlUnit_GetPartsPtr(attacker, part_id);
        auto* weapon = &tot::custom::unitBobOmb_weaponBomb;
        uint32_t unk0 = 0U;
        cw->total_damage = ttyd::battle_damage::BattleCalculateDamage(
            target, attacker, attacker_part, weapon, &unk0, 0x20000U);
    }
}

// Replaces the original TTYD logic in BattleAudience_ApRecoveryBuild.
void ApplyAttackSuccessBonuses(SpBonusInfo* bonus_info) {
    auto* battleWork = ttyd::battle::g_BattleWork;
    auto& audience_work = battleWork->audience_work;
    auto& bingo_work = battleWork->bingo_work;

    // Don't give SP or bingo card bonuses for First Strikes.
    if (battleWork->turn_count < 1) return;

    if (!audience_work.impending_bonus_info) {
        audience_work.impending_bonus_info =
            (SpBonusInfo*)ttyd::battle::BattleAlloc(sizeof(SpBonusInfo));
    }
    memcpy(audience_work.impending_bonus_info, bonus_info, sizeof(SpBonusInfo));

    BattleWorkUnit* mario = ttyd::battle::BattleGetMarioPtr(battleWork);
    BattleWorkUnit* party = ttyd::battle::BattleGetPartyPtr(battleWork);
    
    // Calculate the square root of the audience value used for attacks.
    float total_audience_value = 0.0f;
    auto* audience_kind_tbl = ttyd::battle_audience_kind_data::audience_kind;
    for (int32_t i = 0; i < 13; ++i) {
        total_audience_value += 
            ttyd::battle_audience::BattleAudience_GetPPAudienceNumKind(i) * 
            audience_kind_tbl[i].attack_sp_multiplier;
    }
    float sqrt_audience_value = ttyd::_core_sqrt(total_audience_value);
    
    // Multipliers for AC difficulty (if successful) / Stylish completion.
    float ac_diff_multiplier = bonus_info->ac_success_multiplier;
    float stylish_multiplier = bonus_info->stylish_multiplier;

    // Multiplier of 1x if one of AC or Stylish was successful, 2x if both were.
    float success_multiplier = 0.0f;
    if (ac_diff_multiplier > 0) success_multiplier += 1.0f;
    if (stylish_multiplier > 0) success_multiplier += 1.0f;

    // Bonus multipliers are now additive instead of multiplicative.
    float bonus_multiplier = 1.0f;
    // Mario Danger/Peril adds 50%, 100% (slight nerf to original 2x, 3x).
    if (mario->status_flags & BattleUnitStatus_Flags::IN_PERIL) {
      bonus_multiplier += 1.0f;
    } else if (mario->status_flags & BattleUnitStatus_Flags::IN_DANGER) {
      bonus_multiplier += 0.5f;
    }
    // Partner Danger/Peril adds 50%, 100%.
    if (party) {
      if (party->status_flags & BattleUnitStatus_Flags::IN_PERIL) {
          bonus_multiplier += 1.0f;
      } else if (party->status_flags & BattleUnitStatus_Flags::IN_DANGER) {
          bonus_multiplier += 0.5f;
      }
    }
    // Having an active BINGO frenzy adds 100%, or 200% for Shine bingo.
    if (bingo_work.active_bingo_turn_count > 0) {
        bonus_multiplier += bingo_work.active_bingo_sp_multiplier - 1.0f;
    }

    audience_work.impending_star_power +=
        sqrt_audience_value * (ac_diff_multiplier + stylish_multiplier)
        * success_multiplier * bonus_multiplier;

    if (ttyd::system::irand(100) < (uint32_t)bonus_info->bingo_slot_chance) {
        ttyd::battle_break_slot::BattleBreakSlot_PointInc();
    }
}

void HandleAppealReaction(BattleWorkAudience* aud_work, BattleWorkUnit* unit) {
    int32_t aud_size = ttyd::battle_audience::BattleAudience_GetPPAudienceNum();
    if (aud_size > 0) {
        // Rebalanced regen: 40-80 instead of 25-75 base, + 40 per Super Appeal.
        int32_t sp_regen = 40 + (aud_size / 5);
        sp_regen += unit->badges_equipped.super_appeal * 40;
        aud_work->impending_star_power += sp_regen;

        ttyd::battle_audience::BattleAudienceSoundCheer(180, 60);
        ttyd::battle_audience::BattleAudienceSoundClap(180, 60);
        ttyd::battle_audience::BattleAudienceJoy(0);
    }
    ttyd::battle_audience::BattleAudienceAddTargetNum(
        aud_work->base_target_audience * 0.05f, 0.0f);
}

void SpendAndIncrementPartySwitchCost() {
    if (g_PartySwitchPlayerInitiated) {
        // Spend FP (and track total FP spent in BattleActRec).
        auto* battleWork = ttyd::battle::g_BattleWork;
        auto* unit = ttyd::battle::BattleGetMarioPtr(battleWork);
        
        int32_t switch_fp_cost = g_PartySwitchNextFpCost;
        int32_t fp = ttyd::battle_unit::BtlUnit_GetFp(unit);
        ttyd::battle_unit::BtlUnit_SetFp(unit, fp - switch_fp_cost);
        ttyd::battle_actrecord::BtlActRec_AddPoint(
            &battleWork->act_record_work.mario_fp_spent, switch_fp_cost);
        
        // Increment cost of next use, until it hits 3 FP, or the player's max.
        if (switch_fp_cost < 3 && switch_fp_cost < unit->max_fp) {
            ++g_PartySwitchNextFpCost;
        }
    }
}

void ReorderAndFilterWeaponTargets() {
    auto& twork = ttyd::battle::g_BattleWork->weapon_targets_work;

    // If weapon is 'select side', filter targets that don't match side.
    if (twork.num_targets > 1 &&
        twork.weapon_target_class_flags & AttackTargetClass_Flags::SELECT_SIDE) {
        int8_t new_indices[74] = { 0 };
        int32_t new_num_targets = 0;

        for (int32_t i = 0; i < twork.num_targets; ++i) {
            if (battle::CheckOnSelectedSide(i)) {
                new_indices[new_num_targets] = twork.target_indices[i];
                ++new_num_targets;
            }
        }

        for (int32_t i = 0; i < new_num_targets; ++i) {
            twork.target_indices[i] = new_indices[i];
        }
        twork.num_targets = new_num_targets;
        twork.current_target = 0;
    }

    // For certain multitarget weapons, reorder targets so the attacker
    // is targeted last, to make sure the attack doesn't end prematurely.
    switch (twork.weapon->item_id) {
        case ItemType::TRADE_OFF:
        case ItemType::LOVE_PUDDING:
        case ItemType::PEACH_TART:
            for (int32_t i = 0; i < twork.num_targets - 1; ++i) {
                int32_t target_unit_idx = 
                    twork.targets[twork.target_indices[i]].unit_idx;
                if (target_unit_idx == twork.attacker_idx) {
                    // Swap with last target.
                    int32_t tmp = twork.target_indices[i];
                    twork.target_indices[i] = 
                        twork.target_indices[twork.num_targets - 1];
                    twork.target_indices[twork.num_targets - 1] = tmp;
                    break;
                }
            }
            break;
    }
}

void ApplyFixedPatches() {
    // Override Action Command difficulty with fixed option.
    g_BattleActionCommandSetDifficulty_trampoline = mod::hookFunction(
        ttyd::battle_ac::BattleActionCommandSetDifficulty,
        [](BattleWork* battleWork, BattleWorkUnit* unit, int32_t base) {
            // Replace original logic entirely.
            int32_t difficulty = getActionCommandDifficulty();
            battleWork->ac_manager_work.base_ac_difficulty = difficulty;
            battleWork->ac_manager_work.ac_difficulty = difficulty;
        });
    // Override Action Command difficulty for guards.
    mod::writeBranchPair(
        reinterpret_cast<void*>(g_BattleActionCommandCheckDefence_GetDifficulty_BH),
        reinterpret_cast<void*>(g_BattleActionCommandCheckDefence_GetDifficulty_EH),
        reinterpret_cast<void*>(StartGetGuardDifficulty),
        reinterpret_cast<void*>(BranchBackGetGuardDifficulty));
    
    // Handle Superguard cost option & party-vs.party guardable moves.
    g_BattleActionCommandCheckDefence_trampoline = mod::hookFunction(
        ttyd::battle_ac::BattleActionCommandCheckDefence,
        [](BattleWorkUnit* unit, BattleWeapon* weapon) {
            const int32_t sp_cost =
                g_Mod->state_.GetOption(OPTNUM_SUPERGUARD_SP_COST);
            
            int8_t guard_frames[7];
            int8_t superguard_frames[7];
            bool restore_guard_frames = false;
            bool restore_superguard_frames = false;

            if ((weapon->special_property_flags &
                AttackSpecialProperty_Flags::TOT_PARTY_UNGUARDABLE) &&
                !GetSWByte(GSW_Battle_DooplissMove)) {
                // Disable guarding AND Superguarding during party-vs.-party
                // moves, unless they're being used by Doopliss.
                restore_guard_frames = true;
                memcpy(guard_frames, ttyd::battle_ac::guard_frames, 7);
                for (int32_t i = 0; i < 7; ++i) {
                    ttyd::battle_ac::guard_frames[i] = 0;
                }

                restore_superguard_frames = true;
                memcpy(superguard_frames, ttyd::battle_ac::superguard_frames, 7);
                for (int32_t i = 0; i < 7; ++i) {
                    ttyd::battle_ac::superguard_frames[i] = 0;
                }
            } else if (ttyd::mario_pouch::pouchGetAP() < sp_cost) {
                // Temporarily disable Superguarding if SP is too low.
                restore_superguard_frames = true;
                memcpy(superguard_frames, ttyd::battle_ac::superguard_frames, 7);
                for (int32_t i = 0; i < 7; ++i) {
                    ttyd::battle_ac::superguard_frames[i] = 0;
                }
            }

            const int32_t defense_result =
                g_BattleActionCommandCheckDefence_trampoline(unit, weapon);

            if (defense_result == 5) {
                // Successful Superguard, subtract SP and track in play stats.
                ttyd::mario_pouch::pouchAddAP(-sp_cost);
                g_Mod->state_.ChangeOption(STAT_RUN_SUPERGUARDS);
                g_Mod->state_.ChangeOption(STAT_PERM_SUPERGUARDS);
                if (g_Mod->state_.GetOption(STAT_PERM_SUPERGUARDS) >= 100)
                    AchievementsManager::MarkCompleted(
                        AchievementId::AGG_SUPERGUARD_100);
            }

            if (restore_guard_frames) {
                memcpy(ttyd::battle_ac::guard_frames, guard_frames, 7);
            }
            if (restore_superguard_frames) {
                memcpy(ttyd::battle_ac::superguard_frames, superguard_frames, 7);
            }

            return defense_result;
        });

    // Run additional logic at start of BattleCheckDamage.
    g_BattleCheckDamage_trampoline = mod::hookFunction(
        ttyd::battle_damage::BattleCheckDamage, [](
            BattleWorkUnit* attacker, BattleWorkUnit* target, 
            BattleWorkUnitPart* part, BattleWeapon* weapon, 
            uint32_t extra_params) {
                if (attacker) {
                    attacker->last_attacker_weapon = weapon;
                    attacker->last_target_weapon = nullptr;
                    attacker->last_target_weapon_cr_flags = 0;
                }
                if (target) {
                    target->last_attacker_weapon = nullptr;
                    target->last_target_weapon = weapon;
                    if (weapon)
                        target->last_target_weapon_cr_flags = 
                            weapon->counter_resistance_flags;
                }
                return g_BattleCheckDamage_trampoline(
                    attacker, target, part, weapon, extra_params);
            });
    
    // Replacing several core damage / status infliction functions.
    g_BattlePreCheckDamage_trampoline = mod::hookFunction(
        ttyd::battle_damage::BattlePreCheckDamage, [](
            BattleWorkUnit* attacker, BattleWorkUnit* target, 
            BattleWorkUnitPart* part, BattleWeapon* weapon, 
            uint32_t extra_params) {
                // Replaces vanilla logic completely.
                return PreCheckDamage(
                    attacker, target, part, weapon, extra_params);
            });
    g_BattleSetStatusDamageFromWeapon_trampoline = mod::hookFunction(
        ttyd::battle_damage::BattleSetStatusDamageFromWeapon, [](
            BattleWorkUnit* attacker, BattleWorkUnit* target, 
            BattleWorkUnitPart* part, BattleWeapon* weapon, 
            uint32_t extra_params) {
                // Replaces vanilla logic completely.
                return GetStatusDamageFromWeapon(
                    attacker, target, part, weapon, extra_params);
            });
    g_BtlUnit_CheckRecoveryStatus_trampoline = mod::hookFunction(
        ttyd::battle_unit::BtlUnit_CheckRecoveryStatus, [](
            BattleWorkUnit* unit, int8_t status_type) {
                // Replaces vanilla logic completely.
                return StatusEffectTick(unit, status_type);
            });
    g_BattleCalculateDamage_trampoline = mod::hookFunction(
        ttyd::battle_damage::BattleCalculateDamage, [](
            BattleWorkUnit* attacker, BattleWorkUnit* target,
            BattleWorkUnitPart* target_part, BattleWeapon* weapon,
            uint32_t* unk0, uint32_t unk1) {
            // Replaces vanilla logic completely.
            int32_t damage = CalculateBaseDamage(
                attacker, target, target_part, weapon, unk0, unk1);
                
            // Increment HP/FP Drain counter if the move can be damaging.
            if (weapon->damage_function) {
                ++attacker->total_damage_dealt_this_attack;
            }
            
            // Randomize damage dealt, if option enabled.
            int32_t damage_scale = g_Mod->state_.GetOption(OPT_RANDOM_DAMAGE);
            if (damage_scale != 0) {
                // Generate a number from -25 to 25 in increments of 5.
                int32_t scale = (ttyd::system::irand(11) - 5) * 5;
                // Scale by 1x or 2x based on the setting.
                scale *= damage_scale;
                // Round damage modifier away from 0.
                damage += (damage * scale + (scale > 0 ? 50 : -50)) / 100;
            }
            
            return damage;
        });
    g_BattleCalculateFpDamage_trampoline = mod::hookFunction(
        ttyd::battle_damage::BattleCalculateFpDamage, [](
            BattleWorkUnit* attacker, BattleWorkUnit* target,
            BattleWorkUnitPart* target_part, BattleWeapon* weapon,
            uint32_t* unk0, uint32_t unk1) {
            // Replaces vanilla logic completely.
            int32_t damage = CalculateBaseFpDamage(
                attacker, target, target_part, weapon, unk0, unk1);
            
            // Randomize damage dealt, if option enabled.
            int32_t damage_scale = g_Mod->state_.GetOption(OPT_RANDOM_DAMAGE);
            if (damage_scale != 0) {
                // Generate a number from -25 to 25 in increments of 5.
                int32_t scale = (ttyd::system::irand(11) - 5) * 5;
                // Scale by 1x or 2x based on the setting.
                scale *= damage_scale;
                // Round damage modifier away from 0.
                damage += (damage * scale + (scale > 0 ? 50 : -50)) / 100;
            }
            
            return damage;
        });
        
    // Replace damage calculation for counters to make Payback, Hold Fast
    // and Return Postage deal 1x damage, and make Bob-omb counters
    // deal the same damage that the explosion normally would have.
    mod::writeBranchPair(
        reinterpret_cast<void*>(g_BattleCheckDamage_CalculateCounterDamage_BH),
        reinterpret_cast<void*>(g_BattleCheckDamage_CalculateCounterDamage_EH),
        reinterpret_cast<void*>(StartCalculateCounterDamage),
        reinterpret_cast<void*>(BranchBackCalculateCounterDamage));
        
    // Skip explosion KO logic for regular / Hyper Bob-ombs.
    mod::writeBranch(
        reinterpret_cast<void*>(g_BattleCheckDamage_CheckExplosiveKO_BH),
        reinterpret_cast<void*>(StartCheckExplosiveKO));
    mod::writeBranch(
        reinterpret_cast<void*>(BranchBackCheckExplosiveKO),
        reinterpret_cast<void*>(g_BattleCheckDamage_CheckExplosiveKO_EH));
    mod::writeBranch(
        reinterpret_cast<void*>(ConditionalBranchCheckExplosiveKO),
        reinterpret_cast<void*>(g_BattleCheckDamage_CheckExplosiveKO_CH1));
            
    // Track damage taken.
    g_BattleDamageDirect_trampoline = mod::hookFunction(
        ttyd::battle_damage::BattleDamageDirect, [](
            int32_t unit_idx, BattleWorkUnit* target, BattleWorkUnitPart* part,
            int32_t damage, int32_t fp_damage, uint32_t unk0, 
            uint32_t damage_pattern, uint32_t unk1) {

            auto* battleWork = ttyd::battle::g_BattleWork;

            // Save original damage so elemental healing still works.
            const int32_t original_damage = damage;
            // Track damage taken, if target is player/enemy and damage > 0.
            if (target->current_kind == BattleUnitType::MARIO ||
                target->current_kind >= BattleUnitType::GOOMBELLA) {
                if (damage < 0) damage = 0;
                if (damage > 99) damage = 99;
                g_Mod->state_.ChangeOption(STAT_RUN_PLAYER_DAMAGE, damage);
                g_Mod->state_.ChangeOption(STAT_PERM_PLAYER_DAMAGE, damage);
            } else if (target->current_kind <= BattleUnitType::BONETAIL) {
                if (damage < 0) damage = 0;
                if (damage > 99) damage = 99;
                g_Mod->state_.ChangeOption(STAT_RUN_ENEMY_DAMAGE, damage);
                g_Mod->state_.ChangeOption(STAT_PERM_ENEMY_DAMAGE, damage);
                if (g_Mod->state_.GetOption(STAT_PERM_ENEMY_DAMAGE) >= 15000) {
                    AchievementsManager::MarkCompleted(
                        AchievementId::AGG_DAMAGE_15000);
                }

                // Track direct damage from Infatuated attackers.
                if (unit_idx >= 0 && 
                    battleWork->battle_units[unit_idx]->alliance == 0 &&
                    battleWork->battle_units[unit_idx]->true_kind 
                        <= BattleUnitType::BONETAIL) {
                    g_Mod->state_.ChangeOption(
                        STAT_RUN_INFATUATE_DAMAGE, damage);
                }

                // Check for Superguarded dragon bites.
                if (unit_idx == -5 && unk0 == 0x137 &&
                    target->last_attacker_weapon == &custom::unitDragon_weaponBite) {
                    AchievementsManager::MarkCompleted(AchievementId::MISC_SUPERGUARD_BITE);
                    SetSWF(GSWF_Battle_Hooktail_BiteReactionSeen);
                }
            }

            // If Mario is damaged, store the unit type that's hitting him.
            if (target->current_kind == BattleUnitType::MARIO) {
                int32_t attacker_type = 0;
                if (unit_idx >= 0) {
                    auto* unit = battleWork->battle_units[unit_idx];
                    if (unit &&
                        unit->current_kind != BattleUnitType::MARIO &&
                        unit->current_kind != BattleUnitType::SYSTEM) {
                        attacker_type = unit->current_kind;
                    }
                }
                g_Mod->state_.SetOption(STAT_PERM_LAST_ATTACKER, attacker_type);
            }

            // Run normal damage logic.
            g_BattleDamageDirect_trampoline(
                unit_idx, target, part, original_damage, fp_damage, 
                unk0, damage_pattern, unk1);
        });

    // Attack FX changes.
    g_BattleCheckPikkyoro_trampoline = mod::hookFunction(
        ttyd::battle_damage::BattleCheckPikkyoro, [](
            BattleWeapon* weapon, uint32_t* flags) {
            // Completely replace existing logic to pick a sound.
            if (!(weapon->special_property_flags & 
                AttackSpecialProperty_Flags::MAKES_ATTACK_FX_SOUND)) return;
            
            if (int32_t id = CosmeticsManager::PickActiveFX(); id > 0) {
                *flags |= id * 0x100'0000;
            }
        });
    // Replace BattleDamageDirect logic that reads flags for picked sound.
    mod::writeBranch(
        reinterpret_cast<void*>(g_BattleDamageDirect_CheckPlayAttackFX_BH),
        reinterpret_cast<void*>(StartCheckPlayAttackFX));
    mod::writeBranch(
        reinterpret_cast<void*>(BranchBackCheckPlayAttackFX),
        reinterpret_cast<void*>(g_BattleDamageDirect_CheckPlayAttackFX_EH));
    mod::writeBranch(
        reinterpret_cast<void*>(ConditionalBranchCheckPlayAttackFX),
        reinterpret_cast<void*>(g_BattleDamageDirect_CheckPlayAttackFX_CH1));

    // Add support for Snow Whirled-like variant of button pressing AC.
    // Override choosing buttons:
    mod::writeBranchPair(
        reinterpret_cast<void*>(g_battleAcMain_ButtonDown_ChooseButtons_BH),
        reinterpret_cast<void*>(StartButtonDownChooseButtons),
        reinterpret_cast<void*>(BranchBackButtonDownChooseButtons));
    // Don't end attack early on wrong button.
    mod::writeBranch(
        reinterpret_cast<void*>(g_battleAcMain_ButtonDown_WrongButton_BH),
        reinterpret_cast<void*>(StartButtonDownWrongButton));
    mod::writeBranch(
        reinterpret_cast<void*>(BranchBackButtonDownWrongButton),
        reinterpret_cast<void*>(g_battleAcMain_ButtonDown_WrongButton_EH));
    mod::writeBranch(
        reinterpret_cast<void*>(ConditionalBranchButtonDownWrongButton),
        reinterpret_cast<void*>(g_battleAcMain_ButtonDown_WrongButton_CH1));
    // Reset buttons instead of ending attack when completed:
    mod::writeBranch(
        reinterpret_cast<void*>(g_battleAcMain_ButtonDown_CheckComplete_BH),
        reinterpret_cast<void*>(StartButtonDownCheckComplete));
    mod::writeBranch(
        reinterpret_cast<void*>(BranchBackButtonDownCheckComplete),
        reinterpret_cast<void*>(g_battleAcMain_ButtonDown_CheckComplete_EH));
    mod::writeBranch(
        reinterpret_cast<void*>(ConditionalBranchButtonDownCheckComplete),
        reinterpret_cast<void*>(g_battleAcMain_ButtonDown_CheckComplete_CH1));
    // Override what the 'success' criteria for the move is.
    g_battleAcMain_ButtonDown_trampoline = mod::hookFunction(
        ttyd::ac_button_down::battleAcMain_ButtonDown,
        [](BattleWork* battleWork) {
            // Run original code.
            uint32_t result = g_battleAcMain_ButtonDown_trampoline(battleWork);
            if (battleWork->ac_manager_work.ac_state == 1002 &&    
                battleWork->ac_manager_work.ac_params[4] == -417U) {
                // Override AC result.
                battleWork->ac_manager_work.ac_result =
                    battleWork->ac_manager_work.ac_output_params[1] >= 4 ? 2 : 0;
            }
            return result;
        });
            
    // Add support for "permanent statuses" (turn count >= 100):
    // Skip drawing status icon if 100 or over:
    mod::writeBranch(
        reinterpret_cast<void*>(
            g_battle_status_icon_SkipIconForPermanentStatus_BH),
        reinterpret_cast<void*>(StartStatusIconDisplay));
    mod::writeBranch(
        reinterpret_cast<void*>(BranchBackStatusIconDisplay),
        reinterpret_cast<void*>(
            g_battle_status_icon_SkipIconForPermanentStatus_EH));
    mod::writeBranch(
        reinterpret_cast<void*>(ConditionalBranchStatusIconDisplay),
        reinterpret_cast<void*>(
            g_battle_status_icon_SkipIconForPermanentStatus_CH1));
    
    // Toggle off Scoped status, and force freeze-break anytime a Frozen enemy
    // is hit by a non-Ice attack.
    mod::writeBranchPair(
        reinterpret_cast<void*>(g_BattleCheckDamage_AlwaysFreezeBreak_BH),
        reinterpret_cast<void*>(StartToggleScopedAndCheckFreezeBreak),
        reinterpret_cast<void*>(BranchBackToggleScopedAndCheckFreezeBreak));
            
    // Track the total damage dealt by Poison status.
    mod::writeBranchPair(
        reinterpret_cast<void*>(g_btlseqTurn_TrackPoisonDamage_BH),
        reinterpret_cast<void*>(StartTrackPoisonDamage),
        reinterpret_cast<void*>(BranchBackTrackPoisonDamage));
            
    // Skip drawing Huge / Tiny status arrows when inflicted.
    mod::writePatch(
        reinterpret_cast<void*>(g_BattleSetStatusDamage_Patch_SkipHugeTinyArrows),
        0x2c19000cU /* cmpwi r25, 12 */);

    // Make Feeling Fine work on all negative statuses and not Electric.
    auto* ff_table = (int32_t*)g_BattleSetStatusDamage_FeelingFine_SwitchTable;
    for (int32_t i = 0; i < StatusEffectType::STATUS_MAX; ++i) {
        switch (i) {
            case StatusEffectType::SLEEP:
            case StatusEffectType::STOP:
            case StatusEffectType::DIZZY:
            case StatusEffectType::CONFUSE:
            case StatusEffectType::POISON:
            case StatusEffectType::BURN:
            case StatusEffectType::FREEZE:
            case StatusEffectType::TINY:
            case StatusEffectType::ATTACK_DOWN:
            case StatusEffectType::DEFENSE_DOWN:
            case StatusEffectType::SLOW:
            case StatusEffectType::OHKO:
                ff_table[i] = g_BattleSetStatusDamage_Patch_FeelingFineYesCase;
                break;
            default:
                ff_table[i] = g_BattleSetStatusDamage_Patch_FeelingFineNoCase;
                break;
        }
    }

    // Make unequipping items from enemies no longer revert their max FP.
    mod::writePatch(
        reinterpret_cast<void*>(g_BtlUnit_EquipItem_Patch_ReviseHpFp),
        0x60000000U /* nop */);

    // Make midbosses regain their Huge status and flipped shell/bomb-flippable
    // enemies retain their stunned state when KOed with a Life Shroom.
    g_BtlUnit_ClearStatus_trampoline = mod::hookFunction(
        ttyd::battle_unit::BtlUnit_ClearStatus, [](BattleWorkUnit* unit) {

            int32_t flipped_turns = 0;
            switch (unit->true_kind) {
                case BattleUnitType::KOOPS:
                    // Koops already correctly revives as active.
                    break;
                default:
                    flipped_turns = unit->flipped_turns;
                    break;
            }

            // Run original logic.
            g_BtlUnit_ClearStatus_trampoline(unit);

            // Re-apply permanent huge status.
            if (unit->status_flags & BattleUnitStatus_Flags::MIDBOSS) {
                unit->size_change_strength = 1;
                unit->size_change_turns = 100;
            }
            // Re-apply flipped turn count.
            if (flipped_turns > 0) {
                unit->flipped_turns = flipped_turns;
            }
        });
        
    // Change frame windows for guarding / Superguarding at different levels
    // of Simplifiers / Unsimplifiers to be more symmetric.
    const int8_t kGuardFrames[] =     { 12, 10, 9, 8, 7, 6, 5, 0 };
    const int8_t kSuperguardFrames[]  = { 5, 4, 4, 3, 2, 2, 1, 0 };
    mod::writePatch(
        ttyd::battle_ac::guard_frames, kGuardFrames, sizeof(kGuardFrames));
    mod::writePatch(
        ttyd::battle_ac::superguard_frames, kSuperguardFrames, 
        sizeof(kSuperguardFrames));
        
    // Override the default target audience size.
    mod::writeBranchPair(
        reinterpret_cast<void*>(g_BattleAudience_SetTargetAmount_BH),
        reinterpret_cast<void*>(StartSetTargetAudienceCount),
        reinterpret_cast<void*>(BranchBackSetTargetAudienceCount));

    // Replace the logic for applying attack SP / Bingo card bonuses.
    g_BattleAudience_ApRecoveryBuild_trampoline = mod::hookFunction(
        ttyd::battle_audience::BattleAudience_ApRecoveryBuild,
        [](SpBonusInfo* bonus_info) {
            // Replaces vanilla logic completely.
            ApplyAttackSuccessBonuses(bonus_info);
        });

    // Replace the logic for handling an Appeal action.
    g_BattleAudience_Case_Appeal_trampoline = mod::hookFunction(
        ttyd::battle_audience::BattleAudience_Case_Appeal,
        [](BattleWorkUnit* unit) {
            // Replaces vanilla logic completely.
            auto* aud_work = &ttyd::battle::g_BattleWork->audience_work;
            HandleAppealReaction(aud_work, unit);
        });

    // Add replacement for vanilla logic about Hammer Bros. throwing hammer if
    // Mario misses a Hammer attack's Action Command.
    g_BattleAudience_Case_ActionCommandBad_trampoline = mod::hookFunction(
        ttyd::battle_audience::BattleAudience_Case_ActionCommandBad,
        [](BattleWeapon* weapon) {
            // Call original logic.
            g_BattleAudience_Case_ActionCommandBad_trampoline(weapon);
            // Check for whether the attack was a Hammer, and queue event if so.
            if (weapon->damage_function == (void*)GetWeaponPowerFromSelectedLevel &&
                weapon->damage_function_params[7] >= MoveType::HAMMER_BASE &&
                weapon->damage_function_params[7] <= MoveType::HAMMER_BASE + 7) {
                ttyd::battle_audience::BattleAudienceAddPhaseEvtList(7);
            }
        });
        
    // Disable stored EXP at all levels.
    g_BattleStoreExp_trampoline = mod::hookFunction(
        ttyd::battle::BattleStoreExp, [](BattleWork* work, int32_t exp){});
    // Disable EXP gain, including pity EXP point, at all levels.
    mod::writePatch(
        reinterpret_cast<void*>(g_btlseqEnd_Patch_CheckDisableExpLevel),
        0x2c000000U /* cmpwi r0, (level) 0 */);
    // Have coin gfx pop out of enemies instead of EXP when defeated.
    mod::writePatch(
        reinterpret_cast<void*>(g_effStarPointDisp_Patch_SetIconId),
        0x38a00193U /* li r5, IconType::COIN */);
        
    // Equipping new badges.
    g__EquipItem_trampoline = mod::hookFunction(
        ttyd::battle::_EquipItem, 
        [](BattleWorkUnit* unit, uint32_t flags, int32_t item) {
            // Mario or enemies:
            if ((flags & 4) == 0) {
                if (item == ItemType::TOT_PERFECT_POWER) {
                    ++unit->badges_equipped.unk_03;
                }
                if (item == ItemType::TOT_PITY_STAR) {
                    ++unit->badges_equipped.simplifier;
                }
            }
            // Partner or enemies:
            if ((flags & 2) == 0) {
                if (item == ItemType::TOT_PERFECT_POWER_P) {
                    ++unit->badges_equipped.unk_03;
                }
                if (item == ItemType::TOT_PITY_STAR_P) {
                    ++unit->badges_equipped.simplifier;
                }
            }
            
            // Enable Triple Dip action for any # of copies of Double Dip (P).
            if ((flags & 6) == 2 && item == ItemType::DOUBLE_DIP) {
                unit->badges_equipped.double_dip = 1;
                unit->badges_equipped.triple_dip = 1;
                return;
            }
            if ((flags & 6) == 4 && item == ItemType::DOUBLE_DIP_P) {
                unit->badges_equipped.double_dip = 1;
                unit->badges_equipped.triple_dip = 1;
                return;
            }
            
            // Disable badges.
            if (item == ItemType::SIMPLIFIER || item == ItemType::UNSIMPLIFIER) {
                return;
            }
            
            // Handle vanilla badges.
            g__EquipItem_trampoline(unit, flags, item);
        });
            
    // Disable Super Charge / Toughen Up as menu option.
    mod::writePatch(
        reinterpret_cast<void*>(g__btlcmd_MakeOperationTable_Patch_NoSuperCharge),
        0x480000b8U /* unconditional branch */);

    // Track first use of moves at every level.
    g__btlcmd_SetAttackEvent_trampoline = mod::hookFunction(
        ttyd::battle_seq_command::_btlcmd_SetAttackEvent,
        [](BattleWorkUnit* unit, BattleWorkCommand* command_work) {
            // Run vanilla logic.
            g__btlcmd_SetAttackEvent_trampoline(unit, command_work);

            // Track the player having used jump/hammer, Special, partner moves.
            switch (command_work->current_cursor_type) {
                case 0:
                case 1:
                case 4:
                case 6: {
                    ttyd::battle::BattleWorkCommandCursor* cursor;
                    ttyd::battle_seq_command::_btlcmd_GetCursorPtr(
                        command_work, command_work->current_cursor_type, &cursor);
                    const int32_t move_type = 
                        command_work->weapon_table[cursor->abs_position].index;
                    MoveManager::LogMoveUse(move_type);
                    break;
                }
                case 2: {
                    ttyd::battle::BattleWorkCommandCursor* cursor;
                    ttyd::battle_seq_command::_btlcmd_GetCursorPtr(
                        command_work, command_work->current_cursor_type, &cursor);
                    const int32_t item_type = 
                        command_work->weapon_table[cursor->abs_position].weapon->item_id;

                    if (item_type == ItemType::TRADE_OFF &&
                        ttyd::battle::g_BattleWork->turn_count <= 1 &&
                        g_Mod->state_.IsFinalBossFloor()) {
                        // Track that a Trade Off was used on the first turn.
                        g_Mod->state_.ChangeOption(STAT_RUN_TRADE_OFF_ON_BOSS);
                    }
                    break;
                }
            }
        });
        
    // Quick Change FP cost:
    // Signal that party switch was initiated by the player.
    mod::writeBranchPair(
        reinterpret_cast<void*>(g__btlcmd_SetAttackEvent_SwitchPartnerCost_BH),
        reinterpret_cast<void*>(StartSpendFpOnSwitchPartner),
        reinterpret_cast<void*>(BranchBackSpendFpOnSwitchPartner));
    // Pay and increment cost when actual party switch action begins.
    g_btlevtcmd_ChangeParty_trampoline = mod::hookFunction(
        ttyd::battle_event_cmd::btlevtcmd_ChangeParty,
        [](EvtEntry* evt, bool isFirstCall) {
            if (ttyd::mario_pouch::pouchEquipCheckBadge(ItemType::QUICK_CHANGE))
                SpendAndIncrementPartySwitchCost();
            // Run vanilla logic.
            return g_btlevtcmd_ChangeParty_trampoline(evt, isFirstCall);
        });

    // Set rate of confusion proc dynamically.
    mod::writeBranchPair(
        reinterpret_cast<void*>(g_btlSeqAct_SetConfuseProcRate_BH),
        reinterpret_cast<void*>(StartSetConfuseProcRate),
        reinterpret_cast<void*>(BranchBackSetConfuseProcRate));
        
    // Run additional logic on confusion proc.
    g_BattleSetConfuseAct_trampoline = mod::hookFunction(
        ttyd::battle_seq_command::BattleSetConfuseAct,
        [](BattleWork* battleWork, BattleWorkUnit* unit){
            // Cancel Quick Change signal.
            g_PartySwitchPlayerInitiated = false;
            // Reset move levels to avoid using higher-level moves in Confusion.
            MoveManager::ResetSelectedLevels();
            // Run vanilla logic.
            return g_BattleSetConfuseAct_trampoline(battleWork, unit);
        });

    // Display battle messages (add support for printing text directly).
    g_btlevtcmd_AnnounceMessage_trampoline = mod::hookFunction(
        ttyd::battle_message::btlevtcmd_AnnounceMessage,
        [](EvtEntry* evt, bool isFirstCall) {
            // Run original logic.
            int32_t result = 
                g_btlevtcmd_AnnounceMessage_trampoline(evt, isFirstCall);

            // Replace string if using custom 'direct' mode.
            int32_t mode = evtGetValue(evt, evt->evtArguments[0]);
            if (mode == 3) {
                sprintf(
                    ttyd::battle::g_BattleWork->announce_msg_buf,
                    (const char*)evtGetValue(evt, evt->evtArguments[3]));
            }
            return result;
        });

    // Handle target selection for moves that can select between sides.
    // Player input to switch sides:
    mod::writeBranchPair(
        reinterpret_cast<void*>(g_BattleCommandInput_HandleSelectSide_BH),
        reinterpret_cast<void*>(StartCommandHandleSideSelection),
        reinterpret_cast<void*>(BranchBackCommandHandleSideSelection));
    // Drawing cursors only over selected side:
    mod::writeBranch(
        reinterpret_cast<void*>(
            g_BattleCommandDisplay_HandleSelectSide_BH),
        reinterpret_cast<void*>(StartCommandDisplaySideSelection));
    mod::writeBranch(
        reinterpret_cast<void*>(BranchBackCommandDisplaySideSelection),
        reinterpret_cast<void*>(
            g_BattleCommandDisplay_HandleSelectSide_EH));
    mod::writeBranch(
        reinterpret_cast<void*>(ConditionalBranchCommandDisplaySideSelection),
        reinterpret_cast<void*>(
            g_BattleCommandDisplay_HandleSelectSide_CH1));
    // Filters targets w.r.t. side selecting attacks, and changes targeting
    // order for multitargets so the user hits themselves after other targets.
    g_btlevtcmd_GetSelectEnemy_trampoline = mod::hookFunction(
        ttyd::battle_event_cmd::btlevtcmd_GetSelectEnemy,
        [](EvtEntry* evt, bool isFirstCall) {
            ReorderAndFilterWeaponTargets();
            return g_btlevtcmd_GetSelectEnemy_trampoline(evt, isFirstCall);
        });

    // Reverse direction of fire particles from Hooktail, etc. breath.
    g_init_breath_trampoline = mod::hookFunction(
        ttyd::eff_gonbaba_breath::init_breath,
        [](EffGonbabaBreathWork* work, int32_t unk0, int32_t type) {
            // Run original logic.
            g_init_breath_trampoline(work, unk0, type);
            // Reverse direction of particles if set.
            if (g_GonbabaBreathDir == 1) {
                work->velocity.x *= -1.0f;
            }
        });

    // Override weapon-from-command logic for Doopliss, since he shares party
    g_btlevtcmd_CommandGetWeaponAddress_trampoline = mod::hookFunction(
        ttyd::battle_event_cmd::btlevtcmd_CommandGetWeaponAddress,
        [](EvtEntry* evt, bool isFirstCall) {
            int32_t unit_idx = ttyd::battle_sub::BattleTransID(
                evt, evtGetValue(evt, evt->evtArguments[0]));
            auto* battleWork = ttyd::battle::g_BattleWork;
            auto* unit = ttyd::battle::BattleGetUnitPtr(battleWork, unit_idx);

            // Override for Doopliss so it doesn't attempt to use the party's
            // currently selected weapon instead.
            if (unit->true_kind == BattleUnitType::DOOPLISS_CH_8) {
                evtSetValue(evt, evt->evtArguments[1], PTR(custom::unitDoopliss_weaponSelected));
                return 2;
            }

            // Otherwise, use the original function's logic.
            return g_btlevtcmd_CommandGetWeaponAddress_trampoline(evt, isFirstCall);
        });
    g_BattleGetSelectWeapon_trampoline = mod::hookFunction(
        ttyd::battle_seq_command::BattleGetSelectWeapon,
        [](BattleWork* battleWork) {
            auto* unit = battleWork->battle_units[battleWork->active_unit_idx];
            if (unit->true_kind == BattleUnitType::DOOPLISS_CH_8) {
                return custom::unitDoopliss_weaponSelected;
            }

            // Run original logic.
            return g_BattleGetSelectWeapon_trampoline(battleWork);
        });
}

void SetTargetAudienceAmount() {
    // Set audience target value based on current floor.
    const int32_t floor = g_Mod->state_.floor_;
    float target_amount = floor * 2.0f + 5.0f;
    if (target_amount > 200.f) target_amount = 200.f;
    
    auto& audience_work = ttyd::battle::g_BattleWork->audience_work;
    audience_work.base_target_audience = target_amount;
}

double ApplySpRegenMultiplier(double base_regen) {
    // Leaving as stub function in case this ends up being a ToT option.
    return base_regen;
}

void ToggleScopedStatus(
    BattleWorkUnit* attacker, BattleWorkUnit* target, BattleWeapon* weapon) {
    if (attacker->current_kind <= BattleUnitType::BONETAIL) return;
    
    // If the player attempted to land damage or a negative/neutral status.
    bool player_attempted_attack = false;
    if (weapon->damage_function) {
        player_attempted_attack = true;
    } else {
        if (weapon->sleep_time || 
            weapon->stop_time ||
            weapon->dizzy_time ||
            weapon->poison_time ||
            weapon->confuse_time ||
            weapon->burn_time ||
            weapon->freeze_time ||
            weapon->size_change_time ||
            weapon->atk_change_time ||
            weapon->def_change_time ||
            weapon->slow_time ||
            weapon->allergic_time ||
            weapon->fright_chance ||
            weapon->gale_force_chance ||
            weapon->ohko_chance) {
            player_attempted_attack = true;
        }
    }
    if (player_attempted_attack) {
        target->status_flags &= ~BattleUnitStatus_Flags::SCOPED;
        target->status_flags &= ~BattleUnitStatus_Flags::SCOPED_PLUS;
    }
}

void QueueCustomStatusMessage(BattleWorkUnit* unit, const char* announce_msg) {
    // Queue custom message, using status 1 (Stop)'s unused no-effect entry.
    ttyd::battle_status_effect::_st_chg_msg_data[2].msg_no_effect = announce_msg;
    ttyd::battle_status_effect::BattleStatusChangeInfoSetAnnouce(
        unit, /* placeholder status + turns */ 1, 1, /* no effect */ 0);
    ttyd::battle_status_effect::BattleStatusChangeMsgSetAnnouce(
        unit, /* placeholder status */ 1, /* no effect */ 0);
}

int32_t GetPartySwitchCost() {
    return ttyd::mario_pouch::pouchEquipCheckBadge(ItemType::QUICK_CHANGE) 
        ? g_PartySwitchNextFpCost : 0;
}

void ResetPartySwitchCost() {
    g_PartySwitchNextFpCost = 1;
}

void SignalPlayerInitiatedPartySwitch() {
    if (ttyd::mario_pouch::pouchEquipCheckBadge(ItemType::QUICK_CHANGE)) {
        g_PartySwitchPlayerInitiated = true;
    }
}

int32_t GetCurrentEnemyAtk(BattleWorkUnit* unit, int32_t reference_atk) {
    bool should_scale = true;
    if (reference_atk < 0) {
        int32_t atk, def;
        GetTattleDisplayStats(unit->current_kind, &atk, &def);
        reference_atk = atk;
        should_scale = false;
    }
    return CalculateAtkImpl(
        unit, reference_atk, 
        /* sp flags */ AttackSpecialProperty_Flags::ALL_BUFFABLE,
        should_scale, /* ice power? */ false);
}

int32_t GetCurrentEnemyDef(BattleWorkUnit* unit) {
    int32_t part_id = BtlUnit_GetBodyPartsId(unit);
    auto* part = BtlUnit_GetPartsPtr(unit, part_id);
    return CalculateDefImpl(unit, part, /* element */ 0);
}

void HandleSideSelection() {
    auto& twork = ttyd::battle::g_BattleWork->weapon_targets_work;

    if (twork.num_targets > 1 &&
        twork.weapon_target_class_flags & AttackTargetClass_Flags::SELECT_SIDE) {
        int32_t change = 0;
        if (ttyd::battle_pad::BattlePadCheckRepeat(0x80000)) ++change;
        if (ttyd::battle_pad::BattlePadCheckRepeat(0x40000)) --change;
        if (change) {
            // Iterate between targets until finding one on the opposite side.
            int32_t new_target = twork.current_target;
            do {
                new_target += change;
                if (new_target >= twork.num_targets) new_target = 0;
                if (new_target < 0) new_target = twork.num_targets - 1;

                if (!CheckOnSelectedSide(new_target)) break;
            } while (twork.current_target != new_target);

            if (twork.current_target != new_target) {
                ttyd::pmario_sound::psndSFXOn("SFX_BTL_CURSOR_MOVE2");
                twork.current_target = new_target;
            }
        }
    }
}

bool CheckOnSelectedSide(int32_t target_idx) {
    auto* battleWork = ttyd::battle::g_BattleWork;
    auto& twork = battleWork->weapon_targets_work;
    if (twork.weapon_target_class_flags & AttackTargetClass_Flags::SELECT_SIDE) {
        BattleWorkUnit* primary_selection_unit =
            battleWork->battle_units[
                twork.targets[twork.target_indices[twork.current_target]].unit_idx];

        BattleWorkUnit* target_unit =
            battleWork->battle_units[
                twork.targets[twork.target_indices[target_idx]].unit_idx];

        // If target unit is on opposite side of field from primary selection.
        // TODO: Arbitrary choice for "left" / "right" division.
        if ((primary_selection_unit->home_position.x < -30.0f) !=
            (target_unit->home_position.x < -30.0f))
            return false;
    }
    // If not select-side, don't filter anything out.
    return true;
}

bool CheckPlayAttackFx(uint32_t flags, gc::vec3* position) {
    const int32_t id = (flags & 0x1f00'0000) / 0x100'0000;
    const char* fx_name = CosmeticsManager::GetSoundFromFXGroup(id);
    const auto* data = CosmeticsManager::GetAttackFxData(id);
    if (!fx_name) return false;

    int32_t sfx_id = ttyd::pmario_sound::psndSFXOn_3D(fx_name, position);
    if (data->randomize_pitch) {
        // Play at one of a few random pitches.
        int16_t pitch = 0x400 * (ttyd::system::irand(3) - 1);
        ttyd::pmario_sound::psndSFX_pit(sfx_id, pitch);
    }
    return true;
}

// Applies a custom status effect to the target.
// Params: unit, part, status_flag, color1, color2, sfx_name, announce_msg
EVT_DEFINE_USER_FUNC(evtTot_ApplyCustomStatus) {
    int32_t unit_idx = ttyd::battle_sub::BattleTransID(
        evt, evtGetValue(evt, evt->evtArguments[0]));
    int32_t part_idx = evtGetValue(evt, evt->evtArguments[1]);
    auto* battleWork = ttyd::battle::g_BattleWork;
    auto* unit = ttyd::battle::BattleGetUnitPtr(battleWork, unit_idx);
    auto* part = ttyd::battle::BattleGetUnitPartsPtr(unit_idx, part_idx);
    
    uint32_t status_flag = evtGetValue(evt, evt->evtArguments[2]);
    uint32_t color1 = evtGetValue(evt, evt->evtArguments[3]);
    uint32_t color2 = evtGetValue(evt, evt->evtArguments[4]);
    const char* sfx_name = (const char*)evtGetValue(evt, evt->evtArguments[5]);
    const char* announce_msg = (const char*)evtGetValue(evt, evt->evtArguments[6]);
    
    // Apply status flag.
    unit->status_flags |= status_flag;
    
    // Spawn splash effect.
    gc::vec3 pos;
    ttyd::battle_unit::BtlUnit_GetPos(unit, &pos.x, &pos.y, &pos.z);
    // Adjust position for center point based on whether on ceiling.
    if ((unit->attribute_flags & 2) == 0) {
        pos.y += unit->height / 2;
    } else {
        pos.y -= unit->height / 2;
    }
    switch (unit->current_kind) {
        case BattleUnitType::HOOKTAIL:
        case BattleUnitType::GLOOMTAIL:
        case BattleUnitType::BONETAIL:
            pos.x -= 300;
            pos.z += 30;
            break;
        case BattleUnitType::SHADOW_QUEEN_PHASE_2:
            pos.z += 40;
            break;
    }
    auto* eff = ttyd::eff_stamp_n64::effStampN64Entry(
        pos.x, pos.y, pos.z + 10.0f, 2);
    // Apply user-determined colors to effect.
    ((char*)eff->eff_work)[0x38] = color1 >> 16;
    ((char*)eff->eff_work)[0x39] = (color1 >> 8) & 0xff;
    ((char*)eff->eff_work)[0x3a] = color1        & 0xff;
    ((char*)eff->eff_work)[0x3b] = color2 >> 16;
    ((char*)eff->eff_work)[0x3c] = (color2 >> 8) & 0xff;
    ((char*)eff->eff_work)[0x3d] = color2        & 0xff;
    
    // Play sound effect.
    ttyd::battle_unit::BtlUnit_GetHitPos(unit, part, &pos.x, &pos.y, &pos.z);
    ttyd::pmario_sound::psndSFXOn_3D(sfx_name, &pos);
    
    // Queue custom status message.
    QueueCustomStatusMessage(unit, announce_msg);
    
    return 2;
}

EVT_DEFINE_USER_FUNC(evtTot_SetGonbabaBreathDir) {
    g_GonbabaBreathDir = evtGetValue(evt, evt->evtArguments[0]);
    return 2;
}

}  // namespace battle
}  // namespace mod::tot::patch