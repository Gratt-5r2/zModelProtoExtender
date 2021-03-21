// Supported with union (c) 2020 Union team
// Union SOURCE file

namespace GOTHIC_ENGINE {
  static MapArray<zCModelPrototype*, zCModelPrototype*> ModelProtoUniqueList;
  static zCModel* CurrentModel = Null;



  HOOK Ivk_zCModelPrototype_SearchAniIndex PATCH( &zCModelPrototype::SearchAniIndex, &zCModelPrototype::SearchAniIndex_Union );

  int zCModelPrototype::SearchNewAniIndex_Union( const zSTRING& aniName ) const {
    if( protoAnis.GetNum() == 0 )
      return 0;

    uint l = 0;
    uint r = protoAnis.GetNum();

    while( true ) {
      int pivot = (l + r) / 2;

      int dim = strcmp( aniName, protoAnis[pivot]->GetAniName() );
      if( dim == 0 )
        return pivot;

      bool upper = dim > 0;

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



  static int PushExternalAni( zCModelPrototype* baseProto, zCModelAni* ani ) {
    if( !baseProto )
      return Invalid;

    auto pair = ModelProtoUniqueList[baseProto];
    if( pair.IsNull() )
      return Invalid;

    // Find the best indes for new animation in the list
    int aniIndex    = baseProto->SearchNewAniIndex_Union( ani->aniName );
    auto& protoList = pair.GetValues();

    for( uint i = 0; i < protoList.GetNum(); i++ ) {
      zCModelPrototype* proto = protoList[i];
      auto& aniList = proto->protoAnis;

      if( i == 0 ) {
        // For first object in the list (base prototype)
        // include a new animation directly.
        aniList.InsertAtPos( ani, aniIndex );
        ani->AddRef();
      }
      else
        aniList.InsertAtPos( Null, aniIndex );

      // Update animation indexes by new
      // position in the list. Must change
      // only right objects of new inedx.
      for( int j = aniIndex; j < aniList.GetNum(); j++ )
        if( aniList[j] )
          aniList[j]->aniID = j;
    }

    return aniIndex;
  }



  HOOK Ivk_zCModelPrototype_PrepareAsModelProtoOverlay PATCH( &zCModelPrototype::PrepareAsModelProtoOverlay, &zCModelPrototype::PrepareAsModelProtoOverlay_Union );

  void oCAniCtrl_Human::CopyAniIndexes( oCAniCtrl_Human* other ) {
    static int length = (int)&dummyLastVar - (int)&s_dead1;
    memcpy( &s_dead1, &other->s_dead1, length );
  }



  void zCModelPrototype::UpdateNpcsAniCtrl( zCModelPrototype* baseProto ) {
    oCAniCtrl_Human* baseAniCtrl = Null;

    auto* list = ogame->GetGameWorld()->voblist_npcs->next;
    while( list ) {

      // At first we need to find
      // Npcs with current base prototype.
      oCNpc* npc = list->data;
      zCModel* model = npc->GetModel();
      if( model && model->modelProtoList[0] == baseProto ) {

        // For next need to update
        // all animation indexes in HumanAI.
        if( npc->anictrl ) {
          if( !baseAniCtrl ) {

            // For first Npc in loop,
            // calculate new indexes by method.
            npc->anictrl->InitAnimations();
            baseAniCtrl = npc->anictrl;
          }
          else
            // For other Npcs indexes
            // can be copied directly.
            npc->anictrl->CopyAniIndexes( baseAniCtrl );
        }
      }

      list = list->next;
    }
  }



  int zCModelPrototype::PrepareAsModelProtoOverlay_Union( zCModelPrototype* baseModelProto ) {
    if( !baseModelProto )
      return False;

    // For including new animations in run-time, need
    // to add new animation key in all overlays by
    // base prototype. For quick access to all overlays
    // we will map it by base prototypes. Base
    // prototypes includes in Map as Key object.
    if( ModelProtoUniqueList[baseModelProto].IsNull() )
      ModelProtoUniqueList.Insert( baseModelProto, baseModelProto );

    // This list must have similar animation layout
    // as baseProto. That mean all animation indexes
    // should be identical as parent prototype.
    // Not included animations to overlay marked as Null.
    zCArraySort<zCModelAni*> aniListEquals( baseModelProto->protoAnis.GetNum() );
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
        // Invalid index means that ani object is a New animation.
        // Need to add it to all prototypes by the current base proto.
        int newIndex = PushExternalAni( baseModelProto, ani );
        if( newIndex != Invalid ) {
          aniListEquals.InsertAtPos( ani, newIndex );
          UpdateAniCtrl = true;
        }
      }
    }

    // Overlays are added to list of its parent prototype.
    // If added new animations, then need to update
    // animation indexes in HumanAI for all Npcs
    // which have same base prototypes.
    ModelProtoUniqueList.Insert( baseModelProto, this );
    if( UpdateAniCtrl && CurrentModel )
      UpdateNpcsAniCtrl( baseModelProto );

    auto oldCompare = protoAnis.GetCompare();
    protoAnis = aniListEquals;
    protoAnis.SetCompare( oldCompare );
    this->baseModelProto = baseModelProto;
    return True;
  }



  static string MDS = ".MDS";
  static string MSB = ".MSB";

  Array<string> GetPrototypeFileList() {
    Array<string> protoFileList;

    char** files = Null;
    long numInList = vdf_filelist_virtual( files );
    for( long i = 0; i < numInList; i++ ) {
      string file = files[i];
      if( file.EndWith( MDS ) || file.EndWith( MSB ) )
        protoFileList |= file.GetPattern( "\\", ".", -1 );
    }

    delete[] files;
    files = Null;
    numInList = vdf_filelist_physical( files );
    for( long i = 0; i < numInList; i++ ) {
      string file = files[i];
      if( file.EndWith( MDS ) || file.EndWith( MSB ) )
        protoFileList |= file.GetPattern( "\\", ".", -1 );
    }

    delete[] files;
    return protoFileList;
  }



  Array<string> GetPrototypeFileListChilds( zCModelPrototype* proto ) {
    static Array<string> protoFileList = GetPrototypeFileList();
    Array<string> childFileList;

    string start = string::Combine( "%z.",  proto->modelProtoName );
    for( uint i = 0; i < protoFileList.GetNum(); i++ )
      if( protoFileList[i].StartWith( start ) && protoFileList[i] != (string&)proto->modelProtoName )
        childFileList += protoFileList[i];
    
    return childFileList;
  }



  void zCModelPrototype::CopyAnimations( zCModelPrototype* proto ) {
    if( protoAnis.GetNum() != proto->protoAnis.GetNum() )
      return;

    for( int i = 0; i < protoAnis.GetNum(); i++ ) {
      zCModelAni*& aniOld = protoAnis[i];
      zCModelAni*& aniNew = proto->protoAnis[i];

      if( aniNew == Null )
        continue;
      
      if( aniOld != Null )
        aniOld->Release();

      aniOld = aniNew;
      aniNew->AddRef();
    }
  }
  


  HOOK Hook_zCModelPrototype_Load PATCH( &zCModelPrototype::Load, &zCModelPrototype::Load_Union );

  zCModelPrototype* zCModelPrototype::Load_Union( zSTRING const& protoName, zCModelPrototype* baseProto ) {
    zCModelPrototype* proto = Hook_zCModelPrototype_Load(protoName, baseProto);
    if( proto ) {

      // Check child prototype files
      Array<string> childs = GetPrototypeFileListChilds( proto );
      for( uint i = 0; i < childs.GetNum(); i++ ) {

        // Try to load child prototype
        zCModelPrototype* childProto = Hook_zCModelPrototype_Load( childs[i] + ".MDS", proto );
        if( childProto ) {
          proto->CopyAnimations( childProto );
          if( childProto->refCtr > 1 )
            childProto->Release(); // FIXME
        }
      }
    }

    return proto;
  }



  HOOK Hook_zCModelMeshLib_LoadMDM PATCH_IF( &zCModelMeshLib::LoadMDM, &zCModelMeshLib::LoadMDM_Union, false );

  int zCModelMeshLib::LoadMDM_Union( zCFileBIN& file, zCModelPrototype* proto, zCModel* model, zCModelMeshLib** meshLib ) {
    int success = Hook_zCModelMeshLib_LoadMDM( file, proto, model, meshLib );
    return success;
  }
}