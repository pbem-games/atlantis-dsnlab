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
#ifndef GAME_CLASS
#define GAME_CLASS

class Game;

#include "aregion.h"
#include "alist.h"
#include "faction.h"
#include "production.h"
#include "object.h"

extern AList * Trifle_rep;
#define CURRENT_ATL_VER MAKE_ATL_VER( 4, 0, 4 )

class OrdersCheck
{
public:
    OrdersCheck();
    virtual ~OrdersCheck();

    Aoutfile *pCheckFile;
    Unit dummyUnit;
    Faction dummyFaction;
    Order dummyOrder;
    int numshows;

    void Error( const AString &error );
};

class Game
{
    friend class Faction;
public:
    Game();
    ~Game();
  
    int NewGame();	
    int OpenGame();
    void DummyGame();

    int RunGame();
    int EditGame( int *pSaveGame );
    int SaveGame();
    int WritePlayers();
    int ReadPlayers();
    int ReadPlayersLine( AString *pToken, AString *pLine, Faction *pFac,
                         int newPlayer );
    void WriteNewFac( Faction *pFac );

    int ViewMap(const AString &,const AString &);
    // LLS
    void UnitFactionMap();
    // dsnlab
    void GameStatistic();

    int DoOrdersCheck( const AString &strOrders, const AString &strCheck );

    Faction *AddFaction(int nn=-1);

    //
    // Give this particular game a chance to set up the faction. This is in
    // extra.cpp.
    //
    int SetupFaction( Faction *pFac );

    void ViewFactions();
  
    //
    // Get a new unit, with its number assigned.
    //
    Unit *GetNewUnit( Faction *fac, int an = 0 );

    //
    // Setup the array of units.
    //
    void SetupUnitNums();
  
    //
    // Get a unit by its number.
    //
    Unit *GetUnit( int num );

    int TurnNumber();

private:
    //
    // Game editing functions.
    //
    ARegion *EditGameFindRegion();
    void EditGameFindUnit();
    void EditGameRegion( ARegion *pReg );
    void EditGameUnit( Unit *pUnit );
    void EditGameUnitItems( Unit *pUnit );
    void EditGameUnitSkills( Unit *pUnit );
    void EditGameUnitMove( Unit *pUnit );

    void PreProcessTurn();
    void Do1Move(AString *);
    void Do1Set(AString *);
    void ReadOrders();
    void RunOrders();
    void ClearOrders(Faction *);
    void MakeFactionReportLists();
    void CountAllMages();
    void WriteReport();
    void DeleteDeadFactions();
    
    //
    // Standard creation functions.
    //
    void CreateCityMons();
    void CreateWMons();
    void CreateLMons();

    //
    // Game-specific creation functions (see world.cpp).
    //
    void CreateWorld();
    void CreateNPCFactions();
    void CreateCityMon( ARegion *pReg, int percent );
    int MakeWMon( ARegion *pReg );
    void MakeLMon( Object *pObj );

    void WriteSurfaceMap( Aoutfile *f, ARegionArray *pArr, int type );
    void WriteUnderworldMap( Aoutfile *f, ARegionArray *pArr, int type );
    char GetRChar( ARegion *r );
    AString GetXtraMap(ARegion *,int);

    AList factions;
    AList newfactions; /* List of strings */
    AList battles;
    ARegionList regions;
    int factionseq;
    int unitseq;
    Unit **ppUnits;
    int maxppunits;
    int shipseq;
    int year;
    int month;

    enum {
        GAME_STATUS_UNINIT,
        GAME_STATUS_NEW,
        GAME_STATUS_RUNNING,
        GAME_STATUS_FINISHED,
    };
    int gameStatus;

    int guardfaction;
    int monfaction;
    int doExtraInit;

    //  
    // Parsing functions
    //
    void ParseError( OrdersCheck *pCheck, Unit *pUnit, Faction *pFac,
                     const AString &strError );
    UnitId *ParseUnit(AString * s);
    int ParseDir(AString * token);


    void ParseOrders(int faction, Aorders *ordersFile, OrdersCheck *pCheck );
    void ProcessOrder( int orderNum, Unit *unit, AString *order,
                       OrdersCheck *pCheck );
    void ProcessMoveOrder(Unit *,AString *, OrdersCheck *pCheck );
    void ProcessAdvanceOrder(Unit *,AString *, OrdersCheck *pCheck );
    Unit *ProcessFormOrder( Unit *former, AString *order,
                            OrdersCheck *pCheck );
    void ProcessAddressOrder(Unit *,AString *, OrdersCheck *pCheck );
    void ProcessAvoidOrder(Unit *,AString *, OrdersCheck *pCheck );
    void ProcessGuardOrder(Unit *,AString *, OrdersCheck *pCheck );
    void ProcessNameOrder(Unit *,AString *, OrdersCheck *pCheck );
    void ProcessDescribeOrder(Unit *,AString *, OrdersCheck *pCheck );
    void ProcessBehindOrder(Unit *,AString *, OrdersCheck *pCheck );
    void ProcessGiveOrder(Unit *,AString *, OrdersCheck *pCheck );
    void ProcessDeclareOrder(Faction *,AString *, OrdersCheck *pCheck );
    void ProcessStudyOrder(Unit *,AString *, OrdersCheck *pCheck );
    void ProcessTeachOrder(Unit *,AString *, OrdersCheck *pCheck );
    void ProcessWorkOrder(Unit *, OrdersCheck *pCheck );
    void ProcessProduceOrder(Unit *,AString *, OrdersCheck *pCheck );
    void ProcessBuyOrder( Unit *, AString *, OrdersCheck *pCheck );
    void ProcessSellOrder(Unit *,AString *, OrdersCheck *pCheck );
    void ProcessTradeOrder( Unit *, AString *, OrdersCheck *pCheck );
    void ProcessAttackOrder(Unit *,AString *, OrdersCheck *pCheck );
    void ProcessBuildOrder( Unit *, AString *, OrdersCheck *pCheck );
    void ProcessSailOrder(Unit *,AString *, OrdersCheck *pCheck );
    void ProcessEnterOrder(Unit *,AString *, OrdersCheck *pCheck );
    void ProcessLeaveOrder(Unit *, OrdersCheck *pCheck );
    void ProcessPromoteOrder(Unit *,AString *, OrdersCheck *pCheck );
    void ProcessTaxOrder(Unit *, OrdersCheck *pCheck );
    void ProcessPillageOrder(Unit *, OrdersCheck *pCheck );
    void ProcessConsumeOrder(Unit *, AString *, OrdersCheck *pCheck );
    void ProcessRevealOrder(Unit *,AString *, OrdersCheck *pCheck );
    void ProcessFindOrder(Unit *,AString *, OrdersCheck *pCheck );
    void ProcessDestroyOrder(Unit *, OrdersCheck *pCheck );
    void ProcessQuitOrder(Unit *,AString *, OrdersCheck *pCheck );
    void ProcessRestartOrder(Unit *,AString *, OrdersCheck *pCheck );
    void ProcessAssassinateOrder(Unit *,AString *, OrdersCheck *pCheck );
    void ProcessStealOrder(Unit *,AString *, OrdersCheck *pCheck );
    void ProcessFactionOrder(Unit *,AString *, OrdersCheck *pCheck );
    void ProcessClaimOrder(Unit *,AString *, OrdersCheck *pCheck );
    void ProcessCombatOrder(Unit *,AString *, OrdersCheck *pCheck );
    void ProcessCastOrder(Unit *,AString *, OrdersCheck *pCheck );
    void ProcessEntertainOrder(Unit *, OrdersCheck *pCheck );
    void ProcessForgetOrder(Unit *,AString *, OrdersCheck *pCheck );
    void ProcessReshowOrder(Unit *,AString *, OrdersCheck *pCheck );
    void ProcessHoldOrder(Unit *,AString *, OrdersCheck *pCheck );
    void ProcessNoaidOrder(Unit *,AString *, OrdersCheck *pCheck );
    void ProcessAutoTaxOrder(Unit *,AString *, OrdersCheck *pCheck );
    void ProcessOptionOrder(Unit *,AString *, OrdersCheck *pCheck );
    void ProcessPasswordOrder(Unit *,AString *, OrdersCheck *pCheck );
    
    void ProcessRouteOrder(Unit *,AString * , OrdersCheck *pCheck);
    void ProcessLast_Order(Unit *,AString * , OrdersCheck *pCheck);
	void ProcessNospoilsOrder(Unit *, AString *, OrdersCheck *pCheck);
	void ProcessSpoilsOrder(Unit *, AString *, OrdersCheck *pCheck);
	void ProcessTrifleOrder(Unit *, AString *, OrdersCheck *pCheck);
    //
    // Game running functions
    //

    //
    // This can be called by parse functions
    //
    int CountMages(Faction *);
    void CountItems (int **);
    int CountItem (Faction *, int);
    void FindDeadFactions();
    void DeleteEmptyUnits();
    void DeleteEmptyInRegion(ARegion *);
    void EmptyHell();
    void DoGuard1Orders();
    void DoGiveOrders();

    //
    // Faction limit functions.
    //
    // The first 3 are game specific and can be found in extra.cpp. They
    // may return -1 to indicate no limit.
    //
    int AllowedMages( Faction *pFac );
    int AllowedTaxes( Faction *pFac );
    int AllowedTrades( Faction *pFac );
    int TaxCheck( ARegion *pReg, Faction *pFac );
    int TradeCheck( ARegion *pReg, Faction *pFac );
// dsnlab
    int TradeCheckFishing( ARegion *pReg, Faction *pFac );
    int TradeCheckRoadbuild( ARegion *pReg, Faction *pFac );

    //
    // The DoGiveOrder returns 0 normally, or 1 if no more GIVE orders
    // should be allowed
    //
    int DoGiveOrder(ARegion *,Unit *,GiveOrder *);

    //
    // These are game specific, and can be found in extra.cpp
    //
    void CheckUnitMaintenance( int consume );
    void CheckFactionMaintenance( int consume );
    void CheckAllyMaintenance();

    void CheckUnitMaintenanceItem(int item, int value, int consume );
    void CheckFactionMaintenanceItem(int item, int value, int consume );
    void CheckAllyMaintenanceItem(int item, int value);
    void AssessMaintenance();

    void GrowWMons(int);
    void GrowLMons(int);
    void PostProcessUnit(ARegion *,Unit *);

    //
    // PostProcessUnitExtra can be used to provide game-specific unit
    // post processing.
    //
    void PostProcessUnitExtra(ARegion *,Unit *);
    void PostProcessTurn();

    //
    // CheckVictory is used to see if the game is over.
    //
    Faction *CheckVictory();

    void EndGame( Faction *pVictor );

// dsnlab

    void RunTradeOrders ();
    void DoTrade (ARegion *);
    TradeOrder * GetOrder (Unit *, TradeOrder *);
    void RealTrade (ARegion *, int);
    void FindMinMaxPrice (ARegion *, int, int &, int &, AList &, AList &);
    void RunBuyOrders();
    void DoBuy(ARegion *,Market *);
    int GetBuyAmount(ARegion *,Market *);
    void RunSellOrders();
    void DoSell(ARegion *,Market *);
    int GetSellAmount(ARegion *,Market *);
    void DoAttackOrders();
    void CheckWMonAttack(ARegion *,Unit *);
    Unit *GetWMonTar(ARegion *,int,Unit *);
    int CountWMonTars(ARegion *,Unit *);
    void AttemptAttack(ARegion *,Unit *,Unit *,int,int=0);
    void DoAutoAttacks();
    void DoAutoAttacksRegion(ARegion *);
    void DoAdvanceAttack(ARegion *,Unit *);
    void DoAutoAttack(ARegion *,Unit *);
    void DoAdvanceAttacks(AList *);
    void DoAutoAttackOn(ARegion *,Unit *);
    void RemoveEmptyObjects();
    void RunEnterOrders();
    void Do1EnterOrder(ARegion *,Object *,Unit *);
    void RunPromoteOrders();
    void Do1PromoteOrder(Object *,Unit *);
    void RunPillageOrders();
    int CountPillagers(ARegion *);
    void ClearPillagers(ARegion *);
    void RunPillageRegion(ARegion *);
    void RunTaxOrders();
    void RunTaxRegion(ARegion *);
    int CountTaxers(ARegion *);
    void RunFindOrders();
    void RunFindUnit(Unit *);
    void RunDestroyOrders();
    void Do1Destroy(ARegion *,Object *,Unit *);
    void RunQuitOrders();
    void RunForgetOrders();
    void Do1Quit(Faction *);
    void SinkShips();
    void RunStealOrders();
    AList * CanSeeSteal(ARegion *,Unit *);
    void Do1Steal(ARegion *,Object *,Unit *);
    void Do1Assassinate(ARegion *,Object *,Unit *);
    void AdjustCityMons( ARegion *pReg );
    void AdjustCityMon( ARegion *pReg, Unit *u );
	
    //
    // Month long orders
    //
    void RunMoveOrders();
    Location * DoAMoveOrder(Unit *,ARegion *,Object *);
    void DoMoveEnter(Unit *,ARegion *,Object **);
    void RunMonthOrders();
    void RunStudyOrders(ARegion *);
    void Do1StudyOrder(Unit *,Object *);
    void RunTeachOrders();
    void Do1TeachOrder(ARegion *,Unit *);
    void RunProduceOrders(ARegion *);
    int ValidProd(Unit *,ARegion *,Production *);
    int FindAttemptedProd(ARegion *,Production *);
    void RunAProduction(ARegion *,Production *);
    void RunUnitProduce(ARegion *,Unit *);
    void Run1BuildOrder(ARegion *,Object *,Unit *);
    void RunSailOrders();
    ARegion * Do1SailOrder(ARegion *,Object *,Unit *);
    void ClearCastEffects();
    void RunCastOrders();
    void RunACastOrder(ARegion *,Object *,Unit *);
    void RunTeleportOrders();
    void RemoveInactiveFactions();
    void Olding();
    void ClearKilledFars(Unit *);

    int GetItemBonus (Unit *, int);

    //
    // include spells.h for spell function declarations
    //
#define GAME_SPELLS
#include "spells.h"
#undef GAME_SPELLS

    //
    // Battle function
    //
    void KillDead(Location *);
    int RunBattle(ARegion *,Unit *,Unit *,int = 0,int = 0);
    void GetSides(ARegion *,AList &,AList &,AList &,AList &,Unit *,Unit *,
                  int = 0,int = 0);
    int CanAttack(ARegion *,AList *,Unit *);
    void GetAFacs(ARegion *,Unit *,Unit *,AList &,AList &,AList &);
    void GetDFacs(ARegion *,Unit *,AList &);
};


#endif
