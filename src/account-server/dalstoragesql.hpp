/*
 *  The Mana World Server
 *  Copyright 2008 The Mana World Development Team
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
 */

#ifndef _TMWSERV_DALSTORAGE_SQL_H_
#define _TMWSERV_DALSTORAGE_SQL_H_

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#if !defined (MYSQL_SUPPORT) && !defined (SQLITE_SUPPORT) && \
    !defined (POSTGRESQL_SUPPORT)
#error "(dalstorage.hpp) no database backend defined"
#endif


#include <string>

// TODO: Fix problem with PostgreSQL null primary key's.

/**
 * MySQL specificities:
 *     - TINYINT is an integer (1 byte) type defined as an extension to
 *       the SQL standard.
 *     - all integer types can have an optional (non-standard) attribute
 *       UNSIGNED (http://dev.mysql.com/doc/mysql/en/numeric-types.html)
 *
 * SQLite3 specificities:
 *     - any column (but only one for each table) with the exact type of
 *       'INTEGER PRIMARY KEY' is taken as auto-increment.
 *     - the supported data types are: NULL, INTEGER, REAL, TEXT and BLOB
 *       (http://www.sqlite.org/datatype3.html)
 *     - the size of TEXT cannot be set, it is just ignored by the engine.
 *     - IMPORTANT: foreign key constraints are not yet supported
 *       (http://www.sqlite.org/omitted.html). Included in case of future
 *       support.
 *
 * Notes:
 *     - the SQL queries will take advantage of the most appropriate data
 *       types supported by a particular database engine in order to
 *       optimize the server database size.
 */


/**
 * TABLE: tmw_accounts.
 */
static const char *ACCOUNTS_TBL_NAME = "tmw_accounts";

/**
 * TABLE: tmw_characters.
 *     - gender is 0 for male, 1 for female.
 */
static const char *CHARACTERS_TBL_NAME = "tmw_characters";

/**
 * TABLE: tmw_char_skills.
 */
static const char *CHAR_SKILLS_TBL_NAME = "tmw_char_skills";

/**
 * TABLE: tmw_char_status_effects.
 */
static const char *CHAR_STATUS_EFFECTS_TBL_NAME = "tmw_char_status_effects";

/**
 * TABLE: tmw_inventories.
 */
static const char *INVENTORIES_TBL_NAME("tmw_inventories");

/**
 * TABLE: tmw_items.
 */
static const char *ITEMS_TBL_NAME("tmw_items");

/**
 * TABLE: tmw_guilds.
 * Store player guilds
 */
static const char *GUILDS_TBL_NAME = "tmw_guilds";

/**
 * TABLE: tmw_guild_members.
 * Store guild members
 */
static const char *GUILD_MEMBERS_TBL_NAME = "tmw_guild_members";

/**
 * TABLE: tmw_quests.
 */
static const char *QUESTS_TBL_NAME = "tmw_quests";

/**
 * TABLE: tmw_world_states
 */
static const char *WORLD_STATES_TBL_NAME = "tmw_world_states";

/**
 * TABLE: tmw_post
 * Store letters sent by characters
 */
static const char *POST_TBL_NAME = "tmw_post";

/**
 * TABLE: tmw_post_attachments
 * Store attachments per letter.
 */
static const char *POST_ATTACHMENTS_TBL_NAME = "tmw_post_attachments";

/**
 * TABLE: tmw_auctions
 * Store items auctions.
 */
static const char *AUCTION_TBL_NAME = "tmw_auctions";

/**
 * TABLE: tmw_auction_bids
 * Store bids on auctions.
 */
static const char *AUCTION_BIDS_TBL_NAME = "tmw_auction_bids";

/**
 * TABLE: tmw_online_list
 * List currently online users.
 */
static const char *ONLINE_USERS_TBL_NAME = "tmw_online_list";

/**
 * TABLE: tmw_transactions
 * Stores all transactions
 */
static const char *TRANSACTION_TBL_NAME = "tmw_transactions";

#endif // _TMWSERV_DALSTORAGE_SQL_H_
