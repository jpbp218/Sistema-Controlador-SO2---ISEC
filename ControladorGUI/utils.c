#include "utils.h"

void criaAeroporto(Aeroporto ap, Aeroporto* BufAeroportos,int * numAeroportos, Sinc * sinc){
	WaitForSingleObject(sinc->mutex, INFINITE);
	CopyMemory(&BufAeroportos[(*numAeroportos)++], &ap, sizeof(Aeroporto));
	ReleaseMutex(sinc->mutex);
}

BOOL existeAeroportoPerto(Aeroporto ap, Aeroporto* BufAeroportos, int numAeroportos) {
	Aeroporto aux;
	for (int i = 0; i < numAeroportos; i++)
	{
		CopyMemory(&aux, &BufAeroportos[i], sizeof(Aeroporto));
		if (sqrt(pow((aux.pos.x - ap.pos.x),2) + pow((aux.pos.y - ap.pos.y),2)) < 10.00000)
			return TRUE;
	}
	return FALSE;
}

BOOL existeNome(Aeroporto ap, Aeroporto* BufAeroportos, int numAeroportos) {
	Aeroporto aux;
	for (int i = 0; i < numAeroportos; i++)
	{
		CopyMemory(&aux, &BufAeroportos[i], sizeof(Aeroporto));
		if (wcscmp(ap.nome, aux.nome) == 0)
			return TRUE;
	}
	return FALSE;
}

void listaAeroportos(MemDados* mem) {
	Aeroporto aux;
	int numAeroportos = mem->BufCircular->nAeroportos;

	for (int i = 0; i < numAeroportos ; i++)
	{
		CopyMemory(&aux, &mem->BufAeroportos[i], sizeof(Aeroporto));
		_ftprintf(stdout, TEXT("Aeroporto \"%s\" - (%d,%d)\n"), aux.nome, aux.pos.x, aux.pos.y);
	}
}

BOOL verificaCordsAp(Aeroporto ap) { // retorna true quando as coordenadas são válidas
	return ap.pos.x >= 0 && ap.pos.x <= 1000 && ap.pos.y >= 0 && ap.pos.y <= 1000;
}


int verificaAviao(int nAviao, Aviao avioes[],DWORD id){
	for (int i = 0; i < nAviao ; i++) {
		if (avioes[i].id == id)
			return i;
	}
	return -1;
}

void apagaAviao(int pos, int * nAviao, Aviao avioes[]){
	for (int i = pos; i < *nAviao - 1; i++) {
		avioes[i].id = avioes[i + 1].id;
		avioes[i].pos.x = avioes[i + 1].pos.x;
		avioes[i].pos.y = avioes[i + 1].pos.y;
	}
	*nAviao -= 1;
}

BOOL validaCordsAviao(MemDados* mem, Aviao avioes[], int nAviao, Aviao aviao) {
	for (int i = 0; i < nAviao; i++)
	{
		if (aviao.pos.x == avioes[i].pos.x && aviao.pos.y == avioes[i].pos.y && isAeroporto(mem, aviao.pos) == FALSE)
			return FALSE;
	}
	return TRUE;
}

BOOL isAeroporto(MemDados* mem, Coordenadas cords) {
	Aeroporto aux;
	int numAeroportos = mem->BufCircular->nAeroportos;

	for (int i = 0; i < numAeroportos; i++)
	{
		CopyMemory(&aux, &mem->BufAeroportos[i], sizeof(Aeroporto));
		if (cords.x == aux.pos.x && cords.y == aux.pos.y)
			return TRUE;
	}
	return FALSE;
}

BOOL aviaoChegou(Aviao aviao) {
	return aviao.pos.x == aviao.destino.pos.x && aviao.pos.y == aviao.destino.pos.y;
}

void encerraAvioes(MemDados* mem,Aviao avioes[],int nAviao){
	HANDLE evento[2];
	Mensagens msgResposta;
	TCHAR nomeEvento[20];

	evento[1] = mem->mutexMensagens;
	_stprintf_s(msgResposta.mensagem, TAM_MENSAGEM, TEXT("TERMINAR"));

	for(int i = 0; i < nAviao;i++){
		_stprintf_s(nomeEvento, 19, TEXT("%d"), avioes[i].id);
		evento[0] = OpenEvent(EVENT_ALL_ACCESS, TRUE, nomeEvento);
		WaitForMultipleObjects(2, evento, FALSE, INFINITE);
		CopyMemory(&mem->BufMens->mensagem, &msgResposta, sizeof(Mensagens));
		ReleaseMutex(evento[1]);
		SetEvent(evento[0]);
		CloseHandle(evento[0]);
	}
}

BOOL existeAeroporto(Aeroporto ap, Aeroporto* BufAeroportos, int numAeroportos) {
	Aeroporto aux;
	for (int i = 0; i < numAeroportos; i++)
	{
		CopyMemory(&aux, &BufAeroportos[i], sizeof(Aeroporto));
		if (wcscmp(ap.nome, aux.nome) == 0)
			return TRUE;
	}
	return FALSE;
}

BOOL existeAeroportoAsString(TCHAR ap[], Aeroporto* BufAeroportos, int numAeroportos) {
	Aeroporto aux;
	for (int i = 0; i < numAeroportos; i++)
	{
		CopyMemory(&aux, &BufAeroportos[i], sizeof(Aeroporto));
		if (wcscmp(ap, aux.nome) == 0)
			return TRUE;
	}
	return FALSE;
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

void removeCliente(PDATAPIPES dadosPipes, HANDLE hPipe) {
	for(int i = 0; i < TOTAL_PASSAGEIROS; i++){
		if(dadosPipes->clientes[i] == hPipe ){
			dadosPipes->clientes[i] = NULL;
			_tcscpy_s(dadosPipes->structClientes[i].nome,sizeof(dadosPipes->structClientes[i].nome)/sizeof(TCHAR),TEXT(""));
			return;
		}
	}
}


HBITMAP LoadImagemDisco(TCHAR* nome) {
	int coderro;
	HBITMAP aux;
	aux = (HBITMAP)LoadImage(NULL, nome,
		IMAGE_BITMAP, 0, 0, LR_CREATEDIBSECTION | LR_DEFAULTSIZE | LR_LOADFROMFILE);
	if (aux == NULL) {
		coderro = GetLastError();
	}
	return aux;
}
