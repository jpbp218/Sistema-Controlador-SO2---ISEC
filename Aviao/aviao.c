#include <windows.h>
#include <tchar.h>
#include <strsafe.h>
#include <fcntl.h>
#include <stdlib.h>  
#include <io.h>
#include <stdio.h>
#include <math.h>
#include "../aviao.h"
#include "../memoria.h"
#include "../registo.h"

#define BUF_SIZE 255
#define TAMANHO 200

float calculaDistancia(int x1, int y1, int x2, int y2);
BOOL obtemAeroporto(TCHAR nome[], Sinc sinc, MemDados* dados, Aeroporto* ap);
DWORD WINAPI RecebeMensagens(LPVOID param);

typedef struct {
	HANDLE eventoContinua;
	HANDLE evento;
	HANDLE eventoEncerraAviao;
	Mensagens msg;
	int continua;
	MemDados* dadosMem;
	int* continuaThreadTeclado;
} TLER, * PTLER;

typedef struct {
	//Aviao* bufferCircular;	// Ponteiro para o buffer circular
	Aviao* thisAviao;
	Sinc sinc;
	char emViagem;			// Flag
	MemDados* dadosMem;
	HANDLE eventoEncerraAviao;
	int control;
	PTLER lerMensagem;
} TDATA, *PTDATA;

DWORD WINAPI ThreadViagem(LPVOID param);
BOOL verificaCordsAv(Aviao ap);
int _tmain(int argc, LPTSTR argv[])
{
	HANDLE eventSem[2];
	Aviao dados;
	TCHAR AeroportoInicial[TAMANHO], comando[TAMANHO];
	MemDados memDados;
	HANDLE threadViagem = NULL, hthread, threadMensagens;
	TDATA dadosThread;
	MemDados sem;
	Sinc sinc;
	REGISTO_DADOS valoresMax; // Extrutura com os valores máximo dos aviões e dos aeroportos que podem ser criados
	TLER threadLeMsgData;
	dadosThread.lerMensagem = &threadLeMsgData;

	srand( (unsigned)time( NULL ) );  
	dados.flagChegada = 0;
	if((eventSem[0] = threadLeMsgData.evento = CreateEvent(NULL, TRUE, FALSE,EVENTO)) == NULL){
		_ftprintf(stderr, TEXT("Erro ao criar evento!\n"));
		return -1;
	}

#ifdef UNICODE
	_setmode(_fileno(stdin), _O_WTEXT);
	_setmode(_fileno(stdout), _O_WTEXT);
	_setmode(_fileno(stderr), _O_WTEXT);
#endif

	// Verifica se a memória partilhada já está criada
	if (!abreFileMap(&memDados))
	{
		_ftprintf(stderr, TEXT("Não existe um controlador ativo! Tente mais tarde...\n"));
		return -1;
	}

	// Obtem número máximo de aviões e aeroportos (Default - 5 Aviões e 3 Aeroportos) 
	if (!verificaChave(&valoresMax)) {
		_ftprintf(stderr, TEXT("Erro a obter valores máximos de aeroportos e de aviões!\n"));
		return -1;
	}

	if (!criaMapViewOfFiles(&memDados, valoresMax, FALSE))
		return -1;

	dadosThread.dadosMem = &memDados;
	threadLeMsgData.dadosMem = &memDados;
	// Cria mecanismos de sincronização
	if (!criaSinc(valoresMax.numMaxAvioes, &sinc, &sem))
		return -1;
	dadosThread.dadosMem->semAviao = sem.semAviao;
	dadosThread.dadosMem->semControl = sem.semControl;
	dadosThread.dadosMem->mutex = sem.mutex;

	// =============================== Verifica se os argumentos estão corretos ===============================
	if (argc != 4)
	{
		_ftprintf(stdout, TEXT("Argumentos inválidos! --> <lotação> <velocidade> <aeroporto_inicial>\n\n"));
		return -1;
	}

	if ((dados.cap_max = _ttoi(argv[1])) <= 0) 
	{
		_ftprintf(stdout, TEXT("Número de lotação inválido!\n"));
		return -1;
	}

	if ((dados.velocidade = _ttoi(argv[2])) <= 0)
	{
		_ftprintf(stdout, TEXT("Velocidade inválida!\n"));
		return -1;
	}
	
	TCHAR nomeEventoEncerraAviao[100];
	_stprintf_s(nomeEventoEncerraAviao, 99, TEXT("%s%d"), EVENTO_ENCERRA_AVIAO,GetCurrentProcessId());

	dados.id = GetCurrentProcessId(); // ID gerado a cada execução --> PID

	if((threadLeMsgData.eventoEncerraAviao = CreateEvent(NULL, TRUE, FALSE,nomeEventoEncerraAviao)) == NULL){
		_ftprintf(stderr, TEXT("Erro ao criar evento!\n"));
		return -1;
	}
	dadosThread.eventoEncerraAviao = threadLeMsgData.eventoContinua;
	//Aviao da estrutura
	dadosThread.dadosMem->BufCircular->in = 0;
	dadosThread.dadosMem->BufCircular->out = 0;

	dadosThread.emViagem = 0;
	dadosThread.sinc = sinc;
	dadosThread.thisAviao = &dados;
	wcscpy_s(dados.partida.nome,49, argv[3]);

	_ftprintf(stdout, TEXT("[%04d] A aguardar que se possa conectar...\n\n"), dados.id);

	WaitForMultipleObjects(2, sinc.eventoAceitaAviao, TRUE, INFINITE); // Espera até ter autorização ou até haver vaga para entrar

	if (memDados.BufCircular->nAeroportos == -1) // Verifica se o controlador ainda está ativo, caso contrário é encerrado
	{
		_ftprintf(stdout, TEXT("O controlador já não está ativo, a terminar..."));
		return 0;
	}

	while (!obtemAeroporto(dados.partida.nome, sinc, &memDados, &dados.partida))
	{
		_ftprintf(stdout, TEXT("O Aeroporto não é válido, introduza um novo nome ou escreva \"sair\" para terminar...\nNome Aeroporto >>> "));
		_tscanf_s(TEXT("%s"), &dados.partida.nome, 49);
		if (wcscmp(dados.partida.nome, TEXT("sair")) == 0)
			return -1;
	}
	dados.destino = dados.partida; // Apenas para não ficar o aeroporto com lixo
	dados.pos = dados.partida.pos; // Atualiza posição atual para a posição do aeroporto de partida
	
	// ==== Registo do Avião ====
	//Colocar o semaforo da exclusão mútua em espera
	WaitForSingleObject(dadosThread.dadosMem->semAviao,INFINITE);
	WaitForSingleObject(dadosThread.dadosMem->mutex, INFINITE);
	//Semaforo assinalado escreve no buffer 
	CopyMemory(&dadosThread.dadosMem->BufCircular->buffer[dadosThread.dadosMem->BufCircular->in], &dados, sizeof(Aviao));
	//Incrementa a posição da head do buffer
	dadosThread.dadosMem->BufCircular->in = (dadosThread.dadosMem->BufCircular->in + 1) % TAM;
	//Assinala o semaforo da exclusão mútua
	ReleaseMutex(dadosThread.dadosMem->mutex);
	//Assinala o semaforo para o consumidor conseguir ler 
	ReleaseSemaphore(dadosThread.dadosMem->semControl,1,NULL);

	// ============================================== Início ===================================================

	_ftprintf(stdout, TEXT("Lotação: %d\nVelocidade: %d\nAeroporto: %s\n\n"), dados.cap_max, dados.velocidade, dados.partida.nome);

	_ftprintf(stdout, TEXT("Bem vindo ao sistema do avião! O seu identificador é %d\n\n"), dados.id);

	TCHAR nomeEvento[30];
	_stprintf_s(nomeEvento, 29, TEXT("continua%d"), GetCurrentProcessId());

	// Evento de sincronizañçao de threads
	threadLeMsgData.eventoContinua = CreateEvent(
		NULL,
		FALSE,
		FALSE,
		nomeEvento
	);

	if (threadLeMsgData.eventoContinua == NULL)
	{
		_ftprintf(stderr, TEXT("Erro ao criar mecanismo de leitura de mensagens!"));
		return -1;
	}

	threadLeMsgData.continuaThreadTeclado = &threadLeMsgData.continua;
	threadMensagens = CreateThread(NULL, 0, RecebeMensagens, &threadLeMsgData, 0, NULL); // Cria a thread responsável por ler mensagens enviadas pelo controlador

	// Prepara-se para receber comandos do utilizador
	while (threadLeMsgData.continua)
	{

		_ftprintf(stdout, TEXT("\n[%04d] Comando: "), dados.id);
		_tscanf_s(TEXT("%s"), &comando, TAMANHO - 1);

		if (threadLeMsgData.continua) // Apenas para evitar comandos após o termino da aplicação
		{
			if (wcscmp(comando, TEXT("sair")) == 0) {
				_ftprintf(stdout, TEXT("A terminar...\n"));
				break;
			}
			else if (dadosThread.emViagem == 1)
				_ftprintf(stdout, TEXT("[%04d] Comando não autorizado durante a viagem! Apenas pode utilizar o comando \"sair\"...\n"), dados.id);
			else if (wcscmp(comando, TEXT("destino")) == 0) {
				Aeroporto ap;
				do
				{
					_ftprintf(stdout, TEXT("Indique o nome do aeroporto: "));
					_tscanf_s(TEXT("%s"), &ap.nome, 49);
				} while (!obtemAeroporto(ap.nome, sinc, &memDados, &dados.destino));
				_ftprintf(stdout, TEXT("\nDestino alterado com sucesso!\n"));
			}
			else if (wcscmp(comando, TEXT("iniciar")) == 0) { // Iniciar uma viagem
				dadosThread.emViagem = 1; // Quando terminar a viagem, muda a flag para 0

				//Enviar informação para que todos os passageiros embarquem
				_tcscpy_s(dados.msg, sizeof(dados.msg) / sizeof(TCHAR), TEXT("embarcar"));
				WaitForSingleObject(dadosThread.dadosMem->semAviao, INFINITE);
				WaitForSingleObject(dadosThread.dadosMem->mutex, INFINITE);
				CopyMemory(&dadosThread.dadosMem->BufCircular->buffer[dadosThread.dadosMem->BufCircular->in], &dados, sizeof(Aviao));
				dadosThread.dadosMem->BufCircular->in = (dadosThread.dadosMem->BufCircular->in + 1) % TAM;
				ReleaseMutex(dadosThread.dadosMem->mutex);
				ReleaseSemaphore(dadosThread.dadosMem->semControl, 1, NULL);
				_tcscpy_s(dados.msg, sizeof(dados.msg) / sizeof(TCHAR), TEXT(""));

				// Iniciar thread responsável por efetuar a viagem
				threadViagem = CreateThread(NULL, 0, ThreadViagem, &dadosThread, 0, NULL); // Cria a thread responsável por efetuar a viagem
				_ftprintf(stdout, TEXT("\nA iniciar viagem...\n"));
			}
			else
			{
				_ftprintf(stdout, TEXT("Comando não encontrado, utilize um dos seguintes comandos:\n"));
				_ftprintf(stdout, TEXT("\tdestino --> Escolhe um aeroporto para ser o próximo destino\n"));
				_ftprintf(stdout, TEXT("\tembarcar --> Embarcam todos os passageiros que estejam no aeroporto com o mesmo destino que o avião\n"));
				_ftprintf(stdout, TEXT("\tiniciar --> Iniciar viagem até ao aeroporto definido previamente\n"));
				_ftprintf(stdout, TEXT("\tsair --> Sair do sistema\n"));
			}
		}
	}


	//Se não estiver em viagem encerra tudo bem
	//Se estiver muda a flag para terminar a thread da viagem
	//Espera que a thread termine
	//E ativa o evento de receber msg para terminar tambem

	if (dadosThread.emViagem == 1){
		dadosThread.emViagem = 0;
		WaitForSingleObject(threadViagem, INFINITE);
	}		
	threadLeMsgData.continua = 0;
	SetEvent(threadLeMsgData.eventoEncerraAviao);
	WaitForSingleObject(threadMensagens, INFINITE);
	CloseHandle(threadLeMsgData.eventoContinua);
	fechaViewFile(dadosThread.dadosMem);
	return 0;
} // todo Run-Time Check Failure #2 - Stack around the variable 'dados' was corrupted.

DWORD WINAPI ThreadViagem(LPVOID param) {
	PTDATA dados = (PTDATA)param;
	HINSTANCE LibDll = NULL;
	TCHAR DLL[TAMANHO];
	_stprintf_s(DLL, TAMANHO, TEXT("SO2_TP_DLL_2021.dll"));
	Aviao aux;
	aux = *dados->thisAviao;
	aux.flagChegada = 0;
	// Efetua calculos da velocidade
	float distancia = calculaDistancia(dados->thisAviao->pos.x, dados->thisAviao->pos.y, dados->thisAviao->destino.pos.x , dados->thisAviao->destino.pos.y);
	
	if (distancia <= 0) {
		_ftprintf(stdout, TEXT("O avião chegou ao seu destino (%d , %d)\n"), dados->thisAviao->pos.x, dados->thisAviao->pos.y);
		dados->emViagem = 0;
		return 0;
	}

	float tempoTotal = distancia / dados->thisAviao->velocidade;
	float tempoEspera = (tempoTotal / distancia) * 1000; // Tempo para o sleep convertido em ms
	
	// Função da DLL
	int(*move)(int) = NULL;
	// Carrega a DLL
	LibDll = LoadLibrary(DLL);

	int next_x = 0, next_y = 0, res;

	if (LibDll != NULL) // DLL carregada com sucesso
	{
		move = (int(*)(int)) GetProcAddress(LibDll, "move");
		if (move != NULL)
		{
			do
			{
				Sleep(tempoEspera);
				res = move(dados->thisAviao->pos.x, dados->thisAviao->pos.y, dados->thisAviao->destino.pos.x, dados->thisAviao->destino.pos.y, &next_x, &next_y);
				aux.pos.x = next_x;
				aux.pos.y = next_y;

				if (!res)
					aux.flagChegada = 1;

				// ==== Registo das novas coordenadas ====
				//Colocar o semaforo da exclusão mútua em espera
				WaitForSingleObject(dados->dadosMem->semAviao, INFINITE);
				WaitForSingleObject(dados->dadosMem->mutex, INFINITE);
				//Semaforo assinalado escreve no buffer 
				CopyMemory(&dados->dadosMem->BufCircular->buffer[dados->dadosMem->BufCircular->in], &aux, sizeof(Aviao));
				//Incrementa a posição da head do buffer
				dados->dadosMem->BufCircular->in = (dados->dadosMem->BufCircular->in + 1) % TAM;
				//Assinala o semaforo da exclusão mútua
				ReleaseMutex(dados->dadosMem->mutex);
				//Assinala o semaforo para o consumidor conseguir ler 
				ReleaseSemaphore(dados->dadosMem->semControl, 1, NULL);


				switch (res)
				{
				case 0:
					_ftprintf(stdout, TEXT("Chegou ao seu destino (%d , %d)\n"), next_x, next_y);
					dados->thisAviao->pos.x = next_x;
					dados->thisAviao->pos.y = next_y;
					dados->thisAviao->partida = dados->thisAviao->destino;
					dados->emViagem = 0;
					break;
				case 1:
					WaitForSingleObject(dados->lerMensagem->eventoContinua, INFINITE); // Aguarda que a resposta tenha sido lida pela thread responsável por ler todos as mensagens
					if (wcscmp(dados->lerMensagem->msg.mensagem, TEXT("OK")) == 0) // Se puder avançar avança, se não fica parado
					{
						_ftprintf(stdout, TEXT("A deslocar para (%d , %d)\n"), next_x, next_y);
						dados->thisAviao->pos.x = next_x;
						dados->thisAviao->pos.y = next_y;
					}
					else if (wcscmp(dados->lerMensagem->msg.mensagem, TEXT("NAO_PERMITIDO")) == 0) { // Posição ocupada, aviao deve efetuar um desvio
						int cont = 0;
						do{ // Desvia-se para umas coordenadas próximas de forma aleatória
							dados->thisAviao->pos.x = next_x + (rand() % 2);
							dados->thisAviao->pos.y = next_y + (rand() % 2);
						} while (!verificaCordsAv(*dados->thisAviao) && cont++ < 5);	// Verifica que está dentro dos limites pré definidos		
						_ftprintf(stdout, TEXT("A efetuar desvio (%d , %d)!\n"), next_x, next_y);
					} else{
						dados->emViagem = 0;
					}
					break;
				default:
					_ftprintf(stdout, TEXT("Ocorreu um erro a calcular a rota!\n"));
					break;
				}	
			} while (res != 0 && dados->emViagem == 1);
		}
		else
			_ftprintf(stdout, TEXT("Erro ao encontrar função %s\n"), DLL);
	}
	else
	{
		_ftprintf(stderr, TEXT("Erro %d ao carregar DLL! A cancelar viagem...\n"), GetLastError());
		return -1;
	}
	FreeLibrary(LibDll); // Liberta a DLL
	dados->emViagem = 0;
	return 0;
}

float calculaDistancia(int x1, int y1, int x2, int y2) {
	return sqrt((x1 - x2) * (x1 - x2) + (y1 - y2) * (y1 - y2));
}

BOOL obtemAeroporto(TCHAR nome[], Sinc sinc, MemDados * dados, Aeroporto * ap) { // Verifica se o aeroporto existe e devolve uma estrutura do mesmo
	Aeroporto aux[TAM];
	int numAeroportos = 0;

	WaitForSingleObject(sinc.mutex, INFINITE);
	numAeroportos = dados->BufCircular->nAeroportos;
	CopyMemory(aux, dados->BufAeroportos, sizeof(Aeroporto) * numAeroportos);
	ReleaseMutex(sinc.mutex);

	for (int i = 0; i < numAeroportos; i++)
	{
		if (wcscmp(aux[i].nome, nome) == 0) {
			CopyMemory(ap, &aux[i], sizeof(Aeroporto));
			return TRUE;
		}
	}
	return FALSE;
}

DWORD WINAPI RecebeMensagens(LPVOID param) { // Thread responsável por receber as mensagens enviadas pelo cotrolador e efetuar o tratamento das mesmas
	PTLER dados = (PTLER)param;
	dados->continua = 1;
	HANDLE eventos[2];
	TCHAR nomeEvento[20];
	_stprintf_s(nomeEvento, 19, TEXT("%d"), GetCurrentProcessId());

	eventos[0] = dados->eventoEncerraAviao;
	eventos[1] = CreateEvent(
		NULL,               
		FALSE,              
		FALSE,             
		nomeEvento 
	);

	if (eventos[1] == NULL)
	{
		_ftprintf(stderr, TEXT("Erro ao criar mecanismo de leitura de mensagens!"));
		return -1;
	}

	while (dados->continua)
	{
			WaitForMultipleObjects(2, eventos, FALSE, INFINITE); // Espera ter alguma coisa para ler na memória partilhada
			WaitForSingleObject(dados->dadosMem->mutexMensagens, INFINITE); // Garante acesso único
			CopyMemory(&dados->msg, dados->dadosMem->BufMens, sizeof(Mensagens));
			ReleaseMutex(dados->dadosMem->mutexMensagens);
			if (wcscmp(dados->msg.mensagem, TEXT("OK")) == 0 || wcscmp(dados->msg.mensagem, TEXT("NAO_PERMITIDO")) == 0)
				SetEvent(dados->eventoContinua); // Avisa a thread responsável pelo tratamento do comando que pode ir ler a sua mensagem
			else if (wcscmp(dados->msg.mensagem, TEXT("TERMINAR")) == 0){ // Recebe instroções para terminar a execução do programa
				dados->continua = 0;
				*dados->continuaThreadTeclado = 0;
				_ftprintf(stdout, TEXT("\n\nO controlador foi desligado!\n\n[ESCREVER ALGO E CARREGAR ENTER PARA CONTINUAR]\n\n\n"));
				SetEvent(dados->eventoContinua);
				SetEvent(dados->eventoEncerraAviao);
			}
			else{ 
				dados->continua = 0;
			}
	}

	CloseHandle(eventos[1]);
	return 0;
}

BOOL verificaCordsAv(Aviao ap) { // retorna true quando as coordenadas são válidas
	return ap.pos.x >= 0 && ap.pos.x <= 1000 && ap.pos.y >= 0 && ap.pos.y <= 1000;
}

