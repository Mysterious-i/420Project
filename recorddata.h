#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

int record(char *file_name,int nt,int iters,double exec_time)
{
    FILE *file;
    if((file = fopen(file_name, "a")) == 0){
        printf("error openening file %s.\n", file_name);
        exit(-1);
    }
    fprintf(file,"%d",nt);
    fprintf(file,"%c",',');
    fprintf(file,"%d",iters);
    fprintf(file,"%c",',');
    fprintf(file,"%e",exec_time);
    fprintf(file,"%c",'\n');
    fclose(file);
    
    return 0;
}
