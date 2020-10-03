// Supported with union (c) 2020 Union team
// Union HEADER file

namespace GOTHIC_ENGINE {
  /*bool operator > ( const zSTRING& l_, const zSTRING& r_ ) {
    zSTRING l = l_; l.Upper();
    zSTRING r = r_; r.Upper();

    uint Length = min( l.Length(), r.Length() );
    for( uint i = 0; i < Length; i++ )
      if( l[i] != r[i] )
        return (byte&)l[i] >( byte& )r[i];

    return l.Length() > r.Length();
  }

  bool operator < ( const zSTRING& l_, const zSTRING& r_ ) {
    zSTRING l = l_; l.Upper();
    zSTRING r = r_; r.Upper();

    uint Length = min( l.Length(), r.Length() );
    for( uint i = 0; i < Length; i++ )
      if( l[i] != r[i] )
        return (byte&)l[i] < (byte&)r[i];

    return l.Length() < r.Length();
  }*/





  static CMutex ReleaseMutex;

  template<int DELAY, class T>
  void DelayedRelease( T object );

  template<int DELAY, class T>
  void DelayedReleaseAsync( T object ) {
    Sleep( DELAY );
    ReleaseMutex.Enter();
    object->Release();
    // cmd << "Object released!" << endl;
    ReleaseMutex.Leave();
  }

  template<int DELAY, class T>
  void DelayedRelease( T object ) {
    auto asyncProc = DelayedReleaseAsync<DELAY, T>;
    CThread thread( asyncProc );
    thread.Detach( object );
  }
}