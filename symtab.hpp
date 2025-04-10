#ifndef __SYMTAB__
#define __SYMTAB__

#include <string>
#include <unordered_map>
using namespace std;

struct SymbolTableEntry {
  string identifier;

  // Normally other info should be included here, in our given problem
  // description we don't need anything extra.
};

class SymbolTable {
  unordered_map<string, SymbolTableEntry> data;
public:

  bool insert(const string& identifier, const SymbolTableEntry& entry = SymbolTableEntry()) {
    bool flag = data.find(identifier) == data.end();
    data[identifier] = entry;
    data[identifier].identifier = identifier;
    return flag;
  }

  SymbolTableEntry& lookup(const string& identifier) {return data.at(identifier);}
  const SymbolTableEntry& lookup(const string& identifier) const {return data.at(identifier);}

  using iterator = decltype(data)::iterator;
  using const_iterator = decltype(data)::const_iterator;
  using FullEntry = decltype(data)::value_type;

  iterator begin() {return data.begin();}
  const_iterator begin() const {return data.begin();}

  iterator end() {return data.end();}
  const_iterator end() const {return data.end();}
};

#endif