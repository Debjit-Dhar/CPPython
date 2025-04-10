#include "parser.hpp"
#include "lexer.hpp"
#include "symtab.hpp"
#include <iostream>
#include <fstream>
#include <cstdlib>
#include <cerrno>
#include <cstring>
#include <utility>
using namespace std;

int main(int argc, char *argv[]) {
  //First build the parser
  ParsingTable parsingTable;
  GotoTable gotoTable;
  SymbolTable symbolTable;
  size_t startState;
  {
    //Output language info into this file
    ofstream info = ofstream("grammar.txt");
    Language language = genLanguage();
    info << "Language information:\n" << language << endl;
    ParsingTableGenerator p = ParsingTableGenerator(language);
    info << "\nState transitions:\n";
    p.printTransitions(info);
    info << "\nParsing table:\n";
    p.printParsingTable(info);
    info.close();
    startState = p.getStartState();
    parsingTable = std::move(p.parsingTable);
    gotoTable = std::move(p.gotoTable);
  }

  if(argc < 2) {
    cerr << "Usage: " << argv[0] << " <input file>\n";
    return EXIT_FAILURE;
  }
  vector<Token> tokens;
  {
    ifstream file = ifstream(argv[1]);
    if(!file) {
      cerr << "ERROR! Couldn't read file: " << strerror(errno) << endl;
      return EXIT_FAILURE;
    }
    tokens = doLex(readFile(file), symbolTable);
    file.close();
  }

  return parse(parsingTable, gotoTable, startState, tokens, symbolTable, cout) ? EXIT_SUCCESS : EXIT_FAILURE;
}