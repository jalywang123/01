GPU=1
CUDNN=1
OPENCV=1
DEBUG=0

# This is what I use, uncomment if you know your arch and want to specify
# ARCH= -gencode arch=compute_52,code=compute_52
ARCH= -gencode arch=compute_30,code=sm_30 \
	  -gencode arch=compute_35,code=sm_35 \
	  -gencode arch=compute_50,code=[sm_50,compute_50] \
	  -gencode arch=compute_52,code=[sm_52,compute_52]

VPATH=./darknet/include:./darknet/src:./detection:./kcf
TARGET=detect
OBJDIR=./obj/
CXXOBJDIR=./objcpp/

CC=gcc
CXX=g++
AR=ar
NVCC=nvcc
OPTS=-Ofast
LDFLAGS= -lm -pthread
COMMON=
CFLAGS=-Wall -Wfatal-errors -Wno-unused-result -fPIC
CXXFLAGS=-pipe -std=c++0x -Wall -W -fPIC
INCPATH=-I. -I./darknet/include -I./darknet/src -I./detection -I./kcf

ifeq ($(DEBUG), 1)
OPTS=-O0 -g
endif

CFLAGS+=$(OPTS)
CXXFLAGS+=$(OPTS)

ifeq ($(OPENCV), 1)
COMMON+= -DOPENCV
CFLAGS+= -DOPENCV
LDFLAGS+= `pkg-config --libs opencv`
COMMON+= `pkg-config --cflags opencv`
endif

ifeq ($(GPU), 1)
COMMON+= -DGPU -I/usr/local/cuda/include/
CFLAGS+= -DGPU
LDFLAGS+= -L/usr/local/cuda/lib64 -lcuda -lcudart -lcublas -lcurand
endif

ifeq ($(CUDNN), 1)
COMMON+= -DCUDNN
CFLAGS+= -DCUDNN
LDFLAGS+= -lcudnn
endif

OBJ=gemm.o utils.o cuda.o deconvolutional_layer.o convolutional_layer.o list.o image.o activations.o im2col.o col2im.o blas.o crop_layer.o dropout_layer.o maxpool_layer.o softmax_layer.o data.o matrix.o network.o connected_layer.o cost_layer.o parser.o option_list.o detection_layer.o route_layer.o box.o normalization_layer.o avgpool_layer.o layer.o compare.o local_layer.o shortcut_layer.o activation_layer.o rnn_layer.o gru_layer.o crnn_layer.o demo.o batchnorm_layer.o region_layer.o reorg_layer.o tree.o lstm_layer.o libyolo.o
CXXOBJ=main.o detection.o fhog.o kcftracker.o tracker.o dataAssociation.o tagEdgeExtraction.o
ifeq ($(GPU), 1)
OBJ+=convolutional_kernels.o deconvolutional_kernels.o activation_kernels.o im2col_kernels.o col2im_kernels.o blas_kernels.o crop_layer_kernels.o dropout_layer_kernels.o maxpool_layer_kernels.o network_kernels.o avgpool_layer_kernels.o
endif

OBJS = $(addprefix $(OBJDIR), $(OBJ))
CXXOBJS = $(addprefix $(CXXOBJDIR), $(CXXOBJ))

all: obj objcpp $(TARGET)

$(TARGET): $(CXXOBJS) $(OBJS)
	$(CXX) $(LIBS) $^ -o $@ $(LDFLAGS)

$(CXXOBJDIR)%.o: %.cpp
	$(CXX) $(CXXFLAGS) $(INCPATH) -c $< -o $@

$(OBJDIR)%.o: %.c
	$(CC) $(COMMON) $(CFLAGS) $(INCPATH) -c $< -o $@

$(OBJDIR)%.o: %.cu
	$(NVCC) $(ARCH) $(COMMON) $(INCPATH) --compiler-options "$(CFLAGS)" -c $< -o $@

obj:
	mkdir -p obj

objcpp:
	mkdir -p objcpp

.PHONY: clean

clean:
	rm -rf $(OBJS) $(CXXOBJS) $(TARGET)

