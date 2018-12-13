#include "max/MaxDictMarker.h"
#include <iostream>

namespace Nes {


MaxDictMarker::MaxDictMarker()
  : id(0),
    recursionLevel(0) { }

MaxDictMarker::MaxDictMarker(int id__, int recursionLevel__)
  : id(id__),
    recursionLevel(recursionLevel__) { }
  
bool MaxDictMarker::operator==(const MaxDictMarker& src) const {
  // deliberately exclude recursion level from equality check:
  // we're concerned merely with whether the symbols are the same
  return ((id == src.id));
}

bool MaxDictMarker::operator<(const MaxDictMarker& src) const {
  if (id < src.id) return true;
  else if (id > src.id) return false;
  
  if (recursionLevel < src.recursionLevel) return true;
  else if (recursionLevel > src.recursionLevel) return false;
  
  return false;
}


}
