#include "PackageParser.h"
#include "PackageParserSemantic.h"
#include "Package.h"
#include "PrimitiveTypes.h"
#include <assert.h>
#include <stack>

//TODO: remove this
#include <chrono>

Node *PackageParser::ParseIncrementStatement(Node *designator, Node *parent)
{
  Node *statement = new Node(IncrementStatementNode);
  statement->AddChild(designator);
  statement->startToken = designator->startToken;

  Consume(); // at ++ after this

  statement->endToken = GetCurrentTokenPos();

  if(LookAhead(1) != TOKEN_NEWLINE)
    ErrorMinor("Expected a newline"); // don't set failed statement for that. Report error allow continued parse

  parent->AddChild(statement);
  return statement;
}


Node *PackageParser::ParseDecrementStatement(Node *designator, Node *parent)
{
  Node *statement = new Node(DecrementStatementNode);
  statement->AddChild(designator);
  statement->startToken = designator->startToken;

  Consume(); // at -- after this

  statement->endToken = GetCurrentTokenPos();

  if(LookAhead(1) != TOKEN_NEWLINE)
    ErrorMinor("Expected a newline"); // don't set failed statement for that. Report error allow continued parse

  parent->AddChild(statement);
  return statement;
}


Node *PackageParser::ParseDesignator()
{
  Node *designatorNode = new Node(DesignatorNode, GetCurrentTokenPos(), GetCurrentTokenPos());

  //now on identifier, moves current token to TOKEN_DOT
  //ConsumeIgnoreNewLine();
  // moves current token to TOKEN_IDENTIFIER
  //ConsumeIgnoreNewLine();

  if(LookAhead(0) != TOKEN_IDENTIFIER)
  {
    ErrorCritical("Expected identifier");
    return designatorNode;
  }

  // first identifier
  if(LookAhead(1) == TOKEN_OPEN_PAREN)
    ParseFunctionCall(designatorNode);
  else
    designatorNode->AddChild( new Node(IdentifierNode, GetCurrentTokenPos(), GetCurrentTokenPos()) );

  ConsumeIgnoreNewLine(); // moves to .

  if(LookAhead(0) == TOKEN_DOT)
  {
    while(LookAhead(0) == TOKEN_DOT)
    {
      ConsumeIgnoreNewLine();
      if(LookAhead(1) == TOKEN_OPEN_PAREN)
        ParseFunctionCall(designatorNode);
      else
        designatorNode->AddChild( new Node(IdentifierNode, GetCurrentTokenPos(), GetCurrentTokenPos()) );
      ConsumeIgnoreNewLine();
    }

    RewindIgnoreNewline(); // make sure last token belongs to this designator
  }
  else
    RewindIgnoreNewline(); // make sure last token belongs to this designator

  designatorNode->endToken = GetCurrentTokenPos();
  return designatorNode;
}

Package *PackageParser::Parse()
{

  package = new Package(this);

  { // TODO: remove timing
    auto start = std::chrono::system_clock::now();
    packageInfo.Lex();
    // std::cout << "Lex took: " << std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now() - start).count() << "\n";
  }


  // TODO: remove timing
  auto start = std::chrono::system_clock::now();

  Node *mainNode = GetMainNode();

  TokenType token = LookAhead(0);
  while(token != TOKEN_INVALID && status != CriticalError)
  {
    switch(LookAhead(0))
    {
    case TOKEN_PACKAGE:
      ParsePackage(mainNode);
      break;
    case TOKEN_DEF:
      ParseFunctionDefinition(mainNode);
      break;
    case TOKEN_VAR:
      ParseVariableDeclaration(mainNode);
      break;
    case TOKEN_TYPE:
      ParseTypeDefinition(mainNode);
      break;
    case TOKEN_NEWLINE:
      break; // ignore new line
    default:
      // TODO: predict error and use ErrorMinor as much as possible
      ErrorCritical("Unexpected token");
      break;
    }

    ConsumeIgnoreNewLine();
    token = LookAhead(0);
  }

  //TODO: remove timing
  //std::cout << "Parse took: " << std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now() - start).count() << "\n";

  if( status == NoError )
  {
    PackageParserSemantic semanticParser(*this);
    semanticParser.Parse();
  }

  if(status == NoError)
  {
    return package;
  }
  else
  {
    delete package;
    return nullptr;
  }
}

Node *PackageParser::ParseTypeLegends(Node *parent)
{
  Node *legendsNode = new Node(TypeLegendsNode, GetCurrentTokenPos(), GetCurrentTokenPos());
  parent->AddChild(legendsNode);

  ConsumeIgnoreNewLine();

  if(LookAhead(0) == TOKEN_IDENTIFIER)
  {

    legendsNode->AddChild(new Node(IdentifierNode, GetCurrentTokenPos(), GetCurrentTokenPos()));

    ConsumeIgnoreNewLine();
    while(LookAhead(0) != TOKEN_OPEN_CURLY)
    {
      if(LookAhead(0) == TOKEN_COMMA)
      {
        ConsumeIgnoreNewLine();
        if(LookAhead(0) == TOKEN_IDENTIFIER)
          legendsNode->AddChild(new Node(IdentifierNode, GetCurrentTokenPos(), GetCurrentTokenPos()));
        else
        {
          // TODO: better error
          ErrorCritical("Expected ,");
          return legendsNode;
        }
      }
      else
      {
        // TODO: better error
        ErrorCritical("Expected ,");
        return legendsNode;
      }
      ConsumeIgnoreNewLine();
    }


  }
  else
  {
    // TODO: better error
    ErrorCritical("Expected an interface or type legend");
    return legendsNode;
  }

  legendsNode->endToken = GetCurrentTokenPos();


  return legendsNode;
}

Node *PackageParser::ParseTypeDefinition(Node *mainNode)
{
  Node *typeDefNode = new Node(TypeDefinitionNode);
  typeDefNode->startToken = GetCurrentTokenPos();
  typeDefNode->endToken = GetCurrentTokenPos();
  mainNode->AddChild(typeDefNode);

  ConsumeIgnoreNewLine();

  if(LookAhead(0) == TOKEN_IDENTIFIER)
    typeDefNode->AddChild(new Node(IdentifierNode, GetCurrentTokenPos(), GetCurrentTokenPos()));
  else
  {
    // TODO: better error
    ErrorCritical("Expected a type name");
    return typeDefNode;
  }

  ConsumeIgnoreNewLine();

  // possible type legends
  if(LookAhead(0) == TOKEN_COLON)
  {
    ParseTypeLegends(typeDefNode);
  }

  if(status == CriticalError)
    return typeDefNode;

  ConsumeIgnoreNewLine(); // enters definition block

  while( LookAhead(0) != TOKEN_CLOSE_CURLY )
  {
    if(LookAhead(0) == TOKEN_DEF)
      ParseFunctionDefinition(typeDefNode);
    else if(LookAhead(0) == TOKEN_VAR)
      ParseVariableDeclaration(typeDefNode);

    ConsumeIgnoreNewLine();
  }

  return typeDefNode;
}

void PackageParser::ParsePackage(Node *parent)
{
  Consume();

  if(LookAhead(0) != TOKEN_IDENTIFIER)
  {
    ErrorMinor("Package name expected after 'package'");
    return;
  }

  if(package->name.size())
  {
    ErrorMinor("Package name declared more than once");
    return;
  }

  package->name = GetTokenAsString(GetCurrentTokenPos());
}

bool PackageParser::ParseAssignment(Node *parent)
{
  if(LookAhead(0) != TOKEN_ASSIGN)
    return false;

  Consume();

  Node *assignmentNode = new Node(AssignmentNode);
  assignmentNode->startToken = GetCurrentTokenPos();
  parent->AddChild(assignmentNode);
  Consume();

  Node *r = ParseExpression(assignmentNode);
  assignmentNode->endToken = GetCurrentTokenPos();

  if(r)
    return true;
  else
    return false;
}

bool PackageParser::ParseTypeDeclaration(Node *parent)
{
  bool result = true;
  if(LookAhead(0) != TOKEN_COLON)
  {
    ErrorMinor("Missing :");
    result = false;
  }

  Node *typeDeclaration = new Node(TypeNameNode);
  parent->AddChild(typeDeclaration);

  Consume();

  if(!IsTokenVariableType(LookAhead(0)))
  {
    ErrorMinor("Not a valid variable type");
    result = false;
  }

  typeDeclaration->startToken = GetCurrentTokenPos();
  typeDeclaration->endToken = GetCurrentTokenPos();

  return result;
}

bool PackageParser::ParseVariableDeclaration(Node *parent)
{
  if(LookAhead(0) != TOKEN_VAR)
    return false;

  bool result = true;

  Node *variableDeclaration = new Node(VariableDeclarationNode);
  parent->AddChild(variableDeclaration);
  variableDeclaration->startToken = GetCurrentTokenPos();

  Consume();

  if(LookAhead(0) != TOKEN_IDENTIFIER)
  {
    result = false;
    ErrorMinor("Expected Identifier");
  }

  variableDeclaration->AddChild(new Node(IdentifierNode, GetCurrentTokenPos(), GetCurrentTokenPos()));

  // parse possible type 
  if(LookAhead(1) == TOKEN_COLON)
  {
    Consume();
    bool r = ParseTypeDeclaration(variableDeclaration);
    if(!r)
      result = false;
  }

  if(LookAhead(1) == TOKEN_ASSIGN)
  {
    Consume();
    bool r = ParseAssignment(variableDeclaration);
    if(!r)
      result = false;
  }

  Consume();
  if(LookAhead(0) != TOKEN_NEWLINE)
  {
    ErrorMinor("Missing ;");
    Rewind(); // assume ; is there. continue parsing
  }

  return result;
}

bool PackageParser::ParseWhileStatement(Node *parent)
{
  bool whileFailed = false;

  if(LookAhead(0) != TOKEN_WHILE)
    return false;

  Node *whileStatement = new Node(WhileNode);
  parent->AddChild(whileStatement);
  whileStatement->startToken = GetCurrentTokenPos();

  Consume();


  Node *hasExpression = ParseExpression(whileStatement);
  if(!hasExpression)
  {
    whileFailed = true;
    ErrorMinor("while statement has invalid expression");
  }


  Consume();
  if(LookAhead(0) != TOKEN_NEWLINE)
  {
    whileFailed = true;
    ErrorMinor("While expression should end with a newline or ';'");
  }

  Consume();

  bool hasStatement = ParseStatement(whileStatement);
  if(hasStatement)
    whileFailed = true;

  whileStatement->endToken = GetCurrentTokenPos();

  return whileFailed;
}

Node *PackageParser::ParseIfStatement()
{
  if(LookAhead(0) != TOKEN_IF)
    return false;

  bool ifStatementResult = true;

  Node *ifStatement = new Node(IfNode);
  ifStatement->startToken = GetCurrentTokenPos();

  // jump to expression start 
  Consume();

  Node *expression = ParseExpression(ifStatement);
  if(!expression) ifStatementResult = false; // set result false if can't parse expression

  if(!ifStatementResult)
    ErrorMinor("Invalid expression in if statement");

  Consume();

  if(LookAhead(0) != TOKEN_NEWLINE)
    ErrorMinor("If statement must end with a newline or a ';' ");

  ConsumeIgnoreNewLine();
  ifStatementResult = ParseStatement(ifStatement);

  Consume();
  // consume all elif expressions
  while(ifStatementResult)
  {
    if(LookAhead(0) == TOKEN_ELIF)
    {
      Node *elseIf = new Node(ElseIfNode);
      elseIf->startToken = GetCurrentTokenPos();
      Consume();
      if(LookAhead(0) == TOKEN_OPEN_PAREN)
      {

        Consume();
        Node  *expr = ParseExpression(elseIf);
        if(!expr)
        {
          ifStatementResult = false;
          ErrorMinor("Invalid expression in elif statement");
        }

        Consume();
        if(LookAhead(0) != TOKEN_CLOSE_PAREN)
        {
          ifStatementResult = false;
          ErrorMinor("Missing )");
        }

        Consume();
        bool hasStatement = ParseStatement(elseIf);

        if(!hasStatement || !expression)
        {
          delete elseIf;
          ifStatementResult = false;
        }
        else
        {
          elseIf->endToken = GetCurrentTokenPos();
          ifStatement->AddChild(elseIf);
          Consume();
        }
      }
      else
      {
        delete elseIf;
        ErrorMinor("Expected (");
        ifStatementResult = false;
        Rewind(); // when error happens last token of if statement should be 'else if' token
      }

    }
    else // there aren't any elifs anymore.
    {
      Rewind();
      break;
    }
  }

  if(!ifStatementResult)
  {
    delete ifStatement;
    return false;
  }

  Consume();
  // try for an else statement
  if(LookAhead(0) == TOKEN_ELSE)
  {
    Node *elseStatement = new Node(ElseNode);
    elseStatement->startToken = GetCurrentTokenPos();

    Consume();
    ParseStatement(elseStatement);
    elseStatement->endToken = GetCurrentTokenPos();
    ifStatement->AddChild(elseStatement);
  }
  else
    Rewind(); // rewind so we are at last token of the if statement

  if(ifStatementResult)
  {
    ifStatement->endToken = GetCurrentTokenPos();
  }
  else
  {
    delete ifStatement;
    ifStatement = nullptr;
  }

  return ifStatement;
}

bool PackageParser::ParseStatement(Node *parent)
{
  bool result = true;
  // try expression

  bool r = false;

  TokenType token = LookAhead(0);

  switch(token)
  {
  case TOKEN_NEWLINE: // just an extra ; . useful for things like=>  while(expr);
    break;  // do what? just an empty statement
  case TOKEN_RETURN:
    {
      Node *statement = new Node(ReturnStatementNode);
      statement->startToken = GetCurrentTokenPos();
      Consume();
      ParseExpression(statement);
      statement->endToken = GetCurrentTokenPos();
      Consume(); 
      if(LookAhead(0) != TOKEN_NEWLINE)
      {
        ErrorMinor("Missing ;");
        Rewind();
      }
      parent->AddChild(statement);
    }
    break;
  case TOKEN_BREAK:
    {
      Node *statement = new Node(BreakStatementNode);
      statement->startToken = GetCurrentTokenPos();
      statement->endToken = GetCurrentTokenPos();
      Consume();
      if(LookAhead(0) != TOKEN_NEWLINE)
      {
        ErrorMinor("Missing ;");
        Rewind();
      }
    }
    break;
  case TOKEN_OPEN_PAREN: // starts with a ( the its an invoke statement, which starts with an expression
    {
      Node *invoke = new Node();
      invoke->type = InvokeStatementNode;
      invoke->startToken = GetCurrentTokenPos();
      Node *r = ParseExpression(invoke); // expression ends with last token it has
      invoke->endToken = GetCurrentTokenPos();
      parent->AddChild(invoke);
      if(r)
      {
        Consume();
        if(LookAhead(0) != TOKEN_NEWLINE)
        {
          ErrorMinor("Expected a newline"); // don't set failed statement for that. Report error allow continued parse
          Rewind();
        }
      }
      else
      {
        result = false;
      }
    }
    break;
  case TOKEN_CONTINUE:
    {
      Node *statement = new Node(ContinueStatementNode);
      statement->startToken = GetCurrentTokenPos();
      statement->endToken = GetCurrentTokenPos();
      Consume();
      if(LookAhead(0) != TOKEN_NEWLINE)
      {
        ErrorMinor("Missing ;");
        Rewind();
      }
    }
    break;
  case TOKEN_IDENTIFIER: // starts with an identifier, then it can be an expression or an assignment statement
    {
      Node *desig = ParseDesignator();

      Consume();

      if(LookAhead(0) == TOKEN_ASSIGN)
      {

        Node *assignmentNode = new Node(AssignmentNode);
        assignmentNode->startToken = desig->startToken;
        assignmentNode->AddChild(desig);
        Consume(); // move to start of the expression
        ParseExpression(assignmentNode);
        assignmentNode->endToken = GetCurrentTokenPos();
        parent->AddChild(assignmentNode);

        Consume();
        if(LookAhead(0) != TOKEN_NEWLINE)
        {
          ErrorMinor("Expected a newline");
          Rewind();
        }
      }
      else if(LookAhead(0) == TOKEN_INCREMENT)
      {
        Rewind();
        ParseIncrementStatement(desig, parent);
      }
      else if(LookAhead(0) == TOKEN_DECREMENT)
      {
        Rewind();
        ParseDecrementStatement(desig, parent);
      }
      else
      {
        RewindTo(desig->startToken);
        delete desig;
        Node *invoke = new Node();
        invoke->type = InvokeStatementNode;
        invoke->startToken = GetCurrentTokenPos();
        Node *r = ParseExpression(invoke); // expression ends with last token it has
        invoke->endToken = GetCurrentTokenPos();
        parent->AddChild(invoke);
        if(r)
        {
          Consume();
          if(LookAhead(0) != TOKEN_NEWLINE)
          {
            ErrorMinor("Expected a newline"); // don't set failed statement for that. Report error allow continued parse
            Rewind();
          }
        }
        else
        {
          result = false;
        }
      }
      break;
    }
  case TOKEN_IF:
    {
      Node *ifStmnt = ParseIfStatement();
      parent->AddChild(ifStmnt);
    }
    break;
  case TOKEN_WHILE:
    r = ParseWhileStatement(parent);
    break;
  case TOKEN_OPEN_CURLY:
    r = ParseBlock(parent);
    break;
  default:
    ErrorCritical("Unexpected token");
    break;
  }

  return result;
}



// INT, bool, float, identifier etc... (does not include void)
bool PackageParser::IsTokenVariableType(TokenType type)
{
  switch (type)
  {
  case TOKEN_IDENTIFIER:
    return true;
  case TOKEN_INT:
    return true;
  case TOKEN_BOOL:
    return true;
  default:
    return false;
  }
}

// same as IsTokenVariableType but also accepts void
bool PackageParser::IsTokenAType(TokenType type)
{
  if(IsTokenVariableType(type))
    return true;
  else if(type == TOKEN_VOID)
    return true;
  return false;
}

bool PackageParser::ParseBlock(Node *parent)
{
  if(LookAhead(0) != TOKEN_OPEN_CURLY)
    return false;

  Node *block = new Node(BlockNode);
  block->startToken = GetCurrentTokenPos();
  ConsumeIgnoreNewLine();

  // proceed till we find a }
  while(LookAhead(0) != TOKEN_CLOSE_CURLY && status != CriticalError)
  {
    bool result;
    switch (LookAhead(0))
    {
    case TOKEN_VAR:
      result = ParseVariableDeclaration(block);
      break;
    default:
      result = ParseStatement(block);
      break;
    }
    ConsumeIgnoreNewLine();

    if(!result)
      break;
  }

  // must end with '}' . MUST!
  if(LookAhead(0) != TOKEN_CLOSE_CURLY)
  {
    ErrorMinor("Block should end with }");
    delete block;
    return false;
  }

  Consume();
  if(LookAhead(0) == TOKEN_INVALID) // end of section
  {
    Rewind();
  }
  else if(LookAhead(0) != TOKEN_NEWLINE )
  {
    ErrorMinor("Missing newline at the end of block");
    Rewind();
  }

  block->endToken = GetCurrentTokenPos();
  parent->AddChild(block);

  return true;
}

bool PackageParser::ParseParameter(Node *parent)
{

  if(LookAhead(0) != TOKEN_IDENTIFIER)
    return false;

  // end position updated if typename exists
  Node *parameter = new Node(ParameterNode, GetCurrentTokenPos(), GetCurrentTokenPos());
  parent->AddChild(parameter);

  // name of the paramater
  parameter->AddChild(new Node(IdentifierNode, GetCurrentTokenPos(), GetCurrentTokenPos()));

  // parse optional type identifier
  if(LookAhead(1) == TOKEN_COLON)
  {
    Consume();
    bool r = ParseTypeDeclaration(parameter);
    if(r)
      parameter->endToken = GetCurrentTokenPos();
  }

  return true;
}

bool PackageParser::ParseParameterList(Node *parent)
{
  if(LookAhead(0) != TOKEN_IDENTIFIER)
    return false;

  Node *parameterList = new Node(ParameterListNode);
  parent->AddChild(parameterList);
  parameterList->startToken = GetCurrentTokenPos();
  while(ParseParameter(parameterList))
  {
    if(LookAhead(1) != TOKEN_COMMA) // peek 1 ahead for comma
      return true;
    Consume();Consume(); // jump over comma
  }
  parameterList->endToken = GetCurrentTokenPos();
  return true;
}

bool PackageParser::ParseFunctionDefinition(Node *parent)
{
  if( LookAhead(0) != TOKEN_DEF)
    return false;

  if(LookAhead(1) != TOKEN_IDENTIFIER)
    return false;

  if(LookAhead(2) != TOKEN_OPEN_PAREN)
    return false;

  Node *functionDeclaration = new Node(FunctionDeclarationNode); 
  functionDeclaration->startToken = GetCurrentTokenPos();
  functionDeclaration->AddChild(new Node(IdentifierNode, GetCurrentTokenPos() + 1, GetCurrentTokenPos() + 1));

  // move 3 times to arrive at parameter list
  Consume();Consume();Consume();

  if(LookAhead(0) != TOKEN_CLOSE_PAREN)
  {
    ParseParameterList(functionDeclaration); // ends with the last typename or identifier.
    Consume(); // moves to )
  }
  else // empty parameter list
  {
    functionDeclaration->AddChild(new Node(ParameterListNode, GetCurrentTokenPos(), GetCurrentTokenPos()) );
  }


  if(LookAhead(0) != TOKEN_CLOSE_PAREN)
  {
    ErrorMinor("Expected ) ");
    delete functionDeclaration;
    return false;
  }

  //Consume();  
  ConsumeIgnoreNewLine();
  ParseBlock(functionDeclaration);

  functionDeclaration->endToken = GetCurrentTokenPos();
  parent->AddChild(functionDeclaration);
  return true;
}

void PackageParser::ParseFunctionCall(Node *parent)
{
  if(LookAhead(0) == TOKEN_IDENTIFIER && LookAhead(1) == TOKEN_OPEN_PAREN)
  {
    bool isParseFailed = false;
    Node *functionCall = new Node(FunctionCallNode);
    functionCall->startToken = GetCurrentTokenPos();
    // consume twice to move to parameters
    Consume();
    Consume();

    while(LookAhead(0) != TOKEN_CLOSE_PAREN)
    {
      Node *expr = ParseExpression(functionCall);
      Consume();

      TokenType token = LookAhead(0);
      if(token != TOKEN_COMMA)
        break;
      Consume(); // jump over comma
    }


    if(isParseFailed)
    {
      delete functionCall;
    }
    else
    {
      functionCall->endToken = GetCurrentTokenPos();
      parent->AddChild(functionCall);
    }
  }



}

bool PackageParser::IsOperatorLegal()
{
  TokenType token = LookAhead(0);

  switch (token)
  {
  case TOKEN_INCREMENT:
    if(LookAhead(-1) != TOKEN_IDENTIFIER)
    {
      ErrorMinor("Increment operator can only be used on variables");
      return false;
    }
    break;
  case TOKEN_DECREMENT:
    if(LookAhead(1) != TOKEN_IDENTIFIER ||  LookAhead(-1) != TOKEN_IDENTIFIER)
    {
      ErrorMinor("Decrement operator can only be used on variables");
      return false;
    }
    break;
  default:
    break;
  }

  return true;
}



INT PackageParser::OperatorArgumentCount(TokenType type)
{
  switch (type)
  {
  case TOKEN_PLUS:
  case TOKEN_MINUS:
  case TOKEN_STAR:
  case TOKEN_SLASH:
  case TOKEN_PERCENT:
  case TOKEN_EQUAL:
  case TOKEN_NOTEQUAL:
  case TOKEN_AND:
  case TOKEN_OR:
  case TOKEN_LESSTHAN:
  case TOKEN_LESSTHANEQUAL:
  case TOKEN_GREATERTHAN:
  case TOKEN_GREATERTHANEQUAL:
    return 2;
  case TOKEN_NOT:
  case TOKEN_INCREMENT:
  case TOKEN_DECREMENT:
  case TOKEN_NEGATE:
    return 1;
  default:
    assert(0);
    return 1;
  }
}

bool PackageParser::IsLeftAssociated(TokenType type)
{
  switch (type)
  {
  case TOKEN_PLUS:
  case TOKEN_MINUS:
  case TOKEN_STAR:
  case TOKEN_SLASH:
  case TOKEN_PERCENT:
  case TOKEN_EQUAL:
  case TOKEN_NOTEQUAL:
  case TOKEN_AND:
  case TOKEN_OR:
  case TOKEN_LESSTHAN:
  case TOKEN_LESSTHANEQUAL:
  case TOKEN_GREATERTHAN:
  case TOKEN_GREATERTHANEQUAL:
    return true;
  case TOKEN_NOT:
  case TOKEN_NEGATE:
    return false;
  default:
    assert(0);
    return false;
  }
}

INT PackageParser::GetPrecedence(TokenType type)
{ 
  switch (type)
  {
  case TOKEN_NOT:
  case TOKEN_NEGATE:
    return 18-3;
  case TOKEN_STAR:
  case TOKEN_SLASH:
  case TOKEN_PERCENT:
    return 18-5;
  case TOKEN_PLUS:
  case TOKEN_MINUS:
    return 18-6;
  case TOKEN_LESSTHAN:
  case TOKEN_LESSTHANEQUAL:
  case TOKEN_GREATERTHAN:
  case TOKEN_GREATERTHANEQUAL:
    return 18-8;
  case TOKEN_EQUAL:
  case TOKEN_NOTEQUAL:
    return 18-9;
  case TOKEN_AND:
    return 18-13;
  case TOKEN_OR:
    return 18-14;
  default:
    assert(0);
    return 0;
  }
}


Node *PackageParser::ParseExpression(Node *parent, bool reportErrors)
{
  // Turns infix to postfix. (Shunting-yard)
  // Then adds the result as a single ExpressionNode child to parent

  std::stack<INT> output;
  std::stack< std::pair<INT, TokenType> > stack;

  TokenType token = LookAhead(0);

  // add start and end locations later
  Node *expression = new Node(ExpressionNode);
  expression->startToken = GetCurrentTokenPos();

  INT numOfConstants = 0, numOfDoubleOperators = 0, numberOfSingleOperator = 0;
  bool endOfStream = false;
  bool endOfExpression = false;
  bool isFailed = false;

  INT openParanOnStack = 0;

  while(true)
  {
    switch (token)
    {
    case TOKEN_INVALID: //end of stream;
      endOfStream = true;
      ErrorMinor("Expression ended unexpectedly");
      break;
    case TOKEN_COMMA: // comma means end of this expression
    case TOKEN_NEWLINE: // semicolon means end of this expression
    case TOKEN_CLOSE_CURLY: // }
      endOfExpression = true;
      Rewind(); // rewind so , and ; are not part of the expression node
      break;
    case TOKEN_IDENTIFIER:
      {
        Node *desig = ParseDesignator();
        expression->AddChild(desig);
      }
      numOfConstants++;
      break;
    case TOKEN_CONSTANT_INT:
    case TOKEN_CONSTANT_TRUE:
    case TOKEN_CONSTANT_FALSE:
      expression->AddChild(new Node(Node::GetTokenNodeType(token), GetCurrentTokenPos(), GetCurrentTokenPos()));
      numOfConstants++;
      break;
    case TOKEN_OPEN_PAREN:
      stack.push( std::make_pair(GetCurrentTokenPos(), LookAhead(0)));
      openParanOnStack++;
      break;
    case TOKEN_CLOSE_PAREN:
      if(stack.empty() || openParanOnStack == 0) // must be the end of the expression.
      {
        endOfExpression = true;
        Rewind(); // rewind so ) is not part of the expression node
      }
      else
      {
        bool poppedOperator = false;
        while(stack.top().second != TOKEN_OPEN_PAREN)
        {
          poppedOperator = true;
          expression->AddChild(new Node(Node::GetTokenNodeType(GetTokenType(stack.top().first)), stack.top().first, stack.top().first));

          if(OperatorArgumentCount(stack.top().second) == 2)
            numOfDoubleOperators++;
          else
            numberOfSingleOperator++;

          stack.pop();
        }

        // at this point  ( must be on top of stack
        if(stack.top().second == TOKEN_OPEN_PAREN)
        {
          poppedOperator = true;
          stack.pop();
          openParanOnStack--;
        }

        if(!poppedOperator)
          ErrorCritical("No expression before )");
      }
      break;
    case TOKEN_DECREMENT:
      ErrorMinor("Decrement '--' is not allowed in expression. Use it as a seperate statement");
      break;
    case TOKEN_INCREMENT:
      ErrorMinor("Increment '++' is not allowed in expression. Use it as a seperate statement");
      break;
    case TOKEN_NEGATE:
    case TOKEN_STAR: // all operators
    case TOKEN_MINUS:
    case TOKEN_PLUS:
    case TOKEN_SLASH:
    case TOKEN_PERCENT:
    case TOKEN_NOT:
    case TOKEN_EQUAL:
    case TOKEN_NOTEQUAL:
    case TOKEN_AND:
    case TOKEN_OR:
    case TOKEN_LESSTHAN:
    case TOKEN_LESSTHANEQUAL:
    case TOKEN_GREATERTHAN:
    case TOKEN_GREATERTHANEQUAL:
      if(!IsOperatorLegal())
        break; // breaks from switch. 

      if(token == TOKEN_MINUS)
        if( numOfConstants == 0)
          ErrorMinor("Invalid expression");

      while(!stack.empty() && Lexer::IsOperator(stack.top().second) )
      {

        if(
          ( IsLeftAssociated(token) && GetPrecedence(token) <= GetPrecedence(stack.top().second) ) // left associativity
          || ( !IsLeftAssociated(token) && GetPrecedence(token) < GetPrecedence(stack.top().second) ) // right associativity
          )
        {

          if(OperatorArgumentCount(stack.top().second) == 2)
            numOfDoubleOperators++;
          else
            numberOfSingleOperator++;

          expression->AddChild(new Node( Node::GetTokenNodeType(GetTokenType(stack.top().first)), stack.top().first, stack.top().first));
          stack.pop();
        }
        else
          break;
      }

      stack.push( std::make_pair(GetCurrentTokenPos(), LookAhead(0)));
      break;
    default:
      endOfStream = true;
      isFailed = true;
      ErrorMinor("Expression ended unexpectedly");
      Rewind(); // so last token is where it failed
      break;
    }

    if(endOfExpression || endOfStream)
      break;

    Consume();
    token = LookAhead(0);
  }

  expression->endToken = GetCurrentTokenPos();

  // add tokens left in stack to output
  while(!stack.empty())
  {
    if(Lexer::IsOperator(stack.top().second))
    {
      expression->AddChild(new Node( Node::GetTokenNodeType(GetTokenType(stack.top().first)), stack.top().first, stack.top().first));
      if(OperatorArgumentCount(stack.top().second) == 2)
        numOfDoubleOperators++;
      else
        numberOfSingleOperator++;
      stack.pop();
    }
    else if(stack.top().second == TOKEN_OPEN_PAREN || stack.top().second == TOKEN_CLOSE_PAREN) // TODO: if there are only 1 close paran, then we might be in a if statement or a function call
    {
      isFailed = true;
      ErrorMinor("Mismatched parenthesis");
      stack.pop();
    }
    else
      assert(0); // only parans end operators must be on the stack. this means i forgot to add an operator to IsOperator function
  }

  if( numOfConstants == 0)
    ErrorMinor("Invalid expression");

  //  if( numberOfSingleOperator != 0)
  //    numOfConstants -= numberOfSingleOperator;

  if( numOfConstants != (numOfDoubleOperators + 1)  )
    ErrorMinor("Invalid expression");

  // if has children its an expression. Might still have caused parsing failure.
  if(!isFailed)
  {
    if(expression->HasChildren()) // empty
    {
      parent->AddChild(expression);
      return expression;
    }
    else
    {
      delete expression;
      return nullptr;
    }
  }
  else
  {
    delete expression;
    return nullptr;
  }
}