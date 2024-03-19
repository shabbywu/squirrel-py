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
