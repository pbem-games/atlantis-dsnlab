// START A3HEADER
//
// This source file is part of the Atlantis PBM game program.
// Copyright (C) 1995-1999 Geoff Dunbar
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program, in the file license.txt. If not, write
// to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
// Boston, MA 02111-1307, USA.
//
// See the Atlantis Project web page for details:
// http://www.prankster.com/project
//
// END A3HEADER
#ifndef GAME_DEFS
#define GAME_DEFS

#include "helper.h"

/* Directions */
enum {
    D_NORTH,
    D_NORTHEAST,
    D_SOUTHEAST,
    D_SOUTH,
    D_SOUTHWEST,
    D_NORTHWEST,
    NDIRS
};

extern char **DirectionStrs;

extern char **MonthNames;

class GameDefs {
public:
    char *RULESET_NAME;
    ATL_VER RULESET_VERSION;
    ATL_VER ENGINE_VERSION;
    char *BUILDNAME_NAME;

    int FOOT_SPEED;
    int HORSE_SPEED;
    int SHIP_SPEED;
    int FLY_SPEED;
    int MAX_SPEED;
	
    int WAGON_CAPACITY;
  
    int STUDENTS_PER_TEACHER;
    int MAINTENANCE_COST;
    int LEADER_COST;
    int STARVE_PERCENT;
    int START_MONEY;
    int WORK_FRACTION;
    int ENTERTAIN_FRACTION;
    int ENTERTAIN_INCOME;
  
    int TAX_INCOME;

    int HEALS_PER_MAN;
  
    int GUARD_REGEN; /* percent */
    int CITY_GUARD;
    int GUARD_MONEY;
    int CITY_POP;
  
    int WMON_FREQUENCY;
    int LAIR_FREQUENCY;
  
    int FACTION_POINTS;

    int MAX_AC_X;
    int MAX_AC_Y;

    int TURN_COST;

    int TIMES_REWARD;

    int TOWNS_EXIST;
    int LEADERS_EXIST;
    int SKILL_LIMIT_NONLEADERS;
    int MAGE_NONLEADERS;
    int RACES_EXIST;
    int GATES_EXIST;
    int FOOD_ITEMS_EXIST;
    int CITY_MONSTERS_EXIST;
    int WANDERING_MONSTERS_EXIST;
    int LAIR_MONSTERS_EXIST;
    int WEATHER_EXISTS;
    int OPEN_ENDED;

    //
    // RANDOM_ECONOMY determines whether the economy for different regions
    // is randomized, or is always the same.
    //
    int RANDOM_ECONOMY;

    //
    // If VARIABLE_ECONOMY is set, the economy of a region is altered after
    // each turn.
    //
    int VARIABLE_ECONOMY;

    //
    // Some economy figures.
    //
    int CITY_MARKET_NORMAL_AMT;
    int CITY_MARKET_ADVANCED_AMT;
    int CITY_MARKET_TRADE_AMT;

    //
    // The type of faction limits that are in effect in this game.
    //
    int MAX_INACTIVE_TURNS;

    enum {
        FACLIM_MAGE_COUNT,
        FACLIM_FACTION_TYPES,
    };
    int FACTION_LIMIT_TYPE;
    int TRIFLE_COST;
};

extern GameDefs * Globals;

#endif
