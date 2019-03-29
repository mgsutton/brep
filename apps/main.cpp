#include <vector>
#include "brep.h"
int main(int argc, char *argv[]) {
  std::string file = "E:\\sutton\\dev\\brep\\testInputs\\blockWithHole.brep";
  padt::brep::BRep brep;
  brep.buildBRepFromFile(file);
  for (const auto ah : brep.assemblies()) {
    auto bbox = brep.boundingBox(ah);
    std::cout << "Assembly " << ah << " bounding box center: " << bbox.center()
              << std::endl;
    std::cout << "Assembly " << ah << " bounding box volume: " << bbox.volume()
              << std::endl;
  }
  return 0;
}