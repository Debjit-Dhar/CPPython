#include "parser.hpp"
#include <algorithm>
#include <stack>
#include <variant>
#include <utility>
using namespace std;

ostream &operator<<(ostream &out, const Item &item) {
  out << '<' << item.getNonterminal() << "> ::= ";
  for (size_t i = 0; i < item.getRule().size(); i++) {
    if (i == item.position())
      out << ". ";
    out << item.getRule()[i];
    if (i < (item.getRule().size() - 1))
      out << ' ';
  }
  if(item.position() == item.getRule().size()) out << " .";
  return out;
}

ostream &operator<<(ostream& out, const vector<Item>& items) {
  for(const Item& item : items) out << item << endl;
  return out;
}

vector<Item> sortedItems(const ItemSet &itemSet, const Language &l) {
  struct ItemComparator {
    unordered_map<string, size_t> keys;
    ItemComparator(const Language &l) {
      for (size_t i = 0; i < l.order.size(); i++)
        keys[l.order[i]] = i;
    }
    // is left < right?
    bool operator()(const Item &left, const Item &right) const {
      auto k1 = keys.at(left.getNonterminal());
      auto k2 = keys.at(right.getNonterminal());
      
      if(k1 < k2) return true;
      else if(k1 > k2) return false;
      
      k1 = left.position(); k2 = right.position();
      if(k1 < k2) return true;
      else if(k1 > k2) return false;
      
      return left.getRule() < right.getRule();
    }
  };
  vector<Item> result = vector<Item>(itemSet.begin(), itemSet.end());
  sort(result.begin(), result.end(), ItemComparator(l));
  return result;
}

ItemSet closure(const Item &i, const Language &l) {
  ItemSet result;
  // A sort of depth-first search for closure.
  stack<Item> stck;
  stck.push(i);
  while (!stck.empty()) {
    const Item item = std::move(stck.top());
    stck.pop();
    if (result.find(item) != result.end())
      continue;
    result.insert(item);
    //Note: short-circuit operator is important here.
    if (item.isAtEnd() || !holds_alternative<string>(item.expected_symbol()))
      continue;
    const string& newNonterminal = get<string>(item.expected_symbol());
    for(const Rule& rule : l.prods.at(newNonterminal))
      stck.push(Item(newNonterminal, rule, 0));
  }
  return result;
}

ItemSet closure(const ItemSet& itemSet, const Language& l) {
  ItemSet result;
  for(const Item& item : itemSet) {
    const ItemSet& partial = closure(item, l);
    result.insert(partial.begin(), partial.end());
  }
  return result;
}

ItemSet computeGoto(const ItemSet& itemSet, const Symbol& symbol, const Language& l) {
  ItemSet targetItemSet; 
  for(const Item& item : itemSet) {
    if(item.isAtEnd()) continue; //This is a termination state.
    if(item.expected_symbol() == symbol) targetItemSet.insert(item.advancedItem());  
  }
  return closure(targetItemSet, l);
}