#pragma once

#include "Node.h"
#include "PrimitiveTypes.h"

#include <string>
#include <unordered_map>
#include <unordered_set>

class Block;
class Function;
class Bytecode;
class Node;
class Statement;
class VariableDecleration;
class Package;
class PackageParser;
class PackageParserSemantic;
class Expression;
class Parameter;

enum BuiltinTypes
{
  TypeIdUnknown = 0, // type is explicitly decleared but not known what it is atm 
  TypeIdGeneric, // type is not explicitly declered
  TypeIdVoid, // used only for void returning functions 
  TypeIdInteger,
  TypeIdFloat,
  TypeIdDouble,
  TypeIdBool
};

enum StatementType
{
  ST_NoStatement,
  ST_Assignment,
  ST_IncrementStatement,
  ST_DecrementStatement,
  ST_InvokeStatement,
  ST_VariableDecleration,
  ST_BlockStatement,
  ST_BlockEndStatement, // a hidden statement, at the end of each block
  ST_IfStatement,
  ST_WhileStatement,
  ST_ReturnStatement
};

enum DesignatorType
{
  DT_Unknown,
  DT_LocalValue,
  DT_ParameterValue,
  DT_HeapValue,
  DT_FunctionCall
};

class Designator
{
public:

  std::string name;
  DesignatorType type;
  std::vector<Expression*> *expressions;
  INT typeId;
  Designator *parent;

  union
  {
    Function *function; // function this designator calls
    INT32 address; // address of the variable
  };

  bool isComplete;

  bool FindFunctionCall(Block *block, Package *package);
  Parameter *FindVariableInParameters(Function *function, Package *package);
  void CalculateAddress(Block *block, PackageParserSemantic *parser);

  Designator(): function(nullptr), isComplete(false), type(DT_Unknown), parent(nullptr), expressions(nullptr) {  }

  void Finalise()
  {
    // TODO: how about heap ?

  }

  virtual ~Designator();

};

enum ExpressionValueType
{
  EVT_Unknown,
  EVT_PlusOperator,
  EVT_EqualsOperator,
  EVT_NotEqualOperator,
  EVT_MinusOperator,
  EVT_NegateOperator,
  EVT_MultiplyOperator,
  EVT_DivideOperator,
  EVT_ConstInt, // also used for constant folding, so we don't generate unneeded instructions
  EVT_ConstBool,// also used for constant folding, so we don't generate unneeded instructions
  EVT_Designator,
  EVT_RegisterInt,
  EVT_RegisterBool
};

class ExpressionValue
{
public:

  ExpressionValueType type;

  ExpressionValue() : stringValue(0)
  {
    type = EVT_Unknown;
  }

  ExpressionValue(ExpressionValueType _type) : type(_type), stringValue(0)
  {
  }

  ExpressionValue(ExpressionValueType _type, INT32 value) : type(_type), intValue(value)
  {
  }

  ExpressionValue(bool value)
  {
    type = EVT_ConstBool;
    intValue = (INT32)value;
  }

  ExpressionValue(INT32 value) : intValue(value)
  {
    type = EVT_ConstInt; // TODO: replace this
  }

  ExpressionValue(float value) : floatValue(value)
  {
    type = EVT_ConstInt; // TODO: replace this
  }

  ExpressionValue(double value) : doubleValue(value)
  {
    type = EVT_ConstInt; // TODO: replace this
  }

  ExpressionValue(INT64 value) : longValue(value)
  {
    type = EVT_ConstInt; // TODO: replace this
  }

  ExpressionValue(Node * designatorNode, Statement *statement, PackageParserSemantic *parser);

  void Destroy()
  {
    if(type == EVT_Designator && stringValue)
      delete stringValue;
  }

  ~ExpressionValue()
  {
    //just deleting is not enough, call Destroy on it
  }

  union
  {
    long long longValue;
    double doubleValue;
    float floatValue;
    INT32 intValue;
    Designator *stringValue;
  };

};

class ExpressionTemp
{
public:


  ExpressionTemp()
  {

  }

};

class Expression
{
public:

  std::vector<ExpressionValue> expressionValues;
  Statement *statement;
  ExpressionTemp *temp;
  INT returnTypeId;
  bool isComplete;

  Expression(Statement *_statement) : statement(_statement), isComplete(true)
  {
    temp = new ExpressionTemp();
    returnTypeId = TypeIdUnknown;
  }

  // called if this expression complete. all types/designators are complete
  INT CheckTypeCorrectness(PackageParser &packageParser);

  void Finalise()
  {
    delete temp;
    temp = nullptr;
  }

  ~Expression()
  {
    if(temp)
      delete temp;
    for(auto &i:expressionValues)
      i.Destroy();
  }
};

class Statement
{
public:

  bool isComplete;
  Block *parentBlock;
  StatementType statementType;

  Statement(Block *_block) : parentBlock(_block), isComplete(false)
  {
    statementType = ST_NoStatement;
  }

  virtual ~Statement() { }

};


class VariableDeclerationStatement : public Statement
{
public:

  VariableDecleration *variableDecleration;

  VariableDeclerationStatement(Block *block, VariableDecleration *vdec ) : Statement(block), variableDecleration(vdec)
  {
    statementType = ST_VariableDecleration;
  }

  ~VariableDeclerationStatement();

};

class AssignmentStatementTemp
{
public:

  AssignmentStatementTemp()
  {

  }

};

class AssignmentStatement : public Statement 
{
public:

  Designator *leftHand;
  Expression *rightHand;
  AssignmentStatementTemp *temp;

  AssignmentStatement(Block *_block) : Statement(_block)
  {
    temp = new AssignmentStatementTemp();
    statementType = ST_Assignment;
  }

  void Finalise()
  {
    delete temp;
    temp = nullptr;
  }

  virtual ~AssignmentStatement()
  {
    if(temp)
      delete temp;
    delete leftHand;
    delete rightHand;
  }

};

class IncrementStatement : public Statement
{
public:

  Designator *designator;

  IncrementStatement(Block *_block) : Statement(_block) 
  {
    statementType = ST_IncrementStatement; 
  }

  IncrementStatement(Block *_block, Designator *_designator) : Statement(_block), designator(_designator)
  {
    statementType = ST_IncrementStatement; 
  }

  virtual ~IncrementStatement()
  {
    delete designator;
  }


};

class DecrementStatement : public Statement
{
public:

  Designator *designator;

  DecrementStatement(Block *_block) : Statement(_block) 
  {
    statementType = ST_DecrementStatement; 
  }

  DecrementStatement(Block *_block, Designator *_designator) : Statement(_block), designator(_designator)
  {
    statementType = ST_DecrementStatement; 
  }

  virtual ~DecrementStatement()
  {
    delete designator;
  }


};


class BlockStatement : public Statement
{
public:

  Block *block;

  BlockStatement(Block *_block) : Statement(_block)
  {
    block = 0;
    statementType = ST_BlockStatement;
  }

  virtual ~BlockStatement();

};

// Statements like=>  MyFunc() 
class InvokeStatement : public Statement
{
public:

  Expression *expression;

  InvokeStatement(Block *_block) : Statement(_block)
  {
    expression = 0;
    statementType = ST_InvokeStatement;
  }

  virtual ~InvokeStatement()
  {
    delete expression;
  }


};

class ReturnStatement : public Statement
{
public:

  Expression *expression;

  ReturnStatement(Block *_block) : Statement(_block)
  {
    expression = 0;
    statementType = ST_ReturnStatement;
  }

  virtual ~ReturnStatement()
  {
    delete expression;
  }

};

class IfStatement : public Statement
{
public:

  Expression *expression;
  Statement *statement;

  IfStatement(Block *_block) : Statement(_block)
  {
    statementType = ST_IfStatement;
    expression = 0;
    statement = 0;
  }

  virtual ~IfStatement()
  {
    delete statement;
    delete expression;
  }

};

class WhileStatement : public Statement
{
public:

  Expression *expression;
  Statement *statement;

  WhileStatement(Block *_block) : Statement(_block)
  {
    statementType = ST_WhileStatement;
    expression = 0;
    statement = 0;
  }

  virtual ~WhileStatement()
  {
    delete statement;
    delete expression;
  }

};

class VariableDeclerationTemp
{
public:

  std::string typeName;
  std::string name;

  VariableDeclerationTemp()
  {

  }

};

class VariableDecleration
{
public:

  bool isComplete;
  VariableDeclerationTemp *temp;
  INT variableType;
  INT32 position;

  VariableDecleration()  : isComplete(false)
  {
    position = 0;
    variableType = TypeIdUnknown;
    temp = new VariableDeclerationTemp();
  }

  void Finalise()
  {
    delete temp;
    temp = nullptr;
  }

  ~VariableDecleration()
  {
    if(temp)
      delete temp;
  }

};

class BlockTemp
{
public:

  std::unordered_map<std::string, size_t> unfinishedDeclerationStatements;

  // INT designates its position in 
  std::unordered_map<size_t, Statement*> unfinishedStatements;
  std::unordered_map<std::string, size_t> variableDeclerations;

  BlockTemp()
  {

  }

  ~BlockTemp()
  {
  }

};

class Block
{
private:

  friend class ExecutionContext;

public:

  std::vector<Statement*> statements;


  // starts true
  bool isComplete;

  Block *parent;
  Function *function;

  BlockTemp *temp;

  Block(Block *_parent, Function *_function): parent(_parent), function(_function), isComplete(true)
  {
    temp = new BlockTemp();
  }

  void Finalise()
  {
    delete temp;
    temp = nullptr;
  }

  void AddStatement(Statement *statement);

  ~Block()
  {

    for(auto it = statements.begin(); it != statements.end(); ++it)
      delete *it;

    if(temp)
      delete temp;
  }

  VariableDecleration *GetVariableDeclerationIfCompleted(const std::string &name);

  // returns false if variable with that name already defined in this block
  bool AddVariableDeclaration(const std::string &name, VariableDecleration *variableDeclaration);


};

class ParameterTemp
{
public:

  std::string typeName;

  ParameterTemp()
  {

  }

};

class Parameter
{
public:

  ParameterTemp *temp;
  INT variableType;
  INT32 memoryIndex;
  bool isComplete;

  Parameter() : isComplete(false)
  {
    memoryIndex = 0;
    temp = new ParameterTemp();
    variableType = TypeIdUnknown;
  }

  void Finalise()
  {
    if(temp)
    {
      delete temp;
      temp = nullptr;
    }
  }

  ~Parameter()
  {
    if(temp)
    {
      delete temp;
      temp = nullptr;
    }
  }
};

class ParameterListTemp
{
public:


  ParameterListTemp()
  {

  }

};

class ParameterList
{
public:

  std::unordered_map<std::string, Parameter*> parameters;
  ParameterListTemp *temp;

  bool isComplete;

  ParameterList() : isComplete(false)
  {
    temp = new ParameterListTemp();
  }

  void Finalise()
  {
    if(temp)
    {
      delete temp;
      temp = nullptr;
    }
  }

  ~ParameterList()
  {
    if(temp)
      delete temp;
    for(auto it = parameters.begin(); it != parameters.end();)
    {
      delete it->second;
      it = parameters.erase(it);
    }
  }

};

class FunctionTemp
{
public:

  bool isComplete;

  FunctionTemp() : isComplete(true)
  {

  }

};

class Function
{
public:

  std::string name;
  INT32 id;
  INT returnTypeId;
  Package *package; // package this function belongs to
  ParameterList *parameterList;
  Block *block;
  FunctionTemp *temp;
  INT32 stackSize;
  INT32 parameterSize;

  Function(Package *_package) : package(_package), id(-1), returnTypeId(0), parameterList(0), block(0), stackSize(0), parameterSize(0)
  {
    temp = new FunctionTemp();
  }

  void Finalise()
  {
    delete temp;
    temp = nullptr;
  }

  virtual ~Function()
  {
    if(temp) delete temp;
    delete block;
    delete parameterList;
  }

};

class GlobalFunction : public Function
{
public:
  GlobalFunction(Package *_package) : Function(_package) { }
};

class Method : public Function
{
public:
  Method(Package *_package) : Function(_package) { }
};

class TypeTempInfo
{
public:

  std::unordered_map<std::string, VariableDecleration*> incompleteVariables;

  std::unordered_set<std::string> interfaces;

  TypeTempInfo()
  {

  }

};

class Type
{
private:

public:

  bool isComplete;
  TypeTempInfo *temp;

  // max type size is int32
  INT32 size;
  INT typeId;

  std::unordered_set<INT> legends;
  std::unordered_set<INT> interfaces;

  std::unordered_map<std::string, VariableDecleration*> variables;
  std::unordered_map<std::string, Method*> methods;

  Type() : isComplete(false)
  {
    size = 0;
    typeId = 0;
    temp = new TypeTempInfo();
  }

  ~Type()
  {
    if(temp)
      delete temp;
    for(auto it = variables.begin(); it != variables.end();)
    {
      delete it->second;
      it = variables.erase(it);
    }

    for(auto it = methods.begin(); it != methods.end();)
    {
      delete it->second;
      it = methods.erase(it);
    }
  }

  void Finalise()
  {
    delete temp;
    temp = nullptr;
  }

};

class PackageTemp
{
public:

  PackageParser *parser;
  std::unordered_map<std::string, Type*> typeNames;
  std::unordered_map<std::string, Type*> incompleteTypeNames;

  std::unordered_map<std::string, GlobalFunction*> incompleteGlobalFunctions;
};

class Package
{
public:

  PackageTemp *temp;

  std::string name;

  INT32 globalFunctionCount;

  std::unordered_map<std::string, INT> globalFunctionNames;
  std::unordered_map<INT, GlobalFunction*> globalFunctions;

  std::unordered_map<INT, Type*> types;

  Package(PackageParser *package_parser) :globalFunctionCount(0) { temp = new PackageTemp(); temp->parser = package_parser; }

  ~Package()
  {
    for(auto it = globalFunctions.begin(); it != globalFunctions.end();)
    {
      delete it->second;
      it = globalFunctions.erase(it);
    }

    for(auto it = types.begin(); it != types.end();)
    {
      delete it->second;
      it = types.erase(it);
    }

    delete temp;
  }

  void Finalise()
  {
    delete temp;
    temp = nullptr;
  }

  void Error(const std::string &msg);

  // TODO: remove magic number 10
  size_t GetNewTypeId() { return types.size() + 10; }

  INT32 GetNewFunctionId() { return globalFunctionCount++; }

  size_t GetTypeId(const std::string &typeName)
  {
    {
      auto it = temp->typeNames.find(typeName);
      if(it != temp->typeNames.end())
        return it->second->typeId;
    }

    {
      auto it = temp->incompleteTypeNames.find(typeName);
      if(it != temp->incompleteTypeNames.end())
        return it->second->typeId;
    }

    if(typeName == "int")
      return TypeIdInteger;
    else if(typeName == "bool")
      return TypeIdBool;

    return TypeIdUnknown;
  }

  // TODO: do this for real
  INT32 GetSizeOf(INT type) 
  {
    switch (type)
    {
    case TypeIdInteger:
      return 4;
    case TypeIdFloat:
      return 4;
    case TypeIdDouble:
      return 8;
    case TypeIdBool:
      return 1;
    default:
      break;
    }

    return types[type]->size; 
  }

  void AddGlobalFunction(const std::string &name, GlobalFunction *function);

};