from _squirrel import (
    __version__,
    __author__,
    get_static_vm,
    StaticVM,
    SQUIRREL_VERSION,
    SQVM,
    compile,
    compile_bb
)
from _squirrel.types import *  # noqa: F401, F403


__all__ = [
    "__version__",
    "__author__",
    "get_static_vm",
    "StaticVM",
    "SQVM",
    "compile",
    "compile_bb",
    "SQUIRREL_VERSION",
]
