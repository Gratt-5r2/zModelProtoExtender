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







  MapArray<zCModelPrototype*, zCModelPrototype*> ModelProtoUniqueList;



  static int PushExternalAni( zCModelAni* ani ) {
    if( !CurrentModel )
      return Invalid;

    zCArray<zCModelPrototype*>& protoList = CurrentModel->modelProtoList;
    if( protoList.GetNum() == 0 )
      return Invalid;

    // push ani proto to base ani list
    zCModelPrototype* baseProto = protoList[0];
    int newAniIndex = baseProto->SearchNewAniIndex_Union( ani->aniName );
    baseProto->protoAnis.InsertAtPos( ani, newAniIndex );
    ani->AddRef();

    auto& childs = ModelProtoUniqueList[baseProto];
    if( !childs.IsNull() ) {

      // push nullptr to childs prototypes
      for( uint i = 0; i < childs.GetNum(); i++ ) {
        zCArraySort<zCModelAni*>& protoAnis = childs[i]->protoAnis;
        if( i != 0 )
          protoAnis.InsertAtPos( Null, newAniIndex );

        // update next animations after current
        for( int j = newAniIndex; j < protoAnis.GetNum(); j++ )
          if( protoAnis[j] )
            protoAnis[j]->aniID = j;
      }
    }

    return newAniIndex;
  }






  HOOK Ivk_zCModelPrototype_PrepareAsModelProtoOverlay AS( &zCModelPrototype::PrepareAsModelProtoOverlay, &zCModelPrototype::PrepareAsModelProtoOverlay_Union );



  void oCAniCtrl_Human::SetAniIDs( oCAniCtrl_Human* other ) {
    static int length = (int)&dummyLastVar - (int)&s_dead1;
    memcpy( &s_dead1, &other->s_dead1, length );
  }



  void zCModelPrototype::UpdateNpcsAniCtrl() {
    oCNpc* baseNpc               = CurrentModel->homeVob->CastTo<oCNpc>();
    zCModelPrototype* baseProto  = CurrentModel->modelProtoList[0];
    oCAniCtrl_Human* baseAniCtrl = baseNpc->anictrl;
    if( !baseAniCtrl )
      return;

    baseAniCtrl->Init( baseNpc );

    auto* list = ogame->GetGameWorld()->voblist_npcs->next;
    while( list ) {
      oCNpc* otherNpc = list->data;
      oCAniCtrl_Human* otherAniCtrl = otherNpc->anictrl;

      // Check animation controller for other npc.
      // If its null - update not needed.
      if( otherNpc != baseNpc && otherAniCtrl ) {
        zCModelPrototype* otherProto = otherNpc->GetModel()->modelProtoList[0];

        // Set animation IDs as equal
        // for identity NPC model
        if( otherProto == baseProto )
          otherAniCtrl->SetAniIDs( baseAniCtrl );
      }

      list = list->next;
    }
  }



  int zCModelPrototype::PrepareAsModelProtoOverlay_Union( zCModelPrototype* baseModelProto ) {
    if( !baseModelProto )
      return Invalid;

    if( ModelProtoUniqueList[baseModelProto].IsNull() )
      ModelProtoUniqueList.Insert( baseModelProto, baseModelProto );

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

    ModelProtoUniqueList.Insert( baseModelProto, this );
    if( UpdateAniCtrl && CurrentModel )
      UpdateNpcsAniCtrl();

    protoAnis = aniListEquals;
    this->baseModelProto = baseModelProto;
    return True;
  }





  HOOK Hook_zCModelMeshLib_LoadMDM AS( &zCModelMeshLib::LoadMDM, &zCModelMeshLib::LoadMDM_Union );

  int zCModelMeshLib::LoadMDM_Union( zCFileBIN& file, zCModelPrototype* proto, zCModel* model, zCModelMeshLib** meshLib ) {
    return Hook_zCModelMeshLib_LoadMDM( file, proto, model, meshLib );
  }
}