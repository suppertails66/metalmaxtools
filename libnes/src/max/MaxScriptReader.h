#ifndef MAXSCRIPTREADER_H
#define MAXSCRIPTREADER_H


#include "util/TStream.h"
#include "util/TThingyTable.h"
#include <string>
#include <vector>

namespace Nes {


class MaxScriptReader {
public:
  MaxScriptReader(BlackT::TStream& src__,
                  BlackT::TStream& dst__,
                  const BlackT::TThingyTable& thingy__);
  
  void operator()();
protected:
  struct ThingyValueAndKey {
    std::string value;
    int key;
    
    bool operator<(const ThingyValueAndKey& src) const {
      return (value.size() > src.value.size());
    }
  };

  BlackT::TStream& src;
  BlackT::TStream& dst;
  BlackT::TThingyTable thingy;
  std::vector<ThingyValueAndKey> thingiesBySize;
  int lineNum;
  
  void outputNextSymbol(BlackT::TStream& ifs);
  
  bool checkSymbol(BlackT::TStream& ifs, std::string& symbol);
  
  void processDirective(BlackT::TStream& ifs);
  void processLoadTable(BlackT::TStream& ifs);
  
  void loadThingy(const BlackT::TThingyTable& thingy__);
  
  // parse functions
  void skipSpace(BlackT::TStream& ifs) const;
  bool checkString(BlackT::TStream& ifs) const;
  bool checkInt(BlackT::TStream& ifs) const;
  std::string matchString(BlackT::TStream& ifs) const;
  int matchInt(BlackT::TStream& ifs) const;
  void matchChar(BlackT::TStream& ifs, char c) const;
  std::string matchName(BlackT::TStream& ifs) const;
  
  std::string getRemainingContent(BlackT::TStream& ifs) const;
  
};


}


#endif
