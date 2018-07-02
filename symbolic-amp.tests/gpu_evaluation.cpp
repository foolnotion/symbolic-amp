#include "CppUnitTest.h"

#include <memory>
#include <algorithm>
#include <vector>
#include <unordered_map>
#include <string>

#include "../symbolic-amp/interpreter.h"
#include "../symbolic-amp/amp_interpreter.h"
#include "../symbolic-amp/random.h"
#include "../symbolic-amp/node.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;
using namespace std;

namespace symbolicamptests
{		
  TEST_CLASS(SymbolicAmpTests)
  {
  public:
    
    TEST_METHOD(TestGpuEvaluation)
    {
      int rows = 100;
      auto rand = make_unique<random>();
      auto values = vector<double>(rows);
      generate(begin(values), end(values), [&] { return rand->next_double(); });
      auto data = unordered_map<string, vector<double>>{ { "x1", values } };

      auto gpu_interp = make_unique<amp_interpreter>(data);


      auto tree = node::Random(rand.get(), data, 5);

      auto evaluation = gpu_interp->evaluate(tree);
      for (int row = 0; row < rows; ++row) {

        auto v = interpreter::evaluate(tree, row, data);
        Assert::AreEqual(v, evaluation[row], L"Evaluated values should be the same", LINE_INFO());
      }
    }
  };
}