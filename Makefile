
NAME := tzst
PROJDIR := .

SRCS := \
	base64.cpp \
	joinpath.cpp \
	main.cpp \
	misc.cpp \
	tar.cpp \
	tzst.cpp \
	zs.cpp \
	zstd/lib/common/debug.c \
	zstd/lib/common/entropy_common.c \
	zstd/lib/common/error_private.c \
	zstd/lib/common/fse_decompress.c \
	zstd/lib/common/pool.c \
	zstd/lib/common/threading.c \
	zstd/lib/common/xxhash.c \
	zstd/lib/common/zstd_common.c \
	zstd/lib/compress/fse_compress.c \
	zstd/lib/compress/hist.c \
	zstd/lib/compress/huf_compress.c \
	zstd/lib/compress/zstd_compress.c \
	zstd/lib/compress/zstd_compress_literals.c \
	zstd/lib/compress/zstd_compress_sequences.c \
	zstd/lib/compress/zstd_compress_superblock.c \
	zstd/lib/compress/zstd_double_fast.c \
	zstd/lib/compress/zstd_fast.c \
	zstd/lib/compress/zstd_lazy.c \
	zstd/lib/compress/zstd_ldm.c \
	zstd/lib/compress/zstd_opt.c \
	zstd/lib/compress/zstd_preSplit.c \
	zstd/lib/compress/zstdmt_compress.c \
	zstd/lib/decompress/huf_decompress.c \
	zstd/lib/decompress/zstd_ddict.c \
	zstd/lib/decompress/zstd_decompress.c \
	zstd/lib/decompress/zstd_decompress_block.c

LIBS := 

CC := gcc
CXX := g++
LD := $(CXX)
INCLUDEPATH := -Izstd/lib
DEFINES := -DZSTD_DISABLE_ASM
CFLAGS := -O3 $(INCLUDEPATH) $(DEFINES)
CXXFLAGS := -O3 $(INCLUDEPATH) $(DEFINES)

OBJS := $(SRCS:%.c=%.o)
OBJS := $(OBJS:%.cpp=%.o)
OBJS := $(OBJS:%.cc=%.o)
DEPS := $(SRCS:%.c=%.d)
DEPS := $(DEPS:%.cpp=%.d)
DEPS := $(DEPS:%.cc=%.d)

all: $(NAME)

$(NAME): $(OBJS)
	$(LD) $(OBJS) -o $(NAME) $(LIBS)

.c.o:
	$(CC) $(CFLAGS) -MMD -MP -MF $(<:%.c=%.d) -c $< -o $(<:%.c=%.o)

.cc.o:
	$(CXX) $(CXXFLAGS) -MMD -MP -MF $(<:%.cc=%.d) -c $< -o $(<:%.cc=%.o)

.cpp.o:
	$(CXX) $(CXXFLAGS) -MMD -MP -MF $(<:%.cpp=%.d) -c $< -o $(<:%.cpp=%.o)

.PHONY: clean
clean:
	-rm $(NAME)
	find $(PROJDIR) -name "*.o" -exec rm {} \;
	find $(PROJDIR) -name "*.d" -exec rm {} \;

.PHONY: run
run:
	./$(NAME)

.PHONY: install
install:
	install -m 755 $(NAME) ~/.local/bin/

.PHONY: uninstall
uninstall:
	rm ~/.local/bin/$(NAME)

-include $(DEPS)

