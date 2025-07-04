#pragma once

#include "Instruction.h"

#include <vector>
#include <unordered_map>
#include <string>

class Bytecode;
class FunctionBytecode;

class FunctionBytecode
{
public:

  std::list<Instruction> instructions;
  std::vector<Instruction> optimizedInstructions;

  // positions jump instructions jump to
  std::list<std::list<Instruction>::iterator> jumpLocations;
  std::list<std::list<Instruction>::iterator> jumpInstructionPositions;

  void CompactInstructions()
  {
    optimizedInstructions.reserve(instructions.size() + 1);

    for(auto it = instructions.begin(); it != instructions.end();)
    {
      optimizedInstructions.emplace_back(*it);
      it = instructions.erase(it);
    }

  }

};

class BytecodeTemp
{
public:

};

class Bytecode
{
public:

  BytecodeTemp *temp;

  std::unordered_map<std::string, INT> globalFunctionNames;
  std::unordered_map<INT, FunctionBytecode*> functionBytecodes;

  Bytecode() : temp(new BytecodeTemp()) { }

  ~Bytecode()
  {
    for(auto it = functionBytecodes.begin(); it != functionBytecodes.end(); )
    {
      delete it->second;
      it = functionBytecodes.erase(it);
    }
  }

  void Finalise()
  {
    delete temp;
  }

  void Bytecode::GetByteCode(std::string &str, bool lineNumbers = true);

  FunctionBytecode *GetFunctionBytecode(const std::string &name);
  FunctionBytecode *GetFunctionBytecodeIndex(size_t id);

};
