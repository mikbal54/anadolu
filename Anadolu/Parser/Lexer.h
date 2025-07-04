#pragma once

#include "TokenType.h"
#include "PrimitiveTypes.h"

#include <string>
#include <unordered_map>
#include <memory>

class Module; 

class Token
{
public:

  //default cons.
  Token():
    type(TOKEN_IDENTIFIER),
    intValue(0),
    start(-1),
    length(-1)
  {}

  Token(TokenType _type, size_t _start, size_t _length): type(_type), start(_start), length(_length) { }

  ~Token()
  {
  }


  TokenType type;
  size_t start, length;
  union
  {
    INT intValue;
    float floatValue;
  };

  size_t GetRow(){ return 0; }
  size_t GetColumn() { return 0; }
};

class Lexer
{
private:

  friend class Parser;

  // methods
  size_t GetEndPositionOfWord(const std::string &input, INT pos);

  void LexTokenStartingWithALetter(const std::string &input, size_t &loc);

  void LexNumberConstant(const std::string &input, size_t &loc);

public:

  std::vector<std::string> errors;
  std::vector<Token> &tokens;

  Lexer(std::vector<Token> &tokens_);

  static bool IsReservedWord(TokenType type);
  static bool IsInfixOperator(TokenType);
  static bool IsOperator(TokenType);

  void Error(const std::string &msg) { errors.emplace_back(msg); }

  char GetCharAt(size_t loc);

  TokenType GetTokenAt(size_t loc)
  {
    if(loc >= tokens.size())
      return TOKEN_INVALID;
    if(tokens.size() <= 0)
      return TOKEN_INVALID;
    return tokens[loc].type;
  }

  void Lex(const std::string &input);

};
