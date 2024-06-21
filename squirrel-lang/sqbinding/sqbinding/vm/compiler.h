#ifndef _SQBINDING_COMPILER_H_
#define _SQBINDING_COMPILER_H_

#include <squirrel.h>
#include <string>
#include <sstream>
#include <iostream>
#include <cstring>
#include <memory.h>
#include "sqbinding/detail/common/errors.hpp"

void printCompileError(HSQUIRRELVM, const SQChar * desc, const SQChar * source, SQInteger line, SQInteger column);
std::string compile(std::string sourcecode, std::string sourcename);
std::string compile_bb(std::string sourcecode, std::string sourcename);

#endif
