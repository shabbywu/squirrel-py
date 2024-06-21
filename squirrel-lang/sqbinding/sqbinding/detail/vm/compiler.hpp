#pragma once

#include <squirrel.h>
#include <string>
#include <sstream>
#include <iostream>
#include <cstring>
#include <memory.h>
#include "sqbinding/detail/common/errors.hpp"
#include "printer.hpp"

static SQInteger write_stringbuf(SQUserPointer output,SQUserPointer source,SQInteger size)
{
    return ((std::stringbuf*)output)->sputn((const char*)source, size);
}

std::string compile(std::string sourcecode, std::string sourcename) {
    HSQUIRRELVM v;
    v=sq_open(1024);
    sq_pushroottable(v);
    sq_setcompilererrorhandler(v, printCompileError);

    if(!SQ_SUCCEEDED(sq_compilebuffer(v, sourcecode.c_str(), sourcecode.length(), sourcename.c_str(), SQTrue))) {
        throw sqbinding::value_error("invalid sourcecode, failed to compile");
    }

    std::stringbuf buff;
    if(!SQ_SUCCEEDED(sq_writeclosure(v, write_stringbuf, (SQUserPointer)&buff))) {
        throw sqbinding::value_error("failed serialize closure");
    }
    sq_close(v);
    return buff.str();
}

// the standard XXTEA block cipher algorithm
#define DELTA 0x9e3779b9
#define MX(p) (((z>>5^y<<2) + (y>>3^z<<4)) ^ ((sum^y) + (key[((p)&3)^e] ^ z)))

// the key used by Battle Brothers
static uint32_t bbkey[4] = { 238473842, 20047425, 14005, 978629342 };
static void encrypt(uint32_t *values, uint32_t count, const uint32_t key[4])
{
    uint32_t rounds = 6 + 52/count, sum = 0, y, z = values[count-1];
    do
    {
        sum += DELTA;
        uint32_t e = (sum >> 2) & 3, p;
        for (p = 0; p < count-1; p++)
        {
            y = values[p+1];
            z = values[p] += MX(p);
        }
        y = values[0];
        z = values[count-1] += MX(p);
    } while (--rounds);
}

static SQInteger write_encryptedbuf(SQUserPointer output, SQUserPointer source, SQInteger length)
{

    memcpy(output, source, length);
    if (length >= 8) {
        encrypt((uint32_t*)output, length / sizeof(uint32_t), bbkey); // encrypt the data
    }
    return length;
}


static std::string compile_bb(std::string sourcecode, std::string sourcename) {
    HSQUIRRELVM v;
    v=sq_open(1024);
    sq_pushroottable(v);
    sq_setcompilererrorhandler(v, printCompileError);

    if(!SQ_SUCCEEDED(sq_compilebuffer(v, sourcecode.c_str(), sourcecode.length(), sourcename.c_str(), SQTrue))) {
        throw sqbinding::value_error("invalid sourcecode, failed to compile");
    }

    std::stringbuf buff;
    if(!SQ_SUCCEEDED(sq_writeclosure(v, write_encryptedbuf, (SQUserPointer)&buff))) {
        throw sqbinding::value_error("failed serialize closure");
    }
    sq_close(v);
    return buff.str();
}
