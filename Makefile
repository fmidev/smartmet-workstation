# Linux build for SmartMet Workstation
# Requires: cmake, Qt6, boost, trax, giza, imagine2, gis (from ~/hub/)

BUILD_DIR ?= build_linux
CMAKE ?= cmake

.PHONY: all configure build clean

all: build

configure:
	mkdir -p $(BUILD_DIR)
	$(CMAKE) -S . -B $(BUILD_DIR) \
		-DCMAKE_BUILD_TYPE=RelWithDebInfo \
		-DDISABLE_CPPRESTSDK=ON \
		-DDISABLE_EXTREME_TOOLKITPRO=ON \
		-DDISABLE_UNIRAS_TOOLMASTER=ON \
		-DUNIX_BUILD=ON

build: configure
	$(CMAKE) --build $(BUILD_DIR) -j$$(nproc)

clean:
	rm -rf $(BUILD_DIR)
