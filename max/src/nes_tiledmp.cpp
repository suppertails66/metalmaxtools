#include "nes/NesPattern.h"
#include "nes/NesRom.h"
#include "util/TOpt.h"
#include "util/TBufStream.h"
#include "util/TStringConversion.h"
#include "util/TGraphic.h"
#include "util/TPngConversion.h"
#include <string>
#include <iostream>

const static int defaultTilesPerRow = 16;

using namespace std;
using namespace BlackT;
using namespace Nes;

int main(int argc, char* argv[]) {
  if (argc < 5) {
    cout << "NES pattern data dumper" << endl;
    cout << "Usage: <rom> <offset> <numtiles> <outfile>" << endl;
    cout << "Options: " << endl;
    cout << "  -w   Set width of output in tiles" << endl;
    
    return 0;
  }
  
  int offset = TStringConversion::stringToInt(string(argv[2]));
  int numTiles = TStringConversion::stringToInt(string(argv[3]));
  string outfile = string(argv[4]);
  
  int tilesPerRow = defaultTilesPerRow;
  TOpt::readNumericOpt(argc, argv, "-w", &tilesPerRow);
  
  NesRom rom = NesRom(string(argv[1]));
  TBufStream ifs(rom.size());
  ifs.write((char*)rom.directRead(0), rom.size());
  
  int width = tilesPerRow * NesPattern::width;
  int height = numTiles / tilesPerRow;
  if ((numTiles % tilesPerRow) != 0) ++height;
  height *= NesPattern::height;
  
  TGraphic g(width, height);
  g.clearTransparent();
  
  ifs.seek(offset);
  for (int i = 0; i < numTiles; i++) {
    int x = (i % tilesPerRow) * NesPattern::width;
    int y = (i / tilesPerRow) * NesPattern::height;
    
    NesPattern pattern;
    pattern.read(ifs);
    
    pattern.drawGrayscale(g, x, y, false);
  }
  
  TPngConversion::graphicToRGBAPng(outfile, g);
  
  return 0;
}
