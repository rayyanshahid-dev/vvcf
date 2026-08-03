/* testing out htslib apis for bam reading 
   this will eventually be folded into main.c
   as part of the general API using htslib,
   and will be tied to the lookup logic code
   that references the existing allele tables 
   and bitmask

   much of my code references Vasudeva Sarma's
   'read_bam.c' from the htslib documentation

   I think it's a decent start - just need to tidy
   up the cleanup function and arrange it properly
   the fact it compiles is good enough. 

   not sure if I'm happy with the cleanup the way it 
   is at the moment, but i'll get around to changing 
   it later. 
*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <getopt.h>
#include <stdint.h>
#include <htslib/sam.h>

int file_read(int argc, char *argv[]) {

/* 
    Presumably, this sets up all the header tags
    and initializes their values to zero

    tidname     = target/reference ID
    flags       = BAM/SAM format flags for filtering
    sam_hdr_t   = SAM header structure
    bam1_t      = contains BAM core data structures (refer to htslib documentation for more info)
*/

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
    while ((ret_r = sam_read1(inputfile, in_sam_header, bamdata)) >= 0) {
        const char *qname = bam_get_qname(bamdata);
        const char *rname = sam_hdr_tid2name(in_sam_header, bamdata->core.tid);
        uint32_t *cigar     = bam_get_cigar(bamdata);
        int pos = bamdata->core.pos + 1;  // 0-based to 1-based
        int mapq = bamdata->core.qual;
        int l_qseq = bamdata->core.l_qseq;
        int flag   = bamdata->core.flag;
        int i;
        
        fprintf(fp, "%d: %s\t%s\t%d\t%d\t%d\t%d\n",
               record_count,
               qname ? qname : "",
               rname ? rname : "",
               pos,
               mapq,
               l_qseq,
               flag);

    for(i = 0; i < bamdata->core.n_cigar; i++){
        fprintf(fp, "CIGAR: %d%c", bam_cigar_oplen(cigar[i]), bam_cigar_opchr(cigar[i]));
        fprintf(fp, "\n");
        }

        record_count++;

        // limit output for testing, don't do the whole file!!
        /* TODO: REMOVE BEFORE DAY 10 */
        if (record_count >= 10000) break;
    }

    if (ret_r < -1) {
        fprintf(stderr, "Error reading BAM file (code: %d)\n", ret_r);
        goto cleanup;
    }

    printf("Processed %d records.\n", record_count);
    ret = EXIT_SUCCESS;

cleanup:
    if (bamdata) bam_destroy1(bamdata);
    if (in_sam_header) sam_hdr_destroy(in_sam_header);
    if (inputfile) sam_close(inputfile);
    return ret;
}
