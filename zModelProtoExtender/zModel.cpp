// Supported with union (c) 2020 Union team
// Union SOURCE file

namespace GOTHIC_ENGINE {
  HOOK Hook_zCModel_GetAniIDFromAniName_Union AS( &zCModel::GetAniIDFromAniName, &zCModel::GetAniIDFromAniName_Union );

  int zCModel::GetAniIDFromAniName_Union( const zSTRING& aniName ) const {
    for( int i = 0; i < modelProtoList.GetNum(); i++ ) {
      zCModelAni* ani = modelProtoList[0]->SearchAni( aniName );
      if( ani )
        return ani->aniID;
    }

    return Invalid;
  }





  HOOK Hook_zCModel_ApplyModelProtoOverlay AS( &zCModel::ApplyModelProtoOverlay, &zCModel::ApplyModelProtoOverlay_Union );

  int zCModel::ApplyModelProtoOverlay_Union( const zSTRING& mdsFileName ) {
    if( modelProtoList.GetNum() == 0 )
      return False;

    CurrentModel = this;

    zCModelPrototype* modelProto = zCModelPrototype::Load( mdsFileName, modelProtoList[0] );

    // TO DO
    // CheckAndApplyModelContext( modelProto );

    bool_t Ok = False;
    if( modelProto ) {
      ActivateAdditionalAnis( modelProto );
      Ok = ApplyModelProtoOverlay( modelProto );
      modelProto->Release();
    }

    CurrentModel = Null;
    return Ok;
  };




  HOOK Hook_oCNpc_ApplyOverlay AS( &oCNpc::ApplyOverlay, &oCNpc::ApplyOverlay_Union );

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




  HOOK Hook_zCModel_RemoveModelProtoOverlay AS( &zCModel::RemoveModelProtoOverlay, &zCModel::RemoveModelProtoOverlay_Union );

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




  HOOK Hook_zCModel_RemoveModelProtoOverlayByName AS( &zCModel::RemoveModelProtoOverlay, &zCModel::RemoveModelProtoOverlayByName_Union );

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



  // TO DO
  // Add new animations with new names
  void zCModel::CheckAndApplyModelContext( zCModelPrototype* modelProto ) {
    string contextName = modelProto->modelProtoName + ".CONTEXT";
    string contextData;
    contextData.ReadFromVdf( contextName, VDF_DEFAULT );

    rowString contextRows = contextData;
    for( uint i = 0; i < contextRows.GetNum(); i++ ) {
      string& row = contextRows[i];
      Array<string> tokens = row.Split( "->" );

      if( tokens.GetNum() != 2 )
        Message::Fatal( "Invalid context syntax in " +  A i + " line.", contextName );

      RenameModelAni(
        modelProto,
        tokens[0].Shrink().Upper(),
        tokens[1].Shrink().Upper()
        );
    }
  }



  void zCModel::RenameModelAni( zCModelPrototype* modelProto, zSTRING oldName, zSTRING newName ) {
    int index = modelProto->SearchNewAniIndex_Union( oldName );
    if( index == Invalid )
      return;

    zCModelAni* ani = modelProto->protoAnis[index];
    if( ani )
      ani->aniName = newName;
  }
}