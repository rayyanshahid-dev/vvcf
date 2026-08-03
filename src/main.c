/*
 * vvcf - Pharmacogenomics Decision Support Tool
 *
 * Copyright (c) 2026 Rayyan Shahid <rayyanshahid.dev@proton.me>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 *
 * vvcf processes BAM/VCF files to generate pharmacogenomics reports using
 * compile-time LUTs generated from authoritative ClinPGx/PharmVar data.
 *
 * Pipeline:
 *   1. Extract variants from BAM (CIGAR walking, FLAG filtering)
 *   2. Match variants to allele definitions (bitmask comparison)
 *   3. Determine allele function status (Normal, Decreased, No, Increased, Uncertain)
 *   4. Combine alleles into diplotypes (allele_a, allele_b)
 *   5. Map diplotype to phenotype 
 *   6. Generate clinical report (CDS interpretation text + EHR notation)
 *
 * Design principles:
 *   - Data-oriented design: SoA over AoS, .rodata over heap
 *   - Build-time preprocessing: LUTs generated offline, no runtime parsing
 *   - Zero-copy: mmap'd input, trying to avoid unnecessary allocations
 *   - Cache-friendly: aiming to fit LUTs in L1
 *   - Branchless hot path: uses shifts instead of strcmp
 *
 * Dependencies:
 *   - htslib (BAM/SAM/CRAM parsing)
 *   - libc (standard library)
 *
 * References:
 *   - PharmVar: https://www.pharmvar.org/gene/CYP2C19
 *   - ClinPGx: https://www.clinpgx.org
 *   - CPIC: https://cpicpgx.org
 *   - Mike Acton: Data-Oriented Design (CppCon 2014)
 */

#include <htslib/sam.h>
#include <stdio.h>
#include <stdlib.h>

#include "../includes/structs.h"
#include "../includes/rsid.h"
#include "../includes/cyp2c19_allele_lut.h"
#include "../includes/cyp2c19_cds_lut.h"
#include "../includes/dtable_lut.h"


static void usage_info(FILE *fp){
    fprintf(fp, "Usage: vvcf [inputfile] [outputfile]\n");
}

int main(int argc, char *argv[]){
    printf("vvcf started\n");

    if(argc < 2){
        usage_info(stderr);
        return 1;
    }
    // Right now, main should only do four things:
    // 1. extract variants from BAM file and compare against allele definition table
    // 2. look up allele functionality reference to see function status
    // 3. combine two alleles to form diplotype and reference against diplotype phenotype table
    // 4. take clinical data from CDS and assign based on phenotype
    for (int i = 0; i < 1000000; i++){
        for (int j = 0; j < allele_name_count; j++){
        volatile const char *name = cyp2c19_allele_names[j]; 
        }
    }

    printf("CYP2C19 allele position 27 (rs3758581): %u\n", cyp2c19_allele_pos[27]);
    return file_read(argc, argv);

    return 0;
}
