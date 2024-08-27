#pragma once
#include <cstring>
#include <iostream>
#include <memory.h>
#include <squirrel.h>
#include <stdarg.h>

static void printStdout(HSQUIRRELVM vm, const SQChar *format, ...) {
    va_list vl;
    va_start(vl, format);
    char *text = va_arg(vl, char *);
    std::cout << text;
    va_end(vl);
}

static void printStdErr(HSQUIRRELVM vm, const SQChar *format, ...) {
    va_list vl;
    va_start(vl, format);
    char *text = va_arg(vl, char *);
    std::cerr << text;
    va_end(vl);
}

static void printCompileError(HSQUIRRELVM, const SQChar *desc, const SQChar *source, SQInteger line, SQInteger column) {
    std::cerr << "desc:" << desc << std::endl;
    std::cerr << "source:" << source << std::endl;
    std::cerr << "line:" << line << std::endl;
    std::cerr << "column:" << column << std::endl;
}
