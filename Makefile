BUILD_DIR=build
CROSS_BUILD_DIR=$(BUILD_DIR)/dockcross
CROSS_SCRIPTS_DIR=dockcross

all: cmake-debug make install

release: cmake-release make install

cmake-debug: CMakeLists.txt
	mkdir -p build
	cmake -DCMAKE_BUILD_TYPE=Debug -B$(BUILD_DIR) -H.

cmake-release: CMakeLists.txt
	mkdir -p build
	cmake -DCMAKE_BUILD_TYPE=Release -B$(BUILD_DIR) -H.

make: $(BUILD_DIR)/Makefile
	make -C$(BUILD_DIR)

install:
	make -C$(BUILD_DIR) install


all-cross: check-docker $(CROSS_BUILD_DIR)/linux-x64 $(CROSS_BUILD_DIR)/linux-armv6


DOCKER_OK := $(shell docker version 2> /dev/null)

check-docker:
ifndef DOCKER_OK
	$(error "Docker is not available please install Docker")
endif


$(CROSS_BUILD_DIR)/%: CMakeLists.txt
	$(CROSS_SCRIPTS_DIR)/dockcross-$* cmake -DCMAKE_BUILD_TYPE=Release -B$@ -H.
	$(CROSS_SCRIPTS_DIR)/dockcross-$* make -C$@


clean:
	rm -fr $(BUILD_DIR)
