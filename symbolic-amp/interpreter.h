#pragma once
#include "node.h"
#include <unordered_map>
#include <string>

struct instruction
{
  op_code             opcode;
  int                  arity;
  int                  index;
  double               value;
  double              weight;
  std::string          label;
  std::vector<double>   data;
};

class interpreter
{
public:
  interpreter() {}
  ~interpreter() {}

  static std::vector<instruction> compile(node *root, std::unordered_map<std::string, std::vector<double>>& data)
  {
    std::vector<instruction> instructions(root->GetLength());
    instructions[0] = instruction{ root->GetOpCode(), root->SubtreeCount() };
    auto nodes = root->IterateBreadth();
    int c = 1;
    for (size_t i = 0; i < nodes.size(); ++i) {
      auto node = nodes[i];
      if (node->SubtreeCount() == 0)
        continue;
      auto subtrees = node->Subtrees();
      for (size_t j = 0; j != subtrees.size(); ++j)
      {
        auto subtree = subtrees[j];
        instruction instr;
        instr.label = subtree->GetName();
        instr.opcode = subtree->GetOpCode();
        if (subtree->GetOpCode() == VARIABLE)
        {
          instr.data = data[instr.label];
          instr.weight = subtree->GetWeight();
        }
        if (subtree->GetOpCode() == CONSTANT)
        {
          instr.value = subtree->GetValue();
        }

        instructions[c + j] = instr;
      }
      instructions[i].index = c;
      instructions[i].arity = node->SubtreeCount();
      c += node->SubtreeCount();
    }
    return instructions;
  }

  static double evaluate(node* root, int row, std::unordered_map<std::string, std::vector<double>>& data)
  {
    auto instructions = compile(root, data);
    return evaluate(instructions, row);
  }

  static std::vector<double> evaluate(node *root, const std::vector<int>& rows, std::unordered_map<std::string, std::vector<double>>& data)
  {
    auto values = std::vector<double>(rows.size());
    int row = 0;
    auto instructions = compile(root, data);
    std::generate(begin(values), end(values), [&]() { return evaluate(instructions, rows[row++]); });
    return values;
  }

  static double evaluate(std::vector<instruction>& code, int row)
  {
    for (auto it = std::rbegin(code); it != std::rend(code); ++it)
    {
      switch (it->opcode)
      {
      case VARIABLE:
      {
        auto weight = it->weight;
        it->value = it->data[row] * weight;
        break;
      }
      case ADD:
      {
        it->value = code[it->index].value + code[it->index + 1].value;
        break;
      }
      case SUB:
      {
        it->value = code[it->index].value - code[it->index + 1].value;
        break;
      }
      case MUL:
      {
        it->value = code[it->index].value * code[it->index + 1].value;
        break;
      }
      case DIV:
      {
        it->value = code[it->index].value / code[it->index + 1].value;
        break;
      }

      default: break;
      }
    }
    return code[0].value;
  }
};

