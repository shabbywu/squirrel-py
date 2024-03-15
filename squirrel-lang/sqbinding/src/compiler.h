#ifndef _SQBINDING_COMPILER_H_
#define _SQBINDING_COMPILER_H_

#include <squirrel.h>
#include <string>
#include <sstream>
#include <iostream>
#include <cstring>
#include <memory.h>
#include <pybind11/pybind11.h>

void printCompileError(HSQUIRRELVM, const SQChar * desc, const SQChar * source, SQInteger line, SQInteger column);
pybind11::bytes compile(std::string sourcecode, std::string sourcename);
pybind11::bytes compile_bb(std::string sourcecode, std::string sourcename);

#endif