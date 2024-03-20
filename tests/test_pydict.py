import math
from dataclasses import dataclass
from squirrel import SQVM
from squirrel import SQTable


def test_set_from_squirrel():
    vm = SQVM()
    pydict = {}
    vm.get_roottable()["pydict"] = pydict
    vm.execute(
        """
    pydict.say = function () {
        return "hello world"
    }
    pydict.foo = "foo"
    pydict.table = {}
    pydict.int_ = 1
    pydict.float_ = 1.2
    """
    )
    assert str(pydict["say"]()) == "hello world"
    assert str(pydict["foo"]) == "foo"
    assert type(pydict["table"]) is SQTable
    assert pydict["int_"] == 1
    assert math.isclose(pydict["float_"], 1.2, rel_tol=1e-6)


def test_read_from_squirrel():
    @dataclass
    class Dummy:
        a: int
        b: float

    def _assert(res):
        assert res

    vm = SQVM()
    vm.get_roottable()["pydict"] = {
        "say": lambda: "hello world",
        "foo": "foo",
        "dict": {"flag": "flag"},
        "int_": 1,
        "float_": 1.2,
        "dummy": Dummy(a=1, b=2.2),
    }
    vm.get_roottable().bindfunc("assert", _assert)
    vm.execute('assert(pydict.say() == "hello world")')
    vm.execute('assert(pydict.foo == "foo")')
    vm.execute('assert(pydict.dict.flag == "flag")')
    vm.execute("assert(pydict.int_ == 1)")
    vm.execute("assert(pydict.float_ == 1.2)")
    vm.execute("assert(pydict.dummy.a == 1)")
    vm.execute("assert(pydict.dummy.b == 2.2)")
