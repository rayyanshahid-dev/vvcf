# vvcf

## How vvcf Works

vvcf bakes CPIC/ClinPGx pharmacogenomics data directly into the binary. A Julia build-time pipeline converts Excel reference tables into C headers, which are compiled into dense `.rodata` arrays. This keeps the lookup tables in L1/L2 cache, avoiding main RAM hits during variant processing.

**The pipeline:**

1. **BAM parsing**: htslib streams the input file (statically linked. no libcurl, no runtime internet access required).
2. **Variant extraction**: CIGAR walking + FLAG filtering isolates reads at target positions.
3. **Allele matching**: Each variant is a bitfield. vvcf uses bit shifts to match against precomputed allele masks, with dual-point verification via chromosome position and rsID.
4. **Phenotype lookup**: The matched allele pair indexes into a 38×38 phenotype matrix (1444 bytes to fit in L1).
5. **Report generation**: Results are written to a preformatted Markdown template (also generated programmatically). 

The entire binary is statically linked. Everything it needs is baked in at compile time. No runtime internet access required. No external reads save for the input BAM file. That goes for Windows as well. It's compiled using mingw under MSYS2, and binaries will be provided to users on Windows machines. No installation required.

# usage

The tool is CLI for now. Open up a terminal and navigate to the 'vvcf' directory. 

## Linux

- installation:

```
make all
```

### running the program:

```
./vvcf 'input_file.bam' 'output_file'
```

## Windows

Executable binaries will be provided - on Windows, there is no need to build the program yourself as the executable is compiled with the Windows system libraries. However, if you wish, the Makefile provides a way for you to build it from source provided you have the following dependencies:

* -lbz2 (bzip2)
* -llzma (liblzma)
* -lws2_32 (Winsock2)
* -lsystre (to fix regcomp issues)
* -ltre (see above)
* -lregex (not a native header on Windows)
* -lintl (GNU gettext when linking POSIX regex)
* -liconv (library for character set conversion)

### running the program:

```
./vvcf 'input_file.bam' 'output_file'
```

## Notes

For now, the tool is limited to one gene (CYP2C19) and only parses BAM files. Additionally, it is designed to format output documents into Markdown, so please ensure to add the '.md' file extension to your output filename when calling the program.

## status
in active development
