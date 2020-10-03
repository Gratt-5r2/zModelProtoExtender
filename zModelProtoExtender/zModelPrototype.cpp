// Supported with union (c) 2020 Union team
// Union SOURCE file

namespace GOTHIC_ENGINE {
  static zCModel* CurrentModel = Null;



  bool operator < ( const zSTRING& s1, const zSTRING& s2 ) {
    return strcmp( s1, s2 ) < 0; // (string&)s1 < (string)s2;
  }



  HOOK Ivk_zCModelPrototype_SearchAniIndex AS( &zCModelPrototype::SearchAniIndex, &zCModelPrototype::SearchAniIndex_Union );

  int zCModelPrototype::SearchNewAniIndex_Union( const zSTRING& aniName ) const {
    if( protoAnis.GetNum() == 0 )
      return 0;

    uint l = 0;
    uint r = protoAnis.GetNum();

    while( true ) {
      int pivot = (l + r) / 2;
      if( protoAnis[pivot]->aniName == aniName )
        return pivot;

      bool upper = protoAnis[pivot]->aniName < aniName;

      if( r - l <= 1 )
        return upper ? r : l;

      if( upper )
        l = pivot;
      else
        r = pivot;
    }
  }

  int zCModelPrototype::SearchAniIndex_Union( const zSTRING& aniName ) const {
    int index = SearchNewAniIndex_Union( aniName );
    if( index < protoAnis.GetNum() && protoAnis[index] && protoAnis[index]->aniName == aniName )
      return index;

    return Invalid;
  }







  static int PushExternalAni( zCModelAni* ani ) {
    if( !CurrentModel )
      return Invalid;

    zCArray<zCModelPrototype*>& protoList = CurrentModel->modelProtoList;
    if( protoList.GetNum() == 0 )
      return Invalid;

    // push ani proto to base ani list
    int newAniIndex = protoList[0]->SearchNewAniIndex_Union( ani->aniName );
    protoList[0]->protoAnis.InsertAtPos( ani, newAniIndex );
    ani->AddRef();

    // push nullptr to childs prototypes
    for( int i = 0; i < protoList.GetNum(); i++ ) {
      zCArraySort<zCModelAni*>& protoAnis = protoList[i]->protoAnis;
      if( i != 0 )
        protoAnis.InsertAtPos( Null, newAniIndex );

      // update next animations after current
      for( int j = newAniIndex; j < protoAnis.GetNum(); j++ )
        if( protoAnis[j] )
          protoAnis[j]->aniID = j;
    }

    return newAniIndex;
  }






  HOOK Ivk_zCModelPrototype_PrepareAsModelProtoOverlay AS( &zCModelPrototype::PrepareAsModelProtoOverlay, &zCModelPrototype::PrepareAsModelProtoOverlay_Union );

  int zCModelPrototype::PrepareAsModelProtoOverlay_Union( zCModelPrototype* baseModelProto ) {
    if( !baseModelProto )
      return Invalid;

    zCArraySort<zCModelAni*> aniListEquals;
    for( int i = 0; i < baseModelProto->protoAnis.GetNum(); i++ )
      aniListEquals.Insert( Null );

    bool UpdateAniCtrl = false;
    for( int i = 0; i < protoAnis.GetNum(); i++ ) {
      zCModelAni* ani = protoAnis[i];
      int index = baseModelProto->SearchAniIndex_Union( ani->aniName );

      if( index != Invalid ) {
        aniListEquals[index] = ani;
        aniListEquals[index]->aniID = index;
      }
      else {
        int newIndex = PushExternalAni( ani );
        if( newIndex != Invalid ) {
          aniListEquals.InsertAtPos( ani, newIndex );
          UpdateAniCtrl = true;
        }
      }
    }

    if( UpdateAniCtrl ) {
      oCNpc* npc = CurrentModel->homeVob->CastTo<oCNpc>();
      if( npc && npc->anictrl )
        npc->anictrl->Init( npc );
    }

    protoAnis = aniListEquals;
    this->baseModelProto = baseModelProto;
    return True;
  }





  HOOK Hook_zCModelMeshLib_LoadMDM AS( &zCModelMeshLib::LoadMDM, &zCModelMeshLib::LoadMDM_Union );

  int zCModelMeshLib::LoadMDM_Union( zCFileBIN& file, zCModelPrototype* proto, zCModel* model, zCModelMeshLib** meshLib ) {
    return Hook_zCModelMeshLib_LoadMDM( file, proto, model, meshLib );
  }
}