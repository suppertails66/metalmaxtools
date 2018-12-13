#ifndef MAXDICTSTRING_H
#define MAXDICTSTRING_H


#include "max/MaxDictChar.h"
#include "util/TStringConversion.h"
#include "exception/TGenericException.h"
#include <list>
#include <vector>
#include <cmath>
#include <iostream>

namespace Nes {


template <class T>
class MaxDictString {
public:
  const static int npos = -1;
  
  typedef std::vector< MaxDictChar<T> > DictCharCollection;

  MaxDictString<T>() { }
  
  bool operator==(const MaxDictString<T>& src) const {
    return compare(src);
  }
  
  bool operator!=(const MaxDictString<T>& src) const {
    return !(*this == src);
  }
  
  bool operator<(const MaxDictString<T>& src) const {
    int sz = std::min(chars_.size(), src.chars_.size());
    for (int i = 0; i < sz; i++) {
      const MaxDictChar<T>& a = chars_[i];
      const MaxDictChar<T>& b = src.chars_[i];
      
      if (a < b) return true;
      else if (b < a) return false;
    }
    
    // if everything matches, shorter string wins
    if (chars_.size() < src.chars_.size()) return true;
    else if (chars_.size() > src.chars_.size()) return false;
    
    return false;
  }
  
  MaxDictString<T>& operator=(const MaxDictString<T>& src) {
    clone(src);
    return *this;
  }
  
  MaxDictString<T>(const MaxDictString<T>& src) {
    clone(src);
  }
  
  MaxDictString<T>& operator+(const MaxDictString<T>& src) {
    append(src);
    return *this;
  }
  
  void append(const MaxDictString<T>& src,
              int pos = 0,
              int len = npos) {
    if (len == npos) len = src.size() - pos;
    if (pos < 0) return;
    
//    if (pos + len > src.size()) return;
//    if ((pos + len) > src.size()) len = src.size() - pos;
    
    len += pos;
    for (int i = pos; i < len; i++) {
      chars_.push_back(src.chars_[i]);
    }
  }
  
  bool compare(const MaxDictString<T>& src,
               int myOffset = 0,
               int otherOffset = 0,
               int length = npos) const {
/*    typename DictCharCollection::const_iterator it
      = chars_.begin() + myOffset;
    typename DictCharCollection::const_iterator jt
      = src.chars_.begin() + otherOffset; */
    
    if (length == npos) {
      length = std::min((chars_.size() - myOffset),
                        (src.chars_.size() - otherOffset));
    }
    
    length = std::min((int)length, (int)(chars_.size() - myOffset));
    length = std::min((int)length, (int)(src.chars_.size() - otherOffset));
    
/*    for (int i = 0; i < length; i++) {
      if (*(it++) != *(jt++)) return false;
    } */
    
    for (int i = 0; i < length; i++) {
      if (chars_[i + myOffset] != src.chars_[i + otherOffset])
        return false;
    }
    
    return true;
  }
  
  int size() const {
    return chars_.size();
  }
  
  void pushBack(const MaxDictChar<T>& src) {
    chars_.push_back(src);
  }
  
  void appendChar(const T& t) {
    chars_.push_back(MaxDictChar<T>());
    chars_.back().changeChar(&t);
  }
  
  const MaxDictChar<T>& getItem(int index) const {
    if (index >= chars_.size()) {
      throw BlackT::TGenericException(T_SRCANDLINE,
                              "MaxDictString<T>::getItem()",
                              "Out-of-range access: "
                                + BlackT::TStringConversion::intToString(
                                    index));
    }
    
    return chars_[index];
  }
  
  void changeChar(int index, const T& t) {
    if (index >= chars_.size()) {
      throw BlackT::TGenericException(T_SRCANDLINE,
                              "MaxDictString<T>::setChar()",
                              "Out-of-range access: "
                                + BlackT::TStringConversion::intToString(
                                    index));
    }
    
    chars_[index].changeChar(&t);
  }
  
  void changeDictMarker(int index, int id, int recursionLevel) {
    if (index >= chars_.size()) {
      throw BlackT::TGenericException(T_SRCANDLINE,
                              "MaxDictString<T>::setChar()",
                              "Out-of-range access: "
                                + BlackT::TStringConversion::intToString(
                                    index));
    }
    
    chars_[index].changeDictMarker(id, recursionLevel);
  }
  
  void clear() {
    chars_.clear();
  }
  
  MaxDictString<T> substr(int pos, int len = npos) const {
    if (len == npos) len = size() - pos;
    if ((len <= 0) || (pos < 0)) return MaxDictString<T>();
    
    MaxDictString<T> result;
    
//    if ((len <= 0) || (pos < 0)) return result;
    
    result.append(*this, pos, len);
    return result;
  }
  
protected:
  DictCharCollection chars_;
  
  void clone(const MaxDictString<T>& src) {
    chars_.clear();
    for (typename DictCharCollection::const_iterator it
          = src.chars_.begin();
         it != src.chars_.end();
         ++it) {
      chars_.push_back(*it);
    }
  }
  
};


}


#endif
