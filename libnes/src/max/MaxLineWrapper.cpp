#include "max/MaxLineWrapper.h"
#include "util/TParse.h"
#include "util/TStringConversion.h"
#include "exception/TGenericException.h"
#include <iostream>

using namespace BlackT;

namespace Nes {

// arbitrary names assigned to speaker IDs for use in SETSPKR directives
const static std::string spkrIdTable[] = {
  // 0x00
  "anon",
  "dad",
  "sis",
  "mortem",
  "igor",
  "wolf",
  "blank",
  "char2",
  // 0x08
  "char2dad",
  "char2mom",
  "char3",
  "ulie",
  "maria",
  "akemi",
  "hitomi",
  "paula",
  // 0x10
  "paul",
  "char1",
  "caesar",
  "tsumaku",
  "nana",
  "beverly",
  "shijima",
  "himio",
  // 0x18
  "muscle",
  "yazoo",
  "gomez",
  "nina",
  "sergeant",
  "molina",
  "optica",
  "valdez",
  // 0x20
  "null",
  "marilyn",
  "noah",
  
};
const static int numSpkrIds = sizeof(spkrIdTable) / sizeof(std::string);

// when printing SPKR-formatted dialogue, X is reset to this value after
// each linebreak
const static int spkrLineInitialX = 0;
// and this value after a box clear
const static int spkrBoxInitialX = -2;

const static int lowestOpcode = 0xDF;

const static int code_END     = 0x9F;
const static int code_opE2    = 0xE2;
const static int code_TCHOICE = 0xE3;
const static int code_opE4    = 0xE4;
const static int code_BR      = 0xE5;
const static int code_opE6    = 0xE6;
const static int code_BR2     = 0xE7;
const static int code_HERO    = 0xE8;
const static int code_opE9    = 0xE9;
const static int code_opEA    = 0xEA;
const static int code_CHOICE  = 0xEB;
const static int code_SKIP    = 0xED;
const static int code_SPKR    = 0xF0;
const static int code_opF1    = 0xF1;
const static int code_opF8    = 0xF8;
const static int code_NVAR    = 0xF9;
const static int code_opFA    = 0xFA;
const static int code_opFB    = 0xFB;
const static int code_IP_SVAR = 0xFC;
const static int code_SVAR    = 0xFD;
const static int code_KEY     = 0xFE;
const static int code_SPACE   = 0xFF;

// added for translation!

// wait for key, but don't automatically do a linebreak afterwards
const static int code_KEYNOBR = 0xDF;
// linebreak for only "half" a line (single-spaced)
const static int code_HALFBR = 0xE1;

MaxLineWrapper::MaxLineWrapper(BlackT::TStream& src__,
                ResultCollection& dst__,
                const BlackT::TThingyTable& thingy__,
                SpkrSizeTable spkrSizeTable__,
                int xSize__,
                int ySize__)
  : TLineWrapper(src__, dst__, thingy__, xSize__, ySize__),
    spkrOn(false),
    curSpkr(0),
    breakMode(breakModeBr),
    spkrSizeTable(spkrSizeTable__) {
  
}

int MaxLineWrapper::widthOfKey(int key) {
  // HERO
//  if ((key == code_HERO)) return 4;
  if ((key == code_HERO)) return 8;
  // SPKR
//  else if ((key == code_SPKR)) return 8;
  // NVAR
  else if ((key == code_NVAR)) return 4;
  // SVAR (FIXME)
//  else if ((key == code_SVAR)) return 4;
  else if ((key == code_SVAR)) return 8;
  // space
  else if ((key == code_SPACE)) return 1;
  // end/control code
  else if ((key == code_END) || (key >= lowestOpcode)) return 0;
  
  // note that opE2, opE9, etc., are not handled, since they run game-defined
  // script and we have no idea what the bound on the size of the output
  // is until runtime. all occurrences of these must be handled manually.
  
  return 1;
}

bool MaxLineWrapper::isWordDivider(int key) {
/*  if (
      // space
      ((key >= 0x01) && (key <= 0x0D))
      // END
      || (key == code_END)
      // BR
      || (key == code_BR)
      // BR2
      || (key == code_BR2)
      // space
      || (key == code_SPACE)
     ) return true; */
  if (
      // END
      (key == code_END)
      // space
      || (key == code_SPACE)
      || (key == code_CHOICE)
      || (key == code_TCHOICE)
      || (key == code_BR)
      || (key == code_BR2)
      || (key == code_SKIP)
     ) return true;
     
/*  if (
      (key < lowestOpcode)
      // HERO
      || (key == code_HERO)
      // NVAR
      || (key == code_NVAR)
      // SVAR
      || (key == code_SVAR)
      || (key == code_opE2)
      || (key == code_opE9)
      || (key == code_opF1)
      || (key == code_opF8)
      || (key == code_opFA)
      || (key == code_IP_SVAR)
     ) return false;
  
  return true; */
  
  return false;
}

bool MaxLineWrapper::isLinebreak(int key) {
  if (
      // TCHOICE
//      (key == code_TCHOICE)
      // BR
      (key == code_BR)
      // BR2
      || (key == code_BR2)
      || (key == code_HALFBR)
      // CHOICE
//      || (key == code_CHOICE)
      // KEY
//      || (key == code_KEY)
      ) return true;
  
  return false;
}

bool MaxLineWrapper::isBoxClear(int key) {
  // END
  if ((key == code_END)
      // KEY
      || (key == code_KEY)
      // CHOICE
      || (key == code_CHOICE)
      // TCHOICE
      || (key == code_TCHOICE)
      // SPKR
      || (key == code_SPKR)
      || (key == code_opE4)
      // KEYNOBR (new for translation)
      || (key == code_KEYNOBR)) return true;
  
  return false;
}

void MaxLineWrapper::onBoxFull() {
//  if (lineHasContent) {
    std::string content;
  //      std::cerr << "x" << std::endl;
    if (lineHasContent) {
      // wait
      content = thingy.getEntry(code_KEY);
      currentScriptBuffer.write(content.c_str(), content.size());
    }
    // clear
//    content = thingy.getEntry(0x43);
//    currentScriptBuffer.write(content.c_str(), content.size());
    // linebreak
    stripCurrentPreDividers();
    
//    outputLinebreak();
    
//    content = thingy.getEntry(0x44);
//    currentScriptBuffer.write(content.c_str(), content.size());
//    stripCurrentPreDividers();
    
    currentScriptBuffer.put('\n');
    xPos = 0;
    yPos = 0;

//  std::cerr << "WARNING: line " << lineNum << ":" << std::endl;
//  std::cerr << "  overflow at: " << std::endl;
//  std::cerr << streamAsString(currentScriptBuffer)
//    << std::endl
//    << streamAsString(currentWordBuffer) << std::endl;

//  if (spkrOn) {
//    xPos = spkrBoxInitialX;
//  }
}

int MaxLineWrapper::linebreakKey() {
  switch (breakMode) {
  case breakModeBr:
    return code_BR;
    break;
  case breakModeBr2:
    return code_BR2;
    break;
  default:
    return code_BR;
    break;
  }
}

void MaxLineWrapper::onSymbolAdded(BlackT::TStream& ifs, int key) {
/*  if (key == 0x42) waitPending = true;
  else {
    if (
        // end
        (key == 0x00)
        // other
        || ((key >= 0x40) && (key <= 0x4B))
        // close
        || (key == 0x7F)
       ) {
      
    }
    else {
      waitPending = false;
    }
  } */
  
  if (!ifs.eof()) {
    // SPKR
    if (key == code_SPKR) {
      int pos = ifs.tell();
      
      // grab speaker code from next byte, which must be raw
      Symbol sym = fetchNextSymbol(ifs);
      if (sym.raw) {
        curSpkr = fromLitString(sym.litstr);
        spkrOn = true;
        
//        applySpkrOffset();
//        std::cerr << sym.litstr << " " << curSpkr << std::endl;
      }
      
      ifs.seek(pos);
    }
  }
}

void MaxLineWrapper::afterLinebreak(
    LinebreakSource clearSrc, int key) {
  if (clearSrc != linebreakBoxEnd) {
    if (spkrOn) {
      xPos = spkrLineInitialX;
    }
  }
}

void MaxLineWrapper::beforeBoxClear(
    BoxClearSource clearSrc, int key) {
  if ((clearSrc == boxClearManual)
      && ((key == code_CHOICE) || (key == code_TCHOICE))) {
    // CHOICE and TCHOICE act as box clears, but they scroll the top line
    // off the screen to make room for the yes/no prompt.
    // If we used all four lines in the most recent box, we need to insert
    // a KEY before the CHOICE/TCHOICE to make sure the top line isn't
    // removed before it's presented to the player.
    //
    // ...except okay, the game inserts an erroneous extra linebreak if you
    // try to do a KEY, then a CHOICE immediately afterward.
    // then I guess we'll just have to manually edit every instance of this!
    // yay!
    // in principle this is automatable, but we'd have to keep track of where
    // the previous line ended and it's probably not worth the trouble here.
    //
    // but wait, now i've implemented a new opcode that does a WAIT with no
    // linebreak, so just do that
    if ((ySize != -1) && (yPos >= (ySize - 1))) {
      // solution 1! (abandoned)
//      onBoxFull();

      // solution 2! (abandoned)
//      std::cout << "Line " << lineNum << ":" << std::endl
//        << "  WARNING: Full box followed by CHOICE/TCHOICE!" << std::endl
//        << "           Top line will be scrolled out without pausing."
//        << std::endl;
      
      // solution 3! still here for now!!
      std::string content = thingy.getEntry(code_KEYNOBR);
      currentScriptBuffer.write(content.c_str(), content.size());
    }
    
  }
}

void MaxLineWrapper::afterBoxClear(
    BoxClearSource clearSrc, int key) {
//  if (spkrOn) {
//    xPos = spkrBoxInitialX;
//  }

  // SPKR is on, and conditions for displaying SPKR prefix are met:
  // a. current output char is "first" in a message (i.e. the previous output
  //    char was END, CHOICE, or TCHOICE)
  // b. current output char is the next following a KEY
  if (spkrOn) {
    // this is how it works in the original game: speaker name is
    // unconditionally printed after every keywait
//    applySpkrOffset();

    // but that's really awkward when sentences span multiple boxes, as
    // we frequently need to do in the translation, so the game has been
    // modified to print the speaker only at the box start or after a
    // SPKR command
//    if ((key == code_END) || (key == code_CHOICE)
//        || (key == code_TCHOICE) || (key == code_SPKR)) {
    if (((clearSrc == boxClearManual) && (key != code_KEY))
        || (key == code_SPKR)) {
      applySpkrOffset();
    }
  }

  // op E4 pauses but does not automatically break the line
  if (((clearSrc == boxClearManual) && (key == code_opE4))) {
    yPos = -1;
  }
}

bool MaxLineWrapper::processUserDirective(BlackT::TStream& ifs) {
  TParse::skipSpace(ifs);
  
  std::string name = TParse::matchName(ifs);
  TParse::matchChar(ifs, '(');
  
  for (int i = 0; i < name.size(); i++) {
    name[i] = toupper(name[i]);
  }
  
  if (name.compare("SPKRON") == 0) {
    spkrOn = true;
    
    // default to SPKR 0 == anonymous
    curSpkr = 0;
    applySpkrOffset();
    
    return true;
  }
  else if (name.compare("SPKROFF") == 0) {
    spkrOn = false;
    
    // reset any offset from end of previous box
    xPos = 0;
    
    return true;
  }
  else if (name.compare("SETSPKR") == 0) {
//    curSpkr = TParse::matchInt(ifs);
    std::string curSpkrName = TParse::matchName(ifs);
    
    if (curSpkrName.compare("OFF") == 0) {
      curSpkr = -1;
    }
    else {
      bool spkrExists = false;
      for (int i = 0; i < numSpkrIds; i++) {
        if (curSpkrName.compare(spkrIdTable[i]) == 0) {
          curSpkr = i;
          spkrExists = true;
          break;
        }
      }
      
      if (!spkrExists) {
        throw TGenericException(T_SRCANDLINE,
                                "MaxLineWrapper::processUserDirective()",
                                "Line "
                                  + TStringConversion::intToString(lineNum)
                                  + ":\n  Unknown speaker ID: "
                                  + curSpkrName);
      }
    }
    
    // couldn't decide whether to make spkrOn a separate variable or not, so
    // let's have the worst of all worlds!
    if (curSpkr > 0) {
      spkrOn = true;
      applySpkrOffset();
    }
    else {
      spkrOn = false;
      xPos = 0;
    }
    
    return true;
  }
  else if (name.compare("SETBREAKMODE") == 0) {
    std::string breakModeName = TParse::matchName(ifs);
    
    if (breakModeName.compare("BR") == 0) {
      breakMode = breakModeBr;
    }
    else if (breakModeName.compare("BR2") == 0) {
      breakMode = breakModeBr2;
    }
    else {
      throw TGenericException(T_SRCANDLINE,
                              "MaxLineWrapper::processUserDirective()",
                              "Line "
                                + TStringConversion::intToString(lineNum)
                                + ":\n  Unknown linebreak mode: "
                                + breakModeName);
    }
    
    return true;
  }
  
  return false;
}

void MaxLineWrapper::applySpkrOffset() {
  xPos = spkrBoxInitialX;
//  xPos += 8;
  xPos += spkrSizeTable[curSpkr];
//  std::cout << std::hex << curSpkr << " " << spkrSizeTable[curSpkr] << std::endl;
}

}
