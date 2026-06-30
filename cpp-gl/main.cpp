#include <iostream>
#include <vector>

int main() {
  std::vector<float> vv;
  vv.push_back(1.0f);
  vv.push_back(2.5f);
  // uint32_t is always 32 bites
  // unsigned int is platform dependent but at least 16 bits
  // for (uint32_t i = 0; i <vv.size(); i++) {
  //   std::cout << vv[i] << std::endl;
  // }
  
  for (auto & element : vv) {
    std::cout << element <<std::endl;
  }

  std::cout << "hello" << std::endl;
  return 0;
}
