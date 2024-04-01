from squirrel import SQVM


def test_execute():
    vm = SQVM()
    ret = vm.execute(
        """
    return {
        a = {}
        b = []
    }
    """
    )
    assert vm.collectgarbage() == 0
    assert sorted(ret.keys()) == ["a", "b"]
