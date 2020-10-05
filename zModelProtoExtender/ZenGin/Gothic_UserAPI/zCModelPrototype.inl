// Supported with union (c) 2020 Union team

// User API for zCModelPrototype
// Add your methods here


int __fastcall SearchAniIndex_Union( const zSTRING& aniName ) const;
int __fastcall SearchNewAniIndex_Union( const zSTRING& aniName ) const;
int PrepareAsModelProtoOverlay_Union( zCModelPrototype* baseModelProto );
static zCModelPrototype* Load_Union( zSTRING const&, zCModelPrototype* );
static zCModelPrototype* SearchName_Union( zSTRING );
static void UpdateNpcsAniCtrl();
int AddRef() { return ++refCtr; }