.PHONY: test clean

BUILD_DIR=build
CROSS_BUILD_DIR=$(BUILD_DIR)/dockcross
CROSS_SCRIPTS_DIR=dockcross

ifneq ($(ZENOH_DEBUG),)
	ZENOH_DEBUG_OPT := -DZENOH_DEBUG=$(ZENOH_DEBUG)
endif

all: cmake-debug make

release: cmake-release make

cmake-debug: CMakeLists.txt
	mkdir -p $(BUILD_DIR)
	cmake $(ZENOH_DEBUG_OPT) -DCMAKE_BUILD_TYPE=Debug -B$(BUILD_DIR) -H.

cmake-release: CMakeLists.txt
	mkdir -p $(BUILD_DIR)
	cmake $(ZENOH_DEBUG_OPT) -DCMAKE_BUILD_TYPE=Release -B$(BUILD_DIR) -H.

make: $(BUILD_DIR)/Makefile
	make -C$(BUILD_DIR)

install:
	make -C$(BUILD_DIR) install

test:
	make -C$(BUILD_DIR) test

all-cross: check-docker $(CROSS_BUILD_DIR)/linux-x64 $(CROSS_BUILD_DIR)/linux-armv6


DOCKER_OK := $(shell docker version 2> /dev/null)

check-docker:
ifndef DOCKER_OK
	$(error "Docker is not available please install Docker")
endif


$(CROSS_BUILD_DIR)/%: CMakeLists.txt
	$(CROSS_SCRIPTS_DIR)/dockcross-$* cmake -DJAVA_HOME=${JAVA_HOME} -DCMAKE_BUILD_TYPE=Release -B$@ -H.
	$(CROSS_SCRIPTS_DIR)/dockcross-$* make -C$@


clean:
	rm -fr $(BUILD_DIR)
