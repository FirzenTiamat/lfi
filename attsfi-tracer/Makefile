CC = aarch64-lfi-linux-musl-clang 
#CC = clang 
#CC = gcc

Name = cflowtracer

Target = $(Name).o
Srouce = $(Name).c
IRTarget = $(Name).ll

AR = llvm-ar
LD = ld.lld
RM = rm -f
LINK = llvm-link
LLC = llc

#INC := -I.

OPT_LEVEL = -O3

CFLAGS += $(INC) $(OPT_LEVEL) -flto -Wno-void-pointer-to-int-cast
CLANG_VI_FLAGS = -emit-llvm -S -fno-discard-value-names 
CLANG_FLAGS =  $(CLANG_VI_FLAGS) -mllvm --inline-threshold=10000  #-disable-O0-optnone
LINKFLAGS += -fuse-ld=lld -flto 

all: $(Target) slib

$(Target): $(Srouce)
	$(CC) $(CFLAGS) -c $< -o $@

slib: $(Target)
	$(AR) -rc lib$(Name).a $(Target)

.PHONY: ir
ir: $(IRTarget)
$(IRTarget): $(Srouce)
	$(CC) $(CFLAGS) $(CLANG_FLAGS) -c $< -o $@ 

.PHONY: clean irclean
clean:
	$(RM) $(Target) 

irclean:
	$(RM) $(IRTarget)