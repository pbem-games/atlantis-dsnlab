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
#include "rules.h"
#include "i_std.h"

void Game::RunOrders()
{
    //
    // Form and instant orders are handled during parsing
    //
    Awrite("Running FIND Orders...");
    RunFindOrders();
    Awrite("Running ENTER/LEAVE Orders...");
    RunEnterOrders();
    Awrite("Running PROMOTE Orders...");
    RunPromoteOrders();
    Awrite("Running Combat...");
    DoAttackOrders();
    DoAutoAttacks();
    Awrite("Running STEAL/ASSASSINATE Orders...");
    RunStealOrders();
    Awrite("Running GIVE/PAY/TRANSFER Orders...");
    DoGiveOrders();
    Awrite("Running DESTROY Orders...");
    RunDestroyOrders();
    Awrite("Running PILLAGE Orders...");
    RunPillageOrders();
    Awrite("Running TAX Orders...");
    RunTaxOrders();
    Awrite("Running GUARD 1 Orders...");
    DoGuard1Orders();
    Awrite("Running Magic Orders...");
    ClearCastEffects();
    RunCastOrders();
    Awrite("Running SELL Orders...");
    RunSellOrders();
    Awrite("Running BUY Orders...");
    RunBuyOrders();
    Awrite("Running TRADE Orders...");
    RunTradeOrders();
    Awrite("Running FORGET Orders...");
    RunForgetOrders();
    Awrite("Running QUIT Orders...");
    RunQuitOrders();
    Awrite("Removing Empty Units...");
    DeleteEmptyUnits();
    SinkShips();
    Awrite("Running Sail Orders...");
    RunSailOrders();
    Awrite("Running Move Orders...");
    RunMoveOrders();
    FindDeadFactions();
    Awrite("Running Teach Orders...");
    RunTeachOrders();
    Awrite("Running Month-long Orders...");
    RunMonthOrders();
    RunTeleportOrders();
    Awrite("Assessing Maintenance costs...");
    AssessMaintenance();
    Awrite("Post-Turn Processing...");
    PostProcessTurn();
    DeleteEmptyUnits();
    EmptyHell();
    RemoveEmptyObjects();
}

void Game::ClearCastEffects()
{
    forlist(&regions) {
        ARegion * r = (ARegion *) elem;
        forlist(&r->objects) {
            Object * o = (Object *) elem;
            forlist(&o->units) {
                Unit * u = (Unit *) elem;
                u->SetFlag(FLAG_INVIS,0);
            }
        }
    }
}
    
void Game::RunCastOrders()
{
    forlist(&regions) {
        ARegion * r = (ARegion *) elem;
        forlist(&r->objects) {
            Object * o = (Object *) elem;
            forlist(&o->units) {
                Unit * u = (Unit *) elem;
                if (u->castorders)
                {
                    RunACastOrder(r,o,u);
                    delete u->castorders;
                    u->castorders = 0;
                }
            }
        }
  }
}

int Game::CountMages( Faction *pFac )
{
    int i = 0;
    forlist( &regions ) {
        ARegion * r = (ARegion *) elem;
        forlist( &r->objects ) {
            Object * o = (Object *) elem;
            forlist(&o->units) {
                Unit * u = (Unit *) elem;
                if (u->faction == pFac && u->type == U_MAGE) i++;
            }
        }
    }
    return( i );
}

int Game::TaxCheck( ARegion *pReg, Faction *pFac )
{
    if( Globals->FACTION_LIMIT_TYPE == GameDefs::FACLIM_FACTION_TYPES )
    {
        if( AllowedTaxes( pFac ) == -1 )
        {
            //
            // No limit.
            //
            return( 1 );
        }

        forlist( &( pFac->war_regions )) {
            ARegion *x = ((ARegionPtr *) elem)->ptr;
            if( x == pReg )
            {
                //
                // This faction already performed a tax action in this
                // region.
                //
                return 1;
            }
        }
        if( pFac->war_regions.Num() >= AllowedTaxes( pFac ))
        {
            //
            // Can't tax here.
            //
            return 0;
        } 
        else
        {
            //
            // Add this region to the faction's tax list.
            //
            ARegionPtr *y = new ARegionPtr;
            y->ptr = pReg;
            pFac->war_regions.Add(y);
            return 1;
        }
    }
    else
    {
        //
        // No limit on taxing regions in this game.
        //
        return( 1 );
    }
}
// dsnlab
int Game::TradeCheckFishing (ARegion *pReg, Faction *pFac)
{
  if( Globals->FACTION_LIMIT_TYPE == GameDefs::FACLIM_FACTION_TYPES )
  {
    if( AllowedTrades (pFac) == -1 )
    {
      //
      // No limit on trading on this faction.
      //
      return 1;
    }
    forlist (&(pFac->fishing_regions))
    {
      ARegion * x = ((ARegionPtr *) elem)->ptr;
      if (x == pReg)
      {
        //
        // This faction has already performed a trade action in this
        // region.
        //
        return 1;
      }
    }
    if (pFac->fishing_regions.Num() >= (AllowedTrades(pFac)))
    {
      //
      // This faction is over its trade limit.
      //
      return 0;
    } else
    {
      //
      // Add this region to the faction's trade list, and return 1.
      //
      ARegionPtr * y = new ARegionPtr;
      y->ptr = pReg;
      pFac->fishing_regions.Add(y);
      return 1;
    }
  }else
  {
    return 1;
  }
}

int Game::TradeCheckRoadbuild (ARegion *pReg, Faction *pFac)
{
  if( Globals->FACTION_LIMIT_TYPE == GameDefs::FACLIM_FACTION_TYPES )
  {
    if( AllowedTrades (pFac) == -1 )
    {
      //
      // No limit on trading on this faction.
      //
      return 1;
    }
    forlist (&(pFac->roadbuild_regions))
    {
      ARegion * x = ((ARegionPtr *) elem)->ptr;
      if (x == pReg)
      {
        //
        // This faction has already performed a trade action in this
        // region.
        //
        return 1;
      }
    }
    if (pFac->roadbuild_regions.Num() >= (AllowedTrades(pFac)))
    {
      //
      // This faction is over its trade limit.
      //
      return 0;
    } else
    {
      //
      // Add this region to the faction's trade list, and return 1.
      //
      ARegionPtr * y = new ARegionPtr;
      y->ptr = pReg;
      pFac->roadbuild_regions.Add(y);
      return 1;
    }
  }else
  {
    return 1;
  }
}

int Game::TradeCheck( ARegion *pReg, Faction *pFac )
{
    if( Globals->FACTION_LIMIT_TYPE == GameDefs::FACLIM_FACTION_TYPES )
    {
        if( AllowedTrades( pFac ) == -1 )
        {
            //
            // No limit on trading on this faction.
            //
            return( 1 );
        }

        forlist( &( pFac->trade_regions )) {
            ARegion * x = ((ARegionPtr *) elem)->ptr;
            if (x == pReg )
            {
                //
                // This faction has already performed a trade action in this
                // region.
                //
                return 1;
            }
        }
        if ( pFac->trade_regions.Num() >= AllowedTrades( pFac ))
        {
            //
            // This faction is over its trade limit.
            //
            return 0;
        } 
        else
        {
            //
            // Add this region to the faction's trade list, and return 1.
            //
            ARegionPtr * y = new ARegionPtr;
            y->ptr = pReg;
            pFac->trade_regions.Add(y);
            return 1;
        }
    }
    else
    {
        //
        // No limit on trade in this game.
        //
        return( 1 );
    }
}

void Game::RunStealOrders() {
  forlist(&regions) {
    ARegion * r = (ARegion *) elem;
    forlist(&r->objects) {
      Object * o = (Object *) elem;
      forlist(&o->units) {
	Unit * u = (Unit *) elem;
	if (u->stealorders) {
	  if (u->stealorders->type == O_STEAL) {
	    Do1Steal(r,o,u);
	  } else
	    if (u->stealorders->type == O_ASSASSINATE) {
	      Do1Assassinate(r,o,u);
	    }
	  delete u->stealorders;
	  u->stealorders = 0;
	}
      }
    }
  }
}

AList * Game::CanSeeSteal(ARegion * r,Unit * u) {
  AList * retval = new AList;
  forlist(&factions) {
    Faction * f = (Faction *) elem;
    if (r->Present(f)) {
      if (f->CanSee(r,u)) {
	FactionPtr * p = new FactionPtr;
	p->ptr = f;
	retval->Add(p);
      }
    }
  }
  return retval;
}

void Game::Do1Assassinate(ARegion * r,Object * o,Unit * u)
{
    AssassinateOrder * so = (AssassinateOrder *) u->stealorders;
    Unit * tar = r->GetUnitId(so->target,u->faction->num);

    if (!tar || !tar->IsAlive()) {
        u->Error("ASSASSINATE: Invalid unit given.");
        return;
    }
  
    if (tar->type == U_GUARD || tar->type == U_WMON) {
        u->Error("ASSASSINATE: Can only assassinate other player's "
                 "units.");
        return;
    }
  
    if (u->GetMen() != 1) {
        u->Error("ASSASSINATE: Must be executed by a 1-man unit.");
        return;
    }
/*  
    if (!u->CanSee(r,tar))
    {
        u->Error("ASSASSINATE: Non-existent unit."); 
        return;
    }
*/
    AList * seers = CanSeeSteal(r,u);
    int succ = 1;
    forlist(seers) {
        Faction * f = ((FactionPtr *) elem)->ptr;
        if (f == tar->faction) {
            succ = 0;
            break;
        }
        if (f->GetAttitude(tar->faction->num) == A_ALLY) {
            succ = 0;
            break;
        }
        if (f->num == guardfaction) {
            succ = 0;
            break;
        }
    }
  
    if (!succ) {
        AString temp = *(u->name) + " is caught attempting to assassinate " +
            *(tar->name) + " in " + *(r->name) + ".";
        forlist(seers) {
            Faction * f = ((FactionPtr *) elem)->ptr;
            f->Event(temp);
        }
        return;
    }
// LLS
//#ifdef xxxxx
    //
    // New rule; if a target has an amulet of true seeing they can't be
    // assassinated by someone with a ring of invisibility
    //
    if (u->items.GetNum( I_RINGOFI ) &&
        tar->items.GetNum( I_AMULETOFTS ) >= tar->GetMen()) {
        tar->Event( "Assassination prevented by amulet of true seeing." );
        u->Event( AString( "Attempts to assassinate " ) + *(tar->name) + ", but "
                  "is prevented by amulet of true seeing." );
        return;
    }
//#endif

    RunBattle(r,u,tar,1);
}

void Game::Do1Steal(ARegion * r,Object * o,Unit * u)
{
    StealOrder * so = (StealOrder *) u->stealorders;
    Unit * tar = r->GetUnitId(so->target,u->faction->num);
  
    if (!tar) {
        u->Error("STEAL: Invalid unit given.");
        return;
    }
  
    if (tar->type == U_GUARD || tar->type == U_WMON) {
        u->Error("STEAL: Can only steal from other player's "
                 "units.");
        return;
    }
  
    if (u->GetMen() != 1) {
        u->Error("STEAL: Must be executed by a 1-man unit.");
        return;
    }
  
    AList * seers = CanSeeSteal(r,u);
    int succ = 1;
    {
        forlist(seers) {
            Faction * f = ((FactionPtr *) elem)->ptr;
            if (f == tar->faction) {
                succ = 0;
                break;
            }
            if (f->GetAttitude(tar->faction->num) == A_ALLY) {
                succ = 0;
                break;
            }
            if (f->num == guardfaction) {
                succ = 0;
                break;
            }
        }
    }
  
    if (!succ) {
        AString temp = *(u->name) + " is caught attempting to steal from " +
            *(tar->name) + " in " + *(r->name) + ".";
        forlist(seers) {
            Faction * f = ((FactionPtr *) elem)->ptr;
            f->Event(temp);
        }
        return;
    }

//#ifdef xxxxx
    //
    // New rule; if a target has an amulet of true seeing they can't be
    // stolen from by someone with a ring of invisibility
    //
    if (u->items.GetNum( I_RINGOFI ) &&
        tar->items.GetNum( I_AMULETOFTS ) >= tar->GetMen()) {
        tar->Event( "Theft prevented by amulet of true seeing." );
        u->Event( AString( "Attempts to steal from " ) + *(tar->name) + ", but "
                  "is prevented by amulet of true seeing." );
        return;
    }
//#endif

    int amt = 1;
    if (so->item == I_SILVER) {
        amt = tar->GetMoney();
        if (amt < 400) {
            amt = amt / 2;
        } else {
            amt = 200;
        }
    }
  
    if (tar->items.GetNum(so->item) < amt) {
        amt = 0;
    }

    int canget = 1;
    if (so->item == I_TRIFLE)
    {
      if (u->items.GetNum(so->item) != 0)
      {
        u->Error (AString("STEAL: Target unit already have trifle."));
        canget = 0;
      }
    }

    AString * gname = new AString (*tar->items.GetName (so->item));
    if (canget == 1)
    {
      u->items.SetNum(so->item,u->items.GetNum(so->item) + amt);
      u->items.SetName(so->item, gname);
    }
    tar->items.SetNum(so->item,tar->items.GetNum(so->item) - amt);
  
    {
        AString temp = *(u->name) + " steals " + 
            ItemString(so->item,amt) + " from " + *(tar->name) + ".";
        forlist(seers) {
            Faction * f = ((FactionPtr *) elem)->ptr;
            f->Event(temp);
        }
    }
  
    tar->Event(AString("Has ") + ItemString(so->item,amt) + " stolen.");
    return;
}
	
void Game::SinkShips()
{
    forlist(&regions) {
        ARegion * r = (ARegion *) elem;
        if (r->type == R_OCEAN)
        {
            forlist(&r->objects) {
                Object * o = (Object *) elem;
                if (o->IsBoat() && o->units.Num() == 0)
                {
                    r->objects.Remove(o);
                    delete o;
                }
                if (o->type == O_DUMMY)
                {
                    forlist(&o->units) {
                        Unit * u = (Unit *) elem;
                        if (!u->CanSwim())
                        {
                            r->Kill(u);
                            u->Event("Drowns in the ocean.");
                        }
                    }
                }
            }
        }
    }
}

void Game::RunForgetOrders()
{
    forlist(&regions) {
        ARegion *r = (ARegion *) elem;
        forlist(&r->objects) {
            Object *o = (Object *) elem;
            forlist(&o->units) {
                Unit *u = (Unit *) elem;
// dsnlab
                if (u->type != U_MAGE)
                {
                  forlist(&u->forgetorders) {
                      ForgetOrder *fo = (ForgetOrder *) elem;
                      u->ForgetSkill(fo->skill);
                      u->Event(AString("Forgets ") + SkillStrs(fo->skill) + ".");
                  }
		}
                u->forgetorders.DeleteAll();
            }
        }
    }
}

void Game::RunQuitOrders()
{
    forlist(&factions) {
        Faction * f = (Faction *) elem;
        if (f->quit)
        {
            Do1Quit(f);
            AString x( "NoAddress" );
            f->SetAddress (x);
            f->quit = 0;
        }
    }
}

void Game::Do1Quit(Faction * f)
{
    forlist(&regions) {
        ARegion * r = (ARegion *) elem;
        forlist(&r->objects) {
            Object * o = (Object *) elem;
            forlist(&o->units) {
                Unit * u = (Unit *) elem;
                if (u->faction == f)
                {
		  if ((r->type == R_NEXUS) || (r->IsStartingCity()) || (f->quit == QUIT_REAL))
		  {
		    ClearKilledFars(u);
		    o->units.Remove(u);
		    delete u;
		  }else
		  {
		    u->SetFlag(FLAG_HOLDING,1);
		    AString *x = new AString( "I am already quit" );
		    u->SetDescribe(x);
		  }
                }
            }
        }
    }
}

void Game::RunDestroyOrders()
{
    forlist(&regions) {
        ARegion * r = (ARegion *) elem;
        forlist(&r->objects) {
            Object * o = (Object *) elem;
            Unit * u = o->GetOwner();
            if (u)
            {
                if (u->destroy)
                {
                    Do1Destroy(r,o,u);
                    continue;
                } 
                else
                {
                    forlist(&o->units) {
                        ((Unit *) elem)->destroy = 0;
                    }
                }
            }
        }
    }
}

void Game::Do1Destroy(ARegion * r,Object * o,Unit * u) {
  if (r->type == R_OCEAN) {
    u->Error("DESTROY: Can't destroy a ship while at sea.");
    forlist(&o->units) {
      ((Unit *) elem)->destroy = 0;
    }
    return;
  }

  if (!u->GetMen()) {
      u->Error("DESTROY: Empty units cannot destroy structures.");
      forlist(&o->units) {
          ((Unit *) elem)->destroy = 0;
      }
      return;
  }
// dsnlab
//  if (ObjectDefs[o->type].protect > u->GetMen ())
//  {
//      u->Error("DESTROY: Too low men in unit to destroy structure.");
//      forlist(&o->units) {
//          ((Unit *) elem)->destroy = 0;
//      }
//      return;
//  }
  if (o->CanDestroy()) {
    u->Event(AString("Destroys ") + *(o->name) + ".");
    Object * dest = r->GetDummy();
    forlist(&o->units) {
      Unit * u = (Unit *) elem;
      u->destroy = 0;
      u->MoveUnit( dest );
    }
    r->objects.Remove(o);
    delete o;
  } else {
    u->Error("DESTROY: Can't destroy that.");
    forlist(&o->units) {
      ((Unit *) elem)->destroy = 0;
    }
  }
}
		
void Game::RunFindOrders() {
  forlist(&regions) {
    ARegion * r = (ARegion *) elem;
    forlist(&r->objects) {
      Object * o = (Object *) elem;
      forlist(&o->units) {
	Unit * u = (Unit *) elem;
	RunFindUnit(u);
      }
    }
  }
}

void Game::RunFindUnit(Unit * u) {
  forlist(&u->findorders) {
    u->faction->Event (AString ("FIND: disabled"));
/*
    FindOrder * f = (FindOrder *) elem;
    Faction * fac = GetFaction(&factions,f->find);
    if (fac) {
      u->faction->Event(AString("The address of ") + *(fac->name) +
			" is " + *(fac->address) + ".");
    } else {
      u->Error(AString("FIND: ") + f->find + " is not a valid "
	       "faction number.");
    }
*/
  }
  u->findorders.DeleteAll();
}

void Game::RunTaxOrders() {
  forlist(&regions) {
    RunTaxRegion((ARegion *) elem);
  }
}

int Game::CountTaxers(ARegion * reg)
{
    int t = 0;
    forlist(&reg->objects) {
        Object * o = (Object *) elem;
        forlist(&o->units) {
            Unit * u = (Unit *) elem;
            if (u->GetFlag(FLAG_AUTOTAX)) u->taxing = TAX_TAX;
            if (u->taxing == TAX_TAX) {
                if (!reg->CanTax(u)) {
                    u->Error("TAX: A unit is on guard.");
                    u->taxing = TAX_NONE;
                } else {
                    int men = u->Taxers();
                    if (men) {
                        if( !TaxCheck(reg, u->faction ))
                        {
                            u->Error( "TAX: Faction can't tax that many "
                                      "regions.");
                            u->taxing = TAX_NONE;
                        } else {
                            t += men;
                        }
                    } else {
                        u->Error("TAX: Unit cannot tax.");
                        u->taxing = TAX_NONE;
                        u->SetFlag(FLAG_AUTOTAX,0);
                    }
                }
            }
        }
    }
    return t;
}

void Game::RunTaxRegion(ARegion * reg)
{
    int taxers = CountTaxers(reg);
    int desired = taxers * Globals->TAX_INCOME;
    if (desired < reg->money) desired = reg->money;
    
    forlist(&reg->objects) {
        Object * o = (Object *) elem;
        forlist(&o->units) {
            Unit * u = (Unit *) elem;
            if (u->taxing == TAX_TAX)
            {
                int t = u->Taxers();
                double fAmt = ((double) t) * ((double) Globals->TAX_INCOME ) *
                    ((double) reg->money ) / ((double) desired );
                int amt = (int) fAmt;
                reg->money -= amt;
                desired -= t * Globals->TAX_INCOME;
                u->SetMoney(u->GetMoney() + amt);
                u->Event(AString("Collects $") + amt + " in taxes in " + 
                         reg->ShortPrint( &regions ) + ".");
                u->taxing = TAX_NONE;
            }
        }
    }
}

void Game::RunPillageOrders() {
  forlist (&regions) {
    RunPillageRegion((ARegion *) elem);
  }
}

int Game::CountPillagers(ARegion * reg)
{
    int p = 0;
    forlist(&reg->objects) {
        Object * o = (Object *) elem;
        forlist(&o->units) {
            Unit * u = (Unit *) elem;
            if (u->taxing == TAX_PILLAGE) {
                if (!reg->CanTax(u)) {
                    u->Error("PILLAGE: A unit is on guard.");
                    u->taxing = TAX_NONE;
                } else {
                    int men = u->Taxers();
                    if (men) {
                        if( !TaxCheck( reg, u->faction ))
                        {
                            u->Error( "PILLAGE: Faction can't tax that many "
                                      "regions." );
                            u->taxing = TAX_NONE;
                        } else {
                            p += men;
                        }
                    } else {
                        u->Error("PILLAGE: Not a combat ready unit.");
                        u->taxing = TAX_NONE;
                    }
                }
            }
        }
    }
    return p;
}

void Game::ClearPillagers(ARegion * reg) {
  forlist(&reg->objects) {
    Object * o = (Object *) elem;
    forlist(&o->units) {
      Unit * u = (Unit *) elem;
      if (u->taxing == TAX_PILLAGE) {
	u->Error("PILLAGE: Not enough men to pillage.");
	u->taxing = TAX_NONE;
      }
    }
  }
}

void Game::RunPillageRegion(ARegion * reg) {
  if (reg->type == R_OCEAN) return;
  if (reg->money < 1) return;
  if (reg->Wages() < 11) return;
  
  /* First, count up pillagers */
  int pillagers = CountPillagers(reg);
  
  if (pillagers * 2 < reg->money / Globals->TAX_INCOME) {
    ClearPillagers(reg);
    return;
  }
  
  AList * facs = reg->PresentFactions();
  int amt = reg->money * 2;
  forlist(&reg->objects) {
    Object * o = (Object *) elem;
    forlist(&o->units) {
      Unit * u = (Unit *) elem;
      if (u->taxing == TAX_PILLAGE) {
	u->taxing = TAX_NONE;
	int num = u->Taxers();
	int temp = (amt * num)/pillagers;
	amt -= temp;
	pillagers -= num;
	u->SetMoney(u->GetMoney() + temp);
	u->Event(AString("Pillages $") + temp + " from " + 
             reg->ShortPrint( &regions ) + ".");
	forlist(facs) {
	  Faction * fp = ((FactionPtr *) elem)->ptr;
	  if (fp != u->faction) {
	    fp->Event(*(u->name) + " pillages " + *(reg->name) + ".");
	  }
	}
      }
    }
  }
  delete facs;
  
  /* Destroy economy */
  reg->money = 0;
  reg->wages -= 6;
  if (reg->wages < 6) reg->wages = 6;
}
				
void Game::RunPromoteOrders()
{
    /* First, do any promote orders */
    {
        forlist(&regions) {
            ARegion * r = (ARegion *) elem;
            forlist(&r->objects) {
                Object * o = (Object *) elem;
                if (o->type != O_DUMMY)
                {
                    Unit *u = o->GetOwner();
                    if( u && u->promote )
                    {
                        Do1PromoteOrder(o,u);
                        delete u->promote;
                        u->promote = 0;
                    }
                }
            }
        }
    }
    
    /* Then, clear out other promote orders */
    {
        forlist(&regions) {
            ARegion * r = (ARegion *) elem;
            forlist(&r->objects) {
                Object * o = (Object *) elem;
                forlist(&o->units) {
                    Unit * u = (Unit *) elem;
                    if (u->promote)
                    {
                        delete u->promote;
                        u->promote = 0;
                    }
                }
            }
        }
    }	
}

void Game::Do1PromoteOrder(Object * obj,Unit * u) {
  Unit * tar = obj->GetUnitId(u->promote,u->faction->num);
  if (!tar) {
    u->Error("PROMOTE: Can't find target.");
    return;
  }
  obj->units.Remove(tar);
  obj->units.Insert(tar);
}

void Game::RunEnterOrders() {
  forlist(&regions) {
    ARegion * r = (ARegion *) elem;
    forlist(&r->objects) {
      Object * o = (Object *) elem;
      forlist(&o->units) {
	Unit * u = (Unit *) elem;
	if (u->enter)
	  Do1EnterOrder(r,o,u);
      }
    }
  }
}

void Game::Do1EnterOrder(ARegion * r,Object * in,Unit * u) {
  Object * to;
  if (u->enter == -1) {
    to = r->GetDummy();
    u->enter = 0;
    if (r->type == R_OCEAN && !u->CanSwim()) {
      u->Error("LEAVE: Can't leave a ship in the ocean.");
      return;
    }
  } else {
    to = r->GetObject(u->enter);
    u->enter = 0;
    if (!to) {
      u->Error("ENTER: Can't enter that.");
      return;
    }
    if (!to->CanEnter(r,u)) {
      u->Error("ENTER: Can't enter that.");
      return;
    }
    if (to->ForbiddenBy(r, u)) {
      u->Error("ENTER: Is refused entry.");
      return;
    }
  }
  u->MoveUnit( to );
}

void Game::RemoveEmptyObjects()
{
    forlist(&regions) {
        ARegion * r = (ARegion *) elem;
        forlist(&r->objects) {
            Object * o = (Object *) elem;
            if (o->incomplete == ObjectDefs[o->type].cost) {
                forlist(&o->units) {
                    Unit * u = (Unit *) elem;
                    u->MoveUnit( r->GetDummy() );
                }
                r->objects.Remove(o);
                delete o;
            }
        }
    }
}

void Game::EmptyHell()
{
{
  forlist(&regions) {
    ARegion * r = (ARegion *) elem;
    forlist(&r->hell) {
      Unit * u = (Unit *) elem;
      ClearKilledFars(u);
    }
  }
}
    forlist(&regions) {
        ((ARegion *) elem)->ClearHell();
    }
}

void Game::PostProcessUnit(ARegion *r,Unit *u)
{
    PostProcessUnitExtra( r, u );
}

void Game::EndGame( Faction *pVictor )
{
    forlist( &factions ) {
        Faction *pFac = (Faction *) elem;
        pFac->exists = 0;
        if( pFac == pVictor )
        {
            pFac->quit = QUIT_WON_GAME;
        }
        else
        {
            pFac->quit = QUIT_GAME_OVER;
        }

        if( pVictor )
        {
            pFac->Event( *( pVictor->name ) + " has won the game!" );
        }
        else
        {
            pFac->Event( "The game has ended with no winner." );
        }
    }

    gameStatus = GAME_STATUS_FINISHED;
}

void Game::PostProcessTurn()
{
    forlist(&regions) {
        ARegion * r = (ARegion *) elem;
        r->PostTurn();

        if( Globals->CITY_MONSTERS_EXIST && ( r->town || r->type == R_NEXUS ))
        {
            AdjustCityMons( r );
        }

        forlist (&r->objects) {
            Object *o = (Object *) elem;
            forlist (&o->units) {
                Unit *u = (Unit *) elem;
                PostProcessUnit(r,u);
            }
        }
    }

    if( Globals->WANDERING_MONSTERS_EXIST )
    {
        GrowWMons(Globals->WMON_FREQUENCY);
    }
    
    if( Globals->LAIR_MONSTERS_EXIST )
    {
        GrowLMons(Globals->LAIR_FREQUENCY);
    }

    //
    // Check if there are any factions left.
    //
    int livingFacs = 0;
    {
        forlist( &factions ) {
            Faction *pFac = (Faction *) elem;
// dsnlab
            if (!pFac->IsNPC ())
            {
              if (*pFac->address == "NoAddress")
              {
                if (!pFac->name->CheckPrefix ("[QUIT]"))
                {
                  AString newname = AString("[QUIT] ") + *pFac->name;
                  pFac->SetNameNoChange (&newname);
                }
                pFac->password = new AString ("12313213213");
                pFac->defaultattitude = A_NEUTRAL;
                pFac->attitudes.DeleteAll();
              }
            }
            if( pFac->exists )
            {
                livingFacs = 1;
//                break;
            }
        }
    }

    if( !livingFacs )
    {
        EndGame( 0 );
    }
    else if( !( Globals->OPEN_ENDED ))
    {
        Faction *pVictor = CheckVictory();
        if( pVictor )
        {
            EndGame( pVictor );
        }
    }
}

void Game::DoAutoAttacks()
{
    forlist(&regions) {
        ARegion * r = (ARegion *) elem;
        DoAutoAttacksRegion(r);
    }
}

void Game::DoAutoAttacksRegion(ARegion * r)
{
    forlist(&r->objects) {
        Object * o = (Object *) elem;
        forlist(&o->units) {
            Unit * u = (Unit *) elem;
            if (u->IsAlive() && u->canattack)
            {
                DoAutoAttack(r,u);
            }
        }
    }
}

void Game::DoAdvanceAttacks(AList * locs) {
  forlist(locs) {
    Location * l = (Location *) elem;
    Unit * u = l->unit;
    ARegion * r = l->region;
    if (u->IsAlive() && u->canattack) {
      DoAutoAttack(r,u);
    }
    if (u->IsAlive() && u->canattack && u->guard == GUARD_ADVANCE) {
      DoAdvanceAttack(r,u);
      u->guard = GUARD_NONE;
    }
    if (u->IsAlive()) {
      DoAutoAttackOn(r,u);
    }
    if(u->guard == GUARD_ADVANCE) u->guard = GUARD_NONE;
  }
}

void Game::DoAutoAttackOn(ARegion * r,Unit * t) {
  forlist(&r->objects) {
    Object * o = (Object *) elem;
    forlist(&o->units) {
      Unit * u = (Unit *) elem;
      if (u->guard != GUARD_AVOID && (u->GetAttitude(r,t) == A_HOSTILE) &&
	  u->IsAlive() && u->canattack) {
	AttemptAttack(r,u,t,1);
      }
      if (!t->IsAlive())
	return;
    }
  }
}

void Game::DoAdvanceAttack(ARegion * r,Unit * u) {
  Unit * t = r->Forbidden(u);
  while (t && u->IsAlive() && u->canattack) {
    AttemptAttack(r,u,t,1,1);
    t = r->Forbidden(u);
  }
}

void Game::DoAutoAttack(ARegion * r,Unit * u) {
  forlist(&r->objects) {
    Object * o = (Object *) elem;
    forlist(&o->units) {
      Unit * t = (Unit *) elem;
      if (u->guard != GUARD_AVOID && (u->GetAttitude(r,t) == A_HOSTILE)) {
	AttemptAttack(r,u,t,1);
      }
      if (u->IsAlive() == 0 || u->canattack == 0)
	return;
    }
  }
}

int Game::CountWMonTars(ARegion * r,Unit * mon) {
  int retval = 0;
  forlist(&r->objects) {
    Object * o = (Object *) elem;
    forlist(&o->units) {
      Unit * u = (Unit *) elem;
      if (u->type == U_NORMAL || u->type == U_MAGE) {
	if (mon->CanSee(r,u) && mon->CanCatch(r,u)) {
	  retval += u->GetMen();
	}
      }
    }
  }
  return retval;
}

Unit * Game::GetWMonTar(ARegion * r,int tarnum,Unit * mon) {
  forlist(&r->objects) {
    Object * o = (Object *) elem;
    forlist(&o->units) {
      Unit * u = (Unit *) elem;
      if (u->type == U_NORMAL || u->type == U_MAGE) {
	if (mon->CanSee(r,u) && mon->CanCatch(r,u)) {
	  int num = u->GetMen();
	  if (num && tarnum < num) return u;
	  tarnum -= num;
	}
      }
    }
  }
  return 0;
}

void Game::CheckWMonAttack(ARegion * r,Unit * u) {
  int tars = CountWMonTars(r,u);
  if (!tars) return;

  int rand = 300 - tars;
  if (rand < 100) rand = 100;
  if (getrandom(rand) >= u->Hostile()) return;

  Unit * t = GetWMonTar(r,getrandom(tars),u);
  if (t) AttemptAttack(r,u,t,1);
}

void Game::DoAttackOrders() {
  forlist(&regions) {
    ARegion * r = (ARegion *) elem;
    forlist(&r->objects) {
      Object * o = (Object *) elem;
      forlist(&o->units) {
	Unit * u = (Unit *) elem;
	if (u->type == U_WMON) {
	  if (u->canattack && u->IsAlive()) {
	    CheckWMonAttack(r,u);
	  }
	} else {
	  if (u->IsAlive() && u->attackorders) {
	    AttackOrder * ord = u->attackorders;
	    while (ord->targets.Num()) {
	      UnitId * id =
		(UnitId *) ord->targets.First();
	      ord->targets.Remove(id);
	      Unit * t = r->GetUnitId(id,u->faction->num);
	      delete id;
	      if (u->canattack && u->IsAlive()) {
		if (t) {
		  AttemptAttack(r,u,t,0);
		} else {
		  u->Error("ATTACK: Non-existent unit.");
		}
	      }
	    }
	    delete ord;
	    u->attackorders = 0;
	  }
	}
      }
    }
  }
}

/*
 * Presume that u is alive, can attack, and wants to attack t.
 * Check that t is alive, u can see t, and u has enough riding
 * skill to catch t.
 *
 * Return 0 if success.
 * 1 if t is already dead.
 * 2 if u can't see t
 * 3 if u lacks the riding to catch t
 */
void Game::AttemptAttack(ARegion * r,Unit * u,Unit * t,int silent,int adv)
{
    if (!t->IsAlive()) return;
    
    if (!u->CanSee(r,t))
    {
        if (!silent) { u->Error("ATTACK: Non-existent unit."); }
        return;
    }
    
    if (!u->CanCatch(r,t))
    {
        if (!silent) { u->Error("ATTACK: Can't catch that unit."); }
        return;
    }
    RunBattle(r,u,t,0,adv);
    return;
}

void Game::RunSellOrders() {
  forlist((&regions)) {
    ARegion * r = (ARegion *) elem;
    {
      forlist((&r->markets)) {
	Market * m = (Market *) elem;
	if (m->type == M_SELL)
	  DoSell(r,m);
      }
    }
    {
      forlist((&r->objects)) {
	Object * obj = (Object *) elem;
	forlist((&obj->units)) {
	  Unit * u = (Unit *) elem;
	  forlist((&u->sellorders)) {
	    SellOrder * o = (SellOrder *) elem;
	    u->Error("SELL: Can't sell that.");
	  }
	  u->sellorders.DeleteAll();
	}
      }
    }
  }
}

int Game::GetSellAmount(ARegion * r,Market * m) {
  int num = 0;
  forlist((&r->objects)) {
    Object * obj = (Object *) elem;
    forlist((&obj->units)) {
      Unit * u = (Unit *) elem;
      forlist ((&u->sellorders)) {
	SellOrder * o = (SellOrder *) elem;
	if (o->item == m->item) {
	  if (o->num > u->items.GetNum(o->item)) {
	    o->num = u->items.GetNum(o->item);
	    u->Error("SELL: Unit attempted to sell more than it had.");
	  }
	  if (o->num < 0) o->num = 0;
	  num += o->num;
	}
      }
    }
  }
  return num;
}

void Game::DoSell(ARegion * r,Market * m) {
  /* First, find the number of items being sold */
  int attempted = GetSellAmount(r,m);
  
  if (attempted < m->amount) attempted = m->amount;
  m->activity = 0;
  int oldamount = m->amount;
  forlist((&r->objects)) {
    Object * obj = (Object *) elem;
    forlist((&obj->units)) {
      Unit * u = (Unit *) elem;
      forlist((&u->sellorders)) {
	SellOrder * o = (SellOrder *) elem;
	if (o->item == m->item) {
	  int temp = 0;
	  if (attempted) {
	    temp = (m->amount * o->num + getrandom(attempted))
	      / attempted;
	    if (temp<0) temp = 0;
	  }
	  attempted -= o->num;
	  m->amount -= temp;
	  m->activity += temp;
	  u->items.SetNum(o->item,u->items.GetNum(o->item) - temp);
	  u->SetMoney(u->GetMoney() + temp * m->price);
	  u->sellorders.Remove(o);
	  u->Event(AString("Sells ") + ItemString(o->item,temp)
		   + " at $" + m->price + " each.");
	  delete o;
	  break;
	}
      }
    }
  }
  m->amount = oldamount;
}

// dsnlab

void Game::RunTradeOrders ()
{
  forlist((&regions))
  {
    ARegion * r = (ARegion *) elem;
    DoTrade (r);
  }
}

void Game::DoTrade (ARegion *r)
{
  forlist((&r->objects))
  {
    Object * obj = (Object *) elem;
    forlist((&obj->units))
    {
      Unit * u = (Unit *) elem;
      AString desc ("I");
      int setdesc = 0;
      forlist(&u->tradeorders)
      {
        TradeOrder * o = (TradeOrder *) elem;
        if (ItemDefs[o->item].type & IT_MAN)
        {
          u->Error ("TRADE: Can't trade living creatures.");
          u->tradeorders.Remove (o);
          delete o;
          continue;
        }
        if (ItemDefs[o->item].flags & ItemType::CANTGIVE)
        {
          u->Error (AString ("TRADE: Can't trade ") + ItemDefs[o->item].names + ".");
          u->tradeorders.Remove (o);
          delete o;
          continue;
        }
        if ((u->items.GetNum (o->item) == 0) && (o->doing == TRADE_SELL))
        {
          u->Error ("TRADE: Unit attempted to sell more than it had.");
          u->tradeorders.Remove (o);
          delete o;
          continue;
        }
        if (o->num > u->items.GetNum (o->item) && o->doing == TRADE_SELL)
        {
          o->num = u->items.GetNum (o->item);
          u->Error ("TRADE: Unit attempted to trade more than it had.");
        }
        if ((o->num * o->price > u->GetMoney ()) && (o->doing == TRADE_BUY))
        {
          o->num = u->GetMoney () / o->price;
          u->Error ("BUY: Unit attempted to buy more than it could afford.");
          if (o->num == 0)
          {
            u->tradeorders.Remove (o);
            delete o;
            continue;
          }
        }
        if (setdesc) desc += AString (",");
        switch (o->doing)
        {
          case TRADE_SELL:
            desc += AString (" sell");
            break;
          case TRADE_BUY:
            desc += AString (" buy");
            break;
        }
        o->unit_num = u->num;
        desc += AString (" ") + AString (o->num) + AString (" ") + AString (ItemDefs[o->item].names) + AString (" by ") + AString (o->price) + AString (" silv each");
        setdesc = 1;
      }
      if (setdesc == 1)
      {
        AString *tmp = new AString (desc);
        u->SetDescribe (tmp);
      }
    }
  }
  for (int i = 0; i < NITEMS; i++)
  {
    RealTrade (r, i);
  }
}

TradeOrder * Game::GetOrder (Unit * u, TradeOrder * c)
{
  forlist (&u->tradeorders)
  {
    TradeOrder * o = (TradeOrder *) elem;
    if ((o->num == c->num) && (o->item == c->item) && (o->price == c->price))
      return o;
  }
  return 0;
}
void Game::RealTrade (ARegion * r, int item)
{
  int fl = 1;
  int minprice;
  int maxprice;
  AList mino, maxo;
  while (fl)
  {
    FindMinMaxPrice (r, item, minprice, maxprice, mino, maxo);
    if ((minprice != -1) && (maxprice != -1))
    {
      int dealmin = 0;
      int dealmax = 0;
{
      forlist (&mino)
      {
        TradeOrder * o = (TradeOrder *) elem;
        dealmin += o->num;
      }
}
{
      forlist (&maxo)
      {
        TradeOrder * o = (TradeOrder *) elem;
        dealmax += o->num;
      }
}
      int deal = min (dealmin, dealmax);
      if (minprice <= maxprice)
      {
        // ????, ?? ??????? ????? ??????????? ??????
        int dealprice = (minprice + maxprice) / 2;
        // ?????????? ????????
        int dealstore = dealmax;
{
        forlist (&maxo)
        {
          TradeOrder * c = (TradeOrder *) elem;
          Unit * maxu1 = GetUnit (c->unit_num);
          TradeOrder * o = GetOrder (maxu1, c);
          int rdeal = o->num * deal/dealmax;
          maxu1->items.SetNum (item, maxu1->items.GetNum (item) + rdeal);
          maxu1->SetMoney (maxu1->GetMoney () - dealprice * rdeal);
          maxu1->Event (AString ("TRADE: Buys ") + AString (rdeal) + AString (" ") + AString(ItemDefs[item].names) + AString (" at $") + AString (dealprice) + AString (" each."));
          deal -= rdeal;
          dealmax -= o->num;
          o->num -= rdeal;
          if (!o->num)
          {
//              o->num = -1;
            maxu1->tradeorders.Remove (o);
            delete o;
          }
        }
}
        dealmax = dealstore;
        deal = min (dealmin, dealmax);
        dealstore = dealmin;
{
        forlist (&mino)
        {
          TradeOrder * c = (TradeOrder *) elem;
          Unit * minu = GetUnit (c->unit_num);
          TradeOrder * o = GetOrder (minu, c);
          int rdeal = o->num * deal/dealmin;
          minu->items.SetNum (item, minu->items.GetNum (item) - rdeal);
          minu->SetMoney (minu->GetMoney () + dealprice * rdeal);
          minu->Event (AString ("TRADE: Sells ") + AString (rdeal) + AString (" ") + AString(ItemDefs[item].names) + AString (" at $") + AString (dealprice) + AString (" each."));
          deal -= rdeal;
          dealmin -= o->num;
          o->num -= rdeal;
          if (!o->num)
          {
//              o->num = -1;
            minu->tradeorders.Remove (o);
            delete o;
          }
        }
}
        dealmin = dealstore;
      }else
      {
        fl = 0;
      }
    }else
    {
      fl = 0;
    }
  }
  maxo.Empty();
  mino.Empty();
}

void Game::FindMinMaxPrice (ARegion * r, int item, int &minprice, int &maxprice, AList &mino, AList &maxo)
{
  minprice = -1;
  maxprice = -1;

  maxo.Empty();
  mino.Empty();

  forlist((&r->objects))
  {
    Object * obj = (Object *) elem;
    forlist((&obj->units))
    {
      Unit * u = (Unit *) elem;
      forlist(&u->tradeorders)
      {
        TradeOrder * o = (TradeOrder *) elem;
        if (o->num != -1)
        {
        if (o->item == item)
        {
          if (((minprice >= o->price) || (minprice == -1)) && (o->doing == TRADE_SELL))
          {
            minprice = o->price;
            if (minprice > o->price)
            {
              mino.Empty ();
            }
            TradeOrder *c = new TradeOrder;
            c->item = o->item;
            c->price = o->price;
            c->num = o->num;
            c->unit_num = o->unit_num;
            mino.Add (c);
          }
          if ((maxprice <= o->price) && (o->doing == TRADE_BUY))
          {
            maxprice = o->price;
            if (maxprice < o->price)
            {
              maxo.Empty ();
            }
            TradeOrder *c = new TradeOrder;
            c->item = o->item;
            c->price = o->price;
            c->num = o->num;
            c->unit_num = o->unit_num;
            maxo.Add (c);
          }
        }
        }
      }
    }
  }
}

void Game::RunBuyOrders() {
  forlist((&regions)) {
    ARegion * r = (ARegion *) elem;

    {
      forlist((&r->markets)) {
	Market * m = (Market *) elem;
	if (m->type == M_BUY)
	  DoBuy(r,m);
      }
    }

    {
      forlist((&r->objects)) {
	Object * obj = (Object *) elem;
	forlist((&obj->units)) {
	  Unit * u = (Unit *) elem;
	  forlist((&u->buyorders)) {
	    BuyOrder * o = (BuyOrder *) elem;
	    u->Error("BUY: Can't buy that.");
	  }
	  u->buyorders.DeleteAll();
	}
      }
    }
  }
}

int Game::GetBuyAmount(ARegion * r,Market * m)
{
    int num = 0;
    forlist((&r->objects)) {
        Object * obj = (Object *) elem;
        forlist((&obj->units)) {
            Unit * u = (Unit *) elem;
            forlist ((&u->buyorders)) {
                BuyOrder * o = (BuyOrder *) elem;
                if (o->item == m->item) {
                    if (ItemDefs[o->item].type & IT_MAN) {
                        if (u->type == U_MAGE) {
                            u->Error("BUY: Mages can't recruit more men.");
                            o->num = 0;
                        }
                        if ((o->item == I_LEADERS && u->IsNormal()) ||
                            (o->item != I_LEADERS && u->IsLeader())) {
                            u->Error("BUY: Can't mix leaders and normal men.");
                            o->num = 0;
                        }
                    }
                    if (ItemDefs[o->item].type & IT_TRADE) {
                        if( !TradeCheck( r, u->faction )) {
                            u->Error( "BUY: Can't buy trade items in that "
                                      "many regions.");
                            o->num = 0;
                        }
                    }
                    if (o->num * m->price > u->GetMoney()) {
                        o->num = u->GetMoney() / m->price;
                        u->Error( "BUY: Unit attempted to buy more than it "
                                  "could afford.");
                    }
                    num += o->num;
                }
                if (o->num < 1) {
                    u->buyorders.Remove(o);
                    delete o;
                }
            } 
        }
    }
//AString("x");
    return num;
}

void Game::DoBuy(ARegion * r,Market * m) {
  /* First, find the number of items being purchased */
  int attempted = GetBuyAmount(r,m);
  
  if (m->amount != -1)
    if (attempted < m->amount) attempted = m->amount;

  m->activity = 0;
  int oldamount = m->amount;
  forlist((&r->objects)) {
    Object * obj = (Object *) elem;
    forlist((&obj->units)) {
      Unit * u = (Unit *) elem;
      forlist((&u->buyorders)) {
	BuyOrder * o = (BuyOrder *) elem;
	if (o->item == m->item) {
	  int temp = 0;
	  if (m->amount == -1) {
	    /* unlimited market */
	    temp = o->num;
	  } else {
	    if (attempted) {
	      temp = (m->amount * o->num + getrandom(attempted))
		/ attempted;
	      if (temp < 0) temp = 0;
	    }
	    attempted -= o->num;
	    m->amount -= temp;
	    m->activity += temp;
	  }

	  if (ItemDefs[o->item].type & IT_MAN) {
	    /* recruiting; must dilute skills */
	    SkillList * sl = new SkillList;
	    u->AdjustSkills();
	    delete sl;
	  }
	  u->items.SetNum(o->item,u->items.GetNum(o->item)
			  + temp);
	  u->SetMoney(u->GetMoney() - temp * m->price);
	  u->buyorders.Remove(o);
	  u->Event(AString("Buys ") + ItemString(o->item,temp)
		   + " at $" + m->price + " each.");
	  delete o;
	  break;
	}
      }
    }
  }

  m->amount = oldamount;
}

void Game::CheckUnitMaintenanceItem(int item, int value, int consume)
{
    forlist((&regions)) {
        ARegion * r = (ARegion *) elem;
        forlist((&r->objects)) {
            Object * obj = (Object *) elem;
            forlist((&obj->units)) {
                Unit * u = (Unit *) elem;
                if (u->needed && ((!consume) || 
                                  (u->GetFlag(FLAG_CONSUMING_UNIT) ||
                                   u->GetFlag(FLAG_CONSUMING_FACTION)))) {
                    int amount = u->items.GetNum(item);
                    if (amount) {
                        u->needed -= amount * value;
                        if (u->needed < 0) {
                            u->items.SetNum(item, -(u->needed / value));
                            u->needed = 0;
                        } else {
                            u->items.SetNum(item, 0);
                        }
                        u->Event (AString ("Consume ") +
                                  AString (amount -  u->items.GetNum(item)) +
                                  " " + ItemDefs[item].names + " [" +
                                  ItemDefs[item].abr + "]");
                    }
                }
            }
        }
    }
}

void Game::CheckFactionMaintenanceItem(int item, int value, int consume)
{
    forlist((&regions)) {
        ARegion * r = (ARegion *) elem;
        forlist((&r->objects)) {
            Object * obj = (Object *) elem;
            forlist((&obj->units)) {
                Unit * u = (Unit *) elem;
                if (u->needed && ((!consume) ||
                                  u->GetFlag(FLAG_CONSUMING_FACTION))) {
                    /* Go through all units again */
                    forlist((&r->objects)) {
                        Object * obj2 = (Object *) elem;
                        forlist((&obj2->units)) {
                            Unit * u2 = (Unit *) elem;
                            
                            if (u->faction == u2->faction && u != u2) {
                                int amount = u2->items.GetNum(item);
                                if (amount) {
                                    u->needed -= amount * value;
                                    if (u->needed <= 0) {
                                        u2->items.SetNum(item,
                                                         -(u->needed / value));
                                        u2->Event(*(u->name) + " borrows " +
                                                  ItemString(item,
                                                             amount +
                                                             (u->needed /
                                                              value)) +
                                                  " for maintenance.");
                                        u->Event(AString("Borrows ") + 
                                                 ItemString(item,
                                                            amount +
                                                            (u->needed /
                                                             value)) +
                                                 " from " + *(u2->name) +
                                                 " for "
                                                 "maintenance.");
                                        u->needed = 0;
                                        break;
                                    } else {
                                        u2->items.SetNum(item, 0);
                                        u2->Event(*(u->name) + " borrows " + 
                                                  ItemString(item, amount) +
                                                  " for maintenance.");
                                        u->Event(AString("Borrows ") + 
                                                 ItemString(item, amount) +
                                                 " from " + *(u2->name) +
                                                 " for maintenance.");
                                    }
                                }
                            }
                        }
                        
                        if (u->needed == 0) break;
                    }
                }
            }
        }
    }
}

void Game::CheckAllyMaintenanceItem(int item, int value)
{
    forlist((&regions)) {
        ARegion * r = (ARegion *) elem;
        forlist((&r->objects)) {
            Object * obj = (Object *) elem;
            forlist((&obj->units)) {
                Unit * u = (Unit *) elem;
                if (u->needed) {
                    /* Go through all units again */
                    forlist((&r->objects)) {
                        Object * obj2 = (Object *) elem;
                        forlist((&obj2->units)) {
                            Unit * u2 = (Unit *) elem;
                            if (u->faction != u2->faction && 
                                u2->GetAttitude(r,u) == A_ALLY) {
                                int amount = u2->items.GetNum(item);
                                if (amount) {
                                    u->needed -= amount * value;
                                    if (u->needed <= 0) {
                                        u2->Event(*(u->name) + " borrows " +
                                                  ItemString(item,
                                                             amount +
                                                             (u->needed /
                                                              value)) +
                                                  " for maintenance.");
                                        u->Event(AString("Borrows ") + 
                                                 ItemString(item,
                                                            amount +
                                                            (u->needed /
                                                             value)) +
                                                 " from " + *(u2->name) +
                                                 " for "
                                                 "maintenance.");
                                        u2->items.SetNum(item, -(u->needed /
                                                                 value));
                                        u->needed = 0;
                                        break;
                                    } else {
                                        u2->items.SetNum(item, 0);
                                        u2->Event(*(u->name) + " borrows " + 
                                                  ItemString(item, amount) +
                                                  " for maintenance.");
                                        u->Event(AString("Borrows ") + 
                                                 ItemString(item, amount) +
                                                 " from " + *(u2->name) +
                                                 " for maintenance.");
                                    }
                                }
                            }
                        }
                        
                        if (u->needed == 0) break;
                    }
                }
            }
        }
    }
}

void Game::AssessMaintenance()
{
    /* First pass: set needed */
    {
        forlist((&regions)) {
            ARegion * r = (ARegion *) elem;
            forlist((&r->objects)) {
                Object * obj = (Object *) elem;
                forlist((&obj->units)) {
                    Unit * u = (Unit *) elem;
                    u->needed = u->MaintCost();
                }
            }
        }
    }

    //
    // Check for CONSUMEing units.
    //
    if( Globals->FOOD_ITEMS_EXIST )
    {
        CheckUnitMaintenance( 1 );
        CheckFactionMaintenance( 1 );
    }

    //
    // Check the unit for money.
    //
    CheckUnitMaintenanceItem(I_SILVER, 1, 0 );

    //
    // Check other units in same faction for money
    //
    CheckFactionMaintenanceItem(I_SILVER, 1, 0 );

    if( Globals->FOOD_ITEMS_EXIST )
    {
        //
        // Check unit for possible food items.
        //
        CheckUnitMaintenance( 0 );

        //
        // Fourth pass; check other units in same faction for food items
        //
        CheckFactionMaintenance( 0 );
    }

    //
    // Check unclaimed money.
    //
    {
        forlist((&regions)) {
            ARegion * r = (ARegion *) elem;
            forlist((&r->objects)) {
                Object * obj = (Object *) elem;
                forlist((&obj->units)) {
                    Unit * u = (Unit *) elem;
                    if (u->needed && u->faction->unclaimed)
                    {
                        /* Now see if faction has money */
                        if (u->faction->unclaimed >= u->needed)
                        {
                            u->Event(AString("Claims ") + u->needed +
                                     " silver for maintenance.");
                            u->faction->unclaimed -= u->needed;
                            u->needed = 0;
                        } 
                        else
                        {
                            u->Event(AString("Claims ") +
                                     u->faction->unclaimed +
                                     " silver for maintenance.");
                            u->needed -= u->faction->unclaimed;
                            u->faction->unclaimed = 0;
                        }
                    }
                }
            }
        }
    }

    //
    // Check other allied factions for $$$.
    //
    CheckAllyMaintenanceItem( I_SILVER, 1 );

    if( Globals->FOOD_ITEMS_EXIST )
    {
        //
        // Check other factions for food items.
        //
        CheckAllyMaintenance();
    }

    //
    // Last, if the unit still needs money, starve some men.
    //
    {
        forlist((&regions)) {
            ARegion * r = (ARegion *) elem;
            forlist((&r->objects)) {
                Object * obj = (Object *) elem;
                forlist((&obj->units)) {
                    Unit * u = (Unit *) elem;
                    if (u->needed)
                    {
                        u->Short(u->needed);
                    }
                }
            }
        }
    }
}

void Game::DoGiveOrders()
{
    forlist((&regions)) {
        ARegion * r = (ARegion *) elem;
        forlist((&r->objects)) {
            Object * obj = (Object *) elem;
            forlist((&obj->units)) {
                Unit * u = (Unit *) elem;
                forlist((&u->giveorders)) {
                    Order * o = (Order *) elem;
                    if (DoGiveOrder(r,u,(GiveOrder *) o)) break;
                }
                u->giveorders.DeleteAll();
            }
        }
    }
}

int Game::DoGiveOrder(ARegion * r,Unit * u,GiveOrder * o)
{
// dsn
    if (o->except > -1)
    {
      o->amount = u->items.GetNum(o->item) - o->except;
      if (o->amount < 1)
        return 0;
    }
    int amt = o->amount;
    if (amt != -2 && amt > u->items.GetNum(o->item))
    {
        u->Error("GIVE: Not enough.");
        amt = u->items.GetNum(o->item);
    }
  
    if (o->target->unitnum == -1)
    {
        /* Give 0 */
        if (amt == -1)
        {
            u->Error("Can't discard a whole unit.");
            return 0;
        }
    
        u->Event(AString("Discards ") + ItemString(o->item,amt) + ".");
        if (ItemDefs[o->item].type & IT_MAN)
        {
            u->SetMen(o->item,u->GetMen(o->item) - amt);
        }
        else
        {
            u->items.SetNum(o->item,u->items.GetNum(o->item) - amt);
        }
        return 0;
    }
  
    Unit * t = r->GetUnitId(o->target,u->faction->num);
    if (!t)
    {
        u->Error(AString("GIVE: Nonexistant target (") + o->target->Print() +
                 ").");
        return 0;
    }

    if (o->item != I_SILVER &&
        t->faction->GetAttitude(u->faction->num) < A_FRIENDLY)
    {
        u->Error("GIVE: Target is not a member of a friendly faction.");
        return 0;
    }
    
    if (amt == -1)
    {
        /* Give unit */
        if (u->type == U_MAGE)
        {
            if (CountMages(t->faction) >= AllowedMages( t->faction )) {
                u->Error("GIVE: Faction has too many mages.");
                return 0;
            }
        }
  
        int notallied = 1;
        if (t->faction->GetAttitude(u->faction->num) == A_ALLY)
        {
            notallied = 0;
        }
        
        u->Event(AString("Gives unit to ") + *(t->faction->name) + ".");
        AString *uFactionName = u->faction->name;
        u->faction = t->faction;
        u->Event(AString("Is given to your faction by ") + *(uFactionName) + ".");

        if (notallied && u->monthorders && u->monthorders->type == O_MOVE &&
            ((MoveOrder *) u->monthorders)->advancing)
        {
            u->Error("Unit cannot advance after being given.");
            delete u->monthorders;
            u->monthorders = 0;
        }
    
        /* Check if any new skill reports have to be shown */
        forlist(&(u->skills)) {
            Skill * skill = (Skill *) elem;
            int newlvl = u->GetRealSkill(skill->type);
            int oldlvl = u->faction->skills.GetDays(skill->type);
            if (newlvl > oldlvl)
            {
                for (int i=oldlvl+1; i<=newlvl; i++)
                {
                    u->faction->shows.Add(new ShowSkill(skill->type,i));
                }
                u->faction->skills.SetDays(skill->type,newlvl);
            }
        }
    
        return notallied;
    }
  
    /* If the item to be given is a man, combine skills */
    if (ItemDefs[o->item].type & IT_MAN)
    {
        if (u->nomove) t->nomove = 1;

        if (u->type == U_MAGE || t->type == U_MAGE)
        {
            u->Error("GIVE: Magicians can't transfer men.");
            return 0;
        }

        if (o->item == I_LEADERS && t->IsNormal())
        {
            u->Error("GIVE: Can't mix leaders and normal men.");
            return 0;
        } 
        else
        {
            if (o->item != I_LEADERS && t->IsLeader())
            {
                u->Error("GIVE: Can't mix leaders and normal men.");
                return 0;
            }
        }

        if (u->faction != t->faction)
        {
            u->Error("GIVE: Can't give men to another faction.");
            return 0;
        }
        SkillList * temp = u->skills.Split(u->GetMen(),amt);
        t->skills.Combine(temp);
        delete temp;
    }

    if( ItemDefs[ o->item ].flags & ItemType::CANTGIVE )
    {
        u->Error(AString("GIVE: Can't give ") + ItemDefs[o->item].names + ".");
        return 0;
    }

    if ( o->item == I_TRIFLE)
    {
      if (t->items.GetNum (I_TRIFLE) != 0)
      {
        u->Error (AString("GIVE: Target unit already have trifle."));
        return 0;
      }
    }

    u->Event(AString("Gives ") + ItemString(o->item,amt) + " to " +
             *t->name + ".");
    if (u->faction != t->faction)
    {
        t->Event(AString("Receives ") + ItemString(o->item,amt) +
                 " from " + *u->name + ".");
    }
    AString * gname = new AString (*u->items.GetName (o->item));
    u->items.SetNum(o->item,u->items.GetNum(o->item) - amt);
    t->items.SetNum(o->item,t->items.GetNum(o->item) + amt);
    t->items.SetName (o->item, gname);

    if (ItemDefs[o->item].type & IT_MAN)
    {
        t->AdjustSkills();
    }

    return 0;
}
  
void Game::DoGuard1Orders() {
  forlist((&regions)) {
    ARegion * r = (ARegion *) elem;
    forlist((&r->objects)) {
      Object * obj = (Object *) elem;
      forlist((&obj->units)) {
	Unit * u = (Unit *) elem;
	if (u->guard == GUARD_SET ||
	    u->guard == GUARD_GUARD) {
	  if (!u->Taxers()) {
	    u->guard = GUARD_NONE;
	    u->Error("Must be combat "
		     "ready to be on guard.");
	    continue;
	  }
	  if (u->type != U_GUARD && r->HasCityGuard()) {
	    u->guard = GUARD_NONE;
	    u->Error("Is prevented from guarding by the Guardsmen.");
	    continue;
	  }
	  int facg = r->IsGuarded1();
	  if ( facg != u->faction->num && facg && u->guard == GUARD_SET)
	  {
	    u->guard = GUARD_NONE;
	    u->Error("GUARD: A unit is on guard.");
	    continue;
	  }
	}
      }
    }
{
    forlist((&r->objects)) {
      Object * obj = (Object *) elem;
      forlist((&obj->units)) {
	Unit * u = (Unit *) elem;
	if ( u->guard == GUARD_SET )
	{
	  u->guard = GUARD_GUARD;
	}
      }
}
    }
  }
}

void Game::FindDeadFactions() {
  forlist((&factions)) {
    ((Faction *) elem)->CheckExist(&regions);
  }
}

void Game::DeleteEmptyUnits() {
  forlist((&regions)) {
    ARegion * region = (ARegion *) elem;
    DeleteEmptyInRegion(region);
  }
}

void Game::DeleteEmptyInRegion(ARegion * region) {
  forlist(&region->objects) {
    Object * obj = (Object *) elem;
    forlist (&obj->units) {
      Unit * unit = (Unit *) elem;
      if (unit->IsAlive() == 0) {
	region->Kill(unit);
      }
    }
  }
}



