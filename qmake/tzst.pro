DESTDIR = ../$$PWD/_bin
QMAKE_PROJECT_DEPTH=0
TARGET = tzst
TEMPLATE = app
CONFIG -= qt
CONFIG -= app_bundle
CONFIG += console c++17 nostrip debug_info

QMAKE_CXXFLAGS += -g

DEFINES += ZSTD_DISABLE_ASM

SOURCES += \
	../base64.cpp \
	../joinpath.cpp \
	../main.cpp \
	../misc.cpp \
	../tar.cpp \
	../tzst.cpp \
	../zs.cpp

HEADERS += \
	../base64.h \
	../joinpath.h \
	../misc.h \
	../tar.h \
	../tzst.h \
	../zs.h

HEADERS += \
	../zstd/lib/zdict.h \
	../zstd/lib/zstd.h \
	../zstd/lib/zstd_errors.h \
	../zstd/lib/common/allocations.h \
	../zstd/lib/common/bits.h \
	../zstd/lib/common/bitstream.h \
	../zstd/lib/common/compiler.h \
	../zstd/lib/common/cpu.h \
	../zstd/lib/common/debug.h \
	../zstd/lib/common/error_private.h \
	../zstd/lib/common/fse.h \
	../zstd/lib/common/huf.h \
	../zstd/lib/common/mem.h \
	../zstd/lib/common/pool.h \
	../zstd/lib/common/portability_macros.h \
	../zstd/lib/common/threading.h \
	../zstd/lib/common/xxhash.h \
	../zstd/lib/common/zstd_deps.h \
	../zstd/lib/common/zstd_internal.h \
	../zstd/lib/common/zstd_trace.h \
	../zstd/lib/compress/clevels.h \
	../zstd/lib/compress/hist.h \
	../zstd/lib/compress/zstd_compress_internal.h \
	../zstd/lib/compress/zstd_compress_literals.h \
	../zstd/lib/compress/zstd_compress_sequences.h \
	../zstd/lib/compress/zstd_compress_superblock.h \
	../zstd/lib/compress/zstd_cwksp.h \
	../zstd/lib/compress/zstd_double_fast.h \
	../zstd/lib/compress/zstd_fast.h \
	../zstd/lib/compress/zstd_lazy.h \
	../zstd/lib/compress/zstd_ldm.h \
	../zstd/lib/compress/zstd_ldm_geartab.h \
	../zstd/lib/compress/zstd_opt.h \
	../zstd/lib/compress/zstd_preSplit.h \
	../zstd/lib/compress/zstdmt_compress.h \
	../zstd/lib/decompress/zstd_ddict.h \
	../zstd/lib/decompress/zstd_decompress_block.h \
	../zstd/lib/decompress/zstd_decompress_internal.h

SOURCES += \
	../zstd/lib/common/debug.c \
	../zstd/lib/common/entropy_common.c \
	../zstd/lib/common/error_private.c \
	../zstd/lib/common/fse_decompress.c \
	../zstd/lib/common/pool.c \
	../zstd/lib/common/threading.c \
	../zstd/lib/common/xxhash.c \
	../zstd/lib/common/zstd_common.c \
	../zstd/lib/compress/fse_compress.c \
	../zstd/lib/compress/hist.c \
	../zstd/lib/compress/huf_compress.c \
	../zstd/lib/compress/zstd_compress.c \
	../zstd/lib/compress/zstd_compress_literals.c \
	../zstd/lib/compress/zstd_compress_sequences.c \
	../zstd/lib/compress/zstd_compress_superblock.c \
	../zstd/lib/compress/zstd_double_fast.c \
	../zstd/lib/compress/zstd_fast.c \
	../zstd/lib/compress/zstd_lazy.c \
	../zstd/lib/compress/zstd_ldm.c \
	../zstd/lib/compress/zstd_opt.c \
	../zstd/lib/compress/zstd_preSplit.c \
	../zstd/lib/compress/zstdmt_compress.c \
	../zstd/lib/decompress/huf_decompress.c \
	../zstd/lib/decompress/zstd_ddict.c \
	../zstd/lib/decompress/zstd_decompress.c \
	../zstd/lib/decompress/zstd_decompress_block.c

