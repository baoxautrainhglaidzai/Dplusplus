#include "dpp/runtime.hpp"

#include <utility>

#include "dpp/interpreter.hpp"
#include "dpp/lexer.hpp"
#include "dpp/parser.hpp"

namespace dpp {

ast::Program parseSource(const std::string& source) {
    Lexer lexer(source);
    Parser parser(lexer.tokenize());
    return parser.parse();
}

void runSource(const std::string& source, InputProvider input_provider, OutputHandler output_handler, RandomProvider random_provider) {
    auto program = parseSource(source);
    Interpreter interpreter(std::move(input_provider), std::move(output_handler), std::move(random_provider));
    interpreter.execute(program);
}

}  // namespace dpp
