#include "Parser/PackageParser.h"
#include "Parser/Package.h"
#include "Parser/PrimitiveTypes.h"
#include "VM/VM.h"
#include "VM/Bytecode.h"
#include "VM/BytecodeGenerator.h"
#include "VM/ExecutionContext.h"

#include <iostream>
#include <fstream>
#include <sstream>
#include <chrono>

using namespace std;

void MessageOut(const std::string &msg, INT row, INT column, INT messageLevel)
{
  if(messageLevel = MESSAGE_ERROR)
    std::cout << "Error: ";
  else
    std::cout << "Warning: ";

  std::cout << row << ":" << column << " " << msg << "\n";
}

void LoadFile(const std::string &fileName, std::string &content )
{
  std::ifstream file(fileName);

  file.seekg(0, std::ios::end);   
  content.reserve( (INT32)file.tellg());
  file.seekg(0, std::ios::beg);

  content.assign((std::istreambuf_iterator<char>(file)),
    std::istreambuf_iterator<char>());

}

// runs test file, returns result as integer
INT RunTestFile(const std::string &file,  INT numOfBytesParameters, bool printInstructions = false)
{
  INT returnValue = -1;
  string input;
  LoadFile(file, input);
  PackageInfo packageInfo;
  packageInfo.name = "First";
  packageInfo.AddScriptSection(input);

  PackageParser parser(packageInfo);
  parser.outputFunction = MessageOut;
  Package *package = parser.Parse();

  if(package)
  {
    VM vm;
    vm.AddPackage(package);
    vm.GenerateByteCode();

    if(vm.status == VM::VM_Available)
    {

      if(printInstructions)
      {
        std::string out;
        vm.GetBytecodeAsString(out, true);
        std::cout << "\n " << file << "\nInstructions:\n" << out;
      }

      auto start = std::chrono::steady_clock::now();
      ExecutionContext context(vm.GetBytecode(), vm.GetGlobalFunctionBytecode("main"));
      context.CreateReturnMemory();
      char *params = nullptr;
      if(numOfBytesParameters)
      {
        params = new char[numOfBytesParameters];
        memset(params, 0, numOfBytesParameters);
        context.SetParameter(params);
      }

      context.Execute();
      auto elapsed = std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::steady_clock::now() - start);
      std::cout << "Runtime: " << elapsed.count() << "mcs" << std::endl;

      returnValue =  *((INT32*)context.GetReturnValue());
      context.DestroyReturnMemory();
    }

    delete package;
    package = nullptr;
  }
  else
    std::cout << "Errors in " << file << "!\n";

  /*
  std::string out;
  Node::ConvertToString(parser.GetMainNode(), packageInfo, out);
  std::cout << out;
  */

  return returnValue;
}

void RunTest(const std::string &fileName, INT expectedValue, INT numOfBytesParameters = 0, bool printInstructions = false)
{
  INT ret = RunTestFile(fileName,  numOfBytesParameters, printInstructions);
  std::cout << fileName << " ";
  if(ret == expectedValue)
    std::cout << "[ Success! ]\n";
  else
    std::cout << "[ Failed! ]\n";
  std::cout << "---\n";
}

void main()
{
  //detect memory leaks, check output for them
#ifdef WIN32
  _CrtSetDbgFlag ( _CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF );
#endif

  {
    RunTest("../scripts/Test0.script", 1, 0);    
    RunTest("../scripts/Test1.script", 1);
    RunTest("../scripts/Test2.script", 1);
    RunTest("../scripts/Test3.script", 1);
    RunTest("../scripts/Test4.script", 1);   
    RunTest("../scripts/Test5.script", 1); 
    RunTest("../scripts/Test6.script", 1);  
    RunTest("../scripts/Test7.script", 22);
    RunTest("../scripts/Test8.script", 1);
    RunTest("../scripts/Test9.script", 1);
    RunTest("../scripts/Test10.script", 1);
    RunTest("../scripts/Test11.script", 1);
    RunTest("../scripts/Test12.script", 1);
    RunTest("../scripts/Test13.script", 1);
    RunTest("../scripts/Test14.script", 1);
    RunTest("../scripts/Test15.script", 1);
    RunTest("../scripts/Test16.script", 1);  
    RunTest("../scripts/Test17.script", 1);  
    RunTest("../scripts/Test18.script", 1);
    RunTest("../scripts/Test19.script", 1);
    RunTest("../scripts/Test20.script", 1);
    RunTest("../scripts/Test21.script", 1);
    RunTest("../scripts/Test22.script", 1);
    RunTest("../scripts/Test23.script", 1);
    RunTest("../scripts/Test24.script", 1);
    RunTest("../scripts/Test25.script", 1);   
    RunTest("../scripts/Test26.script", 1);
    RunTest("../scripts/Test27.script", 1);
    RunTest("../scripts/Test28.script", 1);
    RunTest("../scripts/Test29.script", 1000000, 0);
    RunTest("../scripts/Test30.script", 0);
    RunTest("../scripts/Test31.script", 1);
    RunTest("../scripts/Test32.script", 1);
    RunTest("../scripts/Test33.script", 1);
    RunTest("../scripts/Test34.script", 1);
    RunTest("../scripts/Test35.script", 0, 6);
    RunTest("../scripts/Test36.script", 3, 12);
    RunTest("../scripts/Test37.script", -1, 6);
    RunTest("../scripts/Test38.script", 0, 12);
    RunTest("../scripts/Test39.script", 0, 12);
    RunTest("../scripts/Test40.script", 9, 12);
    RunTest("../scripts/Test41.script", 0, 12);
    RunTest("../scripts/Test42.script", 5, 12);  
    RunTest("../scripts/Test43.script", 5, 12);
    RunTest("../scripts/Test44.script", 5, 12); 
    RunTest("../scripts/Test45.script", 75025, 12);  
    RunTest("../scripts/Test46.script", 1000, 0);
    RunTest("../scripts/Test47.script", 7, 0);
    /**/
  }

  std::cout << "\n";
  std::cout << "created nodes: " <<  Node::nodeCount << std::endl;
  std::cout << "destroyed nodes: " << Node::nodeDestroyed << std::endl;
  system("pause");
}