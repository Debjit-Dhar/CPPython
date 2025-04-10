#include "lexer.hpp"
#include "parser.hpp"
#include <unordered_set>
#include <unordered_map>
#include <cassert>
#include <utility>
#include <variant>
using namespace std;

void Language::genFirstSets() {
  first.clear();

  bool runScan;
  do {
    runScan = false;
    //`cout<<"Doing scan...\n";
    for(const string& nonterminal : order) {
      //Does this work? Do we get a clean default empty set?
      unordered_set<TokenType> oldSet, newSet;
      {
        auto itr = first.find(nonterminal);
        if(itr != first.end()) newSet = oldSet = itr->second;
      }
      const auto& ruleset = prods[nonterminal];
      bool nullable = false; //Can this nonterminal produce an empty string?
      for(const auto& rule : ruleset) {
        bool productionNullable = true; //Does this production/ruleset produce an empty string?
        for(const Symbol& symbol : rule) {
          unordered_set<TokenType> symbolFirst;
          assert(symbol.index() > 0);
          switch(symbol.index()) {
            case 1: symbolFirst.insert(get<TokenType>(symbol)); break;
            case 2: {
              auto itr = first.find(get<std::string>(symbol));
              if(itr != first.end()) symbolFirst = itr->second;
            } break;
          } 
          newSet.insert(symbolFirst.begin(), symbolFirst.end());
          newSet.erase(EPSILON);
          //This symbol in rule is not nullable
          if(symbolFirst.find(EPSILON) == symbolFirst.end()) {
            productionNullable = false; break;
          }
        }
        nullable = nullable || productionNullable;
      }
      if (nullable) newSet.insert(EPSILON);
      runScan = runScan || oldSet != newSet;

      // cout << '<' << nonterminal << "> ::= {";
      // auto itr = newSet.begin();
      // while(itr != newSet.end()) {
      //   cout << Symbol(*itr); itr++;
      //   if(itr == newSet.end()) break;
      //   cout << ", ";
      // }
      // cout << "}\n";

      first[nonterminal] = std::move(newSet);
    }

    // cout << endl;
    // for(const string& nt: order) {
    //   cout << '<' << nt << "> ::= {";
    //   unordered_set<TokenType> firstSet;
    //   {
    //     auto itr = first.find(nt);
    //     if(itr != first.end()) firstSet = itr->second;
    //   }
    //   auto itr = firstSet.begin();
    //   while(itr != firstSet.end()) {
    //     cout << Symbol(*itr); itr++;
    //     if(itr == firstSet.end()) break;
    //     cout << ", ";
    //   }
    //   cout << "}\n";
    // }
    // cout << "\n\n";
  } while (runScan);
}

void Language::genFollowSets() {
  if(first.empty()) genFirstSets();
  follow.clear();

  follow[startProd->first] = {END_OF_PROGRAM_INDICATOR};
  bool runScan;
  do {
    runScan = false;
    for(const string& nonterminal : order) {
      const RuleSet& rs = prods[nonterminal];
      for(const Rule& rule : rs) {
        for(auto i = rule.begin(); i != rule.end(); i++) {
          if(!holds_alternative<std::string>(*i)) continue;
          const string& targetSymbol = get<std::string>(*i);

          unordered_set<TokenType> oldSet, newSet, alphaSet, betaSet;
          //Old follow set of the symbol we are modifying
          {
            auto itr = follow.find(targetSymbol);
            if(itr != follow.end()) newSet = oldSet = itr->second;
          }
          //Follow set of the parent nonterminal this production comes from
          {
            auto itr = follow.find(nonterminal);
            if(itr != follow.end()) alphaSet = itr->second;
          }
          //First set of the REST of the production
          betaSet = getFirst(Rule(i + 1, rule.end()));
          newSet.insert(betaSet.begin(), betaSet.end());
          newSet.erase(EPSILON);
          //Add follow set of parent nonterminal if production results in epsilon
          if(betaSet.find(EPSILON) != betaSet.end())
            newSet.insert(alphaSet.begin(), alphaSet.end());
          
          runScan = runScan || oldSet != newSet;
          follow[targetSymbol] = newSet;
        }
      }
    }
  } while (runScan);
}

unordered_set<TokenType> Language::getFirst(const Rule& seq) const {
  if(seq.empty()) return {EPSILON};
  unordered_set<TokenType> result;
  for(const Symbol& symbol: seq) {
    unordered_set<TokenType> symbolFirst;
    assert(symbol.index() > 0);
    switch(symbol.index()) {
      case 1: symbolFirst.insert(get<TokenType>(symbol)); break;
      case 2: symbolFirst = first.at(get<string>(symbol)); break;
    } 
    result.insert(symbolFirst.begin(), symbolFirst.end());
    if(symbolFirst.find(EPSILON) == symbolFirst.end()) {
      result.erase(EPSILON); break;
    } 
  }
  return result;
}