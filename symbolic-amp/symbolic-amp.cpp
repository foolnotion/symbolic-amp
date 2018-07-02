#include <iostream>
#include <memory>
#include <chrono>
#include <unordered_map>
#include <numeric>
#include <execution>

#include "amp.h"
#include "node.h"
#include "interpreter.h"
#include "random.h"
#include "amp_interpreter.h"
#include "util.h"
#include "hierarchicalformatter.h"

using namespace std;
using namespace concurrency;

void gpu_info() {
  cout << "Available accelerators: " << endl;
  for (auto &a : accelerator::get_all())
    wcout << a.description << endl;
}

int main(int argc, char* argv[])
{
  if (argc < 5) {
    cout << "Usage: symbolic-amp.exe <ntrees> <nrows> <nvars> <tree_depth>" << endl;
    return -1;
  }

  //gpu_info();

  auto rnd = make_unique<random>();
  rnd->seed(1234);

  auto ntrees = atol(argv[1]);
  auto nrows = atol(argv[2]);
  auto nvars = atol(argv[3]);
  auto depth = atol(argv[4]);

  // generate rows
  auto rows = vector<int>(nrows);
  int i = 0;
  generate(begin(rows), end(rows), [&i]() { return i++; }); // will rows with consecutive values

  // generate random variable values
  vector<vector<double>> variables(nvars);
  unordered_map<string, vector<double>> data;
  for (int i = 0; i < nvars; ++i) {
    vector<double> values(nrows);
    generate(begin(values), end(values), [&rnd] { return rnd->next_double(); });
    data["x" + std::to_string(i + 1)] = std::move(values);
  }

  // create some trees
  vector<node*> trees(ntrees);
  generate(begin(trees), end(trees), [=, &rnd, &data]() { return node::Random(rnd.get(), data, depth); });
  unsigned long long nodes = accumulate(begin(trees), end(trees), 0, [=](unsigned long len, node* p) { return len + p->GetLength(); });

  cout << "nodes = " << nodes << endl;

  auto ac = accelerator::get_all()[0];
  accelerator::set_default(ac.device_path);

  auto hrc = make_unique<chrono::high_resolution_clock>();
  auto start = hrc->now();
  vector<double> eval(nrows);
  std::for_each(execution::seq, begin(trees), end(trees), [&](node *t){ 
    auto instructions = interpreter::compile(t, data);
    for (int i = 0; i < nrows; ++i) {
      interpreter::evaluate(instructions, i);
    }
  });
  auto cpu_single_time = chrono::duration_cast<chrono::milliseconds>(hrc->now() - start).count() / 1e3;
  auto cpu_single_speed = nodes / cpu_single_time /1e6 * nrows;

  start = hrc->now();
  std::for_each(execution::par, begin(trees), end(trees), [&](node *t){ 
    auto instructions = interpreter::compile(t, data);
    for (int i = 0; i < nrows; ++i) {
      interpreter::evaluate(instructions, i);
    }
  });
  auto cpu_multi_time = chrono::duration_cast<chrono::milliseconds>(hrc->now() - start).count() / 1e3;
  auto cpu_multi_speed = nodes / cpu_multi_time / 1e6 * nrows; 

  try {
    // C++ AMP kernels are Just-In-Time (JIT) compiled from High Level Shader Language (HLSL) bytecode to machine code by the GPU driver at run time. 
    // Compiled kernels are cached until the process terminates.This means that there is an additional compilation overhead that occurs for each call 
    // site of lambda or function marked with restrict(amp). Run the kernel once to force the JIT compiler to run prior to executing the timed kernel.
    auto interp = make_unique<amp_interpreter>(data);
    start = hrc->now();
    std::for_each(execution::seq, begin(trees), end(trees), [&](node *t) {
      interp->evaluate(t);
    });
    auto gpu_time = chrono::duration_cast<chrono::milliseconds>(hrc->now() - start).count() / 1e3;
    auto gpu_speed = nodes / gpu_time / 1e6 * nrows; 
    cout << cpu_single_speed << ";" << cpu_multi_speed << ";" << gpu_speed << endl;
  }
  catch (exception e)
  {
    cout << "ERROR: " << e.what() << endl;
  }

  return 0;
}
