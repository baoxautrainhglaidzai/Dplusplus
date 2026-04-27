#include <fstream>
#include <iostream>
#include <sstream>
#include <string>

#include "dpp/error.hpp"
#include "dpp/runtime.hpp"

int main(int argc, char* argv[]) {
    if (argc != 2) {
        std::cerr << "Usage: dpp <path-to-source-file>\n";
        return 1;
    }

    std::ifstream file(argv[1]);
    if (!file) {
        std::cerr << "Error: File not found: " << argv[1] << '\n';
        return 1;
    }

    std::ostringstream buffer;
    buffer << file.rdbuf();

    try {
        dpp::runSource(buffer.str());
    } catch (const dpp::Error& error) {
        std::cerr << error.what() << '\n';
        return 1;
    }

    return 0;
}
