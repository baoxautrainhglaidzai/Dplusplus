#pragma once

#include <stdexcept>

namespace dpp {

class Error : public std::runtime_error {
public:
    using std::runtime_error::runtime_error;
};

class LexerError : public Error {
public:
    using Error::Error;
};

class ParserError : public Error {
public:
    using Error::Error;
};

class RuntimeError : public Error {
public:
    using Error::Error;
};

}  // namespace dpp
