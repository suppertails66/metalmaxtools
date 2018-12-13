#ifndef MAXDICTMARKER_H
#define MAXDICTMARKER_H


namespace Nes {


class MaxDictMarker {
public:
  
  MaxDictMarker();
  MaxDictMarker(int id__, int recursionLevel__);
  
  bool operator==(const MaxDictMarker& src) const;
  bool operator<(const MaxDictMarker& src) const;
  
  int id;
  int recursionLevel;
  
protected:
  
};


}


#endif
