#include "TestHelper.h"

void executeTest(Parser &parser)
{
  auto node = parser.parse();
  std::stringstream ss;
  mkString(ss, node, 0);
  printf("%s\n", ss.str().c_str());
  delete node;
}
