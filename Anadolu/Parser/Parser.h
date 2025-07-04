#pragma once

#include "PackageParser.h"

class Parser
{
  std::vector<PackageInfo*> packageInfos;

public:

  void AddPackageInfo(PackageInfo *packageInfo){ packageInfos.push_back(packageInfo); }

  void Start();

};
