#pragma once

#include "amp.h"
#include "node.h"
#include <memory>
#include <iostream>
#include <stdexcept>

class amp_instruction
{
public:
  amp_instruction() {}
  op_code                                            opcode;
  int                                                 index;
  double                                              value;
  double                                             weight;
  std::string                                         label;
  std::unique_ptr<concurrency::array_view<double, 1>>  data;
};

class amp_interpreter
{
public:
  explicit amp_interpreter(std::unordered_map<std::string, std::vector<double>>& data)
  {
    rows = static_cast<int>(data.begin()->second.size());
    for (const auto & t : data)
    {
      gpu_data[t.first] = std::make_unique<concurrency::array_view<const double, 1>>(t.second.size(), t.second);
    }
  }
  ~amp_interpreter() {}

  std::vector<amp_instruction> compile(node *root) const;

  std::unique_ptr<concurrency::array_view<double, 1>> evaluate(node *root);
  std::unique_ptr<concurrency::array_view<double, 1>> evaluate(std::vector<amp_instruction>& instructions);

private:
  int rows;
  std::unordered_map<std::string, std::unique_ptr<concurrency::array_view<const double, 1>>> gpu_data;
};
