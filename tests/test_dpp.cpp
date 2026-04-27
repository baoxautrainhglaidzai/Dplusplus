#include <fstream>
#include <functional>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

#include "dpp/error.hpp"
#include "dpp/runtime.hpp"

namespace {

struct Failure : public std::runtime_error {
    using std::runtime_error::runtime_error;
};

void expectEqual(const std::string& actual, const std::string& expected, const std::string& label) {
    if (actual != expected) {
        throw Failure(label + " mismatch.\nExpected:\n" + expected + "\nActual:\n" + actual);
    }
}

void expectThrows(const std::function<void()>& action, const std::string& expected_message, const std::string& label) {
    try {
        action();
    } catch (const dpp::Error& error) {
        if (std::string(error.what()) != expected_message) {
            throw Failure(label + " mismatch.\nExpected: " + expected_message + "\nActual: " + error.what());
        }
        return;
    }

    throw Failure(label + " did not throw.");
}

std::string runProgram(const std::string& source, std::vector<std::string> inputs = {}, double random_value = 0.25) {
    std::vector<std::string> outputs;
    std::size_t input_index = 0;

    dpp::runSource(
        source,
        [&inputs, &input_index]() {
            if (input_index >= inputs.size()) {
                throw Failure("No input values left for test.");
            }
            return inputs[input_index++];
        },
        [&outputs](const std::string& line) {
            outputs.push_back(line);
        },
        [random_value]() {
            return random_value;
        });

    std::ostringstream stream;
    for (std::size_t index = 0; index < outputs.size(); ++index) {
        if (index > 0) {
            stream << '\n';
        }
        stream << outputs[index];
    }
    return stream.str();
}

std::string readFile(const std::string& path) {
    std::ifstream file(path);
    if (!file) {
        throw Failure("Unable to open file: " + path);
    }

    std::ostringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
}

void testVariablesAndArithmetic() {
    expectEqual(
        runProgram(R"(
            let x = 5
            let y = 10
            print x + y
        )"),
        "15",
        "variables and arithmetic");
}

void testFunctionsAndReturnValues() {
    expectEqual(
        runProgram(R"(
            let x = 5
            let y = 10

            fn multiply(a, b) {
                return a * b
            }

            let result = multiply(x, y)
            print result
        )"),
        "50",
        "functions and returns");
}

void testConditionals() {
    expectEqual(
        runProgram(R"(
            let x = 12

            if x > 10 {
                print "big"
            } else {
                print "small"
            }
        )"),
        "big",
        "conditionals");
}

void testInclusiveDescendingLoop() {
    expectEqual(
        runProgram(R"(
            let total = 0

            for i in 3..1 {
                total = total + i
            }

            print total
        )"),
        "6",
        "descending loop");
}

void testBuiltins() {
    expectEqual(
        runProgram(R"(
            print sqrt(81)
            print random()
            print input()
        )", {"hello"}, 0.5),
        "9\n0.5\nhello",
        "builtins");
}

void testFriendlyStringConcatenation() {
    expectEqual(
        runProgram(R"(
            let stars = 3
            print "Friendly stars: " + stars
        )"),
        "Friendly stars: 3",
        "string concatenation");
}

void testUndefinedVariableError() {
    expectThrows(
        []() {
            static_cast<void>(runProgram("print missing"));
        },
        "Error: Undefined variable 'missing' at line 1",
        "undefined variable error");
}

void testReturnOutsideFunction() {
    expectThrows(
        []() {
            static_cast<void>(runProgram("return 1"));
        },
        "Error: 'return' can only be used inside a function",
        "top-level return error");
}

void testMoonlitTeaPartyExample() {
    expectEqual(
        runProgram(readFile("examples/moonlit_tea_party.dpp"), {"honey", "sing", "pour"}),
        "=== Moonlit Tea Party ===\n"
        "A shy fox is hosting a tea party for three starry guests.\n"
        "Help the fox make the forest feel warm and welcome.\n"
        "Lantern 1 is glowing.\n"
        "Lantern 2 is glowing.\n"
        "Lantern 3 is glowing.\n"
        "Pantry: choose honey or mint.\n"
        "Golden honey joins the table.\n"
        "Welcome: choose sing or smile.\n"
        "The guests clap along to your moon song.\n"
        "Serve: choose pour or wave.\n"
        "Honey tea sparkles in every cup.\n"
        "Tea party joy: 5\n"
        "Ending: The fox becomes the most beloved host in the forest.",
        "moonlit tea party example");
}

void testSystemsDemoExample() {
    expectEqual(
        runProgram(readFile("examples/systems_demo.dpp")),
        "=== Systems Demo ===\n"
        "Odd values: [7, 9, 15]\n"
        "Count: 3\n"
        "Last odd: 15\n"
        "After pop: [7, 9]",
        "systems demo example");
}

void testCoreBasicsExample() {
    expectEqual(
        runProgram(readFile("examples/core_basics.dpp")),
        "big\n1\n2\n3\n50",
        "core basics example");
}

void testWhileBreakContinueAndModulo() {
    expectEqual(
        runProgram(R"(
            let i = 0
            let sum = 0

            while i < 10 {
                i = i + 1

                if i % 2 == 0 {
                    continue
                }

                if i > 7 {
                    break
                }

                sum = sum + i
            }

            print sum
        )"),
        "16",
        "while loop with break/continue and modulo");
}

void testListsAndIndexAssignments() {
    expectEqual(
        runProgram(R"(
            let data = [10, 20, 30]
            data[1] = 99
            push(data, 7)
            print data[0]
            print data[1]
            print data[3]
            print len(data)
            print pop(data)
            print len(data)
            print "abc"[1]
        )"),
        "10\n99\n7\n4\n7\n3\nb",
        "lists, indexing, and list builtins");
}

void testLogicalOperatorsShortCircuit() {
    expectEqual(
        runProgram(R"(
            let hit = 0

            fn mark() {
                hit = hit + 1
                return true
            }

            if true || mark() {
                print "or"
            }

            if false && mark() {
                print "and"
            } else {
                print "skip"
            }

            if !false and true {
                print "ok"
            }

            print hit
        )"),
        "or\nskip\nok\n0",
        "logical operators and short-circuit");
}

void testBreakOutsideLoopError() {
    expectThrows(
        []() {
            static_cast<void>(runProgram("break"));
        },
        "Error: 'break' can only be used inside a loop",
        "break outside loop error");
}

void testContinueOutsideLoopError() {
    expectThrows(
        []() {
            static_cast<void>(runProgram("continue"));
        },
        "Error: 'continue' can only be used inside a loop",
        "continue outside loop error");
}

void testIndexOutOfBoundsError() {
    expectThrows(
        []() {
            static_cast<void>(runProgram(R"(
                let data = [1, 2]
                print data[3]
            )"));
        },
        "Error: Index out of bounds at line 3",
        "index bounds error");
}

}  // namespace

int main() {
    try {
        testVariablesAndArithmetic();
        testFunctionsAndReturnValues();
        testConditionals();
        testInclusiveDescendingLoop();
        testBuiltins();
        testFriendlyStringConcatenation();
        testUndefinedVariableError();
        testReturnOutsideFunction();
        testMoonlitTeaPartyExample();
        testSystemsDemoExample();
        testCoreBasicsExample();
        testWhileBreakContinueAndModulo();
        testListsAndIndexAssignments();
        testLogicalOperatorsShortCircuit();
        testBreakOutsideLoopError();
        testContinueOutsideLoopError();
        testIndexOutOfBoundsError();
    } catch (const Failure& failure) {
        std::cerr << failure.what() << '\n';
        return 1;
    } catch (const std::exception& error) {
        std::cerr << "Unexpected failure: " << error.what() << '\n';
        return 1;
    }

    std::cout << "All native D++ tests passed.\n";
    return 0;
}
