#include <stdio.h>
#include <julia.h>
#include "../includes/header.h"

int main(int argc, char* argv[]){
	
	char args1[100];
	if(argc < 2){
		printf("Program usage: vvcf input_file --flag1 --flag2 etc\n");
	}

	fgets(args1, sizeof(args1), stdin);
	printf("Argument was: %s\n", argv[1]);
	return 0;
}
