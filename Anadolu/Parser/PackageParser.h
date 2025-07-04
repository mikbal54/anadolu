#pragma once

#include "Lexer.h"
#include "PackageInfo.h"
#include "Ast.h"
#include "PrimitiveTypes.h"

#include <functional>

class Package;

enum MessageLevel
{
  MESSAGE_WARNING,
  MESSAGE_ERROR
};

class PackageParser
{ 
private:

  friend class PackageParserSemantic;
  friend class BytecodeGenerator;
  friend class Package;
  friend class ExpressionValue;
  friend class Expression;
  friend class Designator;

  Package *package;

  PackageInfo &packageInfo;

  size_t currentTokenPos;

  enum ParserStatus
  {
    NoError,
    MinorError, // there is an error but we can continue parsing
    CriticalError // parser is fucked, all possible errors after this are incomprehensible 
  }status; 

  // check if current operator is a legitimate operator. This assumes LookAhead(0) is an operator. If not it returns true. 
  // Generates error, but does not Consume current token
  bool IsOperatorLegal();
  INT OperatorArgumentCount(TokenType);
  bool IsLeftAssociated(TokenType);
  INT GetPrecedence(TokenType);

  Node *ParseDecrementStatement(Node *designator, Node *parent);
  Node *ParseIncrementStatement(Node *designator, Node *parent);

  Node *ParseDesignator();

  // identifier
  Node *ParseTypeLegends(Node *parent);
  // {FunctionDefinition}
  Node *ParseTypeDefinitionBlock(Node *parent);
  // 'type' identifier [ ':' TypeLegend {',' TypeLegend } ] '{' TypeDefinitionBlock '}'
  Node *ParseTypeDefinition(Node *parent);

  // current position is last token of expression, return resulting node. nullptr if error found
  Node* ParseExpression(Node *parent, bool reportErrors = true);

  void ParsePackage(Node *parent);

  // 'if' '(' expression ')' statement {'elif' '(' expression ')' statement} ['else' statement] 
  Node* ParseIfStatement();

  bool ParseWhileStatement(Node *parent);

  // 'var' identifier [ ':' identifier ] ['=' expression ] ';' 
  bool ParseVariableDeclaration(Node *parent);

  // '=' expression
  bool ParseAssignment(Node *parent);

  // variableDeclaration ';'
  // expression ';'
  // ifStatement 
  // 'while' '(' expression ')' block
  bool ParseStatement(Node *parent);

  // '{' {statement} '}'
  bool ParseBlock(Node *parent);

  bool ParseParameter(Node *parent);
  bool ParseParameterList(Node *parent);
  bool ParseFunctionDefinition(Node *parent);

  void ParseFunctionCall(Node *parent);

  // ':' variableType
  bool ParseTypeDeclaration(Node *parent);

  // INT, bool, float, identifier etc... (does not include void)
  bool IsTokenVariableType(TokenType type);
  bool IsTokenAType(TokenType type); // same as IsTokenVariableType but also accepts void

  size_t GetCurrentTokenPos() { return currentTokenPos; }

  INT GetTokenIntValue(INT pos) { return packageInfo.tokens[pos].intValue ; } 

  // return type of the token, if amount=0 then returns current token 
  TokenType LookAhead(INT amount)
  { 
    size_t pos = (size_t)(currentTokenPos + amount) ;
    if( pos < packageInfo.tokens.size())
      return packageInfo.tokens[pos].type;
    return TOKEN_INVALID;
  }

  TokenType GetTokenType(size_t pos)
  {
    if( pos < packageInfo.tokens.size())
      return packageInfo.tokens[pos].type;
    return TOKEN_INVALID;
  }

  std::string GetTokenAsString(size_t pos)
  {
    return packageInfo.GetTokenAsString(pos);
  }

  // moves current position ahead ignores all new line
  void ConsumeIgnoreNewLine()
  {
    while(true)
    {
      currentTokenPos++;
      if( (size_t)currentTokenPos >= packageInfo.tokens.size())
        break;
      if(packageInfo.tokens[currentTokenPos].type != TOKEN_NEWLINE)
        break;
    }
  }
  // Moves current token 1 position ahead and returns current Token
  void Consume() { currentTokenPos++; }
  // Move current position 1 position back
  void Rewind() { currentTokenPos--; }
  // rewind to a position given
  void RewindTo(size_t pos) { currentTokenPos = pos; }

  // Rewinds 1 position back, however jump over newline tokens
  void RewindIgnoreNewline() 
  {
    while(true)
    {
      currentTokenPos--;
      if( (size_t)currentTokenPos <= 0)
        break;
      if(packageInfo.tokens[currentTokenPos].type != TOKEN_NEWLINE)
        break;
    }
  };

  void ErrorMinor(const std::string &msg)
  {
    // if there isn't an error set yet, set minor error
    if(status == NoError)
      status = MinorError;

    if(!outputFunction)
      return;
    // TODO: line number and column number
    outputFunction(msg, 0, 0, MESSAGE_ERROR);
  }

  void ErrorCritical(const std::string &msg)
  {
    // if there isn't an error set yet, set minor error
    if(status != CriticalError)
      status = CriticalError;

    if(!outputFunction)
      return;
    // TODO: line number and column number
    outputFunction(msg, 0, 0, MESSAGE_ERROR);
  }


public:

  std::function<void(const std::string &msg, INT row, INT column, MessageLevel messageLevel)> outputFunction;

  PackageParser(PackageInfo &_package): packageInfo(_package), currentTokenPos(0), status(NoError) {}

  Package* Parse();

  Node *GetMainNode() { return packageInfo.ast.mainNode; }

};