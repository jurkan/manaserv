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


#ifndef _TMWSERV_DALSTORAGE_H_
#define _TMWSERV_DALSTORAGE_H_

#include <list>
#include <map>

#include "dal/dataprovider.h"

class Account;
class Character;
class ChatChannel;
class Guild;
class Letter;
class Post;

/**
 * A storage class that relies on DAL.
 */
class DALStorage
{
    public:
        /**
         * Constructor.
         */
        DALStorage();


        /**
         * Destructor.
         */
        ~DALStorage();


        /**
         * Connect to the database and initialize it if necessary.
         */
        void open();


        /**
         * Disconnect from the database.
         */
        void close();


        /**
         * Get an account by user name.
         *
         * @param userName the owner of the account.
         *
         * @return the account associated to the user name.
         */
        Account *getAccount(const std::string& userName);

        /**
         * Get an account by ID.
         *
         * @param accountID the ID of the account.
         *
         * @return the account associated with the ID.
         */
        Account *getAccount(int accountID);

        /**
         * Gets a character by database ID.
         *
         * @param id the ID of the character.
         * @param owner the account the character is in.
         *
         * @return the character associated to the ID.
         */
        Character *getCharacter(int id, Account *owner);

        /**
         * Gets a character by character name.
         *
         * @param name of the character
         *
         * @return the character associated to the name
         */
        Character *getCharacter(const std::string &name);

        /**
         * Add a new account.
         *
         * @param account the new account.
         */
        void
        addAccount(Account *account);

        /**
         * Delete an account.
         *
         * @param account the account to delete.
         */
        void delAccount(Account *account);

        /**
         * Update the date and time of the last login.
         *
         * @param account the account that recently logged in.
         */
        void updateLastLogin(const Account *account);

        /**
         * Sets a ban on an account (hence on all its characters).
         *
         * @param id character identifier.
         * @param duration duration in minutes.
         */
        void banCharacter(int id, int duration);

        /**
         * Delete a character in the database.
         *
         * @param charId character identifier.
         * @param startTransaction indicates wheter the function should run in
         *        its own transaction or is called inline of another transaction
         */
        void delCharacter(int charId, bool startTransaction) const;

        /**
         * Delete a character in the database. The object itself i not touched
         * by this function!
         *
         * @param character character object.
         * @param startTransaction indicates wheter the function should run in
         *        its own transaction or is called inline of another transaction
         */
        void delCharacter(Character *character, bool startTransaction) const;

        /**
         * Removes expired bans from accounts
         */
        void checkBannedAccounts();

        /**
         * Tells if the user name already exists.
         * @return true if the user name exists.
         */
        bool doesUserNameExist(std::string const &name);

        /**
         * Tells if the email address already exists.
         * @return true if the email address exists.
         */
        bool doesEmailAddressExist(std::string const &email);

        /**
         * Tells if the character name already exists.
         * @return true if the character name exists.
         */
        bool doesCharacterNameExist(std::string const &name);

        /**
         * Updates the data for a single character, does not update the
         * owning account or the characters name.
         * Primary usage should be storing characterdata received from a
         * game server.
         * returns true if succefull, false otherwise.
         * @param ptr Character to store values in the database.
         * @param startTransaction set to false if this method is called as
         *                         nested transaction.
         * @return true on success
         */
        bool
        updateCharacter(Character *ptr,
                        bool startTransaction = true);

        /**
         * Save changes of a skill to the database permanently.
         *
         * @param character Character thats skill has changed.
         * @param skill_id Identifier of the changed skill.
         *
         * @exception dbl::DbSqlQueryExecFailure.
         */
        void
        flushSkill(const Character* const character, const int skill_id );

        /**
         * Add a new guild
         *
         */
        void
        addGuild(Guild* guild);

        /**
         * Delete a guild
         *
         */
        void
        removeGuild(Guild* guild);

        /**
         * Add member to guild
         *
         */
        void
        addGuildMember(int guild_id, int memberId);

        /**
         * Remove member from guild
         */
        void
        removeGuildMember(int guildId, int memberId);

        /**
         * Save guild member rights
         */
        void
        setMemberRights(int memberId, int rights);

        /**
         * Get guild list
         *@return a list of guilds
         *
         */
        std::list<Guild*>
        getGuildList();

        /**
         * Save changes to the database permanently.
         *
         * @exception dal::DbSqlQueryExecFailure.
         */
        void flushAll();
        void flush(Account *);

        /**
         * Gets the value of a quest variable.
         */
        std::string getQuestVar(int id, std::string const &);

        /**
         * Sets the value of a quest variable.
         */
        void setQuestVar(int id, std::string const &, std::string const &);

        /**
         * Gets the string value of a map specific world state variable.
         *
         * @param name Name of the requested world-state variable.
         * @param map_id Id of the specific map.
         */
        std::string getWorldStateVar(std::string const &name, int map_id = -1);

        /**
         * Sets the value of a world state variable.
         *
         * @param name Name of the world-state vairable.
         * @param value New value of the world-state variable.
         */
        void setWorldStateVar(std::string const &name, std::string const &value);

        /**
         * Sets the value of a world state variable of a specific map.
         *
         * @param name Name of the world-state vairable.
         * @param map_id ID of the specific map
         * @param value New value of the world-state variable.
         */
        void setWorldStateVar(std::string const &name, int map_id,
                              std::string const &value);

        /**
         * Set the level on an account
         *
         * @param id The id of the account
         * @param level The level to set for the account
         */
        void setAccountLevel(int id, int level);

        /**
         * Set the level on a character
         *
         * @param id The id of the character
         * @param level The level to set for the character
         */
        void setPlayerLevel(int id, int level);

        /**
         * Store letter
         *
         * @param letter The letter to store
         */
        void storeLetter(Letter *letter);

        /**
         * Retrieve post
         *
         * @param playerId The id of the player requesting his post
         */
        Post* getStoredPost(int playerId);

        /**
         * Add item to auction
         *
         * @param itemId The id of the item for auction
         * @param player The player who put the item up for auction
         * @param gold The amount of money to buy it
         */
        void addAuctionItem(unsigned int itemId, int playerId, unsigned int gold);

    private:
        /**
         * Copy constructor.
         */
        DALStorage(const DALStorage& rhs);


        /**
         * Assignment operator.
         */
        DALStorage&
        operator=(const DALStorage& rhs);

        /**
         * Gets an account by using a SQL query string.
         *
         * @param query the query for the account
         *
         * @return the account found by the query
         */
        Account *getAccountBySQL(std::string const &query);


        /**
         * Gets a character by character name.
         *
         * @param query the query for the character.
         * @param owner the account the character is in.
         *
         * @return the character found by the query.
         */
        Character *getCharacterBySQL(std::string const &query, Account *owner);

        dal::DataProvider *mDb; /**< the data provider */
};

extern DALStorage *storage;

#endif // _TMWSERV_DALSTORAGE_H_
