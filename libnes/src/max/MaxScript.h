#ifndef MAXSCRIPT_H
#define MAXSCRIPT_H


#include "max/MaxScriptOp.h"
#include "max/MaxDictCharOp.h"
#include "util/TStream.h"
#include <vector>
#include <map>
#include <iostream>

namespace Nes {


struct DictionaryReference {
  int region;
  int id;
};

class MaxScriptRegions;
typedef std::map<int, DictionaryReference> DictionaryReferenceMap;

class MaxScript {
public:
  typedef std::vector<MaxScriptOp> MaxScriptOpCollection;

  MaxScript();
  
  void read(BlackT::TStream& ifs);
  void write(BlackT::TStream& ofs) const;
  void print(std::ostream& ofs,
             const BlackT::TThingyTable& thingy,
             const MaxScriptRegions* regions = NULL,
             bool recursive = false) const;
  
  void toDictString(MaxDictStringOp& dst) const;
  void fromDictString(const MaxDictStringOp& src,
                      const DictionaryReferenceMap& refMap);
  
  MaxScriptOpCollection ops;
  
  int offset;
  
protected:
  
};


}


#endif
