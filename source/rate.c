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
    Task *head = NULL;
    double total_exec_time;

    if(f == NULL){
        printf("ERRO - falha ao tentar abrir arquivo %s\n", argv[1]);
        exit(1);
    }else{
        fscanf(f, "%lf", &total_exec_time);
        printf("%1.lf\n", total_exec_time);

        inserir_inicio_lista_task(&head, f);
        escalonamento_rate(head, total_exec_time);
    }

    fclose(f);

    return 0;
}