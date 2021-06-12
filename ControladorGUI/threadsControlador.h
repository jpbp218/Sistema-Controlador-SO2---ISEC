#ifndef THREADSCONTROLADOR
#define THREADSCONTROLADOR

#include <windows.h>
#include <tchar.h>
#include "framework.h"
#include "ControladorGUI.h"
#include <strsafe.h>
#include <fcntl.h>
#include <io.h>
#include <stdio.h>
#include "../memoria.h"
#include "../registo.h"
#include "../passageiro.h"
#include"utils.h"

#define BUF_SIZE 2048
#define TAMANHO 200
#define TEMP_PING -30000000LL

#define TAMANHO_MENSAGEM 512



/* Thread do teclado */
typedef struct {
	PDATAPIPES pipes;
	int continua;
	Aeroporto aer;
	REGISTO_DADOS valoresMax; //Estrutura para os valores máximos de aviões e aeroportos
	Sinc* sinc; //Ponteiro para os mutexs
	MemDados dadosMem; //Estrutura com os handles da memória partilhada e sincronização
	Aviao* listaAvioes;
	int* nAviao;
	HANDLE evento;
	char* flagMostraA;
} THREADTEC;

/* Thread do consumidor */
typedef struct {
	PDATAPIPES pipes;
	HANDLE handles[2];
	//Estrutura dos semaforos e buffer
	MemDados* dados;
	Sinc* sinc;
	//Lista de aviões 
	Aviao* listaAvioes; // Array de aviões
	int nAviao;
	//Controlo do ciclo
	int control;
	char* flagMostraA;
	HWND * hWnd;
} THREADCONS, * PTHREADCONS;

DWORD WINAPI threadLeitura(LPVOID param);
DWORD WINAPI ThreadConsumidor(LPVOID param);
DWORD WINAPI PingAviao(LPVOID param);
DWORD WINAPI ThreadPassageiros(LPVOID param);

#endif /*THREADSCONTROLADOR*/



















