#include "Parser.h"

void Parser::Start()
{
  for(auto packageInfo : packageInfos)
  {
    PackageParser packageParser(*packageInfo);
    packageParser.Parse();
  }
}