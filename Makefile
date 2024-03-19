.DEFAULT_GOAL:=help

##@ Build
.PHONY: build
build: ## Build packege
	python setup.py build


.PHONY: cmake-build
ifeq ($(OS),Windows_NT)
cmake-build:
	mkdir -p build
	cmake -DCMAKE_BUILD_TYPE=Release -DCMAKE_CONFIGURATION_TYPES="Release" -DPHYSFS_BUILD_SHARED=False -DPHYSFS_BUILD_STATIC=True -DDISABLE_DYNAMIC=True -B build -A Win32 .
	cmake --build build --config Release -v
else
cmake-build:
	mkdir -p build
	cmake -DCMAKE_BUILD_TYPE=Release -DCMAKE_CONFIGURATION_TYPES="Release" -DPHYSFS_BUILD_SHARED=False -DPHYSFS_BUILD_STATIC=True -DDISABLE_DYNAMIC=True -DPYTHON_EXECUTABLE=`which python` -B build .
	cmake --build build --config Release -v
endif


##@ Test
.PHONY: test
test: ## Run pytest
	pytest tests


.PHONY: test-debug
test-debug: ## Run pytest with debug params
	CMAKE_ARGS="-DTRACE_CONTAINER_GC=1" pip install .
	pytest tests


.PHONY: help
help:
	@awk 'BEGIN {FS = ":.*##"; printf "Usage: make \033[36m<target>\033[0m\n"} /^[a-zA-Z_-]+:.*?##/ { printf "  \033[36m%-10s\033[0m %s\n", $$1, $$2 } /^##@/ { printf "\n\033[1m%s\033[0m\n", substr($$0, 5) } ' $(MAKEFILE_LIST)
