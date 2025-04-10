#include "lexer.hpp"
#include "parser.hpp"
#include <variant>
#include <cstdlib>
#include <algorithm>
//#include <utility>
using namespace std;

Language genLanguage() {
  Language l;

  l.add("program", {{"statement_list"}});

  l.add("statement_list", {
    {"statement_list", "st_end", "statement"},
    {"statement"}
  });

  l.add("statement", {
    {"assignment"},
    {"function_call"},
    {"for_loop"},
    {} //epsilon
  });

  l.add("st_end", {
    {SEMICOLON},
    {NEWLINE}
  });

  l.add("assignment", {{IDENTIFIER, EQUALS, "expression"}});

  l.add("function_call", {
    {PRINT, OPEN_PAREN, "expression_list", CLOSE_PAREN},
    {IDENTIFIER, OPEN_PAREN, "expression_list", CLOSE_PAREN},
    {"datatype", OPEN_PAREN, "expression_list", CLOSE_PAREN},
//    {OPEN_PAREN, "expression", CLOSE_PAREN, OPEN_PAREN, "expression_list", CLOSE_PAREN}
  });

  l.add("expression_list", {
    {"expression"},
    {"expression_list", COMMA, "expression"},
    {} //Expressions may be empty
  });

  l.add("expression", {
    {"term"},
    {"expression", PLUS, "term"},
    {"expression", MINUS, "term"},
  });

  l.add("term", {
    {"factor"},
    {"term", MULT, "factor"},
    {"term", DIV, "factor"}
  });

  l.add("factor", {
    {IDENTIFIER},
    {INTEGER},
    {FLOAT},
    {STRING},
    {"list"},
    {TRUE_VALUE},
    {FALSE_VALUE},
    {"function_call"},
    {OPEN_PAREN, "expression", CLOSE_PAREN},
  });

  l.add("datatype", {
    {INT_TYPE},
    {FLOAT_TYPE},
    {STR_TYPE},
    {BOOL_TYPE},
  });

  l.add("for_loop", {{FOR, IDENTIFIER, IN, "expression", COLON, "statement_list", ENDFOR}});

  l.add("list", {{OPEN_BRACKET, "expression_list", CLOSE_BRACKET}});

  l.setStartSymbol("program");

  for(const auto& e : l.prods) {
    for(const auto& r: e.second) {
      for(const auto& symbol : r) {
        if(holds_alternative<std::string>(symbol) && l.prods.find(get<std::string>(symbol)) == l.prods.end()) {
          cerr << "Error! " << symbol << " not in productions!\n";
          cerr << "For <" << e.first << "> := " << r << endl;
          exit(EXIT_FAILURE);
        }
      }
    }
  }

  l.genFirstSets();
  l.genFollowSets();

  return l;
}

ostream& operator<<(ostream& out, const Language& l) {
  out << "Grammar rules:\n\n";

  for(const string& nt: l.order) 
    out << '<' << nt << "> ::= " << l.prods.at(nt) << "\n\n";

  out << "\nFirst sets:\n";
  for(const string& nt : l.order) {
    out << '<' << nt << "> ::= {";
    vector<TokenType> first;
    {
      const auto& firstSet = l.getFirst({nt});
      first.reserve(firstSet.size());
      first.insert(first.end(), firstSet.begin(), firstSet.end());
      sort(first.begin(), first.end());
    }
    auto itr = first.begin();
    while(itr != first.end()) {
      out << Symbol(*itr); itr++;
      if(itr == first.end()) break;
      out << ", ";
    }
    out << "}\n";
  }

  out << "\nFollow sets:\n";
  for(const string& nt : l.order) {
    out << '<' << nt << "> ::= {";
    vector<TokenType> follow;
    {
      const auto& followSet = l.follow.at(nt);
      follow.reserve(followSet.size());
      follow.insert(follow.end(), followSet.begin(), followSet.end());
      sort(follow.begin(), follow.end());
    }
    auto itr = follow.begin();
    while(itr != follow.end()) {
      out << Symbol(*itr); itr++;
      if(itr == follow.end()) break;
      out << ", ";
    }
    out << "}\n";
  }

  out << endl;
  return out;
}

ostream& operator<<(ostream& out, const RuleSet& rs) {
  if(rs.empty()) return out;
  auto itr = rs.begin();
  while(true) {
    out << *itr; itr++;
    if(itr == rs.end()) break;
    else out << " | ";
  }
  return out;
}

ostream& operator<<(ostream& out, const Rule& rule) {
  if(rule.empty()) {
    out << "<epsilon!>";
    return out;
  }
  auto itr = rule.begin();
  while(true) {
    out << *itr; itr++;
    if(itr == rule.end()) break;
    else out << ' ';
  }
  return out;
}

const unordered_map<TokenType, const char *> TOKEN_NAMES = {
  {IDENTIFIER, "IDENTIFIER"},
  {INTEGER, "INTEGER"},
  {FLOAT, "FLOAT"},
  {STRING, "STRING"},
  {EQUALS, "EQUALS"},
  {PLUS, "PLUS"},
  {MINUS, "MINUS"},
  {MULT, "MULT"},
  {DIV, "DIV"},
  {PRINT, "PRINT"},
  {FOR, "FOR"},
  {IN, "IN"},
  {ENDFOR, "ENDFOR"},
  {INT_TYPE, "INT_TYPE"},
  {FLOAT_TYPE, "FLOAT_TYPE"},
  {STR_TYPE, "STR_TYPE"},
  {BOOL_TYPE, "BOOL_TYPE"},
  {TRUE_VALUE, "TRUE_VALUE"},
  {FALSE_VALUE, "FALSE_VALUE"},
  {LIST_TYPE, "LIST_TYPE"},
  {OPEN_PAREN, "OPEN_PAREN"},
  {CLOSE_PAREN, "CLOSE_PAREN"},
  {OPEN_BRACKET, "OPEN_BRACKET"},
  {CLOSE_BRACKET, "CLOSE_BRACKET"},
  {COMMA, "COMMA"},
  {COLON, "COLON"},
  {SEMICOLON, "SEMICOLON"},
  {NEWLINE, "NEWLINE"},
  {END_OF_FILE, "END_OF_FILE"},
  {INVALID, "INVALID"},
  {EPSILON, "<empty string>"}
};

ostream& operator<<(ostream& out, const Symbol& symbol) {
  switch(symbol.index()) {
    case 1llu: out << TOKEN_NAMES.at(get<TokenType>(symbol)); break;
    case 2llu: out << '<' << get<std::string>(symbol) << '>'; break;
    default: out << "!!no such symbol!!"; break;
  }
  return out;
}

/*
string Language::doLeftFactoring(const string& nonterminal) {
  //Perform left-factoring. Returns new nonterminal if done.
  
  //First find a new name
  string newNonterminal;
  {
    unsigned long nameGen = 1;
    while(true) {
      newNonterminal = nonterminal + "_F" + to_string(nameGen);
      if(prods.find(newNonterminal) == prods.end()) break;
      nameGen++;
    }
  }
  
  //Find longest common subsequence.
  Rule longestCommonSubsequence;
  const vector<Rule> rules = vector<Rule>(prods.at(nonterminal).cbegin(), prods.at(nonterminal).cend());
  {
    for(size_t i = 0; i < (rules.size() - 1); i++) {
      for(size_t j = i+1; j < rules.size(); j++) {
        const Rule& seq1 = rules[i];
        const Rule& seq2 = rules[j];
        auto e1 = seq1.begin();
        auto e2 = seq2.begin();
        Rule commonSubsequence;
        for(; e1 != seq1.end() && e2 != seq2.end() && *e1 == *e2; e1++, e2++)
          commonSubsequence.push_back(*e1);
        if(commonSubsequence.size() > longestCommonSubsequence.size())
          longestCommonSubsequence = std::move(commonSubsequence);
      }
    }
    if(longestCommonSubsequence.empty()) {
      //Left-factoring not required.
      return string();
    }
  }
  //cout << "New generated name: " << newNonterminal << " for " << nonterminal << " factoring out " << longestCommonSubsequence << endl;


  //Find rules which need to be left-factored.
  RuleSet leftFactoringRules, nonLeftFactoringRules;
  for(const Rule& rule: rules) {
    if(rule.size() < longestCommonSubsequence.size()) continue;
    bool flag = true;
    for(size_t i = 0; i < longestCommonSubsequence.size(); i++) 
      if(rule[i] != longestCommonSubsequence[i]) {flag = false; break;}
    if(flag) leftFactoringRules.insert(rule);
    else nonLeftFactoringRules.insert(rule);
  }

  //Now change rules
  longestCommonSubsequence.emplace_back(newNonterminal);
  nonLeftFactoringRules.insert(longestCommonSubsequence);
  longestCommonSubsequence.pop_back(); //Revert to original
  prods[nonterminal] = nonLeftFactoringRules;
  //cout << '<' << nonterminal << "> ::= " << nonLeftFactoringRules << endl;

  RuleSet newNonterminalRules;
  for(const Rule& rule : leftFactoringRules) {
    //Subtract the common part
    Rule newRule; newRule.reserve(rule.size() - longestCommonSubsequence.size());
    for(size_t i = longestCommonSubsequence.size(); i < rule.size(); i++)
      newRule.push_back(rule[i]);
    newNonterminalRules.insert(std::move(newRule));
  }
  prods[newNonterminal] = newNonterminalRules;
  //cout << '<' << newNonterminal << "> ::= " << newNonterminalRules << endl;
  
  return newNonterminal;
}

//Multiscan left-factoring
void Language::doLeftFactoring() {
  //Mapping of the originating nonterminal precedence index in order to the new nonterminal
  unordered_map<size_t, string> newNonterminals;
  while(true) {
    //cout << "Doing scan..." << endl;
    newNonterminals.clear();
    for(size_t i = 0; i < order.size(); i++) {
      const string& nonterminal = order[i];
      const string& newNonterminal = doLeftFactoring(nonterminal);
      if(!newNonterminal.empty()) 
        newNonterminals[i] = newNonterminal;
    }
    if(newNonterminals.empty()) break;

    //Create new precedence order
    decltype(order) newOrder;
    newOrder.reserve(order.size() + newNonterminals.size());
    for(size_t i = 0; i < order.size(); i++) {
      newOrder.push_back(order[i]);
      auto itr = newNonterminals.find(i);
      if(itr != newNonterminals.end()) newOrder.push_back(itr->second);
    }
    order = std::move(newOrder);
  }

  //Regernerate these
  genFirstSets();
  genFollowSets();
}
*/