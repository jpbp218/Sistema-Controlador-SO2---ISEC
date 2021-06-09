#ifndef PASSAGEIRO_H
#define PASSAGEIRO_H

#include <tchar.h>

#define PIPE_NAME TEXT("\\\\.\\pipe\\passageiros")
#define TAM_PASS 50

typedef struct {
	TCHAR aeroportoOrigem[TAM_PASS];
	TCHAR aeroportoDestino[TAM_PASS];
	TCHAR nome[TAM_PASS];
	TCHAR msg[200];
	DWORD idAviao;
	int tempoEspera;
	HANDLE evento;
	HANDLE eventoTermina;
	int flagViagem;
}CLIENTE, * PCLIENTE;


#endif /*PASSAGEIRO_H*/

