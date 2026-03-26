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

        if((*verifica_fim_arquivo != 3 && *verifica_fim_arquivo == EOF) || n->period == 0 || n->burst == 0){
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
        n->tempo_passado = time(NULL); // inicializa com o instante atual
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

void escalonamento_rate(Task *h, double total_exec_time){
    FILE *saida = fopen("rate_rcb2.out", "w+");
    
    if(saida == NULL){
        printf("ERRO - falha ao tentar abrir arquivo de saida dos dados.\n");
        exit(1);
        
    }else{
        fprintf(saida, "EXECUTION BY RATE");
        fprintf(saida, "\n\n");

        Task *sendo_executada = NULL;
        double inicio_execucao = time(NULL);
        double fim_execucao = inicio_execucao + (double) total_exec_time;
        int em_idle = 0;
        double idle_cont;
        inicio_execucao = idle_cont = time(NULL);
        

        while (time(NULL) - fim_execucao){
            Task *aux = h;
            Task *menor_burst = NULL;

            int em_idle = 0;
            while (aux != NULL){
                if(difftime(time(NULL), aux->tempo_passado) >= aux->period){
                    if(sendo_executada == aux && sendo_executada->completou_exec_atual == 0){ 
                        double valor_task_executado_agora = difftime(time(NULL), sendo_executada->burst_feito);
                        
                        fprintf(saida, "[%s] for %1.lf units - L\n", sendo_executada->task_name, valor_task_executado_agora);
                        
                        sendo_executada->lost_dl++;
                    }
                    aux->completou_exec_atual = 0;
                    aux->tempo_passado = time(NULL);
                    aux->burst_sobrando = aux->burst;
                    aux->iniciar = 1;
                }

                if(menor_burst == NULL && aux->completou_exec_atual == 0){
                    menor_burst = aux;
                }

                if(menor_burst != NULL){
                    if(aux->completou_exec_atual == 0){
                        if(aux->burst < menor_burst->burst){
                            menor_burst = aux;
                        }
                    }
                }

                aux = aux->prox;
            }

            if(sendo_executada != menor_burst){

                if(sendo_executada == NULL && em_idle == 1){
                    double valor_task_executado_agora = difftime(time(NULL), idle_cont);
                    if(valor_task_executado_agora > 0){
                        fprintf(saida, "idle for %1.lf units", valor_task_executado_agora);
                    }
                
                }else if(sendo_executada != NULL && sendo_executada->completou_exec_atual == 0){
                    double valor_task_executado_agora = difftime(time(NULL), sendo_executada->burst_feito);
                    sendo_executada->burst_sobrando = sendo_executada->burst - valor_task_executado_agora;
                    
                    if(sendo_executada->burst_sobrando < 0){
                        sendo_executada->burst_sobrando = 0;
                    }

                    fprintf(saida, "[%s] for %1.lf units - H\n", sendo_executada->task_name, sendo_executada->burst - sendo_executada->burst_sobrando);
                }

                if(menor_burst != NULL){
                    menor_burst->burst_feito = time(NULL);
                    menor_burst->iniciar = 0;
                    em_idle = 0;
                }else{
                    em_idle = 1;
                    idle_cont = time(NULL);
                }

                sendo_executada = menor_burst;
            }

            if(menor_burst == NULL){
                em_idle = 1;
                sendo_executada = NULL;
            }else{
                if(menor_burst->iniciar == 1){
                    menor_burst->burst_feito = time(NULL);
                    menor_burst->iniciar = 0;
                }

                if(difftime(time(NULL), menor_burst->burst_feito) >= menor_burst->burst_sobrando){
                    fprintf(saida, "[%s] for %1.lf units - F\n", menor_burst->task_name, menor_burst->burst_sobrando);
                    menor_burst->completou_exec_atual = 1;
                    menor_burst->completed_exec++;
                    menor_burst->burst_feito = time(NULL);
                    sendo_executada = NULL;
                    em_idle = 1;
                    idle_cont = time(NULL);
                }
            }
        }

        if(sendo_executada != NULL && sendo_executada->completou_exec_atual == 0){
            double valor_task_executado_agora = difftime(time(NULL), sendo_executada->burst_feito);
            sendo_executada->killed++;
            fprintf(saida, "[%s] for %1.lf units - K", sendo_executada->task_name, valor_task_executado_agora);
        }else if(sendo_executada == NULL && em_idle == 1){
            double valor_task_executado_agora = difftime(time(NULL), idle_cont);
            if(valor_task_executado_agora > 0){
                fprintf(saida, "idle for %1.lf units", valor_task_executado_agora);
            }
        }

        Task *para_imprimir = h;

        fprintf(saida, "\n");
        fprintf(saida, "LOST DEADLINES\n");

        while (para_imprimir != NULL){
            fprintf(saida, "[%s] %d\n", para_imprimir->task_name, para_imprimir->lost_dl);
            para_imprimir = para_imprimir->prox;
        }
        
        fprintf(saida, "\n");

        fprintf(saida,"COMPLETE EXECUTION\n");
        
        para_imprimir = h;
        
        while (para_imprimir != NULL){
            fprintf(saida, "[%s] %d\n", para_imprimir->task_name, para_imprimir->completed_exec);
            para_imprimir = para_imprimir->prox;
        }
        
        fprintf(saida, "\n");
        
        fprintf(saida, "KILLED\n");

        para_imprimir = h;

        while (para_imprimir != NULL){
            if(para_imprimir->prox == NULL){
                fprintf(saida, "[%s] %d", para_imprimir->task_name, para_imprimir->killed);
            }else{
                fprintf(saida, "[%s] %d\n", para_imprimir->task_name, para_imprimir->killed);
            }

            para_imprimir = para_imprimir->prox;
        }
        
        fclose(saida);
    }
}

void escalonamento_edf(Task *h, double total_exec_time){
        FILE *saida = fopen("rate_rcb2.out", "w+");
    
    if(saida == NULL){
        printf("ERRO - falha ao tentar abrir arquivo de saida dos dados.\n");
        exit(1);
        
    }else{
        fprintf(saida, "EXECUTION BY RATE");
        fprintf(saida, "\n\n");

        Task *sendo_executada = NULL;
        double inicio_execucao = time(NULL);
        double fim_execucao = inicio_execucao + (double) total_exec_time;
        int em_idle = 0;
        double idle_cont;
        inicio_execucao = idle_cont = time(NULL);
        

        while (time(NULL) - fim_execucao){
            Task *aux = h;
            Task *menor_period = NULL;

            int em_idle = 0;
            while (aux != NULL){
                if(difftime(time(NULL), aux->tempo_passado) >= aux->period){
                    if(sendo_executada == aux && sendo_executada->completou_exec_atual == 0){ 
                        double valor_task_executado_agora = difftime(time(NULL), sendo_executada->burst_feito);
                        
                        fprintf(saida, "[%s] for %1.lf units - L\n", sendo_executada->task_name, valor_task_executado_agora);
                        
                        sendo_executada->lost_dl++;
                    }
                    aux->completou_exec_atual = 0;
                    aux->tempo_passado = time(NULL);
                    aux->burst_sobrando = aux->burst;
                    aux->iniciar = 1;
                }

                if(menor_period == NULL && aux->completou_exec_atual == 0){
                    menor_period = aux;
                }

                if(menor_period != NULL){
                    if(aux->completou_exec_atual == 0){
                        if(aux->period < menor_period->period){
                            menor_period = aux;
                        }
                    }
                }

                aux = aux->prox;
            }

            if(sendo_executada != menor_period){

                if(sendo_executada == NULL && em_idle == 1){
                    double valor_task_executado_agora = difftime(time(NULL), idle_cont);
                    if(valor_task_executado_agora > 0){
                        fprintf(saida, "idle for %1.lf units", valor_task_executado_agora);
                    }
                
                }else if(sendo_executada != NULL && sendo_executada->completou_exec_atual == 0){
                    double valor_task_executado_agora = difftime(time(NULL), sendo_executada->burst_feito);
                    sendo_executada->burst_sobrando = sendo_executada->burst - valor_task_executado_agora;
                    
                    if(sendo_executada->burst_sobrando < 0){
                        sendo_executada->burst_sobrando = 0;
                    }

                    fprintf(saida, "[%s] for %1.lf units - H\n", sendo_executada->task_name, sendo_executada->burst - sendo_executada->burst_sobrando);
                }

                if(menor_period != NULL){
                    menor_period->burst_feito = time(NULL);
                    menor_period->iniciar = 0;
                    em_idle = 0;
                }else{
                    em_idle = 1;
                    idle_cont = time(NULL);
                }

                sendo_executada = menor_period;
            }

            if(menor_period == NULL){
                em_idle = 1;
                sendo_executada = NULL;
            }else{
                if(menor_period->iniciar == 1){
                    menor_period->burst_feito = time(NULL);
                    menor_period->iniciar = 0;
                }

                if(difftime(time(NULL), menor_period->burst_feito) >= menor_period->burst_sobrando){
                    fprintf(saida, "[%s] for %1.lf units - F\n", menor_period->task_name, menor_period->burst_sobrando);
                    menor_period->completou_exec_atual = 1;
                    menor_period->completed_exec++;
                    menor_period->burst_feito = time(NULL);
                    sendo_executada = NULL;
                    em_idle = 1;
                    idle_cont = time(NULL);
                }
            }
        }

        if(sendo_executada != NULL && sendo_executada->completou_exec_atual == 0){
            double valor_task_executado_agora = difftime(time(NULL), sendo_executada->burst_feito);
            sendo_executada->killed++;
            fprintf(saida, "[%s] for %1.lf units - K", sendo_executada->task_name, valor_task_executado_agora);
        }else if(sendo_executada == NULL && em_idle == 1){
            double valor_task_executado_agora = difftime(time(NULL), idle_cont);
            if(valor_task_executado_agora > 0){
                fprintf(saida, "idle for %1.lf units", valor_task_executado_agora);
            }
        }

        Task *para_imprimir = h;

        fprintf(saida, "\n");
        fprintf(saida, "LOST DEADLINES\n");

        while (para_imprimir != NULL){
            fprintf(saida, "[%s] %d\n", para_imprimir->task_name, para_imprimir->lost_dl);
            para_imprimir = para_imprimir->prox;
        }
        
        fprintf(saida, "\n");

        fprintf(saida,"COMPLETE EXECUTION\n");
        
        para_imprimir = h;
        
        while (para_imprimir != NULL){
            fprintf(saida, "[%s] %d\n", para_imprimir->task_name, para_imprimir->completed_exec);
            para_imprimir = para_imprimir->prox;
        }
        
        fprintf(saida, "\n");
        
        fprintf(saida, "KILLED\n");

        para_imprimir = h;

        while (para_imprimir != NULL){
            if(para_imprimir->prox == NULL){
                fprintf(saida, "[%s] %d", para_imprimir->task_name, para_imprimir->killed);
            }else{
                fprintf(saida, "[%s] %d\n", para_imprimir->task_name, para_imprimir->killed);
            }

            para_imprimir = para_imprimir->prox;
        }
        
        fclose(saida);
    }
}