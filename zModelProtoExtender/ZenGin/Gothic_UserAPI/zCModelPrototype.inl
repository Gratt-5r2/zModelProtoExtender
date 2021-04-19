// Supported with union (c) 2020 Union team

// User API for zCModelPrototype
// Add your methods here


int __fastcall SearchAniIndex_Union( const zSTRING& aniName ) const;
int __fastcall SearchNewAniIndex_Union( const zSTRING& aniName ) const;
int PrepareAsModelProtoOverlay_Union( zCModelPrototype* baseModelProto );
static zCModelPrototype* Load_Union( zSTRING const&, zCModelPrototype* );
zCModelPrototype* InjectExternalModelProto( const zSTRING& protoName );
void InjectExternalModelProtoList();
static zCModelPrototype* SearchName_Union( zSTRING );
static void UpdateNpcsAniCtrl( zCModelPrototype* baseProto );
void CopyAnimationsFrom( zCModelPrototype* proto );
int AddRef() { return ++refCtr; }
int ReadAniEnumMSB_Union( const int, zCFileBIN& );
void EquateNodeListFromProto( zCModelPrototype* );
bool NodeListsIsEqual( zCModelPrototype* );
zCModelPrototype* GetRootModelProto();
int Release_Union();
int ReadModelMSB_Union( zCFileBIN& file );

static zCModelPrototype* Load_Union2( zSTRING const&, zCModelPrototype* );