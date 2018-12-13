#ifndef MAXDICTCHAROP_H
#define MAXDICTCHAROP_H


#include "max/MaxDictChar.h"
#include "max/MaxDictString.h"
#include "max/MaxScriptOp.h"
//#include "max/MaxScript.h"
#include <vector>

namespace Nes {


typedef MaxDictChar<MaxScriptOp> MaxDictCharOp;
typedef MaxDictString<MaxScriptOp> MaxDictStringOp;

//void scriptToString(const MaxScript& src,
//                    MaxDictStringOp& dst);


}


#endif
