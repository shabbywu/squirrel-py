from squirrel import compile


def test_compile():
    source = "print(1)"
    code = compile(source)
    assert isinstance(code, bytes)
    assert b"print" in code


# def test_compile_bb_shortword():
#     source = "print(1)"
#     code = compile_bb(source)
#     assert type(code) is bytes
#     assert b"print" in code
