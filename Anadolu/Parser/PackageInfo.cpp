#include "PackageInfo.h"

void PackageInfo::AddScriptSection(const std::string &section)
{
  script += section;
}

void PackageInfo::Lex()
{
  if(script.empty())
    return;

  // UTF-8 BOM
  // clear bom data
  if(script[0] == 'ï')
  {
    script[0] = ' ';
    script[1] = ' ';
    script[2] = ' ';
  }

  tokens.clear();
  Lexer lexer(tokens);
  lexer.Lex(script);

}

std::string PackageInfo::GetTokenAsString(size_t tokenNumber)
{
  if(tokenNumber >= tokens.size())
    return "eof";
  Token &token = tokens[tokenNumber];
  if(token.type == TOKEN_NEWLINE)
    return " ";
  return script.substr(token.start, token.length);
}