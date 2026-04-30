#ifndef MT_EVENT_H
#define MT_EVENT_H
#include "common.h"
#include "player.h"
#include "entity.h"

enum MT_EventType { MT_EVT_NONE, MT_EVT_TREASURE, MT_EVT_TRAP, MT_EVT_AMBUSH, MT_EVT_BLESSING };

/**
 * @brief 基于概率与随机数引擎生成事件类型
 * @param base_prob 触发任意事件的概率，建议范围 [0.0, 1.0]
 * @param rng 由 controller 传入的统一随机引擎
 * @return 事件类型；未触发时返回 MT_EVT_NONE
 */
MT_EventType mt_event_roll(float base_prob, std::mt19937& rng);

/**
 * @brief 应用事件效果
 * @param p [in,out] 玩家对象，可被修改（血量、防御、背包）
 * @param evt 事件类型
 * @param diff 当前难度，用于生成伏击敌人强度
 * @param out_enemy [out] 仅当 evt=MT_EVT_AMBUSH 时写入有效敌人
 * @note 本函数不做战斗结算；战斗由 controller+combat 负责
 */
void mt_event_apply(MT_Player& p, MT_EventType evt, MT_Difficulty diff, MT_Enemy& out_enemy);
#endif