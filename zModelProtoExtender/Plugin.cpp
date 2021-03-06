// This file added in headers queue
// File: "Sources.h"

namespace GOTHIC_ENGINE {

  // TO DO
  // Your code ...
  
  void Game_Entry() {
#if ENGINE == Engine_G1
    PATCH_EXECUTE( HEX @0x0055E60B = 'EB 29' );
#endif
#if ENGINE == Engine_G1A
    PATCH_EXECUTE( HEX @0x0057699C = 'EB 29' );
#endif
#if ENGINE == Engine_G2
    PATCH_EXECUTE( HEX @0x00572B4A = 'EB 29' );
#endif
#if ENGINE == Engine_G2A
    PATCH_EXECUTE( HEX @0x00577DFA = 'EB 29' );
#endif
  }

  void Game_Init() {
  }

  void Game_Exit() {
  }

  void Game_Loop() {
#if 0
    static bool applied = false;
    if( zMouseKeyPressed( Right ) && !applied ) {
      applied = true;
      player->GetModel()->ApplyModelProtoOverlay( "HUMANS_SPRINT.MDS" );
    }
    else if( !zMouseKeyPressed( Right ) && applied ) {
      applied = false;
      player->GetModel()->RemoveModelProtoOverlay( "HUMANS_SPRINT.MDS" );
    }
#endif

    zCModelPrototype::UpdateDelayedReleaseQueue();
  }

  void Game_SaveBegin() {
  }

  void Game_SaveEnd() {
  }

  void LoadBegin() {
    IsInGame = false;
    ModelProtoUniqueList.Clear();
  }

  void LoadEnd() {
    IsInGame = true;
  }

  void Game_LoadBegin_NewGame() {
    LoadBegin();
  }

  void Game_LoadEnd_NewGame() {
    LoadEnd();
  }

  void Game_LoadBegin_SaveGame() {
    LoadBegin();
  }

  void Game_LoadEnd_SaveGame() {
    LoadEnd();
  }

  void Game_LoadBegin_ChangeLevel() {
    LoadBegin();
  }

  void Game_LoadEnd_ChangeLevel() {
    LoadEnd();
  }

  void Game_LoadBegin_Trigger() {
  }
  
  void Game_LoadEnd_Trigger() {
  }
  
  void Game_Pause() {
  }
  
  void Game_Unpause() {
  }
  
  void Game_DefineExternals() {
  }

  /*
  Functions call order on Game initialization:
    - Game_Entry           * Gothic entry point
    - Game_DefineExternals * Define external script functions
    - Game_Init            * After DAT files init
  
  Functions call order on Change level:
    - Game_LoadBegin_Trigger     * Entry in trigger
    - Game_LoadEnd_Trigger       *
    - Game_Loop                  * Frame call window
    - Game_LoadBegin_ChangeLevel * Load begin
    - Game_SaveBegin             * Save previous level information
    - Game_SaveEnd               *
    - Game_LoadEnd_ChangeLevel   *
  
  Functions call order on Save game:
    - Game_Pause     * Open menu
    - Game_Unpause   * Click on save
    - Game_Loop      * Frame call window
    - Game_SaveBegin * Save begin
    - Game_SaveEnd   *
  
  Functions call order on Load game:
    - Game_Pause              * Open menu
    - Game_Unpause            * Click on load
    - Game_LoadBegin_SaveGame * Load begin
    - Game_LoadEnd_SaveGame   *
  */

#define AppDefault True
  CApplication* lpApplication = !CHECK_THIS_ENGINE ? Null : CApplication::CreateRefApplication(
    Enabled( AppDefault ) Game_Entry,
    Enabled( AppDefault ) Game_Init,
    Enabled( AppDefault ) Game_Exit,
    Enabled( AppDefault ) Game_Loop,
    Enabled( AppDefault ) Game_SaveBegin,
    Enabled( AppDefault ) Game_SaveEnd,
    Enabled( AppDefault ) Game_LoadBegin_NewGame,
    Enabled( AppDefault ) Game_LoadEnd_NewGame,
    Enabled( AppDefault ) Game_LoadBegin_SaveGame,
    Enabled( AppDefault ) Game_LoadEnd_SaveGame,
    Enabled( AppDefault ) Game_LoadBegin_ChangeLevel,
    Enabled( AppDefault ) Game_LoadEnd_ChangeLevel,
    Enabled( AppDefault ) Game_LoadBegin_Trigger,
    Enabled( AppDefault ) Game_LoadEnd_Trigger,
    Enabled( AppDefault ) Game_Pause,
    Enabled( AppDefault ) Game_Unpause,
    Enabled( AppDefault ) Game_DefineExternals
  );
}