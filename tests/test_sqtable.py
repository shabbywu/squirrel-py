import pytest
from squirrel import SQVM


def test_crud():
    vm = SQVM()
    vm.execute(
        """
    sq <- {
        say = function  () {
            return "hello world";
        }
        test_get = function () {
            return flag;
        }

    }
    """
    )
    sq = vm.get_roottable()["sq"]
    sq["flag"] = 1
    assert sq.test_get() == 1
    del sq["flag"]
    with pytest.raises(RuntimeError, match="the index 'flag' does not exist"):
        assert sq.test_get() == 1
    sq["flag"] = 2
    assert sq.test_get() == 2
    sq["flag"] = "test"
    assert str(sq.test_get()) == "test"


def test_get():
    vm = SQVM()
    vm.execute(
        """
        local gt = getroottable();
        who <- "world"
        say <- function  () {
            return "hello " + who;
        }
        a <- {
               who = "a"
               say = say
        }
        b <- {
               who = "b"
               say = say
        }
    """
    )

    rt = vm.get_roottable()
    glboal_say = rt.say
    a_say = rt.a.say
    b_say = rt.b.say

    assert a_say() == "hello a"
    assert b_say() == "hello b"
    assert glboal_say() == "hello world"
