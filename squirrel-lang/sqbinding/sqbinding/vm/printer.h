#pragma once
#include <squirrel.h>

void printStdout(HSQUIRRELVM vm, const SQChar *format,...);
void printStdErr(HSQUIRRELVM vm, const SQChar *format,...);
void printCompileError(HSQUIRRELVM, const SQChar * desc, const SQChar * source, SQInteger line, SQInteger column);
