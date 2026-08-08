/* file parsing and report autogeneration code lives here */
/* adapted from Vasudeva Sarma's 'read_bam.c' file from the 
 * htslib documentation. */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <getopt.h>
#include <stdint.h>
#include <immintrin.h>
#include <htslib/sam.h>
#include "../includes/cyp2c19_allele_lut.h"
#include "../includes/cyp2c19_cds_lut.h"
#include "../includes/dtable_lut.h"

#define MEMORY_BUFFER (64 * 1024) // 64kb buffer to avoid the overhead of fprintf syscalls, just testing

#define TARGET_COUNT 35
#define BASE_COUNT   5


    /* these are just rough numbers at present, can decide to change them 
       if they end up replicating the results from the base tally and the 
       sample report used as reference */

#define MIN_DEPTH     10  /* reads to decide whether a variant is called */
#define HET_DEPTH     0.2 /* above this we decide there's a variant present, up to HOM_ALT_DEPTH */
#define HOM_ALT_DEPTH 0.8 /* 80% and above, it's a homozygous call */


    /* TODO: IO based functions here using memory buffer.
       IO is basically the biggest source of overhead in this program, 
       even if the compiler optimizes some stuff out to SIMD
       The naive version has fprintf everywhere which will be removed
    */

static inline int is_target_position(int ref_pos){
     for (int i = 0; i < TARGET_COUNT; i++) {
        if (cyp2c19_allele_pos[i] == ref_pos) {
            return i;
        }
    }
    return -1;
}

static const uint8_t base_to_idx[16] = {
    [0]  = 4,  // 0 = invalid
    [1]  = 0,  // 1 = A
    [2]  = 1,  // 2 = C
    [3]  = 4,  // 3 = invalid 
    [4]  = 2,  // 4 = G
    [8]  = 3,  // 8 = T
    [15] = 4,  // 15 = N
};

static inline int base_to_index(uint8_t base_code) {
    return base_to_idx[base_code & 0xF];  /* mask to 4 bits */
}

    /* this is all general htslib BAM parsing stuff */

void extract_variants_from_bam(bam1_t *bamdata, uint32_t tally[TARGET_COUNT][BASE_COUNT]){
    int flag            = bamdata->core.flag;
    if (flag & 0x4 || flag & 0x100 || flag & 0x800) return; /* filtering unused/unnecessary flags: unmapped, secondary, supplementary  */
    int ref_pos         = bamdata->core.pos + 1;  // 0-based to 1-based to match HGVS nomenclature + the existing array...
    // int l_qseq          = bamdata->core.l_qseq;  
    uint32_t *cigar     = bam_get_cigar(bamdata); 
    uint32_t seq_pos    = 0;
    uint8_t *seq        = bam_get_seq(bamdata);
    

    int n_cigar = bamdata->core.n_cigar;

    for(int i = 0; i < n_cigar; i++){
        uint32_t op = cigar[i];
        int op_type = bam_cigar_op(op);
        int op_len  = bam_cigar_oplen(op);
        if (op_type == BAM_CMATCH || op_type == BAM_CEQUAL || op_type == BAM_CDIFF) {
            for (int k = 0; k < op_len; k++) {
                int variant_id = is_target_position(ref_pos);
                if (variant_id >= 0) {
                    uint8_t base_code = bam_seqi(seq, seq_pos);
                    int base_idx = base_to_index(base_code);
                    tally[variant_id][base_idx]++;
                }
                ref_pos++;
                seq_pos++;
            }
        } else if (op_type == BAM_CINS || op_type == BAM_CSOFT_CLIP) {
            seq_pos += op_len;
        } else if (op_type == BAM_CDEL || op_type == BAM_CREF_SKIP) {
            ref_pos += op_len;
        }
        
    }
}

    /* this function is about setting a threshold for calling variants. 
       we total the bases and compare them against the minimum number of
       reads at every call. If they're below that, we decide they're 
       just noise. After that, we assign a maximum number to a base among
       ACGT (we discard N for now) and compare them, swapping if necessary.
       Then, we cast the highest value to a float and divide it against
       the total calls to be able to measure them against the thresholds
       defined above. 

       The clinical threshold given is roughly:

       * 10 reads or below = discard
       * 20% < x < 80% threshold = heterozygous
       * >80% = homozygous
    */

int call_var_at_pos(uint32_t *tally) {
    int total = tally[0] + tally[1] + tally[2] + tally[3] + tally[4];

    if (total < MIN_DEPTH) {
        return 0;
    }

    /* find the most common base */
    int max1 = 0;
    for (int i = 1; i < 4; i++) {
        if (tally[i] > tally[max1]) {
            max1 = i;
        }
    }

    /* find the second most common base (skip max1) */
    int max2 = -1;
    for (int i = 0; i < 4; i++) {
        if (i == max1) continue;
        if (max2 == -1 || tally[i] > tally[max2]) {
            max2 = i;
        }
    }

    // if there's no other base, it's homozygous reference
    if (max2 == -1) {
        return 1;
    }

    float alt_fraction = (float)tally[max2] / total;

    /* apply thresholds */
    if (alt_fraction < HET_DEPTH) {
        return 1;  /* homozygous reference */
    } else if (alt_fraction > HOM_ALT_DEPTH) {
        return 3;  /* homozygous alternate */
    } else {
        return 2;  /* heterozygous */ 
    }
}

/*  here, we set up the mask used against the bitmasked alleles.
    again, if the result is between a certain threshold determined
    above, the allele is said to be heterozygous. If the reads
    are overwhelmingly towards one base or the other, then the
    allele is said to be homozygous
*/

uint64_t observed_mask(uint32_t tally[TARGET_COUNT][BASE_COUNT]){
    uint64_t mask = 0;
    for(int i = 0; i < TARGET_COUNT; i++){
        int call = call_var_at_pos(tally[i]);
       
/*      call 2 and 3 refer to heterozygous and homozygous alternate reads so 
        they're masked in order to find the alleles and identify variants. 
        1 represents the reference, and masking against the reference would 
        be useless because it would show up against every single one (because 
        of the alleles present in it as the reference). That's why *38 is not 
        counted, similar to the situation with the 0x0 alleles.*/ 

        // TODO: the given BAM file tested had no reads for other positions
        //       that would have also presented for *2, so more testing is needed


        if(call == 2 || call == 3){
            mask |= (1ULL << i);           
        }    
    }
    return mask;
}

static const int cyp2c19_phenotype_from_functions(cyp2c19_allele_function func_a, cyp2c19_allele_function func_b) {
    if (func_a > func_b) {
        cyp2c19_allele_function tmp = func_a;
        func_a = func_b;
        func_b = tmp;
    }

        /* originally this was going to be done in Julia, but
           I thought it would be easier to have this lookup
           done in runtime, as opposed to generating a massive
           LUT of possible variants and their associated allele
           functionality */
    
    // 0=Normal, 1=Decreased, 2=No Function, 3=Increased, 4=Uncertain
    static const int phenotype_matrix[5][5] = {
     /*   Normal   Decreased  NoFunc     Increased  Uncertain */
        {4,        2,         1,         6,         0}, // Normal
        {2,        2,         3,         2,         0}, // Decreased
        {1,        3,         5,         1,         0}, // No Function
        {6,        2,         1,         7,         0}, // Increased
        {0,        0,         0,         0,         0}, // Uncertain
    };
    
   /* Phenotype IDs: 0=Indeterminate, 1=Intermediate, 2=LikelyIntermediate,
                   3=LikelyPoor, 4=Normal, 5=Poor, 6=Rapid, 7=Ultrarapid */
    return phenotype_matrix[func_a][func_b];
}

// TODO: add in a flag that calls for 'further eval' or something 
//       of the sort, just as a failsafe to alert a clinician/researcher
//       in case of rare variants

    /* this is the general report rendering function; it involves 
    taking in the information from the LUT headers + bitmasks
    and presenting the logic calculated by the other lookup/masking
    functions. */

void print_report(FILE *fp, uint32_t tally[TARGET_COUNT][BASE_COUNT]) {
    /* print the tally table*/
    fprintf(fp, "\n## Base Tally by Target Position\n\n");
    fprintf(fp, "| Variant ID | Position | A | C | G | T | N | Total |\n");
    fprintf(fp, "| :--- | :--- | ---: | ---: | ---: | ---: | ---: | ---: |\n");
    
    /* assign totals per row*/

    for (int i = 0; i < TARGET_COUNT; i++) {
        int total = tally[i][0] + tally[i][1] + tally[i][2] + tally[i][3] + tally[i][4];
        fprintf(fp, "| %d | %u | %u | %u | %u | %u | %u | %d |\n",
                i, cyp2c19_allele_pos[i],
                tally[i][0], tally[i][1], tally[i][2], tally[i][3], tally[i][4], total);
    }

/*    we assign uint64_t mask the value of the observed mask, 
    and we bitwise OR the two alleles [i] and [j] here as we're 
    making a diploid to combine them into one 64-bit uint. then we do mask 
    (which is observed mask taking in tally which is half its size because 
    tally was defined in the function params as a uint32_t. when we perform 
    the bitwise AND and NOT against both [i] and [j], if they equal 0 then 
    they match the bitmasked allele found in the rolling LUT scan. 
    after we find them we break. */

    uint64_t mask = observed_mask(tally);
    fprintf(fp, "\n**Observed Mask:** 0x%016lX\n\n", mask);

    int allele_a = -1, allele_b = -1;
    for (int i = 0; i < CYP2C19_ALLELE_COUNT; i++) {
        for (int j = i; j < CYP2C19_ALLELE_COUNT; j++) {
            uint64_t combined = cyp2c19_allele_masks[i] | cyp2c19_allele_masks[j];
            if ((mask & ~combined) == 0) {
                allele_a = i;
                allele_b = j;
                break;
            }
        }
        if (allele_a != -1) break;
    }

    if (allele_a == -1) {
        fprintf(fp, "**No matching allele combination found.**\n");
        return;
    }

    /* Look up phenotype based on the alleles we've 
       found using the lookup function */
    
    const char *name_a = cyp2c19_allele_names[allele_a];
    const char *name_b = cyp2c19_allele_names[allele_b];

    cyp2c19_allele_function func_a = cyp2c19_get_function(name_a);
    cyp2c19_allele_function func_b = cyp2c19_get_function(name_b);
    
    int phenotype_id = cyp2c19_phenotype_from_functions(func_a, func_b);
    
    const char *phenotype_name = cyp2c19_phenotype_names[phenotype_id];
    const char *interpretation = cyp2c19_consultation_text[phenotype_id];

    fprintf(fp, "\n## Clinical Interpretation\n\n");
    fprintf(fp, "**Alleles matched:** %s / %s\n", name_a, name_b);
    
   /* fprintf(fp, "**Functions:** %d / %d\n", func_a, func_b);*/
    fprintf(fp, "**Allele Function A:** %s\n", cyp2c19_function_names[func_a]);
    fprintf(fp, "**Allele Function B:** %s\n", cyp2c19_function_names[func_b]);
    fprintf(fp, "**Phenotype:** %s\n", phenotype_name);
    
    fprintf(fp, "\n**Interpretation:**\n%s\n", interpretation);

    /* print the drug list */
    // TODO: add helper function to specify + format nicely
    fprintf(fp, "\n## Relevant Drugs for CYP2C19\n\n");
    int substrate_count = sizeof(cyp2c19_substrates) / sizeof(cyp2c19_substrates[0]);
    for (int j = 0; j < substrate_count; j++) {
        fprintf(fp, "%s,\n", cyp2c19_substrates[j]);
        
    }
    fprintf(fp, "\n");
}

    /* this is the file's 'main' function. originally
       it was going to be folded into main, but
       having it here like this is neater and more
       modular. */ 

int file_read(int argc, char *argv[]) {

    uint32_t tally[TARGET_COUNT][BASE_COUNT] = {0};
    const char *inname = NULL;
    samFile *inputfile = NULL;
    FILE *fp           = fopen(argv[2], "w");
    if(!fp){
        fprintf(stderr, "Error: could not open writeable file\n");
        goto cleanup;
    }
    sam_hdr_t *in_sam_header = NULL;
    bam1_t *bamdata = NULL;
    int ret_r = 0;
    int ret = EXIT_FAILURE;

    if (argc != 3) {

        return -1;
    }

    inname = argv[1];

    /* 1. open the BAM file */
    inputfile = sam_open(inname, "r");
    if (!inputfile) {
        fprintf(stderr, "Error: Could not open BAM file '%s'\n", inname);
        goto cleanup;
    }

    /*2. read the header*/
    in_sam_header = sam_hdr_read(inputfile);
    if (!in_sam_header) {
        fprintf(stderr, "Error: Could not read BAM header from '%s'\n", inname);
        goto cleanup;
    }

    /*3. allocate a BAM record*/
    bamdata = bam_init1();
    if (!bamdata) {
        fprintf(stderr, "Error: Failed to allocate BAM record\n");
        goto cleanup;
    }

    /*4. loop thru records*/
    int record_count = 0;
    fprintf(fp, "This file was autogenerated by vvcf for Cytogenomix. Do not modify.\n");
        fprintf(fp, "\n");
    
    /* main file read loop */
    while ((ret_r = sam_read1(inputfile, in_sam_header, bamdata)) >= 0) {
        extract_variants_from_bam(bamdata, tally);
        record_count++;
    }

    if (ret_r < -1) {
        fprintf(stderr, "Error reading BAM file (code: %d)\n", ret_r);
        goto cleanup;
    }

    print_report(fp, tally);
    printf("Processed %d records.\n", record_count);
    ret = EXIT_SUCCESS;


    // TODO: extend this out into its own function at some point 
cleanup:
    if (bamdata) bam_destroy1(bamdata);
    if (in_sam_header) sam_hdr_destroy(in_sam_header);
    if (inputfile) sam_close(inputfile);
    return ret;
}
