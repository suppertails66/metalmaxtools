#include "max/MaxScriptRegions.h"

namespace Nes {


MaxScriptRegions::MaxScriptRegions() { }

void MaxScriptRegions::addRegion(const MaxScriptRegion& scripts) {
//  regions.push_back(scripts);
  if (regions.size() == 0) {
    regions[0] = scripts;
    return;
  }
  
  RegionMap::iterator it = regions.end();
  --it;
  int num = it->first;
  regions[num + 1] = scripts;
}

const MaxScriptRegion& MaxScriptRegions::getRegion(int region) {
  return regions[region];
}


}
