#include <windows.h>
#include <tchar.h>
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
#define TEMP_PING 3000

#define TOTAL_PASSAGEIROS 10
#define TAMANHO_MENSAGEM 512


typedef struct {
	int terminar;
	HANDLE hPipe;
	HANDLE WriteReady;
	HANDLE clientes[TOTAL_PASSAGEIROS];
	CLIENTE structClientes[TOTAL_PASSAGEIROS];
	MemDados* dados;
} DATAPIPES, * PDATAPIPES;
//#define PIPE_NAME TEXT("\\\\.\\pipe\\passageiros")

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
} THREADCONS, * PTHREADCONS;


DWORD WINAPI ThreadTeclado(LPVOID param);
DWORD WINAPI threadLeitura(LPVOID param);
DWORD WINAPI ThreadConsumidor(LPVOID param);
DWORD WINAPI PingAviao(LPVOID param);

int enviaMsg(DATAPIPES dadosPipes);
void broadcastClientes(DATAPIPES dadosPipes);
void iniciaClientes(PDATAPIPES dadosPipes);
void removeCliente(DATAPIPES dadosPipes,HANDLE hPipe);
void adicionaClientes(PDATAPIPES dadosPipes, HANDLE hPipe);
int comunicaPassageiro(HANDLE hPipe, HANDLE evento, TCHAR msg[200]);
void listaPassageiros(PDATAPIPES dados);

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
		if ((i = verificaAviao(dadosBuf->nAviao, dadosBuf->listaAvioes, aux.id)) == -1 && wcscmp(aux.msg, TEXT("embarcar")) != 0) { // Regista um novo avião que se conectou
			_ftprintf(stdout, TEXT("\n\nA registar novo avião! %d\n\nComando:"), aux.id);
			dadosBuf->listaAvioes[dadosBuf->nAviao++] = aux;
		}
		else if (wcscmp(aux.msg, TEXT("embarcar")) == 0) {
			
			for(int i = 0, auxPassageiros = 0; i < TOTAL_PASSAGEIROS && auxPassageiros < aux.cap_max ;i++){
				if(wcscmp(aux.partida.nome, dadosBuf->pipes->structClientes[i].aeroportoOrigem) == 0 && 
					wcscmp(aux.destino.nome, dadosBuf->pipes->structClientes[i].aeroportoDestino) == 0) {
					dadosBuf->pipes->structClientes[i].idAviao = aux.id;
					dadosBuf->pipes->structClientes[i].flagViagem = 1;
					TCHAR msgTemp[200];
					_stprintf_s(msgTemp, sizeof(msgTemp) / sizeof(TCHAR), TEXT("Embarcou no avião %d com destino a %s"), aux.id, aux.destino.nome);
					comunicaPassageiro(dadosBuf->pipes->clientes[i], dadosBuf->pipes->structClientes[i].evento, msgTemp);
					auxPassageiros++;
				}
			}
		}	
		else if(i >= 0){  // Atualiza as coordenadas de um avião e verifica se são válidas ou não
			if (aux.flagChegada == 1 && *dadosBuf->flagMostraA == 1){
				_ftprintf(stdout, TEXT("\n\nO avião %d chegou ao aeroporto de destino!\n\nComando:"), aux.id);
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

			// verifica se as coordenadas para onde o avião pertende ir são válidas
			if (validaCordsAviao(dadosBuf->dados, dadosBuf->listaAvioes, dadosBuf->nAviao, aux)) {
				_stprintf_s(msgResposta.mensagem, TAM_MENSAGEM, TEXT("OK")); // Responde ao avião que a posição pertendida está disponível
				dadosBuf->listaAvioes[i] = aux;

				for(int i = 0; i < TOTAL_PASSAGEIROS;i++){
					if(dadosBuf->pipes->structClientes[i].idAviao == aux.id) {
						TCHAR msgTemp[200];
						_stprintf_s(msgTemp, sizeof(msgTemp) / sizeof(TCHAR), TEXT("Deslocou-se para (%d,%d)"), aux.pos.x, aux.pos.y);
						comunicaPassageiro(dadosBuf->pipes->clientes[i],dadosBuf->pipes->structClientes[i].evento, msgTemp);
					}
				}
			if (*dadosBuf->flagMostraA)
				_ftprintf(stdout, TEXT("\n[%04d] Deslocou-se para %d,%d\n"), aux.id, aux.pos.x, aux.pos.y);
			}
			else {
				_stprintf_s(msgResposta.mensagem, TAM_MENSAGEM, TEXT("NAO_PERMITIDO")); // Responde ao avião que a posição que se pertende deslocar está ocupada e deve efetuar um desvio
				if (*dadosBuf->flagMostraA)
					_ftprintf(stdout, TEXT("\n[%04d] Obstáculo detetado, a efetuar desvio...\n"), aux.id);
			}
				

			WaitForMultipleObjects(2, evento, FALSE, INFINITE);
			CopyMemory(&dadosBuf->dados->BufMens->mensagem, &msgResposta, sizeof(Mensagens));
			ReleaseMutex(evento[1]);
			SetEvent(evento[0]);
			CloseHandle(evento[0]);
		} else{
			_ftprintf(stdout, TEXT("\nERRO"));
			dadosBuf->control = 1;
		}

		dadosBuf->dados->BufCircular->out = (dadosBuf->dados->BufCircular->out + 1) % TAM;
		ReleaseSemaphore(dadosBuf->dados->semAviao, 1, NULL);
	}
	return 0;
}

DWORD WINAPI PingAviao (LPVOID param) { // Tenta comunicar com todos os aviões registrados de 3 em 3 segundos
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
				for(int i = 0; i < TOTAL_PASSAGEIROS;i++){
					if(dados->pipes->structClientes[i].idAviao == dados->listaAvioes[i].id){
						SetEvent(dados->pipes->structClientes[i].eventoTermina);
					}
				}
				_ftprintf(stdout, TEXT("\n\nMayday Mayday, o avião %d deixou de efetuar contato!\n\n"), dados->listaAvioes[i].id);
				apagaAviao(i, &dados->nAviao, dados->listaAvioes);
				ReleaseSemaphore(dados->sinc->eventoAceitaAviao[1], 1, NULL);
			}
			CloseHandle(evento);
		}
	}
	return 0;
}

int _tmain()
{
	DATAPIPES dadosPipe;
	dadosPipe.terminar = 0;
	HANDLE hthread[4];
	HANDLE hPipeTemp;
	int contThread = 0;
	THREADTEC estruturaThread;
	MemDados sem;
	Sinc sinc;
	estruturaThread.continua = 1;
	THREADCONS threadcons;
	char flagMostraA = 0;
	threadcons.flagMostraA = &flagMostraA;
	estruturaThread.flagMostraA = &flagMostraA;
	threadcons.nAviao = 0;
	estruturaThread.nAviao = &threadcons.nAviao;
	estruturaThread.pipes = &dadosPipe;
	threadcons.pipes = &dadosPipe;
	threadcons.sinc = &sinc;

	

	if ((estruturaThread.evento = threadcons.handles[0] = CreateEvent(NULL, TRUE, FALSE, EVENTO)) == NULL) {
		_ftprintf(stderr, TEXT("Erro ao criar evento!\n"));
		return -1;
	}

#ifdef UNICODE
	_setmode(_fileno(stdin), _O_WTEXT);
	_setmode(_fileno(stdout), _O_WTEXT);
	_setmode(_fileno(stderr), _O_WTEXT);
#endif

	_ftprintf(stdout, TEXT("Bem vindo ao sistema de controlador aério!\n\n"));

	// Obtem número máximo de aviões e aeroportos (Default - 5 Aviões e 3 Aeroportos) 
	if (!verificaChave(&estruturaThread.valoresMax)) {
		_ftprintf(stderr, TEXT("Erro a obter valores máximos de aeroportos e de aviões!\n"));
		return -1;
	}

	// Verifica se já existe um segundo processo a decorrer e cria memória partilhada
	if (abreFileMap(&estruturaThread.dadosMem))
	{
		_ftprintf(stderr, TEXT("Já existe um Controlador aberto! A terminar...\n"));
		return -1;
	}
	threadcons.dados = &estruturaThread.dadosMem;
	threadcons.listaAvioes = malloc(estruturaThread.valoresMax.numMaxAvioes * sizeof(Aviao));
	estruturaThread.listaAvioes = threadcons.listaAvioes;

	if (threadcons.listaAvioes == NULL)
	{
		_ftprintf(stderr, TEXT("Erro de memória de aviões\n"));
		return -1;
	}

	// Cria mecanismos de sincronização 
	if (!criaSinc(estruturaThread.valoresMax.numMaxAvioes, &sinc, &sem))
		return -1;

	estruturaThread.sinc = &sinc;
	threadcons.dados->semAviao = sem.semAviao;
	threadcons.dados->mutexMensagens = sem.mutexMensagens;
	threadcons.handles[1] = sem.semControl;

	if (!criaFileMap(&estruturaThread.dadosMem, estruturaThread.valoresMax)) // Criar FileMaps
		return -1;

	if (!criaMapViewOfFiles(&estruturaThread.dadosMem, estruturaThread.valoresMax, TRUE)) // Criar Vistas
		return -1;

	estruturaThread.dadosMem.BufCircular->nAeroportos = 0;
	threadcons.dados->BufCircular->in = 0;
	threadcons.dados->BufCircular->out = 0;
	threadcons.control = 0;
	dadosPipe.dados = &estruturaThread.dadosMem;



	if ((hthread[contThread++] = CreateThread(NULL, 0, ThreadConsumidor, &threadcons, 0, NULL)) == NULL) { //Thread responsável para receber todos os pedidos dos aviões
		_tprintf(_T("Erro a criar thread do buffer no controlador"));
		return -1;
	}

	if ((hthread[contThread++] = CreateThread(NULL, 0, PingAviao, &threadcons, 0, NULL)) == NULL) { //Thread para fazer ping nos aviões e verificar que os mesmos ainda estão ativos
		_tprintf(_T("Erro a criar thread de ping no controlador"));
		return -1;
	}

	_ftprintf(stdout, TEXT("Aviões: %d\nAeroportos: %d\n\n"), estruturaThread.valoresMax.numMaxAvioes, estruturaThread.valoresMax.numMaxAeroportos);

	if ((hthread[contThread++] = CreateThread(NULL, 0, ThreadTeclado, &estruturaThread, 0, NULL)) == NULL) // Lança Thread para ficar responsável pelo teclado
	{
		_ftprintf(stderr, TEXT("Erro a criar thread responsável pelo teclado! A terminar...\n"));
		return -1;
	}

	//////////////////////////////////////////////////////////////////

	BOOL fConnected = FALSE;
	DWORD dwThreadID = 0;
	dadosPipe.hPipe = INVALID_HANDLE_VALUE;

	if( (dadosPipe.WriteReady = CreateEvent(NULL, TRUE, FALSE, NULL)) == NULL){
		_ftprintf(stderr, TEXT("Erro a criar o evento\n"));
		return 1;
	}

	iniciaClientes(&dadosPipe);
	
	while(!dadosPipe.terminar){
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

		dadosPipe.hPipe = hPipeTemp;
		
		if (fConnected) {
			if ((hthread[contThread++] = CreateThread(NULL, 0, threadLeitura, &dadosPipe, 0, dwThreadID)) == NULL) {
				_ftprintf(stderr, TEXT("Erro a criar thread! A terminar...\n"));
				return -1;
			}
			else{
				CloseHandle(hthread[contThread - 1]);
				contThread--;
			}
		}
		else
			CloseHandle(dadosPipe.hPipe);
	}

	// Espera que a thread do teclado termine
	WaitForMultipleObjects(contThread, hthread, TRUE, INFINITE);
	// Unmap das vistas
	fechaViewFile(&estruturaThread.dadosMem);
	// Fecha os handles da extrutura MemDados
	fechaHandleMem(&estruturaThread.dadosMem);
	free(threadcons.listaAvioes);
	return 0;
}

DWORD WINAPI ThreadTeclado(LPVOID param) {  // Thread responsável por receber comandos do utilizador
	THREADTEC* dados = (THREADTEC*)param;
	TCHAR comando[TAMANHO];

	while (dados->continua)
	{
		_ftprintf(stdout, TEXT("Comando: "));
		_tscanf_s(TEXT("%s"), &comando, TAMANHO - 1);

		if (wcscmp(comando, TEXT("encerrar")) == 0) {
			dados->pipes->terminar = 1;
			DeleteFile(PIPE_NAME);
			broadcastClientes(*dados->pipes);
			encerraAvioes(&dados->dadosMem,dados->listaAvioes,*dados->nAviao);
			dados->dadosMem.BufCircular->nAeroportos = -1;					// Coloca o número de aviões negativos para que o aviao.c para que eventos bloqueados possam ter conhecimento que controlador já não existe
			dados->continua = 0;
			SetEvent(dados->evento);						// Evento para ultrapassar todos os waits em que possa estar bloqueado
			SetEvent(dados->sinc->eventoAceitaAviao[0]);	// Garante que todos os aviões suspensos possam avançar para o encerramento
			for(int i = 0; i < TOTAL_PASSAGEIROS;i++){
				if(dados->pipes->clientes[i] == NULL)
					SetEvent(dados->pipes->structClientes[i].eventoTermina);
			}
			_ftprintf(stdout, TEXT("\n\nA encerrar...\n\n"));
			break;
		} 
		else if (wcscmp(comando, TEXT("criar-aeroporto")) == 0) { 
			Aeroporto ap;
			int numAeroportos;
			numAeroportos = dados->dadosMem.BufCircular->nAeroportos; // Obtem número de aeroportos atuais 
			if (numAeroportos < dados->valoresMax.numMaxAeroportos) {
				do // Pede um nome válido
				{
					_ftprintf(stdout, TEXT("Indique o nome do aeroporto: "));
					_tscanf_s(TEXT("%s"), &ap.nome, 49);
				} while (existeNome(ap, dados->dadosMem.BufAeroportos, numAeroportos, dados->sinc));
				
				do // Pede coordenadas válidas
				{
					_ftprintf(stdout, TEXT("Indique a coordenada x: "));
					_tscanf_s(TEXT("%d"), &ap.pos.x); 

					_ftprintf(stdout, TEXT("Indique a coordenada y: ")); 
					_tscanf_s(TEXT("%d"), &ap.pos.y);
				} while (existeAeroportoPerto(ap, dados->dadosMem.BufAeroportos, numAeroportos) || !verificaCordsAp(ap));
				criaAeroporto(ap, dados->dadosMem.BufAeroportos, &dados->dadosMem.BufCircular->nAeroportos, dados->sinc);
				_ftprintf(stdout, TEXT("Aeroporto criado com sucesso\n"));
			}
			else
				_ftprintf(stdout, TEXT("O limite de aeroportos já foi alcançado!\n"));

		} else if (wcscmp(comando, TEXT("aeroportos")) == 0) { // Lista todos os aeroportos
			_ftprintf(stdout, TEXT("\n================== Aeroportos ==================\n"));
			listaAeroportos(&dados->dadosMem);
			_ftprintf(stdout, TEXT("================================================\n"));
		}
		else if (wcscmp(comando, TEXT("avioes")) == 0) { // Lista todos os aviões
			_ftprintf(stdout, TEXT("\n================== Aviões ==================\n"));
			for (int i = 0; i < *dados->nAviao; i++)
				_ftprintf(stdout, TEXT("Aviao -> ID: %d\n\tCapacidade: %d\n\tVelocidade: %d\n\tCoordenadas:(%d,%d)\n"), dados->listaAvioes[i].id, dados->listaAvioes[i].cap_max, dados->listaAvioes[i].velocidade, dados->listaAvioes[i].pos.x, dados->listaAvioes[i].pos.y);
			_ftprintf(stdout, TEXT("============================================\n"));
		}
		else if (wcscmp(comando, TEXT("suspender-avioes")) == 0) { // Suspende todos os aviões --> Ficam em lista de espera até que possa ser atendidos
			ResetEvent(dados->sinc->eventoAceitaAviao[0]);
			_ftprintf(stdout, TEXT("A entrada de novos aviões foi suspensa!\n"));
		}
		else if (wcscmp(comando, TEXT("ativar-avioes")) == 0) { // Ativa a entrada de novos aviões
			SetEvent(dados->sinc->eventoAceitaAviao[0]);
			_ftprintf(stdout, TEXT("A entrada de novos aviões foi ativada!\n"));
		}
		else if (wcscmp(comando, TEXT("ativar-notificacoes")) == 0) { // Mostra no controlador posições em tempo real de todos os aviões
			*dados->flagMostraA = 1;
			_ftprintf(stdout, TEXT("Notificações ativas!\n"));
		}
		else if (wcscmp(comando, TEXT("desativar-notificacoes")) == 0) {
			*dados->flagMostraA = 0;
			_ftprintf(stdout, TEXT("Notificações desativadas!\n"));
		}
		else if (wcscmp(comando, TEXT("passageiros")) == 0) {
			listaPassageiros(dados->pipes);
			_ftprintf(stdout, TEXT("\nComando não implementado\n"));
		}
		else {
			_ftprintf(stdout, TEXT("Comando não encontrado, utilize um dos seguintes comandos:\n"));
			_ftprintf(stdout, TEXT("\tcriar-aeroporto --> Criar um novo aeroporto\n"));
			_ftprintf(stdout, TEXT("\taeroportos --> Mostra todos os aeroportos registados\n"));
			_ftprintf(stdout, TEXT("\tavioes --> Mostra todos os aviões registados\n"));
			_ftprintf(stdout, TEXT("\tpassageiros --> Mostra todos os passageiros registados\n"));
			_ftprintf(stdout, TEXT("\tencerrar --> Encerra todo o sistema\n"));
			_ftprintf(stdout, TEXT("\tsuspender-avioes --> Suspende a entrada de novos aviões no sistema\n"));
			_ftprintf(stdout, TEXT("\tativar-avioes --> Ativa a entrada de novos aviões no sistema\n"));
			_ftprintf(stdout, TEXT("\tativar-notificacoes --> Ativa notificações sobre os aviões\n"));
			_ftprintf(stdout, TEXT("\tdesativar-notificacoes --> Desativa notificações sobre os aviões\n"));
		}
	}
	return 0;
}

void listaPassageiros(PDATAPIPES dados){
	for(int i = 0; i < TOTAL_PASSAGEIROS;i++){
		if(wcscmp( dados->structClientes[i].nome, TEXT("")) != 0){
			_ftprintf(stdout, TEXT("Nome: %s | Origem: %s | Destino: %s\n",dados->structClientes[i].nome,dados->structClientes[i].aeroportoOrigem,dados->structClientes[i].aeroportoDestino));
		}
	}
}

int comunicaPassageiro(HANDLE hPipe,HANDLE evento, TCHAR msg[200]){
	DWORD cbWritten = 0;
	BOOL fSucess = FALSE;
	OVERLAPPED OverlWR = { 0 };
	CLIENTE cliTemp; 
	_tcscpy_s(cliTemp.msg, sizeof(cliTemp.msg)/sizeof(TCHAR), msg);


	ResetEvent(evento);
	OverlWR.hEvent = evento;

	fSucess = WriteFile(hPipe, &cliTemp, sizeof(CLIENTE), &cbWritten, &OverlWR);

	WaitForSingleObject(evento,  INFINITE);

	GetOverlappedResult(hPipe,&OverlWR,&cbWritten,FALSE); //Sem wait

	if(cbWritten < sizeof(CLIENTE))
		_ftprintf(stderr, TEXT("Erro %d no writeFile\n"), GetLastError());

	return 1;
}

void broadcastClientes(DATAPIPES dadosPipes){
	TCHAR msg[200];
	_stprintf_s(msg, 199,TEXT("terminar"));
	for(int i = 0; i < TOTAL_PASSAGEIROS;i++){
		if (dadosPipes.clientes[i] != 0)
			comunicaPassageiro(dadosPipes.clientes[i],dadosPipes.structClientes[i].evento,msg);
	}
}

void iniciaClientes(PDATAPIPES dadosPipes){

	for (int i = 0; i < TOTAL_PASSAGEIROS; i++){
		dadosPipes->structClientes[i].flagViagem = 0;
		dadosPipes->clientes[i] = NULL;
		_tcscpy_s(dadosPipes->structClientes[i].nome, 49, TEXT(""));
	}
}

void adicionaClientes(PDATAPIPES dadosPipes,HANDLE hPipe){
	for(int i = 0 ; i < TOTAL_PASSAGEIROS; i++){
		if(dadosPipes->clientes[i] == NULL){
			dadosPipes->clientes[i] = hPipe;
			return;
		}
	}
}

void removeCliente(PDATAPIPES dadosPipes,HANDLE hPipe) {
	for(int i = 0; i < TOTAL_PASSAGEIROS; i++){
		if(dadosPipes->clientes[i] == hPipe ){
			dadosPipes->clientes[i] = NULL;
			_tcscpy_s(dadosPipes->structClientes[i].nome,sizeof(dadosPipes->structClientes[i].nome)/sizeof(TCHAR),TEXT(""));
			return;
		}
	}
}

DWORD WINAPI threadLeitura(LPVOID param){
	DATAPIPES Resposta;
	CLIENTE Pedido;
	DWORD cbBytesRead = 0, cbReplyBytes = 0;
	int numresp = 0;
	BOOL fSucess = FALSE;
	PDATAPIPES dadosPipe = (PDATAPIPES) param;
	HANDLE hPipe = dadosPipe->hPipe;
	OVERLAPPED OverRd = { 0 };
	HANDLE eventoEscrita;
	HANDLE eventos[2];
	DWORD aux;
	if (hPipe == NULL)
		return -1;

	eventoEscrita = CreateEvent(NULL, TRUE, FALSE, NULL);
	eventos[0] = CreateEvent(NULL, TRUE, FALSE, NULL); //Evento para overlaped
	eventos[1] = CreateEvent(NULL, TRUE, FALSE, NULL); //Handle inicial só para nao meter um evento vazio

	if (eventos[0] == NULL && eventos[1] == NULL)
		return 1;
	adicionaClientes(dadosPipe,hPipe);

	while(1){
		ZeroMemory(&OverRd, sizeof(OverRd));
		ResetEvent(eventos[0]);
		OverRd.hEvent = eventos[0];
		
		fSucess = ReadFile(hPipe, &Pedido, sizeof(CLIENTE), &cbBytesRead, &OverRd);

		aux = WaitForMultipleObjects(2, eventos, FALSE, INFINITE);

		if( aux == WAIT_OBJECT_0 + 1)
			break;
		else if(cbBytesRead < sizeof(CLIENTE) && aux == WAIT_OBJECT_0){
			_ftprintf(stderr, TEXT("Erro ao ler dados\n"));
		}
		
		GetOverlappedResult(hPipe,&OverRd,&cbBytesRead,FALSE); 
		if(wcscmp( Pedido.msg, TEXT("registar")) == 0){
			if(existeAeroportoAsString(Pedido.aeroportoOrigem,dadosPipe->dados->BufAeroportos,dadosPipe->dados->BufCircular->nAeroportos) && existeAeroportoAsString(Pedido.aeroportoDestino,dadosPipe->dados->BufAeroportos,dadosPipe->dados->BufCircular->nAeroportos)){
				char flag = 1;
				for(int i = 0; i < TOTAL_PASSAGEIROS && flag == 1; i++){
					if(wcscmp( (dadosPipe->structClientes + i)->nome, TEXT("")) == 0 ) {
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
			} else{
				TCHAR msg[200];
				_stprintf_s(msg, sizeof(msg) / sizeof(TCHAR),TEXT("ap-invalid"));
				comunicaPassageiro(hPipe,eventoEscrita,msg);
				break;
			}
		} else if(wcscmp( Pedido.msg, TEXT("terminar")) == 0){
			break;
		}
	}

	removeCliente(dadosPipe,hPipe);
	FlushFileBuffers(hPipe);
	DisconnectNamedPipe(hPipe);
	CloseHandle(hPipe);
	return 1;
}


