#include "max/MaxDictGenerator.h"
#include "exception/TGenericException.h"
#include <list>
#include <vector>
#include <iostream>

namespace Nes {


const MaxScriptOp MaxDictGenerator::terminatorOp
  (MaxScriptOp::opEnd, 0, 0);
  
MaxDictGenerator::MaxDictGenerator(
                   MaxScriptRegions& origRegions__,
                   MaxDictScripts& dict__,
                   DictRegionMap& regions__,
                   const BlackT::TThingyTable& thingy__,
                   DictGenType genType__,
                   int minEntryLen__,
                   int maxEntryLen__,
                   int maxNumEntries__,
                   int entriesPerScan__,
                   int maxRecursionLevel__)
  : origRegions_(origRegions__),
    dict_(dict__),
    regions_(regions__),
    thingy_(thingy__),
    genType_(genType__),
    minEntryLen_(minEntryLen__),
    maxEntryLen_(maxEntryLen__),
    maxNumEntries_(maxNumEntries__),
    entriesPerScan_(entriesPerScan__),
    maxRecursionLevel_(maxRecursionLevel__) {
  if (entriesPerScan_ < 0) entriesPerScan_ = 1;
}
  
void MaxDictGenerator::operator()() {
  generate();
}
  
void MaxDictGenerator::generate() {
  generateLocalRegions();
  
  for (int i = 0; i < maxNumEntries_; i++) {
    bool result = doOptimalString();
    if (!result) break;
  }
}
                                
void MaxDictGenerator::generateLocalRegions() {
  for (RegionMap::iterator it = origRegions_.regions.begin();
       it != origRegions_.regions.end();
       ++it) {
    const MaxScriptRegion& scripts = it->second;
    regions_[it->first] = MaxDictScripts();
    MaxDictScripts& stringScripts = regions_[it->first];
    
    for (int j = 0; j < scripts.size(); j++) {
      MaxDictStringOp str;
      scripts[j].toDictString(str);
      stringScripts.push_back(str);
    }
  }
}
  
bool MaxDictGenerator::doOptimalString() {
  StringScoreMap stringScores;
  
  switch (genType_) {
  case dictGenChar:
    // count occurrences of all substrings of all possible lengths in the
    // specified range
    for (int i = maxEntryLen_; i >= minEntryLen_; i--) {
      countSubstrings(stringScores, i);
    }
    break;
  case dictGenWord:
    // count occurrences of all substrings of the specified number
    // of words within the given range
    for (int i = maxEntryLen_; i >= minEntryLen_; i--) {
    countSubstringsWord(stringScores, i);
    }
    break;
  default:
    throw BlackT::TGenericException(T_SRCANDLINE,
                            "MaxDictGenerator::doOptimalString()",
                            "Illegal genType_");
    break;
  }
  
  // list of top N most efficient entries (determined by entriesPerScan_)
  std::vector<StringScoreMap::iterator> topEntries;
  topEntries.reserve(entriesPerScan_);
  
  // score each string according to its compression utility and find the
  // most efficient one(s)
  for (StringScoreMap::iterator it = stringScores.begin();
       it != stringScores.end();
       ++it) {
    // must use at least twice to have any real advantage, and length
    // must be at least three to save any space
    if ((it->second.score < 2)
        || (it->first.size() < 3)) it->second.score = 0;
    else {
      // score: numUses * length
      it->second.score = it->second.score * it->first.size();
    }
    
    // if we haven't reached the required number of entries yet, add
    // to list
    if (topEntries.size() < entriesPerScan_) topEntries.push_back(it);
    // otherwise, see if this entry is better than any existing one
    else {
      for (std::vector<StringScoreMap::iterator>::iterator jt
             = topEntries.begin();
           jt != topEntries.end();
           ++jt) {
        if (it->second.score > (*jt)->second.score) {
          (*jt) = it;
          break;
        }
      }
/*      for (int i = 0; i < topEntries.size(); i++) {
        if (it->second.score > topEntries[i]->second.score) {
          topEntries[i] = it;
          break;
        }
      } */
    }
  }
  
  // add top entries to dictionary
  for (std::vector<StringScoreMap::iterator>::iterator it
         = topEntries.begin();
       it != topEntries.end();
       ++it) {
       
/*    std::cout << "dict " << dict_.size() << std::endl;
    std::cout << "  score: " << (*it)->second.score << std::endl;
    const MaxDictStringOp& bestString = (*it)->first;
    for (int i = 0; i < bestString.size(); i++) {
      if (bestString.getItem(i).type() == MaxDictCharOp::typeChar) {
        bestString.getItem(i).asChar().print(std::cout, thingy_);
      }
      else {
        std::cout << "<D " << bestString.getItem(i).asDictMarker().id
          << ">" << std::endl;
      }
    }
    std::cout << std::endl << std::endl; */

    addToDictionary((*it)->first, (*it)->second.inDict);
    
    // if dictionary is full, we're done
    if (dict_.size() >= maxNumEntries_) return false;
  }
/*  for (int i = 0; i < topEntries.size(); i++) {
    addToDictionary(topEntries[i]->first, topEntries[i]->second.inDict);
    
    // if dictionary is full, we're done
    if (dict_.size() >= maxNumEntries_) return false;
  } */
  
  // if we've run out of possible entries, we're done
  if (stringScores.size() <= entriesPerScan_) return false;
  
/*  for (int i = 0; i < entriesPerScan_; i++) {
//  for (int i = 0; i < 1; i++) {
    const MaxDictStringOp* bestString = NULL;
    double bestScore = 0;
    bool inDict = false;
    for (StringScoreMap::iterator it = stringScores.begin();
         it != stringScores.end();
         ++it) {
      
      if (it->second.score > bestScore) {
        bestString = &(it->first);
        bestScore = it->second.score;
        inDict = it->second.inDict;
      }
    }
    
    // if we reach a point where it's impossible to add any more strings,
    // signal that to the caller
    if (bestString == NULL) return false;

//    std::cout << "best: " << bestScore << std::endl;
//    for (int i = 0; i < bestString->size(); i++) {
//      if (bestString->getItem(i).type() == MaxDictCharOp::typeChar) {
//        bestString->getItem(i).asChar().print(std::cout, thingy_);
//      }
//      else {
//        // currently can't happen due to recursive dictionary generation not
//        // being implemented
//        std::cout << "<SHOULDN'T HAPPEN>";
//      }
//    }
//    std::cout << std::endl << std::endl;
    
    addToDictionary(*bestString, inDict);
    
    // if dictionary is full, we're done
    if (dict_.size() >= maxNumEntries_) return false;
    
    // if adding more than one word per scan, remove the word we just added
    // for the next loop
    if (entriesPerScan_ > 1) {
      stringScores.erase(stringScores.find(*bestString));
    }
  } */
  
/*  std::map<double, MaxDictStringOp> sorted;
  for (StringScoreMap::iterator it = stringScores.begin();
       it != stringScores.end();
       ++it) {
    // score: ratio of number of uses to length
//    it->second.score = it->second.score / it->first.size();
    sorted[it->second.score] = it->first;
  }
  for (std::map<double, MaxDictStringOp>::iterator it = sorted.begin();
       it != sorted.end();
       ++it) {
    std::cout << it->first << ":" << std::endl;
    for (int i = 0; i < it->second.size(); i++) {
      it->second.getItem(i).asChar().print(std::cout, thingy_);
    }
    std::cout << std::endl << std::endl;
  } */
  
  return true;
}

void MaxDictGenerator::countSubstrings(
                StringScoreMap& stringScores,
                int len) {
  // score all substrings of the given length in each region
  for (DictRegionMap::iterator it = regions_.begin();
       it != regions_.end();
       ++it) {
    MaxDictScripts& scripts = it->second;
    
    for (int i = 0; i < scripts.size(); i++) {
      MaxDictStringOp& script = scripts[i];
      countScriptSubstringOccurrences(stringScores, script, len);
    }
  }
  
  // also score all substrings of the given length in the dictionary
//  for (MaxDictScripts::iterator it = dict_.begin();
//       it != dict_.end();
//       ++it) {
//    MaxDictStringOp& script = *it;
//    countScriptSubstringOccurrences(stringScores, script, len, true);
//  }
}

void MaxDictGenerator::countSubstringsWord(
                     StringScoreMap& stringScores,
                     int numWords) {
  for (DictRegionMap::iterator it = regions_.begin();
       it != regions_.end();
       ++it) {
    MaxDictScripts& scripts = it->second;
    
    for (int i = 0; i < scripts.size(); i++) {
      MaxDictStringOp& script = scripts[i];
      countScriptSubstringOccurrencesWord(stringScores, script, numWords);
    }
    
//    std::cerr << "pos: " << it->first << std::endl;
  }
  
  // dictionary
  // ...
}

void MaxDictGenerator::countScriptSubstringOccurrences(
                              StringScoreMap& stringScores,
                              const MaxDictStringOp& script,
                              int len,
                              bool inDict) {
  for (int j = 0; j < script.size() - len; j++) {
    
    countSubstringIfValid(stringScores, script, j, len, inDict);
    
  }
}

void MaxDictGenerator::countScriptSubstringOccurrencesWord(
                              StringScoreMap& stringScores,
                              const MaxDictStringOp& script,
                              int numWords,
                              bool inDict) {
  int startpos = 0;
  int endpos = 0;
  
  // advance endpos to end of first word
  advanceWord(script, &endpos, true);
  
  if (endpos == -1) return;
  
  // move endpos to initial position
  for (int i = 0; i < numWords - 1; i++) {
    advanceWord(script, &endpos, true);
    if (endpos == -1) return;
  }
  
  while (endpos != -1) {
    countSubstringIfValid(stringScores, script, startpos,
                          endpos - startpos, inDict);
    
    // advance start and end to next words
    advanceWord(script, &startpos, false);
    advanceWord(script, &endpos, true);
  }
}

void MaxDictGenerator::advanceWord(const MaxDictStringOp& script,
                 int* posP,
                 bool isTrailer) {
  int& pos = *posP;
  // endpos: skip space after current word
  if (isTrailer) {
    skipSpace(script, posP);
    if (pos == -1) return;
  }
  
  // skip word content
  while (pos < script.size()) {
    const MaxDictCharOp& op = script.getItem(pos);
    if (op.type() == MaxDictCharOp::typeChar) {
      const MaxScriptOp& rawop = op.asChar();
      if (rawop.isTerminator()
          || rawop.isSpace()) {
        break;
      }
    }
    
    ++pos;
  }
  
  // startpos: skip space before next word
  if (!isTrailer) {
    skipSpace(script, posP);
  }
}

void MaxDictGenerator::skipSpace(const MaxDictStringOp& script,
               int* posP) {
  int& pos = *posP;
  
  if (pos >= script.size()) {
    throw BlackT::TGenericException(T_SRCANDLINE,
                            "MaxDictGenerator::advanceWord()",
                            "Unterminated script (1)");
  }
  
  // if first character encountered is terminator, we've reached end of script:
  // set pos to sentinel value and return
  {
    const MaxDictCharOp& op = script.getItem(pos);
    if (op.type() == MaxDictCharOp::typeChar) {
      const MaxScriptOp& rawop = op.asChar();
      if (rawop.isTerminator()) {
        pos = -1;
        return;
      }
    }
  }
  
  while (true) {
    // every valid script must have a terminator for us to find
    if (pos >= script.size()) {
      throw BlackT::TGenericException(T_SRCANDLINE,
                              "MaxDictGenerator::advanceWord()",
                              "Unterminated script (2)");
    }
    
    const MaxDictCharOp& op = script.getItem(pos);
    if (op.type() == MaxDictCharOp::typeChar) {
      const MaxScriptOp& rawop = op.asChar();
      // if at terminator anywhere other than beginning of check, stop and
      // leave it for us to find on next pass
      if (rawop.isTerminator()) {
        return;
      }
      else if (!rawop.isSpace()) {
        return;
      }
    }
    
    ++pos;
  }
}

void MaxDictGenerator::countSubstringIfValid(
                           StringScoreMap& stringScores,
                           const MaxDictStringOp& script,
                           int pos,
                           int len,
                           bool inDict) {
  if ((len > 0) && (isDictionaryable(script, pos, len))) {
    
/*    for (int i = pos; i < pos + len; i++) {
      const MaxDictCharOp& op = script.getItem(i);
      if (op.type() == MaxDictCharOp::typeChar) {
        const MaxScriptOp& subop = op.asChar();
        if (subop.type == MaxScriptOp::opTilemap) {
//          std::cerr << "why" << std::endl;
          std::cerr << pos << " " << len
            << " " << script.size() << " " << i << std::endl;
        }
      }
    } */

    MaxDictStringOp str = script.substr(pos, len);
    
/*    for (int i = 0; i < str.size(); i++) {
      const MaxDictCharOp& op = str.getItem(i);
      if (op.type() == MaxDictCharOp::typeChar) {
        const MaxScriptOp& subop = op.asChar();
        if (subop.type == MaxScriptOp::opTilemap) {
          std::cerr << "why" << std::endl;
        }
      }
    } */
    
    StringScoreMap::iterator findIt = stringScores.find(str);
    if (findIt == stringScores.end()) {
      stringScores[str] = ScoreStruct();
      findIt = stringScores.find(str);
    }

    ++(findIt->second.score);
    findIt->second.inDict = inDict;
  }
}

/*int MaxDictGenerator::countSubstringOccurences(StringScoreMap& stringScores,
                              const MaxDictStringOp& srcStr,
                              int len) {
  int count = 0;
  for (DictRegionMap::iterator it = regions_.begin();
       it != regions_.end();
       ++it) {
    MaxDictScripts& scripts = it->second;
    
    for (int i = 0; i < scripts.size(); i++) {
      MaxDictStringOp& script = scripts[i];
      
      count += countScriptSubstringOccurrences(
        stringScores, script, srcStr, len);
      
    }
  }
  
  return count;
} */

/*int MaxDictGenerator::countScriptSubstringOccurrences(
                              StringScoreMap& stringScores,
                              const MaxDictStringOp& script,
                              const MaxDictStringOp& srcStr,
                              int len) {
  int count = 0;
  for (int i = 0; i < script.size() - len; i++) {
    if ((isDictionaryable(script, i, len))
        && (srcStr.compare(script, 0, i, len) == true)) {
      ++count;
    }
    
  }
  
  return count;
} */

bool MaxDictGenerator::isDictionaryable(const MaxDictStringOp& srcStr,
                      int srcPos,
                      int len) const {
  len += srcPos;
  for (int i = srcPos; i < len; i++) {
    const MaxDictCharOp& charOp = srcStr.getItem(i);
    
    // The following cannot be placed in the dictionary:
    // * Opcode 9F [END]
    // * Tilemap data
    // * Any substring containing a dictionary marker that already has a
    //   recursion depth at the maximum level
    
    if (charOp.type() == MaxDictCharOp::typeChar) {
      const MaxScriptOp& op = charOp.asChar();
      if (!op.isDictionaryable()) return false;
    }
    else if (charOp.type() == MaxDictCharOp::typeDictMarker) {
    
      // never do recursive dictionary lookups
//      return false;
    
      // disallow recursive lookups beyond the maximum level
      const MaxDictMarker& marker = charOp.asDictMarker();
      if ((marker.recursionLevel >= maxRecursionLevel_)) return false;

    }
  }
  
  return true;
}

void MaxDictGenerator::addToDictionary(
                     const MaxDictStringOp& str,
                     bool inDict) {
  int len = str.size();
  
  if (inDict) {
    // if this string appears in the dictionary, we must add a level of
    // recursion to anything that references its index or indices
    
    // if any referencers are already at the recursion limit, they cannot
    // be replaced and are ignored
    
    // not writing this until i have to
    // ...
  }
  
  DictRegionMap newRegions;
  for (DictRegionMap::iterator it = regions_.begin();
       it != regions_.end();
       ++it) {
    newRegions[it->first] = MaxDictScripts();
    MaxDictScripts& newScripts = newRegions[it->first];
    
    MaxDictScripts& scripts = it->second;
    
    for (int i = 0; i < scripts.size(); i++) {
      newScripts.push_back(MaxDictStringOp());
      MaxDictStringOp& newScript = newScripts.back();
    
      MaxDictStringOp& script = scripts[i];
      
      int pos = 0;
      while (pos < script.size()) {
//        MaxDictStringOp checkstr = script.substr(j, len);
        
        int recursionLevel = -1;
        if ((script.size() - pos < len)
            || (str.compare(script, 0, pos, len) != true)
            || (!isDictionaryable(script, pos, len))
            || ((recursionLevel = substrRecursionLevel(script, pos, len))
                  >= maxRecursionLevel_)) {
          // no match: copy character and continue
          newScript.pushBack(script.getItem(pos));
          ++pos;
          continue;
        }
        else {
          // match: replace with dictionary marker
          
          // insert dictionary marker with increased recursion level
          // (recursionLevel will be -1 at this point if the substring was not
          // initially recursive)
          MaxDictCharOp op;
          op.changeDictMarker(dict_.size(), recursionLevel + 1);
          newScript.pushBack(op);
          
          pos += len;
        }
        
      }
      
      // add remaining content from end of script
/*      int copystart = script.size() - len;
      if (copystart < 0) copystart = 0;
      for (int j = copystart; j < script.size(); j++) {
        newScript.pushBack(script.getItem(j));
      } */
    }
  
//  std::cerr << "here" << std::endl;
  }
  
  dict_.push_back(str);
  // add terminator to dictionary entry
  MaxDictCharOp op;
  op.changeChar(&terminatorOp);
  dict_.back().pushBack(op);

/*  std::cout << "ADDED (" << std::dec << dict_.size() - 1 << "): " << std::endl;
  for (int i = 0; i < dict_.back().size(); i++) {
    if (dict_.back().getItem(i).type() == MaxDictCharOp::typeChar) {
      dict_.back().getItem(i).asChar().print(std::cout, thingy_);
    }
    else {
      std::cout << "<D " << dict_.back().getItem(i).asDictMarker().id
        << ">";
    }
  }
  std::cout << std::endl << std::endl; */
  
  regions_ = newRegions;
}
  
int MaxDictGenerator::substrRecursionLevel(
                         const MaxDictStringOp& script,
                         int pos,
                         int len) {
  // a result of -1 will be returned if the string is not recursive
  // (contains no dictionary markers)
  int result = -1;
  
  len += pos;
  for (int i = pos; i < len; i++) {
    const MaxDictCharOp& op = script.getItem(i);
    if (op.type() != MaxDictCharOp::typeDictMarker) continue;
    const MaxDictMarker& marker = op.asDictMarker();
    if (marker.recursionLevel > result) result = marker.recursionLevel;
  }
  
  return result;
}


}
