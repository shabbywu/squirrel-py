import math
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
    assert math.isclose(d["float_"], 1.2, rel_tol=1e6)


def test_read_from_squirrel():
    vm = SQVM()
    vm.get_roottable()["d"] = {
        "say": lambda: "hello world",
        "foo": "foo",
        "dict": {},
        "int_": 1,
        "float_": 1.2,
    }
    vm.execute("r1 <- d.say()")
    vm.execute("r2 <- d.foo")
    vm.execute("r3 <- d.dict")
    vm.execute("r4 <- d.int_")
    vm.execute("r5 <- d.float_")
