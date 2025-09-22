#include "OpenFlightReader.h"
#include <iostream>
int main() {
  OpenFlightReader reader("Model_3_ver164.flt");
  try {
    reader.Read();
  } catch (const std::runtime_error &e) {
    std::cerr << e.what() << std::endl;
    return 1;
  }
  return 0;
}