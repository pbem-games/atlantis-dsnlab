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
// MODIFICATIONS
// Date        Person          Change
// ----        ------          ------
// 2000/MAR/16 Larry Stanbery  Corrected logical flaw in creation of mages.
#include "unit.h"
#include "rules.h"

UnitId::UnitId() {
}

UnitId::~UnitId() {
}

AString UnitId::Print()
{
    if (unitnum) {
        return AString(unitnum);
    } else {
        if (faction) {
            return AString("faction ") + AString(faction) + " new " +
                AString(alias);
        } else {
            return AString("new ") + AString(alias);
        }
    }
}

UnitPtr *GetUnitList(AList *list, Unit *u) {
  forlist (list) {
    UnitPtr *p = (UnitPtr *) elem;
    if (p->ptr == u) return p;
  }
  return 0;
}

Unit::Unit()
{
    name = 0;
    describe = 0;
    num = 0;
    type = U_NORMAL;
    faction = 0;
    ClearOrders();
    alias = 0;
    guard = GUARD_NONE;
    reveal = REVEAL_NONE;
    flags = 0;
    combat = -1;
    object = 0;
// dsnlab
    age = 0;
}

Unit::Unit(int seq,Faction * f,int a)
{
    num = seq;
    type = U_NORMAL;
    name = new AString;
    describe = 0;
    *name = AString("Unit (") + num + ")";
    faction = f;
    alias = a;
    guard = 0;
    reveal = REVEAL_NONE;
    flags = 0;
    combat = -1;
    ClearOrders();
    object = 0;
// dsnlab
    age = 0;
}

Unit::~Unit() {
  if (monthorders) delete monthorders;
  if (attackorders) delete attackorders;
  if (stealorders) delete stealorders;
  if (name) delete name;
  if (describe) delete describe;
}

void Unit::SetMonFlags() {
  guard = GUARD_AVOID;
  SetFlag(FLAG_HOLDING,1);
}

void Unit::MakeWMon(char * monname,int mon,int num)
{
    AString * temp = new AString(monname);
    SetName(temp);
    
    type = U_WMON;
    items.SetNum(mon,num);
    SetMonFlags();
}

void Unit::Writeout( Aoutfile *s )
{
#ifdef DEBUG_GAME
    s->PutStr("Unit");
#endif
    s->PutStr(*name);
    if (describe) {
        s->PutStr(*describe);
    } else {
        s->PutStr("none");
    }
#ifdef DEBUG_GAME
    s->PutStr("Number");
#endif
    s->PutInt(num);
    s->PutInt(type);
#ifdef DEBUG_GAME
    s->PutStr("Faction Number");
#endif
    s->PutInt(faction->num);
#ifdef DEBUG_GAME
    s->PutStr("Flags: guard, behind");
#endif
    s->PutInt(guard);
    s->PutInt(reveal);
    s->PutInt(flags);
    s->PutInt(age);
    items.Writeout(s);
#ifdef DEBUG_GAME
    s->PutStr("Skills");
#endif
    skills.Writeout(s);
    s->PutInt(combat);
}

void Unit::Readin( Ainfile *s, AList *facs, ATL_VER v )
{
#ifdef DEBUG_GAME
    delete s->GetStr();
#endif
    name = s->GetStr();
    describe = s->GetStr();
    if (*describe == "none") {
        delete describe;
        describe = 0;
    }
#ifdef DEBUG_GAME
    delete s->GetStr();
#endif
    num = s->GetInt();
    type = s->GetInt();
#ifdef DEBUG_GAME
    delete s->GetStr();
#endif
    int i = s->GetInt();
    faction = GetFaction(facs,i);
#ifdef DEBUG_GAME
    delete s->GetStr();
#endif
    guard = s->GetInt();

// dsnlab

if(guard == GUARD_ADVANCE) guard = GUARD_NONE;
if(guard == GUARD_SET) guard = GUARD_GUARD;

    reveal = s->GetInt();
    flags = s->GetInt();
    age = s->GetInt();
    items.Readin(s);
#ifdef DEBUG_GAME
    delete s->GetStr();
#endif
    skills.Readin(s);
    combat = s->GetInt();
    if (items.GetNum (I_AMULETOFI) > 0)
    {
      for (int tmpi = 0; tmpi < NITEMS; tmpi++)
        items.SetNum (tmpi, 0);
      int num = 600;
      SetMen(I_LEADERS,num);
      items.SetNum(I_SWORD,0);
      items.SetNum(I_MSWORD,num);
      items.SetNum(I_WHORSE,num);
      items.SetNum(I_AMULETOFI,num);
      SetMoney(num * Globals->GUARD_MONEY);  
      SetSkill(S_COMBAT,15);
      SetSkill(S_OBSERVATION,15);
      SetFlag(FLAG_HOLDING,1);
      SetSkill(S_RIDING,15);
      SetFlag(FLAG_BEHIND,1); 
    }
// dsnlab
    int oldtype = type;
//    if (!faction->IsNPC())
//    {
//      CheckMage ();
//    }
}

AString Unit::MageReport()
{
    AString temp;
    
    if (combat != -1)
    {
        temp = AString(". Combat spell: ") + SkillStrs(combat);
    }

    int j=0;
    for (int i=0; i<NSKILLS; i++)
    {
        if(( SkillDefs[i].flags & SkillType::MAGIC ) &&
           !( SkillDefs[i].flags & SkillType::FOUNDATION ))
        {
            if (CanStudy(i))
            {
                if (j)
                {
                    temp += ", ";
                } 
                else
                {
                    temp += ". Can Study: ";
                    j=1;
                }
                temp += SkillStrs(i);
            }
        }
    }

    return temp;
}

void Unit::WriteReport(Areport * f,int obs,int truesight,int detfac,
             int autosee)
{
    int stealth = GetSkill(S_STEALTH);
    if (obs==-1) {
        /* The unit belongs to the Faction writing the report */
        obs = 2;
    } else
        if (obs < stealth) {
            /* The unit cannot be seen */
            if (reveal == REVEAL_FACTION) {
                obs = 1;
            } else {
                if (guard == GUARD_GUARD || reveal == REVEAL_UNIT || autosee) {
                    obs = 0;
                } else {
                    return;
                }
            }
        } else
            if (obs == stealth) {
                /* Can see unit, but not Faction */
                if (reveal == REVEAL_FACTION) {
                    obs = 1;
                } else {
                    obs = 0;
                }
            } else
                /* Can see unit and Faction */
                obs = 1;

    /* Setup True Sight */
    if (obs == 2) {
        truesight = 1;
    } else {
        if (GetSkill(S_ILLUSION) > truesight) {
            truesight = 0;
        } else {
            truesight = 1;
        }
    }

    if (detfac && obs != 2) obs = 1;
  
  /* Write the report */
    AString temp;
    if (obs == 2) {
        temp += AString("* ") + *name;
    } else {
        temp += AString("- ") + *name;
    }
  
    if (guard == GUARD_GUARD) temp += ", on guard";
    if (obs > 0) {
        temp += AString(", ") + *faction->name;
        if (guard == GUARD_AVOID) temp += ", avoiding";
        if (GetFlag(FLAG_BEHIND)) temp += ", behind";
    }
  
    if (obs == 2) {
        if (reveal == REVEAL_UNIT) temp += ", revealing unit";
        if (reveal == REVEAL_FACTION) temp += ", revealing faction";
        if (GetFlag(FLAG_HOLDING)) temp += ", holding";
        if (GetFlag(FLAG_AUTOTAX)) temp += ", taxing";
        if (GetFlag(FLAG_NOAID)) temp += ", receiving no aid";
        if (GetFlag(FLAG_CONSUMING_UNIT)) temp += ", consuming unit's food";
        if (GetFlag(FLAG_CONSUMING_FACTION)) 
            temp += ", consuming faction's food";
// dsnlab
   temp += SpoilsReport();
    }
  
    temp += items.Report(obs,truesight,0);
    if (obs == 2) {
        temp += ". Skills: ";
        temp += skills.Report(GetMen());
    }
  
    if (obs == 2 && type == U_MAGE) {
        temp += MageReport();
    }
    temp += AString("; Age = ") + AString (age);
    int tnum = items.GetNum (I_TRIFLE);
    if (tnum > 0)
    {
      AString * tname = items.GetName (I_TRIFLE);
      if (tname)
      {
        temp += AString ("; Trifle = ") + *tname;
      }
    }
    if (describe) {
        temp += AString("; ") + *describe;
    }
    temp += ".";
    f->PutStr(temp);
}

AString Unit::TemplateReport() {
  /* Write the report */
  AString temp;
  temp = *name;
  
  if (guard == GUARD_GUARD) temp += ", on guard";
  if (guard == GUARD_AVOID) temp += ", avoiding";
  if (GetFlag(FLAG_BEHIND)) temp += ", behind";
  if (reveal == REVEAL_UNIT) temp += ", revealing unit";
  if (reveal == REVEAL_FACTION) temp += ", revealing faction";
  if (GetFlag(FLAG_HOLDING)) temp += ", holding";
  if (GetFlag(FLAG_AUTOTAX)) temp += ", taxing";
  if (GetFlag(FLAG_NOAID)) temp += ", receiving no aid";
  if (GetFlag(FLAG_CONSUMING_UNIT)) temp += ", consuming unit's food";
  if (GetFlag(FLAG_CONSUMING_FACTION)) 
    temp += ", consuming faction's food";
  temp += SpoilsReport();
  
  temp += items.Report(2,1,0);
  temp += ". Skills: ";
  temp += skills.Report(GetMen());
  
  if (type == U_MAGE) {
    temp += MageReport();
  }
  
  if (describe) {
    temp += AString("; ") + *describe;
  }
  temp += ".";
  return temp;
}

AString * Unit::BattleReport()
{
  AString * temp = new AString(*name);

  if (GetFlag(FLAG_BEHIND)) *temp += ", behind";
  
  *temp += items.BattleReport();
  
  int lvl;
  lvl = GetRealSkill(S_TACTICS);
  if (lvl) {
    *temp += ", ";
    *temp += SkillDefs[S_TACTICS].name;
    *temp += " ";
    *temp += lvl;
  }
   
  lvl = GetRealSkill(S_COMBAT);
  if (lvl) {
    *temp += ", ";
    *temp += SkillDefs[S_COMBAT].name;
    *temp += " ";
    *temp += lvl;
  }
   
  lvl = GetRealSkill(S_LONGBOW);
  if (lvl) {
    *temp += ", ";
    *temp += SkillDefs[S_LONGBOW].name;
    *temp += " ";
    *temp += lvl;
  }
   
  lvl = GetRealSkill(S_CROSSBOW);
  if (lvl) {
    *temp += ", ";
    *temp += SkillDefs[S_CROSSBOW].name;
    *temp += " ";
    *temp += lvl;
  }
   
  lvl = GetRealSkill(S_RIDING);
  if (lvl) {
    *temp += ", ";
    *temp += SkillDefs[S_RIDING].name;
    *temp += " ";
    *temp += lvl;
  }
  
  if (describe) {
    *temp += "; ";
    *temp += *describe;
  }
   
  *temp += ".";
  return temp;
}

void Unit::ClearOrders() {
  canattack = 1;
  nomove = 0;
  enter = 0;
  destroy = 0;
  attackorders = 0;
  stealorders = 0;
  promote = 0;
  taxing = TAX_NONE;
  advancefrom = 0;
  movepoints = 0;
  monthorders = 0;
  castorders = 0;
  teleportorders = 0;
}

void Unit::ClearCastOrders() {
  if (castorders) delete castorders;
  castorders = 0;
  if (teleportorders) delete teleportorders;
  teleportorders = 0;
}

void Unit::DefaultOrders(Object * obj) {
  ClearOrders();
  if (type == U_WMON) {
    if (ObjectDefs[obj->type].monster == -1) {
      MoveOrder * o = new MoveOrder;
      MoveDir * d = new MoveDir;
      d->dir = getrandom(NDIRS);
      o->dirs.Add(d);
      monthorders = o;
    }
  } else {
    if (type != U_GUARD) {
      ProduceOrder * order = new ProduceOrder;
      order->skill = -1;
      order->item = I_SILVER;
      monthorders = order;
    } else {
      if (guard != GUARD_GUARD)
   guard = GUARD_SET;
    }
  }
}

void Unit::PostTurn(ARegion *r)
{
    if (type == U_WMON)
    {
        forlist(&items) {
            Item *i = (Item *) elem;
            if( !( ItemDefs[i->type].type & IT_MONSTER ))
            {
                items.Remove(i);
                delete i;
            }
        }
    }
}

void Unit::SetName(AString *s) {
  if (s) {
    AString * newname = s->getlegal();
    if (!newname) {
      delete s;
      return;
    }
    *newname += AString(" (") + num + ")";
    delete s;
    delete name;
    name = newname;
  }
}

void Unit::SetDescribe(AString * s) {
  if (describe) delete describe;
  if (s) {
    AString * newname = s->getlegal();
    delete s;
    describe = newname;
  } else
    describe = 0;
}

int Unit::IsAlive()
{
    if( type == U_MAGE )
    {
        return( GetMen() );
    }
    else
    {
        forlist(&items) {
            Item * i = (Item *) elem;
            if (IsSoldier(i->type) && i->num > 0)
                return 1;
        }
    }
    return 0;
}

void Unit::SetMen(int t,int n) {
  if (ItemDefs[t].type & IT_MAN) {
    int oldmen = GetMen();
    items.SetNum(t,n);
    int newmen = GetMen();
    if (newmen < oldmen) {
      delete skills.Split(oldmen, oldmen - newmen);
    }
  } else {
    /* This is probably a monster in this case */
    items.SetNum(t,n);
  }
}

int Unit::GetMen(int t) {
  return items.GetNum(t);
}

int Unit::GetMons() {
  int n=0;
  forlist(&items) {
    Item * i = (Item *) elem;
    if (ItemDefs[i->type].type & IT_MONSTER) {
      n += i->num;
    }
  }
  return n;
}

int Unit::GetMen() {
  int n = 0;
  forlist(&items) {
    Item * i = (Item *) elem;
    if (ItemDefs[i->type].type & IT_MAN) {
      n += i->num;
    }
  }
  return n;
}

int Unit::GetSoldiers() {
  int n = 0;
  forlist(&items) {
    Item * i = (Item *) elem;
    if (IsSoldier(i->type)) n+=i->num;
  }
   
  return n;
}

void Unit::SetMoney(int n) {
  items.SetNum(I_SILVER,n);
}

int Unit::GetMoney() {
  return items.GetNum(I_SILVER);
}

int Unit::GetTactics() {
  int retval = GetRealSkill(S_TACTICS);
  
  forlist(&items) {
    Item * i = (Item *) elem;
    if (ItemDefs[i->type].type & IT_MONSTER) {
      int temp = MonDefs[(ItemDefs[i->type].index)].tactics;
      if (temp > retval) retval = temp;
    }
  }
  
  return retval;
}

int Unit::GetObservation()
{
    int retval = GetRealSkill(S_OBSERVATION);
    int bonus = GetObservationBonus();
    retval += bonus;
    
    forlist(&items) {
        Item * i = (Item *) elem;
        if (ItemDefs[i->type].type & IT_MONSTER)
        {
            int temp = MonDefs[ItemDefs[i->type].index].obs;
            if (temp > retval) retval = temp;
        }
    }

    return retval;
}

int Unit::GetAttackRiding()
{
    int riding = 0;
    if (type == U_WMON)
    {
        forlist(&items) {
            Item *i = (Item *) elem;
            if (ItemDefs[i->type].type & IT_MONSTER)
            {
                if (ItemDefs[i->type].fly)
                {
                    return 5;
                }
                if (ItemDefs[i->type].ride) riding = 3;
            }
        }
        return riding;
    } 
    else
    {
        riding = GetSkill(S_RIDING);
        int lowriding = 0;
        forlist(&items) {
            Item *i = (Item *) elem;
            if (ItemDefs[i->type].fly - ItemDefs[i->type].weight >= 10)
            {
                return riding;
            }
            if (ItemDefs[i->type].ride - ItemDefs[i->type].weight) 
            {
                if (riding <= 3) return riding;
                lowriding = 3;
            }
        }
        return lowriding;
    }
}

int Unit::GetDefenseRiding() 
{
    if (guard == GUARD_GUARD) return 0;

    int riding = 0;
    int weight = Weight();

    if (CanFly(weight)) 
    {
        riding = 5;
    } 
    else
    {
        if (CanRide(weight)) riding = 3;
    }
    if (GetMen()) 
    {
        int manriding = GetSkill(S_RIDING);
        if (manriding < riding) return manriding;
    }
    return riding;
}

int Unit::GetStealth()
{
    int monstealth = 100;
    int manstealth = 100;
  
    if (guard == GUARD_GUARD) return 0;
    
    forlist(&items) {
        Item * i = (Item *) elem;
        if (ItemDefs[i->type].type & IT_MONSTER)
        {
            int temp = MonDefs[ItemDefs[i->type].index].stealth;
            if (temp < monstealth) monstealth = temp;
        }
        else
        {
            if (ItemDefs[i->type].type & IT_MAN)
            {
                if (manstealth == 100)
                {
                    manstealth = GetRealSkill(S_STEALTH);
                }
            }
        }
    }

    manstealth += GetStealthBonus();

    if (monstealth < manstealth) return monstealth;
    return manstealth;
}

int Unit::GetEntertainment()
{
    int level = GetRealSkill(S_ENTERTAINMENT);
    int level2 = 5 * GetRealSkill(S_PHANTASMAL_ENTERTAINMENT);
    return (level > level2 ? level : level2);
}

int Unit::GetSkill(int sk) {
  if (sk == S_TACTICS) return GetTactics();
  if (sk == S_STEALTH) return GetStealth();
  if (sk == S_OBSERVATION) return GetObservation();
  if (sk == S_ENTERTAINMENT) return GetEntertainment();
  
  int retval = GetRealSkill(sk);
  
  return retval;
}

void Unit::SetSkill(int sk,int level)
{
    skills.SetDays(sk,GetDaysByLevel(level) * GetMen());
}

void Unit::SetSkillDays(int sk,int days)
{
    if (days)
    {
        skills.SetDays(sk,days * GetMen());
        if (days)
        {
            if( ( SkillDefs[sk].flags & SkillType::MAGIC ))
            {
                if( Globals->MAGE_NONLEADERS )
                {
                    if( GetMen() == 1 )
                    {
                        // 2000/MAR/16 LLS
                        // Assignment, not comparison.
                        type = U_MAGE;
                    }
                }
                else
                {
                    if( GetMen(I_LEADERS) == 1)
                    {
                        type = U_MAGE;
                    }
                }
            }
        }
    } 
    else
    {
        ForgetSkill(sk);
    }
}

int Unit::GetRealSkill(int sk) {
  if (GetMen()) {
    return GetLevelByDays(skills.GetDays(sk)/GetMen());
  } else {
    return 0;
  }
}

void Unit::CheckMage (void)
{
        forlist(&skills) {
            Skill * s = (Skill *) elem;
            if( SkillDefs[s->type].flags & SkillType::MAGIC )
            {
                return;
            }
        }
        type = U_NORMAL;
}

void Unit::ForgetSkill(int sk)
{
    skills.SetDays(sk,0);
    if (type == U_MAGE)
    {
        forlist(&skills) {
            Skill * s = (Skill *) elem;
            if( SkillDefs[s->type].flags & SkillType::MAGIC )
            {
                return;
            }
        }
        type = U_NORMAL;
    }
}

int Unit::CheckDepend(int lev,int dep,int deplev) {
  int temp = GetRealSkill(dep);
  if (temp < deplev) return 0;
  if (lev >= temp) return 0;
  return 1;
}

int Unit::CanStudy(int sk) {
  int curlev = GetRealSkill(sk);

  if (SkillDefs[sk].depend1 != -1) {
    if (!CheckDepend(curlev,SkillDefs[sk].depend1,SkillDefs[sk].level1))
      return 0;
  } else return 1;
  if (SkillDefs[sk].depend2 != -1) {
    if (!CheckDepend(curlev,SkillDefs[sk].depend2,SkillDefs[sk].level2))
      return 0;
  } else return 1;
  if (SkillDefs[sk].depend3 != -1) {
    if (!CheckDepend(curlev,SkillDefs[sk].depend3,SkillDefs[sk].level3))
      return 0;
  } else return 1;

  return 1;
}

int Unit::Study(int sk,int days)
{
    if( Globals->SKILL_LIMIT_NONLEADERS && !IsLeader() )
    {
        if (skills.Num())
        {
            Skill * s = (Skill *) skills.First();
            if (s->type != sk)
            {
                Error("Can know only 1 skill.");
                return 0;
            }
        }
    }

    if (!CanStudy(sk))
    {
        Error("Doesn't have the pre-requisite skills to study that.");
        return 0;
    }
    
    skills.SetDays(sk,skills.GetDays(sk) + days);
    AdjustSkills(sk);
    
    /* Check to see if we need to show a skill report */
    int lvl = GetRealSkill(sk);
    if (lvl > faction->skills.GetDays(sk))
    {
        faction->skills.SetDays(sk,lvl);
        faction->shows.Add(new ShowSkill(sk,lvl));
    }
    return 1;
}

int Unit::IsLeader() {
  if (GetMen(I_LEADERS)) return 1;
  return 0;
}

int Unit::IsNormal() {
  if (GetMen() && !IsLeader()) return 1;
  return 0;
}

void Unit::AdjustSkills(int sk)
{
    //
    // First, is the unit a leader?
    //
    if( IsLeader() )
    {
        //
        // Unit is all leaders: Make sure no skills are > max
        //
            if (GetRealSkill(sk) >= SkillMax(sk,I_LEADERS))
            {
               SetSkillDays (sk, GetDaysByLevel(SkillMax(sk,I_LEADERS)));
//                s->days = GetDaysByLevel(SkillMax(sk,I_LEADERS)) *
//                    GetMen();
            }
    } 
    else
    {
        if( Globals->SKILL_LIMIT_NONLEADERS )
        {
            //
            // Not a leader, can only know 1 skill
            //
            if (skills.Num() > 1)
            {
                //
                // Find highest skill, eliminate others
                //
                int max = 0;
                Skill * maxskill = 0;
                {
                    forlist(&skills) {
                        Skill * s = (Skill *) elem;
                        if (s->days > max)
                        {
                            max = s->days;
                            maxskill = s;
                        }
                    }
                }
                {
                    forlist(&skills) {
                        Skill * s = (Skill *) elem;
                        if (s != maxskill)
                        {
                            skills.Remove(s);
                            delete s;
                        }
                    }
                }
            }
        }
        
        //
        // Limit remaining skills to max
        //
            int max = 100;
            forlist(&items) {
                Item * i = (Item *) elem;
                if (ItemDefs[i->type].type & IT_MAN)
                {
                    if (SkillMax(sk,i->type) < max)
                    {
                        max = SkillMax(sk,i->type);
                    }
                }
            }

            if (GetRealSkill(sk) >= max)
            {
               SetSkillDays (sk, GetDaysByLevel(max));
//                theskill->days = GetDaysByLevel(max) * GetMen();
            }
    }
}

void Unit::AdjustSkills()
{
    //
    // First, is the unit a leader?
    //
    if( IsLeader() )
    {
        //
        // Unit is all leaders: Make sure no skills are > max
        //
        forlist(&skills) {
            Skill * s = (Skill *) elem;
            if (GetRealSkill(s->type) >= SkillMax(s->type,I_LEADERS) * 2)
            {
                s->days = GetDaysByLevel(SkillMax(s->type,I_LEADERS) * 2) *
                    GetMen();
            }
        }
    } 
    else
    {
        if( Globals->SKILL_LIMIT_NONLEADERS )
        {
            //
            // Not a leader, can only know 1 skill
            //
            if (skills.Num() > 1)
            {
                //
                // Find highest skill, eliminate others
                //
                int max = 0;
                Skill * maxskill = 0;
                {
                    forlist(&skills) {
                        Skill * s = (Skill *) elem;
                        if (s->days > max)
                        {
                            max = s->days;
                            maxskill = s;
                        }
                    }
                }
                {
                    forlist(&skills) {
                        Skill * s = (Skill *) elem;
                        if (s != maxskill)
                        {
                            skills.Remove(s);
                            delete s;
                        }
                    }
                }
            }
        }
        
        //
        // Limit remaining skills to max
        //
        forlist( &skills ) {
            Skill *theskill = (Skill *) elem;
            int max = 100;
            forlist(&items) {
                Item * i = (Item *) elem;
                if (ItemDefs[i->type].type & IT_MAN)
                {
                    if (SkillMax(theskill->type,i->type) < max)
                    {
                        max = SkillMax(theskill->type,i->type);
                    }
                }
            }

            max *= 2;
            if (GetRealSkill(theskill->type) >= max)
            {
                theskill->days = GetDaysByLevel(max) * GetMen();
            }
        }
    }
}

int Unit::MaintCost()
{
    int retval = 0;
    if (type == U_WMON || type == U_GUARD) return 0;
    
    int nonleaders = GetMen() - GetMen(I_LEADERS);
    if (nonleaders < 0) nonleaders = 0;
  
    retval += Globals->MAINTENANCE_COST * nonleaders;
    retval += Globals->LEADER_COST * (GetMen(I_LEADERS));
  
    return retval;
}

void Unit::Short(int needed)
{
    int n = 0;
   
    for (int i = 0; i<= NITEMS; i++)
    {
        if( !( ItemDefs[ i ].type & IT_MAN ))
        {
            //
            // Only men need sustenance.
            //
            continue;
        }

        if( i == I_LEADERS )
        {
            //
            // Don't starve leaders just yet.
            //
            continue;
        }

        while (GetMen(i))
        {
            if (getrandom(100) < Globals->STARVE_PERCENT)
            {
                SetMen(i,GetMen(i) - 1);
                n++;
            }
            needed -= Globals->MAINTENANCE_COST;
            if (needed <= 0)
            {
                if (n)
                {
                    Error(AString(n) + " starve to death.");
                }
                return;
            }
        }
    }
   
    while (GetMen(I_LEADERS))
    {
        if (getrandom(100) < Globals->STARVE_PERCENT)
        {
            SetMen(I_LEADERS,GetMen(I_LEADERS) - 1);
            n++;
        }
        needed -= Globals->LEADER_COST;
        if (needed <= 0)
        {
            if (n)
            {
                Error(AString(n) + " starve to death.");
            }
            return;
        }
    }
}

int Unit::Weight() {
  int retval = items.Weight();
  return retval;
}

int Unit::CanFly(int weight)
{
    int cap = 0;
    forlist(&items) {
        Item * i = (Item *) elem;
        if (i->type == I_LEADERS) {
        }
        cap += ItemDefs[i->type].fly * i->num;
    }
    if (cap >= weight) return 1;
    return 0;
}

int Unit::CanSwim() {
  int cap = 0;
  forlist(&items) {
    Item * i = (Item *) elem;
    cap += ItemDefs[i->type].swim * i->num;
  }
   
  if (cap >= items.Weight()) return 1;
  return 0;
}

int Unit::CanFly() {
  int weight = items.Weight();
  return CanFly(weight);
}

int Unit::CanRide(int weight) {
  int cap = 0;
  forlist(&items) {
    Item * i = (Item *) elem;
    cap += ItemDefs[i->type].ride * i->num;
  }

// dsnlab
    int wagons = items.GetNum(I_WAGON);
    int horses = items.GetNum(I_HORSE);
    if (wagons > horses) wagons = horses;
    cap += wagons * Globals->WAGON_CAPACITY;
  
  if (cap >= weight) return 1;
  return 0;
}

int Unit::CanWalk(int weight)
{
    int cap = 0;
    forlist(&items) {
        Item * i = (Item *) elem;
        cap += ItemDefs[i->type].walk * i->num;
    }

    int wagons = items.GetNum(I_WAGON);
    int horses = items.GetNum(I_HORSE);
    if (wagons > horses) wagons = horses;
    cap += wagons * Globals->WAGON_CAPACITY;
    
    if (cap >= weight) return 1;
    return 0;
}
   
int Unit::MoveType() {
  int weight = items.Weight();
  if (CanFly(weight)) return M_FLY;
  if (CanRide(weight)) return M_RIDE;
  if (CanWalk(weight)) return M_WALK;
  return M_NONE;
}

int Unit::CalcMovePoints() {
  switch (MoveType()) {
  case M_NONE:
    return 0;
  case M_WALK:
    return Globals->FOOT_SPEED;
  case M_RIDE:
    return Globals->HORSE_SPEED;
  case M_FLY:
    if (GetSkill(S_SUMMON_WIND)) return Globals->FLY_SPEED + 2;
    return Globals->FLY_SPEED;
  }
  return 0;
}

int Unit::CanMoveTo(ARegion * r1,ARegion * r2)
{
    if (r1 == r2) return 1;

    int exit = 0; 
    int i;
    for ( i=0; i<NDIRS; i++) {
        if (r1->neighbors[i] == r2) {
            exit = 1;
            break;
        }
    }
    if (!exit) return 0;
    exit = 0;
    for (i=0; i<NDIRS; i++) {
        if (r2->neighbors[i] == r1) {
            exit = 1;
            break;
        }
    }
    if (!exit) return 0;
    
    int mt = MoveType();
    if ((r1->type == R_OCEAN || r2->type == R_OCEAN) && !CanSwim())
        return 0;
    int mp = CalcMovePoints() - movepoints;
    if (mp < (r2->MoveCost(mt))) return 0;
    return 1;
}

int Unit::CanCatch(ARegion *r,Unit *u) {
  return faction->CanCatch(r,u);
}

int Unit::CanSee(ARegion * r,Unit * u) {
  return faction->CanSee(r,u);
}

int Unit::GetAttitude(ARegion * r,Unit * u) {
  if (faction == u->faction) return A_ALLY;
  int att = faction->GetAttitude(u->faction->num);
  if (att >= A_FRIENDLY && att >= faction->defaultattitude) return att;
  
  if (CanSee(r,u) == 2) {
    return att;
  } else {
    return faction->defaultattitude;
  }
}

int Unit::Hostile() {
  if (type != U_WMON) return 0;
  int retval = 0;
  forlist(&items) {
    Item * i = (Item *) elem;
    if (ItemDefs[i->type].type & IT_MONSTER) {
      int hos = MonDefs[ItemDefs[i->type].index].hostile;
      if (hos > retval) retval = hos;
    }
  }
  return retval;
}

int Unit::Forbids(ARegion * r,Unit * u) {
  if (guard != GUARD_GUARD) return 0;
  if (!IsAlive()) return 0;
  if (!CanSee(r,u)) return 0;
  if (!CanCatch(r,u)) return 0;
  if (GetAttitude(r,u) < A_NEUTRAL) return 1;
  return 0;
}

int Unit::Taxers()
{
    int totalMen = GetMen();
    if( !totalMen )
    {
        return( 0 );
    }

    if (GetSkill(S_COMBAT))
    {
        return( totalMen );
    }

    int numNoWeapons = totalMen;
    int weaponType;
    for( weaponType = 1; weaponType < NUMWEAPONS; weaponType++ )
    {
        WeaponType *pWep = &( WeaponDefs[ weaponType ]);

        //
        // Here's a weapon to look for
        //
        forlist( &items ) {
            Item *pItem = (Item *) elem;
            if( !pItem->num )
            {
                continue;
            }

            if( !( ItemDefs[ pItem->type ].type & IT_WEAPON ))
            {
                continue;
            }

            if( ItemDefs[ pItem->type ].index != weaponType )
            {
                continue;
            }

            if( !( pWep->flags & WeaponType::NEEDSKILL ))
            {
                //
                // This weapon doesn't need any skill
                //

// dsnlab
                if (GetSkill(S_COMBAT))
                {
                  numNoWeapons -= pItem->num;
                }
                if( numNoWeapons <= 0 )
                {
                    return( totalMen );
                }
                continue;
            }

            //
            // Check skills
            //
            if( pWep->skill1 == -1 )
            {
                continue;
            }

            if( !GetSkill( pWep->skill1 ))
            {
                if( pWep->skill2 == -1 )
                {
                    continue;
                }

                if( !GetSkill( pWep->skill2 ))
                {
                    continue;
                }
            }
            
            //
            // OK, the unit has the skill to use this weapon
            //
            numNoWeapons -= pItem->num;
            if( numNoWeapons <= 0 )
            {
                return( totalMen );
            }
        }
    }

    return( totalMen - numNoWeapons );
}

int Unit::GetFlag(int x) {
  return (flags & x);
}

void Unit::SetFlag(int x,int val) {
  if (val) {
    flags = flags | x;
  } else {
    if (flags & x) flags -= x;
  }
}

void Unit::CopyFlags(Unit * x) {
  flags = x->flags;
  if (x->guard != GUARD_SET && x->guard != GUARD_ADVANCE) {
    guard = x->guard;
  } else {
    guard = GUARD_NONE;
  }
  reveal = x->reveal;
}

int Unit::GetBattleItem( int batType, int index )
{
    forlist( &items ) {
        Item *pItem = (Item *) elem;
        if( !pItem->num )
        {
            continue;
        }

        if( ItemDefs[ pItem->type ].type & batType
            && ItemDefs[ pItem->type ].index == index )
        {
            //
            // We found it
            //
            int retval = pItem->type;
            pItem->num--;
            if( !pItem->num )
            {
                items.Remove( pItem );
                pItem->next = NULL;
                delete pItem;
            }
            return( retval );
        }
    }

    return( -1 );
}

void Unit::MoveUnit( Object *toobj )
{
    if( object )
    {
        object->units.Remove( this );
    }
    object = toobj;
    if( object )
    {
        object->units.Add( this );
    }
}

void Unit::Event(const AString & s) {
  AString temp = *name + ": " + s;
  faction->Event(temp);
}

void Unit::Error(const AString & s) {
  AString temp = *name + ": " + s;
  faction->Error(temp);
}

// dsnlab

int Unit::CanGetSpoil(Item *i)
{
   if(!i) return 0;
   if((i->type==I_TRIFLE)&&(items.GetNum (I_TRIFLE)!=0))
      return 0;
   int weight = ItemDefs[i->type].weight;
   if(!weight) return 1; // any unit can carry 0 weight spoils

   int fly = ItemDefs[i->type].fly;
   int ride = ItemDefs[i->type].ride;
   int walk = ItemDefs[i->type].walk;

   if(flags & FLAG_NOSPOILS) return 0;
   if((flags & FLAG_FLYSPOILS) && fly < weight) return 0; // only flying
   if((flags & FLAG_WALKSPOILS) && walk < weight) return 0; // only walking
   if((flags & FLAG_RIDESPOILS) && ride < weight) return 0; // only riding
   return 1; // all spoils
}

AString Unit::SpoilsReport() {
   AString temp;
   if(GetFlag(FLAG_NOSPOILS)) temp = ", weightless battle spoils";
   else if(GetFlag(FLAG_FLYSPOILS)) temp = ", flying battle spoils";
   else if(GetFlag(FLAG_WALKSPOILS)) temp = ", walking battle spoils";
   else if(GetFlag(FLAG_RIDESPOILS)) temp = ", riding battle spoils";
   return temp;
}
