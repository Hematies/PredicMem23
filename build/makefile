EXCL_FILE += ./tinyxml/xmltest.cpp
INC=-I../PredicMem23 -I./tinyxml
OBJS_SRC+=$(filter-out $(EXCL_FILE), $(wildcard ../PredicMem23/*.cpp ./tinyxml/*.cpp))
OBJ_LINK+=
#LIBS_PATH+= -L/home/arios/Projects/RobotAnno/cJSON-1.7.15/build/install/lib -L/home/arios/Projects/RobotAnno/mosquitto-2.0.15/build/install/lib
#OPT += -shared
#OPT+= -lcjson -lmosquitto -lmosquittopp
#OPT+= -l/home/arios/Projects/RobotAnno/mosquitto-2.0.15/build/install/lib/libmosquitto_static.a -l/home/arios/Projects/RobotAnno/mosquitto-2.0.15/build/install/lib/libmosquittopp_static.a
#LIBS += -lmosquitto_static -lmosquittopp_static -lpthread
OUT_OBJ = predic_mem
COMP = g++
OPT+=-std=c++17 -fmax-errors=5
#COMP = /opt/Xilinx/Vitis/2022.1/gnu/aarch32/lin/gcc-arm-linux-gnueabi/bin/arm-linux-gnueabihf-g++
#OPT+=-Ofast -std=c++11 -lrt -static -funroll-all-loops -pipe -fopenmp #if -flto option is enable throw function doesn't work (segmentatio fault is produced)

OBJ_CLEAN = *.o
OBJ_LINK+= *.o

all: OBJ

OBJ: $(OBJS_SRC)
	clear
	$(COMP) $(OPT) $(LIBS_PATH) -c $(OBJS_SRC) $(INC) $(DEFS) $(LIBS)
	$(COMP) $(OPT) $(LIBS_PATH) -o $(OUT_OBJ) $(OBJ_LINK) $(INC) $(DEFS) $(LIBS)
	rm $(OBJ_CLEAN)

clean:
	rm *.o *.so
	rm build/*.o build/*.so