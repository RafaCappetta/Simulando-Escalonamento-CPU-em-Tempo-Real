#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "task.h"

Task* criar_task(FILE *arquivo, int *verifica_fim_arquivo){
    Task *n = (Task *)malloc(sizeof(Task));

    if(n != NULL){
        char ch;
        int tam_nome = 0;
        int posicao_pre_cont = ftell(arquivo);

        while((ch = fgetc(arquivo)) != EOF && ch != ' '){
            if(ch == '\n'){
                printf("ERRO - formatacao do arquivo incorreta.\n");
                exit(1);
            }
            tam_nome++;
        }

        if(ch == EOF || tam_nome == 0){
            printf("ERRO - parametro invalido para nome da task\n");
            exit(1);
        }

        n->task_name = (char*)malloc((tam_nome + 1) * sizeof(char));

        if(n->task_name == NULL){
            printf("ERRO - falha alocando memória para o nome da task\n");
            exit(1);
        }

        fseek(arquivo, posicao_pre_cont, SEEK_SET);

        *verifica_fim_arquivo = fscanf(arquivo, "%s %lf %lf", n->task_name, &n->period, &n->burst);

        if(*verifica_fim_arquivo != 3 && *verifica_fim_arquivo == EOF){
            free(n->task_name);
            free(n);
            printf("ERRO - argumentos invalidos para preencher elementos fundamentais da task\n");
            exit(1);
        }
        
        n->lost_dl = 0;
        n->completed_exec = 0;
        n->completou_exec_atual = 0;
        n->killed = 0;
        n->burst_sobrando = n->burst;
        n->tempo_passado = time(NULL) - n->period;
        n->iniciar = 0;
        n->prox = NULL;

        printf("%s %1.lf %1.lf\n", n->task_name, n->period, n->burst);
    }
    
    return n;
}

void inserir_inicio_lista_task(Task **head, FILE *arquivo){
    int fim_arquivo = fgetc(arquivo);

    if(fim_arquivo == EOF){
        printf("ERRO - arquivo chegou ao fim sem argumentos suficientes para criar structs");
        exit(1);
    }

    while(fim_arquivo != EOF){
        Task *novo = criar_task(arquivo, &fim_arquivo);

        fim_arquivo = fgetc(arquivo); //para verificar EOF.
        
        novo->prox = *head;
        *head = novo;
    }
}

void escalonamento_rate(Task *h, double total_exec_time){}

void escalonamento_edf(Task *h, double total_exec_time){}