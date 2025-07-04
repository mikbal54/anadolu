#pragma once

#include <vector>

class PackageParser;

class Node;
class Package;
class Block;
class Parameter;
class ParameterList;
class Function;
class GlobalFunction;
class Statement;
class Expression;
class VariableDecleration;
class ReturnStatement;
class Instruction;
class IncrementStatement;
class DecrementStatement;
class Type;
class Method;
class Designator;
class AssignmentStatement;
class IfStatement;
class WhileStatement;
class InvokeStatement;

class PackageParserSemantic
{
public:

  PackageParser &packageParser;

  PackageParserSemantic(PackageParser &_packageParser) : packageParser(_packageParser) 
  {

  }

  void GetByteCode(std::string &str);

  void Parse();

  /// THESE ARE FOR THE SECOND PASS AND BEYOND
  bool TryToCompleteDesignator(Designator *designator, Block *block);
  bool TryToCompleteStatement(Statement *statement);
  bool TryToCompleteExpression(Expression *expression);
  bool TryToCompleteIfStatement(IfStatement *ifStatement);
  bool TryToCompleteVariableDecleration(VariableDecleration *vdecl, Block *block);
  bool TryToCompleteBlock(Block *block);
  bool TryToCompleteGlobalFunction(GlobalFunction *function);
  ///

  InvokeStatement *ParseInvokeStatement( Node *invokeNode, Block *block);

  WhileStatement *ParseWhileStatement(Node *whileStatementNode, Function *function, Block *block);

  IfStatement *ParseIfStatement(Node *ifStatementNode, Function *function, Block *block);

  AssignmentStatement *ParseAssignmentStatement(Node *assignmentNode, Block *block);

  Designator *ParseDesignator(Node *designatorNode, Statement *statement);

  Block *PackageParserSemantic::CreateBlockAndVariables(Node *blockNode, Block *parentBlock, Function *function);

  void ScanFunctionForVariableDeclerations(Node *blockNode, Function *function);

  DecrementStatement *ParseDecrementStatement(Node *incrementStatementNode, Block *block);

  IncrementStatement *ParseIncrementStatement(Node *incrementStatementNode, Block *block);

  ReturnStatement *ParseReturnStatement(Node *returnStatementNode, Block *block);

  VariableDecleration *ParseVariableDecleration(Node *variableDeclerationNode);

  Expression *ParseExpression(Node *expressionNode, Statement *statement);

  Statement *ParseStatement(Node *statementNode, Block *block, Function *function);

  Parameter *ParseParameter(Node *parameterNode, Function *function);

  ParameterList *ParseParameterList(Node *paramaterList, Function *function);

  GlobalFunction* ParseGlobalFunction(Node *functionNode);

  Method* ParseMethod(Node *methodNode);

  Type *ParseTypeDefinition(Node *typeDefinitionNode);

};