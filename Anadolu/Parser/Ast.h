#pragma once 


#include "Lexer.h"
#include "Node.h"

class PackageInfo;

class AST
{
private:

public:

  Node *mainNode;

  AST() : mainNode(new Node(MainNode)) {}
  ~AST() { delete mainNode; }

};