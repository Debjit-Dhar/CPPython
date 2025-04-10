#include <cctype>
#include "lexer.hpp"

using namespace std;

// Keywords lookup table
static const unordered_map<string, TokenType> KEYWORD_LOOKUP = {
    {"print", PRINT},
    {"for", FOR},
    {"in", IN},
    {"endfor", ENDFOR},
    {"int", INT_TYPE},
    {"float", FLOAT_TYPE},
    {"str", STR_TYPE},
    {"bool", BOOL_TYPE},
    {"True", TRUE_VALUE},
    {"False", FALSE_VALUE},
    {"list", LIST_TYPE}
};


// Lexical Analyzer
class Lexer {
private:
  const string& input;
  size_t pos;

public:
  explicit Lexer(const string& input) noexcept : input(input), pos(0) {}

  char peek() const { return pos < input.length() ? input[pos] : '\0'; }
  char advance() { return pos < input.length() ? input[pos++] : '\0'; }
  void skipWhitespaceExceptNewline() {
    while (isspace(peek()) && peek() != '\n')
      advance();
  }

  Token getNextToken() {
    skipWhitespaceExceptNewline();

    if(peek() == '#') {
      // Skip comment
      while(advance() != '\n');
      return {NEWLINE, "\n"};
    }
    if(peek() == '\'') {
      string str;
      advance();
      while(peek() != '\'') {
        str.push_back(peek()); advance();
      }
      advance();
      return {STRING, str};
    }
    if(peek() == '\"') {
      string str;
      advance();
      while(peek() != '\"') {
        str.push_back(peek()); advance();
      }
      advance();
      return {STRING, str};
    }
    
    if (isdigit(peek())) { // Integer or Float
      string num;
      while (isdigit(peek()))
        num += advance();
      if (peek() == '.') {
        num += advance();
        while (isdigit(peek()))
          num += advance();
        return {FLOAT, num};
      }
      return {INTEGER, num};
    }

    if (isalpha(peek())) { // Identifiers or Keywords
      string id;
      while (isalnum(peek()))
        id += advance();
      
      auto itr = KEYWORD_LOOKUP.find(id);
      if(itr != KEYWORD_LOOKUP.end()) return {itr->second, id};
      return {IDENTIFIER, id};
    }

    switch (advance()) {
    case '=':
      return {EQUALS, "="};
    case '+':
      return {PLUS, "+"};
    case '-':
      return {MINUS, "-"};
    case '*':
      return {MULT, "*"};
    case '/':
      return {DIV, "/"};
    case '(':
      return {OPEN_PAREN, "("};
    case ')':
      return {CLOSE_PAREN, ")"};
    case '[':
      return {OPEN_BRACKET, "["};
    case ']':
      return {CLOSE_BRACKET, "]"};
    case ',':
      return {COMMA, ","};
    case ':':
      return {COLON, ":"};
    case ';':
      return {SEMICOLON, ";"};
    case '\n':
      return {NEWLINE, "\n"};
    case '\0':
      return {END_OF_FILE, ""};
    default:
      return {INVALID, ""};
    }
  }
};

// Read Python file
string readFile(istream& stream) {
  string content((istreambuf_iterator<char>(stream)),
                 (istreambuf_iterator<char>()));
  return content;
}

vector<Token> doLex(const string& text, SymbolTable& symtab) {
  vector<Token> arr; Token token;
  Lexer lex(text);
  do {
    token = lex.getNextToken();
    arr.push_back(token);
    if(token.type == IDENTIFIER) symtab.insert(token.value);
  } while (token.type != END_OF_FILE);
  return arr;
}

// Main Function
/*int main(int argc, char *argv[]) {
  // while ((token = lexer.getNextToken()).type != END_OF_FILE) {
  //   tokens.push_back(token);
  // }

  string content;
  if(argc >= 2) {
    ifstream file(argv[1]);
    if(!file) {
      cerr << argv[1] << " not found";
      return 1;
    }
    content = readFile(file);
  } else content = readFile(cin);

  Lexer lexer = Lexer(content);
  Token token;
  while((token = lexer.getNextToken()).type != END_OF_FILE) {
    cout << token.value << ' ' << token.type << '\n';
  }
}
*/