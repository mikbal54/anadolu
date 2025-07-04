#pragma once

#include "Lexer.h"
#include "Ast.h"

#include <vector>

class PackageInfo
{
private:
  friend class PackageParser;
  friend class Lex;
  
  AST ast;
  std::vector<Token> tokens;
  std::string script;

public:
  
  std::string name;

  std::string GetScriptSubStr(size_t begin, size_t length)
  {
    if(begin + length >= script.size())
      return "";
    return script.substr(begin, length);
  }

  bool IsTokenValid(size_t tokenNumber)
  {
    if(tokenNumber >= tokens.size())
      return false;
    return true;
  }

  TokenType GetTokenType(INT tokenNumber)
  {
    if( (size_t)tokenNumber < tokens.size())
      return tokens[tokenNumber].type;
    return TOKEN_INVALID;
  }

  std::string GetTokenAsString(size_t tokenNumber);

  void AddScriptSection(const std::string &section);

  void Lex();

};