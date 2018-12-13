#ifndef MAXDICTGENERATOR_H
#define MAXDICTGENERATOR_H


#include "max/MaxScriptRegions.h"
#include "max/MaxDictCharOp.h"
#include "util/TThingyTable.h"
#include <vector>
#include <map>
#include <unordered_map>

namespace Nes {


typedef std::vector<MaxDictStringOp> MaxDictScripts;
typedef std::map< int, MaxDictScripts > DictRegionMap;
  
class MaxDictGenerator {
public:
  enum DictGenType {
    dictGenChar,
    dictGenWord
  };

  MaxDictGenerator(MaxScriptRegions& origRegions__,
                   MaxDictScripts& dict__,
                   DictRegionMap& regions__,
                   const BlackT::TThingyTable& thingy__,
                   DictGenType genType__ = dictGenChar,
                   int minEntryLen__ = 3,
                   int maxEntryLen__ = 4,
                   int maxNumEntries__ = 512,
                   int entriesPerScan__ = 1,
                   int maxRecursionLevel__ = 0);
  
  void operator()();
  
  void generate();
protected:
  // Static terminator op.
  // Dictionary entries need to have a terminator to point to; this provides
  // it for them.
  const static MaxScriptOp terminatorOp;

  class ScoreStruct {
  public:
    ScoreStruct()
      : score(0),
        inDict(false) { }
  
    double score;
    bool inDict;
  };

  class HashCompare {
  public:
    int operator()(const MaxDictStringOp& op) const {
      int sum = 0;
      for (int i = 0; i < op.size(); i++) {
        const MaxDictCharOp& sub = op.getItem(i);
        if (sub.type() == MaxDictCharOp::typeChar) {
          int value = sub.asChar().type;
          if (value != 0) {
            sum *= value;
            sum += value;
          }
        }
        else if (sub.type() == MaxDictCharOp::typeChar) {
          int value = sub.asDictMarker().id;
          if (value != 0) {
            sum *= value;
            sum += value;
          }
        }
      }
      return sum;
    }
  };
  
//  typedef std::map<MaxDictStringOp, int> StringScoreMap;
  typedef std::unordered_map<
      MaxDictStringOp, ScoreStruct, HashCompare> StringScoreMap;

  MaxScriptRegions& origRegions_;
  MaxDictScripts& dict_;
  DictRegionMap& regions_;
  // this table isn't at all necessary -- it's just used for debug output
  const BlackT::TThingyTable& thingy_;
  DictGenType genType_;
  int maxNumEntries_;
  int minEntryLen_;
  int maxEntryLen_;
  int entriesPerScan_;
  int maxRecursionLevel_;
  
//  std::vector< MaxDictScripts > regions_;
  
  void generateLocalRegions();
  
  bool doOptimalString();
  
  void countSubstrings(StringScoreMap& stringScores,
                       int len);
  void countSubstringsWord(StringScoreMap& stringScores,
                       int numWords);
  void countScriptSubstringOccurrences(StringScoreMap& stringScores,
                                const MaxDictStringOp& script,
                                int len,
                                bool inDict = false);
  void countScriptSubstringOccurrencesWord(StringScoreMap& stringScores,
                                const MaxDictStringOp& script,
                                int numWords,
                                bool inDict = false);
  void countSubstringIfValid(StringScoreMap& stringScores,
                             const MaxDictStringOp& script,
                             int pos,
                             int len,
                             bool inDict);
  void advanceWord(const MaxDictStringOp& script,
                   int* posP,
                 bool isTrailer = false);
  void skipSpace(const MaxDictStringOp& script,
                 int* posP);
  
  void addToDictionary(const MaxDictStringOp& str,
                       bool inDict = false);
                                
/*  int countSubstringOccurences(StringScoreMap& stringScores,
                                const MaxDictStringOp& srcStr,
                                int len); */
/*  int countScriptSubstringOccurrences(StringScoreMap& stringScores,
                                const MaxDictStringOp& script,
                                const MaxDictStringOp& srcStr,
                                int len); */
  bool isDictionaryable(const MaxDictStringOp& srcStr,
                        int srcPos,
                        int len) const;
  
  int substrRecursionLevel(const MaxDictStringOp& script,
                           int pos,
                           int len);
  
};


}


#endif
