from squirrel import SQVM

vm = SQVM()
vm.execute("""
sq <- {
  function say () {
    return "hello world"
  }  
}
""")
sq = vm.get_roottable()["sq"]


assert str(sq["say"]()) == "hello world"
del sq
del vm