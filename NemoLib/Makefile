# files
NAUTY_DIR=./nauty
THREAD_POOL_DIR=./ThreadPool

GRAPH=Graph
STATS=Stats
SUBGRAPH=Subgraph
GRAPH64=graph64
NAUTY_LINK=NautyLink
PROFILE=SubgraphProfile
COUNT=SubgraphCount
RNG=RandomGraphGenerator
RAND_ESU=RandESU

ifeq ($(_USE_THREAD_POOL), 1)
	ANALYSIS=Parallel_RandGraphAnalysis
	LIB_REQUS=$(PROFILE).o $(STATS).hpp.gch $(ANALYSIS).hpp.gch
	LIB_FILES=$(GRAPH).o $(RNG).o $(NAUTY_LINK).o $(PROFILE).o $(RAND_ESU).o
else
	ANALYSIS=RandomGraphAnalysis
	LIB_REQUS=$(PROFILE).o $(STATS).hpp.gch $(ANALYSIS).o
	LIB_FILES=$(GRAPH).o $(RNG).o $(NAUTY_LINK).o $(PROFILE).o $(RAND_ESU).o $(ANALYSIS).o
endif

# compiler settings
CXX=g++
STD_LINUX=-std=c++17
STD_MAC=-std=c++1z

# check OS as clang++ does not support c++17
ifeq ($(OS), Linux)
	STD := $(STD_LINUX)
else
	STD := $(STD_MAC)
endif

FLAGS += -pipe -Ofast -flto -march=native -pthread -funroll-loops $(STD)

ifeq ($(_USE_ADD_FLAGS), 1)
	FLAGS += $(NEMOLIB_ADDITIONAL_FLAGS)
endif

help:
	@echo "Usage:\n\
	\tBuild the 'lib' target using 'make lib' which will build the library into\n\
	\tthe ./lib/ directory as 'libnemolib.a' such that it can be used by g++/clang++.\n\
	\tYou can use 'make test' for an example program built with NemoLib -- this must\n\
	\tbe run after building the lib target ('make lib').\n\n\
	Note:\n\
	\tFor maximum performance, compile with '-march=native' in 'NEMOLIB_ADDITIONAL_FLAGS',\n\
	\tthis will make NemoLib less portable, but faster on the current processor.\n\
	\tE.g.:\n\
	\t\t'make lib _USE_ADD_FLAGS=1 NEMOLIB_ADDITIONAL_FLAGS=-march=native'\n\n\
	\tIt can also be beneficial to use profile-guided optimization available in g++;\n\
	\tcompile once with -fprofile-generate, then run the program as you would when\n\
	\tusing it, and compile once again with -fprofile-use. Though it is time-consuming,\n\
	\tit can reduce runtime significantly.\n\
	\tE.g.:\n\
	\t\t'make lib _USE_ADD_FLAGS=1 NEMOLIB_ADDITIONAL_FLAGS=-fprofile-generate'\n\
	\t\t'make profile'\n\
	\t\t'./main exampleGraph.txt 16 4 1000'\n\
	\t\t'make clean && rm main && rm lib/libnemolib'\n\
	\t\t'make lib _USE_ADD_FLAGS=1 NEMOLIB_ADDITIONAL_FLAGS=-fprofile-use'\n\
	\t\t'make test_profiled'\n"

lib: $(LIB_REQUS)
	ar rvs lib/libnemolib.a $(LIB_FILES) $(NAUTY_DIR)/*.o

test:
	$(CXX) -c Source.cpp -I./ThreadPool/ -o Source.o $(FLAGS)
	$(CXX) -L./lib Source.o -o main -pthread -lnemolib $(FLAGS)

test_profiled:
	$(CXX) -c Source.cpp -I./ThreadPool/ -o Source.o $(FLAGS) -fprofile-use
	$(CXX) -L./lib Source.o -o main -pthread -lnemolib $(FLAGS) -fprofile-use

profile:
	$(CXX) -c Source.cpp -I./ThreadPool/ -o Source.o $(FLAGS) -fprofile-generate
	$(CXX) -L./lib Source.o -o main -pthread -lnemolib $(FLAGS) -fprofile-generate

nauty_objs:
	@$(MAKE) -C $(NAUTY_DIR) all _USE_ADD_FLAGS=$(_USE_ADD_FLAGS) NEMOLIB_ADDITIONAL_FLAGS=$(NEMOLIB_ADDITIONAL_FLAGS)

$(GRAPH).o: $(GRAPH64).hpp.gch
	$(CXX) -c $(GRAPH).cpp $(FLAGS)

$(RNG).o: $(GRAPH).o
	$(CXX) -c $(RNG).cpp $(FLAGS)

$(STATS).hpp.gch: $(GRAPH64).hpp.gch
	$(CXX) -c $(STATS).hpp $(FLAGS)

$(SUBGRAPH).hpp.gch: $(GRAPH64).hpp.gch
	$(CXX) -c $(SUBGRAPH).hpp $(FLAGS)

$(GRAPH64).hpp.gch:
	$(CXX) -c $(GRAPH64).hpp $(FLAGS)

$(NAUTY_LINK).o: nauty_objs $(GRAPH).o $(SUBGRAPH).hpp.gch
	$(CXX) -c $(NAUTY_LINK).cpp $(FLAGS)

$(PROFILE).o: $(NAUTY_LINK).o
	$(CXX) -c $(PROFILE).cpp $(FLAGS)

$(COUNT).hpp.gch: $(NAUTY_LINK).o
	$(CXX) -c $(COUNT).hpp $(FLAGS)

$(RAND_ESU).o: $(NAUTY_LINK).o
	$(CXX) -c $(RAND_ESU).cpp $(FLAGS)

$(ANALYSIS).o: $(COUNT).hpp.gch $(RNG).o $(RAND_ESU).o
	$(CXX) -c $(ANALYSIS).cpp $(FLAGS)

$(ANALYSIS).hpp.gch: $(COUNT).hpp.gch $(RNG).o $(RAND_ESU).o
	$(CXX) -c $(ANALYSIS).hpp $(FLAGS) -I./ThreadPool


clean_nauty:
	@$(MAKE) -C $(NAUTY_DIR) clean

clean_thread_pool:
	@$(MAKE) -C $(THREAD_POOL_DIR) clean

clean: clean_nauty
	rm -f *.o *.h.gch *.hpp.gch
