import math
from squirrel import SQVM


class Dummy:
    def __init__(self, a, b):
        self.a = a
        self.b = b

    def __call__(self):
        return "call dummy"


def test_set_from_squirrel():
    vm = SQVM()
    d = Dummy(a=1, b=2.2)
    vm.get_roottable()["pyobject"] = d
    vm.execute(
        """
    pyobject.say = function () {
        return "hello world"
    }
    pyobject.a = 2
    pyobject.b = 3.3
    """
    )
    assert str(d.say()) == "hello world"
    assert d.a == 2
    assert math.isclose(d.b, 3.3, rel_tol=1e-6)


def test_read_from_squirrel():
    def _assert(res):
        assert res

    vm = SQVM()
    vm.get_roottable()["pyobject"] = Dummy(a=1, b=1.2)
    vm.get_roottable().bindfunc("assert", _assert)
    vm.execute('assert(pyobject() == "call dummy")')
    vm.execute("assert(pyobject.a == 1)")
    vm.execute("assert(pyobject.b == 1.2)")
    vm.execute('assert(typeof(pyobject) == "test_pyobject.Dummy")')


class Puppy:
    def __init__(self, name):
        self.name = name

    def get_name(self):
        return self.name


def test_construct():
    vm = SQVM()
    rt = vm.get_roottable()
    rt.Dummy = Dummy
    vm.execute("dummy <- Dummy(1, 2.2)")
    dummy = rt.dummy
    assert dummy.a == 1
    assert math.isclose(dummy.b, 2.2, rel_tol=1e-6)
    rt.Puppy = Puppy
    vm.execute('puppy <- Puppy("husky")')
    puppy = rt.puppy
    assert isinstance(puppy, Puppy)
    assert puppy.get_name() == "husky"
