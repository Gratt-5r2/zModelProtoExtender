// Supported with union (c) 2020 Union team
// Union SOURCE file

namespace GOTHIC_ENGINE {
  static MapArray<zCModelPrototype*, zCModelPrototype*> ModelProtoUniqueList;
  static MapArray<zCModelPrototype*, zCModelPrototype*> InjectedProtoList;
  static zCModel* CurrentModel = Null;
  static zCModelPrototype* BaseModelProto = Null;
  static bool NeedToEqualateNodes = false;




  HOOK Ivk_zCModelPrototype_SearchAniIndex PATCH_IF( &zCModelPrototype::SearchAniIndex, &zCModelPrototype::SearchAniIndex_Union, 1 );

  int zCModelPrototype::SearchNewAniIndex_Union( const zSTRING& aniName ) const {
    auto& protoAnis = baseModelProto != Null ? baseModelProto->protoAnis : this->protoAnis;

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



  HOOK Ivk_zCModelPrototype_PrepareAsModelProtoOverlay PATCH_IF( &zCModelPrototype::PrepareAsModelProtoOverlay, &zCModelPrototype::PrepareAsModelProtoOverlay_Union, 1 );

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

    baseModelProto = baseModelProto->GetRootModelProto();

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

    // Find all MDS or MSB scripts
    // in virtual file list
    char** files = Null;
    long numInList = vdf_filelist_virtual( files );
    for( long i = 0; i < numInList; i++ ) {
      string file = files[i];
      if( file.EndWith( MDS ) || file.EndWith( MSB ) )
        protoFileList |= file.GetPattern( "\\", ".", -1 );
    }

    // Find all MDS or MSB scripts
    // in physical file list
    delete[] files;
    files = Null;
    numInList = vdf_filelist_physical( files );
    for( long i = 0; i < numInList; i++ ) {
      string file = files[i];
      if( file.EndWith( MDS ) || file.EndWith( MSB ) )
        protoFileList |= file.GetPattern( "\\", ".", -1 );
    }

    protoFileList.QuickSort();

    delete[] files;
    return protoFileList;
  }



  Array<string> GetPrototypeExternalFileList( zCModelPrototype* proto ) {
    static Array<string> protoFileList = GetPrototypeFileList();
    Array<string> childFileList;

    // Check and rerutn all file names which
    // prefix is equal to the source prototype name
    string start = string::Combine( "%z.",  proto->modelProtoName );
    for( uint i = 0; i < protoFileList.GetNum(); i++ )
      if( protoFileList[i].StartWith( start ) && protoFileList[i] != (string&)proto->modelProtoName )
        childFileList += protoFileList[i];

    return childFileList;
  }



  void zCModelPrototype::CopyAnimationsFrom( zCModelPrototype* proto ) {
    if( protoAnis.GetNum() != proto->protoAnis.GetNum() ) {
      cmd << Col16( CMD_RED | CMD_RED ) << "zModelProtoExtender: Bad copy anims: " << protoAnis.GetNum() << "  " << proto->protoAnis.GetNum() << " from " << proto->modelProtoName << Col16() << endl;
      return;
    }

    // Replace all exist animations
    // from external prototype to current.
    for( int i = 0; i < protoAnis.GetNum(); i++ ) {
      zCModelAni*& aniOld = protoAnis[i];
      zCModelAni*& aniNew = proto->protoAnis[i];
      zCModelAni*  aniTmp = Null;

      if( aniNew == Null )
        continue;

      // Move animation from new prototype
      // to current and reserve current
      // animation in new prototype
      aniTmp = aniOld;
      aniOld = aniNew;
      aniNew = aniTmp;
    }
  }
  


  HOOK Hook_zCModelPrototype_Load PATCH( &zCModelPrototype::Load, &zCModelPrototype::Load_Union );

  zCModelPrototype* zCModelPrototype::Load_Union( zSTRING const& protoName, zCModelPrototype* baseProto ) {
    zCModelPrototype* proto = zCModelPrototype::SearchName( protoName );
    if( proto ) {
      proto->AddRef();
      return proto;
    }

    BaseModelProto = baseProto;
    proto = Hook_zCModelPrototype_Load( protoName, baseProto );
    BaseModelProto = Null;

    if( proto )
      proto->InjectExternalModelProtoList();

    return proto;
  }



  HOOK Hook_zCModelPrototype_Release PATCH( &zCModelPrototype::Release, &zCModelPrototype::Release_Union );

  int zCModelPrototype::Release_Union() {
    if( --refCtr > 0 )
      return refCtr;

    auto& injectedList = InjectedProtoList[this];
    if( !injectedList.IsNull() ) {
      for each( zCModelPrototype* proto in injectedList )
        delete proto;
    }

    InjectedProtoList.Remove( this );

    refCtr = 1;
    DeleteFromDelayedReleaseQueue();
    delete this;
    return 0;
  }


  zCModelPrototype* zCModelPrototype::InjectExternalModelProto( const zSTRING& protoName ) {
    BaseModelProto               = this;
    NeedToEqualateNodes          = true;
    zCModelPrototype* childProto = Hook_zCModelPrototype_Load( protoName, this );
    NeedToEqualateNodes          = false;
    BaseModelProto               = Null;
    return childProto;
  }



  void zCModelPrototype::InjectExternalModelProtoList() {
    // Collect all model prototypes which prefix
    // is equal to the name of this model prototype.
    Array<string> childs = GetPrototypeExternalFileList( this );

    // Load all external prototypes. Replace current
    // node positions and animations to a new
    // objects from external prototypes.
    for( uint i = 0; i < childs.GetNum(); i++ ) {
      zCModelPrototype* childProto = InjectExternalModelProto( childs[i] + ".MDS" );
      if( childProto ) {
        CopyAnimationsFrom( childProto );
        InjectedProtoList.Insert( this, childProto );
      }
    }
  }



  bool zCModelPrototype::NodeListsIsEqual( zCModelPrototype* baseProto ) {
    if( nodeList.GetNum() != baseProto->nodeList.GetNum() )
      return false;

    for( int i = 0; i < nodeList.GetNum(); i++ )
      if( nodeList[i]->nodeName != baseProto->nodeList[i]->nodeName )
        return false;

    return true;
  }



  static bool operator == ( zCModelNode* const& node, zSTRING const& nodeName ) {
    return node->nodeName == nodeName;
  }



  static void CopyNodeProperties( zCModelNode* dst, zCModelNode* src ) {
    dst->nodeName        = src->nodeName;
    dst->trafo           = src->trafo;
    dst->trafoObjToWorld = src->trafoObjToWorld;
    dst->translation     = src->translation;
  }



  void zCModelPrototype::EqualizeNodeListToProto( zCModelPrototype* sourceProto ) {
    if( !NeedToEqualateNodes || !sourceProto || NodeListsIsEqual( sourceProto ) )
      return;
    
    // Based on the node list, a new הרûו should be created
    // as backward compatible, which starts with source 
    // nodes and ends with injected nodes. That will allow
    // to use only the necessary nodes in animations.
    Array<zCModelNode*> injectedNodes( nodeList.GetArray(), nodeList.GetNum() );
    Array<zCModelNode*> sourceNodes( sourceProto->nodeList.GetArray(), sourceProto->nodeList.GetNum() );
    Array<zCModelNode*> compatibleNodeList;
    zCModelNode* rootNode = sourceNodes[0];

    for( uint i = 0; i < sourceNodes.GetNum(); i++ ) {
      uint index = injectedNodes.SearchEqual( sourceNodes[i]->nodeName );
      if( index != Invalid ) {
        zCModelNode* node   = sourceNodes[i];
        compatibleNodeList += node;
        CopyNodeProperties( node, injectedNodes[index] );
        injectedNodes.RemoveAt( index );
      }
      else {
        zCModelNode* node   = new zCModelNode();
        node->parentNode    = rootNode;
        compatibleNodeList += node;
        CopyNodeProperties( node, sourceNodes[i] );
      }

      sourceNodes.RemoveAt( i-- );
    }

    // Remaining nodes need to add
    // to the backward compatibility.
    // compatibleNodeList += injectedNodes;
    for( uint i = 0; i < injectedNodes.GetNum(); i++ ) 		{
      zCModelNode* node   = new zCModelNode();
      node->parentNode    = rootNode;
      compatibleNodeList += node;
      CopyNodeProperties( node, injectedNodes[i] );
    }

    injectedNodes.Clear();

    sourceProto->nodeList.DeleteList();
    for( uint i = 0; i < compatibleNodeList.GetNum(); i++ )
      sourceProto->nodeList.Insert( compatibleNodeList[i] );

    // Update animation node indexes. That indexes
    // sould be in the source node list range.
    for( int aniID = 0; aniID < protoAnis.GetNum(); aniID++ ) {
      zCModelAni* ani = protoAnis[aniID];

      for( int nodeID = 0; nodeID < ani->numNodes; nodeID++ ) {
        zSTRING& nodeName = ani->nodeList[nodeID]->nodeName;
        int& nodeIndex = ani->nodeIndexList[nodeID];
        nodeIndex = compatibleNodeList.SearchEqual( nodeName );
      }
    }
  }



  zCModelPrototype* zCModelPrototype::GetRootModelProto() {
    return baseModelProto == Null ? this : baseModelProto->GetRootModelProto();
  }



#if ENGINE >= Engine_G2
  HOOK Hook_zCModelPrototype_ReadModelMSB PATCH( &zCModelPrototype::ReadModelMSB, &zCModelPrototype::ReadModelMSB_Union );

  int zCModelPrototype::ReadModelMSB_Union( zCFileBIN& file ) {
    int Ok = THISCALL( Hook_zCModelPrototype_ReadModelMSB )(file);

    // Equalization of node lists is needed so that when an
    // overlay is inserted with a different skeleton, bones
    // of the inserted one are adapt to bones of the original.
    // And then the indices of all animations are changed.
    EqualizeNodeListToProto( BaseModelProto );

    return Ok;
  }
#else
  HOOK Hook_zCModelPrototype_ReadModel PATCH( &zCModelPrototype::ReadModel, &zCModelPrototype::ReadModel_Union );

  void zCModelPrototype::ReadModel_Union() {
    THISCALL( Hook_zCModelPrototype_ReadModel )();

    // Equalization of node lists is needed so that when an
    // overlay is inserted with a different skeleton, bones
    // of the inserted one are adapt to bones of the original.
    // And then the indices of all animations are changed.
    EqualizeNodeListToProto( BaseModelProto );
  }
#endif



  Array<zCModelPrototype::TDelayedReleaseContext> zCModelPrototype::DelayedReleaseQueue;

  void zCModelPrototype::UpdateDelayedReleaseQueue() {
    static uint delay = 5000;
    uint now = Timer::GetTime();

    for( uint i = 0; i < DelayedReleaseQueue.GetNum(); i++ ) {
      auto context = DelayedReleaseQueue[i];
      if( now - context.StartTime < delay )
        continue;

      DelayedReleaseQueue.RemoveAt( i-- );
      string releasedProtoName = context.Proto->modelProtoName;
      int release;
      
      for( uint j = 0; j < context.ReleasesCount; j++ )
        release = context.Proto->Release();

      cmd << releasedProtoName << " was released " << release << endl;
    }
  }



  void zCModelPrototype::DelayedRelease() {
    uint now = Timer::GetTime();
    for( uint i = 0; i < DelayedReleaseQueue.GetNum(); i++ ) {
      auto& context = DelayedReleaseQueue[i];
      if( context.Proto == this ) {
        context.StartTime = now;
        context.ReleasesCount++;
        return;
      }
    }

    auto& context = DelayedReleaseQueue.Create();
    context.Proto = this;
    context.StartTime = now;
    context.ReleasesCount = 1;
  }



  void zCModelPrototype::DeleteFromDelayedReleaseQueue() {
    for( uint i = 0; i < DelayedReleaseQueue.GetNum(); i++ ) {
      if( DelayedReleaseQueue[i].Proto == this ) {
        cmd << "unrelease " << this->modelProtoName << endl;
        DelayedReleaseQueue.RemoveAt( i );
        return;
      }
    }
  }
}