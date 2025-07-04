#include "BytecodeGenerator.h"
#include "Parser/PackageParser.h"
#include "Parser/Package.h"
#include "Bytecode.h"
#include "Parser/Node.h"
#include "Parser/PrimitiveTypes.h"

#include <assert.h>

void BytecodeGenerator::GenerateFunctionCall(std::list<Instruction> &instructions, INT32 returnRegister, Designator *designator, Function *function)
{

  if(designator->expressions)
  {
    INT32 sizeOfParams = designator->function->parameterSize;
    INT32 parameterRegister = GetAvailableRegister();
    instructions.emplace_back(OP_CallPrep, parameterRegister, sizeOfParams);

    INT32 currentOffset = 0;
    for(Expression *expr : *designator->expressions)
    {
      INT32 exprResult = GenerateExpression(instructions, expr, function);

      switch(expr->returnTypeId)
      {
      case TypeIdInteger:
        instructions.emplace_back(OP_CopyData4ROR, parameterRegister, currentOffset, exprResult);
        break;
      case TypeIdBool:
        instructions.emplace_back(OP_CopyData1ROR, parameterRegister, currentOffset, exprResult);
        break;
      default:
        assert(0); // TODO: other types!
      }

      currentOffset += function->package->GetSizeOf(expr->returnTypeId);
    }

    instructions.emplace_back(OP_Call, designator->function->id, returnRegister, parameterRegister);
    DoneWithTheRegister(instructions, parameterRegister);
  }
  else
    instructions.emplace_back( OP_Call, designator->function->id, returnRegister);


}

BytecodeGenerator::BytecodeGenerator()
{
  hasErrors = false;
  maxRegisterNumber = 0;

  ReleaseAllRegisters();
}

void BytecodeGenerator::Error(const std::string &msg) 
{
  hasErrors = true;
  outputFunction(msg, 0, 0, 1);
}

void BytecodeGenerator::DoneWithTheRegister(INT32 registerNumber)
{
  // register should not be available if we return it
  if(availableRegisters.count(registerNumber))
    assert(0);

  // clear register
  availableRegisters.insert(registerNumber);
}

void BytecodeGenerator::DoneWithTheRegister(std::list<Instruction> &instruction, INT32 registerNumber)
{
  // register should not be available if we return it
  if(availableRegisters.count(registerNumber))
    assert(0);

  // clear register

  // TODO: this clear can be optimized away. 
  // If next writing operator uses a move on this register remove this 
  instruction.emplace_back(OP_ResetR, registerNumber);
  availableRegisters.insert(registerNumber);
}

void BytecodeGenerator::DivideOperator(std::list<Instruction> &instructions, std::list<ExpressionValue> &values)
{
  ExpressionValue righthand = values.back(); values.pop_back();
  ExpressionValue lefthand = values.back(); values.pop_back();

  if(lefthand.type == EVT_ConstInt)
  {
    if(righthand.type == EVT_ConstInt)
    {
      INT32 result;
      if(righthand.intValue == 0)
      {
        result = 0;
        Error("Trying to divide by zero!");
      }
      else
        result = lefthand.intValue / righthand.intValue;

      values.emplace_back(EVT_ConstInt, result);
    }
    else if(righthand.type == EVT_Designator)
    {
      if(righthand.stringValue->typeId == TypeIdInteger)
      {
        INT32 newRegister = GetAvailableRegister();

        if(righthand.stringValue->type == DT_LocalValue)
          instructions.emplace_back(OP_DiviRCL, newRegister, lefthand.intValue, righthand.stringValue->address);
        else if(righthand.stringValue->type == DT_ParameterValue)
          instructions.emplace_back(OP_DiviRCP, newRegister, lefthand.intValue, righthand.stringValue->address);
        else
          assert(0);

        values.emplace_back(EVT_ConstInt, newRegister);
      }
      else
        assert(0);
    }
    else if(righthand.type == EVT_RegisterInt)
    {
      instructions.emplace_back(OP_DiviRCR, righthand.intValue, lefthand.intValue, righthand.intValue);
      values.emplace_back(EVT_RegisterInt, righthand.intValue);
    }
    else
      assert(0);
  }
  else if(lefthand.type == EVT_Designator)
  {
    if(lefthand.stringValue->typeId == TypeIdInteger)
    {
      if(righthand.type == EVT_ConstInt)
      {
        if(lefthand.intValue == 0)
          Error("Divide by zero!");

        INT32 newRegister = GetAvailableRegister();
        instructions.emplace_back(OP_DiviRLC, newRegister, lefthand.stringValue->address, righthand.intValue);
        values.emplace_back(EVT_RegisterInt, newRegister);
      }
      else if(righthand.type == EVT_Designator)
      {
        INT32 newRegister = GetAvailableRegister();

        if(lefthand.stringValue->type == DT_LocalValue)
        {
          if(righthand.stringValue->type == DT_LocalValue)
            instructions.emplace_back(OP_DiviRLL, newRegister, lefthand.stringValue->address, righthand.stringValue->address);
          else if(righthand.stringValue->type == DT_ParameterValue)
            instructions.emplace_back(OP_DiviRLP, newRegister, lefthand.stringValue->address, righthand.stringValue->address);
          else
            assert(0);
        }
        else if(lefthand.stringValue->type == DT_ParameterValue)
        {
          if(righthand.stringValue->type == DT_LocalValue)
            instructions.emplace_back(OP_DiviRPL, newRegister, lefthand.stringValue->address, righthand.stringValue->address);
          else if(righthand.stringValue->type == DT_ParameterValue)
            instructions.emplace_back(OP_DiviRPP, newRegister, lefthand.stringValue->address, righthand.stringValue->address);
          else
            assert(0);
        }
        else
          assert(0);
        values.emplace_back(EVT_RegisterInt, newRegister);
      }
      else if(righthand.type == EVT_RegisterInt )
      {
        instructions.emplace_back(OP_DiviRLR, righthand.intValue, lefthand.stringValue->address, righthand.intValue);
        values.emplace_back(EVT_RegisterInt, righthand.intValue);
      }
      else
        assert(0);
    }
    else
      assert(0);
  }
  else if(lefthand.type == EVT_RegisterInt)
  {
    if(righthand.type == EVT_ConstInt)
    {
      if(righthand.intValue == 0)
        Error("Divide by zero!");

      instructions.emplace_back(OP_DiviRRC, lefthand.intValue, lefthand.intValue, righthand.intValue);
      values.emplace_back(EVT_RegisterInt, lefthand.intValue);
    }
    else if(righthand.type == EVT_Designator)
    {
      if(righthand.stringValue->typeId == TypeIdInteger)
      {
        if(righthand.stringValue->type == DT_LocalValue)
          instructions.emplace_back(OP_DiviRRL, lefthand.intValue, lefthand.intValue, righthand.stringValue->address);
        else if(righthand.stringValue->type == DT_ParameterValue)
          instructions.emplace_back(OP_DiviRRP, lefthand.intValue, lefthand.intValue, righthand.stringValue->address);
        else
          assert(0);
        values.emplace_back(EVT_RegisterInt, lefthand.intValue);
      }
      else
        assert(0);
    }
    else if(righthand.type == EVT_RegisterInt)
    {
      instructions.emplace_back(OP_DiviRRR, lefthand.intValue, lefthand.intValue, righthand.intValue);
      values.emplace_back(EVT_RegisterInt, lefthand.intValue);
      DoneWithTheRegister(instructions, righthand.intValue);
    }
    else
      assert(0);

  }
  else
    assert(0);
}

void BytecodeGenerator::MultiplyOperator(std::list<Instruction> &instructions, std::list<ExpressionValue> &values)
{
  ExpressionValue righthand = values.back(); values.pop_back();
  ExpressionValue lefthand = values.back(); values.pop_back();


  if(lefthand.type == EVT_ConstInt)
  {
    if(righthand.type == EVT_ConstInt)
    {
      INT32 value = lefthand.intValue * righthand.intValue;
      values.emplace_back(EVT_ConstInt, value);
    }
    else if(righthand.type == EVT_Designator)
    {
      INT32 newRegister = GetAvailableRegister();

      if(righthand.stringValue->type == DT_LocalValue)
        instructions.emplace_back(OP_MuliRLC, newRegister, righthand.stringValue->address, lefthand.intValue);
      else if(righthand.stringValue->type == DT_ParameterValue)
        instructions.emplace_back(OP_MuliRPC, newRegister, righthand.stringValue->address, lefthand.intValue);
      else
        assert(0);

      values.emplace_back(EVT_RegisterInt, newRegister);
    }
    else if(righthand.type == EVT_RegisterInt)
    {
      // reuse the right hand register
      instructions.emplace_back(OP_MuliRC, righthand.intValue, lefthand.intValue);
      values.emplace_back(EVT_RegisterInt, righthand.intValue);
    }
    else 
      assert(0);
  }
  else if(lefthand.type == EVT_Designator)
  {
    if(righthand.type == EVT_ConstInt)
    {
      INT32 newRegister = GetAvailableRegister();

      if(lefthand.stringValue->type == DT_LocalValue)
        instructions.emplace_back(OP_MuliRLC, newRegister, lefthand.stringValue->address, righthand.intValue);
      else if(lefthand.stringValue->type == DT_ParameterValue)
        instructions.emplace_back(OP_MuliRPC, newRegister, lefthand.stringValue->address, righthand.intValue);
      else
        assert(0);

      values.emplace_back(EVT_RegisterInt, newRegister);
    }
    else if(righthand.type == EVT_Designator)
    {
      INT32 newRegister = GetAvailableRegister();

      if(lefthand.stringValue->type == DT_LocalValue)
      {
        if(righthand.stringValue->type == DT_LocalValue)
          instructions.emplace_back(OP_MuliRLL, newRegister, lefthand.stringValue->address, righthand.stringValue->address);
        else if(righthand.stringValue->type == DT_ParameterValue)
          instructions.emplace_back(OP_MuliRPL, newRegister, righthand.stringValue->address, lefthand.stringValue->address);
        else
          assert(0);
      }
      else if(lefthand.stringValue->type == DT_ParameterValue)
      {
        if(righthand.stringValue->type == DT_LocalValue)
          instructions.emplace_back(OP_MuliRPL, newRegister, lefthand.stringValue->address, righthand.stringValue->address);
        else if(righthand.stringValue->type == DT_ParameterValue)
          instructions.emplace_back(OP_MuliRPP, newRegister, righthand.stringValue->address, lefthand.stringValue->address);
        else
          assert(0);
      }
      else
        assert(0);

      values.emplace_back(EVT_RegisterInt, newRegister);
    }
    else
      assert(0);

  }
  else if(lefthand.type == EVT_RegisterInt)
  {
    if(righthand.type == EVT_ConstInt)
    {
      // reuse right register
      instructions.emplace_back(OP_MuliRC, lefthand.intValue, righthand.intValue);
      values.emplace_back(EVT_RegisterInt, lefthand.intValue);
    }
    else if( righthand.type == EVT_Designator )
    {
      //reuse left register

      if(righthand.stringValue->type == DT_LocalValue)
        instructions.emplace_back(OP_MuliRL, lefthand.intValue, righthand.stringValue->address);
      else if(righthand.stringValue->type == DT_ParameterValue)
        instructions.emplace_back(OP_MuliRP, lefthand.intValue, righthand.stringValue->address);
      else
        assert(0);

      values.emplace_back(EVT_RegisterInt, lefthand.intValue);
    }
    else if( righthand.type == EVT_RegisterInt )
    {
      // reuse left INT but return right register
      instructions.emplace_back(OP_MuliRR, lefthand.intValue, righthand.intValue);
      values.emplace_back(EVT_RegisterInt, lefthand.intValue);
      DoneWithTheRegister(instructions, righthand.intValue);
    }
    else
      assert(0);
  }
  else
    assert(0);
}

void BytecodeGenerator::NegateOperator(std::list<Instruction> &instructions, std::list<ExpressionValue> &values)
{
  ExpressionValue lefthand = values.back(); values.pop_back();

  if(lefthand.type == EVT_ConstInt)
  {
    INT32 value = -lefthand.intValue;
    values.emplace_back(EVT_ConstInt, value);
  }
  else if(lefthand.type == EVT_RegisterInt)
  {
    instructions.emplace_back(OP_MuliRC, lefthand.intValue, -1);
    values.emplace_back(EVT_RegisterInt, lefthand.intValue); // push back to expression values 
  }
  else if(lefthand.type == EVT_Designator)
  {
    INT32 newRegister = GetAvailableRegister();

    if(lefthand.stringValue->type == DT_LocalValue)
      instructions.emplace_back(OP_MuliRLC, newRegister, lefthand.stringValue->address, -1);
    else if(lefthand.stringValue->type == DT_ParameterValue)
      instructions.emplace_back(OP_MuliRPC, newRegister, lefthand.stringValue->address, -1);
    else
      assert(0);

    values.emplace_back(EVT_RegisterInt, newRegister);
  }
  else
    assert(0);

}

void BytecodeGenerator::MinusOperator(std::list<Instruction> &instructions, std::list<ExpressionValue> &values)
{
  ExpressionValue righthand = values.back(); values.pop_back();
  ExpressionValue lefthand = values.back(); values.pop_back();

  if(lefthand.type == EVT_ConstInt)
  {
    if(righthand.type == EVT_ConstInt)
    {
      INT32 result = lefthand.intValue - righthand.intValue;
      values.emplace_back(EVT_ConstInt, result);
    }
    else if(righthand.type == EVT_Designator)
    {
      if(righthand.stringValue->typeId == TypeIdInteger)
      {
        INT32 newRegister = GetAvailableRegister();

        if(righthand.stringValue->type == DT_LocalValue)
          instructions.emplace_back(OP_SubiRCL, newRegister, lefthand.intValue, righthand.stringValue->address);
        else if(righthand.stringValue->type == DT_ParameterValue)
          instructions.emplace_back(OP_SubiRCP, newRegister, lefthand.intValue, righthand.stringValue->address);
        else
          assert(0);

        values.emplace_back(EVT_RegisterInt, newRegister);
      }
      else
        assert(0); // TODO: handle object - operator
    }
    else if(righthand.type == EVT_RegisterInt)
    {
      // reuse register
      instructions.emplace_back(OP_SubiRCR, righthand.intValue, lefthand.intValue, righthand.intValue);
      values.emplace_back(EVT_RegisterInt, righthand.intValue);
    }
    else
      assert(0);
  }
  else if(lefthand.type == EVT_Designator)
  {
    if(righthand.type == EVT_ConstInt)
    {
      INT32 newRegister = GetAvailableRegister();

      if(lefthand.stringValue->type == DT_LocalValue)
        instructions.emplace_back(OP_SubiRLC, newRegister, lefthand.stringValue->address, righthand.intValue);
      else if(lefthand.stringValue->type == DT_ParameterValue)
        instructions.emplace_back(OP_SubiRPC, newRegister, lefthand.stringValue->address, righthand.intValue);
      else
        assert(0);

      values.emplace_back(EVT_RegisterInt, newRegister);
    }
    else if(righthand.type == EVT_Designator)
    {
      if(righthand.stringValue->typeId == TypeIdInteger)
      {

        INT32 newRegister = GetAvailableRegister();

        if(lefthand.stringValue->type == DT_LocalValue)
        {
          if(righthand.stringValue->type == DT_LocalValue)
            instructions.emplace_back(OP_SubiRLL, newRegister, lefthand.stringValue->address, righthand.stringValue->address);
          else if(righthand.stringValue->type == DT_ParameterValue)
            instructions.emplace_back(OP_SubiRLP, newRegister, lefthand.stringValue->address, righthand.stringValue->address);
          else
            assert(0);
        }
        else if(lefthand.stringValue->type == DT_ParameterValue)
        {
          if(righthand.stringValue->type == DT_LocalValue)
            instructions.emplace_back(OP_SubiRPL, newRegister, lefthand.stringValue->address, righthand.stringValue->address);
          else if(righthand.stringValue->type == DT_ParameterValue)
            instructions.emplace_back(OP_SubiRPP, newRegister, lefthand.stringValue->address, righthand.stringValue->address);
          else
            assert(0);
        }
        else
          assert(0);

        values.emplace_back(EVT_RegisterInt, newRegister);

      }
      else
        assert(0);
    }
    else if(righthand.type == EVT_RegisterInt)
    {
      instructions.emplace_back(OP_SubiRLR, righthand.intValue, lefthand.stringValue->address, righthand.intValue);
      values.emplace_back(EVT_RegisterInt, righthand.intValue);
    }
    else
      assert(0);
  }
  else if(lefthand.type == EVT_RegisterInt)
  {
    if(righthand.type == EVT_ConstInt)
    {
      instructions.emplace_back(OP_SubiRRC, lefthand.intValue, lefthand.intValue, righthand.intValue);
      values.emplace_back(EVT_RegisterInt, lefthand.intValue);
    }
    else if(righthand.type == EVT_Designator)
    {
      if(righthand.stringValue->typeId == TypeIdInteger)
      {
        if(righthand.stringValue->type == DT_LocalValue)
          instructions.emplace_back(OP_SubiRRL, lefthand.intValue, lefthand.intValue, righthand.stringValue->address);
        else if(righthand.stringValue->type == DT_ParameterValue)
          instructions.emplace_back(OP_SubiRRP, lefthand.intValue, lefthand.intValue, righthand.stringValue->address);
        else
          assert(0);
        values.emplace_back(EVT_RegisterInt, lefthand.intValue);
      }
      else
        assert(0);
    }
    else if(righthand.type == EVT_RegisterInt)
    {
      //instead of using a new register we put result in already available right register
      instructions.emplace_back(OP_SubiRR, lefthand.intValue, righthand.intValue);
      values.emplace_back(EVT_RegisterInt, lefthand.intValue);
      // right register is not needed anymore
      DoneWithTheRegister(instructions, righthand.intValue);
    }
    else
      assert(0);

  }
  else 
    assert(0);


}

void BytecodeGenerator::PlusOperator(std::list<Instruction> &instructions, std::list<ExpressionValue> &values)
{
  ExpressionValue righthand = values.back(); values.pop_back();
  ExpressionValue lefthand = values.back(); values.pop_back();


  if(lefthand.type == EVT_ConstInt)
  {
    if(righthand.type == EVT_ConstInt)
    {
      // both hands are constant ints
      INT32 result = lefthand.intValue + righthand.intValue;
      values.emplace_back(EVT_ConstInt, result);
    }
    else if(righthand.type == EVT_RegisterInt)
    {
      instructions.emplace_back(OP_AddiRC, righthand.intValue, lefthand.intValue);
      values.emplace_back(EVT_RegisterInt, righthand.intValue);
    }
    else if(righthand.type == EVT_Designator)
    {
      //left is constant INT right is a designator
      if(righthand.stringValue->typeId == TypeIdInteger)
      {
        INT32 newRegister = GetAvailableRegister();

        if(righthand.stringValue->type == DT_LocalValue)
          instructions.emplace_back(OP_AddiRLC, newRegister, righthand.stringValue->address, lefthand.intValue);
        else if(righthand.stringValue->type == DT_ParameterValue)
          instructions.emplace_back(OP_AddiRPC, newRegister, righthand.stringValue->address, lefthand.intValue);
        else 
          assert(0); // Function call

        values.emplace_back(EVT_RegisterInt, newRegister);
      }
      else
        assert(0);// handle object INT conversion
    }
    else 
      assert(0);
  }
  else if(lefthand.type == EVT_Designator)
  {
    if(righthand.type == EVT_ConstInt)
    {
      INT32 newRegister = GetAvailableRegister();
      if(lefthand.stringValue->type == DT_ParameterValue)
        instructions.emplace_back(OP_AddiRPC, newRegister, lefthand.stringValue->address, righthand.intValue);
      else if(lefthand.stringValue->type == DT_LocalValue)
        instructions.emplace_back(OP_AddiRLC, newRegister, lefthand.stringValue->address, righthand.intValue);
      else
        assert(0);
      values.emplace_back(EVT_RegisterInt, newRegister);
    }
    else if(righthand.type == EVT_Designator)
    {
      if(lefthand.stringValue->type == DT_ParameterValue)
      {
        INT32 newRegister = GetAvailableRegister();
        if(righthand.stringValue->type == DT_ParameterValue)
          instructions.emplace_back(OP_AddiRPP, newRegister, lefthand.stringValue->address, righthand.stringValue->address);
        else if(righthand.stringValue->type == DT_LocalValue)
          instructions.emplace_back(OP_AddiRPL, newRegister, lefthand.stringValue->address, righthand.stringValue->address);
        else
          assert(0);

        values.emplace_back(EVT_RegisterInt, newRegister);
      }
      else if(lefthand.stringValue->type == DT_LocalValue)
      {
        INT32 newRegister = GetAvailableRegister();
        if(righthand.stringValue->type == DT_ParameterValue)
          instructions.emplace_back(OP_AddiRPL, newRegister, lefthand.stringValue->address, righthand.stringValue->address);
        else if(righthand.stringValue->type == DT_LocalValue)
          instructions.emplace_back(OP_AddiRLL, newRegister, lefthand.stringValue->address, righthand.stringValue->address);
        else
          assert(0);

        values.emplace_back(EVT_RegisterInt, newRegister);
      }
      else if(lefthand.stringValue->type == DT_FunctionCall)
      {
        INT32 result = GetAvailableRegister();
        INT32 lhandResult = GetAvailableRegister();
        INT32 rhandResult = GetAvailableRegister();
        GenerateFunctionCall(instructions, lhandResult, lefthand.stringValue, lefthand.stringValue->function);
        GenerateFunctionCall(instructions, rhandResult, righthand.stringValue, righthand.stringValue->function);

        instructions.emplace_back(OP_AddiRRR, result, lhandResult, rhandResult);
        values.emplace_back(EVT_RegisterInt, result);
        DoneWithTheRegister(instructions, lhandResult);
        DoneWithTheRegister(instructions, rhandResult);
      }
      else
        assert(0);
    }
    else if(righthand.type == EVT_RegisterInt)
    {
      if(lefthand.stringValue->type == DT_ParameterValue)
        instructions.emplace_back(OP_AddiRP, righthand.intValue, lefthand.stringValue->address);
      else if(lefthand.stringValue->type == DT_LocalValue)
        instructions.emplace_back(OP_AddiRL, righthand.intValue, lefthand.stringValue->address);
      else
        assert(0);
      values.emplace_back(EVT_RegisterInt, righthand.intValue);
    }
    else 
      assert(0);
  }
  else if(lefthand.type == EVT_RegisterInt)
  {
    if(righthand.type == EVT_ConstInt)
    {
      instructions.emplace_back(OP_AddiRC, lefthand.intValue, righthand.intValue);
      values.emplace_back(EVT_RegisterInt, lefthand.intValue);
    }
    else if(righthand.type == EVT_Designator)
    {
      if(righthand.stringValue->typeId == TypeIdInteger)
      {
        if(righthand.stringValue->type == DT_ParameterValue)
          instructions.emplace_back(OP_AddiRP, lefthand.intValue, righthand.stringValue->address);
        else if(righthand.stringValue->type == DT_LocalValue)
          instructions.emplace_back(OP_AddiRL, lefthand.intValue, righthand.stringValue->address);
        else
          assert(0);

        values.emplace_back(EVT_RegisterInt, lefthand.intValue);
      }
      else
        assert(0); // handle object + operator
    }
    else if(righthand.type == EVT_RegisterInt)
    {
      instructions.emplace_back(OP_AddiRR, lefthand.intValue, righthand.intValue);
      values.emplace_back(EVT_RegisterInt, lefthand.intValue);
      DoneWithTheRegister(instructions, righthand.intValue);
    }
    else
      assert(0);

  }
  else
    assert(0); 
}

void BytecodeGenerator::NotEqualOperator(std::list<Instruction> &instructions, std::list<ExpressionValue> &values)
{
  // first generate a CMP operator
  // then 'not' its value
  EqualsOperator(instructions, values);

  ExpressionValue lastValue =  values.back();
  values.pop_back();

  if(lastValue.type == EVT_RegisterBool)
  {
    instructions.emplace_back(OP_NotbRR, lastValue.intValue, lastValue.intValue);
    values.emplace_back(EVT_RegisterBool, lastValue.intValue);
  }
  else if(lastValue.type == EVT_ConstBool)
  {
    values.emplace_back(EVT_ConstBool, !lastValue.intValue);
  }
  else
    assert(0);
}

void BytecodeGenerator::EqualsOperator(std::list<Instruction> &instructions, std::list<ExpressionValue> &values)
{
  ExpressionValue righthand = values.back(); values.pop_back();
  ExpressionValue lefthand = values.back(); values.pop_back();

  // there are many different operation combinations

  // left is a bool
  if(lefthand.type == EVT_ConstBool)
  {

    // right is also const bool
    if(righthand.type == EVT_ConstBool)
    {
      if(lefthand.intValue == righthand.intValue)
        values.emplace_back(EVT_ConstBool, 1);
      else
        values.emplace_back(EVT_ConstBool, 0);
    }
    else if(righthand.type == EVT_Designator) // local/heap
    {
      // figure right hands type
      if(righthand.stringValue->typeId == TypeIdBool)
      {
        INT32 registerToBeUsed = GetAvailableRegister();

        if(righthand.stringValue->type == DT_ParameterValue)
          instructions.emplace_back(OP_CmpbRPC, registerToBeUsed, righthand.stringValue->address, lefthand.intValue);
        else if(righthand.stringValue->type == DT_LocalValue)
          instructions.emplace_back(OP_CmpbRLC, registerToBeUsed, righthand.stringValue->address, lefthand.intValue);
        else
          assert(0);

        values.emplace_back(EVT_RegisterBool, registerToBeUsed);
      }
      else
        assert(0); // object bool operators not here yet

    }
    else if(righthand.type == EVT_RegisterBool)
    {
      instructions.emplace_back(OP_CmpbRCR, righthand.intValue, lefthand.intValue, righthand.intValue);
      values.emplace_back(EVT_RegisterBool, righthand.intValue);
    }
    else
      assert(0);

  }
  else if( lefthand.type == EVT_ConstInt )
  {

    if(righthand.type == EVT_ConstInt)
    {
      // both are constant, no need for instructions
      if(lefthand.intValue == righthand.intValue)
        values.emplace_back(EVT_ConstBool, 1);
      else
        values.emplace_back(EVT_ConstBool, 0);
    }
    else if(righthand.type == EVT_RegisterInt)
    {
      instructions.emplace_back(OP_CmpbRC, righthand.intValue, righthand.intValue, lefthand.intValue);
      values.emplace_back(EVT_RegisterBool, righthand.intValue);
    }
    else if(righthand.type == EVT_Designator)
    {
      INT32 registerToBeUsed = GetAvailableRegister();
      if(righthand.stringValue->type == DT_ParameterValue)
        instructions.emplace_back(OP_CmpbRPC, registerToBeUsed, righthand.stringValue->address, lefthand.intValue);
      else if(righthand.stringValue->type == DT_LocalValue)
        instructions.emplace_back(OP_CmpbRLC, registerToBeUsed, righthand.stringValue->address, lefthand.intValue);
      else
        assert(0);
      values.emplace_back(EVT_RegisterBool, registerToBeUsed);
    }
    else
      assert(0);
  }
  else if(lefthand.type == EVT_Designator)
  {
    //TODO: heap
    if(righthand.type == EVT_ConstBool)
    {
      INT32 registerToBeUsed = GetAvailableRegister();

      if(lefthand.stringValue->type == DT_ParameterValue)
        instructions.emplace_back(OP_CmpbRPC, registerToBeUsed, lefthand.stringValue->address, righthand.intValue);
      else if(lefthand.stringValue->type == DT_LocalValue)
        instructions.emplace_back(OP_CmpbRLC, registerToBeUsed, lefthand.stringValue->address, righthand.intValue);
      else
        assert(0);

      values.emplace_back(EVT_RegisterBool, registerToBeUsed);
    }
    else if(righthand.type == EVT_ConstInt)
    {
      INT32 registerToBeUsed = GetAvailableRegister();

      if(lefthand.stringValue->type == DT_ParameterValue)
        instructions.emplace_back(OP_CmpiRPC, registerToBeUsed, lefthand.stringValue->address, righthand.intValue);
      else if(lefthand.stringValue->type == DT_LocalValue)
        instructions.emplace_back(OP_CmpiRLC, registerToBeUsed, lefthand.stringValue->address, righthand.intValue);
      else
        assert(0);

      values.emplace_back(EVT_RegisterBool, registerToBeUsed);
    }
    else if(righthand.type == EVT_RegisterBool)
    {
      if(lefthand.stringValue->type == DT_ParameterValue)
        instructions.emplace_back(OP_CmpbRPR, righthand.intValue, lefthand.stringValue->address, righthand.intValue);
      else if(lefthand.stringValue->type == DT_LocalValue)
        instructions.emplace_back(OP_CmpbRLR, righthand.intValue, lefthand.stringValue->address, righthand.intValue);
      else
        assert(0);
      values.emplace_back(EVT_RegisterBool, righthand.intValue);
    }
    else if(righthand.type == EVT_RegisterInt)
    {
      if(lefthand.stringValue->type == DT_ParameterValue)
        instructions.emplace_back(OP_CmpiRPR, righthand.intValue, lefthand.stringValue->address, righthand.intValue);
      else if(lefthand.stringValue->type == DT_LocalValue)
        instructions.emplace_back(OP_CmpiRLR, righthand.intValue, lefthand.stringValue->address, righthand.intValue);
      else
        assert(0);
      values.emplace_back(EVT_RegisterBool, righthand.intValue);
    }
    else if(righthand.type == EVT_Designator)
    {
      // both right and left hand are designators
      if(lefthand.stringValue->typeId == TypeIdBool)
      {

        INT32 registerToBeUsed = GetAvailableRegister();

        if(lefthand.stringValue->type == DT_LocalValue)
        {
          if(righthand.stringValue->type == DT_LocalValue)
            instructions.emplace_back(OP_CmpbRLL, registerToBeUsed, lefthand.stringValue->address, righthand.stringValue->address);
          else if(righthand.stringValue->type == DT_ParameterValue)
            instructions.emplace_back(OP_CmpbRPL, registerToBeUsed, righthand.stringValue->address, lefthand.stringValue->address);
          else
            assert(0);
        }
        else if(lefthand.stringValue->type == DT_ParameterValue)
        {
          if(righthand.stringValue->type == DT_LocalValue)
            instructions.emplace_back(OP_CmpbRPL, registerToBeUsed, lefthand.stringValue->address, righthand.stringValue->address);
          else if(righthand.stringValue->type == DT_ParameterValue)
            instructions.emplace_back(OP_CmpbRPP, registerToBeUsed, lefthand.stringValue->address, righthand.stringValue->address);
          else
            assert(0);
        }

        values.emplace_back(EVT_RegisterBool, registerToBeUsed);
      }
      else if(lefthand.stringValue->typeId == TypeIdInteger)
      {
        INT32 registerToBeUsed = GetAvailableRegister();

        if(lefthand.stringValue->type == DT_LocalValue)
        {
          if(righthand.stringValue->type == DT_LocalValue)
            instructions.emplace_back(OP_CmpiRLL, registerToBeUsed, lefthand.stringValue->address, righthand.stringValue->address);
          else if(righthand.stringValue->type == DT_ParameterValue)
            instructions.emplace_back(OP_CmpiRPL, registerToBeUsed, righthand.stringValue->address, lefthand.stringValue->address);
          else
            assert(0);
        }
        else if(lefthand.stringValue->type == DT_ParameterValue)
        {
          if(righthand.stringValue->type == DT_LocalValue)
            instructions.emplace_back(OP_CmpiRPL, registerToBeUsed, lefthand.stringValue->address, righthand.stringValue->address);
          else if(righthand.stringValue->type == DT_ParameterValue)
            instructions.emplace_back(OP_CmpiRPP, registerToBeUsed, lefthand.stringValue->address, righthand.stringValue->address);
          else
            assert(0);
        }

        values.emplace_back(EVT_RegisterBool, registerToBeUsed);
      }
      else
        assert(0);


    }
    else
      assert(0);
  }
  else if(lefthand.type == EVT_RegisterBool)
  {
    if(righthand.type == EVT_ConstBool)
    {
      instructions.emplace_back(OP_CmpbRCR, lefthand.intValue, righthand.intValue, lefthand.intValue);
      values.emplace_back(EVT_RegisterBool, lefthand.intValue);
    }
    else if(righthand.type == EVT_ConstInt)
    {
      instructions.emplace_back(OP_CmpiRCR, lefthand.intValue, righthand.intValue, lefthand.intValue);
      values.emplace_back(EVT_RegisterBool, lefthand.intValue);
    }
    else if(righthand.type == EVT_RegisterBool)
    {
      instructions.emplace_back(OP_CmpbRRR, lefthand.intValue, righthand.intValue, lefthand.intValue);
      values.emplace_back(EVT_RegisterBool, lefthand.intValue);
      DoneWithTheRegister(instructions,righthand.intValue);
    }
    else if(righthand.type == EVT_Designator)
    {
      // left if register right is designator
      if(righthand.stringValue->typeId == TypeIdBool)
      {
        if(righthand.stringValue->type == DT_LocalValue)
          instructions.emplace_back(OP_CmpbRLR, lefthand.intValue, righthand.stringValue->address, lefthand.intValue);
        else if(righthand.stringValue->type == DT_ParameterValue)
          instructions.emplace_back(OP_CmpbRPR, lefthand.intValue, righthand.stringValue->address, lefthand.intValue);
        else
          assert(0);
        values.emplace_back(EVT_RegisterBool, lefthand.intValue);
      }
      else
        assert(0);
    }
    else
      assert(0);

  }
  else if(lefthand.type == EVT_RegisterInt)
  {
    if(righthand.type == EVT_ConstInt)
    {
      instructions.emplace_back(OP_CmpiRCR, lefthand.intValue, righthand.intValue, lefthand.intValue);
      values.emplace_back(EVT_RegisterBool, lefthand.intValue);
    }
    else if(righthand.type == EVT_Designator)
    {
      if(righthand.stringValue->typeId == TypeIdInteger)
      {
        if(righthand.stringValue->type == DT_LocalValue)
          instructions.emplace_back(OP_CmpiRLR, lefthand.intValue, righthand.stringValue->address, lefthand.intValue);
        else if(righthand.stringValue->type == DT_ParameterValue)
          instructions.emplace_back(OP_CmpiRPR, lefthand.intValue, righthand.stringValue->address, lefthand.intValue);
        else
          assert(0);
        values.emplace_back(EVT_RegisterBool, lefthand.intValue);
      }
      else
        assert(0);
    }
    else if(righthand.type == EVT_RegisterInt)
    {
      instructions.emplace_back(OP_CmpiRRR, lefthand.intValue, lefthand.intValue, righthand.intValue);
      values.emplace_back(EVT_RegisterBool, lefthand.intValue);
      DoneWithTheRegister(instructions, righthand.intValue);
    }
    else
      assert(0);

  }
  else 
    assert(0); // handle INT compare here

}

void BytecodeGenerator::GenerateBytecode(std::list<Instruction> &instructions, Function *function, Statement *statement)
{

  switch (statement->statementType)
  {
  case ST_Assignment:
    {
      AssignmentStatement *asgn = ((AssignmentStatement*)statement);
      INT32 expressionResult = GenerateExpression(instructions, asgn->rightHand, function);
      INT32 addr = asgn->leftHand->address;

      switch (((AssignmentStatement*)statement)->rightHand->returnTypeId )
      {
      case TypeIdBool:
        if(((AssignmentStatement*)statement)->leftHand->type == DT_LocalValue)
          instructions.emplace_back(OP_CopybLR, addr, expressionResult);
        else if(((AssignmentStatement*)statement)->leftHand->type == DT_ParameterValue)
          instructions.emplace_back(OP_CopybPR, addr, expressionResult);
        else
          assert(0);
        break;
      case TypeIdInteger:
        if(((AssignmentStatement*)statement)->leftHand->type == DT_LocalValue)
          instructions.emplace_back(OP_CopyiLR, addr, expressionResult);
        else if(((AssignmentStatement*)statement)->leftHand->type == DT_ParameterValue)
          instructions.emplace_back(OP_CopyiPR, addr, expressionResult);
        else
          assert(0);
        break;
      default:
        assert(0); // TODO: object copy
        break;
      }

      DoneWithTheRegister(instructions, expressionResult);
    }
    break;
  case ST_BlockStatement:
    instructions.emplace_back(OP_BStart);
    {
      Block *newBlock = ((BlockStatement*)statement)->block;
      size_t size =  newBlock->statements.size();
      for(size_t i =0; i < size; ++i)
        GenerateBytecode(instructions, function, newBlock->statements[i]);
    }
    break;
  case ST_BlockEndStatement:
    instructions.emplace_back(OP_BEnd);
    break;
  case ST_IncrementStatement:
    {
      IncrementStatement *incrementStatement = (IncrementStatement*)statement;

      Instruction instruction;
      if(incrementStatement->designator->type == DT_LocalValue)
        instruction.opCode = OP_AddiLC;
      else if(incrementStatement->designator->type == DT_ParameterValue)
        instruction.opCode = OP_AddiPC;
      else
        assert(0);

      instruction.param1 = incrementStatement->designator->address;
      instruction.param2 = 1;
      instructions.push_back(instruction);
    }
    break;
  case ST_DecrementStatement:
    {
      DecrementStatement *decrementStatement = (DecrementStatement*)statement;

      Instruction instruction;

      if(decrementStatement->designator->type == DT_LocalValue)
        instruction.opCode = OP_AddiLC;
      else if(decrementStatement->designator->type == DT_ParameterValue)
        instruction.opCode = OP_AddiPC;
      else
        assert(0);

      // TODO: determine if its local/parameter/heap/sharedHeap
      instruction.param1 = decrementStatement->designator->address;
      instruction.param2 = -1;
      instructions.push_back(instruction);
    }
    break;
  case ST_InvokeStatement:
    {
      INT32 reg = GenerateExpression(instructions, ((InvokeStatement*)statement)->expression, function);
      DoneWithTheRegister(instructions, reg);
    }
    break;
  case ST_VariableDecleration:
    break; // TODO: call constructor
  case ST_ReturnStatement:
    {
      ReturnStatement *returnStatement = (ReturnStatement*)statement;

      INT32 reg = -1;
      if(returnStatement->expression) // might not have an expression
      {
        reg = GenerateExpression(instructions, returnStatement->expression, function);
      }

      if(reg != -1)
      {
        instructions.emplace_back(OP_CopyiXR, reg); // TODO: what about objects? 
        DoneWithTheRegister(instructions, reg);
      }

      instructions.emplace_back(OP_DAllocL);
      instructions.emplace_back(OP_Return);
    }
    break;
  case ST_IfStatement:
    {
      IfStatement *ifStatement = (IfStatement*)statement;
      INT32 reg = GenerateExpression(instructions, ifStatement->expression, function);

      instructions.emplace_back(OP_JumpbR, reg); // will complete parameters later
      DoneWithTheRegister(reg);
      auto &jumpIns = instructions.back();
      size_t endOfExpressionPos = instructions.size() - 1;
      jumpIns.param2 = 0; // don't jump over, just execute as usual
      GenerateBytecode(instructions, function, ifStatement->statement);
      // TODO: this should jump to next elif or else expression
      jumpIns.param3 = (INT32)(instructions.size() - endOfExpressionPos - 1); // end of statements 
    }
    break;
  case ST_WhileStatement:
    {
      WhileStatement *whileStatement = (WhileStatement*)statement;
      size_t whileExpressionPosition = instructions.size();
      INT32 reg = GenerateExpression(instructions, whileStatement->expression, function);

      instructions.emplace_back(OP_JumpbR, reg); // will complete parameters later
      DoneWithTheRegister(reg);
      auto &jumpIns = instructions.back();
      size_t endOfExpressionPos = instructions.size() - 1;
      jumpIns.param2 = 0;
      GenerateBytecode(instructions, function, whileStatement->statement);

      // jump to expression position
      instructions.emplace_back(OP_Jump, (INT32) (whileExpressionPosition - instructions.size() - 1) );

      jumpIns.param3 = (INT32)(instructions.size() - endOfExpressionPos - 1); // end of statements 
    }
    break;
  default:
    assert(0); // unknown statement
    break;
  }
}


void BytecodeGenerator::GenerateFunction(Bytecode *bytecode, const std::string &name, Function *function)
{
  // TODO: handle overloaded functions too!
  ReleaseAllRegisters();
  FunctionBytecode *functionBytecode = new FunctionBytecode();
  bytecode->functionBytecodes[function->id] = functionBytecode;
  bytecode->globalFunctionNames[name] = function->id;

  functionBytecode->instructions.emplace_back(OP_AllocL, function->stackSize);
  auto allocPos = --functionBytecode->instructions.end();

  for(auto statement : function->block->statements)
    GenerateBytecode(functionBytecode->instructions, function, statement);


  allocPos->param2 = maxRegisterNumber;

  // TODO: determine actual size of return value
  allocPos->param3 = 4;

  maxRegisterNumber = 0;

  functionBytecode->CompactInstructions();
}

INT32 BytecodeGenerator::GenerateExpression(std::list<Instruction> &instructions, Expression *expression, Function *function)
{
  std::list<ExpressionValue> executionStack;

  auto &expressionValues = expression->expressionValues;

  size_t size = expressionValues.size();
  for( size_t i = 0; i < size; ++i )
  {

    bool isOperator = false;
    switch (expressionValues[i].type )
    {
    case EVT_ConstBool:
    case EVT_ConstInt:
    case EVT_Designator:
      break;
    case EVT_PlusOperator:
    case EVT_EqualsOperator:
    case EVT_MinusOperator:
    case EVT_NegateOperator:
    case EVT_MultiplyOperator:
    case EVT_DivideOperator:
    case EVT_NotEqualOperator:
      isOperator = true;
      break;
    default:
      assert(0); // unknown  node type in expression
      break;
    }

    executionStack.push_back(expressionValues[i]);

    // pop of stack, generate codes
    if(isOperator)
    {

      ExpressionValueType operatorType = executionStack.back().type;
      executionStack.pop_back();

      if(operatorType == EVT_PlusOperator)
        PlusOperator(instructions, executionStack);
      else if( operatorType == EVT_EqualsOperator )
        EqualsOperator(instructions, executionStack);
      else if(operatorType == EVT_MinusOperator)
        MinusOperator(instructions, executionStack);
      else if(operatorType == EVT_NegateOperator)
        NegateOperator(instructions, executionStack);
      else if(operatorType == EVT_MultiplyOperator)
        MultiplyOperator(instructions, executionStack);
      else if(operatorType == EVT_DivideOperator)
        DivideOperator(instructions, executionStack);
      else if(operatorType == EVT_NotEqualOperator)
        NotEqualOperator(instructions, executionStack);
      else
        assert(0);// unknown operator type

    }

  }

  INT32 returnRegister = GetAvailableRegister();
  // if only one single value then move it temp
  if(executionStack.size() == 1)
  {
    ExpressionValue value = executionStack.back();
    executionStack.pop_back();

    switch (value.type)
    {
    case EVT_ConstBool:
      instructions.emplace_back( OP_CopybRC, returnRegister, value.intValue );
      break;
    case EVT_ConstInt:
      instructions.emplace_back( OP_CopyiRC, returnRegister, value.intValue );
      break;
    case EVT_Designator: // TODO: handle heap
      if(value.stringValue->type == DT_LocalValue)
      {
        if(value.stringValue->typeId == TypeIdInteger )
          instructions.emplace_back( OP_CopyiRL, returnRegister, value.stringValue->address );
        else if(value.stringValue->typeId == TypeIdBool)
          instructions.emplace_back( OP_CopybRL, returnRegister, value.stringValue->address );
        else
          assert(0);
      }
      else if (value.stringValue->type == DT_ParameterValue)
      {
        if(value.stringValue->typeId == TypeIdInteger )
          instructions.emplace_back( OP_CopyiRP, returnRegister, value.stringValue->address );
        else if(value.stringValue->typeId == TypeIdBool)
          instructions.emplace_back( OP_CopybRP, returnRegister, value.stringValue->address );
        else
          assert(0);
      }
      else if(value.stringValue->type == DT_FunctionCall)
      {
        GenerateFunctionCall(instructions, returnRegister, value.stringValue, function);
      }
      else
        assert(0);
      break;
    case EVT_RegisterBool: // move the left over register to r0
      instructions.emplace_back(OP_CopybRR, returnRegister, value.intValue);
      DoneWithTheRegister(instructions, value.intValue);
      break;
    case EVT_RegisterInt: // move the left over register to r0
      instructions.emplace_back(OP_CopyiRR, returnRegister, value.intValue);
      DoneWithTheRegister(instructions, value.intValue);
      break;
    default:
      assert(0);// what's this type? 
      break;
    }

  }

  if(executionStack.size())
    assert(0); // some unhandled thing in expression?

  return returnRegister;
}