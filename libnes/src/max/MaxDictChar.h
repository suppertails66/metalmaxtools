#ifndef MAXDICTCHAR_H
#define MAXDICTCHAR_H


#include "max/MaxDictMarker.h"
#include "exception/TGenericException.h"
#include <cstdlib>
#include <iostream>

namespace Nes {


template <class T>
class MaxDictChar {
public:
  enum Type {
    typeNone,
    typeChar,
    typeDictMarker
  };

  MaxDictChar<T>()
    : type_(typeNone),
      data_(NULL) { }
  
  ~MaxDictChar<T>() {
    destroy();
  }
  
  MaxDictChar<T>& operator=(const MaxDictChar<T>& src) {
    clone(src);
    return *this;
  }
  
  MaxDictChar<T>(const MaxDictChar<T>& src)
    : type_(typeNone),
      data_(NULL) {
    clone(src);
  }
  
  bool operator==(const MaxDictChar<T>& src) const {
    if (type_ != src.type_) return false;
    
    switch (type_) {
    case typeChar:
      return (*static_cast<T*>(data_)
        == *static_cast<T*>(src.data_));
      break;
    case typeDictMarker:
      return (*static_cast<MaxDictMarker*>(data_)
        == *static_cast<MaxDictMarker*>(src.data_));
      break;
    default:
      throw BlackT::TGenericException(T_SRCANDLINE,
                              "MaxDictChar<T>::operator==()",
                              "Illegal type");
      break;
    }
  }
  
  bool operator!=(const MaxDictChar<T>& src) const {
    return !(*this == src);
  }
  
  bool operator<(const MaxDictChar<T>& src) const {
    // if types don't match, char wins over marker
    if (type_ != src.type_) {
      if (type_ == typeChar) return true;
      else return false;
    }
    
    if (type_ == typeChar) {
      return (asChar() < src.asChar());
    }
    else if (type_ == typeDictMarker) {
      return (asDictMarker() < src.asDictMarker());
    }
    
    // equal
    return false;
  }
  
  void changeChar(const T* data__) {
    destroy();
    type_ = typeChar;
    // i'm sure we know what we're doing
    data_ = (void*)(const_cast<T*>(data__));
  }
  
  void changeDictMarker(int id, int recursionLevel) {
    if (type_ != typeDictMarker) {
      destroy();
      type_ = typeDictMarker;
      data_ = (void*)(new MaxDictMarker(id, recursionLevel));
    }
    else {
      *static_cast<MaxDictMarker*>(data_) = MaxDictMarker(id, recursionLevel);
    }
  }
  
  const T& asChar() const {
    if (type_ != typeChar) {
      throw BlackT::TGenericException(T_SRCANDLINE,
                              "MaxDictChar<T>::asChar() const",
                              "asChar() on non-char type");
    }
    
    return *(static_cast<T*>(data_));
  }
  
  T& asChar() {
    if (type_ != typeChar) {
      throw BlackT::TGenericException(T_SRCANDLINE,
                              "MaxDictChar<T>::asChar()",
                              "asChar() on non-char type");
    }
    
    return *(static_cast<T*>(data_));
  }
  
  const MaxDictMarker& asDictMarker() const {
    if (type_ != typeDictMarker) {
      throw BlackT::TGenericException(T_SRCANDLINE,
                              "MaxDictChar<T>::asDictMarker() const",
                              "asDictMarker() on non-dictMarker type");
    }
    
    return *(static_cast<MaxDictMarker*>(data_));
  }
  
  MaxDictMarker& asDictMarker() {
    if (type_ != typeDictMarker) {
      throw BlackT::TGenericException(T_SRCANDLINE,
                              "MaxDictChar<T>::asDictMarker()",
                              "asDictMarker() on non-dictMarker type");
    }
    
    return *(static_cast<MaxDictMarker*>(data_));
  }
  
  Type type() const {
    return type_;
  }
  
protected:
  Type type_;
  void* data_;
  
  void destroy() {
    switch (type_) {
    case typeDictMarker:
//      std::cerr << std::hex << (int)((static_cast<T*>(data_))->type) << std::endl;
      delete static_cast<MaxDictMarker*>(data_);
      break;
    default:
      
      break;
    }
  }
  
  void clone(const MaxDictChar<T>& src) {
    switch (src.type_) {
    case typeChar:
      changeChar(static_cast<T*>(src.data_));
      break;
    case typeDictMarker:
    {
      const MaxDictMarker* marker
        = static_cast<const MaxDictMarker*>(src.data_);
      changeDictMarker(marker->id, marker->recursionLevel);
    }
      break;
    default:
      
      break;
    }
  }

};


}


#endif
