#pragma once
#include <string>
#include <vector>
#include <unordered_map>


class dataset {
public:
  dataset() {}
  ~dataset() {}

  void add(const std::string& variable, std::vector<double>& values)
  {
    if (contains(variable))
      throw std::exception("variable is already present in the dataset.");

    variable_indices[variable] = static_cast<int>(variables.size());
    variables.push_back(variable);
    variable_values.push_back(values);
  }

  void remove(const std::string& variable, std::vector<double>& values)
  {
    if (!contains(variable))
      throw std::exception("the variable is not present in the dataset.");

    auto i = variable_indices[variable];
    variable_indices.erase(variable);
    variables.erase(begin(variables) + i);
    variable_values.erase(begin(variable_values) + i);
  }

  std::vector<double>& operator[](const std::string& variable) 
  {
    if (!contains(variable))
      throw std::exception("the variable is not present in the dataset.");

    auto index = variable_indices[variable];
    return variable_values[index];
  }

  bool contains(const std::string& variable) const
  {
    return variable_indices.find(variable) != end(variable_indices);
  }
  std::vector<std::string> Variables() const { return variables; }

private:
  std::vector<std::string> variables;
  std::unordered_map<std::string, int> variable_indices;
  std::vector<std::vector<double>> variable_values;

};
