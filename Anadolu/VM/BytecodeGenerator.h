#pragma once


#include "Parser/PrimitiveTypes.h"

#include <vector>
#include <list>
#include <set>
#include <unordered_map>
#include <string>

#include <functional>
#include <assert.h>

class Block;
class Node;
class Package;
class Bytecode;
class Function;
class PackageParser;
class Instruction;
class Statement;
class Expression;
class ExpressionValue;
class Designator;

class BytecodeGenerator
{
public:

  // currently unused registers
  // used set to always get lowest register number from the front
  std::set<INT32> availableRegisters;
  std::function<void(const std::string &msg, INT row, INT column, INT messageLevel)> outputFunction;
  std::unordered_map<std::string, Package*> packages;
  

  Bytecode *bytecode;

  // used to find how many registers are need for a function
  INT32 maxRegisterNumber;

  bool hasErrors;

  BytecodeGenerator();

  void Error(const std::string &msg);

  void ReleaseAllRegisters()
  {
    availableRegisters.clear();

    // TODO: remove magic number, make available registers extendable
    for(INT32 i = 0; i < 256; ++i)
      availableRegisters.insert(i);
  }

  INT32 GetAvailableRegister()
  {
    INT32 r =  *availableRegisters.begin();

    if(r > maxRegisterNumber)
      maxRegisterNumber = r;

    availableRegisters.erase(availableRegisters.begin());
    return r;
  }

  // this version does not place ResetR instruction
  void DoneWithTheRegister(INT32 registerNumber);
  void DoneWithTheRegister(std::list<Instruction> &instruction, INT32 registerNumber);

  void GenerateBytecode(std::list<Instruction> &instructions, Function *function, Statement *statement);

  void DivideOperator(std::list<Instruction> &instructions, std::list<ExpressionValue> &values);

  void MultiplyOperator(std::list<Instruction> &instructions, std::list<ExpressionValue> &values);

  void NegateOperator(std::list<Instruction> &instructions, std::list<ExpressionValue> &values);

  void MinusOperator(std::list<Instruction> &instructions, std::list<ExpressionValue> &values);

  void NotEqualOperator(std::list<Instruction> &instructions, std::list<ExpressionValue> &values);

  void EqualsOperator(std::list<Instruction> &instructions, std::list<ExpressionValue> &values);

  void PlusOperator(std::list<Instruction> &instructions, std::list<ExpressionValue> &values);

  INT32 GenerateExpression(std::list<Instruction> &instructions, Expression *expression, Function *function);

  void GenerateFunctionCall(std::list<Instruction> &instructions, INT32 returnRegister, Designator *expression, Function *function);

  void GenerateFunction(Bytecode *bytecode, const std::string &name,  Function *function);


};