import math
from dataclasses import dataclass
from squirrel import SQVM
from squirrel import SQTable


class Dummy:
    def __init__(self, a, b):
        self.a = a
        self.b = b

    def __call__(self, table: SQTable):
        return "call dummy"


def test_set_from_squirrel():
    vm = SQVM()
    d = Dummy(a=1, b=2.2)
    vm.get_roottable()["pyobject"] = d
    vm.execute(
        """
    call <- pyobject()
    pyobject.a = 2
    pyobject.b = 3.3
    """
    )
    assert str(vm.get_roottable().call) == "call dummy"
    assert d.a == 2
    assert math.isclose(d.b, 3.3, rel_tol=1e-6)


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
