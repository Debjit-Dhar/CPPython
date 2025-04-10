#include "parser.hpp"
#include "lexer.hpp"
#include <stack>
#include <utility>
#include <cstdlib>
#include <stdexcept>
using namespace std;

const Accept ACCEPT = Accept();

void ParsingTableGenerator::buildTransitions() {
  // Depth-first search to generate all possible states
  stack<ItemSet> unmarkedStates;
  unmarkedStates.push(startState = closure(
      Item(language.startProd->first, *(language.startProd->second.begin()), 0),
      language));
  while (!unmarkedStates.empty()) {
    const ItemSet state = std::move(unmarkedStates.top());
    unmarkedStates.pop();
    if (stateIndexes.find(state) != stateIndexes.end())
      continue;
    stateIndexes[state] = states.size();
    states.push_back(state);
    for (const string &target : language.order) {
      const auto &targetState = computeGoto(state, Symbol(target), language);
      if (targetState.empty())
        continue;
      unmarkedStates.push(targetState);
    }
    for (const auto &e : TOKEN_NAMES) {
      const Symbol &target = Symbol(e.first);
      const auto &targetState = computeGoto(state, target, language);
      if (targetState.empty())
        continue;
      unmarkedStates.push(targetState);
    }
  }

  stateSymbols.resize(states.size());
  transitions.resize(states.size());
  stateSymbols[0] = END_OF_PROGRAM_INDICATOR;
  for(size_t i = 0; i < states.size(); i++) {
    for(const string& target : language.order) {
      const Symbol& targetSymbol = target;
      const auto &targetState = computeGoto(states[i], targetSymbol, language);
      if (targetState.empty())
        continue;
      //Insert in transition table
      size_t targetStateIndex = transitions[i][targetSymbol] = stateIndexes[targetState];
      if(!stateSymbols[targetStateIndex].index()) stateSymbols[targetStateIndex] = targetSymbol;
      else if(stateSymbols[targetStateIndex] != targetSymbol) {
        cerr << "ERROR! Given grammar is not LR(0).\n";
        cerr << "For state at index " << targetStateIndex << ":\n"; 
        cerr << sortedItems(targetState, language) << endl;
        cerr << "This has multiple incoming symbols: " << targetSymbol << " and " << stateSymbols[targetStateIndex];
        exit(EXIT_FAILURE); 
      }
    }
    for(const auto& e : TOKEN_NAMES) {
      const Symbol& targetSymbol = e.first;
      const auto &targetState = computeGoto(states[i], targetSymbol, language);
      if (targetState.empty())
        continue;
      //Insert in transition table
      size_t targetStateIndex = transitions[i][targetSymbol] = stateIndexes[targetState];
      if(!stateSymbols[targetStateIndex].index()) stateSymbols[targetStateIndex] = targetSymbol;
      else if(stateSymbols[targetStateIndex] != targetSymbol) {
        cerr << "ERROR! Given grammar is not LR(0).\n";
        cerr << "For state at index " << targetStateIndex << ":\n"; 
        cerr << sortedItems(targetState, language) << endl;
        cerr << "This has multiple incoming symbols: " << targetSymbol << " and " << stateSymbols[targetStateIndex];
        exit(EXIT_FAILURE);  
      }
    }
  }
}

void ParsingTableGenerator::printTransitions(ostream& out) const {
  for(size_t i = 0; i < states.size(); i++) {
    out << "\nState " << i << ":\n";
    if(startState == states[i]) out << "This is the start state\n";
    out << "Corresponding symbol is " << stateSymbols[i] << endl;
    out << sortedItems(states[i], language) << endl;
    for(const string& target : language.order) {
      const Symbol& targetSymbol = target;
      try {
        size_t targetStateIndex = transitions[i].at(targetSymbol);
        out << "On " << targetSymbol << ", go to " << targetStateIndex << endl;
      } catch (const std::out_of_range &ex) {}
    }
    for(const auto& e : TOKEN_NAMES) {
      const Symbol& targetSymbol = e.first;
      try {
        size_t targetStateIndex = transitions[i].at(targetSymbol);
        out << "On " << targetSymbol << ", go to " << targetStateIndex << endl;
      } catch (const std::out_of_range &ex) {}
    }
    out << endl;
  }
}

void ParsingTableGenerator::buildParsingTable() {
  //printTransitions(cout);
  parsingTable.resize(states.size());
  gotoTable.resize(states.size());

  for(size_t state_number = 0; state_number < states.size(); state_number++) {
    const ItemSet& itemSet = states[state_number];
    for(const Item& item : itemSet) {
      if(item.isAtEnd()) {
        //Create reduce actions
        for(const TokenType& terminal : language.follow.at(item.getNonterminal())) {
          Action newAction = Action(pair<string, Rule>(item.getNonterminal(), item.getRule()));
          {
            auto itr = parsingTable[state_number].find(terminal);
            if(itr != parsingTable[state_number].end()) {
              const Action& oldAction = itr->second;
              if(oldAction.index() && oldAction != newAction) {
                //Error. More than one entry in table at once.
                cerr << "ERROR! Given grammar is not LR(0).\n";
                cerr << "For state " << state_number << " and input token " << TOKEN_NAMES.at(terminal);
                cerr << ", we have two possible actions: " << oldAction << " and " << newAction << endl;
                exit(EXIT_FAILURE);
              }
            }
          }
          parsingTable[state_number][terminal] = newAction;
        }
        //At end of start symbol?
        if(item.getNonterminal() == language.startProd->first)
          parsingTable[state_number][END_OF_PROGRAM_INDICATOR] = ACCEPT;
      }
      else {
        //Is a terminal expected next?
        if(holds_alternative<TokenType>(item.expected_symbol())) {
          const TokenType terminal = get<TokenType>(item.expected_symbol());
          Action newAction = transitions[state_number].at(item.expected_symbol());
          {
            auto itr = parsingTable[state_number].find(terminal);
            if(itr != parsingTable[state_number].end()) {
              const Action& oldAction = itr->second;
              if(oldAction.index() && oldAction != newAction) {
                //Error. More than one entry in table at once.
                cerr << "ERROR! Given grammar is not LR(0).\n";
                cerr << "For state " << state_number << " and input token " << TOKEN_NAMES.at(terminal);
                cerr << ", we have two possible actions: " << oldAction << " and " << newAction << endl;
                exit(EXIT_FAILURE);
              }
            }
          }
          parsingTable[state_number][terminal] = newAction;
        }
        //Else expected a nonterminal next. Fill goto table
        else {
          const string& nonterminal = get<string>(item.expected_symbol());
          size_t newGoto = transitions[state_number].at(item.expected_symbol());
          {
            auto itr = gotoTable[state_number].find(nonterminal);
            if(itr != gotoTable[state_number].end() && itr->second != newGoto) {
              size_t oldGoto = itr->second;
              cerr << "ERROR! Given grammar is not LR(0).\n";
              cerr << "For state " << state_number << " and goto nonterminal " << nonterminal;
              cerr << ", we have two possible actions: goto state " << oldGoto << " and goto state " << newGoto << endl;
              exit(EXIT_FAILURE);
            }
          }
          gotoTable[state_number][nonterminal] = newGoto;
        }
      }
    }
  }
}

void ParsingTableGenerator::printParsingTable(ostream& out) const {
  for(size_t state_number = 0; state_number < states.size(); state_number++) {
    out << "For state " << state_number << ":\n";
    for(const auto& e : TOKEN_NAMES) {
      const TokenType terminal = e.first;
      try {
        const auto& action = parsingTable[state_number].at(terminal);
        out << "For terminal " << e.second << ", do " << action << endl;
      } catch (const std::out_of_range &ex) {}
    }
    for(const string& nonterminal : language.order) {
      try {
        size_t theGoto = gotoTable[state_number].at(nonterminal);
        out << "For nonterminal " << nonterminal << ", goto state " << theGoto << endl;
      } catch (const std::out_of_range &ex) {}
    }
    out << endl;
  }
}

ostream& operator<<(ostream& out, const Action& action) {
  switch(action.index()) {
    case 1: out << "shift to state " << get<size_t>(action); break;
    case 2: {
      const auto& p = get<pair<string, Rule>>(action);
      out << "reduce <" << p.first << "> ::= " << p.second;
    } break;
    case 3: out << "ACCEPT."; break;
    default: out << "!!no such action!!"; break;
  }
  return out;
}