from squirrel import SQVM


def test_call_python():
    vm = SQVM()
    rt = vm.get_roottable()

    def say(who):
        return "hello " + who

    rt.say = say
    assert vm.execute('return say("world")') == "hello world"


def test_call_sqmetamethod():
    vm = SQVM()
    rt = vm.get_roottable()

    def say(who):
        return "hello " + who

    rt.say = say
    assert vm.execute("return say._call.getinfos().name") == "say"
    assert vm.execute('return say._rawcall("world")') == "hello world"
    assert vm.execute('return say._rawcall.acall([null, "world"])') == "hello world"


def test_override_sqfun():
    vm = SQVM()
    rt = vm.get_roottable()
    vm.execute(
        """
    env <- {
        prefix = "hello "
        function say(who) {
            return prefix + who
        }
    }
    """
    )

    def say(who):
        return "hi " + who

    rt.env.say = say
    assert vm.execute('return env.say("world")') == "hi world"


def test_decorate_sqfunc():
    vm = SQVM()
    rt = vm.get_roottable()
    vm.execute(
        """
    env <- {
        prefix = "hello "
        function say(who) {
            return prefix + who
        }
    }
    """
    )
    original_say = rt.env.say

    def say(who):
        env = vm.execute('return {"prefix": "hi "}')
        return original_say.bindenv(env)(who)

    rt.env.say = say
    assert vm.execute('return env.say("world")') == "hi world"


def test_get_caller_stack():
    vm = SQVM()
    rt = vm.get_roottable()
    get_caller = vm.execute(
        """
    return function () {
        # 0 -> getstackinfos
        # 1 -> get_caller
        # 2 -> caller
        return getstackinfos(2)
    }
    """
    )

    def pycaller():
        caller = get_caller()
        if caller is None:
            return
        return caller.func

    rt.caller = pycaller
    assert vm.execute("return caller()") == "pycaller"
    assert pycaller() is None
