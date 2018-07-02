#include "amp_interpreter.h"
#include <iostream>

using namespace std;
using namespace concurrency;

vector<amp_instruction> amp_interpreter::compile(node *root) const
{
  vector<amp_instruction> instructions(root->GetLength());
  amp_instruction ai(rows);
  ai.opcode = root->GetOpCode();
  instructions[0] = std::move(ai);
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
      amp_instruction instr(rows);
      instr.opcode = subtree->GetOpCode();
      instr.label = subtree->GetName();
      if (subtree->GetOpCode() == VARIABLE)
      {
        instr.weight = subtree->GetWeight();
      }
      if (subtree->GetOpCode() == CONSTANT)
      {
        instr.value = subtree->GetValue();
      }
      instructions[c + j] = std::move(instr);
    }
    instructions[i].index = c;
    c += node->SubtreeCount();
  }
  return instructions;
}

array_view<double, 1> amp_interpreter::evaluate(node *root)
{
  auto instructions = compile(root);
  return evaluate(instructions);
}

array_view<double, 1> amp_interpreter::evaluate(vector<amp_instruction>& code)
{
  for (auto it = rbegin(code); it != rend(code); ++it)
  {
    switch (it->opcode)
    {
    case ADD:
    {
      it->data = code[it->index].data;
      auto a = it->data;
      auto b = code[it->index + 1].data;
      parallel_for_each(a.extent, [=](index<1> idx) restrict(amp)
      {
        a[idx] += b[idx];
      });
      break;
    }
    case SUB:
    {
      it->data = code[it->index].data;
      auto a = it->data;
      auto b = code[it->index + 1].data;
      parallel_for_each(a.extent, [=](index<1> idx) restrict(amp)
      {
        a[idx] -= b[idx];
      });
      break;
    }
    case MUL:
    {
      it->data = code[it->index].data;
      auto a = it->data;
      auto b = code[it->index + 1].data;
      parallel_for_each(a.extent, [=](index<1> idx) restrict(amp)
      {
        a[idx] *= b[idx];
      });
      break;
    }
    case DIV:
    {
      it->data = code[it->index].data;
      auto a = it->data;
      auto b = code[it->index + 1].data;
      parallel_for_each(a.extent, [=](index<1> idx) restrict(amp)
      {
        a[idx] /= b[idx];
      });
      break;
    }
    case VARIABLE:
    {
      concurrency::array_view<double, 1> a = it->data;
      concurrency::array_view<const double, 1> v = *gpu_data[it->label];
      double weight = it->weight;
      parallel_for_each(a.extent, [=](index<1> idx) restrict(amp)
      {
        a[idx] = v[idx] * weight;
      });
      break;
    }
    case CONSTANT: {
      auto a = it->data;
      double v = it->value;
      parallel_for_each(a.extent, [=](index<1> i) restrict(amp)
      {
        a[i] = v;
      });
      break;
    }
    default: break;
    }
  }
  return code[0].data;
}
