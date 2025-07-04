#include "VM.h"

#include "Parser/Package.h"
#include "BytecodeGenerator.h"
#include "Bytecode.h"

VM::~VM()
{
  if(bytecode)
    delete bytecode;
}

void VM::AddPackage(Package *package)
{
  if(!packages.count(package->name))
    packages[package->name] = package;
  // TODO: report error
}

void VM::RemovePackage(Package *package)
{
  if(!packages.count(package->name))
    packages.erase(package->name);

  //TODO: report error
}

void VM::GetBytecodeAsString(std::string &str, bool linenumbers)
{
  if(bytecode)
    bytecode->GetByteCode(str, linenumbers);
}

void VM::GenerateByteCode()
{
  if(bytecode) 
    delete bytecode;

  bytecode = new Bytecode();

  BytecodeGenerator generator;
  for(auto &package : packages)
  {
    generator.packages[package.first] = package.second;
  }

  generator.outputFunction = outputFunction;

  for(auto &package : packages)
  {
    for(auto &function : package.second->globalFunctionNames)
    {
      generator.GenerateFunction(bytecode, function.first, package.second->globalFunctions[function.second]);
    }
  }

  bytecode->Finalise();

  if(generator.hasErrors)
  {
    status = VM_Empty;
    delete bytecode;
    bytecode = nullptr;
  }
  else
    status = VM_Available;
}

FunctionBytecode *VM::GetGlobalFunctionBytecode(const std::string &name)
{
  return bytecode->GetFunctionBytecode(name);
}
