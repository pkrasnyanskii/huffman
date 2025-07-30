# Huffman Compression

File compressor and decompressor written in **C** using the Huffman coding algorithm.

## How it works

1. **Frequency analysis** — counts how often each byte appears in the input file
2. **Tree building** — constructs a binary tree using a min-heap, where rare symbols get longer codes and common ones get shorter codes
3. **Encoding** — replaces each byte with its variable-length bit code and writes the result to a `.huf` file
4. **Decoding** — reads the saved tree from the archive and reconstructs the original file bit by bit

The compressed file stores the Huffman tree followed by the original file size, then the encoded data.

## Project structure

```
huffman/
├── src/
│   ├── main.c          # CLI entry point (argument parsing)
│   ├── huffman.c       # Tree building, code generation, tree serialization
│   ├── compress.c      # Compression logic
│   ├── decompress.c    # Decompression logic
│   └── utils.c         # Progress bar, file size helper
├── include/
│   ├── huffman.h
│   ├── compress.h
│   ├── decompress.h
│   └── utils.h
├── examples/
│   ├── example1.txt    # Sample text file
│   └── example2.bmp    # Sample image file
└── Makefile
```

## Build

Requires `gcc` and `make`.

```bash
make
```

Binary will appear at `./huffman`. To clean build artifacts:

```bash
make clean
```

## Usage

```
huffman -c|-d -i <input_file> -o <output_file>

Options:
  -c           Compress input file
  -d           Decompress input file
  -i <file>    Input file path
  -o <file>    Output file path
  --help       Show this message
```

## Examples

**Compress a text file:**
```bash
./huffman -c -i examples/example1.txt -o output.huf
```

**Decompress it back:**
```bash
./huffman -d -i output.huf -o recovered.txt
```

**Compress an image:**
```bash
./huffman -c -i examples/example2.bmp -o output.huf
```

The tool prints compression ratio on finish:
```
Compression result:
Input size:        14028 bytes
Output size:       7965 bytes
Compression ratio: 1.76
```
