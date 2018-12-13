#ifndef MAXSCRIPTOP_H
#define MAXSCRIPTOP_H


#include "util/TByte.h"
#include "util/TStream.h"
#include "util/TThingyTable.h"
#include <iostream>
#include <cstdlib>

namespace Nes {


struct MaxScriptOpData;
class MaxScriptRegions;

// Generic class for Metal Max script ops.
// I've forgone a class hierarchy, since I suspect creating it would waste a
// lot of time and make things much more complex without providing any real
// benefit.
class MaxScriptOp {
public:
  MaxScriptOp();
  MaxScriptOp(BlackT::TByte type__,
              BlackT::TByte param1__,
              BlackT::TByte param2__);
  
  bool operator==(const MaxScriptOp& src) const;
  bool operator<(const MaxScriptOp& src) const;
  
  void read(BlackT::TStream& ifs, bool* tilemapMode);
  void write(BlackT::TStream& ofs) const;
  
  void print(std::ostream& ofs,
             const BlackT::TThingyTable& thingy,
             const MaxScriptRegions* regions = NULL,
             bool recursive = false) const;
  
  bool isSpace() const;
  bool isTerminator() const;
  bool isDictionaryable() const;
  bool getIsRaw() const;

  BlackT::TByte type;
  BlackT::TByte param1;
  BlackT::TByte param2;
    
  const static int dictionaryRegion11 = 0x11;
  const static int dictionaryRegion13 = 0x13;
  const static int dictionaryRegion14 = 0x14;
  
  const static int opEnd          = 0x9F;
  const static int opE2           = 0xE2;
  const static int opTchoice      = 0xE3;
  const static int opE4           = 0xE4;
  const static int opBr           = 0xE5;
  const static int opE6           = 0xE6;
  const static int opBr2          = 0xE7;
  const static int opHero         = 0xE8;
  const static int opE9           = 0xE9;
  const static int opDic13        = 0xEA;
  const static int opChoice       = 0xEB;
  const static int opDic14        = 0xEC;
  const static int opSkip         = 0xED;
  const static int opSpeed        = 0xEE;
  const static int opEF           = 0xEF;
  const static int opSpkr         = 0xF0;
  const static int opF1           = 0xF1;
  const static int opF2           = 0xF2;
  const static int opDic11        = 0xF3;
  const static int opF4           = 0xF4;
  const static int opF5           = 0xF5;
  const static int opTilemap      = 0xF6;
  const static int opDic          = 0xF7;
  const static int opF8           = 0xF8;
  const static int opNvar         = 0xF9;
  const static int opFA           = 0xFA;
  const static int opFB           = 0xFB;
  const static int opIpSvar       = 0xFC;
  const static int opSvar         = 0xFD;
  const static int opKey          = 0xFE;
  const static int opSpace        = 0xFF;
  
  static MaxScriptOpData getOpData(BlackT::TByte type);
  static bool isCommandOp(BlackT::TByte type);
  static int numParams(BlackT::TByte type);
protected:

  const static BlackT::TByte commandOpStart = 0xE2;
  bool isRaw;
};

struct MaxScriptOpData {
  BlackT::TByte type;
  const char* name;
  int numParams;
  const char* printstr;
  bool postBreak;
};


}


#endif
