#include "Package.h"
#include "PackageParser.h"
#include "PackageParserSemantic.h"

#include <sstream>

Designator::~Designator() 
{
  if(expressions)
  {
    for(Expression *expression : *expressions)
      delete expression;
    delete expressions;
  }
  delete parent;
}

void Package::AddGlobalFunction(const std::string &name, GlobalFunction *function)
{
  // TODO: handle diffent overloaded functions
  globalFunctionNames[name] = function->id;
  globalFunctions[function->id] = function;
}

VariableDeclerationStatement::~VariableDeclerationStatement()
{
  delete variableDecleration;
}

void Block::AddStatement(Statement *statement)
{
  statements.push_back(statement);
  if(!statement->isComplete)
    temp->unfinishedStatements[statements.size() - 1] = statement;

  if(!statement->isComplete)
    isComplete = false;
}

BlockStatement::~BlockStatement()
{
  delete block;
}

INT Expression::CheckTypeCorrectness(PackageParser &packageParser)
{
  std::list<INT> typeIds;

  INT lastOperationType = TypeIdUnknown;
  for(size_t i = 0; i < expressionValues.size(); ++i)
  {
    switch (expressionValues[i].type)
    {
    case EVT_Designator:
      typeIds.push_back(expressionValues[i].stringValue->typeId);
      lastOperationType = expressionValues[i].stringValue->typeId;
      break;
    case EVT_ConstBool:
      typeIds.push_back(TypeIdBool);
      lastOperationType = TypeIdBool;
      break;
    case EVT_ConstInt:
      typeIds.push_back(TypeIdInteger);
      lastOperationType = TypeIdInteger;
      break;
    case EVT_EqualsOperator:
    case EVT_NotEqualOperator:
      {
        INT rightId = typeIds.back(); typeIds.pop_back(); 
        INT leftId = typeIds.back(); typeIds.pop_back();
        // TODO: check if conversion function exists
        if(rightId != leftId)
        {
          //TODO: improve error message
          packageParser.ErrorMinor("Type mismatch, could not compare X and Y");
        }
        lastOperationType = TypeIdBool;
        typeIds.push_back(TypeIdBool);
      }
      break;
    case EVT_PlusOperator:
      {
        INT rightId = typeIds.back(); typeIds.pop_back(); 
        INT leftId = typeIds.back(); typeIds.pop_back();

        if(leftId != TypeIdInteger)
          packageParser.ErrorMinor("Add operator only accepts integers");

        if(rightId != leftId)
        {
          //TODO: improve error message
          packageParser.ErrorMinor("Type mismatch, could not add X and Y");
        }
        lastOperationType = TypeIdInteger;
        typeIds.push_back(TypeIdInteger);
      }
      break;
    case EVT_MinusOperator:
      {
        INT rightId = typeIds.back(); typeIds.pop_back(); 
        INT leftId = typeIds.back(); typeIds.pop_back();

        if(leftId != TypeIdInteger)
          packageParser.ErrorMinor("Subtract operator only accepts integers");

        if(rightId != leftId)
        {
          //TODO: improve error message
          packageParser.ErrorMinor("Type mismatch, could not subtract X from Y");
        }
        lastOperationType = TypeIdInteger;
        typeIds.push_back(TypeIdInteger);
      }
      break;
    case EVT_MultiplyOperator:
      {
        INT rightId = typeIds.back(); typeIds.pop_back(); 
        INT leftId = typeIds.back(); typeIds.pop_back();

        if(leftId != TypeIdInteger)
          packageParser.ErrorMinor("Multiply operator only accepts integers");

        if(rightId != leftId)
        {
          //TODO: improve error message
          packageParser.ErrorMinor("Type mismatch, could not multiply X with Y");
        }
        lastOperationType = TypeIdInteger;
        typeIds.push_back(TypeIdInteger);
      }
      break;
    case EVT_NegateOperator:
      {
        INT leftId = typeIds.back(); typeIds.pop_back();
        if(leftId == TypeIdInteger) // TODO: handle other types
        {
          lastOperationType = TypeIdInteger;
          typeIds.push_back(TypeIdInteger);
        }
        else
        {
          //TODO: improve error message
          packageParser.ErrorMinor("Unary minus operator not allowed on this type");
        }
      }
      break;
    case EVT_DivideOperator:
      {

        INT rightId = typeIds.back(); typeIds.pop_back(); 
        INT leftId = typeIds.back(); typeIds.pop_back();

        if(leftId != TypeIdInteger)
          packageParser.ErrorMinor("Divide operator only accepts integers");

        if(rightId != leftId)
        {
          //TODO: improve error message
          packageParser.ErrorMinor("Type mismatch, could not divide X by Y");
        }
        lastOperationType = leftId;
        typeIds.push_back(leftId);
      }
      break;
    default:
      assert(0); // unhandled operator type
      break;
    }

  }

  return lastOperationType;
}

ExpressionValue::ExpressionValue(Node * designatorNode, Statement *statement, PackageParserSemantic *parser)
{
  type = EVT_Designator;
  stringValue = parser->ParseDesignator(designatorNode, statement);
}

void Package::Error(const std::string &msg) 
{
  temp->parser->ErrorCritical(msg);
}

Parameter *Designator::FindVariableInParameters(Function *function, Package *package)
{

  auto it = function->parameterList->parameters.find(name);
  // found it in parameters
  // maybe complete 
  if(it != function->parameterList->parameters.end())
  {
    // not complete? don't bother, we will return later
    if(!it->second->isComplete)
      return it->second; // we found the parameter in but its not complete.

    return it->second;
  }

  return nullptr;
}

bool Designator::FindFunctionCall(Block *block, Package *package)
{
  bool result = false;

  // TODO: name is not enough, we should have thing like GlobalFunctionIdentifier which can hold param ids too
  if(function == nullptr && name == block->function->name)
    this->function = block->function;


  if(function == nullptr)
  {
    // only one identifier means this is a global function or a method of 'this'
    auto &it = package->globalFunctionNames.find(name);

    // global function may not be known at this moment
    if(it == package->globalFunctionNames.end())
      return false;

    function = package->globalFunctions[it->second];
  }

  if(function->returnTypeId != TypeIdUnknown)
  {
    typeId = function->returnTypeId;
    result = true;
  }
  //TODO: handle 'this' method call

  return result;
}

void Designator::CalculateAddress(Block *block, PackageParserSemantic *parser)
{

  if(parent)
  {
    if(!parent->isComplete)
    {
      parent->CalculateAddress(block, parser);
      if(!parent->isComplete)
        return;
    }
    else
      return;
  }

  // has a parent and its complete
  if(parent)
  {

    if(parent->type == DT_LocalValue)
    {
      // first check if has child with correct name name

      Type *parentType = parser->packageParser.package->types[parent->typeId];

      auto result = parentType->variables.find(name);
      if(result == parentType->variables.end())
      {
        parser->packageParser.package->Error("Variable does not have a child named: " + name );
        isComplete = true; // do this so it won't try to compile this again
        return;
      }

      address = parent->address + result->second->position;
      typeId = result->second->variableType;
      type = DT_LocalValue;
      isComplete = true;
      return;
    }
    else if(parent->type == DT_ParameterValue)
    {
      Type *parentType = parser->packageParser.package->types[parent->typeId];
      auto result = parentType->variables.find(name);
      if(result == parentType->variables.end())
      {
        parser->packageParser.package->Error("Variable does not have a child named: " + name );
        isComplete = true; // do this so it won't try to compile this again
        return;
      }

      address = parent->address + result->second->position;
      isComplete = true;
      typeId = result->second->variableType;
      type = DT_ParameterValue;
      return;
    }
    else
      assert(0); // TODO: handle heap and function call parents

  }
  else // it doesn't have a parent. this means a single identifier
  {
    // if its a function try to find it
    if(type == DT_FunctionCall)
    {
      bool r = FindFunctionCall(block, parser->packageParser.package);
      if(r)
      {
        bool allExpressionsComplete = true;
        if(expressions)
        {
          for(Expression *expr : *expressions)
          {
            if(!expr->isComplete)
              parser->TryToCompleteExpression(expr);
            if(!expr->isComplete)
              allExpressionsComplete = false;
          }
        }

        if(allExpressionsComplete)
          isComplete = true;
      }
      return;
    }

    // if its not a function
    // first try parameters
    Parameter *param = FindVariableInParameters(block->function, parser->packageParser.package);
    if(param)
    {
      type = DT_ParameterValue;

      if(!param->isComplete)
        return;

      address = param->memoryIndex;
      typeId = param->variableType;
      isComplete = true;
      return;
    }

    // could not find in params 
    // try locals
    VariableDecleration *vdecl = block->GetVariableDeclerationIfCompleted(name);
    if(vdecl)
    {
      type = DT_LocalValue;
      if(vdecl->isComplete)
      {
        address = vdecl->position;
        typeId = vdecl->variableType;
        isComplete = true;
        return;
      }
    }

  }


}

VariableDecleration *Block::GetVariableDeclerationIfCompleted(const std::string &name)
{

  auto it = temp->variableDeclerations.find(name);
  if(it == temp->variableDeclerations.end())
  {
    if(!parent)
      return nullptr;
    return parent->GetVariableDeclerationIfCompleted(name);
  }
  return ((VariableDeclerationStatement*) statements[it->second])->variableDecleration;
}
