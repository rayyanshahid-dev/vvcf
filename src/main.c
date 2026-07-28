#include <htslib/sam.h>
#include <stdio.h>
#include <stdlib.h>

// #include "../includes/dypd_diplopheno.h"
// #include "../includes/interpretation.h"
// #include "../includes/structs.h"
#include "../includes/rsid.h"

static void usage_info(FILE *fp){
    fprintf(fp, "Usage: vvcf inputfile\n\
To use vvcf, run the program and include an input BAM file.\n\
This will then autogenerate a pharmacogenomics report based \n\
on the data available in the BAM file.\n");
}

int main(int argc, char** argv[]){

    if(argc < 2){
        usage_info(stderr);
        return 1;
    }
    // Right now, main should only do four things:
    // 1. extract variants from BAM file and compare against allele definition table
    // 2. look up allele functionality reference to see function status
    // 3. combine two alleles to form diplotype and reference against diplotype phenotype table
    // 4. take clinical data from CDS and assign based on phenotype

    return 0;
}
