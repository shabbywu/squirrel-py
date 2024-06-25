import pytest
from squirrel import SQVM


def test_crud():
    vm = SQVM()
    vm.execute(
        """
    sq <- [1, {
        say = function () {
            return "hello world"
        }
    }]
    """
    )
    sq = vm.get_roottable()["sq"]
    assert sq[0] == 1
    assert sq[1].say() == "hello world"
    with pytest.raises(IndexError):
        sq[2]


def test_iterator():
    vm = SQVM()
    sq = vm.execute(
        """
        return [1,2,3,4,5,6,7,8,9,10]
    """
    )
    idx = 0
    for i in sq:
        idx += 1
        assert i == idx
    assert idx == 10
