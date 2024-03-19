from squirrel import SQVM


def test_example_code():
    vm = SQVM()
    vm.execute(
        """
    sq <- {
    function say () {
        return "hello world"
    }
    }
    """
    )
    sq = vm.get_roottable()["sq"]
    assert str(sq.say()) == "hello world"
