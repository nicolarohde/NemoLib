Requirements:
	This version is for MacOS/Linux, it requires either gcc or clang to be installed, 
	other compilers may work but have not been tested. Tests were conducted with 
	gcc 7.3.0 and clang 9.0.0. 
	If you wish to use the threaded version of the library, you need to clone the repository
	https://github.com/nickrohde/Thread_Pool along with this one to have the thread
	pool code for compilation.

Compiling:
	Included is a makefile; to build the library (libnemolib.a), use the 'lib' target;
	there is also a test source file which can be built using the 'test' target. 
	Use the 'help' target or simply 'make' for additional information about the compilation.

	
