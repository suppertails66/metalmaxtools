#include "nes/NesRom.h"
#include "util/TBufStream.h"
#include <iomanip>
#include <iostream>

using namespace std;
using namespace BlackT;
using namespace Nes;

int main(int argc, char* argv[]) {
  if (argc < 2) {
    cout << "Metal Max script region list generator" << endl;
    cout << "Usage: " << argv[0] << " <rom>" << endl;
    
    return 0;
  }
  
  NesRom rom = NesRom(string(argv[1]));
  TBufStream ifs(rom.size());
  ifs.write((char*)rom.directRead(0), rom.size());
  
//  * 33AD6 is a table containing the bank numbers for each region.
//  * 33AAA is a table containing the 0x8000-based initial address for the first
//    script in each region (minus one due to a preincrement operation).
  for (int i = 0; i < 0x16; i++) {
    ifs.seek(0x33AD6 + i);
    int offset = ifs.readu8() * 0x2000;
    ifs.seek(0x33AAA + (i * 2));
    offset += (ifs.readu16le() - 0x8000) + 1;
    cout << "0x" << setw(2) << setfill('0') << hex << i << " "
      << hex << "0x" << offset << endl;
  }
  
  return 0;
}
