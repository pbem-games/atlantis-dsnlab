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
// Date        Person          Comments
// ----        ------          -------- 
// 2000/MAR/14 Larry Stanbery  Fixed SHOW bug.
// 2000/MAR/14 Davis Kulis     Added a new reporting Template.
#include "game.h"
#include "gameio.h"
#include "orders.h"
#include "skills.h"
#include "rules.h"

char * DirectionAbrs[] = {
  "N",
  "NE",
  "SE",
  "S",
  "SW",
  "NW"
};

AList ParsedTrifles;//????? ????????? ??????????

OrdersCheck::OrdersCheck()
{
    pCheckFile = 0;
    numshows = 0;
    dummyUnit.monthorders = 0;
}

void OrdersCheck::Error( const AString &strError )
{
    if( pCheckFile )
    {
        pCheckFile->PutStr("");
        pCheckFile->PutStr("");
        pCheckFile->PutStr(AString("*** Error: ") + strError + " ***");
    }
}

int Game::ParseDir(AString * token)
{
    for (int i=0; i<NDIRS; i++)
    {
        if (*token == DirectionStrs[i]) return i;
        if (*token == DirectionAbrs[i]) return i;
    }
    if (*token == "in") return MOVE_IN;
    if (*token == "out") return MOVE_OUT;
    int num = token->value();
    if (num) return MOVE_ENTER + num;
    return -1;
}

int ParseTF(AString * token)
{
    if (*token == "true") return 1;
    if (*token == "false") return 0;
    if (*token == "t") return 1;
    if (*token == "f") return 0;
    if (*token == "on") return 1;
    if (*token == "off") return 0;
    if (*token == "1") return 1;
    if (*token == "0") return 0;
    return -1;
}

UnitId *Game::ParseUnit(AString * s)
{
    AString * token = s->gettoken();
    if (!token)
    {
        return 0;
    }
  
    if (*token == "0")
    {
        delete token;
        UnitId * id = new UnitId;
        id->unitnum = -1;
        id->alias = 0;
        id->faction = 0;
        return id;
    }
	
    if (*token == "faction")
    {
        delete token;
        /* Get faction number */
        token = s->gettoken();
        if (!token)
        {
            return 0;
        }

        int fn = token->value();
        delete token;
        if (!fn)
        {
            return 0;
        }

        /* Next token should be "new" */
        token = s->gettoken();
        if (!token)
        {
            return 0;
        }

        if (!(*token == "new"))
        {
            delete token;
            return 0;
        }
        delete token;

        /* Get alias number */
        token = s->gettoken();
        if (!token)
        {
            return 0;
        }

        int un = token->value();
        delete token;
        if (!un)
        {
            return 0;
        }

        /* Return UnitId */
        UnitId * id = new UnitId;
        id->unitnum = 0;
        id->alias = un;
        id->faction = fn;
        return id;
    }
  
    if (*token == "new")
    {
        delete token;
        token = s->gettoken();
        if (!token)
        {
            return 0;
        }

        int un = token->value();
        delete token;
        if (!un)
        {
            return 0;
        }

        UnitId * id = new UnitId;
        id->unitnum = 0;
        id->alias = un;
        id->faction = 0;
        return id;
    } 
    else
    {
        int un = token->value();
        delete token;
        if (!un)
        {
            return 0;
        }

        UnitId * id = new UnitId;
        id->unitnum = un;
        id->alias = 0;
        id->faction = 0;
        return id;
    }
}

int ParseFactionType(AString * o,int * type)
{
    int i;
    for (i=0; i<NFACTYPES; i++) type[i] = 0;
    
    AString * token = o->gettoken();
    if (!token) return -1;
    
    if (*token == "generic") {
        delete token;
        for (i=0; i<NFACTYPES; i++) type[i] = 1;
        return 0;
    }
    
    while(token) {
        int foundone = 0;
        for (i=0; i<NFACTYPES; i++) {
            if (*token == FactionStrs[i]) {
                delete token;
                token = o->gettoken();
                if (!token) {
                    return -1;
                }
                type[i] = token->value();
                delete token;
                foundone = 1;
                break;
            }
        }
        if (!foundone) {
            delete token;
            return -1;
        }
        token = o->gettoken();
    }

    int tot = 0;
    for (i=0; i<NFACTYPES; i++) {
        tot += type[i];
    }
    if (tot > Globals->FACTION_POINTS) {
        return -1;
    }
    
    return 0;
}

void Game::ParseError( OrdersCheck *pCheck, Unit *pUnit, Faction *pFaction,
                       const AString &strError )
{
    if( pCheck )
    {
        pCheck->Error( strError );
    }
    else if( pUnit )
    {
        pUnit->Error( strError );
    }
    else if( pFaction )
    {
        pFaction->Error( strError );
    }
}

void Game::ParseOrders( int faction, Aorders *f, OrdersCheck *pCheck )
{
    Faction *fac = 0;
    Unit *unit = 0;
    Unit *former = 0;
  
    AString *order = f->GetLine();
    while (order)
    {
        AString saveorder = *order;
        int getatsign = order->getat();
        AString * token = order->gettoken();
        
        if (token)
        {
            int i = Parse1Order(token);
            switch (i)
            {
            case -1:
                ParseError( pCheck, unit, fac,
                            *token + " is not a valid order." );
                break;
            case O_ATLANTIS:
                if( fac )
                {
                    ParseError( pCheck, 0, fac, "No #END statement given." );
                }
                delete token;
                token = order->gettoken();
                if( !token )
                {
                    ParseError( pCheck, 0, 0, 
                                "No faction number given on #atlantis line." );
                    fac = 0;
                    break;
                }

                if( pCheck )
                {
                    fac = &( pCheck->dummyFaction );
                    pCheck->numshows = 0;
                }
                else
                {
                    fac = GetFaction(&factions,token->value());
                }

                if (!fac) break;

                delete token;
                token = order->gettoken();
                
                if( pCheck )
                {
                    if( !token )
                    {
                        ParseError( pCheck, 0, fac,
                                    "Warning: No password on #atlantis "
                                    "line." );
                    }
                }
                else
                {
                    if( !( *( fac->password ) == "none" ) )
                    {
                        if( !token || !( *( fac->password ) == *token ) )
                        {
                            ParseError( pCheck, 0, fac,
                                        "Incorrect password on #atlantis "
                                        "line." );
                            fac = 0;
                            break;
                        }
                    }

                    if (fac->num == monfaction || fac->num == guardfaction)
                    {
                        fac = 0;
                        break;
                    }
//dsnlab
                    fac->lastorders1 = fac->lastorders;
                    fac->lastorders = TurnNumber();
                }

                unit = 0;
                former = 0;
                break;

            case O_END:
        if (pCheck) pCheck->dummyUnit.ClearOrders(); // add this line
                former = 0;
                unit = 0;
                fac = 0;
                break;

            case O_UNIT:
        if (pCheck) pCheck->dummyUnit.ClearOrders(); // add this line
                if (fac)
                {
                    if (former)
                    {
                        former = 0;
                        ParseError( pCheck, 0, fac, "FORM: without END." );
                    }
                    unit = 0;
                    delete token;

                    token = order->gettoken();
                    if (!token)
                    {
                        ParseError( pCheck, 0, fac,
                                    "UNIT without unit number." );
                        unit = 0;
                        break;
                    }
                    
                    if( pCheck )
                    {
                        if (!token->value())
                        {
                            pCheck->Error("Invalid unit number.");
                        }
                        else
                        {
                            unit = &( pCheck->dummyUnit );
                            unit->monthorders = 0;
                        }
                    }
                    else
                    {
                        unit = GetUnit( token->value() );
                        if( !unit || unit->faction != fac)
                        {
                            fac->Error(*token + " is not your unit.");
                            unit = 0;
                        }
                        else
                        {
                            unit->ClearOrders();
                        }
                    }
                }
                break;
            case O_FORM:
                if (fac)
                {
                    if( unit )
                    {
                        if (former)
                        {
                            ParseError( pCheck, 0, fac,
                                        "FORM: without END." );
                        }
                        former = unit;
                        unit = ProcessFormOrder( unit, order, pCheck );
                        if( !pCheck )
                        {
                            if( unit )
                            {
                                unit->ClearOrders();
                            }
                        }
                    }
                    else
                    {
                        ParseError( pCheck, 0, fac,
                                    "Order given without a unit selected." );
                    }	
                }
                break;
            case O_ENDFORM:
                if (fac)
                {
                    if (former)
                    {
                        unit = former;
                        if( pCheck )
                        {
                            unit->monthorders = 0;
                        }
                        former = 0;
                    } 
                    else
                    {
                        ParseError( pCheck, 0, fac,
                                    "END: without FORM." );
                    }
                }
                break;
            default:
                if (fac)
                {
                    if (unit)
                    {
                        if( !pCheck && getatsign )
                        {
                            unit->oldorders.Add(new AString(saveorder));
                        }

                        ProcessOrder( i, unit, order, pCheck );
                    }
                    else
                    {
                        ParseError( pCheck, 0, fac,
                                    "Order given without a unit selected." );
                    }	
                }
            }
            SAFE_DELETE( token );
        }
        else
        {
            if( !pCheck )
            {
                if( getatsign && fac && unit )
                {
                    unit->oldorders.Add(new AString(saveorder));
                }
            }
        }
        
        delete order;
        if( pCheck )
        {
            pCheck->pCheckFile->PutStr( saveorder );
        }

        order = f->GetLine();
    }
    ParsedTrifles.DeleteAll();
}

void Game::ProcessOrder( int orderNum, Unit *unit, AString *o,
                         OrdersCheck *pCheck )
{
    switch( orderNum )
    {
    case O_ADDRESS:
        ProcessAddressOrder( unit, o, pCheck );
        break;
    case O_ADVANCE:
        ProcessAdvanceOrder( unit, o, pCheck );
        break;
    case O_ASSASSINATE:
        ProcessAssassinateOrder( unit, o, pCheck );
        break;
    case O_ATTACK:
        ProcessAttackOrder( unit, o, pCheck );
        break;
    case O_AUTOTAX:
        ProcessAutoTaxOrder( unit, o, pCheck );
        break;
    case O_AVOID:
        ProcessAvoidOrder( unit, o, pCheck );
        break;
    case O_BEHIND:
        ProcessBehindOrder( unit, o, pCheck );
        break;
    case O_BUILD:
        ProcessBuildOrder( unit, o, pCheck );
        break;
    case O_BUY:
        ProcessBuyOrder( unit, o, pCheck );
        break;
    case O_TRADE:
        ProcessTradeOrder( unit, o, pCheck );
        break;
    case O_CAST:
        ProcessCastOrder( unit, o, pCheck );
        break;
    case O_CLAIM:
        ProcessClaimOrder( unit, o, pCheck );
        break;
    case O_COMBAT:
        ProcessCombatOrder( unit, o, pCheck );
        break;
    case O_CONSUME:
        ProcessConsumeOrder( unit, o, pCheck );
        break;
    case O_DECLARE:
        ProcessDeclareOrder( unit->faction, o, pCheck );
        break;
    case O_DESCRIBE:
        ProcessDescribeOrder( unit, o, pCheck );
        break;
    case O_DESTROY:
        ProcessDestroyOrder( unit, pCheck );
        break;
    case O_ENTER:
        ProcessEnterOrder( unit, o, pCheck );
        break;
    case O_ENTERTAIN:
        ProcessEntertainOrder( unit, pCheck );
        break;
    case O_FACTION:
        ProcessFactionOrder( unit, o, pCheck );
        break;
    case O_FIND:
        ProcessFindOrder( unit, o, pCheck );
        break;
    case O_FORGET:
        ProcessForgetOrder( unit, o, pCheck );
        break;
    case O_GIVE:
        ProcessGiveOrder( unit, o, pCheck );
        break;
    case O_GUARD:
        ProcessGuardOrder( unit, o, pCheck );
        break;
    case O_HOLD:
        ProcessHoldOrder( unit, o, pCheck );
        break;
    case O_LEAVE:
        ProcessLeaveOrder( unit, pCheck );
        break;
    case O_MOVE:
        ProcessMoveOrder( unit, o, pCheck );
        break;
    case O_NAME:
        ProcessNameOrder( unit, o, pCheck );
        break;
    case O_NOAID:
        ProcessNoaidOrder( unit, o, pCheck );
        break;
    case O_OPTION:
        ProcessOptionOrder( unit, o, pCheck );
        break;
    case O_PASSWORD:
        ProcessPasswordOrder( unit, o, pCheck );
        break;
    case O_PILLAGE:
        ProcessPillageOrder( unit, pCheck );
        break;
    case O_PRODUCE:
        ProcessProduceOrder( unit, o, pCheck );
        break;
    case O_PROMOTE:
        ProcessPromoteOrder( unit, o, pCheck );
        break;
    case O_QUIT:
        ProcessQuitOrder( unit, o, pCheck );
        break;
    case O_RESTART:
        ProcessRestartOrder( unit, o, pCheck );
        break;
    case O_REVEAL:
        ProcessRevealOrder( unit, o, pCheck );
        break;
    case O_SAIL:
        ProcessSailOrder( unit, o, pCheck );
        break;
    case O_SELL:
        ProcessSellOrder( unit, o, pCheck );
        break;
    case O_SHOW:
        ProcessReshowOrder( unit, o, pCheck );
        break;
    case O_STEAL:
        ProcessStealOrder( unit, o, pCheck );
        break;
    case O_STUDY:
        ProcessStudyOrder( unit, o, pCheck );
        break;
    case O_TAX:
        ProcessTaxOrder( unit, pCheck );
        break;
    case O_TEACH:
        ProcessTeachOrder( unit, o, pCheck );
        break;
    case O_WORK:
        ProcessWorkOrder( unit, pCheck );
        break;
    case O_ROUTE:
        ProcessRouteOrder( unit, o, pCheck );
        break;
    case O_LAST_:
        ProcessLast_Order( unit, o, pCheck );
        break;
    case O_NOSPOILS:
    	ProcessNospoilsOrder(unit, o, pCheck);
    	break;
    case O_SPOILS:
    	ProcessSpoilsOrder(unit, o, pCheck);
    	break;
    case O_TRIFLE:
    	ProcessTrifleOrder(unit, o, pCheck);
    	break;
    }
}

void Game::ProcessPasswordOrder(Unit * u,AString * o, OrdersCheck *pCheck )
{
    if( pCheck )
    {
        return;
    }

    AString * token = o->gettoken();
    if (u->faction->password) delete u->faction->password;
    if (token) {
        u->faction->password = token;
        u->faction->Event(AString("Password is now: ") + *token);
    } else {
        u->faction->password = new AString("none");
        u->faction->Event("Password cleared.");
    }
}

void Game::ProcessOptionOrder(Unit * u,AString *o, OrdersCheck *pCheck )
{
    AString * token = o->gettoken();
    if (!token)
    {
        ParseError( pCheck, u, 0, "OPTION: What option?" );
        return;
    }

    if (*token == "times")
    {
        delete token;
        if( !pCheck )
        {
            u->faction->Event("Times will be sent to your faction.");
            u->faction->times = 1;
        }
        return;
    }
    
    if (*token == "notimes")
    {
      delete token;
      if( !pCheck )
      {
          u->faction->Event("Times will not be sent to your faction.");
          u->faction->times = 0;
      }
      return;
    }

    if (*token == "template")
    {
        delete token;

        token = o->gettoken();
        if (!token) {
            ParseError( pCheck, u, 0, "OPTION: No template type specified." );
            return;
        }

        int newformat = -1;
        if (*token == "off") {
            newformat = TEMPLATE_OFF;
        }
        if (*token == "short") {
            newformat = TEMPLATE_SHORT;
        }
        if (*token == "long") {
            newformat = TEMPLATE_LONG;
        }
        // DK
        if (*token == "map") {
            newformat = TEMPLATE_MAP;
        }
        delete token;

        if (newformat == -1)
        {
            ParseError( pCheck, u, 0, "OPTION: Invalid template type." );
            return;
        }

        if( !pCheck )
        {
            u->faction->temformat = newformat;
        }

        return;
    }

    delete token;

    ParseError( pCheck, u, 0, "OPTION: Invalid option." );
}
  
void Game::ProcessReshowOrder(Unit * u,AString * o, OrdersCheck *pCheck )
{
    AString * token = o->gettoken();
    if (!token) {
        // LLS
        ParseError( pCheck, u, 0, "SHOW: Show what?" );
        return;
    }

    if( pCheck )
    {
        if( pCheck->numshows++ > 100 )
        {
            if( pCheck->numshows == 102 )
            {
                pCheck->Error( "Too many SHOW orders." );
            }
            return;
        }
    }
    else
    {
        if (u->faction->numshows++ > 100)
        {
            if (u->faction->numshows == 102)
            {
                u->Error("Too many SHOW orders.");
            }
            return;
        }
    }

    if (*token == "skill")
    {
        delete token;
    
        token = o->gettoken();
        if (!token)
        {
            ParseError( pCheck, u, 0, "SHOW: Show what skill?" );
            return;
        }
        int sk = ParseSkill(token);
        delete token;
        // ALT, 25-Jul-2000
        // Fix to prevent segfault if skill doesn't exist
        if (sk == -1) {
            ParseError( pCheck, u, 0, "SHOW: No such skill." );
            return;
        }
    
        token = o->gettoken();
        if (!token)
        {
            ParseError( pCheck, u, 0, "SHOW: No skill level given.");
            return;
        }
        int lvl = token->value();
        delete token;
    
        if( !pCheck )
        {
            if (lvl > u->faction->skills.GetDays(sk))
            {
                u->Error("SHOW: Faction doesn't have that skill.");
                return;
            }
    
            u->faction->shows.Add(new ShowSkill(sk,lvl));
        }
    
        return;
    }
  
    if (*token == "item")
    {
        delete token;
        token = o->gettoken();

        if (!token) {
            ParseError( pCheck, u, 0, "SHOW: Show which item?");
            return;
        }

        int item = ParseItem(token);
        delete token;

        if (item == -1) {
            // LLS
            ParseError( pCheck, u, 0, "SHOW: No such item." );
            return;
        }

        if( !pCheck )
        {
            u->faction->itemshows.Add(new AString(ItemDescription(item)));
        }
        return;
    }

    ParseError( pCheck, u, 0, "SHOW: Show what?");
}

void Game::ProcessForgetOrder(Unit * u,AString * o, OrdersCheck *pCheck )
{
    AString * token = o->gettoken();
    if (!token) {
        ParseError( pCheck, u, 0, "FORGET: No skill given.");
        return;
    }
	
    int sk = ParseSkill(token);
    delete token;
  
    if (sk==-1) {
        ParseError( pCheck, u, 0, "FORGET: Invalid skill.");
        return;
    }
  
    if( !pCheck )
    {
        ForgetOrder *ord = new ForgetOrder;
        ord->skill = sk;
        u->forgetorders.Add(ord);
    }
}

void Game::ProcessEntertainOrder(Unit * unit, OrdersCheck *pCheck )
{
    if( pCheck )
    {
        if (unit->monthorders)
        {
            pCheck->Error("ENTERTAIN: Overwriting previous month-long order.");
        }
        unit->monthorders = &( pCheck->dummyOrder );
        unit->monthorders->type = O_ENTERTAIN;
    }
    else
    {
        if (unit->monthorders) {
            unit->Error("ENTERTAIN: Overwriting previous month-long order.");
            delete unit->monthorders;
        }
        ProduceOrder * o = new ProduceOrder;
        o->item = I_SILVER;
        o->skill = S_ENTERTAINMENT;
        unit->monthorders = o;
    }
}
      
void Game::ProcessCombatOrder(Unit * u,AString * o, OrdersCheck *pCheck )
{
    AString * token = o->gettoken();
    if (!token)
    {
        if( !pCheck )
        {
            u->combat = -1;
            u->Event("Combat spell set to none.");
        }
        return;
    }
    int sk = ParseSkill(token);
    delete token;

    if (sk==-1) {
        ParseError( pCheck, u, 0, "COMBAT: Invalid skill.");
        return;
    }
    if( !( SkillDefs[sk].flags & SkillType::MAGIC ))
    {
        ParseError( pCheck, u, 0, "COMBAT: That is not a magic skill.");
        return;
    }
    if( !( SkillDefs[sk].flags & SkillType::COMBAT ))
    {
        ParseError( pCheck, u, 0, 
                    "COMBAT: That skill cannot be used in combat.");
        return;
    }

    if( !pCheck )
    {
        if (u->type != U_MAGE) {
            u->Error("COMBAT: That unit is not a mage.");
            return;
        }
        if (!u->GetSkill(sk)) {
            u->Error("COMBAT: Unit does not possess that skill.");
            return;
        }
        
        u->combat = sk;
        u->Event(AString("Combat spell set to ") +
                 SkillDefs[sk].name + ".");
    }
}

void Game::ProcessClaimOrder(Unit * u,AString * o, OrdersCheck *pCheck )
{
    AString * token = o->gettoken();
    if (!token) {
        ParseError( pCheck, u, 0, "CLAIM: No amount given.");
        return;
    }
	
    int value = token->value();
    delete token;
    if (!value) {
        ParseError( pCheck, u, 0, "CLAIM: No amount given.");
        return;
    }

    if( !pCheck )
    {
        if (value > u->faction->unclaimed) {
            u->Error("CLAIM: Don't have that much unclaimed silver.");
            value = u->faction->unclaimed;
        }
        u->faction->unclaimed -= value;
        u->SetMoney(u->GetMoney() + value);
        u->Event(AString("Claims $") + value + ".");
    }
}

void Game::ProcessFactionOrder( Unit *u, AString *o, OrdersCheck *pCheck )
{
    if( Globals->FACTION_LIMIT_TYPE != GameDefs::FACLIM_FACTION_TYPES )
    {
        u->Error( "FACTION: Invalid order, no faction types in this game." );
        return;
    }

    int oldfactype[ NFACTYPES ];
    int factype[ NFACTYPES ];

    int i;
    if( !pCheck )
    {
        for( i = 0; i < NFACTYPES; i++ )
        {
            oldfactype[ i ] = u->faction->type[ i ];
        }
    }

    int retval = ParseFactionType(o,factype);
    if (retval == -1)
    {
        ParseError( pCheck, u, 0, "FACTION: Bad faction type.");
        return;
    }
  
    if( !pCheck )
    {
        int m = CountMages(u->faction);
  
        for( i = 0; i < NFACTYPES; i++ )
        {
            u->faction->type[i] = factype[i];
        }

        if( m > AllowedMages( u->faction ))
        {
            u->Error(AString("FACTION: Too many mages to change to that "
                             "faction type."));

            for( i = 0; i < NFACTYPES; i++ )
            {
                u->faction->type[ i ] = oldfactype[ i ];
            }

            return;
        }

        u->faction->lastchange = TurnNumber();
        u->faction->DefaultOrders();
    }
}

void Game::ProcessAssassinateOrder(Unit * u,AString * o, OrdersCheck *pCheck )
{
    UnitId * id = ParseUnit(o);
    if (!id || id->unitnum == -1) {
        ParseError( pCheck, u, 0, "ASSASSINATE: No target given.");
        return;
    }
    if( !pCheck )
    {
        if (u->stealorders) delete u->stealorders;
        AssassinateOrder * ord = new AssassinateOrder;
        ord->target = id;
        u->stealorders = ord;
    }
}

void Game::ProcessStealOrder(Unit * u,AString * o, OrdersCheck *pCheck )
{
    UnitId * id = ParseUnit(o);
    if (!id || id->unitnum == -1) {
        ParseError( pCheck, u, 0, "STEAL: No target given.");
        return;
    }
    AString * token = o->gettoken();
    if (!token) {
        ParseError( pCheck, u, 0, "STEAL: No item given.");
        delete id;
        return;
    }
    int i = ParseItem(token);
    delete token;
    if (i == -1) {
        ParseError( pCheck, u, 0, "STEAL: Bad item given.");
        delete id;
        return;
    }

    if( !pCheck )
    {
        if (IsSoldier(i)) {
            u->Error("STEAL: Can't steal that.");
            delete id;
            return;
        }
        StealOrder * ord = new StealOrder;
        ord->target = id;
        ord->item = i;
        if (u->stealorders) delete u->stealorders;
        u->stealorders = ord;
    }
}

void Game::ProcessQuitOrder(Unit *u,AString *o, OrdersCheck *pCheck )
{
    if( !pCheck )
    {
        if (u->faction->password && !(*(u->faction->password) == "none")) {
            AString *token = o->gettoken();
            if (!token) {
                u->faction->Error("QUIT: Must give the correct password.");
                return;
            }
            
            if (!(*token == *(u->faction->password))) {
                delete token;
                u->faction->Error("QUIT: Must give the correct password.");
                return;
            }
            
            delete token;
// dsnlab
            token = o->gettoken();
            if (token)
            {
              if (*token == "real")
              {
                u->faction->quit = QUIT_REAL;
                return;
              }
            }
        }
        
        if (u->faction->quit != QUIT_AND_RESTART) {
            u->faction->quit = QUIT_BY_ORDER;
        }
    }
}

void Game::ProcessRestartOrder(Unit *u,AString *o, OrdersCheck *pCheck )
{
    if( !pCheck )
    {
        if (u->faction->password && !(*(u->faction->password) == "none"))
        {
            AString *token = o->gettoken();
            if (!token)
            {
                u->faction->Error("RESTART: Must give the correct password.");
                return;
            }
            
            if (!(*token == *(u->faction->password)))
            {
                delete token;
                u->faction->Error("RESTART: Must give the correct password.");
                return;
            }
            
            delete token;
        }

        if (u->faction->quit != QUIT_AND_RESTART)
        {
            u->faction->quit = QUIT_AND_RESTART;
            Faction *pFac = AddFaction();
            pFac->SetAddress( *( u->faction->address ));
            AString *facstr = new AString( AString("Restarting ")
                                           + *( pFac->address ) + "." );
            newfactions.Add(facstr);
        }
    }
}

void Game::ProcessDestroyOrder(Unit * u, OrdersCheck *pCheck )
{
    if( !pCheck )
    {
        u->destroy = 1;
    }
}

void Game::ProcessFindOrder(Unit * u,AString * o, OrdersCheck *pCheck )
{
    AString * token = o->gettoken();
    if (!token) {
        ParseError( pCheck, u, 0, "FIND: No faction number given.");
        return;
    }
    int n = token->value();
    delete token;
    if (n==0) {
        ParseError( pCheck, u, 0, "FIND: No faction number given.");
        return;
    }
    if( !pCheck )
    {
        FindOrder * f = new FindOrder;
        f->find = n;
        u->findorders.Add(f);
    }
}

void Game::ProcessConsumeOrder(Unit *u, AString *o, OrdersCheck *pCheck )
{
    AString *token = o->gettoken();
    if (token) {
        if (*token == "unit") {
            if( !pCheck )
            {
                u->SetFlag(FLAG_CONSUMING_UNIT, 1);
                u->SetFlag(FLAG_CONSUMING_FACTION, 0);
            }
            delete token;
            return;
        }

        if (*token == "faction") {
            if( !pCheck )
            {
                u->SetFlag(FLAG_CONSUMING_UNIT, 0);
                u->SetFlag(FLAG_CONSUMING_FACTION, 1);
            }
            delete token;
            return;
        }

        if (*token == "none") {
            if( !pCheck )
            {
                u->SetFlag(FLAG_CONSUMING_UNIT, 0);
                u->SetFlag(FLAG_CONSUMING_FACTION, 0);
            }
            delete token;
            return;
        }

        delete token;
        ParseError( pCheck, u, 0, "CONSUME: Invalid value.");
    } else {
        if( !pCheck )
        {
            u->SetFlag(FLAG_CONSUMING_UNIT, 0);
            u->SetFlag(FLAG_CONSUMING_FACTION, 0);
        }
    }
}
	
void Game::ProcessRevealOrder(Unit * u,AString * o, OrdersCheck *pCheck)
{
    if( !pCheck )
    {
        AString * token = o->gettoken();
        if (token) {
            if (*token == "unit") {
                u->reveal = REVEAL_UNIT;
                delete token;
                return;
            }
            if (*token == "faction") {
                delete token;
                u->reveal = REVEAL_FACTION;
                return;
            }
            if (*token == "none") {
                delete token;
                u->reveal = REVEAL_NONE;
                return;
            }
        } else {
            u->reveal = REVEAL_NONE;
        }
    }
}

void Game::ProcessTaxOrder(Unit * u, OrdersCheck *pCheck )
{
    if( !pCheck )
    {
        if (u->taxing == TAX_PILLAGE) {
            u->Error("TAX: The unit is already pillaging.");
            return;
        }
        u->taxing = TAX_TAX;
    }
}

void Game::ProcessPillageOrder(Unit * u, OrdersCheck *pCheck )
{
    if( !pCheck )
    {
        if (u->taxing == TAX_TAX) {
            u->Error("PILLAGE: The unit is already taxing.");
            return;
        }
        u->taxing = TAX_PILLAGE;
    }
}

void Game::ProcessPromoteOrder(Unit * u,AString * o, OrdersCheck *pCheck )
{
    UnitId * id = ParseUnit(o);
    if (!id || id->unitnum == -1) {
        ParseError( pCheck, u, 0, "PROMOTE: No target given.");
        return;
    }
    if( !pCheck )
    {
        if (u->promote) {
            delete u->promote;
        }
        u->promote = id;
    }
}

void Game::ProcessLeaveOrder(Unit * u, OrdersCheck *pCheck )
{
    if( !pCheck )
    {
        if (u->monthorders && u->monthorders->type == O_BUILD) return;
        if (u->enter == 0) u->enter = -1;
    }
}

void Game::ProcessEnterOrder(Unit * u,AString * o, OrdersCheck *pCheck )
{
    AString * token = o->gettoken();
    if (!token) {
        ParseError( pCheck, u, 0, "ENTER: No object specified.");
        return;
    }
    int i = token->value();
    delete token;
    if (i) {
        if( !pCheck )
        {
            u->enter = i;
        }
    } else {
        ParseError( pCheck, u, 0, "ENTER: No object specified.");
    }
}

void Game::ProcessBuildOrder( Unit *unit, AString *o, OrdersCheck *pCheck )
{
    AString * token = o->gettoken();
    if (token) {
        int ot = ParseObject(token);
        delete token;
        if (ot==-1) {
            ParseError( pCheck, unit, 0, "BUILD: Not a valid object name.");
            return;
        }
        if( pCheck )
        {
            if (unit->monthorders)
            {
                pCheck->Error("BUILD: Overwriting previous month-long order.");
            }
            unit->monthorders = &( pCheck->dummyOrder );
            unit->monthorders->type = O_BUILD;
            return;
        }
        else
        {
            if ( unit->object->region->type == R_OCEAN) {
                unit->Error("BUILD: Can't build in an ocean.");
                return;
            }
        
            if (ObjectIsShip(ot) && ot != O_BALLOON) {
                if (!unit->object->region->IsCoastal()) {
                    unit->Error("BUILD: Can't build ship in "
                                "non-coastal region.");
                    return;
                }
            }
            if ( unit->object->region->buildingseq > 99) {
                unit->Error("BUILD: The region is full.");
                return;
            }
            Object * obj = new Object( unit->object->region );
            obj->type = ot;
//            if (ObjectIsShip(ot)) {
//                obj->num = shipseq++;
//                obj->SetName(new AString(AString("Ship")));
//            } else {
//                obj->num = unit->object->region->buildingseq++;
//                obj->SetName(new AString(AString("Building")));
//            }
            obj->incomplete = ObjectDefs[obj->type].cost;
            unit->MoveUnit( obj );
            unit->object->region->objects.Add(obj);
        }
    }
    else
    {
        if( pCheck )
        {
            if (unit->monthorders)
            {
                pCheck->Error("BUILD: Overwriting previous month-long order.");
            }
            unit->monthorders = &( pCheck->dummyOrder );
            unit->monthorders->type = O_BUILD;
            return;
        }
    }

    BuildOrder * order = new BuildOrder;
    if (unit->monthorders)
    {
        delete unit->monthorders;
        unit->Error("BUILD: Overwriting previous month-long order.");
    }
    unit->monthorders = order;
    if (unit->enter == -1) unit->enter = 0;
}

void Game::ProcessAttackOrder(Unit * u,AString * o, OrdersCheck *pCheck ) 
{
    UnitId * id = ParseUnit(o);
    while (id && id->unitnum != -1) {
        if( !pCheck )
        {
            if (!u->attackorders) u->attackorders = new AttackOrder;
            u->attackorders->targets.Add(id);
        }
        id = ParseUnit(o);
    }
}

void Game::ProcessLast_Order(Unit * u,AString * o, OrdersCheck *pCheck )
{
  if ( !pCheck )
  {
    u->faction->lastorders = u->faction->lastorders1;
    u->faction->Event ("You have missed turn.");
  }
}

void Game::ProcessNospoilsOrder(Unit *u, AString *o, OrdersCheck *pCheck)
{
	ParseError(pCheck, u, 0, "NOSPOILS: This command is deprecated.  "
			"Use the 'SPOILS' command instead");

    /* Instant order */
    AString * token = o->gettoken();
    if (!token) {
        ParseError( pCheck, u, 0, "NOSPOILS: Invalid value.");
        return;
    }
    int val = ParseTF(token);
    delete token;
    if (val==-1) {
        ParseError( pCheck, u, 0, "NOSPILS: Invalid value.");
        return;
    }
    if( !pCheck ) {
		u->SetFlag(FLAG_FLYSPOILS, 0);
		u->SetFlag(FLAG_RIDESPOILS, 0);
		u->SetFlag(FLAG_WALKSPOILS, 0);
        u->SetFlag(FLAG_NOSPOILS,val);
    }
}

void Game::ProcessSpoilsOrder(Unit *u, AString *o, OrdersCheck *pCheck)
{
	/* Instant order */
	AString *token = o->gettoken();
	int flag = 0;
	int val = 1;
	if(token) {
		if(*token == "none") flag = FLAG_NOSPOILS;
		else if(*token == "walk") flag = FLAG_WALKSPOILS;
		else if(*token == "ride") flag = FLAG_RIDESPOILS;
		else if(*token == "fly") flag = FLAG_FLYSPOILS;
		else if(*token == "all") val = 0;
		else ParseError(pCheck, u, 0, "SPOILS: Bad argument.");
		delete token;
	}

	if(!pCheck) {
		/* Clear all the flags */
		u->SetFlag(FLAG_NOSPOILS, 0);
		u->SetFlag(FLAG_WALKSPOILS, 0);
		u->SetFlag(FLAG_RIDESPOILS, 0);
		u->SetFlag(FLAG_FLYSPOILS, 0);

		/* Set the flag we're trying to set */
		if(flag) {
			u->SetFlag(flag, val);
		}
	}
}

void Game::ProcessRouteOrder(Unit * u,AString * o, OrdersCheck *pCheck )
{
  int moveDir = 0; // ???ࠢ???? 0 - ?⮨? 1 - ? ?????? 2 - ? ??????⥫?
  UnitId * f = ParseUnit(o);
  if (!f) 
  {
  	  ParseError( pCheck, u, 0, "Route: Invalid target.");
	  return;
  }

  UnitId * t = ParseUnit(o);
  if (!t) 
  { 
	  ParseError( pCheck, u, 0, "Route: Invalid target.");         
	  return;
  }
    
  int item = -1;
  AString * token = o->gettoken();
  if (!token) 
  { 
	  ParseError( pCheck, u, 0, "Route: No item given.");
	  return;
  }
  item = ParseItem(token);

  Location * myLoc = regions.FindUnit(u->num);
  Location * tLoc = regions.FindUnit(t->unitnum);
  Location * fLoc = regions.FindUnit(f->unitnum);

  if ((myLoc->region->zloc != tLoc->region->zloc)||
		(myLoc->region->zloc != fLoc->region->zloc))
	{ // ?? ?? ?? ࠧ??? ?஢???!!!
		ParseError(pCheck, u, 0, "Route: Indifferent region levels.");
		return;
	}

  if ((u->faction->num != fLoc->unit->faction->num )||
	  (u->faction->num != tLoc->unit->faction->num))
  {	//	 ?? ?㦮? ?ࠪ樥? ?? ???塞??
		ParseError(pCheck, u, 0, "Route: Invalid target.");
		return;
  }

  int fLim  =0;
  int myLim =0;
  AString * tmp = o->gettoken();
  if (tmp) 
  {	  
	  fLim=tmp->value(); delete tmp;
      tmp = o->gettoken();
	  if (tmp) {  myLim=tmp->value(); delete tmp;  }
  }

  if ((myLoc->region->xloc == tLoc->region->xloc)&&
      (myLoc->region->yloc == tLoc->region->yloc))
	  { // ?? ?? ?冷? ? ??????⥫?? !!!
	  	int amt = u->items.GetNum(item);
		if (amt > myLim)
		{
			if( !pCheck )
			{
				GiveOrder * orderG = new GiveOrder;
                orderG->item = item;
                orderG->target = t;
                orderG->amount = amt - myLim;
                u->giveorders.Add(orderG);
				moveDir = 1;
			} // ??? ??? ????? ?⤠?? ??⮯??? ? ??????
		}
  }

	if ((myLoc->region->xloc == fLoc->region->xloc)&&
		(myLoc->region->yloc == fLoc->region->yloc))
	{ // ?? ?? ?冷? ? ????஬ !!!
		int amt = fLoc->unit->items.GetNum(item);
		if (amt > fLim)
			if( !pCheck )
			{
				GiveOrder * orderR = new GiveOrder;
                orderR->item = item;
				  UnitId * id = new UnitId;
                  id->unitnum = u->num;
                  id->alias = 0;
                  id->faction = 0;
                orderR->target = id;
				orderR->amount = amt-fLim;
                fLoc->unit->giveorders.Add(orderR);
				moveDir = 2;
			} // ??? ??? ???? ????? ????稫? ??⮯??? ? ??????⥫?
	}
	if (moveDir==0)// ??宦? ?? ??? ?? ????।??? ? ???㤠 ?? ᮡ?ࠥ???
	{
	 if (u->items.GetNum(item) > myLim)
	 { // ????? ???? 祣? ⮯??? ? ??????⥫?
		moveDir = 2;
	 }
	 else
	 { // ????? ?? 祣? ⮯??? ? ??????
		moveDir = 1;
	 }
	}


	if(moveDir != 0)  // ???? ⮯???
	{           // ??砫? ⮯???
	if (u->object->IsBoat()) 
	{
		SailOrder *m = 0;
		if( pCheck )
		{
	        if (u->monthorders && u->monthorders->type != O_SAIL)
		    {
			    pCheck->Error("ROUTE: Overwriting previous month-long order.");
			}
			u->monthorders = &( pCheck->dummyOrder );
			u->monthorders->type = O_SAIL;
		}
		else
		{
	        if (u->monthorders && u->monthorders->type != O_SAIL) 
			{
		        u->Error("ROUTE: Overwriting previous month-long order.");
	            delete u->monthorders;
	            u->monthorders = 0;
	        }
	        if (!u->monthorders) 
			{
	            u->monthorders = new SailOrder;
	        }
	        m = (SailOrder *) u->monthorders;
	    }
	}
	else
	{
		MoveOrder *m = 0;		// ?㯮 ᪮??஢??? ?? ProcessMoveOrder
		if( pCheck )			// ?祡??? ?ॢ???
			{
			if (u->monthorders && u->monthorders->type != O_MOVE)
				{
				pCheck->Error("ROUTE: Overwriting previous month-long order.");
				}
			u->monthorders = &( pCheck->dummyOrder );
			u->monthorders->type = O_MOVE;
			}
		else					// ??? ?? ?祡??? ?ॢ???
		{
			if (u->monthorders && u->monthorders->type != O_MOVE)
			{
				u->Error("ROUTE: Overwriting previous month-long order.");
				delete u->monthorders;
				u->monthorders = 0;
			}
        if (!u->monthorders) { u->monthorders = new MoveOrder; }
        m = (MoveOrder *) u->monthorders;
        m->advancing = 0;
		}
	}							// ????? ?㯮 ᪮??஢?????? ?ࠣ?????

		int xMove = 0;
		int yMove = 0;
		if (moveDir == 1)
		{
			xMove = fLoc->region->xloc - myLoc->region->xloc;
			yMove = fLoc->region->yloc - myLoc->region->yloc;
		}
		if (moveDir == 2)
			{
				xMove = tLoc->region->xloc - myLoc->region->xloc;
				yMove = tLoc->region->yloc - myLoc->region->yloc;
			}
	    ARegionArray *pArr = regions.pRegionArrays[ myLoc->region->zloc ];
		int mapXsize = pArr->x;
		if (xMove > mapXsize/2) xMove = xMove - mapXsize;
		if (xMove < - mapXsize/2) xMove = mapXsize + xMove;
		int d = 0;
		int fm = 0;
		while ((xMove!=0)||(yMove!=0))
		{
			/*  "North",		0
			    "Northeast",	1
				"Southeast",	2
				"South",		3
				"Southwest",	4
				"Northwest"		5			????୮? ⠪  */
			
			fm = 0;
			// ???? ?? ᥢ???
			if ((xMove == 0)&&(yMove <0)) {yMove +=2; d=0; ++fm;}
			// ???? ?? 
			if ((xMove == 0)&&(yMove >0)&&(fm==0)) {yMove -=2; d=3; ++fm;}
			// ???? ?? ?????? ???? ??१ ᥢ??
			if ((xMove < 0)&&(yMove ==0)&&(fm==0)) { ++xMove; ++yMove; d=5; ++fm;}
			// ???? ?? ???⮪? ???? ??१ ??
			if ((xMove > 0)&&(yMove ==0)&&(fm==0)) { --xMove; --yMove; d=2; ++fm;}
			// ???? ?? ᥢ?஧?????
			if ((xMove < 0)&&(yMove <0)&&(fm==0))  { ++xMove; ++yMove; d=5; ++fm;}
			// ???? ?? ᥢ?஢??⮪?
			if ((xMove > 0)&&(yMove <0)&&(fm==0))  { --xMove; ++yMove; d=1; ++fm;}
			// ???? ?? ??????
			if ((xMove < 0)&&(yMove >0)&&(fm==0))  { ++xMove; --yMove; d=4; ++fm;}
			// ???? ?? ???⮪?
			if ((xMove > 0)&&(yMove >0)&&(fm==0))  { --xMove; --yMove; d=2; ++fm;}
    		MoveDir * x = new MoveDir;
			x->dir = d;
	        if (u->monthorders->type == O_SAIL) 
			{
				SailOrder * m = (SailOrder *) u->monthorders;
				m->dirs.Add(x);
			}
			if (u->monthorders->type == O_MOVE)
			{
				MoveOrder * m = (MoveOrder *) u->monthorders;
				m->dirs.Add(x);
			}
		
	}  // ???稫? ⮯??? ?? ??????? ??ᯨᠭ??
  }
}

void Game::ProcessTradeOrder(Unit * u,AString * o, OrdersCheck *pCheck )
{
    AString * token = o->gettoken();
    if (!token)
    {
        ParseError( pCheck, u, 0, "TRADE: sell or buy?");
        return;
    }
    int doing = -1;
    if (*token == AString ("SELL"))
    {
      doing = TRADE_SELL;
    }
    if (*token == AString ("BUY"))
    {
      doing = TRADE_BUY;
    }
    if (doing == -1)
    {
        ParseError( pCheck, u, 0, "TRADE: sell or buy?");
        return;
    }
    delete token;
    token = o->gettoken();
    if (!token)
    {
        ParseError( pCheck, u, 0, "TRADE: Number to trade not given.");
        return;
    }
    int num = token->value();
    delete token;
    if (!num)
    {
        ParseError( pCheck, u, 0, "TRADE: Number to trade not given.");
        return;
    }
    token = o->gettoken();
    if (!token)
    {
        ParseError( pCheck, u, 0, "TRADE: Item not given.");
        return;
    }
    int it = ParseItem(token);
    delete token;
    if (it == -1)
    {
        ParseError( pCheck, u, 0, "TRADE: Can't trade that.");
        return;
    }
    token = o->gettoken();
    if (!token)
    {
        ParseError( pCheck, u, 0, "TRADE: Price not given.");
        return;
    }
    int price = token->value();
    delete token;
    if (!price)
    {
        ParseError( pCheck, u, 0, "TRADE: Price not given.");
        return;
    }
    if( !pCheck )
    {
        if (it == I_PEASANT)
        {
            it = u->object->region->race; 
        }
        TradeOrder * b = new TradeOrder;
        b->item = it;
        b->num = num;
        b->price = price;
        b->doing = doing;
        b->unit_num = u->num;
        u->tradeorders.Add(b);
    }
}

void Game::ProcessSellOrder(Unit * u,AString * o, OrdersCheck *pCheck )
{
    AString * token = o->gettoken();
    if (!token) {
        ParseError( pCheck, u, 0, "SELL: Number to sell not given.");
        return;
    }
// dsnlab
    int num;
    if (*token == "all")
    {
      num = -1;
    }else
    {
      num = token->value();
    }
    delete token;
    if (!num) {
        ParseError( pCheck, u, 0, "SELL: Number to sell not given.");
        return;
    }
    token = o->gettoken();
    if (!token) {
        ParseError( pCheck, u, 0, "SELL: Item not given.");
        return;
    }
    int it = ParseItem(token);
    delete token;
    if (it == -1) {
        ParseError( pCheck, u, 0, "SELL: Can't sell that.");
        return;
    }
    if( !pCheck )
    {
        SellOrder * s = new SellOrder;
        s->item = it;
        s->num = num;
        if (num == -1)
        {
          s->num = u->items.GetNum (it);
        }
        u->sellorders.Add(s);
    }
}

void Game::ProcessBuyOrder( Unit *u, AString *o, OrdersCheck *pCheck )
{
    AString * token = o->gettoken();
    if (!token)
    {
        ParseError( pCheck, u, 0, "BUY: Number to buy not given.");
        return;
    }
    int num = token->value();
    delete token;
    if (!num)
    {
        ParseError( pCheck, u, 0, "BUY: Number to buy not given.");
        return;
    }
    token = o->gettoken();
    if (!token)
    {
        ParseError( pCheck, u, 0, "BUY: Item not given.");
        return;
    }
    int it = ParseItem(token);
    delete token;
    if (it == -1)
    {
        ParseError( pCheck, u, 0, "BUY: Can't buy that.");
        return;
    }
    if( !pCheck )
    {
        if (it == I_PEASANT)
        {
            it = u->object->region->race; 
        }
        BuyOrder * b = new BuyOrder;
        b->item = it;
        b->num = num;
        u->buyorders.Add(b);
    }
}

void Game::ProcessProduceOrder(Unit * u,AString * o, OrdersCheck *pCheck )
{
    AString * token = o->gettoken();
    if (!token) {
        ParseError( pCheck, u, 0, "PRODUCE: No item given.");
        return;
    }
    int it = ParseItem(token);
    delete token;
    if (it == -1) {
        ParseError( pCheck, u, 0, "PRODUCE: Can't produce that.");
        return;
    }
    if( pCheck )
    {
        if (u->monthorders)
        {
            pCheck->Error("PRODUCE: Overwriting previous month-long order.");
        }
        u->monthorders = &( pCheck->dummyOrder );
        u->monthorders->type = O_PRODUCE;
    }
    else
    {
        ProduceOrder * p = new ProduceOrder;
        p->item = it;
        p->skill = ItemDefs[it].skill;
        if (u->monthorders) {
            delete u->monthorders;
            u->Error("PRODUCE: Overwriting previous month-long order.");
        }
        u->monthorders = p;
    }
}

void Game::ProcessWorkOrder(Unit * u, OrdersCheck *pCheck )
{
    if( pCheck )
    {
        if (u->monthorders)
        {
            pCheck->Error("WORK: Overwriting previous month-long order.");
        }
        u->monthorders = &( pCheck->dummyOrder );
        u->monthorders->type = O_WORK;
    }
    else
    {
        ProduceOrder * order = new ProduceOrder;
        order->skill = -1;
        order->item = I_SILVER;
        if (u->monthorders) {
            delete u->monthorders;
            u->Error("WORK: Overwriting previous month-long order.");
        }
        u->monthorders = order;
    }
}

void Game::ProcessTeachOrder(Unit * u,AString * o, OrdersCheck *pCheck )
{
    TeachOrder * order = 0;

    if( pCheck )
    {
        if (u->monthorders && u->monthorders->type != O_TEACH) 
        {
            pCheck->Error("TEACH: Overwriting previous month-long order.");
        }
        u->monthorders = &( pCheck->dummyOrder );
        u->monthorders->type = O_TEACH;
    }
    else
    {
        if (u->monthorders && u->monthorders->type == O_TEACH) {
            order = (TeachOrder *) u->monthorders;
        } else {
            order = new TeachOrder;
        }
    }

    int students = 0;
    UnitId * id = ParseUnit(o);
    while (id && id->unitnum != -1) {
        students++;
        if( order )
        {
            order->targets.Add(id);
        }
        id = ParseUnit(o);
    }

    if (!students) {
        ParseError( pCheck, u, 0, "TEACH: No students given.");
        return;
    }

    if( !pCheck )
    {
        if (u->monthorders && u->monthorders->type != O_TEACH) {
            delete u->monthorders;
            u->Error("TEACH: Overwriting previous month-long order.");
        }
        u->monthorders = order;
    }
}

void Game::ProcessStudyOrder(Unit * u,AString * o, OrdersCheck *pCheck )
{
    AString * token = o->gettoken();
    if (!token) {
        ParseError( pCheck, u, 0, "STUDY: No skill given.");
        return;
    }
    int sk = ParseSkill(token);
    delete token;
    if (sk==-1) {
        ParseError( pCheck, u, 0, "STUDY: Invalid skill.");
        return;
    }

    if( pCheck )
    {
        if (u->monthorders)
        {
            pCheck->Error("STUDY: Overwriting previous month-long order.");
        }
        u->monthorders = &( pCheck->dummyOrder );
        u->monthorders->type = O_STUDY;
    }
    else
    {
        StudyOrder * order = new StudyOrder;
        order->skill = sk;
        order->days = 0;
        if (u->monthorders) {
            delete u->monthorders;
            u->Error("STUDY: Overwriting previous month-long order.");
        }
        u->monthorders = order;
    }
}

void Game::ProcessDeclareOrder(Faction * f,AString * o, OrdersCheck *pCheck )
{
    AString * token = o->gettoken();
    if (!token) {
        ParseError( pCheck, 0, f, "DECLARE: No faction given.");
        return;
    }
    int fac;
    if (*token == "default") {
        fac = -1;
    } else {
        fac = token->value();
    }
    delete token;
	
    if( !pCheck )
    {
        Faction * target;
        if (fac != -1) {
            target = GetFaction(&factions,fac);	
            if (!target) {
                f->Error(AString("DECLARE: Non-existent faction ") + fac
                         + ".");
                return;
            }
            if (target == f) {
                f->Error(AString("DECLARE: Can't declare towards your own "
                                 "faction."));
                return;
            }
        }
    }
	
    token = o->gettoken();
    if (!token) {
        if (fac != -1) {
            if( !pCheck )
            {
                f->SetAttitude(fac,-1);
            }
        }
        return;
    }
	
    int att = ParseAttitude(token);
    delete token;
    if (att == -1) {
        ParseError( pCheck, 0, f, "DECLARE: Invalid attitude.");
        return;
    }

    if( !pCheck )
    {
        if (fac == -1) {
            f->defaultattitude = att;
        } else {
            f->SetAttitude(fac,att);
        }
    }
}

void Game::ProcessGiveOrder(Unit *unit,AString * o, OrdersCheck *pCheck )
{
    UnitId * t = ParseUnit(o);
    if (!t) {
        ParseError( pCheck, unit, 0, "GIVE: Invalid target.");
        return;
    }
    AString * token = o->gettoken();
    if (!token) {
        ParseError( pCheck, unit, 0, "GIVE: No amount given.");
        return;
    }
    int amt;
    int except = -1;
    if (*token == "unit") {
        amt = -1;
    } else {

// dsn

        if (*token == "all")
        {
          delete token;
          token = o->gettoken ();
          if (!token) {
              ParseError( pCheck, unit, 0, "GIVE: No item given.");
              return;
          }
          int item = ParseItem(token);
          delete token;
          if (item == -1)
          {
            ParseError( pCheck, unit, 0, "GIVE: Invalid item.");
            return;
          }
          token = o->gettoken ();
          except = 0;
          if (token)
          {
            delete token;
            token = o->gettoken ();
            if (token) except = token->value ();
          }
          if( !pCheck )
          {
            GiveOrder * order = new GiveOrder;
            order->item = item;
            order->target = t;
            order->amount = 0;
// dsn
            order->except = except;
            unit->giveorders.Add(order);
//            return;
          }
          return;
        }else
        {
          amt = token->value();
        }
    }
    delete token;
    int item = I_LEADERS;
    if (amt != -1) {
        token = o->gettoken();
        if (!token) {
            ParseError( pCheck, unit, 0, "GIVE: No item given.");
            return;
        }
        item = ParseItem(token);
        delete token;
    }
    
    if (item == -1) {
        ParseError( pCheck, unit, 0, "GIVE: Invalid item.");
        return;
    }

    if( !pCheck )
    {
        GiveOrder * order = new GiveOrder;
        order->item = item;
        order->target = t;
        order->amount = amt;
        order->except = except;
        unit->giveorders.Add(order);
    }
    return;
}

void Game::ProcessDescribeOrder( Unit *unit, AString *o, OrdersCheck *pCheck )
{
    AString * token = o->gettoken();
    if (!token) {
        ParseError( pCheck, unit, 0, "DESCRIBE: No argument.");
        return;
    }
    if (*token == "unit")
    {
        delete token;
        token = o->gettoken();
        if( !pCheck )
        {
            unit->SetDescribe(token);
        }
        return;
    }
    if (*token == "ship" || *token == "building" || *token == "object" ||
        *token == "structure")
    {
        delete token;
        token = o->gettoken();
        if( !pCheck )
        {
            // ALT, 25-Jul-2000
            // Fix to prevent non-owner units from describing objects
            if ( unit != unit->object->GetOwner() ) {
                unit->Error("DESCRIBE: Unit is not owner.");
                return;
            }
            unit->object->SetDescribe(token);
        }
        return;
    }
    ParseError( pCheck, unit, 0, "DESCRIBE: Can't describe that.");
}

void Game::ProcessTrifleOrder(Unit *unit,AString * o, OrdersCheck *pCheck )
{
  if (!pCheck)
  {
    AString * name = o->gettoken();
    if (!name)
    {
      ParseError (pCheck, unit, 0, "TRIFLE: Invalid name");
      return;
    }
    AString * newname = name->getlegal();
    if (!newname)
    {
      ParseError (pCheck, unit, 0, "TRIFLE: Invalid name");
      return;
    }
    if (unit->faction->unclaimed < Globals->TRIFLE_COST)
    {
      unit->Error ("TRIFLE: Not enough unclaimed silver.");
      return;
    }
    int num = unit->items.GetNum (I_TRIFLE);
    if (num != 0)
    {
      unit->Error ("TRIFLE: Unit can own only one trifle");
      return;
    }
    unit->faction->unclaimed -= Globals->TRIFLE_COST;
//    unit->items.SetNum (I_TRIFLE, 1);
    AString * sname = new AString (AString (":)") + AString (*unit->faction->name->getlegal()) + AString (":) Turn: ") + AString (TurnNumber()) + AString ("] ") + *newname);
    forlist(&ParsedTrifles)
    {
       if(*(AString*)elem==*sname)
       {
          unit->Error (AString("TRIFLE: ")+*newname+" alredy created");
          delete sname;
          return;
       }
    }
    unit->items.AddItem(I_TRIFLE, sname);
    ParsedTrifles.Add(sname);
//    AString * sname = new AString (AString ("[") + AString (*unit->faction->name) + AString (" Turn: ") + AString (TurnNumber()) + AString ("] ") + *newname);
//    unit->items.SetName (I_TRIFLE, sname);
    unit->Event (AString ("New trifle made"));
  }
}
void Game::ProcessNameOrder(Unit *unit,AString * o, OrdersCheck *pCheck )
{
    AString * token = o->gettoken();
    if (!token)
    {
        ParseError( pCheck, unit, 0, "NAME: No argument.");
        return;
    }
    if (*token == "faction") {
        delete token;
        token = o->gettoken();
        if (!token) {
            ParseError( pCheck, unit, 0, "NAME: No name given.");
            return;
        }
        if( !pCheck )
        {
            unit->faction->SetName(token);
        }
        return;
    }
    
    if (*token == "unit") {
        delete token;
        token = o->gettoken();
        if (!token) {
            ParseError( pCheck, unit, 0, "NAME: No name given.");
            return;
        }
        if( !pCheck )
        {
            unit->SetName(token);
        }
        return;
    }
	
    if (*token == "building" || *token == "ship" || *token == "object" ||
        *token == "structure") {
        delete token;
        token = o->gettoken();
        if (!token) {
            ParseError( pCheck, unit, 0, "NAME: No name given.");
            return;
        }
        if( !pCheck )
        {
            // ALT, 25-Jul-2000
            // Fix to prevent non-owner units from renaming objects
            if ( unit != unit->object->GetOwner() ) {
                unit->Error("NAME: Unit is not owner.");
                return;
            }
            unit->object->SetName(token);
        }
        return;
    }
  
    // ALT, 26-Jul-2000
    // Allow some units to rename cities. Unit must be at least the owner
    // of tower to rename village, fort to rename town and castle to
    // rename city.
    if (*token == "village" || *token == "town" || *token == "city")
    {
        delete token;
        token = o->gettoken();

        if ( !token ) {
            ParseError( pCheck, unit, 0, "NAME: No name given.");            
            return;
        }

        if ( !pCheck )
        {
            // ALT, 28-Aug-2000, some checks to prevent core dumps
            if (!unit->object) {
                unit->Error( "NAME: Unit is not inside an object." );
                return;                
            }
            if (!unit->object->region->town) {
                unit->Error( "NAME: Nothing to rename." );
                return;                
            }
            if (unit->object->type == O_CITADEL ||
                ((unit->object->type == O_TOWER || 
                  unit->object->type == O_FORT ||
                  unit->object->type == O_CASTLE) && 
                  unit->object->region->town->TownType() == TOWN_VILLAGE) || 
                ((unit->object->type == O_FORT ||
                  unit->object->type == O_CASTLE) && 
                  unit->object->region->town->TownType() == TOWN_TOWN) || 
                (unit->object->type == O_CASTLE && 
                  unit->object->region->town->TownType() == TOWN_CITY))
            {
                // ALT, 28-Aug-2000, chech if object is finished
                if (unit->object->incomplete > 0) {
                    unit->Error( "NAME: Object is not finished." );
                    return;
                }
                
                if (unit != unit->object->GetOwner()) {
                    unit->Error( "NAME: Unit is not the owner of object." );
                    return;
                }

                AString * newname = token->getlegal();
                if ( !newname ) {
                    unit->Error( "NAME: Illegal name." );
                    return;
                }
                
                // ALT, 28-Aug-2000, unclaimed silved needed
                if (unit->faction->unclaimed < 500) {
                    unit->Error("NAME: Not enough unclaimed silver.");
                    return;
                }                
                unit->faction->unclaimed -= 500;

                // ALT, 28-Aug-2000, prevent renaming of starting cities
                if (unit->object->region->IsStartingCity()) {
                    unit->Error( "NAME: Starting cities can't be renamed." );
                    return;
                }

                unit->Event(AString("Renames ") + *(unit->object->region->town->name) + " to " + *newname + ".");
                unit->object->region->NotifyCity(unit, *(unit->object->region->town->name), *newname);
                unit->object->region->town->name = newname;

            } else {
                unit->Error( "NAME: Object is not large enough to rename city." );
            }
        }        
        return;
    }

    delete token;
    ParseError( pCheck, unit, 0, "NAME: Can't name that.");
}

void Game::ProcessGuardOrder(Unit * u,AString * o, OrdersCheck *pCheck )
{
    /* This is an instant order */
    AString * token = o->gettoken();
    if (!token) {
        ParseError( pCheck, u, 0, "GUARD: Invalid value.");
        return;
    }
    int val = ParseTF(token);
    delete token;
    if (val==-1) {
        ParseError( pCheck, u, 0, "GUARD: Invalid value.");
        return;
    }
    if( !pCheck )
    {
        if (val==0) {
            if (u->guard != GUARD_AVOID)
                u->guard = GUARD_NONE;
        } else {
            u->guard = GUARD_SET;
        }
    }
}

void Game::ProcessBehindOrder(Unit * u,AString * o, OrdersCheck *pCheck )
{
    /* This is an instant order */
    AString * token = o->gettoken();
    if (!token) {
        ParseError( pCheck, u, 0, "BEHIND: Invalid value.");
        return;
    }
    int val = ParseTF(token);
    if (val == -1) {
        ParseError( pCheck, u, 0, "BEHIND: Invalid value.");
        return;
    }
    if( !pCheck )
    {
        u->SetFlag(FLAG_BEHIND,val);
    }
}

void Game::ProcessNoaidOrder(Unit * u,AString * o, OrdersCheck *pCheck )
{
    /* Instant order */
    AString * token = o->gettoken();
    if (!token) {
        ParseError( pCheck, u, 0, "NOAID: Invalid value.");
        return;
    }
    int val = ParseTF(token);
    delete token;
    if (val==-1) {
        ParseError( pCheck, u, 0, "NOAID: Invalid value.");
        return;
    }
    if( !pCheck )
    {
        u->SetFlag(FLAG_NOAID,val);
    }
}

void Game::ProcessHoldOrder(Unit * u,AString * o, OrdersCheck *pCheck )
{
    /* Instant order */
    AString * token = o->gettoken();
    if (!token) {
        ParseError( pCheck, u, 0, "HOLD: Invalid value.");
        return;
    }
    int val = ParseTF(token);
    delete token;
    if (val==-1) {
        ParseError( pCheck, u, 0, "HOLD: Invalid value.");
        return;
    }
    if( !pCheck )
    {
        u->SetFlag(FLAG_HOLDING,val);
    }
}

void Game::ProcessAutoTaxOrder(Unit * u,AString * o, OrdersCheck *pCheck )
{
    /* Instant order */
    AString * token = o->gettoken();
    if (!token) {
        ParseError( pCheck, u, 0, "AUTOTAX: Invalid value.");
        return;
    }
    int val = ParseTF(token);
    delete token;
    if (val==-1) {
        ParseError( pCheck, u, 0, "AUTOTAX: Invalid value.");
        return;
    }
    if( !pCheck )
    {
        u->SetFlag(FLAG_AUTOTAX,val);
    }
}	

void Game::ProcessAvoidOrder(Unit * u,AString * o, OrdersCheck *pCheck )
{
    /* This is an instant order */
    AString * token = o->gettoken();
    if (!token) {
        ParseError( pCheck, u, 0, "AVOID: Invalid value.");
        return;
    }
    int val = ParseTF(token);
    delete token;
    if (val==-1) {
        ParseError( pCheck, u, 0, "AVOID: Invalid value.");
        return;
    }
    if( !pCheck )
    {
        if (val==1) {
            u->guard = GUARD_AVOID;
        } else {
            if (u->guard == GUARD_AVOID)
            {
                u->guard = GUARD_NONE;
            }
        }
    }
}

Unit *Game::ProcessFormOrder( Unit *former, AString *o, OrdersCheck *pCheck )
{
    AString *t = o->gettoken();
    if (!t)
    {
        ParseError( pCheck, former, 0, "Must give alias in FORM order.");
        return 0;
    }
  
    int an = t->value();
    delete t;
    if (!an)
    {
        ParseError( pCheck, former, 0, "Must give alias in FORM order.");
        return 0;
    }
    if( pCheck )
    {
        Unit *retval = &( pCheck->dummyUnit );
        retval->monthorders = 0;
        return retval;
    }
    else
    {
        if( former->object->region->GetUnitAlias( an, former->faction->num ))
        {
            former->Error("Alias multiply defined.");
            return 0;
        }
        Unit *temp = GetNewUnit( former->faction, an );
        temp->CopyFlags( former );
        temp->DefaultOrders( former->object );
        temp->MoveUnit( former->object );
        return temp;
    }
}

void Game::ProcessAddressOrder(Unit * u,AString * o, OrdersCheck *pCheck )
{
    /* This is an instant order */
    AString * token = o->gettoken();
    if (token) {
        if( !pCheck )
        {
            u->faction->address = token;
        }
    } else {
        ParseError( pCheck, u, 0, "ADDRESS: No address given.");
    }
}

void Game::ProcessAdvanceOrder(Unit * u,AString * o, OrdersCheck *pCheck )
{
    MoveOrder *m = 0;
    if( pCheck )
    {
        if (u->monthorders && u->monthorders->type != O_ADVANCE)
        {
            pCheck->Error("ADVANCE: Overwriting previous month-long orders.");
        }
        u->monthorders = &( pCheck->dummyOrder );
        u->monthorders->type = O_ADVANCE;
    }
    else
    {
        if (u->monthorders && u->monthorders->type != O_MOVE)
        {
            u->Error("ADVANCE: Overwriting previous month-long orders.");
            delete u->monthorders;
            u->monthorders = 0;
        }
        if (!u->monthorders) {
            u->monthorders = new MoveOrder;
        }
        m = (MoveOrder *) u->monthorders;
        m->advancing = 1;
    }

    for (;;) {
        AString * t = o->gettoken();
        if (!t) {
            return;
        }
        int d = ParseDir(t);
        delete t;
        if (d!=-1) {
            if( !pCheck )
            {
                MoveDir * x = new MoveDir;
                x->dir = d;
                m->dirs.Add(x);
            }
        } else {
            ParseError( pCheck, u, 0, "ADVANCE: Warning, bad direction.");
            return;
        }
    }
}

void Game::ProcessMoveOrder(Unit * u,AString * o, OrdersCheck *pCheck )
{
    MoveOrder *m = 0;
    if( pCheck )
    {
        if (u->monthorders && u->monthorders->type != O_MOVE)
        {
            pCheck->Error("MOVE: Overwriting previous month-long order.");
        }
        u->monthorders = &( pCheck->dummyOrder );
        u->monthorders->type = O_MOVE;
    }
    else
    {
        if (u->monthorders && u->monthorders->type != O_MOVE) {
            u->Error("MOVE: Overwriting previous month-long order.");
            delete u->monthorders;
            u->monthorders = 0;
        }
        if (!u->monthorders) {
            u->monthorders = new MoveOrder;
        }
        m = (MoveOrder *) u->monthorders;
        m->advancing = 0;
    }

    for (;;) {
        AString * t = o->gettoken();
        if (!t) {
            return;
        }
        int d = ParseDir(t);
        delete t;
        if (d!=-1) {
            if( !pCheck )
            {
                MoveDir * x = new MoveDir;
                x->dir = d;
                m->dirs.Add(x);
            }
        } else {
            ParseError( pCheck, u, 0, "MOVE: Warning, bad direction.");
            return;
        }
    }
}

void Game::ProcessSailOrder(Unit * u,AString * o, OrdersCheck *pCheck )
{
    SailOrder *m = 0;
    if( pCheck )
    {
        if (u->monthorders && u->monthorders->type != O_SAIL)
        {
            pCheck->Error("SAIL: Overwriting previous month-long order.");
        }
        u->monthorders = &( pCheck->dummyOrder );
        u->monthorders->type = O_SAIL;
    }
    else
    {
        if (u->monthorders && u->monthorders->type != O_SAIL) {
            u->Error("SAIL: Overwriting previous month-long order.");
            delete u->monthorders;
            u->monthorders = 0;
        }
        if (!u->monthorders) {
            u->monthorders = new SailOrder;
        }
        m = (SailOrder *) u->monthorders;
    }

    for (;;) {
        AString * t = o->gettoken();
        if (!t) {
            return;
        }
        int d = ParseDir(t);
        delete t;
        if (d == -1) {
            ParseError( pCheck, u, 0, "SAIL: Warning, bad direction.");
            return;
        } else {
            if (d < NDIRS) {
                if( !pCheck )
                {
                    MoveDir * x = new MoveDir;
                    x->dir = d;
                    m->dirs.Add(x);
                }
            } else {
                return;
            }
        }
    }
}		

OrdersCheck::~OrdersCheck()
{
  dummyUnit.ClearOrders(); // dummyUnit.monthorders=0 also works
}
