CC_NAIVE=	g++
OPT_NAIVE=	
LIBS_NAIVE=	-lm -lpthread
CC_VERIFY=	g++
OPT_VERIFY=	-std=c++11
LIBS_VERIFY=	-lm -lpthread
SRCS_NAIVE=	Naive_main.cpp Naive_chunk.cpp Naive_parser.cpp List.cpp Naive_basic.cpp Naive_stdin.cpp
OBJS_NAIVE=	Naive_main.o Naive_chunk.o Naive_parser.o List.o Naive_basic.o Naive_stdin.o
SRCS_VERIFY=	Verify_main.cpp Verify_chunk.cpp Verify_block.cpp Verify_lca.cpp Parser.cpp List.cpp MRCA_hash.cpp Prepare.cpp RMQmode.cpp Basic.cpp
OBJS_VERIFY=	Verify_main.o Verify_chunk.o Verify_block.o Verify_lca.o Parser.o List.o MRCA_hash.o Prepare.o RMQmode.o Basic.o

EXECUTABLE_NAIVE=	IBDdetection_naive
EXECUTABLE_VERIFY=	IBDdetection_verify

# this is only a reminder if the user want to "make all"
all:
	@echo Please choose from \"make naive\", \"make binary\" and \"make verify\" to compile and link...


# this is the n^2 naive method (changed-pair detection) for IBD extraction
naive: main_naive clean mv_naive

$(OBJS_NAIVE): $(SRCS_NAIVE)
	$(CC_NAIVE) -c $*.cpp $(OPT_NAIVE)

main_naive: $(OBJS_NAIVE)
	$(CC_NAIVE) -o $(EXECUTABLE_NAIVE) $(OBJS_NAIVE) $(OPT_NAIVE) $(LIBS_NAIVE)


# this is the verify all trees verification algorithm
verify: main_verify clean mv_verify

$(OBJS_VERIFY): $(SRCS_VERIFY)
	$(CC_VERIFY) -c $*.cpp $(OPT_VERIFY)

main_verify: $(OBJS_VERIFY)
	$(CC_VERIFY) -o $(EXECUTABLE_VERIFY) $(OBJS_VERIFY) $(OPT_VERIFY) $(LIBS_VERIFY)


clean:
	-rm -f *.o

mv_naive:
	@chmod 755 $(EXECUTABLE_NAIVE)
#	@mv $(EXECUTABLE_NAIVE) ../IBD_C_upgrade_test/
#	-@../IBD_C_upgrade_test/$(EXECUTABLE_NAIVE)

mv_verify:
	@chmod 755 $(EXECUTABLE_VERIFY)
#	@mv $(EXECUTABLE_VERIFY) ../IBD_C_upgrade_test/
#	-@../IBD_C_upgrade_test/$(EXECUTABLE_VERIFY)
