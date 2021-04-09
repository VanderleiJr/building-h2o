/* Código feito por Vanderlei Junior, 636932
   Para a disciplina de Sistema Operacionais, ENPE 2020/2

   Less Classical Synchonization Problem - Building H2O
   	Cada molécula de água deve ser composta por dois átomos de hidrogênio e um átomo de oxigênio.
   	Quando as duas tiverem sido criadas, na saída, uma molécula de água é formada.

   Compilar com -pthread

   Abril, 2021
*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <semaphore.h>

#define TRUE 1		//Comodidade

//PODEM ser alterados para teste
#define BUFFER_OXIG 5		//Tamanho máximo do BUFFER de Oxigênio
#define BUFFER_HIDR 10		//Tamanho Máximo do BUFFER de Hidrogênio

#define TEMPO_OXI 2		//Tempo de produção de cada elemento de Oxigênio em segundos
#define TEMPO_HIDRO 1		//Tempo de produção de cada elemento de Hidrogênio em segundos

//Variáveis globais
sem_t s_oxigenio, s_hidrogenio;		//Semáforos
pthread_barrier_t b_molecula;		//Barreiras
int oxigenio = 0; int hidrogenio = 0; int moleculas = 0;	//Quantidade de moléculas criadas

int inOxi = 0; int inHid = 0;					//Quantidade de moléculas no BUFFER
int _bufferOxig[BUFFER_OXIG], _bufferHidr[BUFFER_HIDR]; 	//Respectivos BUFFERS

/*
Função para inserir o elemento criado no seu respectivo BUFFER.
A verificação por char é bem simples e pode ser alterada para outra com mais segurança.
A mensagem printada mostra o número do elemento criado e a quantidade que já tem no BUFFER.
*/
void insereElemento(char elemento, int item){
	if(elemento == 'O'){
		_bufferOxig[inOxi] = item;
		printf("Inseriu o Oxigênio[%d] (Contém: %d) no BUFFER\n\n", item, ++inOxi); fflush(stdout);
	} else {
		_bufferHidr[inHid] = item;
		printf("Inseriu o Hidrogênio[%d] (Contém: %d) no BUFFER\n\n", item, ++inHid); fflush(stdout);
	}
}

/*
Função para retirar o elemento do seu respectivo BUFFER.
A verificação por char é bem simples e pode ser alterada para outra com mais segurança.
A mensagem printada mostra o número do elemento retirado e a quantidade que ainda resta no BUFFER.
*/
void retiraElemento(char elemento){
	if(elemento == 'O'){
		printf("Utilizou o Oxigênio[%d] (Contém: %d) no BUFFER\n", _bufferOxig[inOxi], --inOxi); fflush(stdout);
	} else {
		printf("Utilizou o Hidrogênio[%d] (Contém: %d) no BUFFER\n", _bufferHidr[inHid], --inHid); fflush(stdout);
	}
}

/*
Função da THREAD do Oxigênio
O tempo é necessário para dar uma aleatoriedade, podendo ser melhorada com srand(),
mas para testes, o valor é passado na criação da THREAD.
O semáforo impede de que a mensagem de BUFFER cheio fique spammando no terminal.
O sinal da barreira é emitido quando o BUFFER fica cheio.
*/
void * funOxigenio(void *tempo){
	int contador = 0;
	do{

		if(inOxi == BUFFER_OXIG){
			printf("BUFFER de Oxigênio CHEIO!\n"); fflush(stdout);
			printf("Sinal enviado!\n\n"); fflush(stdout);
			pthread_barrier_wait(&b_molecula);
			sem_wait(&s_oxigenio);
		} else {
			printf("Produziu Oxigênio[%d]\n", ++oxigenio); fflush(stdout);
			insereElemento('O', oxigenio);
		}
		sleep((long int)tempo);
	} while(TRUE);
}

/*
Função da THREAD do Hidrogênio
O tempo é necessário para dar uma aleatoriedade, podendo ser melhorada com srand(),
mas para testes, o valor é passado na criação da THREAD.
O semáforo impede de que a mensagem de BUFFER cheio fique spammando no terminal.
O sinal da barreira é emitido quando o BUFFER fica cheio.
*/
void * funHidrogenio(void *tempo){
	int contador = 0;
	do{
		if(inHid == BUFFER_HIDR){
			printf("BUFFER de Hidrogênio CHEIO!\n"); fflush(stdout);
			printf("Sinal enviado!\n\n"); fflush(stdout);
			pthread_barrier_wait(&b_molecula);
			sem_wait(&s_hidrogenio);
		} else {
			printf("Produziu Hidrogenio[%d]\n", ++hidrogenio); fflush(stdout);
			insereElemento('H', hidrogenio);
		}
		sleep((long int)tempo);
	} while(TRUE);
}

/*
Função apenas para melhorar a leitura do código, podendo ser inserida dentro
da função da THREAD da Molécula.
A criação da molécula se dá até ser possível criar novas moléculas. Após isso,
emite sinal pra criação de novos elementos, pausando a THREAD da criação das moléculas.
*/
void criaMolecula(){
	while((inOxi > 0) && (inHid > 1)){
		retiraElemento('H');
		retiraElemento('O');
		retiraElemento('H');
		printf("Produziu Molécula[%d] de H2O!\n\n", ++moleculas); fflush(stdout);
		sleep(1);
	}

	printf("\t--IMPOSSÍVEL criar mais Molécula de H2O... Voltando a produzir elementos--\n\n"); fflush(stdout);
	sleep(1);
}

/*
Quando os dois BUFFERS estão completos, o último sinal restante para a barreira liberar,
é da própria molécula, que fica somente esperando os outros dois sinais para começar a formação
das moléculas. 
*/
void * funMolecula(){
	while(TRUE){
		pthread_barrier_wait(&b_molecula);

		criaMolecula();

		sem_post(&s_oxigenio);
		sem_post(&s_hidrogenio);
	};
}

/*
main() simples, apenas com a declação tas THREADS e inicialização dos SEMAPHORE E A BARRIER
*/
int main(){
	pthread_t oxi_t, hid_t, h2o_t;						//Declaração

	sem_init(&s_oxigenio, 0, 0);						//Inicialização SEMAPHORE
	sem_init(&s_hidrogenio, 0, 0);						//Inicialização SEMAPHORE

	pthread_barrier_init(&b_molecula, NULL, 3);				//Inicialização BARRIER

	pthread_create(&oxi_t, NULL, funOxigenio, (void *)TEMPO_OXI);		//Criação das THREADS
	pthread_create(&hid_t, NULL, funHidrogenio, (void *)TEMPO_HIDRO);	//Criação das THREADS
	pthread_create(&h2o_t, NULL, funMolecula, NULL);			//Criação das THREADS

	pthread_join(oxi_t, NULL);										
	pthread_join(hid_t, NULL);
	pthread_join(h2o_t, NULL);

	pthread_barrier_destroy(&b_molecula);
	sem_destroy(&s_oxigenio);
	sem_destroy(&s_hidrogenio);

	return 0;
}
