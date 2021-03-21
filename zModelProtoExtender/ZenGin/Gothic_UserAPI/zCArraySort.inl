// Supported with union (c) 2020 Union team

// User API for zCArraySort
// Add your methods here

typedef int(*COMPARE)(const void*, const void*);
COMPARE GetCompare() {
  return Compare;
}