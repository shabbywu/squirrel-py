#pragma once
#include <stdexcept>
#include <string>

namespace sqbinding {
    class value_error : public std::runtime_error {
        public:
            value_error(std::string& message): std::runtime_error(message) {};
            value_error(std::string&& message): std::runtime_error(message) {};
    };

    class key_error : public std::runtime_error {
        public:
            key_error(std::string& message): std::runtime_error(message) {};
            key_error(std::string&& message): std::runtime_error(message) {};
    };

    class index_error : public std::runtime_error {
        public:
            index_error(std::string& message): std::runtime_error(message) {};
            index_error(std::string&& message): std::runtime_error(message) {};
    };
}
