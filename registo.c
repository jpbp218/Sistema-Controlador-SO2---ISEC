#include <windows.h>
#include <tchar.h>
#include <strsafe.h>
#include <fcntl.h>
#include <io.h>
#include <stdio.h>
#include "registo.h"

#define TAM 256
#define AEROPORTOS 3
#define AVIOES 5

int verificaChave(REGISTO_DADOS * pdados) {
	HKEY chave;
	LONG lResult;
	TCHAR chave_nome[TAM], par_valor[TAM], dados[TAM];
	DWORD dataSize = sizeof(dados);
	_stprintf_s(chave_nome, TAM, TEXT("Software\\SO2\\Dados"));

	// Verificar se já existe esta chave registada
	lResult = RegOpenKeyEx(HKEY_CURRENT_USER, chave_nome, 0, KEY_ALL_ACCESS, &chave);

	if (lResult != ERROR_SUCCESS)
	{
		if (lResult == ERROR_FILE_NOT_FOUND) {
			_tprintf(TEXT("Não foram encontrados dados prévios... A usar valores default...\n"));
			if (RegCreateKeyEx(HKEY_CURRENT_USER, chave_nome, 0, NULL, REG_OPTION_NON_VOLATILE, KEY_WRITE, NULL, &chave, &lResult) == ERROR_SUCCESS) { // Cria a chave
				if (lResult == REG_CREATED_NEW_KEY) { // Chave Criada

					// Cria valores do número máximo de aviões
					_stprintf_s(par_valor, TAM, TEXT("Avioes"));
					_stprintf_s(dados, TAM, TEXT("%d"), AVIOES);
						pdados->numMaxAvioes = AVIOES;

					if (RegSetValueEx(chave, par_valor, 0, REG_SZ, dados, sizeof(TCHAR) * _tcslen(par_valor)) == ERROR_SUCCESS) {
						_tprintf(_T("Nº Máximo de aviões: %d\n"), AVIOES);
					}
					else {
						_tprintf(_T("Erro a criar Valor..."));
						RegCloseKey(chave);
						return 0;
					}


					// Cria valores do número máximo de aeroportos
					_stprintf_s(par_valor, TAM, TEXT("Aeroportos"));
					_stprintf_s(dados, TAM, TEXT("%d"), AEROPORTOS);
					pdados->numMaxAeroportos = AEROPORTOS;

					if (RegSetValueEx(chave, par_valor, 0, REG_SZ, dados, sizeof(TCHAR) * _tcslen(par_valor)) == ERROR_SUCCESS) {
						_tprintf(_T("Nº Máximo de Aeroportos: %d\n"), AEROPORTOS);
					}
					else {
						_tprintf(_T("Erro a criar Valor..."));
						RegCloseKey(chave);
						return 0;
					}
				}
			}
			else { // Erro ao criar chave
				_tprintf(TEXT("Erro ao criar a chave...\n"));
				return 0;
			}
		}
		else { // Não é possível abrir a chave
			_tprintf(TEXT("Erro a abrir a chave...\n"));
			return 0;
		}
	}
	else { // Chave já existe, obtem os valores
		int aux;
		// Obtem dados do aeroporto
		_stprintf_s(par_valor, TAM, TEXT("Aeroportos"));
		if (RegQueryValueEx(chave, par_valor, NULL, NULL, (LPBYTE)dados, &dataSize) == ERROR_SUCCESS) {
			if ((aux = _ttoi(dados)) > 0) {
				pdados->numMaxAeroportos = aux;
			} 
			else {
				_tprintf(_T("Erro a consultar valor máximo de Aeroportos...\n"));
				RegCloseKey(chave);
				return 0;
			}	
		}
		else {
			_tprintf(_T("Erro a consultar valor máximo de Aeroportos...\n"));
			RegCloseKey(chave);
			return 0;
		}

		// Obtem dados do Aviões
		_stprintf_s(par_valor, TAM, TEXT("Avioes"));
		if (RegQueryValueEx(chave, par_valor, NULL, NULL, (LPBYTE)dados, &dataSize) == ERROR_SUCCESS) {
			
			if ((aux = _ttoi(dados)) > 0) {
					pdados->numMaxAvioes = aux;
			}
			else {
				_tprintf(_T("Erro a consultar valor máximo de Aviões...\n"));
				RegCloseKey(chave);
				return 0;
			}
		}
		else {
			_tprintf(_T("Erro a consultar valor máximo de Aviões...\n"));
			RegCloseKey(chave);
			return 0;
		}
	}

	RegCloseKey(chave); // fecha a chave
	return 1;
}
