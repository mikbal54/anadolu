#pragma once


enum TokenType
{
  TOKEN_INVALID,
  TOKEN_IDENTIFIER,

  TOKEN_PACKAGE,

  TOKEN_TYPE,

  TOKEN_NEWLINE, // '\n' '\r' or ';'

  TOKEN_VAR, // var
  TOKEN_VOID, // void
  TOKEN_INT, // INT
  TOKEN_BOOL, // bool

  TOKEN_CONSTANT_INT, // a number like 234 1 42 etc..
  TOKEN_CONSTANT_FALSE, // false
  TOKEN_CONSTANT_TRUE, // true

  TOKEN_DEF, // def
  TOKEN_RETURN, // return
  TOKEN_IF, // if
  TOKEN_ELIF, // elif
  TOKEN_ELSE, // else
  TOKEN_WHILE, // while
  TOKEN_FOR, // for

  TOKEN_BREAK, // break
  TOKEN_CONTINUE, // continue

  TOKEN_COMMA, // ,
  TOKEN_COLON, // : 

  TOKEN_OPEN_PAREN, // (
  TOKEN_CLOSE_PAREN, // )
  TOKEN_OPEN_CURLY, // {
  TOKEN_CLOSE_CURLY, // }
  TOKEN_OPEN_BRACKET, // [
  TOKEN_CLOSE_BRACKET, // ]

  TOKEN_ASSIGN, // =

  TOKEN_BITWISE_AND, // & (not used)
  TOKEN_BITWISE_OR, // | (not used)

  TOKEN_DOT, // .

  TOKEN_AND, // &&
  TOKEN_OR, // ||
  TOKEN_EQUAL, // ==
  TOKEN_NOTEQUAL, // !=
  TOKEN_LESSTHAN, // <
  TOKEN_LESSTHANEQUAL, // <=
  TOKEN_GREATERTHAN, // >
  TOKEN_GREATERTHANEQUAL, // >=

  TOKEN_STAR, // *
  TOKEN_PLUS, // +
  TOKEN_MINUS, // -
  TOKEN_SLASH, // /
  TOKEN_PERCENT, // %
  TOKEN_NOT, // !

  TOKEN_INCREMENT, // ++
  TOKEN_DECREMENT, // --

  TOKEN_NEGATE // unary - operator
};
