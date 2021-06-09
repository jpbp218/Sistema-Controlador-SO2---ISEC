#include "threadsControlador.h"
#include "windowsx.h"

INT_PTR CALLBACK ver_passageiros(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK TrataEventos(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK dialog_regista_aeroporto(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
INT_PTR CALLBACK sobre_menu(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
void acrescentaPassageiroLista(PDATAPIPES dados, HWND hwndCtl);

#define MAX_LOADSTRING 100

TCHAR szProgName[] = TEXT("Controlador");
HINSTANCE hInstance;
WCHAR szTitle[MAX_LOADSTRING];                  // The title bar text
WCHAR szWindowClass[MAX_LOADSTRING];            // the main window class name

// Variáveis Globais
DATAPIPES dadosPipe;
HANDLE hthread[4];
HANDLE hPipeTemp;
int contThread = 0;
THREADTEC estruturaThread;
MemDados sem;
Sinc sinc;
THREADCONS threadcons;

HICON plane, airport;
HDC hdcpic;
HDC hdcDB = NULL;
HBITMAP hbDB = NULL;

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

HWND hWnd;		// hWnd é o handler da janela, gerado mais abaixo por CreateWindow()

int WINAPI WinMain(HINSTANCE hInst, HINSTANCE hPrevInst, LPSTR lpCmdLine, int nCmdShow) {
	MSG lpMsg;		// MSG é uma estrutura definida no Windows para as mensagens
	WNDCLASSEX wcApp;	// WNDCLASSEX é uma estrutura cujos membros servem para 
			  // definir as características da classe da janela
	// Perform application initialization:
	hInstance = hInst;
	// ============================================================================
	// 1. Definição das características da janela "wcApp" 
	//    (Valores dos elementos da estrutura "wcApp" do tipo WNDCLASSEX)
	// ============================================================================
	wcApp.cbSize = sizeof(WNDCLASSEX);      // Tamanho da estrutura WNDCLASSEX
	wcApp.hInstance = hInst;		         // Instância da janela actualmente exibida 
								   // ("hInst" é parâmetro de WinMain e vem 
										 // inicializada daí)
	wcApp.lpszClassName = szProgName;       // Nome da janela (neste caso = nome do programa)
	wcApp.lpfnWndProc = TrataEventos;       // Endereço da função de processamento da janela
											// ("TrataEventos" foi declarada no início e
											// encontra-se mais abaixo)
	wcApp.style = CS_HREDRAW | CS_VREDRAW;  // Estilo da janela: Fazer o redraw se for
											// modificada horizontal ou verticalmente

	wcApp.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_TORRE));   // "hIcon" = handler do ícon normal
										   // "NULL" = Icon definido no Windows
										   // "IDI_AP..." Ícone "aplicação"
	wcApp.hIconSm = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_TORRE)); // "hIconSm" = handler do ícon pequeno
										   // "NULL" = Icon definido no Windows
										   // "IDI_INF..." Ícon de informação
	wcApp.hCursor = LoadCursor(NULL, IDC_ARROW);	// "hCursor" = handler do cursor (rato) 
							  // "NULL" = Forma definida no Windows
							  // "IDC_ARROW" Aspecto "seta" 
	wcApp.lpszMenuName = MAKEINTRESOURCE(IDR_MENU1);			// Classe do menu que a janela pode ter
							  // (NULL = não tem menu)
	wcApp.cbClsExtra = 0;				// Livre, para uso particular
	wcApp.cbWndExtra = 0;				// Livre, para uso particular
	wcApp.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);


	// ============================================================================
	// 2. Registar a classe "wcApp" no Windows
	// ============================================================================
	if (!RegisterClassEx(&wcApp))
		return(0);

	//todo Definir a estrutura com os dados necessários
	//wcApp.cbWndExtra = sizeof(DADOS *); DADOS -> Estrutura dos dados necessários 
	//DADOS dados -> Definir a estrutura e inicializar os seus dados necessários
	//SetWindowLongPtr(hWnd,0, (LONG_PTR)& dados); -> Definir os dados
	//DADOS * pont = (DADOS*)GetWindowLongPtr(hWnd,0); -> Obter os dados na função janela

	// ============================================================================
	// 3. Criar a janela
	// ============================================================================
	hWnd = CreateWindow(
		szProgName,			// Nome da janela (programa) definido acima
		TEXT("Controlador"),// Texto que figura na barra do título
		WS_OVERLAPPEDWINDOW,	// Estilo da janela (WS_OVERLAPPED= normal)
		CW_USEDEFAULT,		// Posição x pixels (default=à direita da última)
		CW_USEDEFAULT,		// Posição y pixels (default=abaixo da última)
		1000,		// Largura da janela (em pixels)
		1000,		// Altura da janela (em pixels)
		(HWND)HWND_DESKTOP,	// handle da janela pai (se se criar uma a partir de
						// outra) ou HWND_DESKTOP se a janela for a primeira, 
						// criada a partir do "desktop"
		(HMENU)NULL,			// handle do menu da janela (se tiver menu)
		(HINSTANCE)hInst,		// handle da instância do programa actual ("hInst" é 
						// passado num dos parâmetros de WinMain()
		0);				// Não há parâmetros adicionais para a janela

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
	threadcons.hWnd = &hWnd;
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
	return((int)lpMsg.wParam);	// Retorna sempre o parâmetro wParam da estrutura lpMsg
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

	if ((hthread[contThread++] = CreateThread(NULL, 0, ThreadPassageiros, &dadosPipe, 0, NULL)) == NULL) { //Thread para fazer ping nos aviões e verificar que os mesmos ainda estão ativos
		_tprintf(_T("Erro a criar thread de ping no controlador"));
		return -1;
	}

	// --- inicia pics
	//plane = LoadImagemDisco(TEXT("C:\\Users\\João Pedro\\source\\repos\\Trabalho Prático SO2\\ControladorGUI\\plane.bmp"));  // Carrega Avião
	//airport = LoadImagemDisco(TEXT("C:\\Users\\João Pedro\\source\\repos\\Trabalho Prático SO2\\ControladorGUI\\airport.bmp"));
	plane = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_PLANE));
	airport = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_AIRPORT));

	DWORD errorIcon = GetLastError();

	//HDC hdcjan = GetDC(hWnd);
	//hdcpic = CreateCompatibleDC(hdcjan);
	//SelectObject(hdcpic, plane);
	//ReleaseDC(hWnd, hdcjan);


	// ============================================================================
	// 4. Mostrar a janela
	// ============================================================================
	ShowWindow(hWnd, nCmdShow);	// "hWnd"= handler da janela, devolvido por 
					  // "CreateWindow"; "nCmdShow"= modo de exibição (p.e. 
					  // normal/modal); é passado como parâmetro de WinMain()
	UpdateWindow(hWnd);		// Refrescar a janela (Windows envia à janela uma 
					  // mensagem para pintar, mostrar dados, (refrescar)… 

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
	return((int)lpMsg.wParam);	// Retorna sempre o parâmetro wParam da estrutura lpMsg
}

POINT pt;

LRESULT CALLBACK TrataEventos(HWND hWnd, UINT messg, WPARAM wParam, LPARAM lParam) {
	//PDATA dados = (PDATA)GetWindowLongPtr(hWnd,0);
	HDC hdc;
	PAINTSTRUCT ps;
	RECT area;

	switch (messg) {
		case WM_LBUTTONDOWN:
			pt.x = GET_X_LPARAM(lParam);
			pt.y = GET_Y_LPARAM(lParam);
			break;
		case WM_DESTROY:
			estruturaThread.pipes->terminar = 1;
			DeleteFile(PIPE_NAME);
			broadcastClientes(*estruturaThread.pipes);
			encerraAvioes(&estruturaThread.dadosMem, estruturaThread.listaAvioes, estruturaThread.nAviao);
			estruturaThread.dadosMem.BufCircular->nAeroportos = -1;					// Coloca o número de aviões negativos para que o aviao.c para que eventos bloqueados possam ter conhecimento que controlador já não existe
			estruturaThread.continua = 0;
			SetEvent(estruturaThread.evento);						// Evento para ultrapassar todos os waits em que possa estar bloqueado
			SetEvent(estruturaThread.sinc->eventoAceitaAviao[0]);	// Garante que todos os aviões suspensos possam avançar para o encerramento
			for (int i = 0; i < TOTAL_PASSAGEIROS; i++) {
				if (estruturaThread.pipes->clientes[i] == NULL)
					SetEvent(estruturaThread.pipes->structClientes[i].eventoTermina);
			}

			PostQuitMessage(0);
			break;
		case WM_COMMAND: 
		{
			int wmId = LOWORD(wParam);
			switch (wmId)
			{
			case ID_AEROPORTO_CRIARNOVOAEROPORTO: // Botão Menu Aeroportos
				DialogBox(hInstance, MAKEINTRESOURCE(IDD_CRIAR_AEROPORTO), hWnd, dialog_regista_aeroporto);
				InvalidateRect(hWnd, NULL, TRUE);
				break;
			case ID_PASSAGEIROS_VERPASSAGEIROSREGISTADOS: // Botão Menu Ver Todos os Passageiros
				DialogBox(hInstance, MAKEINTRESOURCE(IDD_LISTA_PASSAGEIROS), hWnd, ver_passageiros);
				break;
			case ID_AVI32772:// Botão checked para aceitar ou não novos aviões
			{
				DWORD antes = CheckMenuItem(GetMenu(hWnd), ID_AVI32772, NULL);
				if (antes == MF_CHECKED) // Suspende novos aviões
				{
					ResetEvent(estruturaThread.sinc->eventoAceitaAviao[0]);
					CheckMenuItem(GetMenu(hWnd), ID_AVI32772, MF_UNCHECKED);
				}
				else // Aceita Novos Aviões
				{
					SetEvent(estruturaThread.sinc->eventoAceitaAviao[0]);
					CheckMenuItem(GetMenu(hWnd), ID_AVI32772, MF_CHECKED);
				}
				break;
			}
			case ID_SOBRE: 
			{
				DialogBox(hInstance, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, sobre_menu);
				break;
			}
			default:
				return DefWindowProc(hWnd, messg, wParam, lParam);
			}
			break;
		}
		case WM_PAINT: 
		{
			hdc = BeginPaint(hWnd, &ps);

			// dbuffer
			//GetClientRect(hWnd, &area); // not ready during WM_CREATE
			//if (hdcDB == NULL) {
			//	hdcDB = CreateCompatibleDC(hdc);
			//	// linha abaixo: tazves fique desactualizada no primiro resize - ver isto
			//	hbDB = CreateCompatibleBitmap(hdc, area.right, area.bottom);
			//	SelectObject(hdcDB, hbDB);
			//}

			//FillRect(hdcDB, &area, (HBRUSH)GetStockObject(GRAY_BRUSH));

			//BitBlt(hdcDB,
			//	500,500,
			//	100, 100,
			//	hdcpic, 0, 0, SRCCOPY);


			//BitBlt(hdc, 0, 0, area.right, area.bottom,
			//	hdcDB, 0, 0, SRCCOPY);


			// Pintar todos os Aviões
			for (int i = 0; i < *estruturaThread.nAviao; i++)
				DrawIcon(hdc, estruturaThread.listaAvioes[i].pos.x, estruturaThread.listaAvioes[i].pos.y, plane);

			// Pintar todos os aeroportos
			for (int i = 0; i < estruturaThread.dadosMem.BufCircular->nAeroportos; i++)
				DrawIcon(hdc, estruturaThread.dadosMem.BufAeroportos[i].pos.x, estruturaThread.dadosMem.BufAeroportos[i].pos.y, airport);
			
			//BitBlt(hdc, 0, 0, area.right, area.bottom,
			//	hdcDB, 0, 0, SRCCOPY);
			
			EndPaint(hWnd, &ps);
			
			return(DefWindowProc(hWnd, messg, wParam, lParam));
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
			numAeroportos = estruturaThread.dadosMem.BufCircular->nAeroportos; // Obtem número de aeroportos atuais 
			
			// Obtem e valida o nome do aeroporto
			Edit_GetText(GetDlgItem(hDlg, IDC_EDIT1),&ap.nome,49);
			if (_tcslen(ap.nome) < 1) {
				MessageBeep(MB_ICONWARNING);
				return(INT_PTR)TRUE;
			}

			if (existeNome(ap, estruturaThread.dadosMem.BufAeroportos, numAeroportos, estruturaThread.sinc)) {
				Edit_SetText(GetDlgItem(hDlg, IDC_EDIT1), TEXT(""));
				MessageBeep(MB_ICONWARNING);
				return(INT_PTR)TRUE;
			}		

			TCHAR bufNum[8];
			// Obtem a posição x
			Edit_GetText(GetDlgItem(hDlg, IDC_EDIT2), &bufNum, 7);
			if (_tcslen(bufNum) < 1) {
				MessageBeep(MB_ICONWARNING);
				return(INT_PTR)TRUE;
			}
			ap.pos.x = _ttoi(bufNum);
			// Obtem a posição y
			Edit_GetText(GetDlgItem(hDlg, IDC_EDIT3), &bufNum, 7);
			if (_tcslen(bufNum) < 1) {
				MessageBeep(MB_ICONWARNING);
				return(INT_PTR)TRUE;
			}
			ap.pos.y = _ttoi(bufNum);

			// Valida coordenadas
			if (existeAeroportoPerto(ap, estruturaThread.dadosMem.BufAeroportos, numAeroportos) || !verificaCordsAp(ap)) {
				Edit_SetText(GetDlgItem(hDlg, IDC_EDIT2), TEXT(""));
				Edit_SetText(GetDlgItem(hDlg, IDC_EDIT3), TEXT(""));
				MessageBeep(MB_ICONWARNING);
				return(INT_PTR)TRUE;
			}

			// Regista o aeroporto
			criaAeroporto(ap, estruturaThread.dadosMem.BufAeroportos, &estruturaThread.dadosMem.BufCircular->nAeroportos, estruturaThread.sinc);

			// Desativa botão de criar aeroportos caso o número máximo tenha sido atingido
			if (estruturaThread.dadosMem.BufCircular->nAeroportos >= estruturaThread.valoresMax.numMaxAeroportos)
				EnableMenuItem(GetSubMenu(GetMenu(hWnd), 0), ID_AEROPORTO_CRIARNOVOAEROPORTO, MF_DISABLED | MF_GRAYED);

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

INT_PTR CALLBACK sobre_menu(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	UNREFERENCED_PARAMETER(lParam);
	switch (message)
	{
	case WM_INITDIALOG:
	{
		// Atualiza texto
		TCHAR auxLabel[200];
		// Aviões
		_stprintf_s(auxLabel, sizeof(auxLabel) / sizeof(TCHAR), TEXT("Máximo de Aviões: %d"), estruturaThread.valoresMax.numMaxAvioes);
		Static_SetText(GetDlgItem(hDlg, IDC_STATIC_AVIOES), auxLabel);
		// Aeroportos
		_stprintf_s(auxLabel, sizeof(auxLabel) / sizeof(TCHAR), TEXT("Máximo de Aeroportos: %d"), estruturaThread.valoresMax.numMaxAeroportos);
		Static_SetText(GetDlgItem(hDlg, IDC_STATIC_AEROPORTOS), auxLabel);

		return (INT_PTR)TRUE;
	}
	case WM_COMMAND:
		if (LOWORD(wParam) == IDOK )// Criar um novo aeroporto
		{
			EndDialog(hDlg, LOWORD(wParam));
			return (INT_PTR)TRUE;
		}
		break;
	}
	return (INT_PTR)FALSE;
}

INT_PTR CALLBACK ver_passageiros(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	UNREFERENCED_PARAMETER(lParam);
	switch (message)
	{
	case WM_INITDIALOG:
	{
		ListBox_ResetContent(GetDlgItem(hDlg, IDC_LIST1)); // Dá Reset a toda a lista
		TCHAR auxL[200];
		acrescentaPassageiroLista(estruturaThread.pipes, GetDlgItem(hDlg, IDC_LIST1));

		return (INT_PTR)TRUE;
	}
	case WM_COMMAND:
		if (LOWORD(wParam) == IDOK)// Criar um novo aeroporto
		{
			EndDialog(hDlg, LOWORD(wParam));
			return (INT_PTR)TRUE;
		}
		break;
	}
	return (INT_PTR)FALSE;
}

void acrescentaPassageiroLista(PDATAPIPES dados,HWND hwndCtl) {
	TCHAR auxL[200];
	for (int i = 0; i < TOTAL_PASSAGEIROS; i++) {
		if (wcscmp(dados->structClientes[i].nome, TEXT("")) != 0) {
			_stprintf_s(auxL, sizeof(auxL) / sizeof(TCHAR), TEXT("Nome: %s | Origem: %s | Destino: %s\n"), dados->structClientes[i].nome, dados->structClientes[i].aeroportoOrigem, dados->structClientes[i].aeroportoDestino);
	      ListBox_InsertString(hwndCtl, i, auxL);
		}
	}
}

