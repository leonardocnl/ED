/*Membros da equipe:
 * Alexandre de Castro Nunes Borges Filho
 * Leonardo Carvalho Silva
 * Gustavo Figueiredo de Oliveira
*/

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <sys/stat.h>
#include <cmath>
#include <algorithm>

using namespace std;

// Vars de uso na Main
// Info sobre membros da guilda
struct dado
{
	char seriesReference[20];
	char period[10];
	double dataValue;
	char status[10];
	char units[10];
	char magnitude;
	char subject[50];
	char group[100];
	char seriesTitle1[80];
	char seriesTitle2[80];
	char seriesTitle3[80];
	char seriesTitle4[60];
	char seriesTitle5[60];
};

//ASCII Art
void printFile() {
	cout << "      ______ _____ _      ______      " << endl;
	cout << "     |  ____|_   _| |    |  ____|     " << endl;
	cout << "     | |__    | | | |    | |__        " << endl;
	cout << "     |  __|   | | | |    |  __|       " << endl;
	cout << "     | |     _| |_| |____| |____      " << endl;
	cout << "     |_|    |_____|______|______|     " << endl;
}

void printView() {
	cout << " __      _______ ________          __" << endl;
	cout << " \\ \\    / /_   _|  ____\\ \\        / /" << endl;
	cout << "  \\ \\  / /  | | | |__   \\ \\  /\\  / / " << endl;
	cout << "   \\ \\/ /   | | |  __|   \\ \\/  \\/ /  " << endl;
	cout << "    \\  /   _| |_| |____   \\  /\\  /   " << endl;
	cout << "     \\/   |_____|______|   \\/  \\/    " << endl;
}

void printMod() {
	cout << "         __  __  ____  _____         " << endl;
	cout << "        |  \\/  |/ __ \\|  __ \\     " << endl;
	cout << "        | \\  / | |  | | |  | |      " << endl;
	cout << "        | |\\/| | |  | | |  | |      " << endl;
	cout << "        | |  | | |__| | |__| |       " << endl;
	cout << "        |_|  |_|\\____/|_____/       " << endl;
}

void printBusca() {
	cout << "  ____  _    _  _____  _____          " << endl;
	cout << " |  _ \\| |  | |/ ____|/ ____|   /\\    " << endl;
	cout << " | |_) | |  | | (___ | |       /  \\   " << endl;
	cout << " |  _ <| |  | |\\___ \\| |      / /\\ \\  " << endl;
	cout << " | |_) | |__| |____) | |____ / ____ \\ " << endl;
	cout << " |____/ \\____/|_____/ \\_____/_/    \\_\\ " << endl;
}

void printExit() {
	cout << "       ________   _______ _______     " << endl;
	cout << "      |  ____\\ \\ / /_   _|__   __|  " << endl;
	cout << "      | |__   \\ V /  | |    | |      " << endl;
	cout << "      |  __|   > <   | |    | |       " << endl;
	cout << "      | |____ / . \\ _| |_   | |      " << endl;
	cout << "      |______/_/ \\_\\_____|  |_|     " << endl;
}

// Visualizar um trecho especifico do arquivo
void displaySection(int begin, int end)
{
	
}

// Get info da Guilda
// Get de arquivo csv
void csvReadWrite(string fileName, string registerName)
{
	dado registro;
	
	ifstream openFile(fileName);
	ofstream{registerName};
	fstream saveFile;
	saveFile.open(registerName, ios::out | ios::binary);
	
	string line;
	string temp;

	getline(openFile, line);
	stringstream inputString(line);

	//Pega Linha e Salva Binario
	while (getline(openFile, line))
	{
		//int size;
		
		inputString.clear();
		inputString.str(line);

		getline(inputString, temp, ',');
		//size = temp.length();
		strcpy(registro.seriesReference, temp.c_str());
		
		getline(inputString, temp, ',');
		//size = temp.length();
		strcpy(registro.period, temp.c_str());

		getline(inputString, temp, ',');
		if(temp != ""){
			registro.dataValue = stod(temp);
		} else
		{
			registro.dataValue = 0;
		}

		getline(inputString, temp, ',');
		//size = temp.length();
		strcpy(registro.status, temp.c_str());
		
		getline(inputString, temp, ',');
		//size = temp.length();
		strcpy(registro.units, temp.c_str());
		
		getline(inputString, temp, ',');
		//size = temp.length();
		registro.magnitude = temp.at(0);
		
		getline(inputString, temp, ',');
		//size = temp.length();
		strcpy(registro.subject, temp.c_str());
		
		getline(inputString, temp, ',');
		//size = temp.length();
		strcpy(registro.group, temp.c_str());
		
		getline(inputString, temp, ',');
		//size = temp.length();
		strcpy(registro.seriesTitle1, temp.c_str());
		
		getline(inputString, temp, ',');
		//size = temp.length();
		strcpy(registro.seriesTitle2, temp.c_str());
		
		getline(inputString, temp, ',');
		//size = temp.length();
		strcpy(registro.seriesTitle3, temp.c_str());
		
		getline(inputString, temp, ',');
		//size = temp.length();
		strcpy(registro.seriesTitle4, temp.c_str());
		
		getline(inputString, temp, ',');
		//size = temp.length();
		strcpy(registro.seriesTitle5, temp.c_str());

		getline(inputString, temp);
		
		saveFile.write((char*)&registro, sizeof(dado));
	}

	openFile.close();
	saveFile.close();
}


void printDado(dado registro){
	string print;
	
	print = registro.seriesReference;
	cout << "Series Reference:  " << print << endl;
	print = registro.period;
	cout << "Period:            " << print << endl;
	cout << "Data Value:        " << registro.dataValue << endl;
	print = registro.status;
	cout << "Status:            " << print << endl;
	print = registro.units;
	cout << "Units:             " << print << endl;
	print = registro.magnitude;
	cout << "Magnitude:         " << print << endl;
	print = registro.subject;
	cout << "Subject:           " << print << endl;
	print = registro.group;
	cout << "Group:             " << print << endl;
	print = registro.seriesTitle1;
	cout << "Series 1:          " << print << endl;
	print = registro.seriesTitle2;
	cout << "Series 2:          " << print << endl;
	print = registro.seriesTitle3;
	cout << "Series 3:          " << print << endl;
	print = registro.seriesTitle4;
	cout << "Series 4:          " << print << endl;
	print = registro.seriesTitle5;
	cout << "Series 5:          " << print << endl << endl;
	
}

// Print arquivo dat
void datPrint(string name)
{
	string exit;
	fstream r;
	int i = 0;
	r.open(name, ios::in | ios::binary);

	ifstream fileSize(name, ios::binary | ios::ate);
	int size = sizeof(dado);
	size = fileSize.tellg() / size;
	fileSize.close();

	dado registro;
	
	cout << "======================================" << endl;
	cout << "           DESEJA IMPRIMIR?           " << endl;
	cout << size << " Registros." << endl;
	cout << "======================================" << endl;
	cout << "| 1. Imprimir      |      2.Voltar   |" << endl;
	cout << "--------------------------------------" << endl;	
	
	cin >> exit;

	if(exit == "1"){
		

		if (r.is_open())
		{
			while (i < size)
			{
				r.read((char*)&registro, sizeof(dado));
				printDado(registro);
				
				i++;
			}
		}
		else
		{
			cout << "Nao foi possivel abrir o arquivo .DAT" << endl;
		}

		r.close();
	}
	
	cout << "======================================" << endl;
	cout << "           FIM DA IMPRESSAO           " << endl;
	cout << "======================================" << endl;
	cout << "| Digite qualquer valor para sair.   |" << endl;
	cout << "--------------------------------------" << endl;
	
	cin >> exit;
}

// Print arquivo dat pelo range
void datPrint(string name, int x, int y)
{
	fstream r;
	r.open(name, ios::in | ios::binary);
	
	int size = sizeof(dado);
	int start = size * x;

	ifstream fileSize(name, ios::binary | ios::ate);
	
	size = fileSize.tellg() / size;
	fileSize.close();
	
	dado registro;
	r.seekg(start, ios::beg);

	if (r.is_open())
	{
		while (x <= y)
		{
			r.read((char*)&registro, sizeof(dado));
			printDado(registro);
			
			x++;
		}
	}
	else
	{
		cout << "Nao foi possivel abrir o arquivo .DAT" << endl;
	}

	r.close();
	
	string exit;
	
	cout << "======================================" << endl;
	cout << "           FIM DA IMPRESSAO           " << endl;
	cout << "======================================" << endl;
	cout << "| Digite qualquer valor para sair.   |" << endl;
	cout << "--------------------------------------" << endl;
	
	cin >> exit;
}

void datSwap(string name, int x, int y)
{
	fstream r;
	r.open(name, ios::in | ios::out | ios::binary);
	
	int size = sizeof(dado);
	int xPos = size * x;
	int yPos = size * y;

	ifstream fileSize(name, ios::binary | ios::ate);
	
	size = fileSize.tellg() / size;
	fileSize.close();
	
	dado registroX;
	dado registroY;

	if (r.is_open() &&( x < size && x >= 0) && (y < size && y >= 0))
	{
		r.seekg(xPos, ios::beg);
		r.read((char*)&registroX, sizeof(dado));
		cout << "REGISTRO X: " << endl;
		printDado(registroX);
		
		r.seekg(yPos, ios::beg);
		r.read((char*)&registroY, sizeof(dado));
		cout << "REGISTRO Y: " << endl;
		printDado(registroY);
		
		r.seekg(xPos, ios::beg);
		r.write((char*)&registroY, sizeof(dado));
		
		r.seekg(yPos, ios::beg);
		r.write((char*)&registroX, sizeof(dado));

	}
	else
	{
		cout << "Nao foi possivel abrir o arquivo .DAT" << endl;
		cout << "Ou a posicao e invalida." << endl;
	}

	r.close();
	
	string exit;
	
	cout << "======================================" << endl;
	cout << "            FIM DA MUDANCA            " << endl;
	cout << "======================================" << endl;
	cout << "| Digite qualquer valor para sair.   |" << endl;
	cout << "--------------------------------------" << endl;
	
	cin >> exit;
}

void datInsert(string name, int x)
{
	string tempFileName = "temp.dat";
    int size = sizeof(dado);
    
    ofstream criar(tempFileName);
    criar.close();

    fstream inputFile;
    fstream tempFile;
    
    inputFile.open(name, ios::in | ios::out | ios::binary);
    tempFile.open(tempFileName, ios::in | ios::out | ios::binary);

	int xPos = size * x;
	
	ifstream fileSize(name, ios::binary | ios::ate);
	
	size = fileSize.tellg() / size;
	fileSize.close();
	
	dado registro;

	if (inputFile.is_open() &&( x < size && x >= 0) && tempFile.is_open())
	{
		string temp;
		
		inputFile.seekg(xPos, ios::beg);
		
		while(!inputFile.eof())
		{
			inputFile.read((char*)&registro, sizeof(dado));
			tempFile.write((char*)&registro, sizeof(dado));
		}
		
		tempFile.close();
		inputFile.close();
		
		tempFile.open(tempFileName, ios::in | ios::out | ios::binary);
		inputFile.open(name, ios::in | ios::out | ios::binary);
		
		inputFile.seekg(xPos, ios::beg);
		
		cout << "======================================" << endl;
		cout << "          INICIO DA INSERCAO          " << endl;
		cout << "======================================" << endl;
		cout << "Series Reference:  ";
		cin.ignore();
		getline(cin, temp);
		strcpy(registro.seriesReference, temp.c_str());
		
		cout << "Period:            ";
		getline(cin, temp);
		strcpy(registro.period, temp.c_str());
		
		cout << "Data Value:        ";
		getline(cin, temp);
		if(temp != ""){
			registro.dataValue = stod(temp);
		} else
		{
			registro.dataValue = 0;
		}
		
		cout << "Status:            ";
		getline(cin, temp);
		strcpy(registro.status, temp.c_str());
		
		cout << "Units:             ";
		getline(cin, temp);
		strcpy(registro.units, temp.c_str());
		
		cout << "Magnitude:         ";
		cin >> registro.magnitude;
		
		cout << "Subject:           ";
		cin.ignore();
		getline(cin, temp);
		strcpy(registro.subject, temp.c_str());
		
		cout << "Group:             ";
		getline(cin, temp);
		strcpy(registro.group, temp.c_str());
		
		cout << "Series 1:          ";
		getline(cin, temp);
		strcpy(registro.seriesTitle1, temp.c_str());
		
		cout << "Series 2:          ";
		getline(cin, temp);
		strcpy(registro.seriesTitle2, temp.c_str());
		
		cout << "Series 3:          ";
		getline(cin, temp);
		strcpy(registro.seriesTitle3, temp.c_str());
		
		cout << "Series 4:          ";
		getline(cin, temp);
		strcpy(registro.seriesTitle4, temp.c_str());
		
		cout << "Series 5:          ";
		getline(cin, temp);
		strcpy(registro.seriesTitle5, temp.c_str());
		
		inputFile.write((char*)&registro, sizeof(dado));
		
		while(!tempFile.eof())
		{
			tempFile.read((char*)&registro, sizeof(dado));
			inputFile.write((char*)&registro, sizeof(dado));
		}
	}
	else
	{
		cout << "Nao foi possivel abrir o arquivo .DAT" << endl;
		cout << "Ou a posicao e invalida." << endl;
	}

	inputFile.close();
	tempFile.close();
	remove(tempFileName.c_str());
	
	string exit;
	
	cout << "======================================" << endl;
	cout << "           FIM DA INSERCAO            " << endl;
	cout << "======================================" << endl;
	cout << "| Digite qualquer valor para sair.   |" << endl;
	cout << "--------------------------------------" << endl;
	
	cin >> exit;
}

void datChange(string name, int x)
{
	fstream r;
	r.open(name, ios::in | ios::out | ios::binary);
	
	int size = sizeof(dado);
	int xPos = size * x;

	ifstream fileSize(name, ios::binary | ios::ate);
	
	size = fileSize.tellg() / size;
	fileSize.close();
	
	dado registro;

	if (r.is_open() &&( x < size && x >= 0))
	{
		string temp;
		r.seekg(xPos, ios::beg);
		r.read((char*)&registro, sizeof(dado));
		cout << "REGISTRO X: " << endl;
		printDado(registro);
		
		cout << "======================================" << endl;
		cout << "          INICIO DA MUDANCA           " << endl;
		cout << "======================================" << endl;
		cout << "Series Reference:  ";
		cin.ignore();
		getline(cin, temp);
		strcpy(registro.seriesReference, temp.c_str());
		
		cout << "Period:            ";
		getline(cin, temp);
		strcpy(registro.period, temp.c_str());
		
		cout << "Data Value:        ";
		getline(cin, temp);
		if(temp != ""){
			registro.dataValue = stod(temp);
		} else
		{
			registro.dataValue = 0;
		}
		
		cout << "Status:            ";
		getline(cin, temp);
		strcpy(registro.status, temp.c_str());
		
		cout << "Units:             ";
		getline(cin, temp);
		strcpy(registro.units, temp.c_str());
		
		cout << "Magnitude:         ";
		cin >> registro.magnitude;
		
		cout << "Subject:           ";
		cin.ignore();
		getline(cin, temp);
		strcpy(registro.subject, temp.c_str());
		
		cout << "Group:             ";
		getline(cin, temp);
		strcpy(registro.group, temp.c_str());
		
		cout << "Series 1:          ";
		getline(cin, temp);
		strcpy(registro.seriesTitle1, temp.c_str());
		
		cout << "Series 2:          ";
		getline(cin, temp);
		strcpy(registro.seriesTitle2, temp.c_str());
		
		cout << "Series 3:          ";
		getline(cin, temp);
		strcpy(registro.seriesTitle3, temp.c_str());
		
		cout << "Series 4:          ";
		getline(cin, temp);
		strcpy(registro.seriesTitle4, temp.c_str());
		
		cout << "Series 5:          ";
		getline(cin, temp);
		strcpy(registro.seriesTitle5, temp.c_str());
		
		r.seekg(xPos, ios::beg);
		r.write((char*)&registro, sizeof(dado));

	}
	else
	{
		cout << "Nao foi possivel abrir o arquivo .DAT" << endl;
		cout << "Ou a posicao e invalida." << endl;
	}

	r.close();
	
	string exit;
	
	cout << "======================================" << endl;
	cout << "            FIM DA MUDANCA            " << endl;
	cout << "======================================" << endl;
	cout << "| Digite qualquer valor para sair.   |" << endl;
	cout << "--------------------------------------" << endl;
	
	cin >> exit;
}

// Leitura de Arquivos
// Handler arquivo
void getFileByName(string &nome)
{
	bool exist = false;
	struct stat sb;
	
	printFile();
	cout << "======================================" << endl;
	cout << "        CONVERSAO ARQUIVO CSV         " << endl;
	cout << "======================================" << endl;
	cout << "| 1. Converter  |  2. Usar Existente  " << endl;
	cout << "--------------------------------------" << endl;
	
	int c;
	cin >> c;
	
	system("cls||clear");
	
	if(c == 1){
		printFile();
		cout << "======================================" << endl;
		cout << "       DIGITE O NOME DO ARQUIVO       " << endl;
		cout << " EXTENSOES PERMITIDAS: '.dat' / '.csv'" << endl;
		cout << "======================================" << endl;
		cin >> nome;
		nome = "./registro/" + nome;

		exist = (stat(nome.c_str(), &sb) == 0);

		if ((nome.size() > 4 && nome.substr(nome.size() - 4) != ".csv") && (nome.size() > 4 && nome.substr(nome.size() - 4) != ".dat"))
		{
			exist = false;
		}

		while (!exist)
		{
			system("cls||clear");
			printFile();
			cout << "======================================" << endl;
			cout << "     REGISTRO AUSENTE OU INVALIDO     " << endl;
			cout << "    VERIFIQUE A PASTA DE REGISTROS	   " << endl;
			cout << "======================================" << endl;
			cin >> nome;
			nome = "./registro/" + nome;

			exist = (stat(nome.c_str(), &sb) == 0);

			if ((nome.size() > 4 && nome.substr(nome.size() - 4) != ".csv") && (nome.size() > 4 && nome.substr(nome.size() - 4) != ".dat"))
			{
				exist = false;
			}
		}
	} else {
		nome = "exit";
	}
}
// Handler da pasta
bool folderHandler(string &nome)
{
	bool path;
	struct stat sb;

	if (stat("./registro", &sb) != 0)
	{
		string dummy;

		printFile();
		cout << "======================================" << endl;
		cout << "      PASTA DE REGISTROS AUSENTE      " << endl;
		cout << "======================================" << endl;
		cout << "| Digite qualquer valor para sair.   |" << endl;
		cout << "--------------------------------------" << endl;

		cin >> dummy;
		path = false;

		return path;
	}
	else
	{
		path = true;
	}

	if (path)
	{
		getFileByName(nome);
	}

	system("cls||clear");

	return path;
}

// Tela Principal
void telaMain()
{
	printFile();
	cout << "======================================" << endl;
	cout << "          SELECIONE UMA OPCAO         " << endl;
	cout << "======================================" << endl;
	cout << "| 1. Adicionar em Posicao            |" << endl;
	cout << "--------------------------------------" << endl;
	cout << "| 2. Visualizar Registros            |" << endl;
	cout << "--------------------------------------" << endl;
	cout << "| 3. Modificar Registro              |" << endl;
	cout << "--------------------------------------" << endl;
	cout << "| 4. Trocar Posicao de Registro      |" << endl;
	cout << "--------------------------------------" << endl;
	cout << "| 5. Imprimir Registro Completo      |" << endl;
	cout << "--------------------------------------" << endl;
	cout << "| 6. Sair/Salvar                     |" << endl;
	cout << "--------------------------------------" << endl;
	int choice;
	cin >> choice;

	system("cls||clear");
	string name;
	string temp;
	int indexX;
	int indexY;

	switch (choice)
	{
	case 1:
		
		cout << "======================================" << endl;
		cout << "     DIGITE A POSICAO DE INSERCAO     " << endl;
		cout << "======================================" << endl;
		cin >> indexX;

		system("cls||clear");
		
		datInsert("registro.dat", indexX);

		break;
	case 2:
		
		cout << "======================================" << endl;
		cout << "       DIGITE AS POSICOES X E Y       " << endl;
		cout << "======================================" << endl;
		cin >> indexX;
		cin >> indexY;

		system("cls||clear");
		
		datPrint("registro.dat" ,indexX, indexY);

		break;
	case 3:
		cout << "======================================" << endl;
		cout << "      DIGITE A POSICAO DE EDICAO      " << endl;
		cout << "======================================" << endl;
		cin >> indexX;
		
		system("cls||clear");
		
		datChange("registro.dat", indexX);
		
		break;
	case 4:
		
		cout << "======================================" << endl;
		cout << "     DIGITE AS POSICOES DE TROCA      " << endl;
		cout << "             X   &   Y                " << endl;
		cout << "======================================" << endl;
		cin >> indexX;
		cin >> indexY;

		system("cls||clear");
		
		datSwap("registro.dat", indexX, indexY);

		break;
	case 5:
		system("cls||clear");
	
		//impressao
		datPrint("registro.dat");
		
		break;
	case 6:
		break;
	default:
		cout << "======================================" << endl;
		cout << "             OPCAO INVALIDA           " << endl;
		cout << "======================================" << endl;
		break;
	}
}
// Funcao para salvar as alteracoes
bool saveChanges()
{
	char choice;
		//printSave();
		cout << "======================================" << endl;
		cout << "      DESEJA SALVAR ALTERACOES?       " << endl;
		cout << "         S (SIM) | N (NAO)            " << endl;
		cout << "======================================" << endl;
		cin >> choice;
		system("cls||clear");
	return (choice == 'S' || choice == 's');
}
// Main
int main()
{
	// Vars
	string nomeArquivo;
	
	// Inicializando variaveis
	if (folderHandler(nomeArquivo)) {
		if(nomeArquivo != "exit")
			csvReadWrite("registro.csv", "registro.dat");
		
		// Loop de tela principal
		bool running = true;
		while (running)
		{
			telaMain();
			char continueChoice;

			printExit();
			cout << "======================================" << endl;
			cout << "           DESEJA CONTINUAR?          " << endl;
			cout << "      S (CONTINUA) | N (FINALIZA)     " << endl;
			cout << "======================================" << endl;
			cin >> continueChoice;
			system("cls||clear");
			if (continueChoice == 'N' || continueChoice == 'n')
			{
				running = false;
			}
		}
	}

	return 0;
}
