#pragma once

#include "Parser/PrimitiveTypes.h"
#include <unordered_map>
#include <string>
#include <functional>

class Function;
class Package;
class Bytecode;
class FunctionBytecode;

class VM
{
private:

  std::unordered_map<std::string, Package*> packages;
  std::function<void(const std::string &msg, INT row, INT column, INT messageLevel)> outputFunction;
  Bytecode *bytecode;

public:

  enum Status
  {
    VM_Empty,
    VM_Available
  }status;

  VM() : bytecode(nullptr) { }

  ~VM();

  Bytecode* GetBytecode() { return bytecode; } 

  void AddPackage(Package *package);

  // removes the package from available packages. Does not delete the package
  void RemovePackage(Package *package);

  void GenerateByteCode();

  void GetBytecodeAsString(std::string &str, bool linenumbers = false);

  FunctionBytecode *GetGlobalFunctionBytecode(const std::string &name);

};