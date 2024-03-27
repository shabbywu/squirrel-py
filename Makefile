.DEFAULT_GOAL:=help

##@ Build
.PHONY: build
build: ## Build packege
	python setup.py build


.PHONY: cmake-build
ifeq ($(OS),Windows_NT)
cmake-build:
	mkdir -p build
	cmake --preset=windows-x86
	cmake --build --preset=windows-x86
else
cmake-build:
	mkdir -p build
	cmake --preset=default
	cmake --build --preset=default
endif

##@ Test
.PHONY: test
test: ## Run pytest
	pytest tests


.PHONY: test-debug
test-debug: ## Run pytest with debug params
	CMAKE_ARGS="-DTRACE_CONTAINER_GC=1" pip install ".[test]" -v
	pytest tests

.PHONY: test-305-debug
test-305-debug: ## Run pytest with debug params and sq305
	CMAKE_ARGS="-DTRACE_CONTAINER_GC=1 -DSQUIRREL305=1" pip install ".[test]" -v
	pytest tests

.PHONY: help
help:
	@awk 'BEGIN {FS = ":.*##"; printf "Usage: make \033[36m<target>\033[0m\n"} /^[a-zA-Z_-]+:.*?##/ { printf "  \033[36m%-10s\033[0m %s\n", $$1, $$2 } /^##@/ { printf "\n\033[1m%s\033[0m\n", substr($$0, 5) } ' $(MAKEFILE_LIST)
