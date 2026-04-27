#pragma once

#include <memory>
#include <string>
#include <variant>

namespace dpp {

class Callable;

using CallablePtr = std::shared_ptr<Callable>;
using Value = std::variant<std::monostate, double, bool, std::string, CallablePtr>;

std::string stringifyValue(const Value& value);
bool valuesEqual(const Value& left, const Value& right);

}  // namespace dpp
