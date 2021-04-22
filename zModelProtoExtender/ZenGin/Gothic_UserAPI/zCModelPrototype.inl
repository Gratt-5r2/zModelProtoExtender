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
void EqualizeNodeListToProto( zCModelPrototype* );
bool NodeListsIsEqual( zCModelPrototype* );
zCModelPrototype* GetRootModelProto();
int Release_Union();
#if ENGINE >= Engine_G2
int ReadModelMSB_Union( zCFileBIN& file );
#else
void ReadModel_Union();
#endif
void Clear_Union();

static zCModelPrototype* Load_Union2( zSTRING const&, zCModelPrototype* );


struct TDelayedReleaseContext {
  zCModelPrototype* Proto;
  uint StartTime;
  uint ReleasesCount;
};

static Array<TDelayedReleaseContext> DelayedReleaseQueue;
static void UpdateDelayedReleaseQueue();
void DelayedRelease();
void DeleteFromDelayedReleaseQueue();