#ifndef UTILS_H
#define UTILS_H

#include <windows.h>
#include <tchar.h>
#include <strsafe.h>
#include <fcntl.h>
#include <io.h>
#include <math.h>
#include <stdio.h>
#include "../memoria.h"
#include "../passageiro.h"

#define TOTAL_PASSAGEIROS 10

typedef struct {
	int terminar;
	HANDLE hPipe;
	HANDLE WriteReady;
	HANDLE clientes[TOTAL_PASSAGEIROS];
	CLIENTE structClientes[TOTAL_PASSAGEIROS];
	MemDados* dados;
} DATAPIPES, * PDATAPIPES;

void broadcastClientes(DATAPIPES dadosPipes);
void iniciaClientes(PDATAPIPES dadosPipes);
void adicionaClientes(PDATAPIPES dadosPipes, HANDLE hPipe);
void removeCliente(PDATAPIPES dadosPipes, HANDLE hPipe);
void criaAeroporto(Aeroporto ap, Aeroporto* BufAeroportos, int* numAeroportos, Sinc* sinc);
BOOL existeAeroportoPerto(Aeroporto ap, Aeroporto* BufAeroportos, int numAeroportos);
BOOL existeNome(Aeroporto ap, Aeroporto* BufAeroportos, int numAeroportos);
void listaAeroportos(MemDados* mem);
BOOL verificaCordsAp(Aeroporto ap);
BOOL validaCordsAviao(MemDados* mem, Aviao avioes[], int nAviao, Aviao aviao);
BOOL isAeroporto(MemDados* mem, Coordenadas cords);
int verificaAviao(int nAviao, Aviao avioes[], DWORD id);
void apagaAviao(int pos, int * nAviao, Aviao avioes[]);
void encerraAvioes(MemDados* mem, Aviao avioes[], int nAviao);
BOOL aviaoChegou(Aviao aviao);
BOOL existeAeroportoAsString(TCHAR ap[], Aeroporto* BufAeroportos, int numAeroportos);
int comunicaPassageiro(HANDLE hPipe, HANDLE evento, TCHAR msg[200]);
HBITMAP LoadImagemDisco(TCHAR* nome);

#endif /*UTILS_H*/



















