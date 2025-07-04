#include "Bytecode.h"

#include <sstream>

FunctionBytecode *Bytecode::GetFunctionBytecodeIndex(size_t id)
{
  return functionBytecodes[id];
}

FunctionBytecode *Bytecode::GetFunctionBytecode(const std::string &name)
{
  auto &it = globalFunctionNames.find(name);
  if(it != globalFunctionNames.end())
    return GetFunctionBytecodeIndex(it->second);
  return nullptr;
}

void Bytecode::GetByteCode(std::string &str, bool lineNumbers)
{
  auto GetInstructionString = [](std::stringstream &ss, INT &stackDepth, Instruction &instruction)
  {

    // indent
    for(INT i = 0; i < stackDepth; ++i)
      ss << "  ";

    switch (instruction.opCode)
    {
    case OP_AllocL:
      ss << "AllocL ";
      ss << instruction.param1;
      ss << " " << instruction.param2;
      break;
    case OP_DAllocL:
      ss << "DAllocL";
      break;
    case OP_BStart:
      ss << "  BStart ";
      stackDepth++;
      break;
    case OP_BEnd:
      ss << "BEnd ";
      stackDepth--;
      break;

    case OP_ResetR:
      ss << "ResetR";
      ss << " r" << instruction.param1;
      break;

    case OP_Jump:
      ss << "Jump";
      ss << " " << instruction.param1;
      break;

    case OP_CallPrep:
      ss << "CallPrep";
      ss << " r" << instruction.param1;
      ss << " " << instruction.param2;
      break;

    case OP_Call:
      ss << "Call";
      ss << " f" << instruction.param1;
      ss << " " << instruction.param2;
      ss << " r" << instruction.param3;
      break;

    case OP_NotbRR:
      ss << "NotbRR";
      ss << " r" << instruction.param1;
      ss << " r" << instruction.param2;
      break;


    case OP_DiviRP:
      ss << "DiviRP";
      ss << " r" << instruction.param1;
      ss << " p" << instruction.param2;
      break;
    case OP_DiviLP:
      ss << "DiviLP";
      ss << " l" << instruction.param1;
      ss << " p" << instruction.param2;
      break;
    case OP_DiviPP:
      ss << "DiviPP";
      ss << " p" << instruction.param1;
      ss << " p" << instruction.param2;
      break;
    case OP_DiviPC:
      ss << "DiviPC";
      ss << " p" << instruction.param1;
      ss << " " << instruction.param2;
      break;
    case OP_DiviPR:
      ss << "DiviPR";
      ss << " p" << instruction.param1;
      ss << " r" << instruction.param2;
      break;
    case OP_DiviPL:
      ss << "DiviPL";
      ss << " p" << instruction.param1;
      ss << " l" << instruction.param2;
      break;
    case OP_DiviRPC:
      ss << "DiviRPC";
      ss << " r" << instruction.param1;
      ss << " p" << instruction.param2;
      ss << " " << instruction.param3;
      break;
    case OP_DiviRPL:
      ss << "DiviRPL";
      ss << " r" << instruction.param1;
      ss << " p" << instruction.param2;
      ss << " l" << instruction.param3;
      break;
    case OP_DiviRPP:
      ss << "DiviRPP";
      ss << " r" << instruction.param1;
      ss << " p" << instruction.param2;
      ss << " p" << instruction.param3;
      break;
    case OP_DiviRPR:
      ss << "DiviRPR";
      ss << " r" << instruction.param1;
      ss << " p" << instruction.param2;
      ss << " r" << instruction.param3;
      break;
    case OP_DiviRCP:
      ss << "DiviRCP";
      ss << " r" << instruction.param1;
      ss << " c" << instruction.param2;
      ss << " p" << instruction.param3;
      break;
    case OP_DiviRLP:
      ss << "DiviRLP";
      ss << " r" << instruction.param1;
      ss << " l" << instruction.param2;
      ss << " p" << instruction.param3;
      break;
    case OP_DiviRRP:
      ss << "DiviRRP";
      ss << " r" << instruction.param1;
      ss << " r" << instruction.param2;
      ss << " p" << instruction.param3;
      break;


    case OP_DiviRLR:
      ss << "DiviRLR";
      ss << " r" << instruction.param1;
      ss << " l" << instruction.param2;
      ss << " r" << instruction.param3;
      break;
    case OP_DiviRRL:
      ss << "DiviRRL";
      ss << " r" << instruction.param1;
      ss << " r" << instruction.param2;
      ss << " l" << instruction.param3;
      break;
    case OP_DiviRRC:
      ss << "DiviRRC";
      ss << " r" << instruction.param1;
      ss << " r" << instruction.param2;
      ss << " c" << instruction.param3;
      break;
    case OP_DiviRCR:
      ss << "DiviRCR";
      ss << " r" << instruction.param1;
      ss << " c" << instruction.param2;
      ss << " r" << instruction.param3;
      break;
    case OP_DiviRLL:
      ss << "DiviRLL";
      ss << " r" << instruction.param1;
      ss << " l" << instruction.param2;
      ss << " l" << instruction.param3;
      break;
    case OP_DiviRLC:
      ss << "DiviRCL";
      ss << " r" << instruction.param1;
      ss << " c" << instruction.param2;
      ss << " l" << instruction.param3;
      break;
    case OP_DiviRCL:
      ss << "DiviRCL";
      ss << " r" << instruction.param1;
      ss << " c" << instruction.param2;
      ss << " l" << instruction.param3;
      break;
    case OP_DiviRRR:
      ss << "DiviRRR";
      ss << " r" << instruction.param1;
      ss << " r" << instruction.param2;
      ss << " r" << instruction.param3;
      break;

    case OP_MuliPC:
      ss << "MuliPC";
      ss << " p" << instruction.param1;
      ss << " " << instruction.param2;
      break;
    case OP_MuliRP:
      ss << "MuliRP";
      ss << " r" << instruction.param1;
      ss << " p" << instruction.param2;
      break;
    case OP_MuliLP:
      ss << "MuliLP";
      ss << " l" << instruction.param1;
      ss << " p" << instruction.param2;
      break;
    case OP_MuliRPR:
      ss << "MuliRPR";
      ss << " r" << instruction.param1;
      ss << " p" << instruction.param2;
      ss << " r" << instruction.param3;
      break;
    case OP_MuliRPC:
      ss << "MuliRPC";
      ss << " r" << instruction.param1;
      ss << " p" << instruction.param2;
      ss << " " << instruction.param3;
      break;
    case OP_MuliRPL:
      ss << "MuliRPL";
      ss << " r" << instruction.param1;
      ss << " p" << instruction.param2;
      ss << " l" << instruction.param3;
      break;
    case OP_MuliRPP:
      ss << "MuliRPP";
      ss << " r" << instruction.param1;
      ss << " p" << instruction.param2;
      ss << " p" << instruction.param3;
      break;

    case OP_MuliRR:
      ss << "MuliRR";
      ss << " r" << instruction.param1;
      ss << " r" << instruction.param2;
      break;

    case OP_MuliRL:
      ss << "MuliRL";
      ss << " r" << instruction.param1;
      ss << " l" << instruction.param2;
      break;

    case OP_MuliRLL:
      ss << "MuliRLL";
      ss << " r" << instruction.param1;
      ss << " l" << instruction.param2;
      ss << " l" << instruction.param3;
      break;

    case OP_MuliRLC:
      ss << "MuliRLC";
      ss << " r" << instruction.param1;
      ss << " l" << instruction.param2;
      ss << " " << instruction.param3;
      break;
    case OP_MuliRC:
      ss << "MuliRC";
      ss << " r" << instruction.param1;
      ss << " " << instruction.param2;
      break;
    case OP_MuliLC:
      ss << "MuliLC";
      ss << " l" << instruction.param1;
      ss << " " << instruction.param2;
      break;

    case OP_JumpbR:
      ss << "JumpbR";
      ss << " r" << instruction.param1;
      ss << " " << instruction.param2;
      ss << " " << instruction.param3;
      break;
      // ADD operators

    case OP_AddiPR:
      ss << "AddiPR";
      ss << " p" << instruction.param1;
      ss << " r" << instruction.param2;
      break;
    case OP_AddiPC:
      ss << "AddiPC";
      ss << " p" << instruction.param1;
      ss << " " << instruction.param2;
      break;
    case OP_AddiRP:
      ss << "AddiRP";
      ss << " r" << instruction.param1;
      ss << " p" << instruction.param2;
      break;
    case OP_AddiRPL:
      ss << "AddiRPL";
      ss << " r" << instruction.param1;
      ss << " p" << instruction.param2;
      ss << " l" << instruction.param3;
      break;
    case OP_AddiRPR:
      ss << "AddiRPR";
      ss << " r" << instruction.param1;
      ss << " p" << instruction.param2;
      ss << " r" << instruction.param3;
      break;
    case OP_AddiRPC:
      ss << "AddiRPC";
      ss << " r" << instruction.param1;
      ss << " p" << instruction.param2;
      ss << " " << instruction.param3;
      break;
    case OP_AddiRPP:
      ss << "AddiRPP";
      ss << " r" << instruction.param1;
      ss << " p" << instruction.param2;
      ss << " p" << instruction.param3;
      break;

    case OP_AddiRRR:
      ss << "AddiRRR";
      ss << " r" << instruction.param1;
      ss << " r" << instruction.param2;
      ss << " r" << instruction.param3;
      break;
    case OP_AddiRLR:
      ss << "AddiRLR";
      ss << " r" << instruction.param1;
      ss << " l" << instruction.param2;
      ss << " r" << instruction.param3;
      break;
    case OP_AddiRLL:
      ss << "AddiRLL";
      ss << " r" << instruction.param1;
      ss << " l" << instruction.param2;
      ss << " l" << instruction.param3;
      break;
    case OP_AddiRLC:
      ss << "AddiRLC";
      ss << " r" << instruction.param1;
      ss << " l" << instruction.param2;
      ss << " " << instruction.param3;
      break;
    case OP_AddiRRC:
      ss << "AddiRRC";
      ss << " r" << instruction.param1;
      ss << " r" << instruction.param2;
      ss << " " << instruction.param3;
      break;
    case OP_AddiLR:
      ss << "AddiLR";
      ss << " l" << instruction.param1;
      ss << " r" << instruction.param2;
      break;
    case OP_AddiRR:
      ss << "AddirR";
      ss << " r" << instruction.param1;
      ss << " r" << instruction.param2;
      break;
    case OP_AddiLC:
      ss << "AddiLC";
      ss << " l" << instruction.param1;
      ss << " " << instruction.param2;
      break;
    case OP_AddiRC:
      ss << "AddiRC";
      ss << " r" << instruction.param1;
      ss << " " << instruction.param2;
      break;

      // SUBTRACT operators

    case OP_SubiRP:
      ss << "SubiRP";
      ss << " r" << instruction.param1;
      ss << " p" << instruction.param2;
      break;
    case OP_SubiPP:
      ss << "SubiPP";
      ss << " p" << instruction.param1;
      ss << " p" << instruction.param2;
      break;
    case OP_SubiPC:
      ss << "SubiPC";
      ss << " p" << instruction.param1;
      ss << " c" << instruction.param2;
      break;
    case OP_SubiPL:
      ss << "SubiPL";
      ss << " p" << instruction.param1;
      ss << " l" << instruction.param2;
      break;
    case OP_SubiRRP:
      ss << "SubiRRP";
      ss << " r" << instruction.param1;
      ss << " r" << instruction.param2;
      ss << " p" << instruction.param3;
      break;
    case OP_SubiRPR:
      ss << "SubiRPR";
      ss << " r" << instruction.param1;
      ss << " p" << instruction.param2;
      ss << " r" << instruction.param3;
      break;
    case OP_SubiRLP:
      ss << "SubiRLP";
      ss << " r" << instruction.param1;
      ss << " l" << instruction.param2;
      ss << " p" << instruction.param3;
      break;
    case OP_SubiRPL:
      ss << "SubiRPL";
      ss << " r" << instruction.param1;
      ss << " p" << instruction.param2;
      ss << " l" << instruction.param3;
      break;
    case OP_SubiRPC:
      ss << "SubiRPC";
      ss << " r" << instruction.param1;
      ss << " p" << instruction.param2;
      ss << " " << instruction.param3;
      break;
    case OP_SubiRCP:
      ss << "SubiRCP";
      ss << " r" << instruction.param1;
      ss << " " << instruction.param2;
      ss << " p" << instruction.param3;
      break;
    case OP_SubiRPP:
      ss << "SubiRPP";
      ss << " r" << instruction.param1;
      ss << " p" << instruction.param2;
      ss << " p" << instruction.param3;
      break;

    case OP_SubiRLL:
      ss << "SubiRLL";
      ss << " r" << instruction.param1;
      ss << " l" << instruction.param2;
      ss << " l" << instruction.param3;
      break;

    case OP_SubiRCL:
      ss << "SubiRCL";
      ss << " r" << instruction.param1;
      ss << " " << instruction.param2;
      ss << " l" << instruction.param3;
      break;

    case OP_SubiRLC:
      ss << "SubiRLC";
      ss << " r" << instruction.param1;
      ss << " l" << instruction.param2;
      ss << " " << instruction.param3;
      break;

    case OP_SubiRRR:
      ss << "SubiRRR";
      ss << " r" << instruction.param1;
      ss << " r" << instruction.param2;
      ss << " r" << instruction.param3;
      break;
    case OP_SubiRLR:
      ss << "SubiRLR";
      ss << " r" << instruction.param1;
      ss << " l" << instruction.param2;
      ss << " r" << instruction.param3;
      break;
    case OP_SubiRRL:
      ss << "SubiRRL";
      ss << " r" << instruction.param1;
      ss << " r" << instruction.param2;
      ss << " l" << instruction.param3;
      break;
    case OP_SubiRCR:
      ss << "SubiRCR";
      ss << " r" << instruction.param1;
      ss << " " << instruction.param2;
      ss << " r" << instruction.param3;
      break;
    case OP_SubiRRC:
      ss << "SubiRRC";
      ss << " r" << instruction.param1;
      ss << " r" << instruction.param2;
      ss << " " << instruction.param3;
      break;
    case OP_SubiRC:
      ss << "SubiRC";
      ss << " r" << instruction.param1;
      ss << " " << instruction.param2;
      break;
    case OP_SubiRR:
      ss << "SubiRR";
      ss << " r" << instruction.param1;
      ss << " r" << instruction.param2;
      break;
    case OP_SubiRL:
      ss << "SubiRL";
      ss << " r" << instruction.param1;
      ss << " l" << instruction.param2;
      break;
    case OP_SubiLR:
      ss << "SubiLR";
      ss << " l" << instruction.param1;
      ss << " r" << instruction.param2;
      break;

      // Copy operators


    case OP_CopyData4ROR:
      ss << "CopyData4ROR";
      ss << " r" << instruction.param1;
      ss << " " << instruction.param2;
      ss << " r" << instruction.param3;
      break;
    case OP_CopyData1ROR:
      ss << "CopyData1ROR";
      ss << " r" << instruction.param1;
      ss << " " << instruction.param2;
      ss << " r" << instruction.param3;
      break;

    case OP_CopyiRR:
      ss << "CopyiRR";
      ss << " r" << instruction.param1;
      ss << " r" << instruction.param2;
      break;
    case OP_CopyiRC:
      ss << "CopyiRC";
      ss << " r" << instruction.param1;
      ss << " " << instruction.param2;
      break;
    case OP_CopyiRL:
      ss << "CopyiRL";
      ss << " r" << instruction.param1;
      ss << " l" << instruction.param2;
      break;
    case OP_CopyiLR:
      ss << "CopyiLR";
      ss << " l" << instruction.param1;
      ss << " r" << instruction.param2;
      break;
    case OP_CopyiXR:
      ss << "CopyiXR";
      ss << " r" << instruction.param1;
      break;
    case OP_CopybRR:
      ss << "CopybRR";
      ss << " r" << instruction.param1;
      ss << " r" << instruction.param2;
      break;
    case OP_CopybLR:
      ss << "CopybLR";
      ss << " l" << instruction.param1;
      ss << " r" << instruction.param2;
      break;
    case OP_CopybRC:
      ss << "CopybRC";
      ss << " r" << instruction.param1;
      ss << " " << instruction.param2;
      break;
    case OP_CopybRL:
      ss << "CopybRL";
      ss << " r" << instruction.param1;
      ss << " l" << instruction.param2;
      break;


    case  OP_CmpbRPP:
      ss << "CmpbRPP";
      ss << " r" << instruction.param1;
      ss << " p" << instruction.param2;
      ss << " p" << instruction.param3;
      break;
    case  OP_CmpbRPL:
      ss << "CmpbRPL";
      ss << " r" << instruction.param1;
      ss << " p" << instruction.param2;
      ss << " l" << instruction.param3;
      break;
    case  OP_CmpbRPR:
      ss << "CmpbRPR";
      ss << " r" << instruction.param1;
      ss << " p" << instruction.param2;
      ss << " r" << instruction.param3;
      break;
    case  OP_CmpbRPC:
      ss << "CmpbRPC";
      ss << " r" << instruction.param1;
      ss << " p" << instruction.param2;
      ss << " c" << instruction.param3;
      break;
    case  OP_CmpiRPP:
      ss << "CmpiRPP";
      ss << " r" << instruction.param1;
      ss << " p" << instruction.param2;
      ss << " p" << instruction.param3;
      break;
    case  OP_CmpiRPL:
      ss << "CmpbRPL";
      ss << " r" << instruction.param1;
      ss << " p" << instruction.param2;
      ss << " l" << instruction.param3;
      break;
    case  OP_CmpiRPR:
      ss << "CmpiRPR";
      ss << " r" << instruction.param1;
      ss << " p" << instruction.param2;
      ss << " r" << instruction.param3;
      break;
    case  OP_CmpiRPC:
      ss << "CmpiRPC";
      ss << " r" << instruction.param1;
      ss << " p" << instruction.param2;
      ss << " " << instruction.param3;
      break;
    case  OP_CmpbRC:
      ss << "CmpbRPC";
      ss << " r" << instruction.param1;
      ss << " " << instruction.param2;
      break;

    case OP_CmpbRRR:
      ss << "CmpbRRR";
      ss << " r" << instruction.param1;
      ss << " r" << instruction.param2;
      ss << " r" << instruction.param3;
      break;
    case OP_CmpbRCR:
      ss << "CmpbRCR";
      ss << " r" << instruction.param1;
      ss << " " << instruction.param2;
      ss << " r" << instruction.param3;
      break;
    case OP_CmpbRLR:
      ss << "CmpbRLR";
      ss << " r" << instruction.param1;
      ss << " l" << instruction.param2;
      ss << " r" << instruction.param3;
      break;
    case OP_CmpiRLL:
      ss << "CmpiRLL";
      ss << " r" << instruction.param1;
      ss << " l" << instruction.param2;
      ss << " l" << instruction.param3;
      break;
    case OP_CmpiRLC:
      ss << "CmpiRLC";
      ss << " r" << instruction.param1;
      ss << " l" << instruction.param2;
      ss << " " << instruction.param3;
      break;
    case OP_CmpiRRR:
      ss << "CmpiRRR";
      ss << " r" << instruction.param1;
      ss << " r" << instruction.param2;
      ss << " r" << instruction.param3;
      break;
    case OP_CmpbRLC:
      ss << "CmpbRLC";
      ss << " r" << instruction.param1;
      ss << " l" << instruction.param2;
      ss << " " << instruction.param3;
      break;
    case OP_CmpiRCR:
      ss << "CmpiRCR";
      ss << " r" << instruction.param1;
      ss << " " << instruction.param2;
      ss << " r" << instruction.param3;
      break;
    case OP_CmpiRLR:
      ss << "CmpiRLR";
      ss << " r" << instruction.param1;
      ss << " l" << instruction.param2;
      ss << " r" << instruction.param3;
      break;

    case OP_Return:
      ss << "Return";
      break;

    case OP_CopyiLP:
      ss << "CopyiLP";
      ss << " r" << instruction.param1;
      ss << " p" << instruction.param2;
      break;
    case OP_CopybLP:
      ss << "CopybLP";
      ss << " l" << instruction.param1;
      ss << " p" << instruction.param2;
      break;
    case OP_CopyiRP:
      ss << "CopyiRP";
      ss << " r" << instruction.param1;
      ss << " p" << instruction.param2;
      break;
    case OP_CopybRP:
      ss << "CopybRP";
      ss << " r" << instruction.param1;
      ss << " p" << instruction.param2;
      break;

    case OP_CopyiPR:
      ss << "CopyiPR";
      ss << " p" << instruction.param1;
      ss << " r" << instruction.param2;
      break;
    case OP_CopybPR: 
      ss << "CopybPR";
      ss << " p" << instruction.param1;
      ss << " r" << instruction.param2;
      break;

    case OP_CopyiPL:
      ss << "CopyiPL";
      ss << " p" << instruction.param1;
      ss << " l" << instruction.param2;
      break;
    case OP_CopybPL:
      ss << "CopybPL";
      ss << " p" << instruction.param1;
      ss << " l" << instruction.param2;
      break;

    default:
      ss << "???";
      break;
    }

    ss << "\n";
  };


  std::stringstream ss;

  INT stackDepth = 0;

  for(auto &funcName : globalFunctionNames)
  {

    FunctionBytecode *funcBcode = functionBytecodes[funcName.second];
    ss << "\n\n";
    for( size_t i = 0; i < funcBcode->optimizedInstructions.size(); ++i  )
    {
      if(lineNumbers)
      {
        if(i < 10)
          ss << i << "   ";
        else if(i < 100)
          ss << i << "  ";
        else if(i < 1000)
          ss << i << " ";
      }
      GetInstructionString(ss, stackDepth, funcBcode->optimizedInstructions[i]);

    }

  }

  str = ss.str();

}
