#include "threadsControlador.h"
#include "windowsx.h"
#include <math.h>

INT_PTR CALLBACK ver_passageiros(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK TrataEventos(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK dialog_regista_aeroporto(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
INT_PTR CALLBACK sobre_menu(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
void acrescentaPassageiroLista(PDATAPIPES dados, HWND hwndCtl);

#define MAX_LOADSTRING 100

//TCHAR szProgName[] = TEXT("Controlador");
//HINSTANCE hInstance;
//WCHAR szTitle[MAX_LOADSTRING];                  // The title bar text
//WCHAR szWindowClass[MAX_LOADSTRING];            // the main window class name
//
//// Variáveis Globais
//DATAPIPES dadosPipe;
//HANDLE hthread[4];
//HANDLE hPipeTemp;
//int contThread = 0;
//THREADTEC estruturaThread;
//MemDados sem;
//Sinc sinc;
//THREADCONS threadcons;
//
//HICON plane, airport;
//HDC hdcpic = NULL;
//HDC hdcDB = NULL;
//HBITMAP hbDB = NULL,hold = NULL;
//HDC hdAirport = NULL,hdPlane = NULL;
//HWND hWnd;		// hWnd é o handler da janela, gerado mais abaixo por CreateWindow()

typedef struct{
	TCHAR szProgName[50]; //  TEXT("Controlador")
	HINSTANCE hInstance;
	WCHAR szTitle[MAX_LOADSTRING];                  // The title bar text
	WCHAR szWindowClass[MAX_LOADSTRING];            // the main window class name

	DATAPIPES dadosPipe;
	HANDLE hthread[4];
	HANDLE hPipeTemp;
	int contThread; // 0
	THREADTEC estruturaThread;
	MemDados sem;
	Sinc sinc;
	THREADCONS threadcons;

	HICON plane, airport;
	HDC hdcpic;
	HDC hdcDB;
	HBITMAP hbDB,hold;
	HDC hdAirport,hdPlane ;
HWND hWnd;		// hWnd é o handler da janela, gerado mais abaixo por CreateWindow()
} DATA, *PDATA;



int WINAPI WinMain(HINSTANCE hInst, HINSTANCE hPrevInst, LPSTR lpCmdLine, int nCmdShow) {
	DATA dados;
	_stprintf_s(dados.szProgName, 50, _TEXT("Controlador"));

	dados.hdcpic = NULL;
	dados.hdcDB = NULL;
	dados.hbDB = NULL;
	dados.hold = NULL;
	dados.hdAirport = NULL;
	dados.hdPlane = NULL;
	dados.dadosPipe.terminar = 0;
	dados.contThread = 0;
	dados.estruturaThread.continua = 1;
	char flagMostraA = 0;
	dados.threadcons.flagMostraA = &flagMostraA;
	dados.estruturaThread.flagMostraA = &flagMostraA;
	dados.threadcons.nAviao = 0;
	dados.estruturaThread.nAviao = &dados.threadcons.nAviao;
	dados.estruturaThread.pipes = &dados.dadosPipe;
	dados.threadcons.pipes = &dados.dadosPipe;
	dados.threadcons.sinc = &dados.sinc;
	dados.threadcons.hWnd = &dados.hWnd;

	MSG lpMsg;		// MSG é uma estrutura definida no Windows para as mensagens
	WNDCLASSEX wcApp;	// WNDCLASSEX é uma estrutura cujos membros servem para 
			  // definir as características da classe da janela
	// Perform application initialization:
	dados.hInstance = hInst;
	// ============================================================================
	// 1. Definição das características da janela "wcApp" 
	//    (Valores dos elementos da estrutura "wcApp" do tipo WNDCLASSEX)
	// ============================================================================
	wcApp.cbSize = sizeof(WNDCLASSEX);      // Tamanho da estrutura WNDCLASSEX
	wcApp.hInstance = hInst;		         // Instância da janela actualmente exibida 
								   // ("hInst" é parâmetro de WinMain e vem 
										 // inicializada daí)
	wcApp.lpszClassName = dados.szProgName;       // Nome da janela (neste caso = nome do programa)
	wcApp.lpfnWndProc = TrataEventos;       // Endereço da função de processamento da janela
											// ("TrataEventos" foi declarada no início e
											// encontra-se mais abaixo)
	wcApp.style = CS_HREDRAW | CS_VREDRAW;  // Estilo da janela: Fazer o redraw se for
											// modificada horizontal ou verticalmente

	wcApp.hIcon = LoadIcon(dados.hInstance, MAKEINTRESOURCE(IDI_TORRE));   // "hIcon" = handler do ícon normal
										   // "NULL" = Icon definido no Windows
										   // "IDI_AP..." Ícone "aplicação"
	wcApp.hIconSm = LoadIcon(dados.hInstance, MAKEINTRESOURCE(IDI_TORRE)); // "hIconSm" = handler do ícon pequeno
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
	// 4. Inicializar programa
	// ============================================================================

	


	if ((dados.estruturaThread.evento = dados.threadcons.handles[0] = CreateEvent(NULL, TRUE, FALSE, EVENTO)) == NULL) {
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
	if (!verificaChave(&dados.estruturaThread.valoresMax)) {
		_ftprintf(stderr, TEXT("Erro a obter valores máximos de aeroportos e de aviões!\n"));
		return -1;
	}

	// Verifica se já existe um segundo processo a decorrer e cria memória partilhada
	if (abreFileMap(&dados.estruturaThread.dadosMem))
	{
		_ftprintf(stderr, TEXT("Já existe um Controlador aberto! A terminar...\n"));
		return -1;
	}
	dados.threadcons.dados = &dados.estruturaThread.dadosMem;
	dados.threadcons.listaAvioes = malloc(dados.estruturaThread.valoresMax.numMaxAvioes * sizeof(Aviao));
	dados.estruturaThread.listaAvioes = dados.threadcons.listaAvioes;

	if (dados.threadcons.listaAvioes == NULL)
	{
		_ftprintf(stderr, TEXT("Erro de memória de aviões\n"));
		return -1;
	}

	// Cria mecanismos de sincronização 
	if (!criaSinc(dados.estruturaThread.valoresMax.numMaxAvioes, &dados.sinc, &dados.sem))
		return -1;

	dados.estruturaThread.sinc = &dados.sinc;
	dados.threadcons.dados->semAviao = dados.sem.semAviao;
	dados.threadcons.dados->mutexMensagens = dados.sem.mutexMensagens;
	dados.threadcons.handles[1] = dados.sem.semControl;

	if (!criaFileMap(&dados.estruturaThread.dadosMem, dados.estruturaThread.valoresMax)) // Criar FileMaps
		return -1;

	if (!criaMapViewOfFiles(&dados.estruturaThread.dadosMem, dados.estruturaThread.valoresMax, TRUE)) // Criar Vistas
		return -1;

	dados.estruturaThread.dadosMem.BufCircular->nAeroportos = 0;
	dados.threadcons.dados->BufCircular->in = 0;
	dados.threadcons.dados->BufCircular->out = 0;
	dados.threadcons.control = 0;
	dados.dadosPipe.dados = &dados.estruturaThread.dadosMem;

	if ((dados.hthread[dados.contThread++] = CreateThread(NULL, 0, ThreadConsumidor, &dados.threadcons, 0, NULL)) == NULL) { //Thread responsável para receber todos os pedidos dos aviões
		_tprintf(_T("Erro a criar thread do buffer no controlador"));
		return -1;
	}

	if ((dados.hthread[dados.contThread++] = CreateThread(NULL, 0, PingAviao, &dados.threadcons, 0, NULL)) == NULL) { //Thread para fazer ping nos aviões e verificar que os mesmos ainda estão ativos
		_tprintf(_T("Erro a criar thread de ping no controlador"));
		return -1;
	}

	if ((dados.hthread[dados.contThread++] = CreateThread(NULL, 0, ThreadPassageiros, &dados.dadosPipe, 0, NULL)) == NULL) { //Thread para fazer ping nos aviões e verificar que os mesmos ainda estão ativos
		_tprintf(_T("Erro a criar thread de ping no controlador"));
		return -1;
	}

	// Inicia ICONS
	dados.plane = LoadIcon(dados.hInstance, MAKEINTRESOURCE(IDI_PLANE));
	dados.airport = LoadIcon(dados.hInstance, MAKEINTRESOURCE(IDI_AIRPORT));



	// ============================================================================
	// 3. Criar a janela
	// ============================================================================
	dados.hWnd = CreateWindow(
		dados.szProgName,			// Nome da janela (programa) definido acima
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
		&dados);				// Não há parâmetros adicionais para a janela

	// ============================================================================
	// 4. Mostrar a janela
	// ============================================================================
	ShowWindow(dados.hWnd, nCmdShow);	// "hWnd"= handler da janela, devolvido por 
					  // "CreateWindow"; "nCmdShow"= modo de exibição (p.e. 
					  // normal/modal); é passado como parâmetro de WinMain()
	UpdateWindow(dados.hWnd);		// Refrescar a janela (Windows envia à janela uma 
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
	WaitForMultipleObjects(dados.contThread, dados.hthread, TRUE, INFINITE);
	// Unmap das vistas
	fechaViewFile(&dados.estruturaThread.dadosMem);
	// Fecha os handles da extrutura MemDados
	fechaHandleMem(&dados.estruturaThread.dadosMem);
	free(dados.threadcons.listaAvioes);
	return((int)lpMsg.wParam);	// Retorna sempre o parâmetro wParam da estrutura lpMsg
}

LRESULT CALLBACK TrataEventos(HWND hWnd, UINT messg, WPARAM wParam, LPARAM lParam) {
	//PDATA dados = (PDATA)GetWindowLongPtr(hWnd,0);
	HDC hdc;
	PAINTSTRUCT ps;
	RECT area;
	RECT click;
	POINT pt;

	LONG_PTR userdata = GetWindowLongPtr(hWnd, GWLP_USERDATA);
	PDATA dados = NULL;

	// Parametros adicionais para a janela  <--------
	if (messg == WM_CREATE) {
		CREATESTRUCT* pCreate = (CREATESTRUCT*)(lParam);
		dados = (PDATA)pCreate->lpCreateParams;
		SetWindowLongPtr(hWnd, GWLP_USERDATA, (LONG_PTR)dados);
	}
	else {
		dados = (PDATA)GetWindowLongPtr(hWnd, GWLP_USERDATA);
	}

	switch (messg) {
		case WM_LBUTTONDOWN: // Quando clica no rato
			GetClientRect(hWnd, &area);
			pt.x = GET_X_LPARAM(lParam);
			pt.y = GET_Y_LPARAM(lParam);
			for (int i = 0; i < dados->estruturaThread.dadosMem.BufCircular->nAeroportos; i++) {
				float x = round((float)dados->estruturaThread.dadosMem.BufAeroportos[i].pos.x / (float)1040 * (float)area.right);
				float y = round((float)dados->estruturaThread.dadosMem.BufAeroportos[i].pos.y / (float)1040 * (float)area.bottom);
				click.right = x + 30;
				click.left = x - 30;
				click.top = y + 30;
				click.bottom = y - 30;
				if (pt.x < click.right && pt.x > click.left && pt.y < click.top && pt.y > click.bottom)
				{
					dados->estruturaThread.dadosMem.BufAeroportos[i].numAvioes = 0;
					dados->estruturaThread.dadosMem.BufAeroportos[i].numPass = 0;
					for (int j = 0; j < *dados->estruturaThread.nAviao; j++)
						if(dados->estruturaThread.listaAvioes[j].pos.x == dados->estruturaThread.dadosMem.BufAeroportos[i].pos.x && dados->estruturaThread.listaAvioes[j].pos.y == dados->estruturaThread.dadosMem.BufAeroportos[i].pos.y)
							dados->estruturaThread.dadosMem.BufAeroportos[i].numAvioes++;
						
					for (int j = 0; j < TOTAL_PASSAGEIROS; j++) 
						if(dados->estruturaThread.pipes->structClientes[j].flagViagem == 0 &&
							wcscmp(dados->estruturaThread.pipes->structClientes[j].aeroportoOrigem, dados->estruturaThread.dadosMem.BufAeroportos[i].nome) == 0)
							dados->estruturaThread.dadosMem.BufAeroportos[i].numPass++;
					
					TCHAR auxApText[100];
					_stprintf_s(auxApText, sizeof(auxApText) / sizeof(TCHAR), 
						TEXT("Aviões: %d\nPassageiros: %d"), 
						dados->estruturaThread.dadosMem.BufAeroportos[i].numAvioes,
						dados->estruturaThread.dadosMem.BufAeroportos[i].numPass);

					MessageBox(
						hWnd,
						auxApText,
						dados->estruturaThread.dadosMem.BufAeroportos[i].nome,
						NULL);

					break;
				}
			}
			break;
		case WM_MOUSEMOVE:
			GetClientRect(hWnd, &area);
			pt.x = GET_X_LPARAM(lParam);
			pt.y = GET_Y_LPARAM(lParam);
			for (int i = 0; i < *dados->estruturaThread.nAviao; i++) {
				if (!isAeroporto(&dados->estruturaThread.dadosMem, dados->estruturaThread.listaAvioes[i].pos))
				{
					float x = round((float)dados->estruturaThread.listaAvioes[i].pos.x / (float)1040 * (float)area.right);
					float y = round((float)dados->estruturaThread.listaAvioes[i].pos.y / (float)1040 * (float)area.bottom);
					click.right = x + 30;
					click.left = x - 30;
					click.top = y + 30;
					click.bottom = y - 30;
					if (pt.x < click.right && pt.x > click.left && pt.y < click.top && pt.y > click.bottom)
					{
						dados->estruturaThread.listaAvioes[i].numPassagBord = 0;
						for (int j = 0; j < TOTAL_PASSAGEIROS; j++)
							if (dados->estruturaThread.pipes->structClientes[j].idAviao == dados->estruturaThread.listaAvioes[i].id)
								dados->estruturaThread.listaAvioes[i].numPassagBord++;		

						TCHAR auxAText[200];
						_stprintf_s(auxAText, sizeof(auxAText) / sizeof(TCHAR),
							TEXT("ID: %d\nAeroporto Origem: %s\nAeoporto Destino: %s\nPassageiros: %d\nVelocidade: %d unidades/segundo"),
							dados->estruturaThread.listaAvioes[i].id,
							dados->estruturaThread.listaAvioes[i].partida.nome,
							dados->estruturaThread.listaAvioes[i].destino.nome,
							dados->estruturaThread.listaAvioes[i].numPassagBord,
							dados->estruturaThread.listaAvioes[i].velocidade);

						MessageBox(
							hWnd,
							auxAText,
							dados->estruturaThread.listaAvioes[i].id,
							NULL);
						break;
					}
				}
			}
			break;
		case WM_DESTROY: // Quando pede para fechar
			dados->estruturaThread.pipes->terminar = 1;
			DeleteFile(PIPE_NAME);
			broadcastClientes(*dados->estruturaThread.pipes);
			encerraAvioes(&dados->estruturaThread.dadosMem, dados->estruturaThread.listaAvioes, *dados->estruturaThread.nAviao);
			dados->estruturaThread.dadosMem.BufCircular->nAeroportos = -1;					// Coloca o número de aviões negativos para que o aviao.c para que eventos bloqueados possam ter conhecimento que controlador já não existe
			dados->estruturaThread.continua = 0;
			SetEvent(dados->estruturaThread.evento);						// Evento para ultrapassar todos os waits em que possa estar bloqueado
			SetEvent(dados->estruturaThread.sinc->eventoAceitaAviao[0]);	// Garante que todos os aviões suspensos possam avançar para o encerramento
			for (int i = 0; i < TOTAL_PASSAGEIROS; i++) {
				if (dados->estruturaThread.pipes->clientes[i] == NULL)
					SetEvent(dados->estruturaThread.pipes->structClientes[i].eventoTermina);
			}

			PostQuitMessage(0);
			break;
		case WM_COMMAND: // Menu
		{
			int wmId = LOWORD(wParam);
			switch (wmId)
			{
			case ID_AEROPORTO_CRIARNOVOAEROPORTO: // Botão Menu Aeroportos
				DialogBoxParam(dados->hInstance, MAKEINTRESOURCE(IDD_CRIAR_AEROPORTO), hWnd, dialog_regista_aeroporto, dados);
				InvalidateRect(hWnd, NULL, TRUE);
				break;
			case ID_PASSAGEIROS_VERPASSAGEIROSREGISTADOS: // Botão Menu Ver Todos os Passageiros
				DialogBoxParam(dados->hInstance, MAKEINTRESOURCE(IDD_LISTA_PASSAGEIROS), hWnd, ver_passageiros, dados);
				break;
			case ID_AVI32772:// Botão checked para aceitar ou não novos aviões
			{
				DWORD antes = CheckMenuItem(GetMenu(hWnd), ID_AVI32772, NULL);
				if (antes == MF_CHECKED) // Suspende novos aviões
				{
					ResetEvent(dados->estruturaThread.sinc->eventoAceitaAviao[0]);
					CheckMenuItem(GetMenu(hWnd), ID_AVI32772, MF_UNCHECKED);
				}
				else // Aceita Novos Aviões
				{
					SetEvent(dados->estruturaThread.sinc->eventoAceitaAviao[0]);
					CheckMenuItem(GetMenu(hWnd), ID_AVI32772, MF_CHECKED);
				}
				break;
			}
			case ID_AVI32777: // Botão checked para mostrar alertas importantes sobre os aviões
			{
				DWORD antes = CheckMenuItem(GetMenu(hWnd), ID_AVI32777, NULL);
				if (antes == MF_CHECKED) // Desliga os alertas
				{
					*dados->estruturaThread.flagMostraA = 0;
					CheckMenuItem(GetMenu(hWnd), ID_AVI32777, MF_UNCHECKED);
				}
				else // Liga alertas sobre aviões
				{
					*dados->estruturaThread.flagMostraA = 1;
					CheckMenuItem(GetMenu(hWnd), ID_AVI32777, MF_CHECKED);
				}
				break;
			}
			case ID_SOBRE: 
			{
				//DialogBox(dados->hInstance, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, sobre_menu);
				DialogBoxParam(dados->hInstance, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, sobre_menu, dados);
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
			
			// Double-Buffer
			GetClientRect(hWnd, &area);
			dados->hdcDB = CreateCompatibleDC(hdc);
			dados->hbDB = CreateCompatibleBitmap(hdc, area.right, area.bottom);
			dados->hold = (HBITMAP)SelectObject(dados->hdcDB, dados->hbDB);

			FillRect(dados->hdcDB, &area, (HBRUSH)GetStockObject(WHITE_BRUSH)); // Pintar todo o ecrã de branco

			// Pintar todos os Aviões
			for (int i = 0; i < *dados->estruturaThread.nAviao; i++) {
				float x = round((float)dados->estruturaThread.listaAvioes[i].pos.x / (float)1040 * (float)area.right);
				float y = round((float)dados->estruturaThread.listaAvioes[i].pos.y / (float)1040 * (float)area.bottom);
				DrawIcon(dados->hdcDB, x, y, dados->plane);
			}

			// Pintar todos os aeroportos
			for (int i = 0; i < dados->estruturaThread.dadosMem.BufCircular->nAeroportos; i++) {
				float x = round((float)dados->estruturaThread.dadosMem.BufAeroportos[i].pos.x / (float)1040 * (float)area.right);
				float y = round((float)dados->estruturaThread.dadosMem.BufAeroportos[i].pos.y / (float)1040 * (float)area.bottom);
				DrawIcon(dados->hdcDB, x, y, dados->airport);
			}
			
			BitBlt(hdc, 0, 0, area.right, area.bottom,
				dados->hdcDB, 0, 0, SRCCOPY);
			
			SelectObject(dados->hdcDB, dados->hold);
			DeleteObject(dados->hbDB);
			DeleteDC(dados->hdcDB);
			EndPaint(hWnd, &ps);
			
			return(DefWindowProc(hWnd, messg, wParam, lParam));
			break;
		}
		case WM_ERASEBKGND:
			break;
		default:
			return(DefWindowProc(hWnd, messg, wParam, lParam));
			break; 
	}
	return(0);
}

INT_PTR CALLBACK dialog_regista_aeroporto(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	PDATA dados = NULL;	

	switch (message)
	{
	case WM_INITDIALOG:
		SetWindowLongPtr(hDlg, GWLP_USERDATA, (LONG_PTR)lParam);
		return (INT_PTR)TRUE;

	case WM_COMMAND:
		if (LOWORD(wParam) == IDOK)// Criar um novo aeroporto
		{
			dados = (PDATA)GetWindowLongPtr(hDlg, GWLP_USERDATA);
			Aeroporto ap;
			int numAeroportos = 0;
			numAeroportos = dados->estruturaThread.dadosMem.BufCircular->nAeroportos; // Obtem número de aeroportos atuais 
			// Obtem e valida o nome do aeroporto
			Edit_GetText(GetDlgItem(hDlg, IDC_EDIT1),&ap.nome,49);
			if (_tcslen(ap.nome) < 1) {
				MessageBeep(MB_ICONWARNING);
				return(INT_PTR)TRUE;
			}

			if (existeNome(ap, dados->estruturaThread.dadosMem.BufAeroportos, numAeroportos, dados->estruturaThread.sinc)) {
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
			if (existeAeroportoPerto(ap, dados->estruturaThread.dadosMem.BufAeroportos, numAeroportos) || !verificaCordsAp(ap)) {
				Edit_SetText(GetDlgItem(hDlg, IDC_EDIT2), TEXT(""));
				Edit_SetText(GetDlgItem(hDlg, IDC_EDIT3), TEXT(""));
				MessageBeep(MB_ICONWARNING);
				return(INT_PTR)TRUE;
			}

			ap.numAvioes = 0;
			ap.numPass = 0;

			// Regista o aeroporto
			criaAeroporto(ap, dados->estruturaThread.dadosMem.BufAeroportos, &dados->estruturaThread.dadosMem.BufCircular->nAeroportos, dados->estruturaThread.sinc);

			// Desativa botão de criar aeroportos caso o número máximo tenha sido atingido
			if (dados->estruturaThread.dadosMem.BufCircular->nAeroportos >= dados->estruturaThread.valoresMax.numMaxAeroportos)
				EnableMenuItem(GetSubMenu(GetMenu(dados->hWnd), 0), ID_AEROPORTO_CRIARNOVOAEROPORTO, MF_DISABLED | MF_GRAYED);

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
	PDATA dados = NULL;	

	switch (message)
	{
	case WM_INITDIALOG:
	{
		SetWindowLongPtr(hDlg, GWLP_USERDATA, (LONG_PTR)lParam);
		dados = (PDATA)GetWindowLongPtr(hDlg, GWLP_USERDATA);
		// Atualiza texto
		TCHAR auxLabel[200];
		// Aviões
		_stprintf_s(auxLabel, sizeof(auxLabel) / sizeof(TCHAR), TEXT("Máximo de Aviões: %d"), dados->estruturaThread.valoresMax.numMaxAvioes);
		Static_SetText(GetDlgItem(hDlg, IDC_STATIC_AVIOES), auxLabel);
		// Aeroportos
		_stprintf_s(auxLabel, sizeof(auxLabel) / sizeof(TCHAR), TEXT("Máximo de Aeroportos: %d"), dados->estruturaThread.valoresMax.numMaxAeroportos);
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
	PDATA dados = NULL;	

	switch (message)
	{
	case WM_INITDIALOG:
	{
		SetWindowLongPtr(hDlg, GWLP_USERDATA, (LONG_PTR)lParam);
		dados = (PDATA)GetWindowLongPtr(hDlg, GWLP_USERDATA);
		ListBox_ResetContent(GetDlgItem(hDlg, IDC_LIST1)); // Dá Reset a toda a lista
		TCHAR auxL[200];
		acrescentaPassageiroLista(dados->estruturaThread.pipes, GetDlgItem(hDlg, IDC_LIST1));

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

