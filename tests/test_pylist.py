import math
from dataclasses import dataclass
from squirrel import SQVM
from squirrel import SQTable


def test_set_from_squirrel():
    vm = SQVM()
    pylist = []
    vm.get_roottable()["pylist"] = pylist
    vm.execute(
        """
    pylist.append(function () {
        return "hello world"
    })
    pylist.append("foo")
    pylist.append({})
    pylist.append(1)
    pylist.append(1.2)
    """
    )
    assert len(pylist) == 5
    assert str(pylist[0]()) == "hello world"
    assert str(pylist[1]) == "foo"
    assert type(pylist[2]) is SQTable
    assert pylist[3] == 1
    assert math.isclose(pylist[4], 1.2, rel_tol=1e-6)

    vm.execute(
        """
    pylist[0] = 1
    pylist[1] = 2
    pylist[2] = 3
    pylist.pop(-2)
    pylist.pop(2)
    """
    )
    assert len(pylist) == 3
    assert pylist[0] == 1
    assert pylist[1] == 2
    assert math.isclose(pylist[2], 1.2, rel_tol=1e-6)


def test_read_from_squirrel():
    @dataclass
    class Dummy:
        a: int
        b: float

    def _assert(res):
        assert res

    vm = SQVM()
    vm.get_roottable()["pylist"] = [
        lambda: "hello world",
        "foo",
        {"flag": "flag"},
        1,
        1.2,
        Dummy(a=1, b=2.2),
    ]
    vm.get_roottable().bindfunc("assert", _assert)
    vm.execute('assert(pylist[0]() == "hello world")')
    vm.execute('assert(pylist[1] == "foo")')
    vm.execute('assert(pylist[2].flag == "flag")')
    vm.execute("assert(pylist[3] == 1)")
    vm.execute("assert(pylist[4] == 1.2)")
