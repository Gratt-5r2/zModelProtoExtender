// Supported with union (c) 2020 Union team
// Union HEADER file

namespace GOTHIC_ENGINE {
  static CMutex ReleaseMutex;

  template<int DELAY, class T>
  void DelayedRelease( T object );

  template<int DELAY, class T>
  void DelayedReleaseAsync( T object ) {
    Sleep( DELAY );
    ReleaseMutex.Enter();
    object->Release();
    ReleaseMutex.Leave();
  }

  template<int DELAY, class T>
  void DelayedRelease( T object ) {
    auto asyncProc = DelayedReleaseAsync<DELAY, T>;
    CThread thread( asyncProc );
    thread.Detach( object );
  }
}