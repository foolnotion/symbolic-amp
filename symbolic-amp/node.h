#ifndef NODE_H
#define NODE_H

#include <vector>
#include <stack>
#include <sstream>
#include <unordered_map>
#include "random.h"

class dataset;

enum op_code { ADD, SUB, MUL, DIV, NEG, EXP, LOG, CONSTANT, VARIABLE };

class node
{
private:
  op_code opcode_;
  std::string name_;
  std::vector<node*> subtrees_;
  node* parent_;

  int length_;
  int depth_;
  double value_;
  double weight_;

public:
  virtual ~node();

  node(op_code opcode) : opcode_(opcode), parent_(nullptr) {
    switch(opcode) {
    case ADD:
      name_ = "+";
      break;
    case SUB:
      name_ = "-";
      break;
    case DIV:
      name_ = "/";
      break;
    case MUL:
      name_ = "-";
      break;
    case EXP:
      name_ = "exp";
      break;
    case LOG:
      name_ = "log";
      break;
    }
  }


  node(op_code opcode, const std::string& name, node* parent = nullptr)
    : opcode_(opcode), name_(name), parent_(parent), length_(0), depth_(0), value_(0), weight_(0) {}

  node(const node& other) : node(other.opcode_, other.name_, other.parent_) {}

  node* Clone() const;

  static node* Random(random* rnd, std::unordered_map<std::string, std::vector<double>>& data, int max_depth);

  virtual std::string ToString() const {
    std::stringstream ss;
    if (opcode_ == CONSTANT)
      ss << value_;
    else if (opcode_ == VARIABLE)
      ss << weight_ << " " << name_;
    else 
      ss << name_;
    return ss.str();
  }

  node const * GetParent() const { return parent_; }
  void SetParent(node* parent) { parent_ = parent; }
  std::vector<node*> const & Subtrees() const { return subtrees_; }
  int SubtreeCount() const { return static_cast<int>(subtrees_.size()); }
  op_code GetOpCode() const { return opcode_; }

  const std::string& GetName() const { return name_; }
  void SetName(const std::string& name) { name_ = name; }
  double GetValue() const { return value_; }
  void SetValue(double value) { value_ = value; }
  double GetWeight() const { return weight_; }
  void SetWeight(double weight) { weight_ = weight; }
  int GetLength();
  int GetDepth();
  void AddSubtree(node* s);
  void InsertSubtree(node *s, int index);
  void RemoveSubtree(node* s);
  int IndexOfSubtree(node *s) const;

  // traversal
  std::vector<node*> IteratePrefix();
  std::vector<node*> IterateBreadth();

  static node* add() { return new node(ADD, "+"); }
  static node* sub() { return new node(SUB, "-"); }
  static node* mul() { return new node(MUL, "*"); }
  static node* div() { return new node(DIV, "/"); }
  static node* neg() { return new node(NEG, "!"); }
  static node* exp() { return new node(EXP, "exp"); }
  static node* log() { return new node(LOG, "log"); }
  static node* constant(double value)
  {
    auto n = new node(CONSTANT, "C");
    n->SetValue(value);
    return n;
  }
  static node* variable(const std::string& name, double weight = 1)
  {
    auto v = new node(VARIABLE, name);
    v->SetWeight(weight);
    return v;
  }
};
#endif // NODE_H
