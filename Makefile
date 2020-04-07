.PHONY: test clean

BUILD_DIR=build
CROSS_BUILD_DIR=$(BUILD_DIR)/crossbuilds
CROSS_SCRIPTS_DIR=crossbuilds

# CROSS_BUILD_TARGETS=linux-x64 linux-armv5 linux-armv6 linux-arm64 osx-64 manylinux2010-x64 #manylinux2010-x86
CROSS_BUILD_TARGETS=manylinux2010-x64

# NOTES:
# - MacOS: can't use multiarch/crossbuild since it uses Clang 3.5.0 which lacks <stdatomic.h> (even using -std=gnu11)
DOCKER_CROSSBUILD_IMAGE=multiarch/crossbuild
DOCKER_OSXCROSS_IMAGE=liushuyu/osxcross

ifneq ($(ZENOH_DEBUG),)
	ZENOH_DEBUG_OPT := -DZENOH_DEBUG=$(ZENOH_DEBUG)
endif

all: cmake-debug make

gcov: cmake-gcov make

release: cmake-release make

cmake-debug: CMakeLists.txt
	mkdir -p $(BUILD_DIR)
	cmake $(ZENOH_DEBUG_OPT) -DCMAKE_BUILD_TYPE=Debug -B$(BUILD_DIR) -H.

cmake-gcov: CMakeLists.txt
	mkdir -p $(BUILD_DIR)
	cmake $(ZENOH_DEBUG_OPT) -DCMAKE_BUILD_TYPE=GCov -B$(BUILD_DIR) -H.

cmake-release: CMakeLists.txt
	mkdir -p $(BUILD_DIR)
	cmake $(ZENOH_DEBUG_OPT) -DCMAKE_BUILD_TYPE=Release -B$(BUILD_DIR) -H.

make: $(BUILD_DIR)/Makefile
	make -C$(BUILD_DIR)

install:
	make -C$(BUILD_DIR) install

test:
	make -C$(BUILD_DIR) test

all-cross: check-docker $(foreach target,$(CROSS_BUILD_TARGETS),$(CROSS_BUILD_DIR)/$(target))

DOCKER_OK := $(shell docker version 2> /dev/null)
DOCKER_CROSSBUILD_INFO := $(shell docker image inspect $(DOCKER_CROSSBUILD_IMAGE) 2> /dev/null)
DOCKER_OSXCROSS_INFO := $(shell docker image inspect $(DOCKER_OSXCROSS_IMAGE) 2> /dev/null)

check-docker:
ifndef DOCKER_OK
	$(error "Docker is not available. Please install Docker")
endif
ifeq ($(DOCKER_CROSSBUILD_INFO),[])
	echo docker pull $(DOCKER_CROSSBUILD_IMAGE)
endif
ifeq ($(DOCKER_OSXCROSS_INFO),[])
	echo docker pull $(DOCKER_OSXCROSS_IMAGE)
endif


$(CROSS_BUILD_DIR)/linux-armv5: CMakeLists.txt
	docker run --rm -v $(PWD):/workdir -e CROSS_TRIPLE=arm-linux-gnueabi $(DOCKER_CROSSBUILD_IMAGE) \
		cmake -DJAVA_HOME=${JAVA_HOME} -DCMAKE_BUILD_TYPE=Release -B$@ -H.
	docker run --rm -v $(PWD):/workdir -e CROSS_TRIPLE=arm-linux-gnueabi $(DOCKER_CROSSBUILD_IMAGE) \
		make VERBOSE=1 -C$@

$(CROSS_BUILD_DIR)/linux-armv6: CMakeLists.txt
	docker run --rm -v $(PWD):/workdir -e CROSS_TRIPLE=arm-linux-gnueabihf $(DOCKER_CROSSBUILD_IMAGE) \
		cmake -DJAVA_HOME=${JAVA_HOME} -DCMAKE_BUILD_TYPE=Release -B$@ -H.
	docker run --rm -v $(PWD):/workdir -e CROSS_TRIPLE=arm-linux-gnueabihf $(DOCKER_CROSSBUILD_IMAGE) \
		make VERBOSE=1 -C$@

$(CROSS_BUILD_DIR)/linux-arm64: CMakeLists.txt
	docker run --rm -v $(PWD):/workdir -e CROSS_TRIPLE=aarch64-linux-gnu $(DOCKER_CROSSBUILD_IMAGE) \
		cmake -DJAVA_HOME=${JAVA_HOME} -DCMAKE_BUILD_TYPE=Release -B$@ -H.
	docker run --rm -v $(PWD):/workdir -e CROSS_TRIPLE=aarch64-linux-gnu $(DOCKER_CROSSBUILD_IMAGE) \
		make VERBOSE=1 -C$@

$(CROSS_BUILD_DIR)/linux-x64: CMakeLists.txt
	docker run --rm -v $(PWD):/workdir $(DOCKER_CROSSBUILD_IMAGE) \
		cmake -DJAVA_HOME=${JAVA_HOME} -DCMAKE_BUILD_TYPE=Release -B$@ -H.
	docker run --rm -v $(PWD):/workdir $(DOCKER_CROSSBUILD_IMAGE) \
		make VERBOSE=1 -C$@

$(CROSS_BUILD_DIR)/osx-64: CMakeLists.txt
	docker run --rm -v $(PWD):/workdir -w /workdir -e CC=x86_64-apple-darwin18-clang -e CXX=x86_64-apple-darwin18-clang $(DOCKER_OSXCROSS_IMAGE) \
		cmake -DCMAKE_SYSTEM_NAME=Darwin -DJAVA_HOME=${JAVA_HOME} -DCMAKE_BUILD_TYPE=Release -B$@ -H.
	docker run --rm -v $(PWD):/workdir -w /workdir -e CC=x86_64-apple-darwin18-clang -e CXX=x86_64-apple-darwin18-clang $(DOCKER_OSXCROSS_IMAGE) \
		make VERBOSE=1 -C$@

$(CROSS_BUILD_DIR)/manylinux2010-x86: CMakeLists.txt
	dockcross/dockcross-manylinux2010-x86 cmake -DJAVA_HOME=${JAVA_HOME} -DCMAKE_BUILD_TYPE=Release -B$@ -H.
	dockcross/dockcross-manylinux2010-x86 make -C$@

$(CROSS_BUILD_DIR)/manylinux2010-x64: CMakeLists.txt
	dockcross/dockcross-manylinux2010-x64 cmake -DJAVA_HOME=${JAVA_HOME} -DCMAKE_BUILD_TYPE=Release -B$@ -H.
	dockcross/dockcross-manylinux2010-x64 make -C$@



clean:
	rm -fr $(BUILD_DIR)
