#include "util/TBufStream.h"
#include "util/TIfstream.h"
#include "util/TOfstream.h"
#include "util/TGraphic.h"
#include "util/TStringConversion.h"
#include "util/TPngConversion.h"
#include "util/TCsv.h"
#include "util/TThingyTable.h"
#include "nes/NesPattern.h"
#include "max/MaxLineWrapper.h"
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
MaxLineWrapper::SpkrSizeTable spkrSizeTable;
//TThingyTable nameTable;
//std::vector<VillgustScriptReader::ResultCollection> sets;

/*void packScripts(const VillgustScriptReader::ResultCollection& set,
                   TStream& ofs, int slotBase = 0x8000, int basePos = 0x0000) {
  int indexBase = ofs.tell();
  int putpos = ofs.tell() + (set.size() * 2);
  for (unsigned int i = 0; i < set.size(); i++) {
    const VillgustScriptReader::ResultString& resultString = set[i];
    ofs.seek(indexBase + (i * 2));
    ofs.writeu16le((putpos % bankSize) + slotBase + basePos);
    ofs.seek(putpos);
    ofs.write(resultString.str.c_str(), resultString.str.size());
    putpos += resultString.str.size();
  }
}

void writeScriptsRaw(const VillgustScriptReader::ResultCollection& set,
                   TStream& ofs) {
  for (unsigned int i = 0; i < set.size(); i++) {
    const VillgustScriptReader::ResultString& resultString = set[i];
    ofs.write(resultString.str.c_str(), resultString.str.size());
  }
}

void writeAsmSet(const VillgustScriptReader::ResultCollection& set,
                 string baseName, string baseBinName,
                 string outIncName) {
  
  {
    std::ofstream ofs((outIncName +  + "_index.inc").c_str());
    for (unsigned int i = 0; i < set.size(); i++) {
      string name = baseName
                    + "_"
                    + TStringConversion::intToString(i);
      ofs << ".dw " << name << endl;
    }
  }
  
  {
    std::ofstream ofs((outIncName +  + "_data.inc").c_str());
    for (unsigned int i = 0; i < set.size(); i++) {
      TBufStream binofs(0x1000);
      string name = baseName
                    + "_"
                    + TStringConversion::intToString(i);
      string binname = baseBinName
                  + TStringConversion::intToString(i)
                  + ".bin";
      
      const VillgustScriptReader::ResultString& resultString = set[i];
      binofs.write(resultString.str.c_str(), resultString.str.size());
      
      ofs << name << ":" << endl;
      ofs << "  .incbin \"" << binname << "\"" << endl;
      
      binofs.save(binname.c_str());
    }
  }
  
  {
    std::ofstream ofs((outIncName +  + ".inc").c_str());
    ofs << ".include \"" << (outIncName +  + "_index.inc") + "\"" << endl;
    ofs << ".include \"" << (outIncName +  + "_data.inc") + "\"" << endl;
  }
  
}

void addSet(string filename) {
  cout << "adding set " << filename << endl;
  
  VillgustScriptReader::ResultCollection set;
//  TIfstream ifs((filename).c_str(), ios_base::binary);
  TBufStream ifs(1);
  ifs.open((filename).c_str());
  VillgustScriptReader(ifs, set, table)();
  sets.push_back(set);
} */

void wrapSet(string filename, string outfile) {
  cout << "wrapping set " << filename << " to " << outfile << endl;

  TBufStream ifs(1);
  ifs.open(filename.c_str());
  TLineWrapper::ResultCollection set;
//  MaxLineWrapper(ifs, set, table, 22, 4)();
  MaxLineWrapper(ifs, set, table, spkrSizeTable, -1, -1)();
  
  TBufStream ofs(0x10000);
  for (unsigned int i = 0; i < set.size(); i++) {
    const TLineWrapper::ResultString& resultString = set[i];
    ofs.write(resultString.str.c_str(), resultString.str.size());
  }
  ofs.save(outfile.c_str());
}

/*void wrapAndPackSet(string prefix, string outprefix,
                    string filenameBase, string outFilenameBase) {
  wrapSet((prefix + filenameBase + ".txt"),
          (outprefix + outFilenameBase + "_wrapped.txt"));
  
  {
    TBufStream ifs(1);
    ifs.open((outprefix + outFilenameBase + "_wrapped.txt").c_str());
    VillgustScriptReader::ResultCollection set;
    VillgustScriptReader(ifs, set, table)();
    
    writeAsmSet(set,
                outFilenameBase,
                outprefix + "maps/" + outFilenameBase + "_",
                outprefix + "maps/" + outFilenameBase);
    
  }
}

void writeScriptsRaw(const MaxRawScriptReader::ResultCollection& set,
                   TStream& ofs) {
  for (unsigned int i = 0; i < set.size(); i++) {
    const MaxRawScriptReader::ResultString& resultString = set[i];
    ofs.write(resultString.str.c_str(), resultString.str.size());
  }
} */

void buildSpkrSizeTable(std::string spkrScriptName) {
  TBufStream ifs(1);
  ifs.open(spkrScriptName.c_str());
  MaxRawScriptReader::ResultCollection set;
  MaxRawScriptReader(ifs, set, table)();
  
  if (set.size() > 0) {
    const MaxRawScriptReader::ResultString& resultString = set[0];
    TBufStream ifs(resultString.str.size());
    ifs.write(resultString.str.c_str(), resultString.str.size());
    ifs.seek(0);
    
    int id = 0;
    int size = 0;
    while (!ifs.eof()) {
      // if terminator
      int next = (unsigned char)ifs.get();
      if ((next == 0x9F)) {
        spkrSizeTable[id] = size;
//        std::cerr << hex << id << " " << size << std::endl;
        ++id;
        size = 0;
      }
      // if SVAR (should only happen with param of 0, 1, or 2 for party
      // members, whose names are capped at 8 characters in the translation)
      else if ((next == 0xFD)) {
        // ignore param
//        int param = (unsigned char)ifs.get();
        ifs.get();
        
        // assume 8 characters
        size += 8;
      }
      else {
        ++size;
      }
    }
  }
  
//  TBufStream ofs(0x10000);
//  writeScriptsRaw(set, ofs);
//  ofs.save((outprefix + "names_specialparties"
//              + ".bin").c_str());
}

int main(int argc, char* argv[]) {
  if (argc < 4) {
    cout << "Metal Max script wrapper" << endl;
    cout << "Usage: " << argv[0]
      << " <inprefix> <table> <outprefix>" << endl;
    
    return 0;
  }
  
  std::string prefix = std::string(argv[1]);
  std::string outprefix = std::string(argv[3]);
  
  // Determine lengths of speaker prexfixes, which are stored in region C
  buildSpkrSizeTable(prefix + "script_region0C.txt");
  
//  table.readUtf8(string(argv[2]));
  table.readSjis(string(argv[2]));
  
//  nameTable.readSjis(string(argv[3]));
  
  wrapSet(prefix + "script_region00.txt", outprefix + "script_region00.txt");
  wrapSet(prefix + "script_region01.txt", outprefix + "script_region01.txt");
  wrapSet(prefix + "script_region02.txt", outprefix + "script_region02.txt");
  wrapSet(prefix + "script_region03.txt", outprefix + "script_region03.txt");
  wrapSet(prefix + "script_region04.txt", outprefix + "script_region04.txt");
  wrapSet(prefix + "script_region05.txt", outprefix + "script_region05.txt");
  wrapSet(prefix + "script_region06.txt", outprefix + "script_region06.txt");
  wrapSet(prefix + "script_region07.txt", outprefix + "script_region07.txt");
  wrapSet(prefix + "script_region08.txt", outprefix + "script_region08.txt");
  wrapSet(prefix + "script_region09.txt", outprefix + "script_region09.txt");
  wrapSet(prefix + "script_region0A.txt", outprefix + "script_region0A.txt");
  wrapSet(prefix + "script_region0B.txt", outprefix + "script_region0B.txt");
  wrapSet(prefix + "script_region0C.txt", outprefix + "script_region0C.txt");
  wrapSet(prefix + "script_region0D.txt", outprefix + "script_region0D.txt");
  wrapSet(prefix + "script_region0E.txt", outprefix + "script_region0E.txt");
  wrapSet(prefix + "script_region0F.txt", outprefix + "script_region0F.txt");
  wrapSet(prefix + "script_region10.txt", outprefix + "script_region10.txt");
//  wrapSet(prefix + "script_region11.txt", outprefix + "script_region11.txt");
  wrapSet(prefix + "script_region12.txt", outprefix + "script_region12.txt");
//  wrapSet(prefix + "script_region13.txt", outprefix + "script_region13.txt");
//  wrapSet(prefix + "script_region14.txt", outprefix + "script_region14.txt");
  wrapSet(prefix + "script_region15.txt", outprefix + "script_region15.txt");
  wrapSet(prefix + "script_region16_(05_part2).txt",
          outprefix + "script_region16_(05_part2).txt");
  wrapSet(prefix + "script_region17_(0B_part2).txt",
          outprefix + "script_region17_(0B_part2).txt");
  wrapSet(prefix + "script_region18_(0E_part2).txt",
          outprefix + "script_region18_(0E_part2).txt");
  wrapSet(prefix + "script_region19_shortitems.txt",
          outprefix + "script_region19_shortitems.txt");
  wrapSet(prefix + "script_region1A_pluralenemies.txt",
          outprefix + "script_region1A_pluralenemies.txt");

  // screw passing in parameters, I have to rewrite this damn thing
  // every time anyway
//  TThingyTable tiletable;
//  tiletable.readSjis("table/villgust_tilemap_en.tbl");

//  sets.resize(numSets);

/*  {
    TBufStream ifs(1);
    ifs.open((prefix + "script_4E000.txt").c_str());
    TLineWrapper::ResultCollection set;
    MaxLineWrapper(ifs, set, table, 22, 4)();
    
    TBufStream ofs(0x10000);
//    writeScriptsRaw(set, ofs);
    for (unsigned int i = 0; i < set.size(); i++) {
      const TLineWrapper::ResultString& resultString = set[i];
      ofs.write(resultString.str.c_str(), resultString.str.size());
    }
    ofs.save((outprefix + "script_4E000_wrapped.txt").c_str());
  } */
  
/*  wrapAndPackSet(prefix, outprefix, "script_0B586", "script0B586");
  wrapAndPackSet(prefix, outprefix, "script_1AFEE", "script1AFEE");
  wrapAndPackSet(prefix, outprefix, "script_4E000", "script4E000");
  wrapAndPackSet(prefix, outprefix, "script_4F000", "script4F000");
  wrapAndPackSet(prefix, outprefix, "script_5C000", "script5C000");
  wrapAndPackSet(prefix, outprefix, "script_5D000", "script5D000");
  wrapAndPackSet(prefix, outprefix, "script_5E000", "script5E000");
  wrapAndPackSet(prefix, outprefix, "script_57000", "script57000");
  
  {
    TBufStream ifs(1);
    ifs.open((prefix + "battle_inventory" + ".txt").c_str());
    VillgustScriptReader::ResultCollection set;
    VillgustScriptReader(ifs, set, tiletable)();
    
    TBufStream ofs(0x10000);
    writeScriptsRaw(set, ofs);
    ofs.save((outprefix + "battle_inventory"
                + ".bin").c_str());
  }
  
  {
    TBufStream ifs(1);
    ifs.open((prefix + "title_options" + ".txt").c_str());
    VillgustScriptReader::ResultCollection set;
    VillgustScriptReader(ifs, set, tiletable)();
    
    TBufStream ofs(0x10000);
    writeScriptsRaw(set, ofs);
    ofs.save((outprefix + "title_options"
                + ".bin").c_str());
  } */
  
/*  wrapSet((prefix + "script_4E000.txt"),
          (outprefix + "script_4E000_wrapped.txt"));
  
  {
    TBufStream ifs(1);
    ifs.open((outprefix + "script_4E000_wrapped.txt").c_str());
    VillgustScriptReader::ResultCollection set;
    VillgustScriptReader(ifs, set, table)();
    
    writeAsmSet(set,
                "script4E000",
                outprefix + "maps/script4E000_",
                outprefix + "maps/script4E000.inc");
    
  } */
  
//  addSet(prefix + "script_4E000_wrapped.txt");
//  for (unsigned int i = 0; i < sets.size(); i++) {
//    TBufStream ofs(0x10000);
//    writeAsmSet(sets[i], ofs, 0x8000, 0x0000);
//  }

/*  addSet(prefix + "script_0B586.txt");
  addSet(prefix + "script_1AFEE.txt");
  addSet(prefix + "script_4E000.txt");
  addSet(prefix + "script_4F000.txt");
  addSet(prefix + "script_5C000.txt");
  addSet(prefix + "script_5D000.txt");
  addSet(prefix + "script_5E000.txt"); */
  
/*  for (unsigned int i = 0; i < sets.size(); i++) {
    TBufStream ofs(0x10000);
    packScripts(sets[i], ofs, 0x8000, 0x0000);
    
    if (ofs.size() >= maxScriptBankSize) {
      cerr << "Error: section " << i << " too big ("
        << ofs.size() << " bytes, max " << maxScriptBankSize << ")" << endl;
      return 1;
    }
    
//    ofs.write(sets[i].c_str(), sets[i].size());
    ofs.save((outprefix + "script_" + TStringConversion::intToString(i)
                + ".bin").c_str());
  } */
  
/*  {
    TBufStream ifs(1);
    ifs.open((prefix + "menus" + ".txt").c_str());
    VillgustScriptReader::ResultCollection set;
    VillgustScriptReader(ifs, set, table_nocmp, table_nocmp, false)();
    
    TBufStream ofs(0x10000);
    writeScriptsRaw(set, ofs);
    ofs.save((outprefix + "menus"
                + ".bin").c_str());
  }
  
  {
    TIfstream ifs((prefix + "intro_text"
      + ".txt").c_str(), ios_base::binary);
    VillgustScriptReader::ResultCollection set;
    VillgustScriptReader(ifs, set, table, table, false)();
    
    TBufStream ofs(0x10000);
    packScripts(set, ofs, 0xBF00);
    ofs.save((outprefix + "intro_text"
                + ".bin").c_str());
  } */
  
  return 0;
}
