import math
from dataclasses import dataclass
from squirrel import SQVM
from squirrel import SQTable


def test_set_from_squirrel():
    vm = SQVM()
    d = {}
    vm.get_roottable()["pydict"] = d
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
    assert str(d["say"]()) == "hello world"
    assert str(d["foo"]) == "foo"
    assert type(d["table"]) is SQTable
    assert d["int_"] == 1
    assert math.isclose(d["float_"], 1.2, rel_tol=1e-6)


def test_read_from_squirrel():
    @dataclass
    class Dummy:
        a: int
        b: float

    def _assert(res):
        assert res

    vm = SQVM()
    vm.get_roottable()["d"] = {
        "say": lambda: "hello world",
        "foo": "foo",
        "dict": {"flag": "flag"},
        "int_": 1,
        "float_": 1.2,
        "dummy": Dummy(a=1, b=2.2),
    }
    vm.get_roottable().bindfunc("assert", _assert)
    vm.execute('assert(d.say() == "hello world")')
    vm.execute('assert(d.foo == "foo")')
    vm.execute('assert(d.dict.flag == "flag")')
    vm.execute("assert(d.int_ == 1)")
    vm.execute("assert(d.float_ == 1.2)")
    vm.execute("assert(d.dummy.a == 1)")
    vm.execute("assert(d.dummy.b == 2.2)")
