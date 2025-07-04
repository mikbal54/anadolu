#pragma once

#include "Parser/PrimitiveTypes.h"

enum OpCode
{
  OP_NoOp,

  OP_BStart, // starts a block. Single parameter block number in parent block


  // allocates local stack. allocates registers.
  // p1: size of locals
  // p2: size of registers
  // p3: size of return register
  OP_AllocL,
  OP_DAllocL, // deallocates locals and registers

  OP_BEnd, // ends a block. Single parameter block number in parent block

  // reset register contents to 0
  OP_ResetR,

  // JUMPS
  // jump 
  // p1: register number
  // p2: number of instructions to be jumped over if true
  // p3: number of instructions to be jumped over if false
  // e.g: JumpbR 0 0 5 => do not execute next 0 instructions if register0 is true, do not exucute next 5 instructions if r0 is false
  // negative parameters work differently. eg. JumpbR 0 0 -7 , this will jump over back 5 instructions.
  // i did this because i do not want to extra canculations in each jmp instruction
  // this instruction also clears value of given instruction
  OP_JumpbR, 

  // jump without any conditions
  OP_Jump,


  // allocates parameter array
  // after OPCallPrep there are SetPL, SetPR calls. these set parameter values
  // p1: register number, address of the allocated memory
  // p2: bytes needed for parameters
  OP_CallPrep,

  // call a function or a few instructions in a far place. Both start and end instructions are executed.
  // p1: function id
  // p2: register number to be used as return address 
  // p3: register number, contains address of the parameter memory
  OP_Call,

  // Recover from function call. unallocs memory reserver for parameters.
  // comes after OP_CallPrep and OP_Call
  OP_CallUnprep,


  // copies data in param3 register to address stored in register1. param2 is the offset
  // **((p1 + p2) = *p3
  // this copies 4 bytes
  // p1: register, inside this register is a memory address
  // p2 offset, integer number. will start copying from (*r1) + offset
  OP_CopyData4ROR, 
  OP_CopyData1ROR, // 1 byte variant
  OP_CopyData8ROR, // 8 byte variant

  // get parameter value
  // p1: local index
  // p2: index of parameter in parameter array
  OP_CopyiLP, 
  OP_CopybLP, 

  // get parameter value
  // p1: register index
  // p2: index of parameter in parameter array
  OP_CopyiRP, 
  OP_CopybRP,

  // set parameter value
  // p1: index of parameter in parameter array
  // p2: register number
  OP_CopyiPR, 
  OP_CopybPR, 

  // set parameter value
  // p1: index of parameter in parameter array
  // p2: local INT index
  OP_CopyiPL, 
  OP_CopybPL, 

  // Divide operators

  OP_DiviRLR,
  OP_DiviRRL,
  OP_DiviRRC,
  OP_DiviRCR,
  OP_DiviRLL,
  OP_DiviRLC,
  OP_DiviRCL,
  OP_DiviRRR,


  // not operator p1 = !p2
  OP_NotbRR,

  // MULTIPLY operators

  OP_MuliPC,
  OP_MuliRP,
  OP_MuliLP,
  OP_MuliRPR,
  OP_MuliRPC,
  OP_MuliRPL,
  OP_MuliRPP,

  // multiply a register with another p1 *= p2
  // p1: target register
  // p2: register
  OP_MuliRR,

  // multiply a register with a local p1 *= p2
  // p1: target register
  // p2: local address
  OP_MuliRL,

  // multiply two local ints put result in register
  // p1: target register
  // p2: local INT
  // p3: local INT
  OP_MuliRLL,

  // multiple local INT with const INT put result INT register p1 = p2 * p3
  // p1: target register
  // p2: local INT
  // p3: constant INT 
  OP_MuliRLC,

  // multiply a register with a INT constant
  // p1: register numner
  // p2: const INT
  OP_MuliRC,

  // multiply a local INT with another INT constant
  // p1: local address
  // p2: const INT
  OP_MuliLC, 

  OP_AddiPR,
  OP_AddiPC,
  OP_AddiRP,
  OP_AddiRPL,
  OP_AddiRPR,
  OP_AddiRPC,
  OP_AddiRPP,

  // add local to register p1 += p2
  // p1: target register
  // p2: local address
  OP_AddiRL,

  // add two registers put result in target
  // p1: target register
  // p2: first register operand
  // p3: second register operand
  OP_AddiRRR,

  // add local INT and a register INT to a register
  // p1: target address
  // p2: local INT address
  // p3: register address
  OP_AddiRLR,

  // add two local values put it in a register
  // p1: target register
  // p2: local INT address
  // p3: local INT address
  OP_AddiRLL,

  // add INT constant and local value. place result in target register
  // p1: target register
  // p2: INT local address
  // p3: const INT
  OP_AddiRLC,

  // add INT constant and register value. place result in target register
  // p1: target register
  // p2: INT register argument
  // p3: const INT
  OP_AddiRRC,

  // add register to local
  // p1: local address
  // p2: register number that will be added to local
  OP_AddiLR,

  // add register to register
  // p1: target register number
  // p2: register that will be added to target register
  OP_AddiRR, // add one register value to another

  // add constant INT to local
  // p1: local target address
  // p2: const integer to be added
  OP_AddiLC, // add single INT to a local


  // add constant INT to register
  // p1: register number
  // p2: constant number
  OP_AddiRC, 


  //SUBTRACT OPERATORS. order of arguments matter
  OP_SubiRP,
  OP_SubiPP,
  OP_SubiPC,
  OP_SubiPL,
  OP_SubiRRP,
  OP_SubiRPR,
  OP_SubiRLP,
  OP_SubiRPL,
  OP_SubiRPC,
  OP_SubiRCP,
  OP_SubiRPP,

  // subtract a local from a local (p1 = p2 - p3)
  // p1: register
  // p2: local INT
  // p3: local INT
  OP_SubiRLL,

  // subtract a local from a const, put result in register (p1 = p2 - p3)
  // p1: target register
  // p2: const INT
  // p3: local INT
  OP_SubiRCL,

  // subtract const INT from a local INT ( p1 = p2 - p3 )
  // p1: target register
  // p2: local INT
  // p3: const INT
  OP_SubiRLC, 

  // subtract a register from another register put result in another register
  // p1: target register
  // p2: left hand register
  // p3: right hand register
  OP_SubiRRR,

  // subtract a register from a local
  // p1: target register
  // p2: local INT
  // p3: register 
  OP_SubiRLR,

  // subtract a local INT from a register
  // p1: target address
  // p2: register
  // p3: local INT
  OP_SubiRRL,

  // subtract a register from a constant INT,  put result in register
  // p1: target register
  // p2: constant INT
  // p3: register
  OP_SubiRCR,

  // subtract constant INT from a register, put result in register ( p1 = p2 - p3 )
  // p1: target register
  // p2: register 
  // p3: constant INT
  OP_SubiRRC,

  // subtract constant INT from a register ( p1 -= p2 )
  // register
  // constant INT
  OP_SubiRC,

  // subtract a register from a register ( p1 -= p2 )
  // p1: register
  // p2: register
  OP_SubiRR,

  // subtract a local from a register ( p1 -= p2  )
  // p1: register
  // p2: local
  OP_SubiRL,

  // subtract a register from local INT ( p1 -= p2 )
  // p1: local INT
  // p2: register
  OP_SubiLR,

  // move constant INT into a register
  // p1: target register
  // p2: const INT value
  OP_CopyiRC,

  // move one register to another
  // p1: target register
  // p2: register that will be moved to target register
  OP_CopyiRR,

  // move local INT to register ( moves in real life)
  // p1: target register
  // p2: local address that will be moved to target register
  OP_CopyiRL,

  // move register to local
  // p1: target local address
  // register to be moved in local address
  OP_CopyiLR,

  // move register to return register
  // p1: register number
  OP_CopyiXR, 

  // move bool register to another bool register
  // p1: target
  // p2: register address
  OP_CopybRR,

  // move register to local 
  // p1: target local address
  // p2: register to be moved in local address
  OP_CopybLR, 

  // move constant bool value to a register
  // p1: target register
  // p2: const bool value
  OP_CopybRC,

  // move local bool to a register
  // p1: target register
  // p2: local bool address
  OP_CopybRL,



  OP_CmpbRPP,
  OP_CmpbRPL,
  OP_CmpbRPR,
  OP_CmpbRPC,
  OP_CmpiRPP,
  OP_CmpiRPL,
  OP_CmpiRPR,
  OP_CmpiRPC,
  OP_CmpbRC,

  // move local bool to another local bool
  // p1: target register
  // p2: local bool address
  // p3: local bool address
  OP_CmpbRLL,

  // compare two registers put result in target register
  // p1: target register
  // p2: first register to compared
  // p3: second register to compared
  OP_CmpbRRR, 

  // compare local bool and const bool
  // p1: target address
  // p2: local bool address
  // p3: const bool value
  OP_CmpbRLC,

  // compare constant bool and register
  // p1: target register
  // p2: constant bool to be compared
  // p3: register to be compared
  OP_CmpbRCR,

  // compare local bool to register
  // p1: target register
  // p2: local address of the bool
  // p3: register to be compared against local bool
  OP_CmpbRLR,

  // compare to local ints
  // p1: target register
  // p2: local INT address
  // p3: loca INT address
  OP_CmpiRLL, 

  // compare local INT and const INT
  // p1: target register
  // p2: local INT address
  // p3: const INT values
  OP_CmpiRLC,

  // compare register with register put result in target register
  // p1: target register
  // p2: register argument 1
  // p3: register argument 2
  OP_CmpiRRR,

  // compare constant INT with register 
  // p1: target register
  // p2: const INT value
  // p3: register argumen
  OP_CmpiRCR, 

  // compare local INT with register
  // p1: target register
  // p2: register argument
  // p3: local INT address 
  OP_CmpiRLR,

  OP_Return,

  // ABOVE printing


  OP_DiviRP,
  OP_DiviLP,
  OP_DiviPP,
  OP_DiviPC,
  OP_DiviPR,
  OP_DiviPL,
  OP_DiviRPC,
  OP_DiviRPL,
  OP_DiviRPP,
  OP_DiviRPR,
  OP_DiviRCP,
  OP_DiviRLP,
  OP_DiviRRP,

  // ABOVE executing


};

class Instruction
{
public:

  OpCode opCode;

  INT32 param1;
  INT32 param2;
  INT32 param3;

  Instruction()
  {
    opCode = OP_NoOp;
    param1 = 0;
    param2 = 0;
    param3 = 0;
  }

  Instruction(OpCode _opCode, INT32 _param1 = 0, INT32 _param2 = 0, INT32 _param3 = 0) : opCode(_opCode), param1(_param1), param2(_param2), param3(_param3) {}

};