#include <windows.h>
#include <tchar.h>
#include <strsafe.h>
#include <fcntl.h>
#include <io.h>
#include <stdio.h>
#include "memoria.h"

BOOL abreFileMap(MemDados * dados) {
    dados->FileMapAviao = OpenFileMapping(FILE_MAP_READ | FILE_MAP_WRITE, TRUE, FICH_MEM_P_A);
    dados->FileMapAeroporto = OpenFileMapping(FILE_MAP_READ | FILE_MAP_WRITE, TRUE, FICH_MEM_P_B);
    dados->FileMapMensagens = OpenFileMapping(FILE_MAP_READ | FILE_MAP_WRITE, TRUE, FILE_MAP_MENSAGENS);
    if (dados->FileMapAviao == NULL || dados->FileMapAeroporto == NULL || dados->FileMapMensagens == NULL)
        return FALSE;
    return TRUE;
}

BOOL criaFileMap(MemDados* dados, REGISTO_DADOS numMax) {
    // =============================== Cria FileMap Aviões =============================== 
    dados->FileMapAviao = CreateFileMapping(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE, 0, sizeof(BufferCircular), FICH_MEM_P_A);
    if (dados->FileMapAviao == NULL) {
        _ftprintf(stderr, TEXT("Erro ao criar o memória partilhada!\n"));
        return FALSE;
    }

    // =============================== Cria FileMap Aeroportos =============================== 
    dados->FileMapAeroporto = CreateFileMapping(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE, 0, numMax.numMaxAeroportos * sizeof(Aeroporto), FICH_MEM_P_B);
    if (dados->FileMapAeroporto == NULL) {
        _ftprintf(stderr, TEXT("Erro ao criar o memória partilhada!\n"));
        return FALSE;
    }
	
	// =============================== Cria FileMap Mensagens =============================== 
	 dados->FileMapMensagens = CreateFileMapping(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE, 0,sizeof(Mensagens), FILE_MAP_MENSAGENS);
    if (dados->FileMapMensagens == NULL) {
        _ftprintf(stderr, TEXT("Erro ao criar o memória partilhada!\n"));
        return FALSE;
    }
    return TRUE;
}

BOOL fechaHandleMem(MemDados* dados) { 
    return CloseHandle(dados->FileMapAviao) && CloseHandle(dados->FileMapAeroporto) && CloseHandle(dados->FileMapMensagens);
}

BOOL fechaViewFile(MemDados* dados) { 
    return UnmapViewOfFile(dados->FileMapAviao) && UnmapViewOfFile(dados->FileMapAeroporto) && UnmapViewOfFile(dados->FileMapMensagens);
}

BOOL criaSinc(int nMaxAvioes, Sinc * dados, MemDados * sem) {
    dados->mutex = CreateMutex(NULL, FALSE, MUTEX_AVIAO);
    sem->mutexMensagens = CreateMutex(NULL, FALSE, MUTEX_MENSAGENS);
	sem->semAviao = CreateSemaphore(NULL, TAM, TAM, SEMAFORO_BUFFER);
	sem->semControl = CreateSemaphore(NULL, 0, 1, SEMAFORO_MUTEX);
	sem->mutex = CreateMutex(NULL, FALSE, MUTEX);

    dados->eventoAceitaAviao[0] = CreateEvent(
        NULL,
        TRUE,
        TRUE,
        EVENTO_ENTRA_AVIAO
    );

    dados->eventoAceitaAviao[1] = CreateSemaphore(NULL, nMaxAvioes, nMaxAvioes, SEMAFORO_ENTRA_AVIAO);

    if (sem->mutexMensagens == NULL ||dados->mutex == NULL || sem->semControl == NULL || sem->semAviao == NULL || sem->mutex == NULL || dados->eventoAceitaAviao == NULL) {
        _ftprintf(stderr, TEXT("Erro na criação dos mecanismos de sincronização.\n"));
        return FALSE;
    }
    return TRUE;
}

BOOL criaMapViewOfFiles(MemDados* dados, REGISTO_DADOS numMax, BOOL isControl) { // Flag isControl = true se for chamado no controlador

    if (dados->FileMapAviao == NULL || dados->FileMapAeroporto == NULL || dados->FileMapMensagens == NULL) {
        _ftprintf(stderr, TEXT("Erro com o FileMap!\n"));
        return FALSE;
    }

    // =============================== Colocar o ponteiro da estrutura a apontar para a vista dos aviões ===============================
    dados->BufCircular = (BufferCircular*)MapViewOfFile(dados->FileMapAviao, FILE_MAP_READ | FILE_MAP_WRITE, 0, 0, sizeof(BufferCircular));
    if (dados->BufCircular == NULL)
    {
        _ftprintf(stderr, TEXT("Erro ao criar vista para o FileMap!\n"));
        fechaHandleMem(dados);
        return FALSE;
    }

    // =============================== Colocar o ponteiro da estrutura a apontar para a vista dos aeroportos ===============================
    if (isControl) // Controlador tem acesso a escrita e leitura
        dados->BufAeroportos = (Aeroporto*)MapViewOfFile(dados->FileMapAeroporto, FILE_MAP_READ | FILE_MAP_WRITE, 0, 0, numMax.numMaxAeroportos * sizeof(Aeroporto));
    else // Aviões apenas tem acesso a leitura
        dados->BufAeroportos = (Aeroporto*)MapViewOfFile(dados->FileMapAeroporto, FILE_MAP_READ, 0, 0, numMax.numMaxAeroportos * sizeof(Aeroporto));

    if (dados->BufAeroportos == NULL)
    {
        _ftprintf(stderr, TEXT("Erro ao criar vista para o FileMap!\n"));
        fechaHandleMem(dados);
        return FALSE;
    }

	dados->BufMens = (Mensagens*)MapViewOfFile(dados->FileMapMensagens, FILE_MAP_READ | FILE_MAP_WRITE, 0, 0, sizeof(Mensagens));
    if (dados->BufMens == NULL)
    {
        _ftprintf(stderr, TEXT("Erro ao criar vista para o FileMap!\n"));
        fechaHandleMem(dados);
        return FALSE;
    }

    return TRUE;
}