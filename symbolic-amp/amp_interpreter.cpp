#include "amp_interpreter.h"
#include <iostream>

using namespace std;
using namespace concurrency;

vector<amp_instruction> amp_interpreter::compile(node *root) const
{
    vector<amp_instruction> instructions(root->GetLength());
    instructions[0].opcode = root->GetOpCode();
    auto nodes = root->IterateBreadth();
    int c = 1;
    for (size_t i = 0; i < nodes.size(); ++i)
    {
        auto node = nodes[i];
        if (node->SubtreeCount() == 0)
            continue;
        auto subtrees = node->Subtrees();
        for (size_t j = 0; j != subtrees.size(); ++j)
        {
            auto subtree = subtrees[j];
            auto & instr = instructions[c + j];
            instr.opcode = subtree->GetOpCode();
            instr.label = subtree->GetName();
            if (subtree->GetOpCode() == VARIABLE)
            {
                instr.weight = subtree->GetWeight();
                instr.data = std::make_unique<concurrency::array_view<double, 1>>(rows);
            }
            if (subtree->GetOpCode() == CONSTANT)
            {
                instr.value = subtree->GetValue();
                instr.data = std::make_unique<concurrency::array_view<double, 1>>(rows);
            }
        }
        instructions[i].index = c;
        c += node->SubtreeCount();
    }
    return instructions;
}

unique_ptr<array_view<double, 1>> amp_interpreter::evaluate(node *root)
{
    auto instructions = compile(root);
    return std::move(evaluate(instructions));
}

unique_ptr<array_view<double, 1> >amp_interpreter::evaluate(vector<amp_instruction>& code)
{
    for (auto it = rbegin(code); it != rend(code); ++it)
    {
        switch (it->opcode)
        {
        case ADD:
        {
            it->data = std::move(code[it->index].data);
            auto a = *it->data;
            auto b = *code[it->index + 1].data;
            parallel_for_each(a.extent, [=](index<1> idx) restrict(amp)
            {
                a[idx] += b[idx];
            });
            b.discard_data();
            break;
        }
        case SUB:
        {
            it->data = std::move(code[it->index].data);
            auto a = *it->data;
            auto b = *code[it->index + 1].data;
            parallel_for_each(a.extent, [=](index<1> idx) restrict(amp)
            {
                a[idx] -= b[idx];
            });
            b.discard_data();
            break;
        }
        case MUL:
        {
            it->data = std::move(code[it->index].data);
            auto a = *it->data;
            auto b = *code[it->index + 1].data;
            parallel_for_each(a.extent, [=](index<1> idx) restrict(amp)
            {
                a[idx] *= b[idx];
            });
            b.discard_data();
            break;
        }
        case DIV:
        {
            it->data = std::move(code[it->index].data);
            auto a = *it->data;
            auto b = *code[it->index + 1].data;
            parallel_for_each(a.extent, [=](index<1> idx) restrict(amp)
            {
                a[idx] /= b[idx];
            });
            b.discard_data();
            break;
        }
        case VARIABLE:
        {
            auto a = *it->data;
            auto v = *gpu_data[it->label];
            double weight = it->weight;
            parallel_for_each(a.extent, [=](index<1> idx) restrict(amp)
            {
                a[idx] = v[idx] * weight;
            });
            break;
        }
        case CONSTANT: {
            auto a = *it->data;
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
    return std::move(code[0].data);
}
