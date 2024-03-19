import math
from squirrel import SQVM


def test_crud():
    vm = SQVM()
    rt = vm.get_roottable()
    d = {}
    rt["pydict"] = d
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
    assert type(d["table"]) is type(rt)
    assert d["int_"] == 1
    assert math.isclose(d["float_"], 1.2, rel_tol=1e6)
