#include "nes/NesRom.h"
#include "max/MaxScript.h"
#include "util/TStringConversion.h"
#include "util/TBufStream.h"
#include "util/TThingyTable.h"
#include <vector>
#include <iostream>

using namespace std;
using namespace BlackT;
using namespace Nes;

int main(int argc, char* argv[]) {
  if (argc < 5) {
    cout << "Metal Max script dumper" << endl;
    cout << "Usage: " << argv[0] << " <rom> <thingytable> <startoffset>"
      << " <numscripts>" << endl;
    
    return 0;
  }
  
  NesRom rom = NesRom(string(argv[1]));
  TThingyTable thingy;
  thingy.readUtf8(string(argv[2]));
  int offset = TStringConversion::stringToInt(string(argv[3]));
  int numScripts = TStringConversion::stringToInt(string(argv[4]));
  
  TBufStream ifs(rom.size());
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
  }
  
  return 0;
}
