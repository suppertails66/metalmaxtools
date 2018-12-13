#include "util/TIfstream.h"
#include "util/TOfstream.h"
#include "util/TBufStream.h"
#include "nes/NesRom.h"
#include <iostream>

using namespace std;
using namespace BlackT;
using namespace Nes;

const static int inputPrgBanks = 0x10;
const static int outputPrgBanks = 0x20;
const static int prgBankSize = 0x4000;
const static int inputPrgSize = inputPrgBanks * prgBankSize;
const static int outputPrgSize = outputPrgBanks * prgBankSize;
const static int oldFixedBankPos = 0x3C000;
const static int newFixedBankPos = 0x7C000;

const static int chrOffset = 0x40000;
const static int chrSize = 0x40000;

int main(int argc, char* argv[]) {
  if (argc < 4) {
    cout << "Metal Max ROM preparer" << endl;
    cout << "Usage: " << argv[0]
      << " <inrom> <outprg> <outchr>" << endl;
    
    return 0;
  }
  
  // open ROM
  
  NesRom rom = NesRom(string(argv[1]));
  TBufStream ifs(rom.size());
  rom.toStream(ifs);
  
  TBufStream prg(outputPrgSize);
  ifs.seek(0);
  prg.writeFrom(ifs, inputPrgSize);
  
  // copy out CHR
  {
    ifs.seek(chrOffset);
    TBufStream ofs(chrSize);
    ofs.writeFrom(ifs, chrSize);
    ofs.save(argv[3]);
  }
  
  // expand ROM
  
//  ifs.setCapacity(outputRomSize);
//  ifs.seek(ifs.size());
//  ifs.padToSize(outputRomSize);
//  ifs.seek(0);
  prg.seek(prg.size());
  prg.padToSize(outputPrgSize);
  
  // copy fixed bank to final bank
  
//  ifs.seek(newFixedBankPos);
//  ifs.write((char*)rom.directRead(oldFixedBankPos), prgBankSize);
  prg.seek(newFixedBankPos);
  prg.write((char*)rom.directRead(oldFixedBankPos), prgBankSize);
  
  // write to file (no iNES header)
  
  prg.save(argv[2]);
  
  return 0;
} 
