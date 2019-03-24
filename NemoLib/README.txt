
To compile:
	You must use Visual Studio 2017 on Windows 10 to compile this library, no other build system 
	and OS are currently supported. 
	
	- Open the solution NemoLib.sln in Visual Studio 2017
	- Right click on the NemoLib project in the solution explorer
	- Go to C/C++->All Options
	- Find Additional Include Directories option
	- Add the directories "../CUDA" and "./ThreadPool"
	- Add the include directory of the NVIDIA GPU Computing Tool Kit installed on your machine
		(*) e.g.: C:\Program Files\NVIDIA GPU Computing Tool Kit\v9.0\include
	- Go to Linker->All Options
	- Find Additional Library Directories
	- Add the directory containing "libgpu.lib", this should be "../CUDA"
	- Add the directory containing "cudart.lib"
		- Note, if you compiled "libgpu.lib" yourself, use whatever runtime library you linked to that
		- If you are using the included "libgpu.lib" you must link the NVIDIA GPU Computing Tool Kit v9.0 runtime
          as it is statically bound to the included "libgpu.lib" file		
	- Apply changes and exit configurations.
	- Ensure you are compiling for x64 as CUDA will not link otherwise.
	- It is suggested to use Release build, but not required.
	
	