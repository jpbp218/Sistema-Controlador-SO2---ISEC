#ifndef MEMORIA_H
#define MEMORIA_H

#include "aviao.h"
#include "registo.h"

#define BUFFERSIZE 255

#define FICH_MEM_P_A TEXT("memoriaPartilhadaA")
#define FICH_MEM_P_B TEXT("memoriaPartilhadaB")
#define FILE_MAP_MENSAGENS TEXT("memoriaPartilhadaM")

#define SEMAFORO_BUFFER TEXT("SEM_BUFFER_AVIAO")
#define SEMAFORO_MUTEX TEXT("SEM_MUTEX")
#define MUTEX TEXT("MUTEX")

#define EVENTO TEXT("EVENTO_ENCERRAR")
#define EVENTO_ENCERRA_AVIAO TEXT("EVENTO_ENCERRA_AVIAO")
#define SEMAFORO_AVIAO TEXT("SO2_SEMF_PROD")
#define SEMAFORO_CONTROLADOR TEXT("SO2_SEMF_CONS")
#define MUTEX_AVIAO TEXT("SO2_MUTEX_PROD")
#define MUTEX_MENSAGENS TEXT("SO2_MUTEX_MSG")

#define EVENTO_ENTRA_AVIAO TEXT("EVENTO_INICIA")
#define SEMAFORO_ENTRA_AVIAO TEXT("SEMAFORO_INICIA")

#define TAM 5
#define TAM_MENSAGEM 100

typedef struct {
	Aviao buffer[TAM];		// Buffer circular
	int nAeroportos;		// N�mero de aeroportos que existem --> Apenas para controlo na leitura e escrita de novos aeroportos	
	int in;					// Posi��o de escrita no buffer circular
	int out;				// Posi��o de leitura no buffer circular
} BufferCircular;

typedef struct { 
	TCHAR mensagem[TAM_MENSAGEM];	// Mensagem enviada do contrloador para o avi�o
} Mensagens;

typedef struct {
	// Buffer Circular
	LPHANDLE FileMapAviao;				// File Map para a estrutura do buffer circular
	BufferCircular* BufCircular;		// Vista para a estrutura do buffer circular
	
	// Semaforos
	HANDLE semAviao;					// Sem�foro para indicar que o avi�o pode escrever
	HANDLE semControl;					// Sem�foro para indicar que o controlador tem de ler
	HANDLE mutex;						// Mutex para garantir acesso �nico ao buffer circular
	HANDLE mutexMensagens;				// Mutex para garantir que uma mensagem � lida pelo respetivo avi�o antes de ser escrito algo por cima
	// Mem�ria aeroportos
	LPHANDLE FileMapAeroporto;			// File Map para o array de aeroportos
	Aeroporto* BufAeroportos;			// Vista para o array de aeroportos

	//Mensagens
	LPHANDLE FileMapMensagens;			// File Map para a estrutura de comunica��o entre o controlador e os avi�es
	Mensagens* BufMens;					// Vista para a estrutura de comunica��o entre o controlador e os avi�es
} MemDados;

typedef struct {
	HANDLE mutex;						// Mutex para garantir que n�o � escrito nenhum avi�o pelo controlador enquanto est� a ser lido pelo aviao
	HANDLE eventoAceitaAviao[2];		// Evento para aceitar ou recusar novos avi�es e sem�foro com o n�mero m�ximo de avi�es que podem ser atendidos pelo controlador
} Sinc;

BOOL abreFileMap(MemDados* dados);
BOOL criaFileMap(MemDados* dados, REGISTO_DADOS numMax);
BOOL fechaHandleMem(MemDados* dados);
BOOL fechaViewFile(MemDados* dados);
BOOL criaSinc(int nMaxAvioes, Sinc* dados,MemDados * sem);
BOOL criaMapViewOfFiles(MemDados* dados, REGISTO_DADOS numMax, BOOL isControl);

#endif /*MEMORIA_H*/
