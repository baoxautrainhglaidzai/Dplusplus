#pragma once

#include <memory>
#include <string>
#include <variant>
#include <vector>

namespace dpp {

class Callable;
struct ListValue;

using CallablePtr = std::shared_ptr<Callable>;
using ListPtr = std::shared_ptr<ListValue>;
using Value = std::variant<std::monostate, double, bool, std::string, ListPtr, CallablePtr>;

struct ListValue {
    std::vector<Value> elements;
};

std::string stringifyValue(const Value& value);
bool valuesEqual(const Value& left, const Value& right);

}  // namespace dpp
