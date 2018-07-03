#include "CppUnitTest.h"

#include <memory>
#include <algorithm>
#include <vector>
#include <unordered_map>
#include <string>
#include <chrono>
#include <numeric>

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
    TEST_METHOD(GpuEvaluationCorrectnessTest)
    {
      auto nrows = 1000UL;
      auto rand = make_unique<random>();
      auto values = vector<double>(nrows);
      generate(begin(values), end(values), [&] { return rand->next_double(); });
      auto data = unordered_map<string, vector<double>>{ { "x1", values } };

      auto gpu_interp = make_unique<amp_interpreter>(data);
      auto tree = node::Random(rand.get(), data, 5);

      auto evaluation = *gpu_interp->evaluate(tree);
      for (auto row = 0; row < nrows; ++row) {
        auto v = interpreter::evaluate(tree, row, data);
        Assert::AreEqual(v, evaluation[row], L"Evaluated values should be the same", LINE_INFO());
      }
    }

    TEST_METHOD(GpuEvaluationSpeedTest) {
      auto repetitions = 10;
      auto depth = 5;
      auto ntrees = (int)1e3;
      auto nrows = (int)1e5;

      // generate test data
      auto rand = make_unique<random>();
      auto values = vector<double>(nrows);
      generate(begin(values), end(values), [&] { return rand->next_double(); });
      auto data = unordered_map<string, vector<double>>{ { "x1", values } };

      auto trees = vector<node*>(ntrees);
      generate(begin(trees), end(trees), [&]() { return node::Random(rand.get(), data, depth); });

      auto gpu_interp = make_unique<amp_interpreter>(data);

      // warm-up
      for (auto tree : trees) {
        gpu_interp->evaluate(tree);
      }

      auto hrc = make_unique<chrono::high_resolution_clock>();
      auto start = hrc->now();
      for (int i = 0; i < repetitions; ++i) {
        for (auto tree : trees) {
          gpu_interp->evaluate(tree);
        }
      }
      unsigned long nodes = accumulate(begin(trees), end(trees), 0, [=](unsigned long len, node* p) { return len + p->GetLength(); });
      auto elapsed = chrono::duration_cast<chrono::milliseconds>(hrc->now() - start).count() / 1e3;
      auto speed = nodes / elapsed * nrows * repetitions;
      ostringstream ss;
      ss << elapsed << " seconds elapsed. " << speed << " nodes/s";
      Logger::WriteMessage(ss.str().c_str());
    }
  };
}