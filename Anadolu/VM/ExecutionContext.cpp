#include "ExecutionContext.h"
#include "Parser/Package.h"
#include "Bytecode.h"

#include <assert.h>
#include <iostream>

#define RegisterAsINT32(i) *((INT32*)(registers + i) )
#define RegisterAsINT32(i) *((INT32*)(registers + i) )
#define RegisterAsINT32(i) *((INT32*)(registers + i) )
#define RegisterAsChar(i) *((char*)(registers + i) )

#define LocalAsInt32(i) *( (INT32*) (locals + i) )
#define LocalAsChar(i) *( (char*)(locals + i) ) 

#define ParamAsInt32(i) *((INT32*)(params + i))
#define ParamAsChar(i) *((char*)(params + i))

ExecutionContext::ExecutionContext(Bytecode *_bytecode, FunctionBytecode *_functionBytecode) 
  : functionBytecode(_functionBytecode),
  instructions(&_functionBytecode->optimizedInstructions), 
  thisValue(nullptr), 
  params(nullptr),
  returnValue(nullptr), 
  locals(nullptr),
  bytecode(_bytecode),
  registers(nullptr),
  executionStatus(NotPrepared)
{

}

ExecutionContext::~ExecutionContext()
{

}

void ExecutionContext::CreateReturnMemory() { returnValue = new char[functionBytecode->optimizedInstructions[0].param3]; }

void ExecutionContext::SetParameter(char *data)
{
  params = data;
}

void ExecutionContext::ExecuteInstructions()
{
  INT size = instructions->size();
  for(INT i = 0; i < size; ++i)
  {
    Instruction &instruction = (*instructions)[i];
    switch (instruction.opCode)
    {
    case OP_DAllocL:
      delete[] locals;
      delete[] registers;
      delete[] params;

      break;
    case OP_AllocL:
      // alloc locals memory
      if(instruction.param1)
      {
        locals = new char[instruction.param1];
        for(INT j = 0; j< instruction.param1; ++j )
          locals[j] = 0;
      }

      // alloc registers memory
      // allocates at least 1 register (r0)
      registers = new INT[instruction.param2 + 1];
      for(INT j = 0; j < instruction.param2 + 1; ++j)
        registers[j] = 0;

      break;
    case OP_ResetR:
      RegisterAsINT32(instruction.param1) = 0;
      break;

    case OP_CopyData4ROR:
      memcpy((char*)(registers[instruction.param1]) + instruction.param2, registers + instruction.param3, 4);
      break;
    case OP_CopyData1ROR:
      memcpy((char*)(registers[instruction.param1]) + instruction.param2, registers + instruction.param3, 1);
      break;
    case OP_CallPrep:
      registers[instruction.param1] = (INT)malloc(8);
      break;

    case OP_Call:
      {
        ExecutionContext exc(bytecode, bytecode->functionBytecodes[instruction.param1]);
        exc.returnValue = (char*)(registers + instruction.param2);
        exc.params = (char*)(*((INT**)(registers + instruction.param3)));
        exc.Execute();
      }
      break;

    case OP_JumpbR:
      if( RegisterAsChar(instruction.param1) == 1)
        i += (INT32)instruction.param2; // its true jump ahead by the given amount
      else
        i += (INT32)instruction.param3; // its true jump ahead by the given amount
      RegisterAsINT32(instruction.param1) = 0;
      break;
    case OP_Jump:
      i += (INT32)instruction.param1;
      break;

    case OP_NotbRR:
      RegisterAsChar(instruction.param1) = !RegisterAsChar(instruction.param2);
      break;

    case OP_DiviRP:
      if(ParamAsInt32(instruction.param2) != 0)
        RegisterAsINT32(instruction.param1) /= ParamAsInt32(instruction.param2);
      else
        RegisterAsINT32(instruction.param1) = 0;
      break;
    case OP_DiviLP:
      if(ParamAsInt32(instruction.param2) != 0)
        LocalAsInt32(instruction.param1) /= ParamAsInt32(instruction.param2);
      else
        LocalAsInt32(instruction.param1) = 0;
      break;
    case OP_DiviPP:
      if(ParamAsInt32(instruction.param2) != 0)
        ParamAsInt32(instruction.param1) /= ParamAsInt32(instruction.param2);
      else
        ParamAsInt32(instruction.param1) = 0;
      break;
    case OP_DiviPC:
      if(instruction.param2 != 0)
        ParamAsInt32(instruction.param1) /= instruction.param2;
      else
        ParamAsInt32(instruction.param1) = 0;
      break;
    case OP_DiviPR:
      if( RegisterAsINT32(instruction.param2) != 0)
        ParamAsInt32(instruction.param1) /= RegisterAsINT32(instruction.param2);
      else
        ParamAsInt32(instruction.param1) = 0;
      break;
    case OP_DiviPL:
      if( LocalAsInt32(instruction.param2) != 0)
        ParamAsInt32(instruction.param1) /= LocalAsInt32(instruction.param2);
      else
        ParamAsInt32(instruction.param1) = 0;
      break;
    case OP_DiviRPC:
      if( instruction.param3 != 0)
        ParamAsInt32(instruction.param1) =  ParamAsInt32(instruction.param2) / instruction.param3;
      else
        ParamAsInt32(instruction.param1) = 0;
      break;
    case OP_DiviRPL:
      if( LocalAsInt32(instruction.param3) != 0)
        RegisterAsINT32(instruction.param1) =  ParamAsInt32(instruction.param2) / LocalAsInt32(instruction.param3);
      else
        RegisterAsINT32(instruction.param1) = 0;
      break;
    case OP_DiviRPP:
      if( ParamAsInt32(instruction.param3) != 0)
        RegisterAsINT32(instruction.param1) =  ParamAsInt32(instruction.param2) / ParamAsInt32(instruction.param3);
      else
        RegisterAsINT32(instruction.param1) = 0;
      break;
    case OP_DiviRPR:
      if( RegisterAsINT32(instruction.param3) != 0)
        RegisterAsINT32(instruction.param1) =  ParamAsInt32(instruction.param2) / RegisterAsINT32(instruction.param3);
      else
        RegisterAsINT32(instruction.param1) = 0;
      break;
    case OP_DiviRCP:
      if( ParamAsInt32(instruction.param3) != 0)
        RegisterAsINT32(instruction.param1) =  instruction.param2 / ParamAsInt32(instruction.param3);
      else
        RegisterAsINT32(instruction.param1) = 0;
      break;
    case OP_DiviRLP:
      if( ParamAsInt32(instruction.param3) != 0)
        RegisterAsINT32(instruction.param1) =  LocalAsInt32(instruction.param2) / ParamAsInt32(instruction.param3);
      else
        RegisterAsINT32(instruction.param1) = 0;
      break;
    case OP_DiviRRP:
      if( ParamAsInt32(instruction.param3) != 0)
        RegisterAsINT32(instruction.param1) =  RegisterAsINT32(instruction.param2) / ParamAsInt32(instruction.param3);
      else
        RegisterAsINT32(instruction.param1) = 0;
      break;

    case OP_DiviRLR:
      if( RegisterAsINT32(instruction.param3) == 0)
      {
        //TODO: show error message
        RegisterAsINT32(instruction.param1) = 0;
        break;
      }
      RegisterAsINT32(instruction.param1) = LocalAsInt32(instruction.param2) / RegisterAsINT32(instruction.param3);
      break;
    case OP_DiviRRL:
      if( LocalAsInt32(instruction.param3) == 0)
      {
        //TODO: show error message
        RegisterAsINT32(instruction.param1) = 0;
        break;
      }
      RegisterAsINT32(instruction.param1) = RegisterAsINT32(instruction.param2) / LocalAsInt32(instruction.param3);
      break;
    case OP_DiviRRC:
      // this constant cannot be zero, we already check it in codegen
      RegisterAsINT32(instruction.param1) = RegisterAsINT32(instruction.param2) / instruction.param3;
      break;
    case OP_DiviRCR:
      if( RegisterAsINT32(instruction.param3) == 0)
      {
        //TODO: show error message
        RegisterAsINT32(instruction.param1) = 0;
        break;
      }
      RegisterAsINT32(instruction.param1) = instruction.param2 / RegisterAsINT32(instruction.param3);
      break;
    case OP_DiviRLL:
      if(RegisterAsINT32(instruction.param3) == 0)
      {
        //TODO: show error message
        RegisterAsINT32(instruction.param1) = 0;
        break;
      }
      RegisterAsINT32(instruction.param1) = LocalAsInt32(instruction.param2) / LocalAsInt32(instruction.param3);
      break;
    case OP_DiviRLC:
      RegisterAsINT32(instruction.param1) = LocalAsInt32(instruction.param2) / instruction.param3;
      break;
    case OP_DiviRCL:
      if(LocalAsInt32(instruction.param3) == 0)
      {
        //TODO: show error message
        RegisterAsINT32(instruction.param1) = 0;
        break;
      }
      RegisterAsINT32(instruction.param1) = instruction.param2 / LocalAsInt32(instruction.param3);
      break;
    case OP_DiviRRR:
      if( RegisterAsINT32(instruction.param3) == 0)
      {
        //TODO: show error message
        RegisterAsINT32(instruction.param1) = 0;
        break;
      }
      RegisterAsINT32(instruction.param1) = RegisterAsINT32(instruction.param2) / RegisterAsINT32(instruction.param3);
      break;


      // multiply operators

    case OP_MuliPC:
      ParamAsInt32(instruction.param1) *= instruction.param2;
      break;
    case OP_MuliRP:
      RegisterAsINT32(instruction.param1) *= ParamAsInt32(instruction.param2);
      break;
    case OP_MuliLP:
      LocalAsInt32(instruction.param1) *= ParamAsInt32(instruction.param2);
      break;
    case OP_MuliRPR:
      RegisterAsINT32(instruction.param1) = ParamAsInt32(instruction.param2) * RegisterAsINT32(instruction.param3);
      break;
    case OP_MuliRPC:
      RegisterAsINT32(instruction.param1) = ParamAsInt32(instruction.param2) * instruction.param3;
      break;
    case OP_MuliRPL:
      RegisterAsINT32(instruction.param1) = ParamAsInt32(instruction.param2) * LocalAsInt32(instruction.param3);
      break;
    case OP_MuliRPP:
      RegisterAsINT32(instruction.param1) = ParamAsInt32(instruction.param2) * ParamAsInt32(instruction.param3);
      break;

    case OP_MuliRR:
      RegisterAsINT32(instruction.param1) *= RegisterAsINT32(instruction.param2);
      break;
    case OP_MuliRL:
      RegisterAsINT32(instruction.param1) *= LocalAsInt32(instruction.param2);   
      break;
    case OP_MuliRLL:
      RegisterAsINT32(instruction.param1) = LocalAsInt32(instruction.param2) * LocalAsInt32(instruction.param3);
      break;
    case OP_MuliRLC:
      RegisterAsINT32(instruction.param1) = LocalAsInt32(instruction.param2 ) * instruction.param3;
      break;
    case OP_MuliRC:
      RegisterAsINT32(instruction.param1) *= instruction.param2;
      break;

      // add operators

    case OP_AddiPR:
      ParamAsInt32(instruction.param1) += RegisterAsINT32(instruction.param2);
      break;
    case OP_AddiPC:
      ParamAsInt32(instruction.param1) += instruction.param2;
      break;
    case OP_AddiRP:
      RegisterAsINT32(instruction.param1) += ParamAsInt32(instruction.param2);
      break;
    case OP_AddiRPL:
      RegisterAsINT32(instruction.param1) = ParamAsInt32(instruction.param2) + LocalAsInt32(instruction.param3);
      break;
    case OP_AddiRPR:
      RegisterAsINT32(instruction.param1) = ParamAsInt32(instruction.param2) + RegisterAsINT32(instruction.param3);
      break;
    case OP_AddiRPC:
      RegisterAsINT32(instruction.param1) = ParamAsInt32(instruction.param2) + instruction.param3;
      break;
    case OP_AddiRPP:
      RegisterAsINT32(instruction.param1) = ParamAsInt32(instruction.param2) + ParamAsInt32(instruction.param3);
      break;

    case OP_AddiRL:
      RegisterAsINT32(instruction.param1) += LocalAsInt32(instruction.param2);
      break;
    case OP_AddiRRR:
      RegisterAsINT32(instruction.param1) = RegisterAsINT32(instruction.param2) + RegisterAsINT32(instruction.param3);
      break;
    case OP_AddiRLR:
      RegisterAsINT32(instruction.param1) = LocalAsInt32(instruction.param2) + RegisterAsINT32(instruction.param3);
      break;
    case OP_AddiRLL:
      RegisterAsINT32(instruction.param1) = LocalAsInt32(instruction.param2) + LocalAsInt32(instruction.param3);
      break;
    case OP_AddiRLC:
      RegisterAsINT32(instruction.param1) = LocalAsInt32(instruction.param2) + instruction.param3;
      break;
    case OP_AddiRRC:
      RegisterAsINT32(instruction.param1) = RegisterAsINT32(instruction.param2) + instruction.param3;
      break;
    case OP_AddiLR:
      LocalAsInt32( instruction.param1) += RegisterAsINT32(instruction.param2);
      break;
    case OP_AddiRR:
      RegisterAsINT32(instruction.param1)  += RegisterAsINT32(instruction.param2);
      break;
    case OP_AddiLC:
      LocalAsInt32(instruction.param1)  += instruction.param2;
      break;
    case OP_AddiRC:
      RegisterAsINT32(instruction.param1)  += instruction.param2;
      break;


      // SUBTRACT operators
    case  OP_SubiRP:
      RegisterAsINT32(instruction.param1) -= ParamAsInt32(instruction.param2);
      break;
    case OP_SubiPP:
      ParamAsInt32(instruction.param1) -= ParamAsInt32(instruction.param2);
      break;
    case OP_SubiPC:
      ParamAsInt32(instruction.param1) -= instruction.param2;
      break;
    case OP_SubiPL:
      ParamAsInt32(instruction.param1) -= LocalAsInt32(instruction.param2);
      break;
    case OP_SubiRRP:
      RegisterAsINT32(instruction.param1) = RegisterAsINT32(instruction.param2) - ParamAsInt32(instruction.param3);
      break;
    case OP_SubiRPR:
      RegisterAsINT32(instruction.param1) = ParamAsInt32(instruction.param2) - RegisterAsINT32(instruction.param3);
      break;
    case OP_SubiRLP:
      RegisterAsINT32(instruction.param1) = LocalAsInt32(instruction.param2) - ParamAsInt32(instruction.param3);
      break;
    case OP_SubiRPL:
      RegisterAsINT32(instruction.param1) = ParamAsInt32(instruction.param2) - LocalAsInt32(instruction.param3);
      break;
    case OP_SubiRPC:
      RegisterAsINT32(instruction.param1) = ParamAsInt32(instruction.param2) - instruction.param3;
      break;
    case OP_SubiRCP:
      RegisterAsINT32(instruction.param1) = instruction.param2 - ParamAsInt32(instruction.param3);
      break;
    case OP_SubiRPP:
      RegisterAsINT32(instruction.param1) = ParamAsInt32(instruction.param2) - ParamAsInt32(instruction.param3);
      break;
    case OP_SubiRLL:
      RegisterAsINT32(instruction.param1)  = LocalAsInt32(instruction.param2) - LocalAsInt32( instruction.param3);
      break;
    case OP_SubiRCL:
      RegisterAsINT32(instruction.param1)  = instruction.param2 - LocalAsInt32( instruction.param3);
      break;
    case OP_SubiRLC:
      RegisterAsINT32(instruction.param1)  = LocalAsInt32( instruction.param2) - instruction.param3;
      break;
    case OP_SubiRRR:
      RegisterAsINT32(instruction.param1)  = RegisterAsINT32(instruction.param2) + RegisterAsINT32(instruction.param3);
      break;
    case OP_SubiRLR:
      RegisterAsINT32(instruction.param1)  = RegisterAsINT32( instruction.param2) - RegisterAsINT32(instruction.param3);
      break;
    case OP_SubiRRL:
      RegisterAsINT32(instruction.param1)  = RegisterAsINT32(instruction.param2) - LocalAsInt32( instruction.param3);
      break;
    case OP_SubiRCR:
      RegisterAsINT32(instruction.param1)  = instruction.param2 - RegisterAsINT32(instruction.param3);
      break;
    case OP_SubiRRC:
      RegisterAsINT32(instruction.param1)  = RegisterAsINT32(instruction.param2) - instruction.param3;
      break;
    case OP_SubiRC:
      RegisterAsINT32(instruction.param1)  -= instruction.param2;
      break;
    case OP_SubiRR:
      RegisterAsINT32(instruction.param1)  -= RegisterAsINT32(instruction.param2);
      break;
    case OP_SubiRL:
      RegisterAsINT32(instruction.param1)  -= LocalAsInt32( instruction.param2);
      break;
    case OP_SubiLR:
      LocalAsInt32( instruction.param1) -=  RegisterAsINT32(instruction.param2);
      break;


      // Copy operators

    case OP_CopybLP:
      LocalAsChar(instruction.param1)  = ParamAsChar(instruction.param2);
      break;
    case OP_CopybRP:
      RegisterAsChar(instruction.param1)  = ParamAsChar(instruction.param2);
      break;
    case OP_CopybPR: 
      ParamAsChar(instruction.param1)  = RegisterAsChar(instruction.param2);
      break;    
    case OP_CopybPL:
      ParamAsChar(instruction.param1)  = LocalAsChar(instruction.param2);
      break;

    case OP_CopyiLP:
      LocalAsInt32(instruction.param1)  = ParamAsInt32(instruction.param2);
      break;
    case OP_CopyiRP:
      RegisterAsINT32(instruction.param1)  = ParamAsInt32(instruction.param2);
      break;
    case OP_CopyiPR:
      ParamAsInt32(instruction.param1)  = RegisterAsINT32(instruction.param2);
      break;
    case OP_CopyiPL:
      ParamAsInt32(instruction.param1)  = LocalAsInt32(instruction.param2);
      break;

    case OP_CopyiRC:
      RegisterAsINT32(instruction.param1)  = instruction.param2;
      break;
    case OP_CopyiRR:
      RegisterAsINT32(instruction.param1)  = RegisterAsINT32(instruction.param2);
      break;
    case OP_CopyiRL:
      RegisterAsINT32(instruction.param1) = LocalAsInt32(instruction.param2);
      break;
    case OP_CopyiLR:
      LocalAsInt32(instruction.param1 ) = RegisterAsINT32(instruction.param2);
      break;
    case OP_CopyiXR:
      * ((INT32*) returnValue) = RegisterAsINT32(instruction.param1);
      break;


      //bools
    case OP_CopybRR:
      RegisterAsChar(instruction.param1) = RegisterAsChar(instruction.param2);
      break;
    case OP_CopybLR:
      LocalAsChar(instruction.param1) = RegisterAsChar(instruction.param2);
      break;
    case OP_CopybRC:
      RegisterAsChar(instruction.param1) = instruction.param2;
      break;
    case OP_CopybRL:
      RegisterAsChar(instruction.param1) = LocalAsChar(instruction.param2);
      break;


      // COMPARISON OPERATORS

    case OP_CmpbRPP:
      if(ParamAsChar( instruction.param2) == ParamAsChar(instruction.param3) )
        RegisterAsChar(instruction.param1) = 1;
      else
        RegisterAsChar(instruction.param1) = 0;
      break;
    case OP_CmpbRPL:
      if(ParamAsChar( instruction.param2) == LocalAsChar(instruction.param3) )
        RegisterAsChar(instruction.param1) = 1;
      else
        RegisterAsChar(instruction.param1) = 0;
      break;
    case OP_CmpbRPR:
      if(ParamAsChar( instruction.param2) == RegisterAsChar(instruction.param3) )
        RegisterAsChar(instruction.param1) = 1;
      else
        RegisterAsChar(instruction.param1) = 0;
      break;
    case OP_CmpbRPC:
      if(ParamAsChar( instruction.param2) == instruction.param3 )
        RegisterAsChar(instruction.param1) = 1;
      else
        RegisterAsChar(instruction.param1) = 0;
      break;
    case OP_CmpiRPP:
      if(ParamAsInt32(instruction.param2) == ParamAsInt32( instruction.param3) )
        RegisterAsChar(instruction.param1) = 1;
      else
        RegisterAsChar(instruction.param1) = 0;
      break;
    case OP_CmpiRPL:
      if(ParamAsInt32( instruction.param2) == LocalAsInt32( instruction.param3) )
        RegisterAsChar(instruction.param1) = 1;
      else
        RegisterAsChar(instruction.param1) = 0;
      break;
    case OP_CmpiRPR:
      if(ParamAsInt32( instruction.param2) == RegisterAsINT32( instruction.param3) )
        RegisterAsChar(instruction.param1) = 1;
      else
        RegisterAsChar(instruction.param1) = 0;
      break;
    case OP_CmpiRPC:
      if(ParamAsInt32( instruction.param2) == instruction.param3 )
        RegisterAsChar(instruction.param1) = 1;
      else
        RegisterAsChar(instruction.param1) = 0;
      break;

    case OP_CmpbRC:
      if(RegisterAsChar( instruction.param1) == LocalAsChar(instruction.param2) )
        RegisterAsChar(instruction.param1) = 1;
      else
        RegisterAsChar(instruction.param1) = 0;
      break;

    case OP_CmpbRLL:
      if(LocalAsChar( instruction.param2) == LocalAsChar(instruction.param3) )
        RegisterAsChar(instruction.param1) = 1;
      else
        RegisterAsChar(instruction.param1) = 0;
      break;
    case OP_CmpbRRR:
      if(RegisterAsChar(instruction.param2) == RegisterAsChar(instruction.param3) )
        RegisterAsChar(instruction.param1) = 1;
      else
        RegisterAsChar(instruction.param1) = 0;
      break;
    case OP_CmpbRLC:
      if( LocalAsChar( instruction.param2) == instruction.param3)
        RegisterAsChar(instruction.param1) = 1;
      else
        RegisterAsChar(instruction.param1) = 0;
      break;
    case OP_CmpbRCR:
      if( instruction.param2 == RegisterAsChar(instruction.param3) )
        RegisterAsChar(instruction.param1) = 1;
      else
        RegisterAsChar(instruction.param1) = 0;
      break;
    case OP_CmpbRLR:
      if( LocalAsChar( instruction.param2) == RegisterAsChar(instruction.param3) )
        RegisterAsChar(instruction.param1) = 1;
      else
        RegisterAsChar(instruction.param1) = 0;
      break;
    case OP_CmpiRLL:
      if( LocalAsInt32(  instruction.param2 ) == LocalAsInt32( instruction.param3)  )
        RegisterAsChar(instruction.param1) = 1;
      else
        RegisterAsChar(instruction.param1) = 0;
      break;
    case OP_CmpiRLC:
      if(LocalAsInt32( instruction.param2) == instruction.param3)
        RegisterAsChar(instruction.param1) = 1;
      else
        RegisterAsChar(instruction.param1) = 0;
      break;
    case OP_CmpiRRR:
      if( RegisterAsINT32(instruction.param2) ==  RegisterAsINT32(instruction.param3) )
        RegisterAsChar(instruction.param1) = 1;
      else
        RegisterAsChar(instruction.param1) = 0;
      break;
    case OP_CmpiRCR:
      if( instruction.param2 == RegisterAsINT32(instruction.param3) )
        RegisterAsChar(instruction.param1) = 1;
      else
        RegisterAsChar(instruction.param1) = 0;
      break;
    case OP_CmpiRLR:
      if( LocalAsInt32(instruction.param2) == RegisterAsINT32(instruction.param3) )
        RegisterAsChar(instruction.param1) = 1;
      else
        RegisterAsChar(instruction.param1) = 0;
      break;

    case OP_Return:
      executionStatus = Returned;
      return;
      // BLOCK OPERATORS
    case OP_BStart:
      break; // TODO: call constructors of this block stack variables
    case OP_BEnd:
      break; // TODO: call destructors of this block stack variables
    default:
      assert(0);// we forgot executing an instruction
      break;
    }
  }
}

void ExecutionContext::Execute()
{
  executionStatus = Executing;
  ExecuteInstructions();
}