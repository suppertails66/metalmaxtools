#include "max/MaxScript.h"
#include "max/MaxScriptRegions.h"
#include "util/TStringConversion.h"
#include "exception/TGenericException.h"

using namespace BlackT;

namespace Nes {


MaxScript::MaxScript()
  : offset(0) { }

void MaxScript::read(BlackT::TStream& ifs) {
  offset = ifs.tell();
  bool tilemapMode = false;
  do {
    ops.push_back(MaxScriptOp());
    ops.back().read(ifs, &tilemapMode);
  } while (!ops.back().isTerminator());
}

void MaxScript::write(BlackT::TStream& ofs) const {
  for (int i = 0; i < ops.size(); i++) {
    ops[i].write(ofs);
  }
}

void MaxScript::print(std::ostream& ofs,
           const BlackT::TThingyTable& thingy,
           const MaxScriptRegions* regions,
           bool recursive) const {
  
  if (!recursive)
    ofs << "// ";
  for (int i = 0; i < ops.size(); i++) {
    ops[i].print(ofs, thingy, regions, recursive);
    if (MaxScriptOp::isCommandOp(ops[i].type)
        && MaxScriptOp::getOpData(ops[i].type).postBreak
        && !ops[i].getIsRaw()) {
//      ofs << std::endl;
      ofs << "// ";
    }
  }
  if (!recursive)
    ofs << std::endl;
  
}

void MaxScript::toDictString(MaxDictStringOp& dst) const {
  for (int i = 0; i < ops.size(); i++) {
    dst.appendChar(ops[i]);
  }
}

void MaxScript::fromDictString(const MaxDictStringOp& src,
                      const DictionaryReferenceMap& refMap) {
  ops.clear();
  
  for (int i = 0; i < src.size(); i++) {
    const MaxDictCharOp& op = src.getItem(i);
    if (op.type() == MaxDictCharOp::typeChar) {
      const MaxScriptOp& subop = op.asChar();
      
      ops.push_back(subop);
    }
    else if (op.type() == MaxDictCharOp::typeDictMarker) {
      const MaxDictMarker& marker = op.asDictMarker();
      
      DictionaryReferenceMap::const_iterator findIt
        = refMap.find(marker.id);
      if (findIt == refMap.end()) {
        throw TGenericException(T_SRCANDLINE,
                                "MaxScript::fromDictString()",
                                "Unindexed dictionary reference: "
                                  + TStringConversion::intToString(marker.id));
      }
      
      MaxScriptOp newop;
      switch (findIt->second.region) {
      case MaxScriptOp::dictionaryRegion11:
        newop.type = MaxScriptOp::opDic11;
        break;
      case MaxScriptOp::dictionaryRegion13:
        newop.type = MaxScriptOp::opDic13;
        break;
      case MaxScriptOp::dictionaryRegion14:
        newop.type = MaxScriptOp::opDic14;
        break;
      default:
        throw TGenericException(T_SRCANDLINE,
                                "MaxScript::fromDictString()",
                                "Illegal dictionary region: "
                                  + TStringConversion::intToString(
                                      findIt->second.region));
        break;
      }
      
      newop.param1 = findIt->second.id;
      newop.param2 = findIt->second.region;
      
      ops.push_back(newop);
    }
    else {
      throw TGenericException(T_SRCANDLINE,
                              "MaxScript::fromDictString()",
                              "Illegal dictionary op type");
    }
  }
}


}
