#pragma once

#include <string>

namespace UWP {
class XeniaUWP {
 public:
  void StartXenia(std::string gamePath, std::string configPath, uint32_t width, uint32_t height);
  void ExecutePendingFunctionsFromUIThread();
};
}