# tzst - Tar Zstandard Archive Tool

A lightweight command-line utility for creating and extracting tar archives compressed with Zstandard (zstd) compression.

## Overview

tzst is a simple and efficient tool that combines the TAR archive format with Zstandard compression to create `.tar.zst` archives. It provides a straightforward interface for archiving directories and extracting compressed archives with minimal dependencies.

## Features

- **Create tar.zst archives** - Compress directories into tar.zst format
- **Extract tar.zst archives** - Decompress and extract tar.zst files
- **Streaming compression** - Efficient memory usage through streaming operations
- **POSIX tar format** - Compatible with standard tar utilities
- **Cross-platform** - Supports both Windows and Unix-like systems
- **Fast compression** - Leverages Zstandard's high-speed compression algorithm

## Building

### Prerequisites

- C++11 compatible compiler (g++, clang, MSVC)
- Zstandard library (libzstd)
- Make (for Unix-like systems) or Qt Creator (optional)

### Build Instructions

#### Using Make (Linux/macOS)

```bash
make
```

#### Using Qt Creator

Open the Qt project file:
```bash
cd qmake
# Open tzst.pro in Qt Creator
```

### Dependencies

The project requires the Zstandard library. The `zstd/` directory contains the Zstandard source code, which can be built separately if needed.

## Usage

### Command Syntax

```bash
tzst [OPTION] ARCHIVE_FILE SOURCE
```

### Options

- `-c` : Create a new archive
- `-x` : Extract an archive

### Examples

#### Creating an archive

Compress a directory into a tar.zst archive:
```bash
tzst -c output.tar.zst /path/to/directory
```

#### Extracting an archive

Extract a tar.zst archive to the current directory:
```bash
tzst -x archive.tar.zst
```

## Project Structure

```
tzst/
├── main.cpp          # Command-line interface and entry point
├── tzst.cpp/h        # Main tar.zst compression/decompression logic
├── tar.cpp/h         # TAR archive format handling
├── zs.cpp/h          # Zstandard compression wrapper
├── misc.cpp/h        # File system utilities
├── joinpath.cpp/h    # Path manipulation utilities
├── base64.cpp/h      # Base64 encoding/decoding utilities
├── Makefile          # Build configuration
└── zstd/             # Zstandard library source code
```

## Architecture

### Core Components

1. **TarWriter/TarReader** - Handles TAR archive format operations
   - Writes/reads TAR headers (POSIX ustar format)
   - Manages file content with 512-byte block alignment
   - Supports long filenames (GNU tar extension)

2. **ZS (Zstandard Wrapper)** - Provides streaming compression interface
   - Streaming compression and decompression
   - Configurable compression levels
   - Checksum verification for data integrity

3. **tzst Module** - Combines TAR and Zstandard operations
   - Archives directories into tar.zst format
   - Extracts tar.zst archives to disk
   - Memory-efficient streaming implementation

## Technical Details

### Compression Process

1. Scan source directory recursively
2. Create TAR archive in memory buffer
3. Stream TAR data through Zstandard compressor
4. Write compressed data to output file

### Decompression Process

1. Read compressed tar.zst file
2. Decompress using Zstandard streaming API
3. Parse TAR format from decompressed data
4. Extract files to destination directory

### Performance

- Uses streaming I/O to minimize memory footprint
- Multi-threaded file writing during extraction
- Efficient 4KB read buffer for file operations
- Zstandard provides fast compression with good compression ratios

## Compatibility

### TAR Format

- POSIX ustar format
- GNU tar long filename extension (for paths > 100 characters)
- Directory entries with proper permissions
- Regular file type support

### Platforms

- **Linux** - Fully supported
- **macOS** - Fully supported
- **Windows** - Supported with appropriate headers

## See Also

- [Zstandard](https://github.com/facebook/zstd) - Fast compression algorithm
- [TAR format specification](https://www.gnu.org/software/tar/manual/html_node/Standard.html)
