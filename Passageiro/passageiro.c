#include <windows.h>
#include <tchar.h>
#include <strsafe.h>
#include <fcntl.h>
#include <stdlib.h>  
#include <io.h>
#include <stdio.h>
#include "../passageiro.h"

#define _SECOND 10000000

DWORD WINAPI ThreadComunicacao(LPVOID param);
DWORD WINAPI Temporizador(LPVOID param);

typedef struct {
	char* termina;
	HANDLE hPipe;
	HANDLE eventoTerminaTemporizador;
	HANDLE eventoTerminaPassageiro;
} TDATA, *PTDATA;

typedef struct {
	PTDATA threadDados;
	CLIENTE* cli;
	OVERLAPPED* OverlWr;
	HANDLE WriteReady;
} TEMP, *PTEMP;

int _tmain(int argc, LPTSTR argv[])
{
	char flagTermina = 0;
	CLIENTE cli;
	HANDLE hthread[2];
	int countThread = 0;
	TDATA threadInfo;
	HANDLE hPipe;
	BOOL fSucess = FALSE;
	DWORD cbWritten, dwMode;
	TCHAR buff[50];
	TEMP threadTemporizador;


#ifdef UNICODE
	_setmode(_fileno(stdin), _O_WTEXT);
	_setmode(_fileno(stdout), _O_WTEXT);
	_setmode(_fileno(stderr), _O_WTEXT);
#endif

	// =============================== Verifica se os argumentos estão corretos ===============================
	if (argc != 4 && argc != 5)
	{
		_ftprintf(stdout, TEXT("Argumentos inválidos! --> <aeroporto_origem> <aeroporto_destino> <nome> <[opt]tempo_espera_s>\n\n"));
		return -1;
	}

	if (argc == 4)
		cli.tempoEspera = INFINITE;
	else if ((cli.tempoEspera = _ttoi(argv[4])) <= 0)
	{
		_ftprintf(stdout, TEXT("Tempo de espera inválido!\n"));
		cli.tempoEspera = INFINITE;
	}

	wcscpy_s(cli.aeroportoOrigem, sizeof(cli.aeroportoOrigem)/ sizeof(TCHAR), argv[1]);
	wcscpy_s(cli.aeroportoDestino, sizeof(cli.aeroportoDestino)/sizeof(TCHAR), argv[2]);
	wcscpy_s(cli.nome, sizeof(cli.nome)/sizeof(TCHAR), argv[3]);

	//TCHAR nomeEvento[200];
	//_stprintf_s(nomeEvento, sizeof(nomeEvento) / sizeof(TCHAR), TEXT("EventoTermina%d"), GetProcessId());

	threadInfo.eventoTerminaTemporizador = CreateEvent(
		NULL,
		FALSE,
		FALSE,
		NULL
	);

	threadInfo.eventoTerminaPassageiro = CreateEvent(NULL,FALSE,FALSE,NULL);

	hPipe = CreateFile(
		PIPE_NAME,
		GENERIC_READ |
		GENERIC_WRITE,
		0 | FILE_SHARE_READ | FILE_SHARE_WRITE,
		NULL,
		OPEN_EXISTING,
		0 | FILE_FLAG_OVERLAPPED,
		NULL
	);

	if (hPipe == INVALID_HANDLE_VALUE)
	{
		_ftprintf(stdout, TEXT("Não existe um controlador ativo!\n"));
		return 3;
	}

	dwMode = PIPE_READMODE_MESSAGE;
	fSucess = SetNamedPipeHandleState(
		hPipe,
		&dwMode,
		NULL,
		NULL
	);

	if (!fSucess)
	{
		_ftprintf(stdout, TEXT("Ocorreu um erro no SetNamedPipeHandleState!\n"));
		return 3;
	}

	threadInfo.hPipe = hPipe;
	threadInfo.termina = &flagTermina;

	hthread[countThread] = CreateThread(
		NULL,
		0,
		ThreadComunicacao,
		&threadInfo,
		0,
		NULL
	);

	if (hthread[countThread] == NULL)
	{
		_ftprintf(stdout, TEXT("Ocorreu um erro na criação da thread de comunicação com o servidor!\n"));
		return 3;
	}

	countThread++;

	HANDLE WriteReady; // Handle para evento de leitura
	OVERLAPPED OverlWr = { 0 };

	WriteReady = CreateEvent(
		NULL,
		TRUE,
		FALSE,
		NULL
	);

	if (WriteReady == NULL)
	{
		_ftprintf(stdout, TEXT("Ocorreu um erro na do evento de leitura do Pipe!\n"));
		return 3;
	}

	// Efetuar registo no servidor
	ZeroMemory(&OverlWr, sizeof(OverlWr));
	ResetEvent(WriteReady);
	OverlWr.hEvent = WriteReady;

	_stprintf_s(cli.msg, 49, TEXT("registar\0"));

	fSucess = WriteFile(
		hPipe,
		&cli,
		sizeof(CLIENTE),
		&cbWritten,
		&OverlWr
	);

	WaitForSingleObject(WriteReady, INFINITE); // Espera que tenha enviado o pedido de registo
	GetOverlappedResult(hPipe, &OverlWr, &cbWritten, FALSE);
	if (cbWritten < sizeof(cli))
	{
		_ftprintf(stdout, TEXT("Erro ao efetuar registo! [%d]\n"), GetLastError());
	}

	_ftprintf(stdout, TEXT("Bem-vindo %s ao sistema de passageiros!\n\n"), cli.nome);

	if (cli.tempoEspera != INFINITE) // Thread responsável por contar o tempo que o cliente espera
	{
		threadTemporizador.cli = &cli;
		threadTemporizador.threadDados = &threadInfo;
		threadTemporizador.OverlWr = &OverlWr;
		threadTemporizador.WriteReady = WriteReady;
		hthread[countThread] = CreateThread(
			NULL,
			0,
			Temporizador,
			&threadTemporizador,
			0,
			NULL
		);

		if (hthread[countThread] == NULL)
			_ftprintf(stdout, TEXT("Ocorreu um erro na criação da thread da temporização, vai ficar eternamente à espera de um avião!\n"));
		else
			countThread++;
	}


	while (!flagTermina)
	{
		_tscanf_s(TEXT("%s"), &buff, 49);
		if (flagTermina)
			break;
		if (wcscmp(buff, TEXT("terminar")) == 0) {
			SetEvent(threadInfo.eventoTerminaPassageiro);
			_stprintf_s(cli.msg, 49, TEXT("terminar"));
			ZeroMemory(&OverlWr, sizeof(OverlWr));
			ResetEvent(WriteReady);
			OverlWr.hEvent = WriteReady;

			fSucess = WriteFile(
				hPipe,
				&cli,
				sizeof(cli),
				&cbWritten,
				&OverlWr
			);

			WaitForSingleObject(WriteReady, INFINITE); // Espera que tenha enviado o pedido de registo
			GetOverlappedResult(hPipe, &OverlWr, &cbWritten, FALSE);
			if (cbWritten < sizeof(cli))
			{
				_ftprintf(stdout, TEXT("Erro ao mandar pedido de terminar! [%d]\n"), GetLastError());
			}

			break;
		}
		else
			_ftprintf(stdout, TEXT("Escreva \"terminar\" para encerrar o programa!\n"));
	}
	flagTermina = TRUE;
	WaitForMultipleObjects(countThread, hthread, TRUE, INFINITE);
	CloseHandle(WriteReady);
	CloseHandle(hPipe);
	return 0;
}

DWORD WINAPI ThreadComunicacao(LPVOID param) {
	PTDATA dados = (PTDATA)param;
	CLIENTE FromControl;

	DWORD cbBytesRead = 0;
	BOOL fSuccess = FALSE;

	HANDLE ReadReady;
	OVERLAPPED OverlRd = { 0 };

	HANDLE arrayHandles[2];

	if (dados->hPipe == NULL)
	{
		_ftprintf(stdout, TEXT("Erro ao receber o Pipe na thread\n"));
		exit(3);
	}

	ReadReady = CreateEvent(
		NULL,	
		TRUE,	
		FALSE,	
		NULL);

	if (ReadReady == NULL)
	{
		_ftprintf(stdout, TEXT("Erro no evento do read na thread de comunicação.\n"));
		exit(3);
	}

	arrayHandles[0] = ReadReady;
	arrayHandles[1] = dados->eventoTerminaPassageiro;

	while (!*dados->termina)
	{
		ZeroMemory(&OverlRd, sizeof(OverlRd));
		OverlRd.hEvent = ReadReady;
		ResetEvent(ReadReady);

		fSuccess = ReadFile(
			dados->hPipe,
			&FromControl,
			sizeof(CLIENTE),
			&cbBytesRead,
			&OverlRd
		);


		if ((WaitForMultipleObjects(2, arrayHandles, FALSE, INFINITE) - WAIT_OBJECT_0) == 1)
			break;

		GetOverlappedResult(dados->hPipe, &OverlRd, &cbBytesRead, FALSE);
		if (cbBytesRead < sizeof(CLIENTE))
			_tprintf(TEXT("ReadFile falhou. Erro %d\n"), GetLastError());
		
		if (wcscmp(FromControl.msg, TEXT("terminar")) == 0) {
			_tprintf(TEXT("O servidor foi encerrado!\n\nEscreva \"fim\" para terminar...\n\n"));
			*dados->termina = 1;
			return 1;
		} else if (wcscmp(FromControl.msg, TEXT("ap-invalid")) == 0) {
			_tprintf(TEXT("Os aeroportos introduzidos são inválidos!\n\nEscreva \"fim\" para terminar...\n\n"));
			*dados->termina = 1;
			return 1;
		} else if (wcscmp(FromControl.msg, TEXT("destino")) == 0) {
			_tprintf(TEXT("Chegou ao seu destino!\n\nEscreva \"fim\" para terminar...\n\n"));
			*dados->termina = 1;
			return 1;
		}
		else if (wcsncmp(FromControl.msg, TEXT("Embarcou"), 8) == 0)
		{
			SetEvent(dados->eventoTerminaTemporizador);
			_tprintf(FromControl.msg);
			_tprintf(TEXT("\n"));
		}
		else {
			_tprintf(FromControl.msg);
			_tprintf(TEXT("\n"));
		}
	}
	return 1;
}

DWORD WINAPI Temporizador(LPVOID param) {
	PTEMP dados = (PTEMP)param;
	HANDLE eventos[2];
	BOOL fSucess = FALSE;
	DWORD cbWritten;
	__int64 qwDueTime;
	LARGE_INTEGER liDueTime;

	qwDueTime = -(dados->cli->tempoEspera) * _SECOND;
	liDueTime.LowPart = (DWORD)(qwDueTime & 0xFFFFFFFF);
	liDueTime.HighPart = (LONG)(qwDueTime >> 32);

	eventos[0] = dados->threadDados->eventoTerminaTemporizador;
	eventos[1] = CreateWaitableTimer(NULL, TRUE, NULL);

	if (eventos[1] == NULL)
	{
		_ftprintf(stdout, TEXT("CreateWaitableTimer falhou (%d)\n"), GetLastError());
		return 1;
	}

	if (!SetWaitableTimer(eventos[1], &liDueTime, 0, NULL, NULL, 0))
	{
		_ftprintf(stdout, TEXT("SetWaitableTimer falhou (%d)\n"), GetLastError());
		return 2;
	}

	if ((WaitForMultipleObjects(2, eventos, FALSE, INFINITE) - WAIT_OBJECT_0) == 0) {
		CloseHandle(eventos[1]);
		return 3;
	}
		

	*dados->threadDados->termina = 1;
	SetEvent(dados->threadDados->eventoTerminaPassageiro);

	_stprintf_s(dados->cli->msg, sizeof(dados->cli->msg) / sizeof(TCHAR), TEXT("terminar"));
	ZeroMemory(dados->OverlWr, sizeof(*dados->OverlWr));
	ResetEvent(dados->WriteReady);
	dados->OverlWr->hEvent = dados->WriteReady;

	fSucess = WriteFile(
		dados->threadDados->hPipe,
		dados->cli,
		sizeof(CLIENTE),
		&cbWritten,
		dados->OverlWr
	);

	WaitForSingleObject(dados->WriteReady, INFINITE); // Espera que tenha enviado o pedido de registo
	GetOverlappedResult(dados->threadDados->hPipe, dados->OverlWr, &cbWritten, FALSE);
	if (cbWritten < sizeof(CLIENTE))
	{
		_ftprintf(stdout, TEXT("Erro ao mandar pedido de terminar! [%d]\n"), GetLastError());
	}

	_ftprintf(stdout, TEXT("O tempo de espera que foi definido acabou, a encerrar...\n\nEscreva \"fim\" para sair\n"));


	return 0;
}

