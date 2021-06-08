#ifndef REGISTO_H
#define REGISTO_H

typedef struct {
	int numMaxAvioes;		// Número máximo de aviões que podem ser atendidos pelo controlador
	int numMaxAeroportos;	// Número máximo de aeroportos que podem ser criados no controlador
} REGISTO_DADOS;

int verificaChave(REGISTO_DADOS* pdados);

#endif /*REGISTO_H*/
