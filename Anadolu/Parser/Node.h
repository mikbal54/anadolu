#pragma once

#include "TokenType.h"
#include "PrimitiveTypes.h"

#include <vector>
#include <assert.h>

class PackageInfo;

enum NodeType
{
  Unknown,
  MainNode,
  TempRegisterNode, // used in expression parser
  DesignatorNode, // like: ident.ident.func() 
  IdentifierNode,
  TypeDefinitionNode,
  TypeLegendsNode,
  TypeNameNode,
  ExpressionNode,
  FunctionCallNode,
  ConstIntNode,
  ConstBoolNode,
  PlusNode, // +
  MinusNode, // -
  EqualsNode, // == 
  NotEqualNode, // !=
  IncNode, // ++
  NegateNode, // -
  MultiplyNode, // *
  DivideNode,
  ParameterNode,
  ParameterListNode,
  FunctionDeclarationNode,
  BlockNode,
  StatementNode,
  IfNode,
  ElseIfNode,
  ElseNode,
  WhileNode,
  VariableDeclarationNode,
  ReturnStatementNode,
  InvokeStatementNode,
  IncrementStatementNode,
  DecrementStatementNode,
  AssignmentNode,
  BreakStatementNode,
  ContinueStatementNode
};

class Node
{
public:

  // TODO: remove these. That's just for test
  static INT nodeCount;
  static INT nodeDestroyed;

  NodeType type;
  Node *parent; // enables backtracking the tree
  Node *prev; // sibling of this node
  Node *next; // sibling of this node
  Node *firstChild; // used to iterate all children
  Node *lastChild; // with this we can add children faster.
  INT startToken;
  INT endToken;

  Node(): type(Unknown), prev(0), next(0), firstChild(0), lastChild(0), parent(0), startToken(-1) { nodeCount++; }

  Node(NodeType _type) : type(_type), prev(0), next(0), firstChild(0), lastChild(0), parent(0), startToken(-1), endToken(-1) { nodeCount++; }
  Node(NodeType _type, INT _startToken, INT _endToken) : type(_type), prev(0), next(0), firstChild(0), lastChild(0), parent(0), startToken(_startToken), endToken(_endToken) { nodeCount++; }

  //TODO: walking tree and destroying it is too slow. Add a hash map and destroy it like that
  ~Node() 
  {
    ++nodeDestroyed;
    if(firstChild)
    {
      std::vector<Node*> children;
      GetChildren(children);
      for(size_t i = 0; i < children.size(); ++i)
        delete children[i];

    }
  }

  static NodeType GetTokenNodeType(TokenType type)
  {
    switch (type)
    {
    case TOKEN_INVALID:
      break;
    case TOKEN_NEGATE:
      return NegateNode;
    case TOKEN_IDENTIFIER:
      return IdentifierNode;
    case TOKEN_CONSTANT_INT:
      return ConstIntNode;
    case TOKEN_CONSTANT_FALSE:
      return ConstBoolNode;
    case TOKEN_CONSTANT_TRUE:
      return ConstBoolNode;
    case TOKEN_EQUAL:
      return EqualsNode;
    case TOKEN_NOTEQUAL:
      return NotEqualNode;
    case TOKEN_PLUS:
      return PlusNode;
    case TOKEN_MINUS:
      return MinusNode;
    case TOKEN_INCREMENT:
      return IncNode;
    case TOKEN_STAR:
      return MultiplyNode;
    case TOKEN_SLASH:
      return DivideNode;
    default:
      break;
    }

    assert(0);
    return Unknown;
  }

  bool HasChildren() { return firstChild ? true : false; }

  void GetChildren(std::vector<Node*> &children)
  {
    children.empty();
    if(!firstChild)
      return;

    Node *nextChild = firstChild;
    while(nextChild)
    {
      children.emplace_back(nextChild);
      nextChild = nextChild->next;
    }
  }

  std::string GetAsString();

  static void ConvertToString(Node *node, PackageInfo &module, std::string &output, INT indent = 0);

  void AddChild(Node *newNode)
  {
    newNode->parent = this;
    if(!firstChild)
    {
      firstChild = newNode;
      lastChild = firstChild;
      return;
    }

    // only a single child
    if(lastChild == firstChild)
    {
      lastChild = newNode;
      lastChild->prev = firstChild;
      firstChild->next = lastChild;
      return;
    }

    lastChild->next = newNode;
    newNode->prev = lastChild;
    lastChild = newNode;
  }

};