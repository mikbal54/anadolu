#include "PackageParserSemantic.h"
#include "PackageParser.h"
#include "Package.h"
#include "PrimitiveTypes.h"

#include <sstream>
#include <functional>
#include <assert.h>

InvokeStatement *PackageParserSemantic::ParseInvokeStatement( Node *invokeNode, Block *block)
{
  InvokeStatement *invoke = new InvokeStatement(block);

  invoke->expression = ParseExpression(invokeNode->firstChild, invoke);

  if(invoke->expression->isComplete)
    invoke->isComplete = true;

  return invoke;
}

bool PackageParserSemantic::TryToCompleteDesignator(Designator *designator, Block *block)
{
  bool didSomething = false;
  if(!designator->isComplete)
  {
    designator->CalculateAddress(block, this);
    if(designator->isComplete)
      didSomething = true;
  }
  return didSomething;
}

bool PackageParserSemantic::TryToCompleteExpression(Expression *expression)
{
  bool didSomething = false;
  bool allExpressionsComplete = true;

  for( ExpressionValue &value : expression->expressionValues)
  {
    if(value.type == EVT_Designator)
    {
      if(!value.stringValue->isComplete)
      {
        value.stringValue->CalculateAddress(expression->statement->parentBlock, this);
        if(value.stringValue->isComplete)
          didSomething = true;
        else
          allExpressionsComplete = false; // this one isn't complete
      }
    }
  }

  // all of the expression are now completed
  if(allExpressionsComplete)
  {
    expression->isComplete = true;
    expression->returnTypeId = expression->CheckTypeCorrectness(packageParser);
  }
  return didSomething;
}

bool PackageParserSemantic::TryToCompleteStatement(Statement *statement)
{
  bool didSomething = false;

  switch (statement->statementType)
  {
  case ST_IncrementStatement:
    ((IncrementStatement*)(statement))->designator->CalculateAddress(statement->parentBlock, this);
    if( ((IncrementStatement*)(statement))->designator->isComplete)
    {
      didSomething = true;
      ((IncrementStatement*)(statement))->isComplete = true;
    }
    break;
  case ST_DecrementStatement:
    ((DecrementStatement*)(statement))->designator->CalculateAddress(statement->parentBlock, this);
    if( ((DecrementStatement*)(statement))->designator->isComplete)
    {
      didSomething = true;
      ((DecrementStatement*)(statement))->isComplete = true;
    }
    break;
  case ST_IfStatement:
    {
      bool r = TryToCompleteIfStatement((IfStatement*)(statement));
      if(r)
        didSomething = true;
    }
    break;
  case ST_ReturnStatement:
    {
      bool r = TryToCompleteExpression( ((ReturnStatement*)(statement))->expression );
      if(r)
      {
        didSomething = true;
        if(((ReturnStatement*)(statement))->expression->isComplete)
          statement->isComplete = true;
      }
    }
    break;
  case ST_BlockStatement:
    {
      bool r = TryToCompleteBlock( ((BlockStatement*)statement)->block );
      if(r)
      {
        didSomething = true;
        if(((BlockStatement*)statement)->block->isComplete)
          ((BlockStatement*)statement)->isComplete = true;
      }
    }
    break;
  case ST_WhileStatement:
    {
      if( !((WhileStatement* ) statement)->expression->isComplete )
      {
        bool r = TryToCompleteExpression(((WhileStatement*)statement)->expression);
        if(r)
          didSomething = true;
      }

      if( !((WhileStatement* ) statement)->statement->isComplete )
      {
        bool r = TryToCompleteStatement(((WhileStatement*)statement)->statement);
        if(r)
          didSomething = true;
      }

      if(((WhileStatement* ) statement)->statement->isComplete && ((WhileStatement* ) statement)->expression->isComplete)
        ((WhileStatement* ) statement)->isComplete = true;

    }
    break;
  case ST_InvokeStatement:
    if(!((InvokeStatement* ) statement)->isComplete)
    {
      bool r = TryToCompleteExpression( ((InvokeStatement*)statement)->expression);
      if(r)
        didSomething = true;
    }

    if(((InvokeStatement* ) statement)->expression->isComplete)
      ((InvokeStatement* ) statement)->isComplete = true;
    break;
  case ST_Assignment:
    if(!((AssignmentStatement* ) statement)->leftHand->isComplete)
    {
      bool r =  TryToCompleteDesignator( ((AssignmentStatement*)statement)->leftHand, ((AssignmentStatement*)statement)->parentBlock );
      if(r)
        didSomething = true;
    }
    if(!((AssignmentStatement* ) statement)->rightHand->isComplete)
    {
      bool r =  TryToCompleteExpression( ((AssignmentStatement*)statement)->rightHand);
      if(r)
        didSomething = true;
    }

    if(((AssignmentStatement* ) statement)->leftHand->isComplete && ((AssignmentStatement* ) statement)->rightHand->isComplete)
      ((AssignmentStatement* ) statement)->isComplete = true;

    break;
  default:
    assert(0);
    break;
  }



  return didSomething;
}

bool PackageParserSemantic::TryToCompleteIfStatement(IfStatement *ifStatement)
{
  bool didSomething = false;

  if(!ifStatement->expression->isComplete)
  {
    bool r = TryToCompleteExpression(ifStatement->expression);
    if(r)
      didSomething = true;
  }

  if( !ifStatement->statement->isComplete )
  {
    bool r = TryToCompleteStatement(ifStatement->statement);
    if(r)
      didSomething = true;
  }

  if(ifStatement->expression->isComplete && ifStatement->statement->isComplete)
    ifStatement->isComplete = true;

  return didSomething;
}

WhileStatement *PackageParserSemantic::ParseWhileStatement(Node *whileStatementNode, Function *function, Block *block)
{
  Block *childBlock = nullptr;

  WhileStatement *whileStatement = new WhileStatement(block);
  // if it has a block
  if(whileStatementNode->firstChild->next)
  {
    switch (whileStatementNode->firstChild->next->type)
    {
    case BlockNode:
      childBlock = CreateBlockAndVariables(whileStatementNode->firstChild->next, block, function);
      break;
    default:
      whileStatement->statement = ParseStatement(whileStatementNode->firstChild->next, block, function);
      break;
    }
  }

  whileStatement->expression = ParseExpression(whileStatementNode->firstChild, whileStatement);
  if(whileStatement->expression->returnTypeId != TypeIdBool)
  {
    packageParser.ErrorMinor("Only bool expressions accepted in 'while' statements");
  }

  if(childBlock)
  {
    whileStatement->statement = new BlockStatement(block);
    ((BlockStatement*)whileStatement->statement)->block = childBlock;
  }

  if(whileStatement->expression->isComplete && whileStatement->statement->isComplete)
    whileStatement->isComplete = true;

  return whileStatement;
}

IfStatement *PackageParserSemantic::ParseIfStatement(Node *ifStatementNode, Function *function, Block *block)
{
  Block *childBlock = nullptr;

  IfStatement *ifStatement = new IfStatement(block);
  // if it has a block
  if(ifStatementNode->firstChild->next)
  {
    switch (ifStatementNode->firstChild->next->type)
    {
    case BlockNode:
      childBlock = CreateBlockAndVariables(ifStatementNode->firstChild->next, block, function);
      break;
    case IfNode:
      ifStatement->statement = ParseIfStatement(ifStatementNode->firstChild->next, function, block);
      break;
    default:
      ifStatement->statement = ParseStatement(ifStatementNode->firstChild->next, block, function);
      break;
    }
  }

  ifStatement->expression = ParseExpression(ifStatementNode->firstChild, ifStatement);
  if(ifStatement->expression->isComplete)
  {
    if(ifStatement->expression->returnTypeId != TypeIdBool)
      packageParser.ErrorMinor("Only bool expressions accepted in 'if' statements");
  }

  if(childBlock)
  {
    ifStatement->statement = new BlockStatement(block);
    ((BlockStatement*)ifStatement->statement)->block = childBlock;
  }

  if(ifStatement->expression->isComplete && ifStatement->statement->isComplete)
    ifStatement->isComplete = true;
  else
    ifStatement->isComplete = false;

  return ifStatement;
}

IncrementStatement *PackageParserSemantic::ParseIncrementStatement(Node *incrementStatementNode, Block *block)
{
  IncrementStatement *inc = new IncrementStatement(block);
  Designator *desig = ParseDesignator(incrementStatementNode, inc);

  // attempt to calculate its address, but types may not be known just yet
  desig->CalculateAddress(block, this);

  inc->designator = desig;

  inc->isComplete = desig->isComplete;
  return inc;
}

DecrementStatement *PackageParserSemantic::ParseDecrementStatement(Node *decrementStatementNode, Block *block)
{

  DecrementStatement *dec = new DecrementStatement(block);
  Designator *desig = ParseDesignator(decrementStatementNode, dec);

  // attempt to calculate its address, but types may not be known just yet
  desig->CalculateAddress(block, this);

  dec->designator = desig;
  dec->isComplete = desig->isComplete;
  return dec;
}

AssignmentStatement *PackageParserSemantic::ParseAssignmentStatement(Node *assignmentNode, Block *block)
{
  AssignmentStatement *assignment = new AssignmentStatement(block);

  assignment->leftHand = ParseDesignator(assignmentNode->firstChild, assignment);
  assignment->leftHand->CalculateAddress(block, this);

  if(!assignment->leftHand->isComplete)
    assignment->isComplete = false;

  assignment->rightHand = ParseExpression(assignmentNode->firstChild->next, assignment); 
  if(!assignment->rightHand->isComplete)
    assignment->isComplete = false;

  if(assignment->rightHand->isComplete && assignment->leftHand->isComplete)
  {
    assignment->isComplete = true;
    assignment->rightHand->Finalise();
  }

  return assignment;
}

Designator *PackageParserSemantic::ParseDesignator(Node *designatorNode, Statement *statement)
{
  auto ParseFunctionCall = [&](Designator *funcCall)
  {
    funcCall->type = DT_FunctionCall;

    Node *exprNode = designatorNode->firstChild->firstChild;
    if(exprNode && !funcCall->expressions)
      funcCall->expressions = new std::vector<Expression*>();

    while(exprNode)
    {
      Expression *expr = ParseExpression(exprNode, statement);
      funcCall->expressions->push_back(expr);
      exprNode = exprNode->next;
    }

  };


  // we left here
  Designator *desg = new Designator();

  Node *child = designatorNode->lastChild;

  desg->name = packageParser.GetTokenAsString(child->startToken);
  if(child->type == FunctionCallNode)
  {
    ParseFunctionCall(desg);
  }

  Designator *lastParent = desg; 
  child = child->prev;
  while(child)
  {

    switch (child->type)
    {
    case IdentifierNode:
      lastParent->parent = new Designator();
      lastParent->parent->name = packageParser.GetTokenAsString(child->startToken);
      break;
    case FunctionCallNode:
      lastParent->parent = new Designator();
      ParseFunctionCall(desg);
      lastParent->parent->name = packageParser.GetTokenAsString(child->startToken);
      break;
    default:
      assert(0);
      break;
    }

    lastParent = lastParent->parent;
    child = child->prev;
  }


  return desg;
}

Method* PackageParserSemantic::ParseMethod(Node *methodNode)
{
  return nullptr;
}

Type *PackageParserSemantic::ParseTypeDefinition(Node *typeDefinitionNode)
{
  Type *type = new Type();
  type->typeId = packageParser.package->GetNewTypeId();

  // start from legendsNode
  Node *child  = typeDefinitionNode->firstChild->next;

  while(child)
  {
    switch (child->type)
    {
    case TypeLegendsNode:
      {
        Node *legendChild = child->firstChild;
        while(legendChild)
        {
          type->temp->interfaces.insert(packageParser.GetTokenAsString(legendChild->startToken));
          legendChild = legendChild->next;
        }
      }
      break;
    case VariableDeclarationNode:
      {
        VariableDecleration *vdecl = ParseVariableDecleration(child);

        std::string name = packageParser.GetTokenAsString(child->startToken + 1);

        if(!vdecl->isComplete)
        {
          type->isComplete = false;
          type->temp->incompleteVariables[name] = vdecl;
        }
        else
        {
          vdecl->isComplete = true;
          type->variables[name] = vdecl;
          vdecl->position = type->size;
          type->size += packageParser.package->GetSizeOf(vdecl->variableType);
        }

      }
      break;
    case FunctionDeclarationNode:
      {
        Method *method = ParseMethod(child);
        type->methods[packageParser.GetTokenAsString(child->startToken + 1)] = method;
      }
      break;
    default:
      break;
    }

    child = child->next;
  }

  if(type->temp->incompleteVariables.empty())
  {
    type->isComplete = true;
    type->Finalise();
  }

  return type;
}

Block *PackageParserSemantic::CreateBlockAndVariables(Node *blockNode, Block *parentBlock, Function *function)
{
  Block *newBlock = new Block(parentBlock, function);

  auto CreateVariableDecleration = [](Node *child, PackageParserSemantic *parser, Block *block, Function *function)
  {
    VariableDecleration *variableDecleration = parser->ParseVariableDecleration(child);
    VariableDeclerationStatement *declStatement = new VariableDeclerationStatement(block, variableDecleration);

    block->statements.push_back(declStatement);

    std::string name = parser->packageParser.GetTokenAsString(child->startToken+1);
    if( block->GetVariableDeclerationIfCompleted(name) )
    {
      parser->packageParser.ErrorMinor("Variable with name:'" +  name + "' already defined");
      delete variableDecleration;
      return;
    }

    // TODO: what if its not completed, but still causes a name clash ??

    // TODO: also parse variable assignment expression

    if(variableDecleration->isComplete)
    {
      // Assign memory location indices and sizes to every local variable
      // Also accumuates total need stack space for this function
      variableDecleration->position = function->stackSize;
      function->stackSize += parser->packageParser.package->GetSizeOf(variableDecleration->variableType);

      block->temp->variableDeclerations[name] = block->statements.size() - 1;
    }
    else
    {
      block->temp->unfinishedDeclerationStatements[name] = block->statements.size() - 1;
      block->isComplete = false;
    }

  };

  Node *child = blockNode->firstChild;
  while(child != nullptr)
  {
    switch(child->type)
    {
    case VariableDeclarationNode:
      {
        CreateVariableDecleration(child, this, newBlock, function);
      }
      break;
    case BlockNode:
      {
        Block *childBlock = CreateBlockAndVariables(child, newBlock, function);
        BlockStatement *blockStatement = new BlockStatement(newBlock);
        blockStatement->block = childBlock;

        if(childBlock->isComplete)
          blockStatement->isComplete = true;
        newBlock->AddStatement(blockStatement);
      }
      break;
    case StatementNode:
      assert(false);
      break;
    case AssignmentNode:
      {
        AssignmentStatement *asgn = ParseAssignmentStatement(child, newBlock);

        newBlock->AddStatement(asgn);

      }
      break;
    case ReturnStatementNode:
      {
        Statement *stmt = ParseStatement(child, newBlock, function);
        newBlock->AddStatement(stmt);
      }
      break;
    case IncrementStatementNode:
      {
        IncrementStatement *inc = ParseIncrementStatement( child->firstChild, newBlock);
        newBlock->AddStatement(inc);
      }
      break;
    case DecrementStatementNode:
      {
        DecrementStatement *dec = ParseDecrementStatement( child->firstChild, newBlock);
        newBlock->AddStatement(dec);
      }
      break;
    case InvokeStatementNode:
      {
        // invoke statement is a solo expression. no assigment or anything
        InvokeStatement *invoke = ParseInvokeStatement( child, newBlock);
        newBlock->AddStatement(invoke);
      }
      break;
    case IfNode:
      {
        IfStatement *ifStatement = ParseIfStatement(child, function, newBlock);
        newBlock->AddStatement(ifStatement);
      }
      break;
    case WhileNode:
      {
        WhileStatement *whileStatement = ParseWhileStatement(child, function, newBlock);
        newBlock->AddStatement(whileStatement);
      }
      break;
    default:
      assert(false);
      break;
    }
    child = child->next;
  }


  // dont add block end opcode is this is a functions main block
  if(parentBlock)
  {
    Statement *blockEnd = new Statement(newBlock);
    blockEnd->statementType = ST_BlockEndStatement;
    newBlock->statements.push_back(blockEnd);
  }


  // Check if all block is complete
  if(!newBlock->isComplete)
    function->temp->isComplete = false;

  return newBlock;
}

VariableDecleration *PackageParserSemantic::ParseVariableDecleration(Node *variableDeclarationNode)
{
  VariableDecleration *variableDecleration = new VariableDecleration();


  if(variableDeclarationNode->firstChild->next)
    variableDecleration->temp->typeName = packageParser.GetTokenAsString(variableDeclarationNode->firstChild->next->startToken);
  else
    variableDecleration->variableType = TypeIdGeneric; 
  // TODO: check assignment, if no assignment generate error

  if(variableDecleration->variableType == TypeIdUnknown)
  {
    variableDecleration->variableType = packageParser.package->GetTypeId(variableDecleration->temp->typeName);

    if(variableDecleration->variableType != TypeIdUnknown)
      variableDecleration->isComplete = true;
  }

  return variableDecleration;
}

ReturnStatement* PackageParserSemantic::ParseReturnStatement(Node *returnStatementNode, Block *block)
{
  ReturnStatement *returnStatement = new ReturnStatement(block);

  if(returnStatementNode->firstChild)
    returnStatement->expression = ParseExpression(returnStatementNode->firstChild, returnStatement);

  if(returnStatement->expression->isComplete)
    returnStatement->isComplete = true;

  return returnStatement;
}

Expression *PackageParserSemantic::ParseExpression(Node *expressionNode, Statement *statement)
{
  Expression *expression = new Expression(statement);
  // TODO: fill in the expression

  INT lastOperationType = 0;

  Node *child = expressionNode->firstChild;

  while(child)
  {
    switch (child->type)
    {
      // two argument operators
    case PlusNode:
    case MinusNode:
    case EqualsNode:
    case NotEqualNode:
    case NegateNode:
    case MultiplyNode:
    case DivideNode:

      if(child->type == PlusNode)
        expression->expressionValues.emplace_back(EVT_PlusOperator);
      else if(child->type == EqualsNode)
        expression->expressionValues.emplace_back(EVT_EqualsOperator);
      else if(child->type == MinusNode)
        expression->expressionValues.emplace_back(EVT_MinusOperator);
      else if(child->type == NegateNode)
        expression->expressionValues.emplace_back(EVT_NegateOperator);
      else if(child->type == MultiplyNode)
        expression->expressionValues.emplace_back(EVT_MultiplyOperator);
      else if(child->type == DivideNode)
        expression->expressionValues.emplace_back(EVT_DivideOperator);
      else if(child->type == NotEqualNode)
        expression->expressionValues.emplace_back(EVT_NotEqualOperator);
      else 
        assert(0);

      break;
    case IdentifierNode:
      assert(0); // identifier has no place in an expression, this should have been a designator
      break;
    case ConstIntNode:
      expression->expressionValues.emplace_back(packageParser.GetTokenIntValue(child->startToken) );
      break;
    case ConstBoolNode:
      if(packageParser.GetTokenAsString(child->startToken) == "true")
        expression->expressionValues.emplace_back(true);
      else
        expression->expressionValues.emplace_back(false);
      break;
    case DesignatorNode:

      //TODO: handle function call
      expression->expressionValues.emplace_back(child, statement, this);
      expression->expressionValues.back().stringValue->CalculateAddress(statement->parentBlock, this);
      if(!expression->expressionValues.back().stringValue->isComplete)
        expression->isComplete = false;

      break;
    default:
      assert(0); // unknown node type in expression
      break;
    }


    child = child->next;
  }

  if(expression->isComplete)
    expression->returnTypeId = expression->CheckTypeCorrectness(packageParser);


  return expression;
}

Statement *PackageParserSemantic::ParseStatement(Node *statementNode, Block *block, Function *function)
{

  Statement *statement = 0;
  Node *child = statementNode->firstChild;

  switch (statementNode->type)
  {
  case StatementNode:
    while(child)
    {
      InvokeStatement *invokeStatement = new InvokeStatement(block);
      statement = static_cast<Statement*>(invokeStatement);

      invokeStatement->expression = ParseExpression(child, invokeStatement);

      child = child->next;
    }
    break;
  case BlockNode:
    {
      BlockStatement *blockStatement = new BlockStatement(block);
      blockStatement->block = new Block(block, function);
      // ParseBlock(blockStatement->block);
    }
    break;
  case IncrementStatementNode:
    statement = ParseIncrementStatement(child, block);
    break;
  case DecrementStatementNode:
    statement = ParseDecrementStatement(child, block);
    break;
  case ReturnStatementNode:
    {
      ReturnStatement *retStatement = ParseReturnStatement(statementNode, block);
      statement = retStatement;

      if(retStatement->isComplete)
      {
        if(retStatement->expression)
        {
          if(function->returnTypeId != 0)
          {
            // TODO: try conversion
            if(retStatement->expression->returnTypeId != function->returnTypeId)
              packageParser.ErrorMinor("Incompatible multiple return types");
          }
          else
            function->returnTypeId = retStatement->expression->returnTypeId;
        }
        else
        {
          function->returnTypeId = TypeIdVoid;
        }
      }
    }
    break;
  default:
    assert(0);
    break;
  }


  return statement;
}

Parameter *PackageParserSemantic::ParseParameter(Node *parameterNode, Function *function)
{
  Parameter *parameter = new Parameter();

  parameter->temp->typeName = packageParser.GetTokenAsString(parameterNode->endToken);

  INT tid = packageParser.package->GetTypeId(parameter->temp->typeName);
  if(tid != TypeIdUnknown)
  {
    parameter->isComplete = true;
    parameter->variableType = tid;
    parameter->memoryIndex = function->parameterSize;
    function->parameterSize += packageParser.package->GetSizeOf(tid);
    parameter->Finalise();
  }
  else
    parameter->isComplete = false;

  return parameter;
}

ParameterList* PackageParserSemantic::ParseParameterList(Node *paramaterListNode, Function *function)
{
  ParameterList *parameterList = new ParameterList();

  bool allParametersKnown = true;

  Node *parameterNode = paramaterListNode->firstChild;
  while(parameterNode)
  {
    Parameter *param = ParseParameter(parameterNode, function);
    parameterList->parameters[packageParser.GetTokenAsString(parameterNode->startToken)] = param;

    if(!param->isComplete)
      allParametersKnown = false;

    parameterNode =  parameterNode->next;
  }

  if(allParametersKnown)
    parameterList->isComplete = true;

  return parameterList;
}

GlobalFunction* PackageParserSemantic::ParseGlobalFunction(Node *functionNode)
{
  GlobalFunction *function = new GlobalFunction(packageParser.package);
  function->id = packageParser.package->GetNewFunctionId();
  function->name = packageParser.GetTokenAsString(functionNode->firstChild->startToken);

  function->parameterList = ParseParameterList(functionNode->firstChild->next, function);

  function->block = CreateBlockAndVariables(functionNode->firstChild->next->next, nullptr, function);

  // ParseBlock(function->block);

  if(function->block->isComplete)
  {
    function->block->Finalise();
  }
  else
    function->temp->isComplete = false;

  return function;
}

bool PackageParserSemantic::TryToCompleteVariableDecleration(VariableDecleration *vdecl, Block *block)
{
  bool didSomething = false;

  if(vdecl->variableType == TypeIdUnknown)
  {

    auto it = packageParser.package->temp->typeNames.find(vdecl->temp->typeName);
    if(it != packageParser.package->temp->typeNames.end())
    {
      vdecl->variableType = it->second->typeId;
      didSomething = true;
      vdecl->isComplete = true;      
    }

  }
  else if( vdecl->variableType == TypeIdGeneric )
  {
    // TODO: depends on expression type
    // check if expression is parsed
  }

  return didSomething;
}

bool PackageParserSemantic::TryToCompleteBlock(Block *block)
{
  bool didSomething = false;

  for( auto it = block->temp->unfinishedDeclerationStatements.begin(); it != block->temp->unfinishedDeclerationStatements.end(); )
  {
    VariableDeclerationStatement *stmt = (VariableDeclerationStatement*) block->statements[it->second];

    bool r = TryToCompleteVariableDecleration( stmt->variableDecleration, block );
    if(r)
    {
      didSomething = true;
      if(stmt->variableDecleration->isComplete)
      {
        // give a position in stack to this variable
        stmt->variableDecleration->position = block->function->stackSize;
        block->function->stackSize += packageParser.package->GetSizeOf(stmt->variableDecleration->variableType);

        stmt->variableDecleration->Finalise();
        block->temp->variableDeclerations[it->first] = it->second;
        it = block->temp->unfinishedDeclerationStatements.erase(it);
      }
      else
        ++it;
    }
    else
      ++it;
  }

  for( auto it = block->temp->unfinishedStatements.begin(); it != block->temp->unfinishedStatements.end(); )
  {
    bool r = TryToCompleteStatement(it->second);
    if(r)
    {
      didSomething = true;
      if(it->second->isComplete)
        it = block->temp->unfinishedStatements.erase(it);
      else
        ++it;
    }
    else
      ++it;
  }


  if(didSomething)
  {
    if(block->temp->unfinishedDeclerationStatements.empty())
      if(block->temp->unfinishedStatements.empty())
        block->isComplete = true;
  }

  return didSomething;
}

bool PackageParserSemantic::TryToCompleteGlobalFunction(GlobalFunction *function)
{
  // this pass actually did something?
  bool didSomething = false;

  if(!function->parameterList->isComplete)
  {
    // is parameter list is not complete don't even bother with the block

    bool allParametersDone = true;

    for(auto &it : function->parameterList->parameters)
    {
      if(!it.second->isComplete)
      {
        INT tid = packageParser.package->GetTypeId(it.second->temp->typeName);
        if(tid != TypeIdUnknown)
        {
          it.second->isComplete = true;
          it.second->variableType = tid;
          it.second->memoryIndex = function->parameterSize;
          function->parameterSize += packageParser.package->GetSizeOf(tid);
          it.second->Finalise();
          didSomething = true;
        }
        else
          allParametersDone = false;
      }
    }

    if(allParametersDone)
      function->parameterList->isComplete = true;
  }

  if(!function->block->isComplete)
  {
    bool r = TryToCompleteBlock(function->block);
    if(r)
      didSomething = true;
  }

  if(function->block->isComplete && function->parameterList->isComplete)
    function->temp->isComplete = true;

  return didSomething;
}

void PackageParserSemantic::Parse()
{
  Node *mainNode = packageParser.GetMainNode();

  Node *child = mainNode->firstChild;

  // first pass
  while(child != nullptr)
  {
    switch(child->type)
    {
    case FunctionDeclarationNode:
      {
        GlobalFunction *function = ParseGlobalFunction(child);

        if(function->temp->isComplete)
        {
          packageParser.package->AddGlobalFunction(packageParser.GetTokenAsString(child->firstChild->startToken), function);
          function->Finalise();
        } 
        else
          packageParser.package->temp->incompleteGlobalFunctions[ packageParser.GetTokenAsString(child->firstChild->startToken) ] = function;
        // TODO: keep templated function seperately
        // TODO: error if function already exists
      }
      break;
    case TypeDefinitionNode:
      {
        std::string name = packageParser.GetTokenAsString(child->firstChild->startToken);
        if(packageParser.package->temp->typeNames.count(name))
        {
          packageParser.ErrorMinor("Type with name: " + name + " already exists!");    
        }
        else if(packageParser.package->temp->incompleteTypeNames.count(name))
        {
          packageParser.ErrorMinor("Type with name: " + name + " already exists!");    
        }
        else
        {
          INT id = packageParser.package->GetNewTypeId();
          Type *type = ParseTypeDefinition(child);
          type->typeId = id;
          if(type->isComplete)
          {
            packageParser.package->temp->typeNames[name] = type;
            packageParser.package->types[id] = type;
            type->Finalise();
          }
          else
          {
            packageParser.package->temp->incompleteTypeNames[name] = type;
          }

        }
      }
      break;
    }

    child = child->next;
  }

  Package *package = packageParser.package;

  while(true)
  {
    if(package->temp->incompleteTypeNames.empty())
      if(package->temp->incompleteGlobalFunctions.empty())
        break; // break from while loop

    bool didSomething = false;
    for( auto it = package->temp->incompleteTypeNames.begin(); it != package->temp->incompleteTypeNames.end();  )
    {

      ++it;
    }

    for( auto it =  package->temp->incompleteGlobalFunctions.begin(); it != package->temp->incompleteGlobalFunctions.end(); )
    {
      bool r = TryToCompleteGlobalFunction(it->second);
      if(r)
        didSomething = true;

      if(it->second->temp->isComplete)
      {
        package->AddGlobalFunction(it->first, it->second);
        it->second->Finalise();
        it = package->temp->incompleteGlobalFunctions.erase(it);
      }
      else
        ++it;
    }


    if(!didSomething)
    {
      // TODO: iterate all types and functions, list errors
      packageParser.ErrorMinor("Error");
      break;
    }
  }



}
