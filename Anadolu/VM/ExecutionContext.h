#pragma once

#include "Parser/PrimitiveTypes.h"

#include <vector>


class FunctionBytecode;
class Function;
class Expression;
class Statement;
class VM;
class Instruction;
class Bytecode;

// A single function execution
class ExecutionContext
{
private:

  // TODO: do coroutines
  enum ExecutionStatus
  {
    NotPrepared,
    Prepared,
    Executing,
    Returned
  }executionStatus;

  Bytecode *bytecode;
  FunctionBytecode *functionBytecode;
  std::vector<Instruction> *instructions;

  // parameters, this is only a pointer.
  // Parameter data is created by the caller, but deleted by this function
  char *params;
  char *locals;

  // registers array.
  // each register holds enough to hold 
  INT *registers; 
  char *returnValue;
  char *thisValue;

  void ExecuteInstructions();

public:

  ExecutionContext(Bytecode *_bytecode, FunctionBytecode *_functionBytecode) ;

  ~ExecutionContext();

  void CreateReturnMemory();
  inline void DestroyReturnMemory() { delete[] returnValue; }

  inline char *GetReturnValue() { return returnValue; }

  void Execute();

  void SetParameter(char *data);

};