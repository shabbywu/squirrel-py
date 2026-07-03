# PySquirrel - A Python binding for Squirrel programming language

Squirrel is a high level imperative, object-oriented programming language, designed to be a light-weight scripting language that fits in the size, memory bandwidth, and real-time requirements of applications like video games.
PySquirrel is intended to be a comprehensive binding of Squirrel 3 for Python.

![github-stars][stars-badge]

| CI | status |
| --- | --- |
| MSVC 2022 | [![AppVeyor][appveyor-badge]][appveyor-link] |
| pip builds | [![Pip Actions Status][actions-pip-badge]][actions-pip-link] |
| [cibuildwheel][] | [![Wheels Actions Status][actions-wheels-badge]][actions-wheels-link] |

[cibuildwheel]:          https://cibuildwheel.readthedocs.io
[stars-badge]:             https://img.shields.io/github/stars/shabbywu/squirrel-py?style=social
[actions-pip-link]:        https://github.com/shabbywu/squirrel-py/actions/workflows/pip.yml
[actions-pip-badge]:       https://github.com/shabbywu/squirrel-py/workflows/Pip/badge.svg
[actions-wheels-link]:     https://github.com/shabbywu/squirrel-py/actions/workflows/wheels.yml
[actions-wheels-badge]:    https://github.com/shabbywu/squirrel-py/workflows/Wheels/badge.svg
[appveyor-link]:           https://ci.appveyor.com/project/shabbywu/squirrel-py
[appveyor-badge]:          https://ci.appveyor.com/api/projects/status/0ns7hel71jbl7fxg?svg=true

An project built with [pybind11](https://github.com/pybind/pybind11).
This requires Python 3.7+; for older versions of Python, check the commit
history.

## Installation

`pip install squirrel-lang`

## Installing from source

Initialize the vendored native sources first:

```bash
git submodule update --init --recursive
```

The build uses vcpkg automatically when `VCPKG_ROOT` is set or `vcpkg` is on
`PATH`. Without vcpkg, install the Python build helpers first:

```bash
python -m pip install cmake pybind11
python -m pip install .
```

Legacy `python setup.py install` is also supported for local installs, but
`python -m pip install .` is the recommended path.

Debug trace macros are disabled by default. Enable them explicitly when needed:

```bash
TRACE_CONTAINER_GC=1 TRACE_OBJECT_CAST=1 python -m pip install .
```

## Test call

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
