#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <time.h>
#include "task.h"

int main(int argc, char **argv){

    if(argc != 2){
        printf("Quantidade de argumentos incorreta\n");
        exit(1);
    }

    FILE *f = fopen(argv[1], "r");

    if(f == NULL){
        printf("ERRO - falha ao tentar abrir arquivo %s", argv[1]);
        exit(1);
    }else{
        Task *head = NULL;
        double total_exec_time;

        fscanf(f, "%lf", &total_exec_time);
        printf("%1.lf\n", total_exec_time);

        inserir_inicio_lista_task(&head, f);
    }

    fclose(f);

    
    return 0;
}