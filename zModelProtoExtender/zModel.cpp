// Supported with union (c) 2020 Union team
// Union SOURCE file

namespace GOTHIC_ENGINE {
  HOOK Hook_zCModel_GetAniIDFromAniName_Union PATCH( &zCModel::GetAniIDFromAniName, &zCModel::GetAniIDFromAniName_Union );

  int zCModel::GetAniIDFromAniName_Union( const zSTRING& aniName ) const {
    for( int i = 0; i < modelProtoList.GetNum(); i++ ) {
      zCModelAni* ani = modelProtoList[0]->SearchAni( aniName );
      if( ani )
        return ani->aniID;
    }

    return Invalid;
  }





  HOOK Hook_zCModel_ApplyModelProtoOverlay PATCH( &zCModel::ApplyModelProtoOverlay, &zCModel::ApplyModelProtoOverlay_Union );

  int zCModel::ApplyModelProtoOverlay_Union( const zSTRING& mdsFileName ) {
    if( modelProtoList.GetNum() == 0 )
      return False;

    CurrentModel = this;

    zCModelPrototype* modelProto = zCModelPrototype::Load( mdsFileName, modelProtoList[0] );

    bool_t Ok = False;
    if( modelProto ) {
      ActivateAdditionalAnis( modelProto );
      Ok = ApplyModelProtoOverlay( modelProto );
      modelProto->Release();
    }

    CurrentModel = Null;
    return Ok;
  };



  HOOK Hook_oCNpc_ApplyOverlay PATCH( &oCNpc::ApplyOverlay, &oCNpc::ApplyOverlay_Union );

  int oCNpc::ApplyOverlay_Union( const zSTRING& mdsFileName__ ) {

    string mdsFileName = mdsFileName__;
    mdsFileName.Upper();
    if( !mdsFileName.EndWith( ".MDS" ) )
      mdsFileName += ".MDS";

    if( visual && !GetModel()->ApplyModelProtoOverlay_Union( mdsFileName ) )
        return False;

    activeOverlays.Insert( mdsFileName );
    return True;
  }




  HOOK Hook_zCModel_RemoveModelProtoOverlay PATCH( &zCModel::RemoveModelProtoOverlay, &zCModel::RemoveModelProtoOverlay_Union );

  void zCModel::RemoveModelProtoOverlay_Union( zCModelPrototype* modelProto ) {
    CurrentModel = this;

    modelProto->AddRef();
    THISCALL( Hook_zCModel_RemoveModelProtoOverlay )(modelProto);
    DeactivateAdditionalAnis( modelProto );

    // Hmmm, this overlay will works a some
    // seconds for a 'soft' anis replacing.
    DelayedRelease<5000>( modelProto );
    CurrentModel = Null;
  }




  HOOK Hook_zCModel_RemoveModelProtoOverlayByName PATCH( &zCModel::RemoveModelProtoOverlay, &zCModel::RemoveModelProtoOverlayByName_Union );

  // So... This function has inline of
  // overload with 'zCModelPrototype*', fix it...
  void zCModel::RemoveModelProtoOverlayByName_Union( const zSTRING& fileName ) {
    zPATH filePath       = fileName;
    zSTRING realFileName = filePath.GetFilename();

    for( int i = 0; i < modelProtoList.GetNumInList(); i++ )
      if( modelProtoList[i]->modelProtoName == realFileName )
        return RemoveModelProtoOverlay_Union( modelProtoList[i] );
  }



  // 'Soft' overlay adding with
  // replacing all active animations
  // to new overlay anis
  void zCModel::ActivateAdditionalAnis( zCModelPrototype* modelProto ) {
    for( int i = 0; i < modelProto->protoAnis.GetNum(); i++ ) {
      zCModelAni* ani = modelProto->protoAnis[i];

      if( ani )
        if( !ReplaceActiveAni( ani ) )
          return StopAnisLayerRange( 0, 9999 );
    }
  }



  // 'Soft' overlay removing with
  // replacing all active ovrerlay
  // animations to bottom-level anis
  void zCModel::DeactivateAdditionalAnis( zCModelPrototype* modelProto ) {
    for( int i = 0; i < modelProto->protoAnis.GetNum(); i++ ) {
      zCModelAni* ani = modelProto->protoAnis[i];

      if( ani ) {
        zCModelAni* oldAni = GetAniFromAniID( i );
        if( !ReplaceActiveAni( oldAni ) )
          return StopAnisLayerRange( 0, 9999 );
      }
    }
  }



  // Find active animation by name
  zCModelAniActive* zCModel::GetActiveAni( const zSTRING& aniName ) {
    for( int i = 0; i < numActiveAnis; i++ )
      if( aniChannels[i]->protoAni->aniName == aniName )
        return aniChannels[i];
    
    return Null;
  }



  // Replace current active animation
  // to other animation with same name
  bool zCModel::ReplaceActiveAni( zCModelAni* ani ) {
    if( !ani )
      return false;

    zCModelAniActive* aniActive = GetActiveAni( ani->aniName );
    if( !aniActive )
      return true;

    if( !aniActive->isFadingOut ) {
      aniActive->isFirstTime     = True;
      aniActive->rotFirstTime    = True;
      aniActive->SetProgressPercent( 1.0f );
      aniActive->nextAniOverride = ani;
    }
    else
    {
      cmd << "Can't smoothing " << ani->aniName   << endl;
      return false;
    }

    return true;
  }



  HOOK Hook_zCModel_CopyProtoNodeList PATCH( &zCModel::CopyProtoNodeList, &zCModel::CopyProtoNodeList_Union );

  void zCModel::CopyProtoNodeList_Union() {
    if( modelProtoList.GetNum() <= 0 || !modelProtoList[0] )
      return;

    int num = modelProtoList[0]->nodeList.GetNum();
    Array<zCModelNodeInst*> preNodeList;

    for( int i = 0; i < num; i++ ) {
      zCModelNode* node = modelProtoList[0]->nodeList[i];
      zCModelNode* parent = node->parentNode;
      zCModelNodeInst* instNode = new zCModelNodeInst( node );
      if( parent )
        instNode->parentNode = SearchNode( parent->nodeName );

      if( instNode->protoNode->nodeName == "" )
        cmd << "EMPTY NODE NAME!!!" << endl;

      node->lastInstNode = instNode;
      nodeList.Insert( instNode );
    }
  }



  HOOK Hook_zCModel_Destructor AS( 0x00577CC0, &zCModel::Destructor );

  void zCModel::Destructor() {
    //
    for( int j = 0; j < modelProtoList.GetNumInList(); j++ ) {
      modelProtoList[j]->Release();
    };
    modelProtoList.DeleteList();

    m_listOfVoiceHandles.DeleteList();

    // ACHTUNG: Meshes gehoeren immer den MeshLibs und muessen deshalb nicht extra released werden !
    RemoveMeshLibAll();

    // SoftSkins
    for( int i = 0; i < meshSoftSkinList.GetNum(); i++ )
      meshSoftSkinList[i]->Release();
    meshSoftSkinList.DeleteList();

    // evtl. 'uebergebliebene' Meshes aus den Nodes entfernen
    for( int i = 0; i < nodeList.GetNum(); i++ )
      nodeList[i]->SetNodeVisualS( 0, 0 );

    //
    RemoveAllAniAttachments();

    //cmd << "#1" << endl;
    // Nodes loeschen (am Ende)
    //if( nodeList.GetNum() > 0 )
    //  delete[]( nodeList[0] );
    //for( int i = 0; i < nodeList.GetNum(); i++ ) {
    //  cmd << "  del " << nodeList[i]->protoNode->nodeName << endl;
    //  delete nodeList[i];
    //  nodeList[i] = 0;
    //}
    //nodeList.DeleteList();
    //cmd << "#2" << endl;

    //
    delete[] aniHistoryList; aniHistoryList = 0;
    delete[] activeAniList; activeAniList = 0;
    activeAniList = 0;
    homeVob = 0;
    numActiveAnis = 0;
  };
}