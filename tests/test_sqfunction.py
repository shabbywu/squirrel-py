import math
import pytest
from squirrel import SQVM


def test_sqclosure_call():
    vm = SQVM()
    rt = vm.get_roottable()
    say = vm.execute(
        """
    env <- {
        prefix = "hello "
        function say(who) {
            return prefix + who
        }
    }
    return env.say
    """
    )
    with pytest.raises(RuntimeError, match="the index 'prefix' does not exist"):
        assert say("world") == "hello world"
    assert say.bindenv(rt.env)("world") == "hello world"
    assert say.call(vm.execute('return {"prefix": "hi "}'), "world") == "hi world"


def test_sqnativeclosure():
    vm = SQVM()
    rt = vm.get_roottable()
    env = vm.execute("return {}")
    assert rt.log(1) == 0
    assert math.isclose(rt.log.bindenv(env)(math.e), 1, rel_tol=1e-5)
