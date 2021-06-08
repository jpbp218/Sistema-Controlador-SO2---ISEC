#include "threadsControlador.h"


// ==================== Threads =======================

DWORD WINAPI ThreadConsumidor(LPVOID param) {
	Aviao aux;
	int i;
	TCHAR nomeEvento[20];
	HANDLE evento[2];
	Mensagens msgResposta;
	THREADCONS* dadosBuf = (THREADCONS*)param;

	while (!dadosBuf->control) {
		WaitForMultipleObjects(2, dadosBuf->handles, FALSE, INFINITE);
		CopyMemory(&aux, &dadosBuf->dados->BufCircular->buffer[dadosBuf->dados->BufCircular->out], sizeof(Aviao));
		if ((i = verificaAviao(dadosBuf->nAviao, dadosBuf->listaAvioes, aux.id)) == -1 && wcscmp(aux.msg, TEXT("embarcar")) != 0) { // Regista um novo avi�o que se conectou
			_ftprintf(stdout, TEXT("\n\nA registar novo avi�o! %d\n\nComando:"), aux.id);
			dadosBuf->listaAvioes[dadosBuf->nAviao++] = aux;
		}
		else if (wcscmp(aux.msg, TEXT("embarcar")) == 0) {

			for (int i = 0, auxPassageiros = 0; i < TOTAL_PASSAGEIROS && auxPassageiros < aux.cap_max; i++) {
				if (wcscmp(aux.partida.nome, dadosBuf->pipes->structClientes[i].aeroportoOrigem) == 0 &&
					wcscmp(aux.destino.nome, dadosBuf->pipes->structClientes[i].aeroportoDestino) == 0) {
					dadosBuf->pipes->structClientes[i].idAviao = aux.id;
					TCHAR msgTemp[200];
					_stprintf_s(msgTemp, sizeof(msgTemp) / sizeof(TCHAR), TEXT("Embarcou no avi�o %d com destino a %s"), aux.id, aux.destino.nome);
					comunicaPassageiro(dadosBuf->pipes->clientes[i], dadosBuf->pipes->structClientes[i].evento, msgTemp);
					auxPassageiros++;
				}
			}
		}
		else if (i >= 0) {  // Atualiza as coordenadas de um avi�o e verifica se s�o v�lidas ou n�o
			if (aux.flagChegada == 1 && *dadosBuf->flagMostraA == 1) {
				_ftprintf(stdout, TEXT("\n\nO avi�o %d chegou ao aeroporto de destino!\n\nComando:"), aux.id);
			}

			for (int i = 0; i < TOTAL_PASSAGEIROS && aux.flagChegada == 1; i++) {
				if (dadosBuf->pipes->structClientes[i].idAviao == aux.id) {
					TCHAR msgTemp[200];
					_stprintf_s(msgTemp, sizeof(msgTemp) / sizeof(TCHAR), TEXT("destino"));
					comunicaPassageiro(dadosBuf->pipes->clientes[i], dadosBuf->pipes->structClientes[i].evento, msgTemp);
					SetEvent(dadosBuf->pipes->structClientes[i].eventoTermina);
				}
			}

			_stprintf_s(nomeEvento, 19, TEXT("%d"), aux.id);
			evento[0] = OpenEvent(EVENT_ALL_ACCESS, TRUE, nomeEvento);
			if (evento[0] == NULL)
				dadosBuf->control = 1;
			evento[1] = dadosBuf->dados->mutexMensagens;

			// verifica se as coordenadas para onde o avi�o pertende ir s�o v�lidas
			if (validaCordsAviao(dadosBuf->dados, dadosBuf->listaAvioes, dadosBuf->nAviao, aux)) {
				_stprintf_s(msgResposta.mensagem, TAM_MENSAGEM, TEXT("OK")); // Responde ao avi�o que a posi��o pertendida est� dispon�vel
				dadosBuf->listaAvioes[i] = aux;

				for (int i = 0; i < TOTAL_PASSAGEIROS; i++) {
					if (dadosBuf->pipes->structClientes[i].idAviao == aux.id) {
						TCHAR msgTemp[200];
						_stprintf_s(msgTemp, sizeof(msgTemp) / sizeof(TCHAR), TEXT("Deslocou-se para (%d,%d)"), aux.pos.x, aux.pos.y);
						comunicaPassageiro(dadosBuf->pipes->clientes[i], dadosBuf->pipes->structClientes[i].evento, msgTemp);
					}
				}
				if (*dadosBuf->flagMostraA)
					_ftprintf(stdout, TEXT("\n[%04d] Deslocou-se para %d,%d\n"), aux.id, aux.pos.x, aux.pos.y);
			}
			else {
				_stprintf_s(msgResposta.mensagem, TAM_MENSAGEM, TEXT("NAO_PERMITIDO")); // Responde ao avi�o que a posi��o que se pertende deslocar est� ocupada e deve efetuar um desvio
				if (*dadosBuf->flagMostraA)
					_ftprintf(stdout, TEXT("\n[%04d] Obst�culo detetado, a efetuar desvio...\n"), aux.id);
			}


			WaitForMultipleObjects(2, evento, FALSE, INFINITE);
			CopyMemory(&dadosBuf->dados->BufMens->mensagem, &msgResposta, sizeof(Mensagens));
			ReleaseMutex(evento[1]);
			SetEvent(evento[0]);
			CloseHandle(evento[0]);
		}
		else {
			_ftprintf(stdout, TEXT("\nERRO"));
			dadosBuf->control = 1;
		}

		dadosBuf->dados->BufCircular->out = (dadosBuf->dados->BufCircular->out + 1) % TAM;
		ReleaseSemaphore(dadosBuf->dados->semAviao, 1, NULL);
	}
	return 0;
}

DWORD WINAPI PingAviao(LPVOID param) { // Tenta comunicar com todos os avi�es registrados de 3 em 3 segundos
	PTHREADCONS dados = (PTHREADCONS)param;
	TCHAR nomeEvento[20];
	HANDLE evento;

	while (!dados->control)
	{
		Sleep(TEMP_PING);
		for (int i = 0; i < dados->nAviao; i++)
		{
			_stprintf_s(nomeEvento, 19, TEXT("%d"), dados->listaAvioes[i].id);
			evento = OpenEvent(EVENT_ALL_ACCESS, TRUE, nomeEvento);
			if (evento == NULL)
			{
				for (int i = 0; i < TOTAL_PASSAGEIROS; i++) {
					if (dados->pipes->structClientes[i].idAviao == dados->listaAvioes[i].id) {
						SetEvent(dados->pipes->structClientes[i].eventoTermina);
					}
				}
				_ftprintf(stdout, TEXT("\n\nMayday Mayday, o avi�o %d deixou de efetuar contato!\n\n"), dados->listaAvioes[i].id);
				apagaAviao(i, &dados->nAviao, dados->listaAvioes);
				ReleaseSemaphore(dados->sinc->eventoAceitaAviao[1], 1, NULL);
			}
			CloseHandle(evento);
		}
	}
	return 0;
}

DWORD WINAPI threadLeitura(LPVOID param) {
	DATAPIPES Resposta;
	CLIENTE Pedido;
	DWORD cbBytesRead = 0, cbReplyBytes = 0;
	int numresp = 0;
	BOOL fSucess = FALSE;
	PDATAPIPES dadosPipe = (PDATAPIPES)param;
	HANDLE hPipe = dadosPipe->hPipe;
	OVERLAPPED OverRd = { 0 };
	HANDLE eventoEscrita;
	HANDLE eventos[2];
	DWORD aux;
	if (hPipe == NULL)
		return -1;

	eventoEscrita = CreateEvent(NULL, TRUE, FALSE, NULL);
	eventos[0] = CreateEvent(NULL, TRUE, FALSE, NULL); //Evento para overlaped
	eventos[1] = CreateEvent(NULL, TRUE, FALSE, NULL); //Handle inicial s� para nao meter um evento vazio

	if (eventos[0] == NULL && eventos[1] == NULL)
		return 1;
	adicionaClientes(dadosPipe, hPipe);

	while (1) {
		ZeroMemory(&OverRd, sizeof(OverRd));
		ResetEvent(eventos[0]);
		OverRd.hEvent = eventos[0];

		fSucess = ReadFile(hPipe, &Pedido, sizeof(CLIENTE), &cbBytesRead, &OverRd);

		aux = WaitForMultipleObjects(2, eventos, FALSE, INFINITE);

		if (aux == WAIT_OBJECT_0 + 1)
			break;
		else if (cbBytesRead < sizeof(CLIENTE) && aux == WAIT_OBJECT_0) {
			_ftprintf(stderr, TEXT("Erro ao ler dados\n"));
		}

		GetOverlappedResult(hPipe, &OverRd, &cbBytesRead, FALSE);
		if (wcscmp(Pedido.msg, TEXT("registar")) == 0) {
			if (existeAeroportoAsString(Pedido.aeroportoOrigem, dadosPipe->dados->BufAeroportos, dadosPipe->dados->BufCircular->nAeroportos) && existeAeroportoAsString(Pedido.aeroportoDestino, dadosPipe->dados->BufAeroportos, dadosPipe->dados->BufCircular->nAeroportos)) {
				char flag = 1;
				for (int i = 0; i < TOTAL_PASSAGEIROS && flag == 1; i++) {
					if (wcscmp((dadosPipe->structClientes + i)->nome, TEXT("")) == 0) {
						_tcscpy_s(dadosPipe->structClientes[i].nome, sizeof(dadosPipe->structClientes[i].nome) / sizeof(TCHAR), Pedido.nome);
						_tcscpy_s(dadosPipe->structClientes[i].aeroportoOrigem, sizeof(dadosPipe->structClientes[i].aeroportoOrigem) / sizeof(TCHAR), Pedido.aeroportoOrigem);
						_tcscpy_s(dadosPipe->structClientes[i].aeroportoDestino, sizeof(dadosPipe->structClientes[i].aeroportoDestino) / sizeof(TCHAR), Pedido.aeroportoDestino);
						dadosPipe->structClientes[i].evento = eventoEscrita;
						TCHAR nameEvento[50];
						_stprintf_s(nameEvento, sizeof(nameEvento) / sizeof(TCHAR), TEXT("SALTAREAD%d"), dadosPipe->clientes[i]);
						dadosPipe->structClientes[i].eventoTermina = eventos[1] = CreateEvent(NULL, TRUE, FALSE, nameEvento);
						flag = 0;
					}
				}
			}
			else {
				TCHAR msg[200];
				_stprintf_s(msg, sizeof(msg) / sizeof(TCHAR), TEXT("ap-invalid"));
				comunicaPassageiro(hPipe, eventoEscrita, msg);
				break;
			}
		}
		else if (wcscmp(Pedido.msg, TEXT("terminar")) == 0) {
			break;
		}
	}

	removeCliente(dadosPipe, hPipe);
	FlushFileBuffers(hPipe);
	DisconnectNamedPipe(hPipe);
	CloseHandle(hPipe);
	return 1;
}

DWORD WINAPI ThreadPassageiros(LPVOID param) {
	BOOL fConnected = FALSE;
	DWORD dwThreadID = 0;
	HANDLE hThread;
	HANDLE hPipeTemp;

	PDATAPIPES dadosPipe = (PDATAPIPES) param;

	dadosPipe->hPipe = INVALID_HANDLE_VALUE;

	if( (dadosPipe->WriteReady = CreateEvent(NULL, TRUE, FALSE, NULL)) == NULL){
		_ftprintf(stderr, TEXT("Erro a criar o evento\n"));
		return 1;
	}

	
	iniciaClientes(dadosPipe);
	
	while(!dadosPipe->terminar){
		hPipeTemp = CreateNamedPipe(PIPE_NAME,
			PIPE_ACCESS_DUPLEX | FILE_FLAG_OVERLAPPED,
			PIPE_TYPE_MESSAGE | PIPE_READMODE_MESSAGE | PIPE_WAIT,
			PIPE_UNLIMITED_INSTANCES,
			BUF_SIZE, BUF_SIZE,
			5000, NULL);

		if(hPipeTemp == INVALID_HANDLE_VALUE){
			_ftprintf(stderr, TEXT("Erro a abrir o pipe\n"));
			return -1; 
		}

		fConnected = ConnectNamedPipe(hPipeTemp, NULL) ? TRUE : (GetLastError() == ERROR_PIPE_CONNECTED);

		dadosPipe->hPipe = hPipeTemp;
		
		if (fConnected) {
			if ((hThread = CreateThread(NULL, 0, threadLeitura, dadosPipe, 0, dwThreadID)) == NULL) {
				_ftprintf(stderr, TEXT("Erro a criar thread! A terminar...\n"));
				return -1;
			}
			else{
				CloseHandle(hThread);
			}
		}
		else
			CloseHandle(dadosPipe->hPipe);
	}
}