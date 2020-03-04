
help:
	@echo "Use 'help' target to display this help menu\n\
	Use 'debug' target to build a debug version of the code in the ./build directory\n\
	Use 'release' target to build a release version of the code in the ./release directory\n\
	Use 'clean' target to delete the debug and release directories\n"

debug:
	meson build 
	cd build && ninja
	cd build/main && ln -s ../../test

release:
	meson release --buildtype=release --optimization=3
	cd release && ninja
	cd release/main && ln -s ../../test

clean:
	rm -rf build release