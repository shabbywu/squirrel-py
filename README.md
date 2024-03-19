PySquirrel - A Python binding for Squirrel programming language.
==============
Squirrel is a high level imperative, object-oriented programming language, designed to be a light-weight scripting language that fits in the size, memory bandwidth, and real-time requirements of applications like video games.
PySquirrel is intended to be a comprehensive binding of Squirrel 3 for Python.


![github-stars][stars-badge]

|      CI              | status |
|----------------------|--------|
| Linux/macOS Travis   | [![Travis-CI][travis-badge]][travis-link] |
| MSVC 2019            | [![AppVeyor][appveyor-badge]][appveyor-link] |
| pip builds           | [![Pip Actions Status][actions-pip-badge]][actions-pip-link] |
| [cibuildwheel][]   | [![Wheels Actions Status][actions-wheels-badge]][actions-wheels-link] |

[cibuildwheel]:          https://cibuildwheel.readthedocs.io
[stars-badge]:             https://img.shields.io/github/stars/shabbywu/squirrel-py?style=social
[actions-pip-link]:        https://github.com/shabbywu/squirrel-py/actions/workflows/pip.yml
[actions-pip-badge]:       https://github.com/shabbywu/squirrel-py/workflows/Pip/badge.svg
[actions-wheels-link]:     https://github.com/shabbywu/squirrel-py/actions/workflows/wheels.yml
[actions-wheels-badge]:    https://github.com/shabbywu/squirrel-py/workflows/Wheels/badge.svg
[travis-link]:             https://travis-ci.org/shabbywu/squirrel-py
[travis-badge]:            https://travis-ci.org/shabbywu/squirrel-py.svg?branch=master&status=passed
[appveyor-link]:           https://ci.appveyor.com/project/shabbywu/squirrel-py
[appveyor-badge]:          https://ci.appveyor.com/api/projects/status/f04io15t7o63916y

An project built with [pybind11](https://github.com/pybind/pybind11).
This requires Python 3.7+; for older versions of Python, check the commit
history.

Installation
------------

 - `pip install squirrel-lang`


License
-------

pybind11 is provided under a BSD-style license that can be found in the LICENSE
file. By using, distributing, or contributing to this project, you agree to the
terms and conditions of this license.

Test call
---------

```python
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
assert str(sq.say()) == "hello world"
```
