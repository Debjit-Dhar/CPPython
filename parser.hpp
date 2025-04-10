#ifndef __PARSER__
#define __PARSER__

#include "lexer.hpp"
#include "symtab.hpp"
#include <iostream>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <variant>
#include <vector>
using namespace std;

// Symbol type. Monostate indicates unassigned.
using Symbol = std::variant<std::monostate, TokenType, std::string>;

ostream &operator<<(ostream &, const Symbol &);

// Type for a single rule for a single nonterminal. Length 0 for <epsilon>
using Rule = std::vector<Symbol>;
// Hash for a rule
template <> struct std::hash<Rule> {
  // Hashing algorithm taken from Java libraries.
  std::size_t operator()(const Rule &rule) const noexcept {
    std::hash<Symbol> hasher;
    std::size_t result = 1;
    for (const Symbol &symbol : rule)
      result = 31 * result + hasher(symbol);
    return result;
  }
};

ostream &operator<<(ostream &, const Rule &);

// Type for a rule set for a rule.
using RuleSet = std::unordered_set<Rule, std::hash<Rule>>;

ostream &operator<<(ostream &, const RuleSet &);

// Represents the language
struct Language {
  // Productions
  unordered_map<string, RuleSet> prods;
  // Priority/precedence sequence of nonterminals, holds ALL nonterminals.
  vector<string> order;
  // Start symbol pointer
  decltype(prods)::iterator startProd;
  // First and follow sets
  unordered_map<string, unordered_set<TokenType>> first, follow;

  void add(const std::string &nt, const RuleSet &rs = RuleSet()) {
    bool found = prods.find(nt) != prods.end();
    prods[nt] = rs;
    if (!found)
      order.push_back(nt);
  }

  void addRule(const string &nt, const Rule &rule) {
    prods.at(nt).insert(rule);
  }

  void setStartSymbol(const string &nt) { startProd = prods.find(nt); }

  //void doLeftFactoring();
  void genFirstSets();
  void genFollowSets();

  unordered_set<TokenType> getFirst(const Rule &) const;

private:
  //string doLeftFactoring(const string& nt);
};
Language genLanguage();
ostream &operator<<(ostream &, const Language &);

// Item set element (the single row)
class Item {
  // The nonterminal of this production
  string nonterminal;
  // The production
  Rule rule;
  // Where are we in the production?
  size_t currentIndex;

public:
  Item(const string &nonterminal = string(), const Rule &rule = Rule(),
       size_t currentIndex = 0llu)
      : nonterminal(nonterminal), rule(rule) {
    this->currentIndex =
        currentIndex > rule.size() ? rule.size() : currentIndex;
  }

  const string &getNonterminal() const noexcept { return nonterminal; }
  const Rule &getRule() const noexcept { return rule; }
  size_t position() const noexcept { return currentIndex; }
  const Symbol &expected_symbol() const noexcept {
    return rule.at(currentIndex);
  }
  bool isAtEnd() const noexcept {return currentIndex >= rule.size();}

  bool operator==(const Item &other) const noexcept {
    return currentIndex == other.currentIndex &&
           nonterminal == other.nonterminal && rule == other.rule;
  }
  bool operator!=(const Item &other) const noexcept {
    return currentIndex != other.currentIndex ||
           nonterminal != other.nonterminal || rule != other.rule;
  }

  Item advancedItem() const noexcept {
    return Item(nonterminal, rule, currentIndex+1);
  }
};
template <> struct std::hash<Item> {
  //Hash an item.
  size_t operator()(const Item &item) const noexcept {
    std::hash<string> strHasher;
    std::hash<Rule> ruleHasher;
    return ((31 + item.position()) * 31 + strHasher(item.getNonterminal())) *
               31 +
           ruleHasher(item.getRule());
  }
};

ostream& operator<<(ostream&, const Item&);

//Item set, a single element in the SLR state machine.
using ItemSet = std::unordered_set<Item, std::hash<Item>>;

//Item sets themselves need to be hashable.
template<> struct std::hash<ItemSet> {
  size_t operator()(const ItemSet& itemSet) const noexcept {
    const auto& itemHasher = itemSet.hash_function();
    //Sum the hashes since it should be independent of order.
    size_t sum = 0;
    for(const Item& item : itemSet) sum += itemHasher(item);
    return sum;
  }
};

//Get a sorted item set array on the basis of order in language. This is an expensive operation.
vector<Item> sortedItems(const ItemSet&, const Language&);
ostream& operator<<(ostream&, const vector<Item>&);

//Closure function
ItemSet closure(const Item&, const Language&);
//Composite closure function
ItemSet closure(const ItemSet&, const Language&);
//Compute GOTO(itemSet, symbol).
ItemSet computeGoto(const ItemSet& itemSet, const Symbol& symbol, const Language&);

//Unique type for accepting.
struct Accept : public virtual monostate {};
extern const Accept ACCEPT;

//Parsing table action.
//Monostate - uninitialized
//size_t - refers to a state index (shift action) OR a goto state for nonterminals.
//pair - is a production to reduce
//Accept - accept.
using Action = variant<monostate, size_t, pair<string, Rule>, Accept>;
ostream& operator<<(ostream&, const Action&);

using ParsingTable = vector<unordered_map<TokenType, Action>>;
using GotoTable = vector<unordered_map<string, size_t>>;

//Parsing table generator.
struct ParsingTableGenerator {
  Language language;
  //Should be obvious
  ItemSet startState;
  //States array.
  vector<ItemSet> states;
  //State transition table. This is NOT THE PARSING TABLE.
  //References indexes from states array and stateIndexes.
  //(meaning it maps index and symbol to another index, hence vector.)
  vector<unordered_map<Symbol, size_t>> transitions;
  //Indexes of states in states array (sort of a reverse mapping.)
  unordered_map<ItemSet, size_t> stateIndexes;
  //Stores the symbol corresponding to a state (the incoming symbol).
  //Map with reference to stateIndexes
  vector<Symbol> stateSymbols;
  //Parsing table. Similar organisation to transitions table.
  ParsingTable parsingTable;
  //Goto table. Similar organisation to goto table.
  GotoTable gotoTable;

  ParsingTableGenerator(const Language& language) : language(language) {
    buildTransitions();
    buildParsingTable();
  }

  void printTransitions(ostream&) const;
  void printParsingTable(ostream&) const;

  size_t getStartState() const {return stateIndexes.at(startState);}

private:
  //build transition table.
  void buildTransitions();
  //build parsing table.
  void buildParsingTable();
};

//The actual parser. Returns true on successful parse.
bool parse(const ParsingTable&, const GotoTable&, const size_t startState, const vector<Token>& tokenSequence, SymbolTable&, ostream& messages);

#endif