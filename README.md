# NemoLib Parallel

## Requirements:

This version is for MacOS/Linux or Windows, it requires either gcc, clang or VC++
to be installed, other compilers may work but have not been tested. Tests
were conducted with gcc 7.3.0 and clang 9.0.0 and Visual Studio 2017.
If you wish to use the threaded version of the library, you need to clone the repository
[Thread_Pool](https://github.com/nickrohde/Thread_Pool/tree/v1.0.0) along with this one to have the thread
pool code for compilation.  

# Compiling:  

## Unix-based OS:

Included is a makefile; to build the library (libnemolib.a), use the **lib** target;
there is also a test source file which can be built using the **test** target.
Use the **help** target or simply **make** for additional information about the compilation.

## Windows:

See the UserGuide folder which includes a detailed [walkthrough](https://github.com/nickrohde/NemoLib/blob/master/UserGuide/VisualStudioSetup.md) for Visual Studio
