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


class A:
    def __init__(self, rt):
        self.rt = rt

    def echo(self):
        return self.rt.message


def test_instance():
    vm = SQVM()
    rt = vm.get_roottable()
    a = A(rt)
    vm.execute('message <- "hello"; container <- {}')
    rt.container.echo = a.echo
    assert rt.container.echo() == "hello"
