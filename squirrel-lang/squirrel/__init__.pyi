from typing import Any, Callable

__version__: str
__author__: str


class StaticVM:
    vm: Any

    def vm_addr(self) -> int:
        ...

    def root_table(self) -> Any:
        ...

    def resolve_native_pointer(self, path: str) -> dict[str, Any]:
        ...

    def get_roottable(self) -> Any:
        ...

    def execute(
        self,
        sourcecode: str,
        env: Any = None
    ) -> Any:
        ...

    def bindfunc(
        self,
        funcname: str,
        func: Callable,
        withenv: bool = False
    ) -> None:
        ...


class tagSQObjectType:
    ...


class tagSQObjectValue:
    ...


SQVM: Callable[..., StaticVM]
get_static_vm: Callable[..., Any]


def compile(
    sourcecode: str,
    sourcename: str = "__main__"
) -> bytes:
    ...


def compile_bb(
    sourcecode: str,
    sourcename: str = "__main__"
) -> bytes:
    ...
