#ifndef HEADER_H
#define HEADER_H

#include <stdio.h>
#include <stdint.h>


typedef struct vcf_header {
	char 		*chrom;
	int32_t  	 pos;
	char 		*id;
	char 		*ref;
	char 		*alt;
	char 		 info[256];
} vcfheader;

typedef struct table_header {
	char		*gene;
	char		*variant;
	char		*zygosity;
	char		*clinical_significance;
	char		*recommendation;
} tableheader;

typedef struct report_header {
	int32_t		patient_id;
	int32_t		date;
	char		*genome;
	char		*guid;
} report_header;

#endif
