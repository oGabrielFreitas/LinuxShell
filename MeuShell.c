/*
-   Developed by:           Gabriel Giacomini de Freitas
-   Date:                   10/2019
-   Object:                 Operation System Curse - UFSM
-   Official Repository:    https://github.com/oGabrielFreitas/LinuxShell
-   Goal:                   Develop a personal shell for Linux
-
-   List of Commands:
-   > Use 'debug' command or '-d' as initial argument to debug
-
-   > Define environment variables by using the syntax: @var_name = var_value
-       You can redefine a value for same variable!
-       You can asset a command and execute with the variable name. Like set @var = imprime ola. And then set @var to execute
-
-   > dir:                  Fork and /bin/ls (accept ls arguments)
-   > espera 'time':        Fork and sleep 'time'
-   > executa 'dir':        Fork and execute 'dir' program
-   > agora:                Fork and show date (/bin/date) (accept date arguments)
-   > limpar:               Reset screen
-   > historico:            Show last 10 commands useds
-   > debug:                Set on/off debug mode
-   > imprime '@var_name':  Show var_value of '@var_name'
-   > imprime 'value':      Just show 'value'
-   > sair:                 Exit
-   
-   Take a look in README for more information.
-
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <sys/wait.h>
#include <sys/types.h>

//Define cores para printf
#define RESET       "\033[0m"
#define BLACK       "\033[30m"             /* Black */
#define RED         "\033[31m"             /* Red */
#define GREEN       "\033[32m"             /* Green */
#define YELLOW      "\033[33m"             /* Yellow */
#define BLUE        "\033[34m"             /* Blue */
#define MAGENTA     "\033[35m"             /* Magenta */
#define CYAN        "\033[36m"             /* Cyan */
#define WHITE       "\033[37m"             /* White */
#define BOLDBLACK   "\033[1m\033[30m"      /* Bold Black */
#define BOLDRED     "\033[1m\033[31m"      /* Bold Red */
#define BOLDGREEN   "\033[1m\033[32m"      /* Bold Green */
#define BOLDYELLOW  "\033[1m\033[33m"      /* Bold Yellow */
#define BOLDBLUE    "\033[1m\033[34m"      /* Bold Blue */
#define BOLDMAGENTA "\033[1m\033[35m"      /* Bold Magenta */
#define BOLDCYAN    "\033[1m\033[36m"      /* Bold Cyan */
#define BOLDWHITE   "\033[1m\033[37m"      /* Bold White */

//Define fun��o para posicionar ponteiro no console
#define gotoxy(x,y) printf("\033[%d;%dH", (y), (x))

#define SPACE " \n" //Define espa�o
#define CMD_MAX 255 //Tamanho maximo de comando
#define VAR_MAX 16 //N�mero m�ximo de vari�veis de ambiente

//Flag global de debug
int flag_debug = 0;

//Struct que define variaveis de ambiente
typedef struct
{
    int id;
    char * name;
    char * value;
    char * arg;
    
}varAm;

//Struct que define fun��o a ser salva
typedef struct 
{  
    int id;
    char * value;
}histS;

//Fun��o que LIGA/DESLIGA Debug mode
int setDebug(int flag_debug){

    if(flag_debug == 1){
        printf(BOLDYELLOW"DEBUG: Debug mode - off\n"RESET);
        return 0;
    }else{
        printf(BOLDYELLOW"DEBUG: Debug mode - on\t Cores: "RESET YELLOW" PAI "CYAN" FILHO "WHITE" FUN��ES \n"RESET);
        return 1;
    }
}

//Fun��o que encontra vari�vel pelo nome
int findVarByName(char * name, varAm * varVet){

    int i = 0;

    if(flag_debug){printf(WHITE"DEBUG: Fun��o "GREEN"findVarByName"WHITE" : Name passado: >%s<\n"RESET, name);}    

    for(i = 0 ; varVet[i].name != NULL ; i++){

        if(flag_debug){printf(WHITE"DEBUG: Fun��o "GREEN"findVarByName"WHITE" : TEST > id: %d / Name: %s / Value: %s\n"RESET, varVet[i].id, varVet[i].name, varVet[i].value);}

        if(!strcmp(varVet[i].name,name)){
            if(flag_debug){printf(WHITE"DEBUG: Fun��o "GREEN"findVarByName"WHITE" : ACHOU no id: %d\n"RESET, varVet[i].id);}

            return varVet[i].id;
        }
    }
    if(flag_debug){printf(WHITE"DEBUG: Fun��o "GREEN"findVarByName"WHITE" : N�O ENCONTROU - Return: -1\n"RESET);}
    return -1;
}

//Fun��io que encontra a quantidade de vari�veis em hist�rico
int getHistIndex(histS * hist){
    if(flag_debug){printf(WHITE"DEBUG: Fun��o "GREEN"getHistIndex"WHITE" : ENTROU\n"RESET);}

    int i = 0;

    for(i = 0; hist[i].value != NULL; i++){
        //if(flag_debug){printf(WHITE"DEBUG: Fun��o "GREEN"getHistIndex"WHITE" : TESTOU id: %d\n"RESET, i);}
        if(i > 8){
            return -1;
        }
    }
    return i;
}

//Fun��o que salva e realoca vari�veis do hist�rico
void indexHist(histS * hist, char * last){
    if(flag_debug){printf(WHITE"DEBUG: Fun��o "GREEN"indexHist"WHITE" : ENTROU para salvar: %s\n"RESET, last);}

    int i = getHistIndex(hist);
    int n = 1;

    if(i == -1){
        if(flag_debug){printf(WHITE"DEBUG: Fun��o "GREEN"indexHist"WHITE" : Mais de 10 vari�veis, ser�o agora realocadas\n"RESET);}

        for(n = 1; n < 10 ; n++){       
            //printf("alocou %d\n", n);     
            hist[n-1].value = malloc(sizeof(histS));
            hist[n-1].value = hist[n].value;
        }
        //printf("alocou 10\n");
        hist[9].value = malloc(sizeof(histS));
        hist[9].value = last;

    }else{
        if(flag_debug){printf(WHITE"DEBUG: Fun��o "GREEN"indexHist"WHITE" : Vari�vel \"%s\" alocada no id: %d\n"RESET, last, i);}

        hist[i].value = malloc(sizeof(histS));
        hist[i].value = last;
    }
}

//----------------------------------------
//MAIN
int main(int argc, char const *argv[])
{

    //Strings
    char command[CMD_MAX];
    char * cmds[CMD_MAX];

    //Hist�rico de fun��es
    histS hist[CMD_MAX];
    char * temp;

    //Variaveis de ambiente
    varAm var[VAR_MAX];
    int var_id = 0;

    //Flags
    int flag_repeat = 0;
    int flag_not_read = 0;

    //Vari�veis
    int cmd_i = 1;
    int n = 0;
    int i = 0;
    int get_id, temp_id;

    //Ter debug como argumento de entrada
    if(argc > 1)
        if(!strcmp(argv[1],"-d"))
            flag_debug = 1;

    system("reset"); //Limpa console

    //Programa entra em loop at� 'sair'
        while(1){

            do{
                flag_repeat = 0; //Flag para n�o repetir o do
                
                //Flag para n�o ler
                if(!flag_not_read){
                //Do While para corrigir bug de entrada vazia
                    do{
                        memset(cmds, 0, sizeof(cmds));  //Limpa cmds

                        printf(BOLDCYAN"msh> "RESET); //Imprime index do Shell
                        fgets(command, CMD_MAX, stdin); //Le entrada de argumentos do usu�rio

                        //Separa argumentos em um array char
                        cmds[0] = strtok(command, SPACE);
                        for(cmd_i = 1 ; cmds[cmd_i-1] != NULL ; cmd_i++){
                            cmds[cmd_i] = strtok(NULL, SPACE);
                        }
                    }while(cmds[0]==NULL); //Repete enquanto o usu�rio n�o insere nada
                }

                //Insere comando na array de hist�rico de comandos
                //Faz uma c�pia de cmds[0] para temp antes, pois strtok edita os valores como ponteiro mesmo no futuro                
                
                temp = malloc(sizeof(cmds[0]));
                strcpy(temp,cmds[0]);
                indexHist(hist,temp);                          
                               
                //DEBUG: Imprime separadamente array de argumentos
                if(flag_debug){
                    for(i = 0; cmds[i];i++){
                        printf(YELLOW"DEBUG: CMDS[%d] =>%s<\n"RESET, i, cmds[i]);
                    }
                }

                //----------------------------------------------------------------
                //Comandos que n�o precisam de fork

                //COMANDO SAIR
                if(!strcmp(cmds[0],"sair")){
                    if(flag_debug){printf(YELLOW"DEBUG: Comando SAIR\n"RESET);}
                    exit(0);
                }

                //COMANDO LIGAR/DESLIGAR MODO DEBUG
                else if(!strcmp(cmds[0],"debug")){
                    if(flag_debug){printf(YELLOW"DEBUG: Entrou em DEBUG\n"RESET);}

                    flag_repeat = 1;
                    flag_not_read = 0;

                    flag_debug = setDebug(flag_debug);
                }

                //COMANDO LIMPAR SHELL
                else if(!strcmp(cmds[0],"limpar")){
                    if(flag_debug){printf(YELLOW"DEBUG: Entrou em LIMPAR\n"RESET);}

                    flag_repeat = 1;
                    flag_not_read = 0;

                    system("reset");
                }

                //COMANDO HISTORICO
                else if(!strcmp(cmds[0],"historico")){
                    if(flag_debug){printf(YELLOW"DEBUG: Entrou em HIST�RICO\n"RESET);}

                    flag_repeat = 1;
                    flag_not_read = 0;

                    n = getHistIndex(hist);

                    if(n == -1){n=10;}

                    for(i = 0 ; i < n ; i++){
                        printf("> %d: %s\n",i, hist[i].value);
                    }

                }

                //COMANDO IMPRIME
                else if(!strcmp(cmds[0],"imprime")){
                    if(flag_debug){printf(YELLOW"DEBUG: Entrou em IMPRIME\n"RESET);}

                    flag_repeat = 1;
                    flag_not_read = 0;

                    if(cmds[1] == NULL){ //Verifica se tem argumento
                        printf(RED"Comando \"%s\" necessita de ao menos 1 par�metro.\n"RESET,cmds[0]);
                    }
                    else if(cmds[1][0] == '@'){ //Verifica se o argumento � uma vari�vel de ambiente

                        get_id = findVarByName(cmds[1], var);

                        if(get_id == -1){
                            printf(RED"Vari�vel \"%s\" n�o encontrada.\n"RESET, cmds[1]);
                        }else{
                            printf("%s\n", var[get_id].value);
                        }

                    }else{  //Se n�o for uma vari�vel, simplesmente imprime o valor
                        printf("%s\n", cmds[1]);
                    }         
                }

                //COMANDO PARA CRIAR VARI�VEL DE AMBIENTE
                /*Verifica se o primeiro char � um '@' que define vari�vel. 
                Verifica se existe algo depois do @, para n�o ser poss�vel criar uma vari�vel chamada @
                Verifica se existe segundo argumento (para evitar falha de segmenta��o)
                Verifica se existe o operador =
                Verifica se existe argumento para ser indexado � vari�vel*/
                else if(cmds[0][0] == '@' && cmds[0][1] != '\0' && cmds[1] != NULL && !strcmp(cmds[1],"=") && cmds[2] != NULL){
                    if(flag_debug){printf(YELLOW"DEBUG: Entrou em CRIAR VARI�VEL\n"RESET);}

                        flag_repeat = 1;
                        flag_not_read = 0;

                        get_id = findVarByName(cmds[0], var);

                        if(get_id == -1){       //Se n�o houver uma vari�vel igual
                            temp_id = var_id;   //Define para salvar no var_id
                            var_id++;           //Incrementa o var_id para salvar a pr�xima

                        }else{
                            temp_id = get_id;   //Se houver vari�vel igual, a fun��o findVarByName vai retornar seu id
                        }

                        var[temp_id].id = temp_id;

                        var[temp_id].name = malloc(1 + sizeof(varAm));
                        strcpy(var[temp_id].name,cmds[0]);

                        var[temp_id].value = malloc(1 + sizeof(varAm));
                        strcpy(var[temp_id].value,cmds[2]);

                        if(cmds[3] != NULL){    //Verifica se a vari�vel tem argumento, se tiver, salva ele
                            var[temp_id].arg = malloc(1 + sizeof(varAm));
                            strcpy(var[temp_id].arg,cmds[3]);

                            if(flag_debug){printf(YELLOW"DEBUG: Vari�vel possui argumento -> var.arg = %s\n", cmds[3]);}

                        }else{//Se n�o tiver, deixa o argumento nulo (Para caso de reescrita de vari�vel)
                            var[temp_id].arg = malloc(1 + sizeof(varAm));
                            var[temp_id].arg = NULL;
                        }

                        if(flag_debug){printf(YELLOW"DEBUG: Vari�vel salva como -> var.id = %d / var.name = %s / var.value = %s \n"RESET, var[temp_id].id, var[temp_id].name, var[temp_id].value);}
                    }

                    //COMANDO PARA A VARI�VEL EXECUTAR FUN��O (DEVE SER O �LTIMO COMANDO)
                    else if(cmds[0][0] == '@'){
                        if(flag_debug){printf(YELLOW"DEBUG: Entrou para tentar executar valor anexado � vari�vel\n"RESET);}
                        
                        flag_repeat = 1;
                        flag_not_read = 0;

                        get_id = findVarByName(cmds[0], var);

                        if(get_id == -1){ //Verifica se a vari�vel existe
                            printf(RED"A vari�vel \"%s\" n�o foi declarada.\n"RESET,cmds[0]);
                        }else{
                            flag_not_read = 1; // Volta ao topo, mas n�o le do usu�rio
                            memset(cmds, 0, sizeof(cmds));  //Limpa cmds

                            cmds[0] = var[get_id].value; //Passa fun��o para escopo

                            if(var[get_id].arg != NULL){    //Verifica se a vari�vel tem um argumento, se tiver, idexa ele no escopo
                                cmds[1] = var[get_id].arg;
                            }
                        }                        
                    }

            }while(flag_repeat); //Repete caso tenha executado alguma fun��o deste escopo (pois as fun��es aqui n�o precisam de fork)

            flag_not_read = 0; //Necess�ria para n�o dar descontinuidade ao programa

            //--------------------------------------------------
            //Comandos que precisam de FORK

            pid_t pid = fork(); //Faz fork        

            switch(pid)
            {
                //Caso fork der errado
                case -1:
                    printf("Erro!\n");
                    exit(EXIT_FAILURE);

                //Caso fork der certo
                case 0:     

                    if(flag_debug){printf(CYAN"DEBUG: Fez Fork - PID: %d\n"RESET, pid);}                

                    //COMANDO DIR
                    if(!strcmp(cmds[0],"dir")){  
                        if(flag_debug){printf(CYAN"DEBUG: Entrou em DIR\n"RESET);}                  
                        execve("/bin/ls",cmds,NULL); 
                        return 1;                 
                    }

                    //COMANDO ESPERA
                    else if(!strcmp(cmds[0],"espera")){
                        if(flag_debug){printf(CYAN"DEBUG: Entrou em ESPERA\n"RESET);}
                        if(cmds[1] == NULL) {
                            printf(RED"Comando \"%s\" necessita de ao menos 1 par�metro.\n"RESET,cmds[0]);
                            exit(0);
                        }
                        sleep(atoi(cmds[1]));
                        exit(0);
                    }

                    //COMANDO EXECUTA
                    //Exemplo para testar executa: >executa /bin/sleep 5
                    else if(!strcmp(cmds[0],"executa")){
                        if(flag_debug){printf(CYAN"DEBUG: Entrou em EXECUTA\n"RESET);}
                        if(cmds[1] == NULL) {
                            printf(RED"Comando \"%s\" necessita de ao menos 1 par�metro.\n"RESET,cmds[0]);
                            exit(0);
                        }
                        execve(cmds[1], (cmds+sizeof(char)),NULL);
                    }

                    //COMANDO AGORA
                    else if(!strcmp(cmds[0],"agora")){
                        if(flag_debug){printf(CYAN"DEBUG: Entrou em AGORA\n"RESET);}
                        execve("/bin/date", cmds,NULL);
                    }

                    //SE NENHUM COMANDO FOI EXECUTADO
                    else{
                        if(flag_debug){printf(CYAN"DEBUG: Filho n�o executou nada e vai se matar\n"RESET);}
                        printf(RED"Comando \"%s\" n�o reconhecido.\n"RESET,cmds[0]);
                        exit(0);
                    }                        
                break;

                //Default para o processo PAI
                default:
                    if(flag_debug){printf(YELLOW"DEBUG: Pai passou para default e vai esperar filho\n"RESET);}
                    wait(NULL);

                    // Pai mata o filho
                    if(flag_debug){printf(YELLOW"DEBUG: Pai vai matar filho com PID: %d\n"RESET, pid);}
                    kill(pid,SIGKILL); //Mata o filho

                    if(flag_debug){printf(YELLOW"DEBUG: Pai saiu da espera\n"RESET);}
                break;

            }           
        } //Fim do While

    return 0;
}

//Fim.