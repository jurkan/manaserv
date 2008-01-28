/*
 *  The Mana World Server
 *  Copyright 2004 The Mana World Development Team
 *
 *  This file is part of The Mana World.
 *
 *  The Mana World  is free software; you can redistribute  it and/or modify it
 *  under the terms of the GNU General  Public License as published by the Free
 *  Software Foundation; either version 2 of the License, or any later version.
 *
 *  The Mana  World is  distributed in  the hope  that it  will be  useful, but
 *  WITHOUT ANY WARRANTY; without even  the implied warranty of MERCHANTABILITY
 *  or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for
 *  more details.
 *
 *  You should  have received a  copy of the  GNU General Public  License along
 *  with The Mana  World; if not, write to the  Free Software Foundation, Inc.,
 *  59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 *
 *  $Id$
 */

#include <algorithm>
#include <cassert>
#include <cmath>

#include "game-server/character.hpp"

#include "defines.h"
#include "game-server/buysell.hpp"
#include "game-server/eventlistener.hpp"
#include "game-server/inventory.hpp"
#include "game-server/item.hpp"
#include "game-server/itemmanager.hpp"
#include "game-server/gamehandler.hpp"
#include "game-server/mapcomposite.hpp"
#include "game-server/mapmanager.hpp"
#include "game-server/trade.hpp"
#include "net/messagein.hpp"
#include "net/messageout.hpp"
#include "serialize/characterdata.hpp"

#include "utils/logger.h"

Character::Character(MessageIn &msg):
    Being(OBJECT_CHARACTER, 65535),
    mClient(NULL), mTransactionHandler(NULL), mDatabaseID(-1),
    mGender(0), mHairStyle(0), mHairColor(0), mLevel(1), mLevelProgress(0),
    mUpdateLevelProgress(false), mRecalculateLevel(true), mTransaction(TRANS_NONE)
{
    Attribute attr = { 0, 0 };
    mAttributes.resize(NB_CHARACTER_ATTRIBUTES, attr);
    mExperience.resize(CHAR_SKILL_NB, 0);
    // Get character data.
    mDatabaseID = msg.readLong();
    mName = msg.readString();
    deserializeCharacterData(*this, msg);
    for (int i = CHAR_ATTR_BEGIN; i < CHAR_ATTR_END; ++i)
    {
        modifiedAttribute(i);
    }
    setSize(16);
    Inventory(this).initialize();
}

void Character::update()
{
    if (mRecalculateLevel)
    {
        mRecalculateLevel = false;
        recalculateLevel();
    }
    Being::update();
}

void Character::perform()
{
    if (mAction != ATTACK || mActionTime > 0) return;

    mActionTime = 1000;
    mAction = STAND;
    raiseUpdateFlags(UPDATEFLAG_ATTACK);

    // TODO: Check slot 2 too.
    int itemId = mPossessions.equipment[EQUIP_FIGHT1_SLOT];
    ItemClass *ic = ItemManager::getItem(itemId);
    int type = ic ? ic->getModifiers().getValue(MOD_WEAPON_TYPE) : WPNTYPE_NONE;

    Damage damage;
    damage.base = getModifiedAttribute(BASE_ATTR_PHY_ATK_MIN);
    damage.delta = getModifiedAttribute(BASE_ATTR_PHY_ATK_DELTA);
    damage.type = DAMAGE_PHYSICAL;
    damage.cth = getModifiedAttribute(BASE_ATTR_HIT)
        + getModifiedAttribute(CHAR_SKILL_WEAPON_BEGIN + type);
    damage.usedSkill = CHAR_SKILL_WEAPON_BEGIN + type;
    if (type)
    {
        ItemModifiers const &mods = ic->getModifiers();
        damage.element = mods.getValue(MOD_ELEMENT_TYPE);
    }
    else
    {
        // No-weapon fighting.
        damage.element = ELEMENT_NEUTRAL;
    }

    int attackRange = 60; //TODO: get from weapon
    int attackAngle = 30; //TODO: get from weapon

    performAttack(damage, attackRange, attackAngle);
}

int Character::getMapId() const
{
    return getMap()->getID();
}

void Character::setMapId(int id)
{
    setMap(MapManager::getMap(id));
}

void Character::cancelTransaction()
{
    TransactionType t = mTransaction;
    mTransaction = TRANS_NONE;
    switch (t)
    {
        case TRANS_TRADE:
            static_cast< Trade * >(mTransactionHandler)->cancel(this);
            break;
        case TRANS_BUYSELL:
            static_cast< BuySell * >(mTransactionHandler)->cancel();
            break;
        case TRANS_NONE:
            return;
    }
}

Trade *Character::getTrading() const
{
    return mTransaction == TRANS_TRADE
        ? static_cast< Trade * >(mTransactionHandler) : NULL;
}

BuySell *Character::getBuySell() const
{
    return mTransaction == TRANS_BUYSELL
        ? static_cast< BuySell * >(mTransactionHandler) : NULL;
}

void Character::setTrading(Trade *t)
{
    if (t)
    {
        cancelTransaction();
        mTransactionHandler = t;
        mTransaction = TRANS_TRADE;
    }
    else
    {
        assert(mTransaction == TRANS_NONE || mTransaction == TRANS_TRADE);
        mTransaction = TRANS_NONE;
    }
}

void Character::setBuySell(BuySell *t)
{
    if (t)
    {
        cancelTransaction();
        mTransactionHandler = t;
        mTransaction = TRANS_BUYSELL;
    }
    else
    {
        assert(mTransaction == TRANS_NONE || mTransaction == TRANS_BUYSELL);
        mTransaction = TRANS_NONE;
    }
}

void Character::sendStatus()
{
    MessageOut attribMsg(GPMSG_PLAYER_ATTRIBUTE_CHANGE);
    for (std::set<size_t>::const_iterator i = mModifiedAttributes.begin(),
         i_end = mModifiedAttributes.end(); i != i_end; ++i)
    {
        int attr = *i;
        attribMsg.writeByte(attr);
        attribMsg.writeShort(getAttribute(attr));
        attribMsg.writeShort(getModifiedAttribute(attr));
    }
    if (attribMsg.getLength() > 2) gameHandler->sendTo(this, attribMsg);
    mModifiedAttributes.clear();

    MessageOut expMsg(GPMSG_PLAYER_EXP_CHANGE);
    for (std::set<size_t>::const_iterator i = mModifiedExperience.begin(),
         i_end = mModifiedExperience.end(); i != i_end; ++i)
    {
        int skill = *i;
        expMsg.writeByte(skill);
        expMsg.writeLong(getExpGot(skill));
        expMsg.writeLong(getExpNeeded(skill));
    }
    if (expMsg.getLength() > 2) gameHandler->sendTo(this, expMsg);
    mModifiedExperience.clear();

    if (mUpdateLevelProgress)
    {
        mUpdateLevelProgress = false;
        MessageOut progressMessage(GPMSG_LEVEL_PROGRESS);
        progressMessage.writeByte(mLevelProgress);
        gameHandler->sendTo(this, progressMessage);
    }
}

void Character::modifiedAttribute(int attr)
{
    if (attr >= CHAR_ATTR_BEGIN && attr < CHAR_ATTR_END)
    {
        for (int i = BASE_ATTR_BEGIN; i < BASE_ATTR_END; ++i)
        {
            int newValue = getAttribute(i);

            if (i == BASE_ATTR_HP){
                newValue = (getModifiedAttribute(CHAR_ATTR_VITALITY) + 10)
                        * (mLevel + 10);
            }
            else if (i == BASE_ATTR_HIT) {
                newValue = getModifiedAttribute(CHAR_ATTR_DEXTERITY)
                        /* + skill in class of currently equipped weapon */;
            }
            else if (i == BASE_ATTR_EVADE) {
                newValue = getModifiedAttribute(CHAR_ATTR_AGILITY);
                        /* TODO: multiply with 10 / (10 * equip_weight)*/
            }
            else if (i == BASE_ATTR_PHY_RES) {
                newValue = getModifiedAttribute(CHAR_ATTR_VITALITY);
                        /* equip defence is through equip modifiers */
            }
            else if (i == BASE_ATTR_PHY_ATK_MIN) {
                newValue = getModifiedAttribute(CHAR_ATTR_STRENGTH);
                        /* weapon attack is applied through equip modifiers */
            }
            else if (i == BASE_ATTR_PHY_ATK_DELTA) {
                newValue =  0;
                        /* + skill in class of currently equipped weapon ( is
                         * applied during the damage calculation)
                         * weapon attack bonus is applied through equip
                         * modifiers.
                         */
            }
            else if (i == BASE_ATTR_MAG_RES) {
                newValue = getModifiedAttribute(CHAR_ATTR_WILLPOWER);
            }
            else if (i == BASE_ATTR_MAG_ATK) {
                newValue = getModifiedAttribute(CHAR_ATTR_WILLPOWER);
            }

            if (newValue != getAttribute(i))
            {
                setAttribute(i, newValue);
                flagAttribute(i);
            }
        }
    }
    flagAttribute(attr);
}

void Character::flagAttribute(int attr)
{
    // Warn the player of this attribute modification.
    mModifiedAttributes.insert(attr);
}

int Character::expForLevel(int level)
{
    return int(pow(level, EXPCURVE_EXPONENT) * EXPCURVE_FACTOR);
}

void Character::receiveExperience(size_t skill, int experience)
{
    if (skill >= CHAR_SKILL_BEGIN && skill < CHAR_SKILL_END)
    {
        // add exp
        long int newExp = mExperience.at(skill - CHAR_SKILL_BEGIN) + experience;
        if (newExp > INT_MAX) newExp = INT_MAX; // avoid integer overflow.
        mExperience.at(skill - CHAR_SKILL_BEGIN) = newExp;
        mModifiedExperience.insert(skill - CHAR_SKILL_BEGIN);

        // check for skill levelup
        while (newExp >= Character::expForLevel(getAttribute(skill) + 1))
        {
            setAttribute(skill, getAttribute(skill) + 1);
            modifiedAttribute(skill);
        }

        mRecalculateLevel = true;
    }
}

void Character::recalculateLevel()
{
    std::list<float> levels;
    for (int a = CHAR_SKILL_BEGIN; a < CHAR_SKILL_END; a++)
    {
        float expGot = getExpGot(a - CHAR_SKILL_BEGIN);
        float expNeed = getExpNeeded(a - CHAR_SKILL_BEGIN);
        levels.push_back(getAttribute(a) + expGot / expNeed);
    }
    levels.sort();

    std::list<float>::iterator i = levels.end();
    float level = 0.0f;
    float factor = 1.0f;
    float factorSum = 0.0f;
    while (i != levels.begin()) //maybe it wouldn't be a bad idea to unroll this loop
    {
        i--;
        level += *i * factor;
        factorSum += factor;
        factor *= LEVEL_SKILL_PRECEDENCE_FACTOR;
    }
    level /= factorSum;
    level += 1.0f; // + 1.0f because the lowest level is 1 and not 0

    while (mLevel < level)
    {
        levelup();
    }

    int levelProgress = int((level - floor(level)) * 100);
    if (levelProgress != mLevelProgress)
    {
        mLevelProgress = levelProgress;
        mUpdateLevelProgress = true;
    }
}

int Character::getExpNeeded(size_t skill)
{
    int level = getAttribute(skill + CHAR_SKILL_BEGIN);
    return Character::expForLevel(level + 1) - expForLevel(level);
}

int Character::getExpGot(size_t skill)
{
    int level = getAttribute(skill + CHAR_SKILL_BEGIN);
    return mExperience.at(skill) - Character::expForLevel(level);
}

void Character::levelup()
{
    mLevel++;

    mCharacterPoints += CHARPOINTS_PER_LEVELUP;
    mCorrectionPoints += CORRECTIONPOINTS_PER_LEVELUP;
    if (mCorrectionPoints > CORRECTIONPOINTS_MAX)
        mCorrectionPoints = CORRECTIONPOINTS_MAX;

    MessageOut levelupMsg(GPMSG_LEVELUP);
    levelupMsg.writeShort(mLevel);
    levelupMsg.writeShort(mCharacterPoints);
    levelupMsg.writeShort(mCorrectionPoints);
    gameHandler->sendTo(this, levelupMsg);
    LOG_INFO(mName<<" reached level "<<mLevel);
}

AttribmodResponseCode Character::useCharacterPoint(size_t attribute)
{
    if (attribute < CHAR_ATTR_BEGIN) return ATTRIBMOD_INVALID_ATTRIBUTE;
    if (attribute >= CHAR_ATTR_END) return ATTRIBMOD_INVALID_ATTRIBUTE;
    if (!mCharacterPoints) return ATTRIBMOD_NO_POINTS_LEFT;

    mCharacterPoints--;
    setAttribute(attribute, getAttribute(attribute) + 1);
    modifiedAttribute(attribute);
    return ATTRIBMOD_OK;
}

AttribmodResponseCode Character::useCorrectionPoint(size_t attribute)
{
    if (attribute < CHAR_ATTR_BEGIN) return ATTRIBMOD_INVALID_ATTRIBUTE;
    if (attribute >= CHAR_ATTR_END) return ATTRIBMOD_INVALID_ATTRIBUTE;
    if (!mCorrectionPoints) return ATTRIBMOD_NO_POINTS_LEFT;
    if (getAttribute(attribute) <= 1) return ATTRIBMOD_DENIED;

    mCorrectionPoints--;
    mCharacterPoints++;
    setAttribute(attribute, getAttribute(attribute) - 1);
    modifiedAttribute(attribute);
    return ATTRIBMOD_OK;
}

void Character::disconnected()
{
    for (Listeners::iterator i = mListeners.begin(),
         i_end = mListeners.end(); i != i_end;)
    {
        EventListener const &l = **i;
        ++i; // In case the listener removes itself from the list on the fly.
        if (l.dispatch->disconnected) l.dispatch->disconnected(&l, this);
    }
}
