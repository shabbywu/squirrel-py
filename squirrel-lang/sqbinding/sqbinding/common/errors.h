#pragma once
#include <stdexcept>
#include <string>

namespace sqbinding {
    class value_error : std::runtime_error {
        public:
            value_error(std::string& message): std::runtime_error(message) {};
            value_error(std::string&& message): std::runtime_error(message) {};
    };
}
