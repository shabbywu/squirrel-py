import os
import re
import shlex
import shutil
import subprocess
import sys
from pathlib import Path
from typing import List, Optional, Union

# Available at setup time due to pyproject.toml
# Keep access to the original install command so legacy setup.py install can
# install local build outputs without setuptools' bdist_egg/easy_install path.
import setuptools.command.install as setuptools_install
from setuptools import Extension, setup
from setuptools.command.build_ext import build_ext
from setuptools.command.install import install

__version__ = "0.2.1"

# Convert distutils Windows platform specifiers to CMake -A arguments
PLAT_TO_CMAKE = {
    "win32": "Win32",
    "win-amd64": "x64",
    "win-arm32": "ARM",
    "win-arm64": "ARM64",
}


def _has_cmake_define(cmake_args: List[str], name: str) -> bool:
    prefixes = (f"-D{name}=", f"-D{name}:")
    return any(arg == f"-D{name}" or arg.startswith(prefixes) for arg in cmake_args)


def _get_cmake_define(cmake_args: List[str], name: str) -> Optional[str]:
    prefixes = (f"-D{name}=", f"-D{name}:")
    for arg in cmake_args:
        if arg.startswith(prefixes):
            return arg.split("=", 1)[1]
    return None


def _split_cmake_args(value: str) -> List[str]:
    return shlex.split(value, posix=os.name != "nt")


# Trace macros are debug-only. Pass an explicit 0 by default so an old CMake
# cache cannot keep TRACE_* enabled after the environment variable is removed.
def _env_flag_enabled(name: str) -> bool:
    return os.environ.get(name, "").lower() in {"1", "true", "yes", "on"}


def _append_env_flag(cmake_args: List[str], name: str) -> None:
    if _has_cmake_define(cmake_args, name):
        return

    value = 1 if _env_flag_enabled(name) else 0
    cmake_args += [f"-D{name}={value}"]


def _vcpkg_toolchain_from_root(root: Union[str, os.PathLike]) -> Optional[Path]:
    toolchain = (
        Path(root).expanduser() / "scripts" / "buildsystems" / "vcpkg.cmake"
    )
    return toolchain.resolve() if toolchain.is_file() else None


# Prefer vcpkg when it is available, but do not require it for source builds.
def _find_vcpkg_toolchain() -> Optional[Path]:
    vcpkg_root = os.environ.get("VCPKG_ROOT")
    if vcpkg_root:
        toolchain = _vcpkg_toolchain_from_root(vcpkg_root)
        if toolchain:
            return toolchain

    vcpkg_executable = shutil.which("vcpkg")
    if not vcpkg_executable:
        return None

    executable_path = Path(vcpkg_executable).resolve()
    for candidate_root in (executable_path.parent, executable_path.parent.parent):
        toolchain = _vcpkg_toolchain_from_root(candidate_root)
        if toolchain:
            return toolchain

    return None


def _configure_vcpkg(cmake_args: List[str]) -> bool:
    existing_toolchain = _get_cmake_define(cmake_args, "CMAKE_TOOLCHAIN_FILE")
    if existing_toolchain:
        return Path(existing_toolchain).name == "vcpkg.cmake"

    vcpkg_toolchain = _find_vcpkg_toolchain()
    if not vcpkg_toolchain:
        return False

    cmake_args += [f"-DCMAKE_TOOLCHAIN_FILE={vcpkg_toolchain}"]
    return True


# Fallback for environments that use pip-installed pybind11 instead of vcpkg.
def _configure_pybind11_from_python(cmake_args: List[str]) -> bool:
    if _has_cmake_define(cmake_args, "pybind11_DIR"):
        return True

    try:
        import pybind11  # type: ignore
    except ImportError:
        return False

    cmake_args += [f"-Dpybind11_DIR={pybind11.get_cmake_dir()}"]
    return True


# Fail before invoking CMake so missing build prerequisites produce actionable
# setup.py errors instead of long subprocess traces.
def _require_cmake() -> None:
    if shutil.which("cmake"):
        return

    raise RuntimeError(
        "CMake is required to build squirrel-lang, but 'cmake' was not found "
        "on PATH. Install it with "
        f"'{sys.executable} -m pip install cmake' or install CMake with your "
        "system package manager, then rerun the build."
    )


def _raise_missing_pybind11() -> None:
    raise RuntimeError(
        "vcpkg was not found and the Python package 'pybind11' is not "
        "installed in this build environment. Install vcpkg and set "
        "VCPKG_ROOT or add vcpkg to PATH, or install pybind11 with "
        f"'{sys.executable} -m pip install pybind11'."
    )


# A CMakeExtension needs a sourcedir instead of a file list.
# The name must be the _single_ output extension from the CMake build.
# If you need multiple extensions, see scikit-build.
class CMakeExtension(Extension):
    def __init__(self, name: str, sourcedir: str = "") -> None:
        super().__init__(name, sources=[])
        self.sourcedir = os.fspath(Path(sourcedir).resolve())


class CMakeBuild(build_ext):
    def build_extension(self, ext: CMakeExtension) -> None:
        # Must be in this form due to bug in .resolve() only fixed in Python 3.10+
        ext_fullpath = Path.cwd() / self.get_ext_fullpath(ext.name)
        extdir = ext_fullpath.parent.resolve()

        # Using this requires trailing slash for auto-detection & inclusion of
        # auxiliary "native" libs

        debug = int(os.environ.get("DEBUG", 0)) if self.debug is None else self.debug
        cfg = "Debug" if debug else "Release"

        # CMake lets you override the generator - we need to check this.
        # Can be set with Conda-Build, for example.
        cmake_generator = os.environ.get("CMAKE_GENERATOR", "")

        cmake_args = [
            f"-DCMAKE_LIBRARY_OUTPUT_DIRECTORY={extdir}{os.sep}",
            f"-DPython_EXECUTABLE={sys.executable}",
            f"-DPYTHON_EXECUTABLE={sys.executable}",
            f"-DPython_ROOT_DIR={os.path.dirname(sys.executable)}",
            f"-DCMAKE_BUILD_TYPE={cfg}",  # not used on MSVC, but no harm
        ]
        build_args = []
        # Adding CMake arguments set as environment variable
        # (needed e.g. to build for ARM OSx on conda-forge)
        if "CMAKE_ARGS" in os.environ:
            cmake_args += _split_cmake_args(os.environ["CMAKE_ARGS"])

        _append_env_flag(cmake_args, "TRACE_CONTAINER_GC")
        _append_env_flag(cmake_args, "TRACE_OBJECT_CAST")

        # In this example, we pass in the version to C++. You might not need to.
        cmake_args += [f"-DVERSION_INFO={__version__}"]

        _require_cmake()

        using_vcpkg = _configure_vcpkg(cmake_args)
        if not using_vcpkg:
            using_pybind11 = _configure_pybind11_from_python(cmake_args)
            if not using_pybind11:
                _raise_missing_pybind11()

        if self.compiler.compiler_type != "msvc":
            # Using Ninja-build since it a) is available as a wheel and b)
            # multithreads automatically. MSVC would require all variables be
            # exported for Ninja to pick it up, which is a little tricky to do.
            # Users can override the generator with CMAKE_GENERATOR in CMake
            # 3.15+.
            if not cmake_generator or cmake_generator == "Ninja":
                try:
                    import ninja  # type: ignore

                    ninja_executable_path = Path(ninja.BIN_DIR) / "ninja"
                    cmake_args += [
                        "-GNinja",
                        f"-DCMAKE_MAKE_PROGRAM:FILEPATH={ninja_executable_path}",
                    ]
                except ImportError:
                    pass

        else:
            # Single config generators are handled "normally"
            single_config = any(x in cmake_generator for x in {"NMake", "Ninja"})

            # CMake allows an arch-in-generator style for backward compatibility
            contains_arch = any(x in cmake_generator for x in {"ARM", "Win64"})

            # Specify the arch if using MSVC generator, but only if it doesn't
            # contain a backward-compatibility arch spec already in the
            # generator name.
            if not single_config and not contains_arch:
                cmake_args += ["-A", PLAT_TO_CMAKE[self.plat_name]]

            # Multi-config generators have a different way to specify configs
            if not single_config:
                cmake_args += [
                    f"-DCMAKE_LIBRARY_OUTPUT_DIRECTORY_{cfg.upper()}={extdir}"
                ]
                build_args += ["--config", cfg]

        if sys.platform.startswith("darwin"):
            # Cross-compile support for macOS - respect ARCHFLAGS if set
            archs = re.findall(r"-arch (\S+)", os.environ.get("ARCHFLAGS", ""))
            if archs:
                cmake_args += ["-DCMAKE_OSX_ARCHITECTURES={}".format(";".join(archs))]

        # Set CMAKE_BUILD_PARALLEL_LEVEL to control the parallel build level
        # across all generators.
        if "CMAKE_BUILD_PARALLEL_LEVEL" not in os.environ:
            # self.parallel is a Python 3 only way to set parallel jobs by hand
            # using -j in the build_ext call, not supported by pip or PyPA-build.
            if hasattr(self, "parallel") and self.parallel:
                # CMake 3.12+ only.
                build_args += [f"-j{self.parallel}"]

        build_temp = Path(self.build_temp) / ext.name
        if not build_temp.exists():
            build_temp.mkdir(parents=True)

        try:
            subprocess.run(
                ["cmake", ext.sourcedir, *cmake_args], cwd=build_temp, check=True
            )
            subprocess.run(
                ["cmake", "--build", ".", *build_args], cwd=build_temp, check=True
            )
        except subprocess.CalledProcessError as e:
            raise RuntimeError("Failed to build extension, ensure cmake is installed") from e


class Install(install):
    def run(self) -> None:
        # setup.py install otherwise delegates to bdist_egg/easy_install, which
        # may try to resolve this package from PyPI after the local build.
        setuptools_install.orig.install.run(self)


setup(
    name="squirrel-lang",
    version=__version__,
    author="shabbywu",
    author_email="shabbywu@qq.com",
    url="https://github.com/shabbywu/squirrel-py",
    description="squirrel-lang - squirrel-lang vm machine, run squirrel-lang with python.",
    long_description="""squirrel-lang - squirrel-lang vm machine, run squirrel-lang with python.
""",
    packages=["squirrel"],
    package_dir={"": "squirrel-lang"},
    extras_require={"test": "pytest"},
    ext_modules=[CMakeExtension("_squirrel")],
    cmdclass={"build_ext": CMakeBuild, "install": Install},
    zip_safe=False,
    python_requires=">=3.7",
    install_requires=[],
)
