#ifndef MAXSCRIPTREGIONS_H
#define MAXSCRIPTREGIONS_H


#include "max/MaxScript.h"
#include <vector>
#include <map>

namespace Nes {


typedef std::vector<MaxScript> MaxScriptRegion;
typedef std::map< int, MaxScriptRegion > RegionMap;

class MaxScriptRegions {
public:
  MaxScriptRegions();
  
  void addRegion(const MaxScriptRegion& scripts);
  const MaxScriptRegion& getRegion(int region);
  
//  std::vector< MaxScriptRegion > regions;
  RegionMap regions;
protected:
  
};


}


#endif
