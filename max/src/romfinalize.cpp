#include "util/TIfstream.h"
#include "util/TOfstream.h"
#include "util/TBufStream.h"
#include "nes/NesRom.h"
#include <cstring>
#include <iostream>

using namespace std;
using namespace BlackT;
using namespace Nes;

const static int outputChrBanks = 32;
const static int outputChrSize = 0x40000;

const static int outputPrgBanks = 0x20;
const static int prgBankSize = 0x4000;
const static int outputPrgSize = (outputPrgBanks * prgBankSize);
const static int outputRomSize = outputPrgSize + outputChrSize;
//const static int oldFixedBankPos = 0x1C000;
//const static int newFixedBankPos = 0x3C000;

int main(int argc, char* argv[]) {
  if (argc < 4) {
    cout << "Metal Max ROM finalizer" << endl;
    cout << "Usage: " << argv[0]
      << " <inprg> <inchr> <outrom>" << endl;
    
    return 0;
  }
  
  // open ROM
  
  NesRom rom = NesRom(string(argv[1]));
  TBufStream ifs(rom.size());
  rom.toStream(ifs);
  
  // expand ROM
  
  ifs.setCapacity(outputRomSize);
  ifs.seek(ifs.size());
//  ifs.expandToSize(outputRomSize);
  ifs.alignToBoundary(outputRomSize);
  ifs.seek(0);
  
  // enforce reset banking restrictions
/*  for (int i = 0; i < outputPrgBanks; i++) {
    ifs.seek((i * prgBankSize) + 0x3FD0);
    ifs.write(bankCode, (sizeof(bankCode) / sizeof(char)) - 1);
    ifs.seek((i * prgBankSize));
    ifs.put(0x80);
  } */
  
  // write CHR
  
  {
    TBufStream chrifs(1);
    chrifs.open(argv[2]);
    ifs.seek(outputPrgSize);
    ifs.writeFrom(chrifs, outputChrSize);
  }
  
  // write to file with iNES header
  
  ifs.seek(0);
  rom.changeSize(outputRomSize);
  rom.fromStream(ifs);
  rom.exportToFile(string(argv[3]), outputPrgBanks, outputChrBanks,
                   NesRom::nametablesVertical,
                   NesRom::mapperMmc3,
                   true);
  
  return 0;
} 
