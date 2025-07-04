#include "Lexer.h"
#include "PackageInfo.h"

using namespace std;

Lexer::Lexer(std::vector<Token> &tokens_) : tokens(tokens_)
{
}

bool Lexer::IsInfixOperator(TokenType type)
{
  switch (type)
  {
  case TOKEN_PLUS:
  case TOKEN_MINUS:
  case TOKEN_STAR:
  case TOKEN_SLASH:
  case TOKEN_PERCENT:
  case TOKEN_EQUAL:
  case TOKEN_NOTEQUAL:
    return true;
  default:
    return false;
  }
}

bool Lexer::IsOperator(TokenType type)
{
  switch (type)
  {
  case TOKEN_PLUS:
  case TOKEN_MINUS:
  case TOKEN_STAR:
  case TOKEN_SLASH:
  case TOKEN_PERCENT:
  case TOKEN_INCREMENT:
  case TOKEN_DECREMENT:
  case TOKEN_NOT:
  case TOKEN_EQUAL:
  case TOKEN_NOTEQUAL:
  case TOKEN_AND:
  case TOKEN_OR:
  case TOKEN_LESSTHAN:
  case TOKEN_LESSTHANEQUAL:
  case TOKEN_GREATERTHAN:
  case TOKEN_GREATERTHANEQUAL:
  case TOKEN_NEGATE:
    return true;
  default:
    return false;
  }
}

size_t Lexer::GetEndPositionOfWord(const std::string &input, INT pos)
{
  for(size_t loc = pos; loc < input.size(); ++loc)
  {
    // all special characters and space/tab/endline/return ends a word
    switch (input[loc])
    {
    case ' ':
    case '\n':
    case '\t':
    case ',':
    case ':':
    case ';':
    case '\r':
    case '$':
    case '{':
    case '}':
    case '(':
    case ')':
    case '*':
    case '/':
    case '-':
    case '+':
    case '=':
    case '!':
    case '>':
    case '<':
    case '&':
    case '|':
    case '.':
      return loc;
    default:
      break;
    }
  }

  throw;
  return -1;
}

void Lexer::LexTokenStartingWithALetter(const std::string &input, size_t &loc)
{
  size_t end_pos = GetEndPositionOfWord(input, loc);
  string word = input.substr(loc, end_pos - loc);

  // if long it's no doubt a keyword
  if( word.size() > sizeof("continue"))
  {
    tokens.emplace_back(TOKEN_IDENTIFIER, loc, end_pos - loc);
  }
  else
  {
    if(word == "bool")
      tokens.emplace_back(TOKEN_BOOL, loc, end_pos - loc);
    else if( word == "type")
      tokens.emplace_back(TOKEN_TYPE, loc, end_pos - loc);
    else if(word == "break")
      tokens.emplace_back(TOKEN_BREAK, loc, end_pos - loc);
    else if(word == "continue")
      tokens.emplace_back(TOKEN_CONTINUE, loc, end_pos - loc);
    else if(word == "var")
      tokens.emplace_back(TOKEN_VAR, loc, end_pos - loc);
    else if(word == "def")
      tokens.emplace_back(TOKEN_DEF, loc, end_pos - loc);
    else if(word == "return")
      tokens.emplace_back(TOKEN_RETURN, loc, end_pos - loc);
    else if(word == "INT")
      tokens.emplace_back(TOKEN_INT, loc, end_pos - loc);
    else if(word == "void")
      tokens.emplace_back(TOKEN_VOID, loc, end_pos - loc);
    else if(word == "if")
      tokens.emplace_back(TOKEN_IF, loc, end_pos - loc);
    else if(word == "while")
      tokens.emplace_back(TOKEN_WHILE, loc, end_pos - loc);
    else if(word == "for")
      tokens.emplace_back(TOKEN_FOR, loc, end_pos - loc);
    else if(word == "false")
      tokens.emplace_back(TOKEN_CONSTANT_FALSE, loc, end_pos - loc);
    else if(word == "true")
      tokens.emplace_back(TOKEN_CONSTANT_TRUE, loc, end_pos - loc);
    else if(word == "package")
      tokens.emplace_back(TOKEN_PACKAGE, loc, end_pos - loc);
    else
      tokens.emplace_back(TOKEN_IDENTIFIER, loc, end_pos - loc);
  }

  loc += word.size() - 1;

}
//TODO: lex float, double and long numbers
void Lexer::LexNumberConstant(const std::string &input, size_t &loc)
{
  size_t end_pos = GetEndPositionOfWord(input, loc);
  size_t value = 0;

  try{
    value = std::stoi(input.substr(loc, end_pos - loc));
  }catch(...){
    //TODO: log the error
  }

  tokens.emplace_back(TOKEN_CONSTANT_INT, loc, end_pos - loc);
  tokens.back().intValue = value;
  loc = end_pos - 1;
}

void Lexer::Lex(const std::string &input)
{
  auto GetCharAt = [&](size_t id) 
  {
    if(id >= input.size())
      return '\0';
    return input[id];
  };

  bool inCommentBlock = false;
  bool inCommentLine = false;

  for(size_t loc = 0; loc < input.size(); ++loc)
  {
    // try to ignore input if in comment line or comment block
    if(inCommentLine || inCommentBlock)
    {
      if(inCommentLine)
      {
        switch (input[loc])
        {
        case '\n':
        case '\r':
          inCommentLine = false;
          tokens.emplace_back(TOKEN_NEWLINE, loc, 1);
          break;
        }
      }
      else // in comment block
      {
        switch (input[loc])
        {
        case '*':
          if(GetCharAt(loc+1) == '/')
          {
            inCommentBlock = false;
            loc++;
          }
          break;
        }
      }
    }
    else
    {
      switch (input[loc])
      {
      case '/':
        switch (GetCharAt(loc+1))
        {
        case '/':
          inCommentLine = true;
          loc++;
          break;
        case '*':
          inCommentBlock = true;
          loc++;
          break;
        default:
          tokens.emplace_back(TOKEN_SLASH, loc, 1);
          break;
        }
        break;
      case '\t':
      case ' ':
        break;
      case '\n':
      case '\r':
      case ';':
        tokens.emplace_back(TOKEN_NEWLINE, loc, 1);
        break;
      case '.':
        tokens.emplace_back(TOKEN_DOT, loc, 1);
        break;
      case ':':
        tokens.emplace_back(TOKEN_COLON, loc, 1);
        break;
      case '=':
        if(GetCharAt(loc+1) == '=')
        {
          tokens.emplace_back(TOKEN_EQUAL, loc, 2);
          loc += 1;
        }
        else
          tokens.emplace_back(TOKEN_ASSIGN, loc, 1);
        break;
      case ',':
        tokens.emplace_back(TOKEN_COMMA, loc, 1);
        break;
      case '$':
        tokens.emplace_back(TOKEN_DEF, loc, 1);
        break;
      case '(':
        tokens.emplace_back(TOKEN_OPEN_PAREN, loc, 1);
        break;
      case ')':
        tokens.emplace_back(TOKEN_CLOSE_PAREN, loc, 1);
        break;
      case '{':
        tokens.emplace_back(TOKEN_OPEN_CURLY, loc, 1);
        break;
      case '}':
        tokens.emplace_back(TOKEN_CLOSE_CURLY, loc, 1);
        break;
      case '[':
        tokens.emplace_back(TOKEN_OPEN_BRACKET, loc, 1);
        break;
      case ']':
        tokens.emplace_back(TOKEN_CLOSE_BRACKET, loc, 1);
        break;
      case '*':
        tokens.emplace_back(TOKEN_STAR, loc, 1);
        break;
      case '<':
        if(GetCharAt(loc+1) == '=')
          tokens.emplace_back(TOKEN_LESSTHANEQUAL, loc++, 2);
        else
          tokens.emplace_back(TOKEN_LESSTHAN, loc, 1);
        break;
      case '>':
        if(GetCharAt(loc+1) == '=')
          tokens.emplace_back(TOKEN_GREATERTHANEQUAL, loc++, 2);
        else
          tokens.emplace_back(TOKEN_GREATERTHAN, loc, 1);
        break;
      case '&':
        if(GetCharAt(loc+1) == '&')
          tokens.emplace_back(TOKEN_AND, loc++, 2);
        else
          tokens.emplace_back(TOKEN_BITWISE_AND, loc, 1);
        break;
      case '|':
        if(GetCharAt(loc+1) == '|')
          tokens.emplace_back(TOKEN_OR, loc++, 2);
        else
          tokens.emplace_back(TOKEN_BITWISE_OR, loc, 1);
        break;
      case '+':
        {
          if( GetCharAt( loc + 1) == '+')
          {
            tokens.emplace_back(TOKEN_INCREMENT, loc, 2);
            loc++; // since it is 2 chars increment by counter one more
          }
          else
            tokens.emplace_back(TOKEN_PLUS, loc, 1);
        }
        break;
      case '-':
        {
          if( GetCharAt( loc + 1) == '-')
          {
            tokens.emplace_back(TOKEN_DECREMENT, loc, 2);
            loc++; // since it is 2 chars increment by counter one more
          }
          else
          {
            TokenType lastToken = GetTokenAt( tokens.size() - 1);
            if(lastToken == TOKEN_OPEN_PAREN)
              tokens.emplace_back(TOKEN_NEGATE, loc, 1);
            else if( IsInfixOperator(lastToken) )
              tokens.emplace_back(TOKEN_NEGATE, loc, 1);
            else if( lastToken == TOKEN_IF )
              tokens.emplace_back(TOKEN_NEGATE, loc, 1);
            else if( lastToken == TOKEN_ELIF )
              tokens.emplace_back(TOKEN_NEGATE, loc, 1);
            else if( lastToken == TOKEN_ELSE )
              tokens.emplace_back(TOKEN_NEGATE, loc, 1);
            else if( lastToken == TOKEN_WHILE)
              tokens.emplace_back(TOKEN_NEGATE, loc, 1);
            else
              tokens.emplace_back(TOKEN_MINUS, loc, 1);
          }
        }
        break;
      case '!':
        if(GetCharAt(loc+1) == '=')
        {
          tokens.emplace_back(TOKEN_NOTEQUAL, loc, 2);
          loc++;
        }
        else
          tokens.emplace_back(TOKEN_NOT, loc, 1);
        break;
      case '%':
        tokens.emplace_back(TOKEN_PERCENT, loc, 1);
        break;
      case '0':
      case '1':
      case '2':
      case '3':
      case '4':
      case '5':
      case '6':
      case '7':
      case '8':
      case '9':
        LexNumberConstant(input, loc);
        break;
      case 'e': // lex potential 'elif' 'else' 'else if'
        if(GetCharAt(loc + 1) == 'l')
        {
          if(input.substr(loc, 7) == "else if") // also accept 'else if' as elif token
          {
            tokens.emplace_back(TOKEN_ELIF, loc, 7);
            loc += 6; // one less because for loop will increase one more
          }
          else if(input.substr(loc, 4) == "else")
          {
            tokens.emplace_back(TOKEN_ELSE, loc, 4);
            loc += 3; // one less because for loop will increase one more
          }
          else if(input.substr(loc, 4) == "elif") // accept 'elif' as elif token
          {
            tokens.emplace_back(TOKEN_ELIF, loc, 4);
            loc += 3; // one less because for loop will increase one more
          }
          else
            LexTokenStartingWithALetter(input, loc);
        }
        else
          LexTokenStartingWithALetter(input, loc);
        break;
      default:
        LexTokenStartingWithALetter(input, loc);
        break;
      }


    }
  }

}