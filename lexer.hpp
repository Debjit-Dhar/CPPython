#ifndef __LEXER__
#define __LEXER__

#include <string>
#include <vector>
#include <iostream>
#include <unordered_map>
#include "symtab.hpp"

using std::istream;
using std::vector;
using std::string;
using std::unordered_map;

// Explicitly inherit from int to get comparison operators and std::hash. That will be important later
enum TokenType : int {
  IDENTIFIER,
  INTEGER,
  FLOAT,
  STRING,
  EQUALS,
  PLUS,
  MINUS,
  MULT,
  DIV,
  PRINT,
  FOR,
  IN,
  ENDFOR,
  INT_TYPE,
  FLOAT_TYPE,
  STR_TYPE,
  BOOL_TYPE,
  TRUE_VALUE,
  FALSE_VALUE,
  LIST_TYPE,
  OPEN_PAREN,
  CLOSE_PAREN,
  OPEN_BRACKET,
  CLOSE_BRACKET,
  COMMA,
  COLON,
  SEMICOLON,
  NEWLINE,
  END_OF_FILE,
  INVALID,

  END_OF_PROGRAM_INDICATOR = END_OF_FILE,

  //To indicate empty string, special
  EPSILON = -1
};

extern const unordered_map<TokenType, const char *> TOKEN_NAMES;

struct Token {
  TokenType type;
  string value;
};

string readFile(istream&);
vector<Token> doLex(const string&, SymbolTable&);

#endif