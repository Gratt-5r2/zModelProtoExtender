// Supported with union (c) 2020 Union team

// User API for zCModel
// Add your methods here

int GetAniIDFromAniName_Union( zSTRING const& ) const;
int ApplyModelProtoOverlay_Union( const zSTRING& mdsFileName );
void RemoveModelProtoOverlay_Union( zCModelPrototype* );
void RemoveModelProtoOverlayByName_Union( zSTRING const& );
zCModelAniActive* GetActiveAni( const zSTRING& aniName );
bool ReplaceActiveAni( zCModelAni* ani );
void ActivateAdditionalAnis( zCModelPrototype* modelProto );
void DeactivateAdditionalAnis( zCModelPrototype* modelProto );
void CheckAndApplyModelContext( zCModelPrototype* modelProto );
void RenameModelAni( zCModelPrototype* modelProto, zSTRING oldName, zSTRING newName );