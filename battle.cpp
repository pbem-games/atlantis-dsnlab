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
#include "game.h"
#include "battle.h"
#include "army.h"
#include "gamedefs.h"
#include "rules.h"

Battle::Battle()
{
    asstext = 0;
}

Battle::~Battle()
{
    if (asstext)
    {
        delete asstext;
    }
}

void Battle::FreeRound(Army * att,Army * def)
{
    /* Write header */
    AddLine(*(att->leader->name) + " gets a free round of attacks.");
    
    /* Update both army's shields */
    att->shields.DeleteAll();
    UpdateShields(att);
    
    def->shields.DeleteAll();
    UpdateShields(def);

    //
    // Update the attacking armies round counter
    //
    att->round++;

    /* Run attacks until done */
    int alv = def->NumAlive();
    while (att->CanAttack() && def->NumAlive())
    {
        int num = getrandom(att->CanAttack());
        int behind;
        Soldier * a = att->GetAttacker(num, behind);
        DoAttack(att->round, a, att, def, behind);
    }
    
    /* Write losses */
// Dweller - print statixtic by remained FSHI which have been participated in battle
    att->WriteRemainedFSHIStatistic( this );
    def->WriteRemainedFSHIStatistic( this );
// end of Dweller
    alv -= def->NumAlive();
    AddLine(*(def->leader->name) + " loses " + alv + ".");
    AddLine("");
    att->Reset();
}

void Battle::DoAttack( int round, 
                       Soldier *a,
                       Army *attackers,
                       Army *def, 
                       int behind )
{
    DoSpecialAttack( round, a, attackers, def, behind );
    if (!def->NumAlive()) return;
    
    if (a->HasEffect(EFFECT_DAZZLE)) a->askill -= 2;

    int numAttacks = a->attacks;
    if( a->attacks < 0 )
    {
        if( round % ( -1 * a->attacks ) == 1 )
        {
            // Tails: added for Catapult
			// WAS: numAttacks = 1;
			if (a->weapon == WEAPON_CATAPULT)
				numAttacks = 80;
			else
				numAttacks = 1;
        }
        else
        {
            numAttacks = 0;
        }
    }

    for (int i = 0; i < numAttacks; i++ )
    {
        WeaponType *pWep = 0;
        if( a->weapon != -1 )
        {
            pWep = &WeaponDefs[ ItemDefs[ a->weapon ].index ];
        }

        if( behind )
        {
            if( !pWep )
            {
                break;
            }

            if( !( pWep->flags & WeaponType::RANGED ))
            {
                break;
            }
        }

        int flags = 0;
        if( pWep )
        {
            flags = pWep->flags;
        }
		
        def->DoAnAttack( 0, 1, ATTACK_COMBAT, a->askill, flags, 0, 0, a->weapon );
        if (!def->NumAlive()) break;
    }
    
    if (a->HasEffect(EFFECT_DAZZLE)) {
        a->ClearEffect(EFFECT_DAZZLE);
        a->askill += 2;
    }
}

void Battle::NormalRound(int round,Army * a,Army * b)
{
    /* Write round header */
    AddLine(AString("Round ") + round + ":");
    
    /* Update both army's shields */
    UpdateShields(a);
    UpdateShields(b);
	
    /* Initialize variables */
    a->round++;
    b->round++;
    int aalive = a->NumAlive();
    int aialive = aalive;
    int balive = b->NumAlive();
    int bialive = balive;
    int aatt = a->CanAttack();
    int batt = b->CanAttack();
	
    /* Run attacks until done */
    while (aalive && balive && (aatt || batt))
    {
        int num = getrandom(aatt + batt);
        int behind;
        if (num >= aatt)
        {
            num -= aatt;
            Soldier * s = b->GetAttacker(num, behind);
            DoAttack(b->round, s, b, a, behind);
        } 
        else
        {
            Soldier * s = a->GetAttacker(num, behind);
            DoAttack(a->round, s, a, b, behind);
        }
        aalive = a->NumAlive();
        balive = b->NumAlive();
        aatt = a->CanAttack();
        batt = b->CanAttack();
    }
    
    /* Finish round */
// Dweller - print statixtic by remained FSHI which have been participated in battle
    a->WriteRemainedFSHIStatistic( this );
    b->WriteRemainedFSHIStatistic( this );
// end of Dweller
    aialive -= aalive;
    AddLine(*(a->leader->name) + " loses " + aialive + ".");
    bialive -= balive;
    AddLine(*(b->leader->name) + " loses " + bialive + ".");
    AddLine("");
    a->Reset();
    b->Reset();
}

/*
void Battle::GetSpoils(AList * losers, ItemList *spoils) {
  forlist(losers) {
    Unit * u = ((Location *) elem)->unit;
    int numalive = u->GetSoldiers();
    int numdead = u->losses;
    forlist(&u->items) {
      Item * i = (Item *) elem;
      if (!IsSoldier(i->type)) {
	int num = (i->num * numdead + getrandom(numalive + numdead)) /
	  (numalive + numdead);
	u->items.SetNum(i->type,i->num - num);
	num = (num + getrandom(2)) / 2;
	spoils->SetNum(i->type,spoils->GetNum(i->type) + num);
      }
    }
  }
}
*/
  void Battle::GetSpoils(AList * losers, ItemList *spoils) {
    forlist(losers) {
      Unit * u = ((Location *) elem)->unit;
      int numalive = u->GetSoldiers();
      int numdead = u->losses;
      forlist(&u->items) {
        Item * i = (Item *) elem;
        if ((!IsSoldier(i->type))&&(i->type!=I_TRIFLE)) {
          int o1 = i->num;
          int o2 = numdead;
          int num = int ((float (i->num) * float (numdead) + float (getrandom(numalive + numdead))) / float ((numalive + numdead)));
		  
          // store the item type in a local variable
          int it = i->type;
          u->items.SetNum(i->type,i->num - num);
          num = (num + getrandom(2)) / 2;
          // use that local variable instead of pointer which may be
          // already deallocated by a call to SetNum()
          //spoils->SetNum(i->type,spoils->GetNum(i->type) + num);
          if ((it,spoils->GetNum(it) + num) < 0)
          {
            Awrite (AString (u->num) + " " + it + " " + (o1*o2) + " " + o2);
          }
          spoils->SetNum(it,spoils->GetNum(it) + num);
        }
		else if(i->type==I_TRIFLE){
			spoils->AddItem(I_TRIFLE,u->items.GetName(I_TRIFLE));
			u->items.SetNum(i->type,0);
			Awrite (u->items.GetName(I_TRIFLE)->Str());
		}
      }
    }
  }

int Battle::Run( ARegion * region,
                  Unit * att,
                  AList * atts,
                  Unit * tar,
                  AList * defs,
                  int ass,
                  ARegionList *pRegs )
{
    Army * armies[2];
    assassination = ASS_NONE;
    attacker = att->faction;
  
    armies[0] = new Army(att,atts,region->type,ass);
    armies[1] = new Army(tar,defs,region->type,ass);
  
    if (ass) {
        FreeRound(armies[0],armies[1]);
    } else {
        if (armies[0]->tac > armies[1]->tac) FreeRound(armies[0],armies[1]);
        if (armies[1]->tac > armies[0]->tac) FreeRound(armies[1],armies[0]);
    }
  
    int round = 1;
    while (!armies[0]->Broken() && !armies[1]->Broken() && round < 101) {
        NormalRound(round++,armies[0],armies[1]);
    }
  
    if ((armies[0]->Broken() && !armies[1]->Broken()) ||
        (!armies[0]->NumAlive() && armies[1]->NumAlive())) {
        if (ass) assassination = ASS_FAIL;
        AddLine(*(armies[0]->leader->name) + " is routed!");
        if (armies[0]->NumAlive())
            FreeRound(armies[1],armies[0]);
        AddLine("Total Casualties:");
        ItemList *spoils = new ItemList;
        armies[0]->Lose(this,spoils);
        GetSpoils(atts, spoils);
        AString temp;
        if (spoils->Num()) {
            temp = AString("Spoils: ") + spoils->Report(2,0,1,1) + ".";
        } else {
            temp = "No spoils.";
        }
        armies[1]->Win(this,spoils);
        AddLine("");
        AddLine(temp);
        AddLine("");
        delete spoils;
        delete armies[0];
        delete armies[1];
        return BATTLE_LOST;
    }

    if ((armies[1]->Broken() && !armies[0]->Broken()) ||
        (!armies[1]->NumAlive() && armies[0]->NumAlive())) {
        if (ass) {
            assassination = ASS_SUCC;
            asstext = new AString(*(armies[1]->leader->name) +
                                  " is assassinated in " +
                                  region->ShortPrint( pRegs ) +
                                  "!");
        }
        AddLine(*(armies[1]->leader->name) + " is routed!");
        if (armies[1]->NumAlive())
            FreeRound(armies[0],armies[1]);
        AddLine("Total Casualties:");
        ItemList *spoils = new ItemList;
        armies[1]->Lose(this,spoils);
        GetSpoils(defs,spoils);
        AString temp;
        if (spoils->Num()) {
            temp = AString("Spoils: ") + spoils->Report(2,0,1,1) + ".";
        } else {
            temp = "No spoils.";
        }
        armies[0]->Win(this,spoils);
        AddLine("");
        AddLine(temp);
        AddLine("");
        delete spoils;
        delete armies[0];
        delete armies[1];
        return BATTLE_WON;
    }
  
    AddLine("The battle ends indecisively.");
    AddLine("");
    AddLine("Total Casualties:");
    armies[0]->Tie(this);
    armies[1]->Tie(this);
    AddLine("");
    delete armies[0];
    delete armies[1];
    return BATTLE_DRAW;
}

void Battle::WriteSides(ARegion * r,
                        Unit * att,
                        Unit * tar,
                        AList * atts,
                        AList * defs,
                        int ass,
                        ARegionList *pRegs )
{
  if (ass) {
    AddLine(*att->name + " attempts to assassinate " + *tar->name
	    + " in " + r->ShortPrint( pRegs ) + "!");
  } else {
    AddLine(*att->name + " attacks " + *tar->name + " in " +
	    r->ShortPrint( pRegs ) + "!");
  }
  AddLine("");
  AddLine("Attackers:");
  {	
    forlist(atts) {
      AString * temp = ((Location *) elem)->unit->BattleReport();
      AddLine(*temp);
      delete temp;
    }
  }
  AddLine("");
  AddLine("Defenders:");
  {	
    forlist(defs) {
      AString * temp = ((Location *) elem)->unit->BattleReport();
      AddLine(*temp);
      delete temp;
    }
  }
  AddLine("");
}

void Battle::Report(Areport * f,Faction * fac) {
  if (assassination == ASS_SUCC && fac != attacker) {
    f->PutStr(*asstext);
    f->PutStr("");
    return;
  }
  forlist(&text) {
    AString * s = (AString *) elem;
    f->PutStr(*s);
  }
}

void Battle::AddLine(const AString & s) {
  AString * temp = new AString(s);
  text.Add(temp);
}

void Game::GetDFacs(ARegion * r,Unit * t,AList & facs)
{
  forlist((&r->objects)) {
    Object * obj = (Object *) elem;
    forlist((&obj->units)) {
      Unit * u = (Unit *) elem;
      if (u->IsAlive()) {
        if (u->faction == t->faction ||
            (u->guard != GUARD_AVOID && u->GetAttitude(r,t) == A_ALLY && t->GetAttitude(r,u) == A_ALLY)) {
          if (!GetFaction2(&facs,u->faction->num)) {
            FactionPtr * p = new FactionPtr;
            p->ptr = u->faction;
            facs.Add(p);
          }
        }
      }
    }
  }
}   
void Game::GetAFacs(ARegion * r,Unit * att,Unit * tar,
		    AList & dfacs,AList & afacs,AList & atts) {
  forlist((&r->objects)) {
    Object * obj = (Object *) elem;
    forlist((&obj->units)) {
      Unit * u = (Unit *) elem;
      if (u->IsAlive() && u->canattack) {
	int add = 0;
	if ((u->faction == att->faction ||
	    u->GetAttitude(r,tar) == A_HOSTILE) &&
	    (u->guard != GUARD_AVOID || u == att)) {
	  add = 1;
	} else {
	  if (u->guard == GUARD_ADVANCE && u->GetAttitude(r,tar) != A_ALLY) {
	    add = 1;
	  } else {
	    if (u->attackorders) {
	      forlist(&(u->attackorders->targets)) {
		UnitId * id = (UnitId *) elem;
//		if (r->GetUnitId(id,u->faction->num) == tar) {
		if (r->GetUnitId(id,u->faction->num) && r->GetUnitId(id,u->faction->num)->faction->num == tar->faction->num) {
		  u->attackorders->targets.Remove(id);
		  delete id;
		  add = 1;
		  break;
		}
	      }
	    }
	  }
	}

	if (add) {
	  if (!GetFaction2(&dfacs,u->faction->num)) {
	    Location * l = new Location;
	    l->unit = u;
	    l->obj = obj;
	    l->region = r;
	    atts.Add(l);
            if (!GetFaction2(&afacs,u->faction->num)) {
              FactionPtr * p = new FactionPtr;
	      p->ptr = u->faction;
	      afacs.Add(p);
	    }
	  }
	}
      }
    }
  }
}

int Game::CanAttack(ARegion * r,AList * afacs,Unit * u) {
  int see = 0;
  int ride = 0;
  forlist(afacs) {
    FactionPtr * f = (FactionPtr *) elem;
    if (f->ptr->CanSee(r,u) == 2) {
      if (ride == 1) return 1;
      see = 1;
    }
    if (f->ptr->CanCatch(r,u)) {
      if (see == 1) return 1;
      ride = 1;
    }
  }
  return 0;
}

void Game::GetSides(ARegion * r,AList & afacs,AList & dfacs,
                    AList & atts,AList & defs,Unit * att,Unit * tar,int ass,
                    int adv)
{
    if (ass)
    {
        /* Assassination attempt */
        Location * l = new Location;
        l->unit = att;
        l->obj = r->GetDummy();
        l->region = r;
        atts.Add(l);
        
        l = new Location;
        l->unit = tar;
        l->obj = r->GetDummy();
        l->region = r;
        defs.Add(l);
		
        return;
    }
    
    int j=NDIRS;
    int noaida = 0, noaidd = 0;
    for (int i=-1;i<j;i++)
    {
        ARegion * r2 = r;
        if (i>=0)
        {
            r2 = r->neighbors[i];
            if (!r2) continue;
            forlist(&r2->objects) {
                /* Can't get building bonus in another region */
                ((Object *) elem)->capacity = 0;
            }
        } else {
            forlist(&r2->objects) {
                Object * o = (Object *) elem;
                /* Set building capacity */
                if (o->incomplete == 0 && o->IsBuilding()) {
                    o->capacity = ObjectDefs[o->type].protect;
                }
            }
        }
        forlist (&r2->objects) {
            Object * o = (Object *) elem;
            forlist (&o->units) {
                Unit * u = (Unit *) elem;
                int add = 0;
                
#define ADD_ATTACK 1
#define ADD_DEFENSE 2
                /* First, can the unit be involved in the battle at all? */
                if (u->IsAlive() && (i==-1 || u->GetFlag(FLAG_HOLDING) == 0)) {
                    
                    if (GetFaction2(&afacs,u->faction->num)) {
                        /* The unit is on the attacking side, check if the unit should
                           be in the battle */
                        if (i == -1 || (!noaida)) {
                            if (u->canattack && (u->guard != GUARD_AVOID || u==att) &&
                                u->CanMoveTo(r2,r) && !::GetUnit(&atts,u->num)) {
                                add = ADD_ATTACK;
                            }
                        }
                    } else {
                        /* The unit is not on the attacking side */
                        /* First, check for the noaid flag; if it is set, only units
                           from this region will join on the defensive side */
                        if (!(i != -1 && noaidd)) {
                            if (u->type == U_GUARD) {
                                /* The unit is a city guardsman */
                                if (i == -1 && adv == 0) 
                                    add = ADD_DEFENSE;
                            } else {
                                /* The unit is not a city guardsman, check if the unit is
                                   on the defensive side */
                                if (GetFaction2(&dfacs,u->faction->num)) {
                                    if (u->guard == GUARD_AVOID) {
                                        /* The unit is avoiding, and doesn't want to be in the
                                           battle if he can avoid it */
                                        if (u == tar ||
                                            (u->faction == tar->faction &&
                                             i==-1 && CanAttack(r,&afacs,u))) {
                                            add = ADD_DEFENSE;
                                        }
                                    } else {
                                        /* The unit is not avoiding, and wants to defend, if
                                           it can */
                                        if (u->CanMoveTo(r2,r)) {
                                            add = ADD_DEFENSE;
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
                
                if (add == ADD_ATTACK) {
                    Location * l = new Location;
                    l->unit = u;
                    l->obj = o;
                    l->region = r2;
                    atts.Add(l);
                } else {
                    if (add == ADD_DEFENSE) {
                        Location * l = new Location;
                        l->unit = u;
                        l->obj = o;
                        l->region = r2;
                        defs.Add(l);
                    }
                }
            }
        }
        
        //
        // If we are in the original region, check for the noaid status of
        // the units involved
        //
        if (i == -1) {
            noaida = 1;
            {
                forlist (&atts) {
                    Location *l = (Location *) elem;
                    if (!l->unit->GetFlag(FLAG_NOAID)) {
                        noaida = 0;
                        break;
                    }
                }
            }
            
            noaidd = 1;
            {
                forlist (&defs) {
                    Location *l = (Location *) elem;
                    if (!l->unit->GetFlag(FLAG_NOAID)) {
                        noaidd = 0;
                        break;
                    }
                }
            }
        }
    }
}

void Game::KillDead(Location * l)
{
    if (!l->unit->IsAlive())
    {
        l->region->Kill(l->unit);
    }
    else
    {
        if (l->unit->advancefrom)
        {
            l->unit->MoveUnit( l->unit->advancefrom->GetDummy() );
        }
    }
}

/*
 BATTLE_IMPOSSIBLE means that battle did not happen due to some reason
		(e.g. cannot attack ally)
 BATTLE_LOST means that attacker lost the battle
 BATTLE_WON means that attacker won the battle
 BATTLE_DRAW - draw
*/ 
int Game::RunBattle(ARegion * r,Unit * attacker,Unit * target,int ass,
                     int adv)
{
    AList afacs,dfacs;
    AList atts,defs;
    FactionPtr * p;
	int result;
    if (ass) {
        /* Assassination attempt */
        p = new FactionPtr;
        p->ptr = attacker->faction;
        afacs.Add(p);
        p = new FactionPtr;
        p->ptr = target->faction;
        dfacs.Add(p);
    } 
    else
    {
        //
        // xxxxx
        //
        if( r->IsSafeRegion() )
        {
            attacker->Error("ATTACK: No battles allowed in safe regions.");
            return BATTLE_IMPOSSIBLE;
        }
        GetDFacs(r,target,dfacs);
        if (GetFaction2(&dfacs,attacker->faction->num)) {
            attacker->Error("ATTACK: Can't attack an ally.");
            return BATTLE_IMPOSSIBLE;
        }
        GetAFacs(r,attacker,target,dfacs,afacs,atts);
    }
  
    GetSides(r,afacs,dfacs,atts,defs,attacker,target,ass,adv);
	if (atts.Num() == 0) {
		// this normally should not happen. 
		Awrite(AString("Cannot gather any attackers!"));
		return BATTLE_IMPOSSIBLE;
	}
	if (defs.Num() == 0) {
		// this normally should not happen
		Awrite(AString("Cannot gather any defenders!"));
		return BATTLE_IMPOSSIBLE;
	}  

    Battle * b = new Battle;
    b->WriteSides(r,attacker,target,&atts,&defs,ass, &regions );
  
    battles.Add(b);
    forlist(&factions) {
        Faction * f = (Faction *) elem;
        if (GetFaction2(&afacs,f->num) || GetFaction2(&dfacs,f->num) ||
            r->Present(f)) {
            BattlePtr * p = new BattlePtr;
            p->ptr = b;
            f->battles.Add(p);
        }
    }
  
    result = b->Run(r,attacker,&atts,target,&defs,ass, &regions );
  
  /* Remove all dead units */
    {
        forlist(&atts) {
            KillDead((Location *) elem);
        }
    }
    {
        forlist(&defs) {
            KillDead((Location *) elem);
        }
    }
	return result;
}
	
	
	

