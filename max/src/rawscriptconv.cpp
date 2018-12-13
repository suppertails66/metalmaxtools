#include "util/TBufStream.h"
#include "util/TIfstream.h"
#include "util/TOfstream.h"
#include "util/TGraphic.h"
#include "util/TStringConversion.h"
#include "util/TPngConversion.h"
#include "util/TCsv.h"
#include "util/TThingyTable.h"
#include "nes/NesPattern.h"
#include "max/MaxRawScriptReader.h"
#include <string>
#include <vector>
#include <iostream>

using namespace std;
using namespace BlackT;
using namespace Nes;

const static int scriptBaseAddr = 0x0000;
const static int maxScriptBankSize = 0x2000;
const static int bankSize = 0x2000;

TThingyTable table;
//TThingyTable nameTable;
std::vector<MaxRawScriptReader::ResultCollection> sets;

void writeScriptsRaw(const MaxRawScriptReader::ResultCollection& set,
                   TStream& ofs) {
  for (unsigned int i = 0; i < set.size(); i++) {
    const MaxRawScriptReader::ResultString& resultString = set[i];
    ofs.write(resultString.str.c_str(), resultString.str.size());
  }
}

int main(int argc, char* argv[]) {
  if (argc < 4) {
    cout << "Metal Max raw script converter" << endl;
    cout << "Usage: " << argv[0]
      << " <inprefix> <table> <outprefix>" << endl;
    
    return 0;
  }
  
  std::string prefix = std::string(argv[1]);
  std::string outprefix = std::string(argv[3]);
  
//  table.readUtf8(string(argv[2]));
  table.readSjis(string(argv[2]));
  
  {
    TBufStream ifs(1);
    ifs.open((prefix + "names_specialparties" + ".txt").c_str());
    MaxRawScriptReader::ResultCollection set;
    MaxRawScriptReader(ifs, set, table)();
    
    TBufStream ofs(0x10000);
    writeScriptsRaw(set, ofs);
    ofs.save((outprefix + "names_specialparties"
                + ".bin").c_str());
  }
  
  {
    TBufStream ifs(1);
    ifs.open((prefix + "names_random_char2" + ".txt").c_str());
    MaxRawScriptReader::ResultCollection set;
    MaxRawScriptReader(ifs, set, table)();
    
    TBufStream ofs(0x10000);
    writeScriptsRaw(set, ofs);
    ofs.save((outprefix + "names_random_char2"
                + ".bin").c_str());
  }
  
  {
    TBufStream ifs(1);
    ifs.open((prefix + "names_random_char3" + ".txt").c_str());
    MaxRawScriptReader::ResultCollection set;
    MaxRawScriptReader(ifs, set, table)();
    
    TBufStream ofs(0x10000);
    writeScriptsRaw(set, ofs);
    ofs.save((outprefix + "names_random_char3"
                + ".bin").c_str());
  }
  
  {
    TBufStream ifs(1);
    ifs.open((prefix + "name_entry" + ".txt").c_str());
    MaxRawScriptReader::ResultCollection set;
    MaxRawScriptReader(ifs, set, table)();
    
    TBufStream ofs(0x10000);
    writeScriptsRaw(set, ofs);
    ofs.save((outprefix + "name_entry"
                + ".bin").c_str());
  }
  
  {
    TBufStream ifs(1);
    ifs.open((prefix + "rental_tanks" + ".txt").c_str());
    MaxRawScriptReader::ResultCollection set;
    MaxRawScriptReader(ifs, set, table)();
    
    TBufStream ofs(0x10000);
    writeScriptsRaw(set, ofs);
    ofs.save((outprefix + "rental_tanks"
                + ".bin").c_str());
  }
  
  {
    TBufStream ifs(1);
    ifs.open((prefix + "name_wolf" + ".txt").c_str());
    MaxRawScriptReader::ResultCollection set;
    MaxRawScriptReader(ifs, set, table)();
    
    TBufStream ofs(0x10000);
    writeScriptsRaw(set, ofs);
    ofs.save((outprefix + "name_wolf"
                + ".bin").c_str());
  }
  
  return 0;
}
