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
#ifndef ORDERS_CLASS
#define ORDERS_CLASS

class Order;
class AttackOrder;
class MoveOrder;
class GiveOrder;
class StudyOrder;
class TeachOrder;
class SellOrder;
class BuyOrder;
class ProduceOrder;
class BuildOrder;
class SailOrder;
class FindOrder;
class StealOrder;
class AssassinateOrder;
class CastOrder;
class CastMindOrder;
class CastRegionOrder;
class TeleportOrder;
class ForgetOrder;
class TradeOrder;
class TrifleOrder;

#include "unit.h"
#include "gamedefs.h"
#include "astring.h"
#include "alist.h"

enum {
  O_ATLANTIS,
  O_END,
  O_UNIT,
  O_ADDRESS,
  O_ADVANCE,
  O_ASSASSINATE,
  O_ATTACK,
  O_AUTOTAX,
  O_AVOID,
  O_BEHIND,
  O_BUILD,
  O_BUY,
  O_CAST,
  O_CLAIM,
  O_COMBAT,
  O_CONSUME,
  O_DECLARE,
  O_DESCRIBE,
  O_DESTROY,
  O_ENDFORM,
  O_ENTER,
  O_ENTERTAIN,
  O_FACTION,
  O_FIND,
  O_FORGET,
  O_FORM,
  O_GIVE,
  O_GUARD,
  O_HOLD,
  O_LEAVE,
  O_MOVE,
  O_NAME,
  O_NOAID,
  O_OPTION,
  O_PASSWORD,
  O_PILLAGE,
  O_PRODUCE,
  O_PROMOTE,
  O_QUIT,
  O_RESTART,
  O_REVEAL,
  O_SAIL,
  O_SELL,
  O_SHOW,
  O_STEAL,
  O_STUDY,
  O_TAX,
  O_TEACH,
  O_WORK,
  O_TRADE,
  O_ROUTE,
// dsnlab
  O_LAST_,
  O_NOSPOILS,
  O_SPOILS,
  O_TRIFLE,
  NORDERS
};

enum {
  M_NONE,
  M_WALK,
  M_RIDE,
  M_FLY,
  M_SAIL
};

enum {
  TRADE_SELL,
  TRADE_BUY
};

#define MOVE_IN 98
#define MOVE_OUT 99
/* Enter is MOVE_ENTER + num of object */
#define MOVE_ENTER 100

extern char ** OrderStrs;

int Parse1Order(AString *);

class Order : public AListElem {
public:
  Order();
  virtual ~Order();
	
  int type;
};

class MoveDir : public AListElem {
public:
  int dir;
};

class MoveOrder : public Order {
public:
  MoveOrder();
  ~MoveOrder();
	
  int advancing;
  AList dirs;
};

class GiveOrder : public Order {
public:
  GiveOrder();
  ~GiveOrder();
	
  int item;
  int amount; /* if amount == -1, transfer whole unit */
// dsn
  int except;
  UnitId * target;
};

class StudyOrder : public Order {
public:
  StudyOrder();
  ~StudyOrder();
	
  int skill;
  int days;
};

class TeachOrder : public Order {
public:
  TeachOrder();
  ~TeachOrder();
	
  AList targets;
};

class ProduceOrder : public Order {
public:
  ProduceOrder();
  ~ProduceOrder();
	
  int item;
  int skill; /* -1 for none */
  int productivity;
};

class BuyOrder : public Order {
public:
  BuyOrder();
  ~BuyOrder();
	
  int item;
  int num;
};

class SellOrder : public Order {
public:
  SellOrder();
  ~SellOrder();
	
  int item;
  int num;
};

class TradeOrder : public Order {
public:
  TradeOrder();
  ~TradeOrder();

  int item;
  int num;
  int price;
  int doing;
  int unit_num;
};

class AttackOrder : public Order {
public:
  AttackOrder();
  ~AttackOrder();
	
  AList targets;
};

class BuildOrder : public Order {
public:
  BuildOrder();
  ~BuildOrder();
};

class SailOrder : public Order {
public:
  SailOrder();
  ~SailOrder();
	
  AList dirs;
};

class FindOrder : public Order {
public:
  FindOrder();
  ~FindOrder();
	
  int find;
};

class StealOrder : public Order {
public:
  StealOrder();
  ~StealOrder();
	
  UnitId * target;
  int item;
};

class AssassinateOrder : public Order {
public:
  AssassinateOrder();
  ~AssassinateOrder();
	
  UnitId * target;
};

class ForgetOrder : public Order {
public:
  ForgetOrder();
  ~ForgetOrder();

  int skill;
};

class CastOrder : public Order {
public:
  CastOrder();
  ~CastOrder();
	
  int spell;
  int level;
};

class CastMindOrder : public CastOrder {
public:
  CastMindOrder();
  ~CastMindOrder();

  UnitId *id;
};

class TeleportOrder : public CastOrder {
public:
  TeleportOrder();
  ~TeleportOrder();

  int gate;
  int xloc;
  int yloc;
  AList units;
};

class CastRegionOrder : public CastOrder {
public:
  CastRegionOrder();
  ~CastRegionOrder();

  int xloc,yloc,zloc;
};

class CastIntOrder : public CastOrder {
public:
  CastIntOrder();
  ~CastIntOrder();
  
  int target;
};

class CastUnitsOrder : public CastOrder {
public:
  CastUnitsOrder();
  ~CastUnitsOrder();

  AList units;
};

class TrifleOrder : public Order {
public:
  TrifleOrder();
  ~TrifleOrder();

  AString *name;
};

#endif

