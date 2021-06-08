#include "threadsControlador.h"
#include "windowsx.h"

int enviaMsg(DATAPIPES dadosPipes);
void broadcastClientes(DATAPIPES dadosPipes);
void iniciaClientes(PDATAPIPES dadosPipes);
void removeCliente(DATAPIPES dadosPipes);
void adicionaClientes(PDATAPIPES dadosPipes, HANDLE hPipe);
int comunicaPassageiro(HANDLE hPipe, HANDLE evento, TCHAR msg[200]);
void listaPassageiros(PDATAPIPES dados);


LRESULT CALLBACK TrataEventos(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK dialog_regista_aeroporto(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);

#define MAX_LOADSTRING 100

TCHAR szProgName[] = TEXT("Controlador");
HINSTANCE hInstance;
WCHAR szTitle[MAX_LOADSTRING];                  // The title bar text
WCHAR szWindowClass[MAX_LOADSTRING];            // the main window class name

// Vari�veis Globais
DATAPIPES dadosPipe;
HANDLE hthread[4];
HANDLE hPipeTemp;
int contThread = 0;
THREADTEC estruturaThread;
MemDados sem;
Sinc sinc;
THREADCONS threadcons;

/*typedef struct{
	DATAPIPES dadosPipe;
	HANDLE hthread[4];
	HANDLE hPipeTemp;
	int contThread;
	THREADTEC estruturaThread;
	MemDados sem;
	Sinc sinc;
	THREADCONS threadcons;
} DATA, *PDATA;*/

int WINAPI WinMain(HINSTANCE hInst, HINSTANCE hPrevInst, LPSTR lpCmdLine, int nCmdShow) {
	HWND hWnd;		// hWnd � o handler da janela, gerado mais abaixo por CreateWindow()
	MSG lpMsg;		// MSG � uma estrutura definida no Windows para as mensagens
	WNDCLASSEX wcApp;	// WNDCLASSEX � uma estrutura cujos membros servem para 
			  // definir as caracter�sticas da classe da janela

	// ============================================================================
	// 1. Defini��o das caracter�sticas da janela "wcApp" 
	//    (Valores dos elementos da estrutura "wcApp" do tipo WNDCLASSEX)
	// ============================================================================
	wcApp.cbSize = sizeof(WNDCLASSEX);      // Tamanho da estrutura WNDCLASSEX
	wcApp.hInstance = hInst;		         // Inst�ncia da janela actualmente exibida 
								   // ("hInst" � par�metro de WinMain e vem 
										 // inicializada da�)
	wcApp.lpszClassName = szProgName;       // Nome da janela (neste caso = nome do programa)
	wcApp.lpfnWndProc = TrataEventos;       // Endere�o da fun��o de processamento da janela
											// ("TrataEventos" foi declarada no in�cio e
											// encontra-se mais abaixo)
	wcApp.style = CS_HREDRAW | CS_VREDRAW;  // Estilo da janela: Fazer o redraw se for
											// modificada horizontal ou verticalmente

	wcApp.hIcon = LoadIcon(NULL, IDI_APPLICATION);   // "hIcon" = handler do �con normal
										   // "NULL" = Icon definido no Windows
										   // "IDI_AP..." �cone "aplica��o"
	wcApp.hIconSm = LoadIcon(NULL, NULL); // "hIconSm" = handler do �con pequeno
										   // "NULL" = Icon definido no Windows
										   // "IDI_INF..." �con de informa��o
	wcApp.hCursor = LoadCursor(NULL, IDC_ARROW);	// "hCursor" = handler do cursor (rato) 
							  // "NULL" = Forma definida no Windows
							  // "IDC_ARROW" Aspecto "seta" 
	wcApp.lpszMenuName = MAKEINTRESOURCE(IDR_MENU1);			// Classe do menu que a janela pode ter
							  // (NULL = n�o tem menu)
	wcApp.cbClsExtra = 0;				// Livre, para uso particular
	wcApp.cbWndExtra = 0;				// Livre, para uso particular
	wcApp.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);

	// Perform application initialization:
	hInstance = hInst;

	// ============================================================================
	// 2. Registar a classe "wcApp" no Windows
	// ============================================================================
	if (!RegisterClassEx(&wcApp))
		return(0);

	//todo Definir a estrutura com os dados necess�rios
	//wcApp.cbWndExtra = sizeof(DADOS *); DADOS -> Estrutura dos dados necess�rios 
	//DADOS dados -> Definir a estrutura e inicializar os seus dados necess�rios
	//SetWindowLongPtr(hWnd,0, (LONG_PTR)& dados); -> Definir os dados
	//DADOS * pont = (DADOS*)GetWindowLongPtr(hWnd,0); -> Obter os dados na fun��o janela

	// ============================================================================
	// 3. Criar a janela
	// ============================================================================
	hWnd = CreateWindow(
		szProgName,			// Nome da janela (programa) definido acima
		TEXT("Controlador"),// Texto que figura na barra do t�tulo
		WS_OVERLAPPEDWINDOW,	// Estilo da janela (WS_OVERLAPPED= normal)
		CW_USEDEFAULT,		// Posi��o x pixels (default=� direita da �ltima)
		CW_USEDEFAULT,		// Posi��o y pixels (default=abaixo da �ltima)
		CW_USEDEFAULT,		// Largura da janela (em pixels)
		CW_USEDEFAULT,		// Altura da janela (em pixels)
		(HWND)HWND_DESKTOP,	// handle da janela pai (se se criar uma a partir de
						// outra) ou HWND_DESKTOP se a janela for a primeira, 
						// criada a partir do "desktop"
		(HMENU)NULL,			// handle do menu da janela (se tiver menu)
		(HINSTANCE)hInst,		// handle da inst�ncia do programa actual ("hInst" � 
						// passado num dos par�metros de WinMain()
		0);				// N�o h� par�metros adicionais para a janela

	// ============================================================================
	// 4. Inicializar programa
	// ============================================================================

	dadosPipe.terminar = 0;
	estruturaThread.continua = 1;
	char flagMostraA = 0; // todo retirar esta flag
	threadcons.flagMostraA = &flagMostraA;
	estruturaThread.flagMostraA = &flagMostraA;
	threadcons.nAviao = 0;
	estruturaThread.nAviao = &threadcons.nAviao;
	estruturaThread.pipes = &dadosPipe;
	threadcons.pipes = &dadosPipe;
	threadcons.sinc = &sinc;
/*
	DATA dados;
	dados.dadosPipe.terminar = 0;
	dados.estruturaThread.continua = 1;
	char flagMostraA = 0; // todo retirar esta flag
	dados.threadcons.flagMostraA = &flagMostraA;
	dados.estruturaThread.flagMostraA = &flagMostraA;
	dados.threadcons.nAviao = 0;
	dados.estruturaThread.nAviao = &threadcons.nAviao;
	dados.estruturaThread.pipes = &dadosPipe;
	dados.threadcons.pipes = &dadosPipe;
	dados.threadcons.sinc = &sinc;

	if ((dados.estruturaThread.evento = dados.threadcons.handles[0] = CreateEvent(NULL, TRUE, FALSE, EVENTO)) == NULL) 
		return -1;

#ifdef UNICODE
	_setmode(_fileno(stdin), _O_WTEXT);
	_setmode(_fileno(stdout), _O_WTEXT);
	_setmode(_fileno(stderr), _O_WTEXT);
#endif

	if (!verificaChave(&dados.estruturaThread.valoresMax)) 
		return -1;
	

	if (abreFileMap(&dados.estruturaThread.dadosMem))
		return -1;
	
	dados.threadcons.dados = &dados.estruturaThread.dadosMem;
	dados.threadcons.listaAvioes = malloc(dados.estruturaThread.valoresMax.numMaxAvioes * sizeof(Aviao));
	dados.estruturaThread.listaAvioes = dados.threadcons.listaAvioes;

	if (dados.threadcons.listaAvioes == NULL)
		return -1;
	
	if (!criaSinc(dados.estruturaThread.valoresMax.numMaxAvioes, &dados.sinc, &dados.sem))
		return -1;

	dados.estruturaThread.sinc = &dados.sinc;
	dados.threadcons.dados->semAviao = dados.sem.semAviao;
	dados.threadcons.dados->mutexMensagens = dados.sem.mutexMensagens;
	dados.threadcons.handles[1] = dados.sem.semControl;

	if (!criaFileMap(&dados.estruturaThread.dadosMem, dados.estruturaThread.valoresMax)) 
		return -1;

	if (!criaMapViewOfFiles(&dados.estruturaThread.dadosMem, dados.estruturaThread.valoresMax, TRUE)) 
		return -1;

	dados.estruturaThread.dadosMem.BufCircular->nAeroportos = 0;
	dados.threadcons.dados->BufCircular->in = 0;
	dados.threadcons.dados->BufCircular->out = 0;
	dados.threadcons.control = 0;
	dados.dadosPipe.dados = &dados.estruturaThread.dadosMem;

	if ((dados.hthread[dados.contThread++] = CreateThread(NULL, 0, ThreadConsumidor, &dados.threadcons, 0, NULL)) == NULL)
		return -1;
	

	if ((dados.hthread[dados.contThread++] = CreateThread(NULL, 0, PingAviao, &dados.threadcons, 0, NULL)) == NULL)
		return -1;
	
	SetWindowLongPtr(hWnd,0, (LONG_PTR)& dados);

	WaitForMultipleObjects(dados.contThread, dados.hthread, TRUE, INFINITE);

	fechaViewFile(&dados.estruturaThread.dadosMem);

	fechaHandleMem(&dados.estruturaThread.dadosMem);
	free(dados.threadcons.listaAvioes);
	return((int)lpMsg.wParam);	// Retorna sempre o par�metro wParam da estrutura lpMsg
*/

	if ((estruturaThread.evento = threadcons.handles[0] = CreateEvent(NULL, TRUE, FALSE, EVENTO)) == NULL) {
		_ftprintf(stderr, TEXT("Erro ao criar evento!\n"));
		return -1;
	}

#ifdef UNICODE
	_setmode(_fileno(stdin), _O_WTEXT);
	_setmode(_fileno(stdout), _O_WTEXT);
	_setmode(_fileno(stderr), _O_WTEXT);
#endif

	_ftprintf(stdout, TEXT("Bem vindo ao sistema de controlador a�rio!\n\n"));

	// Obtem n�mero m�ximo de avi�es e aeroportos (Default - 5 Avi�es e 3 Aeroportos) 
	if (!verificaChave(&estruturaThread.valoresMax)) {
		_ftprintf(stderr, TEXT("Erro a obter valores m�ximos de aeroportos e de avi�es!\n"));
		return -1;
	}

	// Verifica se j� existe um segundo processo a decorrer e cria mem�ria partilhada
	if (abreFileMap(&estruturaThread.dadosMem))
	{
		_ftprintf(stderr, TEXT("J� existe um Controlador aberto! A terminar...\n"));
		return -1;
	}
	threadcons.dados = &estruturaThread.dadosMem;
	threadcons.listaAvioes = malloc(estruturaThread.valoresMax.numMaxAvioes * sizeof(Aviao));
	estruturaThread.listaAvioes = threadcons.listaAvioes;

	if (threadcons.listaAvioes == NULL)
	{
		_ftprintf(stderr, TEXT("Erro de mem�ria de avi�es\n"));
		return -1;
	}

	// Cria mecanismos de sincroniza��o 
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

	if ((hthread[contThread++] = CreateThread(NULL, 0, ThreadConsumidor, &threadcons, 0, NULL)) == NULL) { //Thread respons�vel para receber todos os pedidos dos avi�es
		_tprintf(_T("Erro a criar thread do buffer no controlador"));
		return -1;
	}

	if ((hthread[contThread++] = CreateThread(NULL, 0, PingAviao, &threadcons, 0, NULL)) == NULL) { //Thread para fazer ping nos avi�es e verificar que os mesmos ainda est�o ativos
		_tprintf(_T("Erro a criar thread de ping no controlador"));
		return -1;
	}

	//todo Dialog box para isto _ftprintf(stdout, TEXT("Avi�es: %d\nAeroportos: %d\n\n"), estruturaThread.valoresMax.numMaxAvioes, estruturaThread.valoresMax.numMaxAeroportos);

	// ============================================================================
	// 4. Mostrar a janela
	// ============================================================================
	ShowWindow(hWnd, nCmdShow);	// "hWnd"= handler da janela, devolvido por 
					  // "CreateWindow"; "nCmdShow"= modo de exibi��o (p.e. 
					  // normal/modal); � passado como par�metro de WinMain()
	UpdateWindow(hWnd);		// Refrescar a janela (Windows envia � janela uma 
					  // mensagem para pintar, mostrar dados, (refrescar)� 

	// ============================================================================
	// 5. Loop de Mensagens
	// ============================================================================
	while (GetMessage(&lpMsg, NULL, 0, 0)) {
		TranslateMessage(&lpMsg);
		DispatchMessage(&lpMsg);
	}
	// ============================================================================
	// 6. Fim do programa
	// ============================================================================

	// Espera que a thread do teclado termine
	WaitForMultipleObjects(contThread, hthread, TRUE, INFINITE);
	// Unmap das vistas
	fechaViewFile(&estruturaThread.dadosMem);
	// Fecha os handles da extrutura MemDados
	fechaHandleMem(&estruturaThread.dadosMem);
	free(threadcons.listaAvioes);
	return((int)lpMsg.wParam);	// Retorna sempre o par�metro wParam da estrutura lpMsg
}

LRESULT CALLBACK TrataEventos(HWND hWnd, UINT messg, WPARAM wParam, LPARAM lParam) {
	//PDATA dados = (PDATA)GetWindowLongPtr(hWnd,0);
	switch (messg) {
		case WM_DESTROY:
			// todo terminar as cenas

			//// Espera que a thread do teclado termine
			//WaitForMultipleObjects(contThread, hthread, TRUE, INFINITE);
			//// Unmap das vistas
			//fechaViewFile(&estruturaThread.dadosMem);
			//// Fecha os handles da extrutura MemDados
			//fechaHandleMem(&estruturaThread.dadosMem);
			//free(threadcons.listaAvioes);

			PostQuitMessage(0);
			break;
		case WM_COMMAND: 
		{
			int wmId = LOWORD(wParam);
			switch (wmId)
			{
			case ID_SAIR: // Bot�o Menu Sair
				DestroyWindow(hWnd);
				break;
			case ID_AEROPORTO_CRIARNOVOAEROPORTO: // Bot�o Menu Aeroportos
				DialogBox(hInstance, MAKEINTRESOURCE(IDD_CRIAR_AEROPORTO), hWnd, dialog_regista_aeroporto);
				break;
			case ID_PASSAGEIROS_VERPASSAGEIROSREGISTADOS: // Bot�o Menu Ver Todos os Passageiros
				break;
			case ID_AVI32772: // Bot�o checked para aceitar ou n�o novos avi�es
				break;
			default:
				return DefWindowProc(hWnd, messg, wParam, lParam);
			}
			break;
		}
		default:
			return(DefWindowProc(hWnd, messg, wParam, lParam));
			break; 
	}
	return(0);
}

INT_PTR CALLBACK dialog_regista_aeroporto(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	UNREFERENCED_PARAMETER(lParam);
	switch (message)
	{
	case WM_INITDIALOG:
		return (INT_PTR)TRUE;

	case WM_COMMAND:
		if (LOWORD(wParam) == IDOK)// Criar um novo aeroporto
		{
			Aeroporto ap;
			int numAeroportos;
			numAeroportos = estruturaThread.dadosMem.BufCircular->nAeroportos; // Obtem n�mero de aeroportos atuais 
			
			// Obtem e valida o nome do aeroporto
			Edit_GetText(GetDlgItem(hDlg, IDC_EDIT1),&ap.nome,49);
			if (existeNome(ap, estruturaThread.dadosMem.BufAeroportos, numAeroportos, estruturaThread.sinc)) {
				Edit_SetText(GetDlgItem(hDlg, IDC_EDIT1), TEXT(""));
				return(INT_PTR)TRUE;
			}

			TCHAR bufNum[8];
			// Obtem a posi��o x
			Edit_GetText(GetDlgItem(hDlg, IDC_EDIT2), &bufNum, 7);
			ap.pos.x = _ttoi(bufNum);
			// Obtem a posi��o y
			Edit_GetText(GetDlgItem(hDlg, IDC_EDIT3), &bufNum, 7);
			ap.pos.y = _ttoi(bufNum);

			// Valida coordenadas
			if (existeAeroportoPerto(ap, estruturaThread.dadosMem.BufAeroportos, numAeroportos) || !verificaCordsAp(ap)) {
				Edit_SetText(GetDlgItem(hDlg, IDC_EDIT2), TEXT(""));
				Edit_SetText(GetDlgItem(hDlg, IDC_EDIT3), TEXT(""));
				return(INT_PTR)TRUE;
			}

			// Regista o aeroporto
			criaAeroporto(ap, estruturaThread.dadosMem.BufAeroportos, &estruturaThread.dadosMem.BufCircular->nAeroportos, estruturaThread.sinc);


			EndDialog(hDlg, LOWORD(wParam));
			return (INT_PTR)TRUE;
		}
		else if (LOWORD(wParam) == IDCANCEL)
		{
			EndDialog(hDlg, LOWORD(wParam));
			return (INT_PTR)TRUE;
		}
		break;
	}
	return (INT_PTR)FALSE;
}