#include "max/MaxScriptOp.h"
#include "max/MaxScriptRegions.h"
#include "util/TStringConversion.h"
#include <cstring>
#include <iostream>

using namespace BlackT;

namespace Nes {


const static int tilemapBreakToken = 0x63;

const MaxScriptOpData op9FData
  = { MaxScriptOp::opEnd, "END", 0, "" };

const MaxScriptOpData opData[] = {
  { MaxScriptOp::opE2, "opE2", 0, "", false },
  { MaxScriptOp::opTchoice, "TCHOICE", 0, "", true },
  { MaxScriptOp::opE4, "opE4", 0, "", false },
  { MaxScriptOp::opBr, "BR", 0, "", true },
  { MaxScriptOp::opE6, "opE6", 0, "", false },
  { MaxScriptOp::opBr2, "BR2", 0, "", true },
  { MaxScriptOp::opHero, "HERO", 0, "", false },
  { MaxScriptOp::opE9, "opE9", 0, "", false },
  { MaxScriptOp::opDic13, "DIC13", 1, "", false },
  { MaxScriptOp::opChoice, "CHOICE", 2, "", true },
  { MaxScriptOp::opDic14, "DIC14", 1, "", false },
  { MaxScriptOp::opSkip, "SKIP", 1, "", false },
  { MaxScriptOp::opSpeed, "SPEED", 1, "", false },
  { MaxScriptOp::opEF, "opEF", 2, "", false },
  { MaxScriptOp::opSpkr, "SPKR", 1, "\n// [SPKR]", true },
  { MaxScriptOp::opF1, "opF1", 1, "", false },
  { MaxScriptOp::opF2, "opF2", 1, "", false },
  { MaxScriptOp::opDic11, "DIC11", 1, "", false },
  { MaxScriptOp::opF4, "opF4", 1, "", false },
  { MaxScriptOp::opF5, "opF5", 1, "", false },
  { MaxScriptOp::opTilemap, "TILEMAP", 0, "", false },
  { MaxScriptOp::opDic, "DIC", 2, "", false },
  { MaxScriptOp::opF8, "opF8", 2, "", false },
  { MaxScriptOp::opNvar, "NVAR", 2, "", false },
  { MaxScriptOp::opFA, "opFA", 1, "", false },
  { MaxScriptOp::opFB, "opFB", 1, "", false },
  { MaxScriptOp::opIpSvar, "IP_SVAR", 1, "", false },
  { MaxScriptOp::opSvar, "SVAR", 1, "", false },
  { MaxScriptOp::opKey, "KEY", 0, "", true },
  { MaxScriptOp::opSpace, "SPACE", 0, "", false }
};

MaxScriptOp::MaxScriptOp()
  : type(0),
    param1(0),
    param2(0),
    isRaw(false) { }

MaxScriptOp::MaxScriptOp(BlackT::TByte type__,
            BlackT::TByte param1__,
            BlackT::TByte param2__)
  : type(type__),
    param1(param1__),
    param2(param2__) { }
  
bool MaxScriptOp::operator==(const MaxScriptOp& src) const {
  if (type != src.type) return false;
  
  int numParams = getOpData(type).numParams;
  if ((numParams >= 1) && (param1 != src.param1)) return false;
  if ((numParams >= 2) && (param2 != src.param2)) return false;
  
  return true;
}

bool MaxScriptOp::operator<(const MaxScriptOp& src) const {
  if (type < src.type) return true;
  else if (type > src.type) return false;
  
  int numParams = getOpData(type).numParams;
  if ((numParams >= 1)) {
    if (param1 < src.param1) return true;
    else if (param1 > src.param1) return false;
  }
  if ((numParams >= 2)) {
    if (param2 < src.param2) return true;
    else if (param2 > src.param2) return false;
  }
  
  if (isRaw != src.isRaw) return true;
  
  return false;
}

void MaxScriptOp::read(BlackT::TStream& ifs, bool* tilemapMode) {
  type = ifs.readu8();
  
  if (*tilemapMode) {
    // n.b. 0x63 terminates tilemap mode, but _not_ the script;
    // 0x9F terminates both
    if (type == opEnd
        || (type == tilemapBreakToken)) {
      *tilemapMode = false;
    }
    else {
      isRaw = true;
      
      // 0x43 == dictionary lookup in region 9
      if (type == 0x43) {
        param1 = ifs.readu8();
      }
      // 0x8C == fill (param1) bytes with (param2)
      else if (type == 0x8C) {
        param1 = ifs.readu8();
        param2 = ifs.readu8();
      }
      // 0x9E == skip (param1) bytes
      else if (type == 0x9E) {
        param1 = ifs.readu8();
      }
      
      return;
    }
  }
  
  int params = numParams(type);
  
  if (params >= 1) {
    param1 = ifs.readu8();
  }
  
  if (params >= 2) {
    param2 = ifs.readu8();
  }
  
  if (type == opTilemap) *tilemapMode = true;
}

void MaxScriptOp::write(BlackT::TStream& ofs) const {
  ofs.writeu8(type);
  
  if (isRaw) {
    // 0x43 == dictionary lookup in region 9
    if (type == 0x43) {
      ofs.writeu8(param1);
    }
    // 0x8C == fill (param1) bytes with (param2)
    else if (type == 0x8C) {
      ofs.writeu8(param1);
      ofs.writeu8(param2);
    }
    // 0x9E == skip (param1) bytes
    else if (type == 0x9E) {
      ofs.writeu8(param1);
    }
    
    return;
  }
  
  int params = numParams(type);
  
  if (params >= 1) {
    ofs.writeu8(param1);
  }
  
  if (params >= 2) {
    ofs.writeu8(param2);
  }
}

std::string toRawHex(int value) {
  std::string result = "<$";
  result += ((value < 0x10) ? "0" : "");
  result += TStringConversion::intToString(value, TStringConversion::baseHex)
              .substr(2, std::string::npos);
  result += ">";
  return result;
}

void MaxScriptOp::print(std::ostream& ofs,
                        const BlackT::TThingyTable& thingy,
                        const MaxScriptRegions* regions,
                        bool recursive) const {
  if (isRaw) {
    ofs << toRawHex(type);
      
    // 0x43 == dictionary lookup in region 9
    if (type == 0x43) {
      ofs << toRawHex(param1);
    }
    // 0x8C == fill (param1) bytes with (param2)
    else if (type == 0x8C) {
      ofs << toRawHex(param1);
      ofs << toRawHex(param2);
    }
    // 0x9E == skip (param1) bytes
    else if (type == 0x9E) {
      ofs << toRawHex(param1);
    }
    
    return;
  }
  
  if (!isCommandOp(type)) {
    ofs << thingy.getEntry(type);
    return;
  }
  
  MaxScriptOpData data = getOpData(type);
  
  // don't print terminator messages for recursive prints
  if (recursive && (type == opEnd)) return;
  
  // dictionary lookup
  int dictionaryLookupReg = -1;
  int dictionaryLookupScript = -1;
  switch (type) {
  case opDic11:
    dictionaryLookupScript = param1;
    dictionaryLookupReg = 0x11;
    break;
  case opDic13:
    dictionaryLookupScript = param1;
    dictionaryLookupReg = 0x13;
    break;
  case opDic14:
    dictionaryLookupScript = param1;
    dictionaryLookupReg = 0x14;
    break;
  case opDic:
    dictionaryLookupScript = param1;
    dictionaryLookupReg = param2;
    break;
  default:
    
    break;
  }
  
  if ((dictionaryLookupReg != -1)
      && (regions != NULL)) {
    if (dictionaryLookupReg >= regions->regions.size()) {
      std::cerr << "bad lookup region " << dictionaryLookupReg << std::endl;
      return;
    }
    
//    const MaxScriptRegion& scripts
//      = regions->regions.at(dictionaryLookupReg);
    const MaxScriptRegion& scripts
      = regions->regions.find(dictionaryLookupReg)->second;
    if (dictionaryLookupScript >= scripts.size()) {
      std::cerr << "bad lookup address " << dictionaryLookupScript
        << std::endl;
      return;
    }
    
    scripts[dictionaryLookupScript].print(ofs, thingy, regions, true);
    return;
  }
  
  if (strlen(data.printstr) != 0) {
    ofs << data.printstr;
  }
  else {
    ofs << "[" << data.name << "]";
  }
  
  if (data.numParams >= 1) {
    ofs << "<$"
      << ((param1 < 0x10) ? "0" : "")
      << TStringConversion::intToString(param1, TStringConversion::baseHex)
            .substr(2, std::string::npos)
      << ">";
  }
  
  if (data.numParams >= 2) {
    ofs << "<$"
      << ((param2 < 0x10) ? "0" : "")
      << TStringConversion::intToString(param2, TStringConversion::baseHex)
            .substr(2, std::string::npos)
      << ">";
  }
  
  if (data.postBreak) {
    ofs << std::endl;
//    ofs << "// ";
  }
}

bool MaxScriptOp::isSpace() const {
  if ((type == opSpace)
      || (type == opBr)
      || (type == opBr2)
      || (type == opKey)
      || (type == opSpkr)
      || (type == opSkip)) return true;
  
  return false;
}
  
bool MaxScriptOp::isTerminator() const {
  if (isRaw) return false;

  if ((type == opEnd) || (type == opChoice)
      || (type == opTchoice)) return true;
  
  return false;
}
  
bool MaxScriptOp::isDictionaryable() const {
  if (isRaw
        || (type == opTilemap)
        || (type == tilemapBreakToken)
        || isTerminator()) return false;
  
  return true;
}

MaxScriptOpData MaxScriptOp::getOpData(BlackT::TByte type) {
  if (type == opEnd) return op9FData;
  else return opData[type - commandOpStart];
}

bool MaxScriptOp::isCommandOp(BlackT::TByte type) {
  if ((type == opEnd)
      || ((type >= commandOpStart)
          // spaces are never handled as ops
          && (type != opSpace))) return true;
  return false;
}
  
int MaxScriptOp::numParams(BlackT::TByte type) {
  if (!isCommandOp(type)) return 0;
  return getOpData(type).numParams;
}

bool MaxScriptOp::getIsRaw() const {
  return isRaw;
}


}
