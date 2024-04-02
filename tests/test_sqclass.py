from squirrel import SQVM


def test_override_classmethod():
    vm = SQVM()
    rt = vm.get_roottable()
    i = vm.execute(
        """
    class Class {
        prefix = null
        constructor(prefix){
            this.prefix = prefix
        }
        function say(who) {
            return prefix + who
        }
    }
    return Class("hello ")
    """
    )

    def say(who):
        return "hi " + who

    rt.Class.say = say
    assert i.say("world") == "hi world"
    assert vm.execute('return Class("hello ").say("world")') == "hi world"


def test_decorate_sqfunc():
    vm = SQVM()
    rt = vm.get_roottable()
    vm.execute(
        """
    class Class {
        prefix = null
        constructor(prefix){
            this.prefix = prefix
        }
        function say(who) {
            return prefix + who
        }
    }
    i <- Class("hello ")
    """
    )
    original_say = rt.Class.say

    def say(who):
        env = vm.execute('return {"prefix": "你好 "}')
        return original_say.bindenv(env)(who)

    rt.Class.say = say
    assert vm.execute('return i.say("world")') == "你好 world"
