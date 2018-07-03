#pragma once

#include <map>
#include <sstream>
#include <iostream>
#include "node.h"

namespace {
  namespace render_chars {
    static const char JunctionDown = '┬';
    static const char HorizontalLine = '─';
    static const char VerticalLine = '│';
    static const char JunctionRight = '├';
    static const char CornerRight = '└';
  }

  std::map<op_code, std::string> names = {
      { ADD, "add" },
      { SUB, "sub" },
      { MUL, "mul" },
      { DIV, "div" },
      { NEG, "neg" },
      { EXP, "exp" },
      { LOG, "log" },
      { CONSTANT, "C" },
      { VARIABLE, "V" }
  };
}

class hierarchical_formatter
{
public:
  static std::string format(node* node, const std::string prefix = "") {
    std::ostringstream ss;
    format(node, prefix, ss);
    return ss.str();
  }

private:
  static void format(node* node, const std::string& prefix, std::ostringstream& ss)
  {
    if (node->SubtreeCount() > 0)
    {
      std::string label = names[node->GetOpCode()];
      ss << label;
      std::string padding = prefix + std::string(label.length(), ' ');
      auto & subtrees = node->Subtrees();
      for (size_t i = 0; i < subtrees.size(); ++i)
      {
        char connector, extender = ' ';
        if (i == 0)
        {
          ss << padding;
          if (subtrees.size() > 1)
          {
            connector = render_chars::JunctionDown;
            extender = render_chars::VerticalLine;
          }
          else {
            connector = render_chars::HorizontalLine;
            extender = ' ';
          }
        }
        else {
 //         ss << padding;
          if (i == node->SubtreeCount() - 1) {
            connector = render_chars::CornerRight;
            extender = ' ';
          }
          else {
            connector = render_chars::JunctionRight;
            extender = render_chars::VerticalLine;
          }
        }
        ss << connector + render_chars::HorizontalLine;
        std::string new_prefix = padding + extender + ' ';
        format(subtrees[i], new_prefix, ss);
      }
    }
    else {
      switch (node->GetOpCode()) {
      case CONSTANT:
        ss << " " << node->GetValue();
      case VARIABLE:
        ss << " " << node->GetWeight() << " " << node->GetName();
      default: break;
      }
      ss << std::endl;
    }
  }
};
