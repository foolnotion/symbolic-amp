#include "node.h"
#include <numeric>
#include <algorithm>
#include "dataset.h"

using namespace std;

node::~node()
{
    for (auto s : subtrees_)
        delete s;
}

// deep cloning
node* node::Clone() const
{
    auto n = new node(*this);
    for (auto s : subtrees_)
        n->AddSubtree(s->Clone());
    return n;
}

static void Grow(random *rnd, node* n, std::unordered_map<std::string, std::vector<double>>& data, int depth, int max_depth)
{
    for (int i = 0; i < 2; ++i)
    {
        auto op = depth < max_depth ? (rnd->next_double() > 0.5 ? static_cast<op_code>(rnd->next(DIV)) : VARIABLE) : VARIABLE;
        auto subtree = new node(op, "");

        if (op == VARIABLE)
        {
            auto random_it = std::next(std::begin(data), rnd->next(0, static_cast<int>(data.size() - 1u))); // reserve one for the target
            subtree->SetName(random_it->first);
            subtree->SetWeight(rnd->next_double(-5, +5));
        }
        else if (op == CONSTANT)
        {
            subtree->SetValue(rnd->next_double(-5, +5));
        }
        else
        {
            Grow(rnd, subtree, data, depth + 1, max_depth);
        }
        n->AddSubtree(subtree);
    }
}

node* node::Random(random *rnd, std::unordered_map<std::string, std::vector<double>>& data, int max_depth)
{
    auto op = static_cast<op_code>(rnd->next(DIV));
    auto root = new node(op, "");
    Grow(rnd, root, data, 2, max_depth);
    return root;
}


int node::GetLength() 
{
    if (length_ <= 0)
        length_ = accumulate(begin(subtrees_), end(subtrees_), 1, [&](int length, node *n) { return length + n->GetLength(); });
    return length_;
}

int node::GetDepth()
{
    if (depth_ > 0) return depth_;
    depth_ = 1;
    for (auto s : subtrees_)
        depth_ = max(depth_, s->GetDepth() + 1);
    return depth_;
}

void node::AddSubtree(node* s)
{
    subtrees_.push_back(s);
    s->SetParent(this);
}

void node::InsertSubtree(node* s, int index)
{
    auto it = begin(subtrees_) + index;
    subtrees_.insert(it, s);
    s->SetParent(this);
}

void node::RemoveSubtree(node* s)
{
    remove(begin(subtrees_), end(subtrees_), s);
    s->SetParent(nullptr); // set the parent to null
}

int node::IndexOfSubtree(node* s) const
{
    auto first = begin(subtrees_), last = end(subtrees_);
    auto it = find(begin(subtrees_), end(subtrees_), s);
    if (it == last)
        return -1;
    return static_cast<int>(it - first);
}

std::vector<node*> node::IteratePrefix()
{
    stack<node*> stack;
    stack.push(this);
    vector<node*> prefixList;
    while (!stack.empty())
    {
        auto node = stack.top();
        prefixList.push_back(node);
        stack.pop();

        for (auto s : node->Subtrees())
            stack.push(s);
    }
    return prefixList;
}

std::vector<node*> node::IterateBreadth()
{
    vector<node*> nodes{ this };
    size_t i = 0;
    while (i != nodes.size())
    {
        for (auto s : nodes[i]->Subtrees())
            nodes.push_back(s);
        ++i;
    }
    return nodes;
}
