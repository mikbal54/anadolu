#include "Ast.h"
#include "PackageInfo.h"
#include <functional>

INT Node::nodeCount = 0;
INT Node::nodeDestroyed = 0;

std::string Node::GetAsString()
{
  switch (type)
  {
  case Unknown:
    return "Unknown";
  case MainNode:
    return "Main";
  case PlusNode:
    return "Plus";
  case MinusNode:
    return "Minus";
  case IncNode:
    return "Inc";
  case ExpressionNode:
    return "Expression";
  case FunctionCallNode:
    return "Function";
  case IdentifierNode:
    return "Identifier";
  case TypeNameNode:
    return "TypeName";
  case ParameterNode:
    return "Parameter";
  case ParameterListNode:
    return "ParameterList";
  case FunctionDeclarationNode:
    return "FunctionDeclaration";
  case BlockNode:
    return "Block";
  case StatementNode:
    return "Statement";
  case IfNode:
    return "If";
  case ElseIfNode:
    return "Elif";
  case ElseNode:
    return "Else";
  case WhileNode:
    return "While";
  case VariableDeclarationNode:
    return "VariableDeclaration";
  case AssignmentNode:
    return "Assignment";
  case BreakStatementNode:
    return "Break";
  case ContinueStatementNode:
    return "Continue";
  default:
    return "???";
  }
}

void Node::ConvertToString(Node *node, PackageInfo &module, std::string &output, INT indent)
{
  std::vector<Node*> children;
  node->GetChildren(children);

  for(INT i = 0; i < indent; ++i)
    output += "-";

  output += node->GetAsString();

  if(node->startToken >= 0)
  {
    output += " [ ";
    for(INT i = node->startToken; i <= node->endToken; ++i)
    {
      output += module.GetTokenAsString(i); // TODO: replace with line number and start end strings. Currently outputs too many chars
    }
    output += " ]";
  }

  output += "\n";

  for(auto child : children)
    ConvertToString(child, module, output, indent+1);

}