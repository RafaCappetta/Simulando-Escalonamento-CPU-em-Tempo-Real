#ifndef RATE_H
#define RATE_H

#include <stdio.h>
#include <time.h>

typedef struct Task{
    char *task_name;
    double period;
    double burst;
    double burst_sobrando;
    time_t tempo_passado;
    time_t burst_feito;
    int lost_dl;
    int completed_exec;
    int killed;
    int iniciar;
    int completou_exec_atual;
    struct Task *prox;
} Task;

Task* criar_task(FILE *arquivo, int *verifica_fim_arquivo);

void inserir_inicio_lista_task(Task **head, FILE *arquivo);

void escalonamento_rate(Task *h, double total_exec_time);

void escalonamento_edf(Task *h, double total_exec_time);

#endif