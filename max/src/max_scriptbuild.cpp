#include "max/MaxScriptReader.h"
#include "max/MaxScriptRegions.h"
#include "max/MaxScript.h"
#include "max/MaxDictGenerator.h"
#include "max/MaxScriptOp.h"
#include "nes/NesRom.h"
#include "nes/NesPattern.h"
#include "util/TIniFile.h"
#include "util/TStringConversion.h"
#include "util/TIfstream.h"
#include "util/TOfstream.h"
#include "util/TBufStream.h"
#include "util/TFreeSpace.h"
#include "util/TGraphic.h"
#include "util/TPngConversion.h"
#include "exception/TGenericException.h"
#include <ctime>
#include <vector>
#include <map>
#include <sstream>
#include <iostream>
#include <fstream>

using namespace std;
using namespace BlackT;
using namespace Nes;

const static int scriptBufferSize = 0x100000;
//const static int scriptSizeLimit = 0x2000;

const static int inputRomSize = 0xC0000;
const static int inputPrgPos = 0x0;
const static int inputPrgSize = 0x80000;
const static int inputChrPos = 0x80000;
const static int inputChrSize = 0x40000;
//const static int inputPrgFinalBanksPos = 0x3C000;
//const static int inputPrgFinalBanksSize = 0x4000;

const static int outputRomSize = 0xC0000;
const static int prgBankSize = 0x2000;
const static int outputPrgPos = inputPrgPos;
const static int outputPrgSize = 0x80000;
const static int outputChrPos = 0x80000;
const static int outputChrSize = inputChrSize;
const static int outputPrgHeaderSize = outputPrgSize / 0x4000;
const static int outputChrHeaderSize = outputChrSize / 0x2000;
const static NesRom::MapperType outputMapperType
  = NesRom::mapperMmc3;
const static NesRom::NametableArrangementFlag outputNametablesFlag
  = NesRom::nametablesVertical;
const static int outputPrgFinalBanksPos = 0x7C000;
//const static int outputPrgFinalBanksSize = inputPrgFinalBanksSize;
//const static int newFreePrgBanksPos = 0x40000;
//const static int numNewFreePrgBanks = 0x1E;
const static int newFreePrgBanksPos = 0x44000;
// reserve 2 banks (78000-7BFFF) for hack additions
const static int numNewFreePrgBanks = 0x1C - 2;

const static int scriptBankNumberTableOffset = 0x33AD6;
const static int scriptBankNumberWidth = 1;
const static int scriptPointerTableOffset = 0x33AAA;
const static int scriptPointerWidth = 2;
const static int scriptPointerBase = 0x7FFF;

const static int dictionaryRegion11 = 0x11;
const static int dictionaryRegion13 = 0x13;
const static int dictionaryRegion14 = 0x14;

#define USE_DIRECT_SCRIPT_LOOKUP 0

void status(const char* message) {
  std::cout
    << "  **********************************************************************"
    << std::endl
    << "   " << message
    << std::endl
    << "  **********************************************************************"
    << std::endl;
}

int intFromStream(istream& ifs) {
  string str;
  ifs >> str;
  return TStringConversion::stringToInt(str);
}

void patchGraphic(const char* filename,
                  BlackT::TStream& dst) {
  TGraphic g;
  TPngConversion::RGBAPngToGraphic(string(filename), g);
  int tileW = g.w() / NesPattern::width;
  int tileH = g.h() / NesPattern::height;
  
  for (int j = 0; j < tileH; j++) {
    for (int i = 0; i < tileW; i++) {
      int x = i * NesPattern::width;
      int y = j * NesPattern::height;
      NesPattern pattern;
      pattern.fromGrayscaleGraphic(g, x, y);
      pattern.write(dst);
    }
  }
}

int main(int argc, char* argv[]) {
try {

  if (argc < 2) {
    cout << "Metal Max script builder" << endl;
    cout << "Usage: " << argv[0] << "<buildscript>" << endl;
    
    return 0;
  }
  
  double timer = clock();

//  TIniFile buildscript = TIniFile(string(argv[1]));
  TIniFile buildscript = TIniFile(string(argv[1]));
  
  // set up rom
  
  status("Reading input ROM...");
  
  // open rom
/*  std::string inromName = std::string(argv[1]);
//  std::string inromName = buildscript.valueOfKey("Properties", "InputRom");
  std::cout << "Source: " << inromName << std::endl;
  NesRom rom(inromName);
  
  // make sure this is plausibly a metal max rom
  if (rom.size() != inputRomSize) {
    std::cerr << "*** Error ***" << std::endl;
    std::cerr << "Error: input ROM doesn't exist or is wrong size"
      << std::endl;
    return 1;
  }
  
  // copy prg
  TBufStream prgofs(outputPrgSize);
  prgofs.write((char*)rom.directRead(inputPrgPos), inputPrgSize);
  prgofs.seek(0);
  
  // copy chr
  TBufStream chrofs(outputChrSize);
  chrofs.write((char*)rom.directRead(inputChrPos), inputChrSize);
  chrofs.seek(0); */
  
/*  TFreeSpace freeSpace;
  freeSpace.setBoundarySize(prgBankSize);
  for (int i = 0; i < numNewFreePrgBanks; i++) {
    freeSpace.free(newFreePrgBanksPos + (i * prgBankSize),
                   prgBankSize);
  } */
  
  // get settings
  
  status("Reading settings...");
    
  std::string outPath = 
    buildscript.valueOfKey("Properties", "OutPath");
  
  int numRegions = TStringConversion::stringToInt(
    buildscript.valueOfKey("Properties", "NumInputRegions"));
//  int numGraphics = TStringConversion::stringToInt(
//    buildscript.valueOfKey("Properties", "NumGraphics"));
  
  MaxDictGenerator::DictGenType dictGenType;
  std::string dictGenStr
    = buildscript.valueOfKey("Properties", "DictGenerationMode");
  if (dictGenStr.compare("genChar") == 0) {
    dictGenType = MaxDictGenerator::dictGenChar;
  }
  else if (dictGenStr.compare("genWord") == 0) {
    dictGenType = MaxDictGenerator::dictGenWord;
  }
  else {
    throw TGenericException(T_SRCANDLINE,
                            "main()",
                            "Illegal DictGenerationMode in build script");
  }
  
  int minDictEntryLen = TStringConversion::stringToInt(
    buildscript.valueOfKey("Properties", "MinDictEntryLen"));
  int maxDictEntryLen = TStringConversion::stringToInt(
    buildscript.valueOfKey("Properties", "MaxDictEntryLen"));
  int maxDictEntries = TStringConversion::stringToInt(
    buildscript.valueOfKey("Properties", "MaxDictEntries"));
  int dictEntriesPerPass = TStringConversion::stringToInt(
    buildscript.valueOfKey("Properties", "DictEntriesPerPass"));
  int dictMaxRecursionLevel = TStringConversion::stringToInt(
    buildscript.valueOfKey("Properties", "DictMaxRecursionLevel"));
  
  map<int, bool> excludeRegions;
  string excludeString = buildscript.valueOfKey(
    "Properties", "ExcludeRegions");
  {
    istringstream ifs(excludeString);
    while (ifs.good()) excludeRegions[intFromStream(ifs)] = true;
  }
  
  string defaultThingyName = buildscript.valueOfKey(
    "Properties", "DefaultTable");
  TThingyTable defaultThingy;
  defaultThingy.readSjis(defaultThingyName);
  
  std::cout << "Number of input regions: " << std::dec
    << numRegions << std::endl;
  std::cout << "Default table: "
    << defaultThingyName << std::endl;
  std::cout << "Dictionary generation mode: " << std::dec
    << (dictGenType == MaxDictGenerator::dictGenChar
          ? "genChar" : "genWord") << std::endl;
  std::cout << "Minimum entry length: " << std::dec
    << minDictEntryLen << std::endl;
  std::cout << "Maximum entry length: " << std::dec
    << maxDictEntryLen << std::endl;
  std::cout << "Maximum dictionary entries: " << std::dec
    << maxDictEntries << std::endl;
  std::cout << "Dictionary entries per pass: " << std::dec
    << dictEntriesPerPass << std::endl;
  std::cout << "Max dictionary recursion level: " << std::dec
    << dictMaxRecursionLevel << std::endl;
  std::cout << "Excluded regions:";
  for (map<int, bool>::iterator it = excludeRegions.begin();
       it != excludeRegions.end();
       ++it) {
    std::cout << " $" << std::hex << it->first;
  }
  
/*  std::cout << std::endl;
  std::cout << "Number of input graphics: " << std::dec
    << numGraphics << std::endl;
  
  // do graphics
  
  status("Patching graphics...");
  
  for (int i = 0; i < numGraphics; i++) {
    string numstr = TStringConversion::intToString(i,
                      TStringConversion::baseHex).substr(2, string::npos);
    while (numstr.size() < 2) numstr = "0" + numstr;
    string nameStr = string("Graphic") + numstr;
    
    string grpName = buildscript.valueOfKey(nameStr,
                             "SourceFile");
    int offset = TStringConversion::stringToInt(
      buildscript.valueOfKey(nameStr, "Offset"));
    
    std::cout << "Patching file '" << grpName << "' to CHR $"
      << std::hex << offset << std::endl;
    
    chrofs.seek(offset);
    patchGraphic(grpName.c_str(), chrofs);
  } */
  
  // do dictionary stuff
  
  status("Reading scripts...");
  
  MaxScriptRegions regions;
  int regionId = 0;
  int regionCount = 0;
  std::map<int, std::string> regionIdToOutName;
  while (regionCount < numRegions) {
    while (excludeRegions.find(regionId) != excludeRegions.end()) ++regionId;
    
    string regionNumStr = TStringConversion::intToString(regionId,
                    TStringConversion::baseHex).substr(2, string::npos);
    while (regionNumStr.size() < 2) regionNumStr = "0" + regionNumStr;
    string regionStr = "Region" + regionNumStr;
    
//    cout << regionStr << endl;

    string srcFileName = buildscript.valueOfKey(
      regionStr, "SourceFile");
    string outName = buildscript.valueOfKey(
      regionStr, "OutName");
  
    std::cout << "Reading region $" << std::hex << regionId
      << " (source file: "
      << srcFileName
//      << ", output file: "
//      << outFileName
      << ")" << std::endl;
    
    regionIdToOutName[regionId] = outName;
    
    int srcFileSz;
    {
      TIfstream ifs(srcFileName.c_str(), ios_base::binary);
      srcFileSz = ifs.size();
    }
    
    TBufStream srcifs(srcFileSz);
    {
      TIfstream ifs(srcFileName.c_str(), ios_base::binary);
      srcifs.writeFrom(ifs, srcFileSz);
    }
    srcifs.seek(0);
    
    TBufStream srcofs(scriptBufferSize);
    
    try {
      MaxScriptReader(srcifs, srcofs, defaultThingy)();
    }
    catch (BlackT::TGenericException& e) {
      throw BlackT::TGenericException(e.nameOfSourceFile().c_str(),
                              e.lineNum(),
                              e.source().c_str(),
                              "In file '"
                                + srcFileName
                                + "':\n"
                                + e.problem());
    }
    
//    srcofs.save((srcFileName + ".bin").c_str());
    
    regions.regions[regionId] = MaxScriptRegion();
    MaxScriptRegion& region = regions.regions[regionId];

    srcofs.seek(0);
    while (!srcofs.eof()) {
      region.push_back(MaxScript());
      MaxScript& script = region.back();
      script.read(srcofs);
    }
    
    ++regionCount;
    ++regionId;
  }
  
  // oops
  regionIdToOutName[dictionaryRegion11] = "script_region11";
  regionIdToOutName[dictionaryRegion13] = "script_region13";
  regionIdToOutName[dictionaryRegion14] = "script_region14";
  
  // compress the script data we read and form a dictionary
  status("Performing dictionary compression...");
  MaxDictScripts dict;
  DictRegionMap compressedRegions;
  MaxDictGenerator(regions, dict, compressedRegions, defaultThingy,
                   dictGenType, minDictEntryLen, maxDictEntryLen,
                   maxDictEntries, dictEntriesPerPass,
                   dictMaxRecursionLevel)();
  
  // assign the dictionary entries to the three dictionary regions
  status("Assigning dictionary entries to regions...");
  DictionaryReferenceMap dictIndex;
  {
    MaxDictScripts& dict11
      = (compressedRegions[dictionaryRegion11] = MaxDictScripts());
    MaxDictScripts& dict13
      = (compressedRegions[dictionaryRegion13] = MaxDictScripts());
    MaxDictScripts& dict14
      = (compressedRegions[dictionaryRegion14] = MaxDictScripts());
    
    // currently, entries are just assigned evenly across the banks,
    // and we hope for the best; obviously this could be improved
    int pos11 = 0;
    int pos13 = 0;
    int pos14 = 0;
    MaxDictScripts* putRegion = &dict11;
    int* putPos = &pos11;
    const int* dicRegion = &dictionaryRegion11;
    for (int i = 0; i < dict.size(); i++) {
      putRegion->push_back(dict[i]);
      // referencing dictionary index 9F is misdetected by the game as
      // the end-of-script opcode: place a dummy entry there if we reach
      // it
      if (*putPos == 0x9F) {
        ++(*putPos);
        putRegion->push_back(dict[i]);
      }
//      MaxScript& script = putRegion->back();
//      script.from 
      
      DictionaryReference ref = { *dicRegion, *putPos };
      dictIndex[i] = ref;
      ++(*putPos);
      
      if (putRegion == &dict11) {
//        if (pos11 == 0x9F) { ++pos11; putRegion->push_back(dict[i]); }
//        DictionaryReference ref = { dictionaryRegion11, pos11++ };
 //       dictIndex[i] = ref;
        putRegion = &dict13;
        putPos = &pos13;
        dicRegion = &dictionaryRegion13;
      }
      else if (putRegion == &dict13) {
        putRegion = &dict14;
        putPos = &pos14;
        dicRegion = &dictionaryRegion14;
      }
      else {
        putRegion = &dict11;
        putPos = &pos11;
        dicRegion = &dictionaryRegion11;
      }
    }
  }
  
  // convert dictionary scripts to regular scripts (resolving dictionary
  // lookups in the process)
  // note: the dictionary strings use pointers to the original region
  // structure, so it's important that we create a new one instead of
  // reusing the old one

  std::string outIncFileName
    = outPath + "script_include" + ".inc";
  std::string outIndexIncFileName
    = outPath + "script_index_include" + ".inc";

  std::ofstream incofs(outIncFileName.c_str());
  std::ofstream indexincofs(outIndexIncFileName.c_str());

  std::string outIncSecFileName
    = outPath + "script_include_sections" + ".inc";
  std::ofstream scrsecofs(outIncSecFileName.c_str());
  
  // DIRECT LOOKUP
  #if USE_DIRECT_SCRIPT_LOOKUP
    indexincofs << ".include \"script_index_macro.s\"" << std::endl;
    // index routine banks
      indexincofs << ".bank $19 slot 3" << std::endl;
      indexincofs
        << ".section \"script region to index routine bank table\" free"
        << std::endl;
      indexincofs
        << "  scriptRegionToIndexRoutineBank:"
        << std::endl;
      for (DictRegionMap::const_iterator it = compressedRegions.begin();
           it != compressedRegions.end();
           ++it) {
        int regionId = it->first;
        std::string outBaseName = regionIdToOutName[regionId];
        indexincofs << "  .db :" << outBaseName << "_indexLookup" << std::endl;
      }
      indexincofs << ".ends" << std::endl;
      // index routine pointers
      indexincofs << ".bank $19 slot 3" << std::endl;
      indexincofs
        << ".section \"script region to index routine pointer table\" free"
        << std::endl;
      indexincofs
        << "  scriptRegionToIndexRoutinePointer:"
        << std::endl;
      for (DictRegionMap::const_iterator it = compressedRegions.begin();
           it != compressedRegions.end();
           ++it) {
        int regionId = it->first;
        std::string outBaseName = regionIdToOutName[regionId];
        indexincofs << "  .dw " << outBaseName << "_indexLookup" << std::endl;
      }
    
    indexincofs << ".ends" << std::endl;
  #else
    // bank table
      incofs << ".bank $19 slot 3" << std::endl;
      incofs
        << ".section \"script region to bank table\" free"
        << std::endl;
      incofs
        << "  scriptRegionBankTable:"
        << std::endl;
      for (DictRegionMap::const_iterator it = compressedRegions.begin();
           it != compressedRegions.end();
           ++it) {
        int regionId = it->first;
        std::string outBaseName = regionIdToOutName[regionId];
        incofs << "  .db :" << outBaseName << std::endl;
      }
      incofs << ".ends" << std::endl;
      
      // pointer table
      incofs << ".bank $19 slot 3" << std::endl;
      incofs
        << ".section \"script region to decremented pointer table\" free"
        << std::endl;
      incofs
        << "  scriptRegionBasePtrTable:"
        << std::endl;
      for (DictRegionMap::const_iterator it = compressedRegions.begin();
           it != compressedRegions.end();
           ++it) {
        int regionId = it->first;
        std::string outBaseName = regionIdToOutName[regionId];
        incofs << "  .dw " << outBaseName << "-1" << std::endl;
      }
    
    incofs << ".ends" << std::endl;
  #endif
  
  
  status("Creating final compressed scripts...");
  MaxScriptRegions regionsFinal;
  bool hadError = false;
  for (DictRegionMap::const_iterator it = compressedRegions.begin();
       it != compressedRegions.end();
       ++it) {
    int regionId = it->first;
    MaxScriptRegion& region
      = (regionsFinal.regions[regionId] = MaxScriptRegion());
    const MaxDictScripts& dictRegion = it->second;
    
//    std::ofstream fofs(("region_" + TStringConversion::intToString(
//                   regionId) + "_cmp.txt").c_str(), std::ios_base::binary);

    std::vector<int> scriptOffsets;
    TBufStream ofs(scriptBufferSize);
    for (int i = 0; i < dictRegion.size(); i++) {
      const MaxDictStringOp& op = dictRegion[i];
      region.push_back(MaxScript());
      MaxScript& script = region.back();
      script.fromDictString(op, dictIndex);
      
/*      fofs << "// Script " << std::hex << regionId << "-"
        << std::hex << i
        << " (" << std::hex << ofs.tell() << ")" << std::endl;
      script.print(fofs, defaultThingy);
      fofs << std::endl << std::endl; */

      scriptOffsets.push_back(ofs.tell());
      script.write(ofs);
    }
    
    if (ofs.size() >= prgBankSize) {
      std::cerr << "*** Error ***" << std::endl;
      std::cerr << "Error: Region $" << std::hex << regionId
        << " exceeds $" << std::hex << prgBankSize
          << " byte size limit (output was $"
          << std::hex << ofs.size() << " bytes)" << std::endl;
      hadError = true;
//      return 1;
    }
  
    // output data to rom
    
/*    int writepos = freeSpace.claim(ofs.size());
    if (writepos == -1) {
      std::cerr << "*** Error ***" << std::endl;
      std::cerr << "Error: Out of free ROM space (couldn't claim "
        << ofs.size() << " bytes)" << std::endl;
      return 1;
    } */
    
    std::string outBaseName = regionIdToOutName[regionId];
    std::string outFileName
      = outPath + outBaseName + ".bin";
    std::string outIndexName
      = outPath + outBaseName + "_index" + ".inc";
    
    std::cout << "Outputting region $" << std::hex << regionId << std::endl;
//    std::cout << "  Offset: $" << writepos << std::endl;
    std::cout << "  Output file: " << outFileName << std::endl;
    std::cout << "  Size: $" << ofs.size() << std::endl;
    std::cout << std::endl;
    
    TBufStream scrofs(0x10000);
    ofs.seek(0);
    scrofs.writeFrom(ofs, ofs.size());
    scrofs.save(outFileName.c_str());
//    prgofs.seek(writepos);
//    ofs.seek(0);
//    prgofs.writeFrom(ofs, ofs.size());

/*    incofs << ".slot 2" << std::endl;
    incofs << ".section \"" << outBaseName << "\" superfree" << std::endl;
    incofs << "  " << outBaseName << ":" << std::endl;
    incofs << "    .incbin \"" << outFileName << "\"" << std::endl;
    incofs << ".ends" << std::endl; */
    
    scrsecofs << ".slot 2" << std::endl;
    scrsecofs << ".section \"" << outBaseName << "\" superfree" << std::endl;
    scrsecofs << "  " << outBaseName << ":" << std::endl;
    scrsecofs << "    .incbin \"" << outFileName << "\"" << std::endl;
    scrsecofs << ".ends" << std::endl;

    indexincofs << ".slot 3" << std::endl;
    indexincofs << ".section \"" << outBaseName << " index\" superfree" << std::endl;
    indexincofs << "  " << outBaseName << "_index:" << std::endl;
    for (int i = 0; i < scriptOffsets.size(); i++) {
      indexincofs << "    .dw " << outBaseName << "+" << scriptOffsets[i]
        << "-1" << std::endl;
    }
    #if USE_DIRECT_SCRIPT_LOOKUP
      indexincofs << "    " << outBaseName << "_indexLookup:" << std::endl;
      indexincofs << "      generateLookupRegionScriptRoutine "
        // param 1: region bank
        << ":" << outBaseName
        // param 2: index pointer
        << " " << outBaseName << "_index"
        << std::endl;
    #endif
    indexincofs << ".ends" << std::endl;
    
    // update bank number
/*    prgofs.seek(scriptBankNumberTableOffset
                  + (regionId * scriptBankNumberWidth));
    prgofs.writeu8(writepos / prgBankSize);
    
    // update pointer
    prgofs.seek(scriptPointerTableOffset + (scriptPointerWidth * regionId));
    prgofs.writeu16le((writepos % prgBankSize) + scriptPointerBase); */
  }
  
  // export rom
/*  status("Exporting ROM...");
  std::string outromName = std::string(argv[2]);
  std::cout << "Destination: " << outromName << std::endl;
  rom.changeSize(outputRomSize);
  prgofs.seek(0);
  chrofs.seek(0);
  prgofs.read((char*)(rom.directWrite(outputPrgPos)),
               outputPrgSize);
  chrofs.read((char*)(rom.directWrite(outputChrPos)),
               outputChrSize);
  rom.exportToFile(outromName,
                   outputPrgHeaderSize,
                   outputChrHeaderSize,
                   outputNametablesFlag,
                   outputMapperType,
                   true); */
                   
  if (!hadError) {
    status("Success!");
    std::cout << std::endl;
  }
  
  timer = (double)(clock() - timer) / (double)CLOCKS_PER_SEC;
  
  std::cout << "Build time: " << timer << " sec." << std::endl << std::endl;
  
  if (hadError) {
    std::cerr << "*** One or more errors occurred. Check the build log."
              << std::endl << std::endl;
    return 1;
  }
  
  return 0;
  
}
catch (TException& e) {
  std::cerr << "*** Error ***" << std::endl;
  std::cerr << "An exception occurred:"
    << std::endl << std::endl;
  std::cerr << e.what() << std::endl;
  return 1;
}
catch (std::exception& e) {
  std::cerr << "*** Error ***" << std::endl;
  std::cerr << "An unhandled exception occurred:"
    << std::endl << std::endl;
  std::cerr << e.what() << std::endl;
  return 1;
}
catch (...) {
  std::cerr << "*** Error ***" << std::endl;
  std::cerr << "Error: An unknown exception occurred"
    << std::endl << std::endl;
  return 1;
}

}
