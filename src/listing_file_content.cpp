#include <fstream>
#include <iomanip>
#include <iostream>
#include <string>

int main() {
  std::ifstream ifs{ "listing_file_conten.cpp" };
  std::string line;
  int x;

  if (ifs.is_open()) {
    while (std::getline(ifs, line)) {
      std::cout << ">>> Line:\
// " << std::quoted(line)
                << "\n";
    }
    ifs.close();
  }

  return 0;
}
