#ifndef AVIAO_H
#define AVIAO_H

#include <tchar.h>

typedef struct {
	int x;
	int y;
} Coordenadas;

typedef struct {
	TCHAR nome[50];			// Nome do aeroporto
	Coordenadas pos;		// Posição do aeroporto
	int numAvioes;			// Quantidade atual de aviões
	int numPass;			// Quantidade atual de Passageiros
} Aeroporto;

typedef struct {
	DWORD id;				// Identificador único do avião --> PID
	int cap_max;			// Capacidade máxima de passageiros que o avião tem
	int velocidade;			// Velocidade do avião
	Coordenadas pos;		// Posição atual do avião
	Aeroporto partida;		// Aeroporto de onde o avião iniciou a sua viagem
	Aeroporto destino;		// Aeroporto de onde o avião se pertende deslocar
	TCHAR msg[50];			// Mensagem de controlo para comunicação com o controlador
	int numPassagBord;		// Número de passageiros a bordo
	int flagChegada;		// Flag de controlo para avisar o controlador que chegou ao seu destino
} Aviao;


#endif /*AVIAO_H*/
