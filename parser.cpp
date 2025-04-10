#include "parser.hpp"
#include "lexer.hpp"
#include <cassert>
#include <stdexcept>
#include <utility>
using namespace std;

using ParsingStackElement = pair<Symbol, size_t>;
using ParsingStack = vector<ParsingStackElement>;

inline ostream &operator<<(ostream &out, const ParsingStackElement &element) {
  out << '(' << element.first << " | state " << element.second << ')';
  return out;
}

inline ostream &operator<<(ostream &out, const ParsingStack &pstack) {
  if (pstack.empty()) {
    out << "empty";
    return out;
  }
  auto itr = pstack.begin();
  while (true) {
    out << *itr;
    itr++;
    if (itr == pstack.end())
      break;
    out << ", ";
  }
  return out;
}

inline ostream &operator<<(ostream &out, const Token &token) {
  out << '[' << TOKEN_NAMES.at(token.type) << " `" << token.value << "`]";
  return out;
}

bool parse(const ParsingTable &parsingTable, const GotoTable &gotoTable,
           const size_t startState, const vector<Token> &inputArray,
           SymbolTable &symbolTable, ostream &out) {
  // In its current form, symbol table is unused otherwise.
  out<<"Identifiers used from symbol table:\n";
  for(const SymbolTable::FullEntry &e : symbolTable) {
    out << e.first << endl;
  }
  out << "\n\n";

  ParsingStack stack = {{END_OF_PROGRAM_INDICATOR, startState}};
  size_t i = 0; // Index into inputArray
  bool errorFlag = false, acceptFlag = false;
  Symbol currentSymbol;
  size_t currentState;
  while (!stack.empty() && i < inputArray.size()) {
    currentSymbol = stack.back().first;
    currentState = stack.back().second;
    out << "\nStack is " << stack << ", current next input token is "
        << inputArray[i] << endl;
    Action action;
    try {
      action = parsingTable[currentState].at(inputArray[i].type);
    } catch (const std::out_of_range &ex) {
    }
    if (!action.index()) {
      out << "ERROR! Unexpected terminal.\n";
      errorFlag = true;
      goto parseEnd;
    }
    out << "We'll do " << action << endl;
    switch (action.index()) {
    case 3: // Accept
      out << "Input string has been accepted.\n";
      acceptFlag = true;
      goto parseEnd;
    case 1: { // Shift action
      const size_t shiftTo = get<size_t>(action);
      stack.push_back({inputArray[i].type, shiftTo});
      i++;
    } break;
    case 2: { // Reduce action
      const auto &reduction = get<pair<string, Rule>>(action);
      const string &nonterminal = reduction.first;
      const Rule &production = reduction.second;
      for (auto ritr = production.rbegin(); ritr != production.rend(); ritr++) {
        currentSymbol = stack.back().first;
        currentState = stack.back().second;
        assert(currentSymbol == *ritr); // Reduce symbols should match!
        stack.pop_back();
      }
      currentSymbol = stack.back().first;
      currentState = stack.back().second;
      stack.push_back({nonterminal, gotoTable[currentState].at(nonterminal)});
    } break;
    }
  }

parseEnd:
  return acceptFlag && !errorFlag;
}