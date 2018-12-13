#include "max/MaxDictGenerator.h"
#include "max/MaxDictChar.h"
#include "max/MaxDictString.h"
#include "max/MaxDictCharOp.h"
#include "nes/NesRom.h"
#include "util/TStringConversion.h"
#include "util/TBufStream.h"
#include "util/TIfstream.h"
#include "util/TOfstream.h"
#include "util/TThingyTable.h"
#include <vector>
#include <fstream>
#include <iostream>

using namespace std;
using namespace BlackT;
using namespace Nes;

int main(int argc, char* argv[]) {
  if (argc < 5) {
    cout << "Metal Max dictionary generator (test)" << endl;
    cout << "Usage: " << argv[0]
      << " <rom> <thingytable> <regionfile> <outfile>" << endl;
    
    return 0;
  }
  
  NesRom rom = NesRom(string(argv[1]));
  TThingyTable thingy;
  thingy.readSjis(string(argv[2]));
  std::ifstream regifs(argv[3]);
  
  TBufStream ifs(rom.size());
  ifs.write((char*)rom.directRead(0), rom.size());
  
  MaxScriptRegions scriptRegions;
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
//      << dec << numScripts << endl;

    ifs.seek(address);
    
    vector<MaxScript> scripts;
    for (int i = 0; i < numScripts; i++) {
      scripts.push_back(MaxScript());
      scripts.back().read(ifs);
    }
    scriptRegions.addRegion(scripts);
  }
  
//  MaxDictChar<int> test;
//  MaxDictString<int> test2;

/*  MaxDictStringOp test;
  MaxDictStringOp test2;
  scriptToString(scriptRegions.getRegion(0)[1], test);
  scriptToString(scriptRegions.getRegion(0)[2], test2);
  
  cout << test.size() << endl;
  cout << test2.size() << endl;
  test.append(test2);
  cout << test.size() << endl; */
  
  MaxDictScripts dict;
  DictRegionMap regions;
  MaxDictGenerator generator
    = MaxDictGenerator(scriptRegions, dict, regions, thingy);
  generator.generate();
  
  return 0;
}
