/* CYP2C19 Diplotype Phenotype Lookup */ 

/* frontloading all the logic into C lookups instead of 
   building a replica of the XLSX sheet in Julia and then 
   generating a superfluous C header 
*/

#ifndef DTABLE_LUT_H
#define DTABLE_LUT_H

#include <stdio.h>
#include <string.h>
#include "cyp2c19_allele_lut.h" 


typedef enum {
	FUNC_NORMAL = 0,
    FUNC_DECREASED = 1,
    FUNC_NO_FUNCTION = 2,
    FUNC_INCREASED = 3,
    FUNC_UNCERTAIN = 4,
    FUNC_UNKNOWN = 5
} cyp2c19_allele_function;

static const struct {
    const char *name;
    cyp2c19_allele_function func;
} cyp2c19_function_status[] = {
    
	/* normal function */
    {"*1",  FUNC_NORMAL},
    {"*11", FUNC_NORMAL},
    {"*13", FUNC_NORMAL},
    {"*15", FUNC_NORMAL},
    {"*18", FUNC_NORMAL},
    {"*28", FUNC_NORMAL},
    {"*38", FUNC_NORMAL},

    /* no function */ 
    {"*2",  FUNC_NO_FUNCTION},
    {"*3",  FUNC_NO_FUNCTION},
    {"*4",  FUNC_NO_FUNCTION},
    {"*5",  FUNC_NO_FUNCTION},
    {"*6",  FUNC_NO_FUNCTION},
    {"*7",  FUNC_NO_FUNCTION},
    {"*8",  FUNC_NO_FUNCTION},
    {"*22", FUNC_NO_FUNCTION},
    {"*24", FUNC_NO_FUNCTION},
    {"*35", FUNC_NO_FUNCTION},
    {"*36", FUNC_NO_FUNCTION},
    {"*37", FUNC_NO_FUNCTION},

    /* increased function */
    {"*17", FUNC_INCREASED},

    /* decreased function */
    {"*9",  FUNC_DECREASED},
    {"*10", FUNC_DECREASED},
    {"*16", FUNC_DECREASED},
    {"*19", FUNC_DECREASED},
    {"*25", FUNC_DECREASED},
    {"*26", FUNC_DECREASED},

    /* uncertain function */
    {"*12", FUNC_UNCERTAIN},
    {"*14", FUNC_UNCERTAIN},
    {"*23", FUNC_UNCERTAIN},
    {"*29", FUNC_UNCERTAIN},
    {"*30", FUNC_UNCERTAIN},
    {"*31", FUNC_UNCERTAIN},
    {"*32", FUNC_UNCERTAIN},
    {"*33", FUNC_UNCERTAIN},
    {"*34", FUNC_UNCERTAIN},
    {"*39", FUNC_UNCERTAIN},
    {"*40", FUNC_UNCERTAIN},
    {"*41", FUNC_UNCERTAIN},
    {"*42", FUNC_UNCERTAIN},
};

#define CYP2C19_FUNCTION_COUNT (sizeof(cyp2c19_function_status) / sizeof(cyp2c19_function_status[0]))

static inline cyp2c19_allele_function cyp2c19_get_function(const char* name){
    for(size_t i = 0; i < CYP2C19_FUNCTION_COUNT; i++){
        if(strcmp(cyp2c19_function_status[i].name, name) == 0){
            return cyp2c19_function_status[i].func;
        }
    }
    return FUNC_UNKNOWN;
}

#endif