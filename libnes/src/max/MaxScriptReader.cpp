#include "max/MaxScriptReader.h"
#include "util/TBufStream.h"
#include "util/TStringConversion.h"
#include "exception/TGenericException.h"
#include <cctype>
#include <algorithm>
#include <string>
#include <iostream>

using namespace BlackT;

namespace Nes {


MaxScriptReader::MaxScriptReader(
                  BlackT::TStream& src__,
                  BlackT::TStream& dst__,
                  const BlackT::TThingyTable& thingy__)
  : src(src__),
    dst(dst__),
    thingy(thingy__),
    lineNum(0) {
  loadThingy(thingy__);
}

void MaxScriptReader::operator()() {
//  while (src.remaining() > 0) {
//    
//  }
  while (!src.eof()) {
    std::string line;
    src.getLine(line);
    ++lineNum;
//    std::cout << next << std::endl;
    
    if (line.size() <= 0) continue;
    
    // discard lines containing only ASCII spaces and tabs
    bool onlySpace = true;
    for (int i = 0; i < line.size(); i++) {
      if ((line[i] != ' ')
          && (line[i] != '\t')) {
        onlySpace = false;
        break;
      }
    }
    if (onlySpace) continue;
    
    TBufStream ifs(line.size());
    ifs.write(line.c_str(), line.size());
    ifs.seek(0);
    
    // check for comments
    if ((ifs.size() >= 2)
        && (ifs.peek() == '/')) {
      ifs.get();
      if (ifs.peek() == '/') continue;
      else ifs.unget();
    }
    
    // check for special stuff
    if (ifs.peek() == '#') {
      // directives
      ifs.get();
      processDirective(ifs);
      continue;
    }
    
//    while (!ifs.eof()) {
//      std::cout << ifs.get();
//    }
//    std::cout << std::endl;
    
    while (!ifs.eof()) {
      outputNextSymbol(ifs);
    }
  }
}
  
void MaxScriptReader::loadThingy(const BlackT::TThingyTable& thingy__) {
  thingy = thingy__;

  // To efficiently unmap script values, we order the thingy's strings
  // by length
  thingiesBySize.clear();
  thingiesBySize.resize(thingy.entries.size());
  int pos = 0;
  for (TThingyTable::RawTable::iterator it = thingy.entries.begin();
       it != thingy.entries.end();
       ++it) {
    ThingyValueAndKey t = { it->second, it->first };
    thingiesBySize[pos++] = t;
  }
  std::sort(thingiesBySize.begin(), thingiesBySize.end());
  
//  for (int i = 0; i < thingiesBySize.size(); i++) {
//    std::cout << thingiesBySize[i].key
//      << " " << thingiesBySize[i].value << std::endl;
//  }
}
  
void MaxScriptReader::outputNextSymbol(TStream& ifs) {
  // literal value
  if ((ifs.remaining() >= 5)
      && (ifs.peek() == '<')) {
    int pos = ifs.tell();
    
    ifs.get();
    if (ifs.peek() == '$') {
      ifs.get();
      std::string valuestr = "0x";
      valuestr += ifs.get();
      valuestr += ifs.get();
      
      if (ifs.peek() == '>') {
        ifs.get();
        int value = TStringConversion::stringToInt(valuestr);
        dst.writeu8(value);
        return;
      }
    }
    
    // not a literal value
    ifs.seek(pos);
  }

  for (int i = 0; i < thingiesBySize.size(); i++) {
    if (checkSymbol(ifs, thingiesBySize[i].value)) {
      int symbolSize;
      if (thingiesBySize[i].key <= 0xFF) symbolSize = 1;
      else if (thingiesBySize[i].key <= 0xFFFF) symbolSize = 2;
      else if (thingiesBySize[i].key <= 0xFFFFFF) symbolSize = 3;
      else symbolSize = 4;
      
      dst.writeInt(thingiesBySize[i].key, symbolSize,
        EndiannessTypes::big, SignednessTypes::nosign);
      
      return;
    }
  }
  
  std::string remainder;
  ifs.getLine(remainder);
  throw TGenericException(T_SRCANDLINE,
                          "MaxScriptReader::outputNextSymbol()",
                          "Line "
                            + TStringConversion::intToString(lineNum)
                            + ":\n  Couldn't match symbol at: '"
                            + remainder
                            + "'");
}
  
bool MaxScriptReader::checkSymbol(BlackT::TStream& ifs, std::string& symbol) {
  if (symbol.size() > ifs.remaining()) return false;
  
  int startpos = ifs.tell();
  for (int i = 0; i < symbol.size(); i++) {
    if (symbol[i] != ifs.get()) {
      ifs.seek(startpos);
      return false;
    }
  }
  
  return true;
}
  
void MaxScriptReader::processDirective(BlackT::TStream& ifs) {
  skipSpace(ifs);
  
  std::string name = matchName(ifs);
//  std::cerr << name << std::endl;
  matchChar(ifs, '(');
  
  for (int i = 0; i < name.size(); i++) {
    name[i] = toupper(name[i]);
  }
  
  if (name.compare("LOADTABLE") == 0) {
    processLoadTable(ifs);
  }
  else {
    throw TGenericException(T_SRCANDLINE,
                            "MaxScriptReader::processDirective()",
                            "Line "
                              + TStringConversion::intToString(lineNum)
                              + ":\n  Unknown directive: "
                              + name);
  }
  
  matchChar(ifs, ')');
}

void MaxScriptReader::processLoadTable(BlackT::TStream& ifs) {
  std::string tableName = matchString(ifs);
  TThingyTable table(tableName);
  loadThingy(table);
}

void MaxScriptReader::skipSpace(BlackT::TStream& ifs) const {
  ifs.skipSpace();
}

bool MaxScriptReader::checkString(BlackT::TStream& ifs) const {
  skipSpace(ifs);
  
  if (!ifs.eof() && (ifs.peek() == '"')) return true;
  return false;
}

bool MaxScriptReader::checkInt(BlackT::TStream& ifs) const {
  skipSpace(ifs);
  
  if (!ifs.eof() && (ifs.peek() == '"')) return true;
  return false;
}

std::string MaxScriptReader::matchString(BlackT::TStream& ifs) const {
  skipSpace(ifs);
  if (ifs.eof()) {
    throw TGenericException(T_SRCANDLINE,
                            "MaxScriptReader::matchString()",
                            "Line "
                              + TStringConversion::intToString(lineNum)
                              + ":\n  Unexpected end of line");
  }
  
  if (!checkString(ifs)) {
    throw TGenericException(T_SRCANDLINE,
                            "MaxScriptReader::matchString()",
                            "Line "
                              + TStringConversion::intToString(lineNum)
                              + ":\n  Unexpected non-string at: "
                              + getRemainingContent(ifs));
  }
  
  ifs.get();
  
  std::string result;
  while (!ifs.eof() && (ifs.peek() != '"')) result += ifs.get();
  
  matchChar(ifs, '"');
  
  return result;
}

int MaxScriptReader::matchInt(BlackT::TStream& ifs) const {
  skipSpace(ifs);
  if (ifs.eof()) {
    throw TGenericException(T_SRCANDLINE,
                            "MaxScriptReader::matchInt()",
                            "Line "
                              + TStringConversion::intToString(lineNum)
                              + ":\n  Unexpected end of line");
  }
  
  if (!checkInt(ifs)) {
    throw TGenericException(T_SRCANDLINE,
                            "MaxScriptReader::matchInt()",
                            "Line "
                              + TStringConversion::intToString(lineNum)
                              + ":\n  Unexpected non-int at: "
                              + getRemainingContent(ifs));
  }
  
  std::string numstr;
  numstr += ifs.get();
  if ((numstr[0] == '0') && (tolower(ifs.peek()) == 'x')) numstr += ifs.get();
  
  char next = ifs.peek();
  while (!ifs.eof()
         && (isdigit(next)
         || (tolower(next) == 'a')
         || (tolower(next) == 'b')
         || (tolower(next) == 'c')
         || (tolower(next) == 'd')
         || (tolower(next) == 'e')
         || (tolower(next) == 'f'))) numstr += next;
  
  return TStringConversion::stringToInt(numstr);
}

void MaxScriptReader::matchChar(BlackT::TStream& ifs, char c) const {
  skipSpace(ifs);
  if (ifs.eof()) {
    throw TGenericException(T_SRCANDLINE,
                            "MaxScriptReader::matchChar()",
                            "Line "
                              + TStringConversion::intToString(lineNum)
                              + ":\n  Unexpected end of line");
  }
  
  if (ifs.peek() != c) {
    throw TGenericException(T_SRCANDLINE,
                            "MaxScriptReader::matchChar()",
                            "Line "
                              + TStringConversion::intToString(lineNum)
                              + ":\n  Expected '"
                              + c
                              + "', got '"
                              + ifs.get()
                              + "'");
  }
  
  ifs.get();
}
  
std::string MaxScriptReader
  ::getRemainingContent(BlackT::TStream& ifs) const {
  std::string content;
  while (!ifs.eof()) content += ifs.get();
  return content;
}

std::string MaxScriptReader::matchName(BlackT::TStream& ifs) const {
  skipSpace(ifs);
  if (ifs.eof()) {
    throw TGenericException(T_SRCANDLINE,
                            "MaxScriptReader::matchName()",
                            "Line "
                              + TStringConversion::intToString(lineNum)
                              + ":\n  Unexpected end of line");
  }
  
  if (!isalpha(ifs.peek())) {
    throw TGenericException(T_SRCANDLINE,
                            "MaxScriptReader::matchName()",
                            "Line "
                              + TStringConversion::intToString(lineNum)
                              + ":\n  Couldn't read name at: "
                              + getRemainingContent(ifs));
  }
  
  std::string result;
  result += ifs.get();
  while (!ifs.eof()
         && (isalnum(ifs.peek()))) {
    result += ifs.get();
  }
  
  return result;
}


}
