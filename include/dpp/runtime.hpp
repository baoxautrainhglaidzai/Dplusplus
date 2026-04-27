#pragma once

#include <string>

#include "dpp/ast.hpp"
#include "dpp/interpreter.hpp"

namespace dpp {

ast::Program parseSource(const std::string& source);
void runSource(const std::string& source, InputProvider input_provider = {}, OutputHandler output_handler = {}, RandomProvider random_provider = {});

}  // namespace dpp
