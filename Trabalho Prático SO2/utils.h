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

#endif /*UTILS_H*/



















