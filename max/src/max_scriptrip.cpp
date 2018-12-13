#include "nes/NesRom.h"
#include "max/MaxScript.h"
#include "max/MaxScriptRegions.h"
#include "util/TStringConversion.h"
#include "util/TBufStream.h"
#include "util/TThingyTable.h"
#include <vector>
#include <iostream>
#include <fstream>

using namespace std;
using namespace BlackT;
using namespace Nes;

int main(int argc, char* argv[]) {
  if (argc < 4) {
    cout << "Metal Max script dumper" << endl;
    cout << "Usage: " << argv[0] << " <rom> <thingytable> <regionfile>"
      << endl;
    
    return 0;
  }
  
  NesRom rom = NesRom(string(argv[1]));
  TThingyTable thingy;
  thingy.readSjis(string(argv[2]));
  std::ifstream regifs(argv[3]);
  
  TBufStream ifs(rom.size());
  ifs.write((char*)rom.directRead(0), rom.size());
  
  MaxScriptRegions scriptRegions;
  int totalNumScripts = 0;
  while (regifs.good()) {
    regifs.get();
    if (!regifs.good()) break;
    regifs.unget();
    
    int regionNum;
    regifs >> std::hex >> regionNum;
    if (!regifs.good()) break;
    int address;
    regifs >> std::hex >> address;
    if (!regifs.good()) break;
    int numScripts;
    regifs >> std::dec >> numScripts;
    if (!regifs.good()) break;
    
//    cout << hex << regionNum << " " << hex << address << " "
///      << dec << numScripts << endl;
//    if ((regionNum == 0x11) || (regionNum == 0x13) || (regionNum == 0x14)) {
//    
//    }
//    else {
//      totalNumScripts += numScripts;
//    }

    ifs.seek(address);
    
    vector<MaxScript> scripts;
    for (int i = 0; i < numScripts; i++) {
      scripts.push_back(MaxScript());
      scripts.back().read(ifs);
    }
    scriptRegions.addRegion(scripts);
  }
  
//  cout << totalNumScripts << endl;
  
  for (int i = 0; i < scriptRegions.regions.size(); i++) {
    vector<MaxScript>& scripts = scriptRegions.regions[i];
    
    string filenum = TStringConversion::intToString(i,
                        TStringConversion::baseHex).substr(2,
                            string::npos);
    string filenum_orig = filenum;
    while (filenum.size() < 2) filenum = "0" + filenum;
    string filename = "script_region" + filenum + ".txt";
    
    std::ofstream ofs(filename.c_str(), ios_base::trunc);
    for (int j = 0; j < scripts.size(); j++) {
      ofs << "// Script " << filenum_orig << "-"
        << TStringConversion::intToString(j,
            TStringConversion::baseHex) << " ["
        << TStringConversion::intToString(scripts[j].offset,
            TStringConversion::baseHex)
        << "]" << std::endl;
      scripts[j].print(ofs, thingy, &scriptRegions);
      ofs << std::endl << std::endl;
    }
  }
  
//  TStringConversion::stringToInt(string(argv[3]));
  
/*  TBufStream ifs(rom.size());
  ifs.write((char*)rom.directRead(0), rom.size());
  
  vector<MaxScript> scripts;
  ifs.seek(offset);
  for (int i = 0; i < numScripts; i++) {
    cout << "// Script " << i << " ["
      << TStringConversion::intToString(ifs.tell(),
          TStringConversion::baseHex)
      << "]" << endl;
    scripts.push_back(MaxScript());
    scripts.back().read(ifs);
    scripts.back().print(cout, thingy);
    cout << endl << endl;
  } */
  
  return 0;
}
