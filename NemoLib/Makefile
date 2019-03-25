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
	ANALYSIS=Parallel_RandomGraphAnalysis
	LIB_REQUS=$(PROFILE).o $(STATS).hpp.gch $(ANALYSIS).hpp.gch $(PARALLEL_ESU).hpp.gch
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

# delete will generate 3 warnings that can be ignored, thus -w
FLAGS += -pipe -Ofast -march=native -pthread -funroll-loops $(STD)

ifeq ($(_USE_ADD_FLAGS), 1)
	FLAGS += $(NEMOLIB_ADDITIONAL_FLAGS)
endif


lib: $(LIB_REQUS)
	ar rvs lib/libnemolib.a $(LIB_FILES) $(NAUTY_DIR)/*.o

test:
	$(CXX) -c Source.cpp -o Source.o $(FLAGS)
	$(CXX) -L./lib -lnemolib Source.o -o main

nauty_objs:
	$(MAKE) -C $(NAUTY_DIR) all

thread_pool:
	$(MAKE) -C $(THREAD_POOL_DIR) all

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

$(ANALYSIS).hpp.gch: $(COUNT).hpp.gch $(RNG).o $(RAND_ESU).o thread_pool
	$(CXX) -c $(ANALYSIS).hpp $(FLAGS)

$(PARALLEL_ESU).hpp.gch: $(RAND_ESU).o thread_pool
	$(CXX) -c $(PARALLEL_ESU).hpp $(FLAGS)

clean_nauty:
	$(MAKE) -C $(NAUTY_DIR) clean

clean_thread_pool:
	$(MAKE) -C $(THREAD_POOL_DIR) clean

clean: clean_nauty
	rm -f *.o *.h.gch *.hpp.gch
