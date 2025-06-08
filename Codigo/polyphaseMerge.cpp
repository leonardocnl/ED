/*
 * Trabalho ED: Intercalacao Polifasica
 * Alexandre de Castro
 * Leonardo Carvalho
 * Gustavo Figueiredo
*/

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <sys/stat.h> // For stat() to get file size
#include <cmath>
#include <algorithm> // For sort and min/max
#include <cstdio>    // For remove()
#include <cstring>   // For strcpy()
#include <vector>    // For vector
#include <filesystem> // For filesystem::resize_file

using namespace std;

// Representa um registro de dados econômicos com layout fixo para eficiência de I/O
struct Dado {
    double dataValue;
    char seriesTitle1[80];
    char seriesTitle2[80];
    char seriesTitle3[80];
    char seriesTitle4[60];
    char seriesTitle5[60];
    char group[100];
    char subject[50];
    char seriesReference[20];
    char period[10];
    char status[10];
    char units[10];
    char magnitude;
    uint8_t isEndOfRun;

	//Valores 'default' de dado
    Dado() : dataValue(0.0), magnitude(0), isEndOfRun(0) {
        seriesTitle1[0] = '\0';
        seriesTitle2[0] = '\0';
        seriesTitle3[0] = '\0';
        seriesTitle4[0] = '\0';
        seriesTitle5[0] = '\0';
        group[0] = '\0';
        subject[0] = '\0';
        seriesReference[0] = '\0';
        period[0] = '\0';
        status[0] = '\0';
        units[0] = '\0';
    }


    // Implementa lógica de prioridade para ordenação Crescente: EOR tem maior prioridade que dados
    bool operator>(const Dado& other) const {
        if (this->isEndOfRun == 1 && other.isEndOfRun == 0) {
            return true;
        }
        if (this->isEndOfRun == 0 && other.isEndOfRun == 1) {
            return false;
        }
        return this->dataValue > other.dataValue;
    }

    bool operator<(const Dado& other) const {
        if (this->isEndOfRun == 0 && other.isEndOfRun == 1) {
            return true;
        }
        if (this->isEndOfRun == 1 && other.isEndOfRun == 0) {
            return false;
        }
        return this->dataValue < other.dataValue;
    }

    bool operator==(const Dado& other) const {
        if (this->isEndOfRun != other.isEndOfRun) {
            return false;
        }
        if (this->isEndOfRun == 1) {
            return true;
        }
        return this->dataValue == other.dataValue;
    }

    bool operator<=(const Dado& other) const {
        return (*this < other) || (*this == other);
    }
    
    bool operator>=(const Dado& other) const {
        return (*this > other) || (*this == other);
    }

    bool endMarker() const {
        return isEndOfRun == 1;
    }

    void setAsDataRecord() {
        isEndOfRun = 0;
    }
	
	//Define Marcador EOF
    void setAsEndOfRun() {
        isEndOfRun = 1;
        dataValue = 0.0;
        seriesTitle1[0] = '\0';
        seriesTitle2[0] = '\0';
        seriesTitle3[0] = '\0';
        seriesTitle4[0] = '\0';
        seriesTitle5[0] = '\0';
        group[0] = '\0';
        subject[0] = '\0';
        seriesReference[0] = '\0';
        period[0] = '\0';
        status[0] = '\0';
        units[0] = '\0';
        magnitude = 0;
    }
};

const long long DATA_SIZE = sizeof(Dado);
const int AVAILABLE_MEMORY_SIZE = 64 * 1024 * 1024;  // Limite conservador para sistemas com pouca RAM

// Lê múltiplos registros de uma vez para reduzir chamadas de sistema
int blockRead(ifstream& arquivo, Dado* buffer, size_t bufferSize) {
    arquivo.read(reinterpret_cast<char*>(buffer), bufferSize * DATA_SIZE);
    streamsize bytesRead = arquivo.gcount();
    int recordsRead = bytesRead / DATA_SIZE;
    if (arquivo.eof()) {
        cout << "blockRead - Stream EOF bit set." << endl;
    }
    if (arquivo.fail()) {
        cout << "blockRead - Stream FAIL bit set." << endl;
    }
    if (arquivo.bad()) {
        cout << "blockRead - Stream BAD bit set." << endl;
    }
    arquivo.clear();    
    return recordsRead;
}

void blockWrite(ofstream& arquivo, Dado* buffer, size_t numRegistros) {
    for (size_t i = 0; i < numRegistros; ++i) {
        arquivo.write((char*)&buffer[i], DATA_SIZE);
    }
}

// Abstrai uma "fita" do algoritmo de intercalação polifásica
struct PolyphaseTape {
    string filename;
    mutable fstream stream;
    long long actualRuns;    // Sequências reais de dados ordenados
    long long dummyRuns;     // Sequências fictícias para balanceamento Fibonacci

    PolyphaseTape(const string& fname) : filename(fname), actualRuns(0), dummyRuns(0) {}

    ~PolyphaseTape() {
        if (stream.is_open()) {
            stream.close();
        }
    }

    void rewindAndClearFlags() {
        if (stream.is_open()) {
            stream.clear();
            stream.seekg(0);
            stream.seekp(0);
        }
    }
    
    // Lê um registro completo ou falha - não permite leituras parciais
    bool readRecord(Dado& record) {
        if (!stream.is_open()) {
            cerr << "ERROR: Attempted to read from closed tape stream: " << filename << endl;
            return false;
        }
        streampos beforeReadPos = stream.tellg();
        stream.read(reinterpret_cast<char*>(&record), DATA_SIZE);
        streamsize bytesRead = stream.gcount();
        streampos afterReadPos = stream.tellg();
        if (bytesRead < DATA_SIZE) { 
            if (stream.eof()) {
                cout << "blockRead - Stream EOF bit set." << endl; 
            } else if (stream.fail() && !stream.eof()) {
                cerr << "ERROR: Read error on tape " << filename << " (gcount: " << bytesRead << ")" << endl;
            }
            stream.clear();
            return false;
        }
        return true;
    }

    bool writeRecord(const Dado& record) {
        if (!stream.is_open()) {
            cerr << "ERROR: Attempted to write to closed tape stream: " << filename << endl;
            return false;
        }
        if (!stream.write(reinterpret_cast<const char*>(&record), DATA_SIZE)) {
            cerr << "ERROR: Failed to write record to tape '" << filename << "'. Stream state: " << stream.rdstate() << " Error: " << strerror(errno) << endl;
            stream.clear();
            return false;
        }
        streampos afterWritePos = stream.tellp();
        return true;
    }
    
    // Fallback para sistemas sem filesystem
    long long getFileSize() const {
        if (filename.empty()) {
            return -1;
        }
        try {
            return filesystem::file_size(filename);
        } catch (const filesystem::filesystem_error& e) {
            if (stream.is_open()) {
                streampos currentPos = stream.tellg(); 
                stream.seekg(0, ios::end);
                long long size = stream.tellg();
                stream.seekg(currentPos);
                return size;
            }
            return -1;
        }
    }
};

// Gerencia leitura sequencial de uma fita durante intercalação
class MergeFileIn {
public:
    PolyphaseTape* tape;
    Dado currentReg;
    bool hasData;  // Indica se currentReg contém dados válidos da sequência atual

    MergeFileIn(PolyphaseTape* t) : tape(t), hasData(false) {}

    // Para na primeira marca EOR encontrada - define limite da sequência lógica
    bool loadNextDataRecord() {
        currentReg = Dado();
        hasData = false;
        Dado tempReg;
        bool readSuccess = tape->readRecord(tempReg);
        if (!readSuccess) {
            currentReg = Dado(); 
            return false; 
        }
        if (tempReg.isEndOfRun == 1) {
            currentReg = Dado(); 
            hasData = false;
            return false; 
        } else {
            currentReg = tempReg; 
            hasData = true;
            return true; 
        }
    }

    // Sequências dummy são conceituais - não existem fisicamente no arquivo
    void prepareForNewLogicalRun() {
        if (tape->actualRuns == 0 && tape->dummyRuns == 0) {
            tape->stream.clear();
            hasData = false;
            currentReg = Dado();
        } else if (tape->actualRuns > 0) {
            hasData = loadNextDataRecord();
            if (!hasData) {
                cerr << "WARNING: Expected actual run on tape " << tape->filename << " but could not load first data record (immediate EOR/EOF or corruption). At run " << " (phase loop 'i')" << endl;
            }
            tape->actualRuns--; 
        } else {
            hasData = false; 
            currentReg = Dado(); 
            tape->dummyRuns--; 
        }
    }

    bool advance() {
        if (!hasData) {
            return false;
        }
        hasData = loadNextDataRecord(); 
        return hasData; 
    }
    
    Dado& getCurrentRecord() {
        return currentReg;
    }

    bool hasRecords() const {
        return hasData;
    }

    bool isEndOfCurrentRun() const {
        return !hasData;
    }
};

// Calcula distribuição Fibonacci necessária para balancear intercalação polifásica
long long calculateInitialDummyRuns(long long totalActualRunsFromInputFile, long long fibTargets[3]) {
    if (totalActualRunsFromInputFile == 0) {
        fibTargets[0] = fibTargets[1] = fibTargets[2] = 0;
        cout << "Fibonacci targets: 0, 0, 0 (No actual runs)" << endl;
        return 0;
    }
    long long fibA = 0; 
    long long fibB = 1; 
    long long fibC = 1; 
    int kIndex = 2; 
    // Encontra o primeiro número Fibonacci >= total de sequências
    while (fibC < totalActualRunsFromInputFile) {
        long long nextFibC = fibB + fibC;
        fibA = fibB;
        fibB = fibC;
        fibC = nextFibC;
        kIndex++;
    }
    fibTargets[0] = fibC; 
    fibTargets[1] = fibB; 
    fibTargets[2] = fibA; 
    long long totalFibRunsAtCurrentLevel = fibTargets[0] + fibTargets[1] + fibTargets[2];
    cout << "calculateInitialDummyRuns called with total_actual_runs: " << totalActualRunsFromInputFile << endl;
    cout << "k_index (F_k): " << kIndex << endl;
    cout << "Calculated Fibonacci targets (t0,t1,t2): " << fibTargets[0] << ", "
              << fibTargets[1] << ", " << fibTargets[2] << endl;
    cout << "Total runs at target Fib level (sum of targets): " << totalFibRunsAtCurrentLevel << endl;
    long long numDummiesNeeded = totalFibRunsAtCurrentLevel - totalActualRunsFromInputFile;
    cout << "Total dummy runs needed for system: " << numDummiesNeeded << endl;
    return numDummiesNeeded;
}

void cleanTemp() {
    cout << "Cleaning temporary files..." << endl;
    if (remove("./registro/t0.dat") != 0 && errno != ENOENT) cerr << "Error removing t0.dat: " << strerror(errno) << endl;
    if (remove("./registro/t1.dat") != 0 && errno != ENOENT) cerr << "Error removing t1.dat: " << strerror(errno) << endl;
    if (remove("./registro/t2.dat") != 0 && errno != ENOENT) cerr << "Error removing t2.dat: " << strerror(errno) << endl;
    if (remove("./registro/temp_initial_runs.dat") != 0 && errno != ENOENT) cerr << "Error removing t2.dat: " << strerror(errno) << endl;
    cout << "Temporary files cleaned." << endl;
}

// Inicializa estruturas de dados e arquivos auxiliares para intercalação polifásica
bool inicializarTapesAuxiliares(PolyphaseTape* tapes[], const string nomesArquivos[], int numArquivos) {
    for(int i = 0; i < numArquivos; ++i) {
        tapes[i] = new PolyphaseTape(nomesArquivos[i]);
        if (remove(tapes[i]->filename.c_str()) != 0 && errno != ENOENT) {
            cerr << "WARNING: Could not remove old auxiliary file (might not exist): " << tapes[i]->filename << ": " << strerror(errno) << endl;
        }
    }
    return true;
}

// Valida se há memória suficiente para o buffer de ordenação
bool validarBufferMemoria(int tamanhoMemoriaDisponivelBytes, size_t& capacidadeBufferRegistros) {
    capacidadeBufferRegistros = tamanhoMemoriaDisponivelBytes / DATA_SIZE;
    cout << "Calculated capacidadeBufferRegistros: " << capacidadeBufferRegistros << endl;
    
    if (capacidadeBufferRegistros == 0) {
        cerr << "Error: Buffer size too small for one record. Must be at least " << DATA_SIZE << " bytes." << endl;
        return false;
    }
    return true;
}

// Valida arquivo de entrada e conta registros totais
bool validarArquivoEntrada(string arquivoEntrada, ifstream& entradaOriginal, long long& totalRecords) {
    entradaOriginal.open(arquivoEntrada, ios::in | ios::binary);
    
    if (!entradaOriginal.is_open()) {
        cerr << "Error opening input file: " << arquivoEntrada << endl;
        return false;
    }
    
    entradaOriginal.seekg(0, ios::end);
    totalRecords = entradaOriginal.tellg() / DATA_SIZE; 
    entradaOriginal.clear(); 
    entradaOriginal.seekg(0); 
    cout << "Total records in input file: " << totalRecords << endl;
    
    if (totalRecords == 0) {
        cout << "Input file is empty. Nothing to sort." << endl;
        entradaOriginal.close();
        return false;
    }
    
    return true;
}

// Cria sequências ordenadas iniciais a partir do arquivo de entrada
long long criarSequenciasIniciais(ifstream& entradaOriginal, const string& tempFilename, 
                                 Dado* bufferMemoria, size_t capacidadeBuffer, int mode) {
    ofstream tempInitialRunsStream;
    tempInitialRunsStream.open(tempFilename, ios::out | ios::binary);
    
    if (!tempInitialRunsStream.is_open()) {
        cerr << "FATAL ERROR: Could not open temporary initial runs file: " << tempFilename << endl;
        return -1;
    }
    
    long long totalActualRunsPhysicallyCreated = 0; 
    cout << "Starting initial run creation loop (Phase 1: Writing to temp file)..." << endl;

    // Fase 1: Cria sequências ordenadas em ordem decrescente que cabem na memória
    bool continueReading = true;
    while (continueReading) {
        int lidos = blockRead(entradaOriginal, bufferMemoria, capacidadeBuffer);
        if (lidos == 0) {
            cout << "End of input file reached or no more records to read for initial runs." << endl;
            continueReading = false;
        } else {
			if(mode == 1){
				sort(bufferMemoria, bufferMemoria + lidos);
			} else {
				sort(bufferMemoria, bufferMemoria + lidos, greater<Dado>());
			}
            tempInitialRunsStream.write(reinterpret_cast<char*>(bufferMemoria), lidos * DATA_SIZE);
            Dado eorMarker;
            eorMarker.setAsEndOfRun(); 
            tempInitialRunsStream.write(reinterpret_cast<char*>(&eorMarker), DATA_SIZE);
            totalActualRunsPhysicallyCreated++; 
        }
    }

    entradaOriginal.close(); 
    tempInitialRunsStream.close(); 
    cout << "Finished Phase 1. Total " << totalActualRunsPhysicallyCreated << " runs created in temp file." << endl;
    
    return totalActualRunsPhysicallyCreated;
}

// Libera recursos alocados durante o processo de intercalação
void liberarRecursos(Dado* bufferMemoria, PolyphaseTape* tapes[], int numArquivos) {
    if (bufferMemoria) {
        delete[] bufferMemoria;
    }
    for(int i = 0; i < numArquivos; ++i) {
        delete tapes[i];
    }
}

// Distribui sequências iniciais nos arquivos auxiliares seguindo padrão Fibonacci
bool distribuirSequenciasFibonacci(const string& tempFilename, PolyphaseTape* tapes[], 
                                  long long totalActualRuns, int numArquivos) {
    // Calcula distribuição Fibonacci para balanceamento
    long long fibTargets[3];
    calculateInitialDummyRuns(totalActualRuns, fibTargets);
    
    // Abre arquivo temporário para leitura
    ifstream tempInitialRunsReaderStream(tempFilename, ios::in | ios::binary);
    if (!tempInitialRunsReaderStream.is_open()) {
        cerr << "ERRO FATAL: Nao foi possivel abrir o arquivo temporario de runs iniciais para leitura (fase de distribuicao)." << endl;
        return false;
    }

    // Abre fitas auxiliares para escrita
    ofstream distribTapesWriteStreams[3];
    for (int i = 0; i < numArquivos; ++i) {
        distribTapesWriteStreams[i].open(tapes[i]->filename, ios::out | ios::binary | ios::trunc);
        if (!distribTapesWriteStreams[i].is_open()) {
            cerr << "ERRO FATAL: Nao foi possivel abrir a fita de distribuicao " << i << " para escrita" << endl;
            tempInitialRunsReaderStream.close();
            return false;
        }
        tapes[i]->actualRuns = 0;
        tapes[i]->dummyRuns = 0;
    }

    // Distribui runs entre as duas primeiras fitas
    int inputTape1Idx = 0;
    int inputTape2Idx = 1;
    
    long long runsForTape0 = fibTargets[1];
    long long runsForTape1 = totalActualRuns - runsForTape0;
    
    runsForTape0 = max(0LL, runsForTape0);
    runsForTape1 = max(0LL, runsForTape1);
    
    cout << "Iniciando Fase 2: Distribuindo runs do arquivo temp para fitas auxiliares..." << endl;
    cout << "Runs determinadas para distribuir: Fita " << inputTape1Idx << " recebe " << runsForTape0
         << " runs, Fita " << inputTape2Idx << " recebe " << runsForTape1 << " runs." << endl;

    // Distribui as runs
    for (int i = 0; i < totalActualRuns; ++i) {
        vector<Dado> currentRunData;
        Dado record;

        // Lê uma run completa
        while (tempInitialRunsReaderStream.read((char*)&record, DATA_SIZE) && record.isEndOfRun != 1) {
            streamsize bytesRead = tempInitialRunsReaderStream.gcount();
            if (bytesRead < DATA_SIZE) {
                tempInitialRunsReaderStream.clear();
            } else if (record.isEndOfRun != 1) {
                currentRunData.push_back(record);
            }
        }

        // Decide para qual fita enviar esta run
        int targetTapeForThisRun;
        if (tapes[inputTape1Idx]->actualRuns < runsForTape0) {
            targetTapeForThisRun = inputTape1Idx;
        } else {
            targetTapeForThisRun = inputTape2Idx;
        }

        // Escreve os dados da run na fita de destino
        if (!currentRunData.empty()) {
            distribTapesWriteStreams[targetTapeForThisRun].write((char*)currentRunData.data(), currentRunData.size() * DATA_SIZE);
        }

        // Escreve marcador EOR
        Dado eorMarkerDist;
        eorMarkerDist.setAsEndOfRun();
        distribTapesWriteStreams[targetTapeForThisRun].write((char*)&eorMarkerDist, DATA_SIZE);

        tapes[targetTapeForThisRun]->actualRuns++;
    }

    // Fecha arquivos
    tempInitialRunsReaderStream.close();
    remove(tempFilename.c_str());

    for (int i = 0; i < numArquivos; ++i) {
        if (distribTapesWriteStreams[i].is_open()) {
            distribTapesWriteStreams[i].close();
        }
    }

    // Calcula runs dummy finais
    tapes[0]->dummyRuns = fibTargets[0] - tapes[0]->actualRuns;
    tapes[1]->dummyRuns = fibTargets[1] - tapes[1]->actualRuns;
    tapes[2]->dummyRuns = fibTargets[2] - tapes[2]->actualRuns;

    tapes[0]->dummyRuns = max(0LL, tapes[0]->dummyRuns);
    tapes[1]->dummyRuns = max(0LL, tapes[1]->dummyRuns);
    tapes[2]->dummyRuns = max(0LL, tapes[2]->dummyRuns);

    cout << "Runs iniciais criadas e distribuidas. Runs reais (tapes[i]->actualRuns): "
         << tapes[0]->actualRuns << ", " << tapes[1]->actualRuns << ", " << tapes[2]->actualRuns << endl;
    cout << "Runs dummy iniciais (tapes[i]->dummyRuns): "
         << tapes[0]->dummyRuns << ", " << tapes[1]->dummyRuns << ", " << tapes[2]->dummyRuns << endl;
    
    return true;
}

// Executa uma fase de intercalação entre as fitas
bool executarFaseIntercalacao(PolyphaseTape* tapes[], int& inputTapeIdx0, int& inputTapeIdx1, 
                             int& outputTapeIdx, int& prevOutputTapeIdx, int faseNumero, int mode) {
    // Verifica se há sequências para intercalar
    long long runsToMergeThisPass = min(
        tapes[inputTapeIdx0]->actualRuns + tapes[inputTapeIdx0]->dummyRuns,
        tapes[inputTapeIdx1]->actualRuns + tapes[inputTapeIdx1]->dummyRuns
    );

    if (runsToMergeThisPass == 0) {
        cerr << "ERRO GRAVE: runs_to_merge_this_pass e 0, mas a ordenacao nao esta completa." << endl;
        return false;
    }

    // Gerencia as posições da fita e as flags para esta fase
    tapes[outputTapeIdx]->rewindAndClearFlags();

    // Fita de Entrada 0:
    if (inputTapeIdx0 == prevOutputTapeIdx) {
        tapes[inputTapeIdx0]->rewindAndClearFlags();
    } else {
        tapes[inputTapeIdx0]->stream.clear();
    }

    // Fita de Entrada 1:
    if (inputTapeIdx1 == prevOutputTapeIdx) {
        tapes[inputTapeIdx1]->rewindAndClearFlags();
    } else {
        tapes[inputTapeIdx1]->stream.clear();
    }

    prevOutputTapeIdx = outputTapeIdx;

    tapes[outputTapeIdx]->actualRuns = 0;
    tapes[outputTapeIdx]->dummyRuns = 0;

    cout << "\nInicio da Fase " << faseNumero << "." << endl;
    cout << "  Fitas de Entrada: t" << inputTapeIdx0 << " (R:" << tapes[inputTapeIdx0]->actualRuns 
         << ",D:" << tapes[inputTapeIdx0]->dummyRuns << ")"
         << ", t" << inputTapeIdx1 << " (R:" << tapes[inputTapeIdx1]->actualRuns 
         << ",D:" << tapes[inputTapeIdx1]->dummyRuns << ")" << endl;
    cout << "  Fita de Saida: t" << outputTapeIdx << endl;
    cout << "  Runs para intercalar nesta passagem (logico): " << runsToMergeThisPass << endl;

    MergeFileIn* entradasMerge[2];
    entradasMerge[0] = new MergeFileIn(tapes[inputTapeIdx0]);
    entradasMerge[1] = new MergeFileIn(tapes[inputTapeIdx1]);

    long long actualRunsProducedThisPhase = 0;
    long long dummyRunsProducedThisPhase = 0;

    for (long long i = 0; i < runsToMergeThisPass; ++i) {
        bool thisOutputRunIsActual = false;

        if (tapes[inputTapeIdx0]->actualRuns > 0) {
            entradasMerge[0]->prepareForNewLogicalRun();
            if (entradasMerge[0]->hasRecords()) {
                thisOutputRunIsActual = true;
            }
        } else if (tapes[inputTapeIdx0]->dummyRuns > 0) {
            entradasMerge[0]->prepareForNewLogicalRun();
        }
        
        if (tapes[inputTapeIdx1]->actualRuns > 0) {
            entradasMerge[1]->prepareForNewLogicalRun();
            if (entradasMerge[1]->hasRecords()) {
                thisOutputRunIsActual = true;
            }
        } else if (tapes[inputTapeIdx1]->dummyRuns > 0) {
            entradasMerge[1]->prepareForNewLogicalRun();
        }

        // Intercala registros de ambas as fitas 
		while (entradasMerge[0]->hasRecords() || entradasMerge[1]->hasRecords()) {
			Dado recordToWrite;
			bool recordTaken = false;

			if (entradasMerge[0]->hasRecords() && entradasMerge[1]->hasRecords()) {
				if (mode == 1) { // Ascendente
					if (entradasMerge[0]->getCurrentRecord() <= entradasMerge[1]->getCurrentRecord()) {
						recordToWrite = entradasMerge[0]->getCurrentRecord();
						recordTaken = true;
						entradasMerge[0]->advance();
					} else {
						recordToWrite = entradasMerge[1]->getCurrentRecord();
						recordTaken = true;
						entradasMerge[1]->advance();
					}
				} else { // Descendente
					if (entradasMerge[0]->getCurrentRecord() >= entradasMerge[1]->getCurrentRecord()) {
						recordToWrite = entradasMerge[0]->getCurrentRecord();
						recordTaken = true;
						entradasMerge[0]->advance();
					} else {
						recordToWrite = entradasMerge[1]->getCurrentRecord();
						recordTaken = true;
						entradasMerge[1]->advance();
					}
				}
			} else if (entradasMerge[0]->hasRecords()) {
				recordToWrite = entradasMerge[0]->getCurrentRecord();
				recordTaken = true;
				entradasMerge[0]->advance();
			} else if (entradasMerge[1]->hasRecords()) {
				recordToWrite = entradasMerge[1]->getCurrentRecord();
				recordTaken = true;
				entradasMerge[1]->advance();
			}

			if (recordTaken) {
				if (!recordToWrite.endMarker()) {
					tapes[outputTapeIdx]->writeRecord(recordToWrite);
				}
			}
		}

        if (thisOutputRunIsActual) {
            Dado eorMarker;
            eorMarker.setAsEndOfRun();
            tapes[outputTapeIdx]->writeRecord(eorMarker);
            actualRunsProducedThisPhase++;
        } else {
            dummyRunsProducedThisPhase++;
        }
    }

    delete entradasMerge[0];
    delete entradasMerge[1];

    // Trunca arquivo de saída
    try {
        long long currentWritePos = tapes[outputTapeIdx]->stream.tellp();
        filesystem::resize_file(tapes[outputTapeIdx]->filename, currentWritePos);
        cout << "Fita de saida " << outputTapeIdx << " truncada para " << currentWritePos << " bytes." << endl;
    } catch (const filesystem::filesystem_error& e) {
        cerr << "Falha ao truncar fita de saida " << outputTapeIdx << ": " << e.what() << endl;
    }

    tapes[outputTapeIdx]->actualRuns = actualRunsProducedThisPhase;
    tapes[outputTapeIdx]->dummyRuns = dummyRunsProducedThisPhase;

    // Determina próxima configuração de fitas
    int exhaustedInputTapeIdx = -1;
    if (tapes[inputTapeIdx0]->actualRuns == 0 && tapes[inputTapeIdx0]->dummyRuns == 0) {
        exhaustedInputTapeIdx = inputTapeIdx0;
    } else if (tapes[inputTapeIdx1]->actualRuns == 0 && tapes[inputTapeIdx1]->dummyRuns == 0) {
        exhaustedInputTapeIdx = inputTapeIdx1;
    }

    if (exhaustedInputTapeIdx == -1) {
        cerr << "ERRO GRAVE: Nenhuma fita de entrada foi esgotada apos a fase de intercalacao." << endl;
        return false;
    }

    // Encontra próxima fita de entrada disponível
    int nextInputTapeIdx = -1;
    for (int i = 0; i < 3; ++i) {
        if (i != exhaustedInputTapeIdx && i != prevOutputTapeIdx) {
            nextInputTapeIdx = i;
            break;
        }
    }

    inputTapeIdx0 = nextInputTapeIdx;
    inputTapeIdx1 = prevOutputTapeIdx;
    outputTapeIdx = exhaustedInputTapeIdx;

    cout << "Fim da Fase " << faseNumero << ". Configuracao da proxima fase: Fitas de Entrada: t" 
         << inputTapeIdx0 << ", t" << inputTapeIdx1 << "; Fita de Saida: t" << outputTapeIdx << endl;

    return true;
}

// Executa todas as fases de intercalação até obter arquivo final
bool executarIntercalacaoCompleta(PolyphaseTape* tapes[], int numArquivos, int& outputTapeIdxFinal, int mode) {
    // Abre todas as fitas como fstream para as fases de intercalação
    for(int i = 0; i < numArquivos; ++i) {
        tapes[i]->stream.open(tapes[i]->filename, ios::in | ios::out | ios::binary);
        if (!tapes[i]->stream.is_open()) {
            cerr << "ERRO FATAL: Nao foi possivel abrir a fita " << i << " para leitura/escrita nas fases de intercalacao" << endl;
            return false;
        }
        tapes[i]->rewindAndClearFlags();
    }

    cout << "\nIniciando fases de intercalacao..." << endl;

    int inputTapeIdx[2];
    int outputTapeIdx;
    int prevOutputTapeIdx = 2; // Para a primeira fase de intercalação, t2 é a saída inicial.

    inputTapeIdx[0] = 0;
    inputTapeIdx[1] = 1;
    outputTapeIdx = 2;

    int faseNum = 0;
    bool endMerge = false;

    while (!endMerge) {
        faseNum++;

        int tapesWithActualDataRunsCount = 0;
        int tapeHoldingFinalResultIdx = -1;

        for (int i = 0; i < numArquivos; ++i) {
            if (tapes[i]->actualRuns > 0) {
                tapesWithActualDataRunsCount++;
                tapeHoldingFinalResultIdx = i;
            }
        }

        if (tapesWithActualDataRunsCount == 1 &&
            tapes[tapeHoldingFinalResultIdx]->actualRuns == 1 &&
            tapes[tapeHoldingFinalResultIdx]->dummyRuns == 0) {

            bool allOthersEmpty = true;
            for (int i = 0; i < numArquivos; ++i) {
                if (i != tapeHoldingFinalResultIdx) {
                    if (tapes[i]->actualRuns != 0 || tapes[i]->dummyRuns != 0) {
                        allOthersEmpty = false;
                        break;
                    }
                }
            }

            if (allOthersEmpty) {
                outputTapeIdx = tapeHoldingFinalResultIdx;
                outputTapeIdxFinal = outputTapeIdx;
                endMerge = true;
            }
        }

        if (!endMerge) {
            if (tapesWithActualDataRunsCount == 0) {
                cerr << "ERRO GRAVE: Todas as runs reais perdidas durante a intercalacao! Saindo do loop." << endl;
                return false;
            }

            if (!executarFaseIntercalacao(tapes, inputTapeIdx[0], inputTapeIdx[1], 
                                        outputTapeIdx, prevOutputTapeIdx, faseNum, mode)) {
                return false;
            }

            // Proteção contra loop infinito
            if (faseNum > 100) {
                cerr << "ERROR: Too many merge phases. Possible infinite loop." << endl;
                return false;
            }
        }
    }

    cout << "Intercalacao polifasica completa apos " << faseNum << " fases." << endl;
    return true;
}

// Copia arquivo final ordenado para destino especificado
bool finalizarArquivoOrdenado(PolyphaseTape* tapes[], string arquivoSaida, int outputTapeIdxFinal) {
    cout << "\nIniciando copia final para o arquivo de saida: " << arquivoSaida << endl;
    ofstream saidaFinal;
    saidaFinal.open(arquivoSaida, ios::out | ios::binary | ios::trunc);

    if (!saidaFinal.is_open()) {
        cerr << "Erro ao abrir o arquivo de saida final: " << arquivoSaida << endl;
        return false;
    }

    PolyphaseTape* finalSortedTape = tapes[outputTapeIdxFinal];
    finalSortedTape->rewindAndClearFlags();

    long long recordsCopied = 0;
    Dado tempDado;

    // Lê e copia os registros exceto EOR
    while(finalSortedTape->stream.read((char*)&tempDado, DATA_SIZE)) {
        if (tempDado.isEndOfRun == 0) {
            saidaFinal.write(reinterpret_cast<char*>(&tempDado), DATA_SIZE);
            recordsCopied++;
        }
    }

    saidaFinal.close();

    cout << "Total de registros copiados para a saida final: " << recordsCopied << endl;
    cout << "Intercalacao polifasica concluida com sucesso." << endl;
    
    return true;
}

// Implementa algoritmo de intercalação polifásica para ordenação externa
// Divide o processo em etapas bem definidas usando funções auxiliares
void polyphaseMerge(string arquivoEntrada, string arquivoSaida, int tamanhoMemoriaDisponivelBytes, int mode) {
    const int NUM_ARQUIVOS_AUXILIARES = 3;  // Intercalação 3-way
    string nomesArquivosAuxiliares[NUM_ARQUIVOS_AUXILIARES];
    nomesArquivosAuxiliares[0] = "./registro/t0.dat";
    nomesArquivosAuxiliares[1] = "./registro/t1.dat";
    nomesArquivosAuxiliares[2] = "./registro/t2.dat";
    
    // Variáveis principais do algoritmo
    long long initialTotalActualRuns = 0; 
    string tempInitialRunsFilename = "./registro/temp_initial_runs.dat";
    Dado* bufferMemoria = nullptr; 
    PolyphaseTape* tapes[NUM_ARQUIVOS_AUXILIARES];

    // Etapa 1: Inicializar estruturas auxiliares
    if (!inicializarTapesAuxiliares(tapes, nomesArquivosAuxiliares, NUM_ARQUIVOS_AUXILIARES)) {
        liberarRecursos(bufferMemoria, tapes, NUM_ARQUIVOS_AUXILIARES);
        return;
    }

    // Etapa 2: Validar capacidade de memória
    size_t capacidadeBufferRegistros;
    if (!validarBufferMemoria(tamanhoMemoriaDisponivelBytes, capacidadeBufferRegistros)) {
        liberarRecursos(bufferMemoria, tapes, NUM_ARQUIVOS_AUXILIARES);
        return;
    }
    
    bufferMemoria = new Dado[capacidadeBufferRegistros];
    cout << "Phase 1: Creating initial runs and distributing for Polyphase Merge..." << endl;
    
    // Etapa 3: Validar arquivo de entrada
    ifstream entradaOriginal;
    long long totalRecordsInSourceFile;
    if (!validarArquivoEntrada(arquivoEntrada, entradaOriginal, totalRecordsInSourceFile)) {
        liberarRecursos(bufferMemoria, tapes, NUM_ARQUIVOS_AUXILIARES);
        cleanTemp();
        return;
    }
    
    // Etapa 4: Criar sequências ordenadas iniciais
    initialTotalActualRuns = criarSequenciasIniciais(entradaOriginal, tempInitialRunsFilename, 
                                                    bufferMemoria, capacidadeBufferRegistros, mode);
    
    if (initialTotalActualRuns == -1) {
        liberarRecursos(bufferMemoria, tapes, NUM_ARQUIVOS_AUXILIARES);
        return;
    }
    
    cout << "Phase 2: Distributing runs using Fibonacci pattern..." << endl;
    
    // Etapa 5: Distribuir sequências usando padrão Fibonacci
    if (!distribuirSequenciasFibonacci(tempInitialRunsFilename, tapes, 
                                      initialTotalActualRuns, NUM_ARQUIVOS_AUXILIARES)) {
        cerr << "ERROR: Failed to distribute runs using Fibonacci pattern." << endl;
        liberarRecursos(bufferMemoria, tapes, NUM_ARQUIVOS_AUXILIARES);
        return;
    }
    
    cout << "Phase 3: Executing polyphase merge..." << endl;
    
    // Etapa 6: Executar intercalação completa
    int outputTapeIdxFinal = 0;
    if (!executarIntercalacaoCompleta(tapes, NUM_ARQUIVOS_AUXILIARES, outputTapeIdxFinal, mode)) {
        cerr << "ERROR: Polyphase merge execution failed." << endl;
        liberarRecursos(bufferMemoria, tapes, NUM_ARQUIVOS_AUXILIARES);
        return;
    }
    
    cout << "Phase 4: Finalizing sorted output file..." << endl;
    
    // Etapa 7: Finalizar arquivo ordenado
    if (!finalizarArquivoOrdenado(tapes, arquivoSaida, outputTapeIdxFinal)) {
        cerr << "ERROR: Failed to create final sorted file." << endl;
        liberarRecursos(bufferMemoria, tapes, NUM_ARQUIVOS_AUXILIARES);
        return;
    }
    
    cout << "Polyphase merge completed successfully with " << initialTotalActualRuns 
         << " initial runs processed." << endl;
    
    // Limpeza final
    liberarRecursos(bufferMemoria, tapes, NUM_ARQUIVOS_AUXILIARES);
    cleanTemp();
}


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

// Função auxiliar para fazer parsing CSV seguro (sem usar vector)
int parseCSVLine(const string& line, string fields[], int max_fields) {
    int field_count = 0;
    string current_field = "";
    bool inside_quotes = false;
    
    for (size_t i = 0; i < line.length() && field_count < max_fields; i++) {
        char c = line[i];
        
        if (c == '"') {
            // Toggle estado das aspas
            inside_quotes = !inside_quotes;
            // Não incluir as aspas no campo final
        } else if (c == ',' && !inside_quotes) {
            // Vírgula fora de aspas = separador de campo
            fields[field_count] = current_field;
            field_count++;
            current_field = "";
        } else {
            // Qualquer outro caractere (incluindo vírgulas dentro de aspas)
            current_field += c;
        }
    }
    
    // Adicionar o último campo se ainda há espaço
    if (field_count < max_fields) {
        fields[field_count] = current_field;
        field_count++;
    }
    
    return field_count;
}

// Get de arquivo csv
void csvReadWrite(string fileName, string registerName) {
	printFile();
	cout << "======================================" << endl;
	cout << "          LEITURA ARQUIVO CSV         " << endl;
	cout << "======================================" << endl << endl;
	
	int cont = 0;
	ifstream openFile(fileName);
	ofstream saveFile;
	saveFile.open(registerName, ios::out | ios::binary);
	
	if (!openFile.is_open()) {
		cerr << "Erro ao abrir arquivo CSV: " << fileName << endl;
		return;
	}
	
	if (!saveFile.is_open()) {
		cerr << "Erro ao criar arquivo binario: " << registerName << endl;
		openFile.close();
		return;
	}
	
	string line;
	long long records_written = 0;
	long long problematic_lines = 0;
	long long total_lines_processed = 0;

	// Pular cabeçalho
	getline(openFile, line);

	//Pega Linha e Salva Binario
	while (getline(openFile, line))
	{
		total_lines_processed++;
		
		// Pular linhas vazias
		if (line.empty()) {
			continue;
		}
		
		// Fazer parsing CSV adequado (agora trata aspas corretamente)
		string fields[13];
		int field_count = parseCSVLine(line, fields, 13);
		
		// Verificar se temos o número correto de campos
		if (field_count != 13) {
			problematic_lines++;
			// Mostrar apenas as primeiras 10 linhas problemáticas para debug
			if (problematic_lines <= 10) {
				cout << "Linha com " << field_count << " campos (esperados=13), linha " << total_lines_processed + 1 << ": " << line.substr(0, 50) << "..." << endl;
			}
			// IMPORTANTE: Não ignorar a linha, tentar processar mesmo assim
			// Ajustar o número de campos conforme necessário
			if (field_count < 13) {
				// Se há menos campos, preencher com strings vazias
				for (int i = field_count; i < 13; ++i) {
					fields[i] = "";
				}
			}
			// Se há mais campos, simplesmente usar apenas os primeiros 13
			// (não precisamos fazer nada especial, já temos fields[0] a fields[12])
		}
		
		// Inicializar completamente o registro para evitar lixo de memória
		Dado registro = {};

		// Copiar campos na ordem correta (garantir que não exceda limites dos arrays)
		strncpy(registro.seriesReference, fields[0].c_str(), sizeof(registro.seriesReference) - 1);
		registro.seriesReference[sizeof(registro.seriesReference) - 1] = '\0';
		
		strncpy(registro.period, fields[1].c_str(), sizeof(registro.period) - 1);
		registro.period[sizeof(registro.period) - 1] = '\0';

		// Data value com tratamento de erro
		if(!fields[2].empty()){
			try {
				registro.dataValue = stod(fields[2]);
			} catch (const exception& e) {
				registro.dataValue = 0.0; // Valor padrão se conversão falhar
			}
		} else {
			registro.dataValue = 0.0; // Valor padrão para campos vazios
		}
		
		strncpy(registro.status, fields[3].c_str(), sizeof(registro.status) - 1);
		registro.status[sizeof(registro.status) - 1] = '\0';
		
		strncpy(registro.units, fields[4].c_str(), sizeof(registro.units) - 1);
		registro.units[sizeof(registro.units) - 1] = '\0';
		
		if (!fields[5].empty()) {
			registro.magnitude = fields[5].at(0);
		} else {
			registro.magnitude = ' '; // Espaço como valor padrão
		}
		
		strncpy(registro.subject, fields[6].c_str(), sizeof(registro.subject) - 1);
		registro.subject[sizeof(registro.subject) - 1] = '\0';
		
		strncpy(registro.group, fields[7].c_str(), sizeof(registro.group) - 1);
		registro.group[sizeof(registro.group) - 1] = '\0';
		
		strncpy(registro.seriesTitle1, fields[8].c_str(), sizeof(registro.seriesTitle1) - 1);
		registro.seriesTitle1[sizeof(registro.seriesTitle1) - 1] = '\0';
		
		strncpy(registro.seriesTitle2, fields[9].c_str(), sizeof(registro.seriesTitle2) - 1);
		registro.seriesTitle2[sizeof(registro.seriesTitle2) - 1] = '\0';
		
		strncpy(registro.seriesTitle3, fields[10].c_str(), sizeof(registro.seriesTitle3) - 1);
		registro.seriesTitle3[sizeof(registro.seriesTitle3) - 1] = '\0';
		
		strncpy(registro.seriesTitle4, fields[11].c_str(), sizeof(registro.seriesTitle4) - 1);
		registro.seriesTitle4[sizeof(registro.seriesTitle4) - 1] = '\0';
		
		strncpy(registro.seriesTitle5, fields[12].c_str(), sizeof(registro.seriesTitle5) - 1);
		registro.seriesTitle5[sizeof(registro.seriesTitle5) - 1] = '\0';
		
		registro.isEndOfRun = 0;
		
		cont++;
		
		saveFile.write((char*)&registro, DATA_SIZE);
		records_written++;
	}
	
	// After the loop in converterCsvToBinary:
	cout << "CSV conversion complete. Final binary file size: " << saveFile.tellp() << " bytes." << endl;

	openFile.close();
	saveFile.close();
	
	cout << "Total de linhas processadas: " << total_lines_processed << endl;
	cout << "Registros convertidos do CSV: " << records_written << endl;
	cout << "Linhas com problemas de formatacao (corrigidas): " << problematic_lines << endl;
	
	// Calcular estatísticas
	double success_rate = (double)records_written / total_lines_processed * 100.0;
	cout << "Taxa de sucesso: " << success_rate << "%" << endl;
}

void printDado(Dado registro){
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
void datPrint(string name) {
	string exit;
	fstream r;
	r.open(name, ios::in | ios::binary);

	ifstream fileSize(name, ios::binary | ios::ate);
	int size = DATA_SIZE;
	size = fileSize.tellg() / size;
	fileSize.close();

	Dado registro;
	
	cout << "======================================" << endl;
	cout << "           DESEJA IMPRIMIR?           " << endl << endl;
	cout << size << " Registros." << endl;
	cout << "======================================" << endl;
	cout << "| 1. Imprimir      | 2.Voltar        |" << endl;
	cout << "--------------------------------------" << endl;	
	
	cin >> exit;

	if(exit == "1"){
		

		if (r.is_open())
		{
			while (r.read((char*)&registro, DATA_SIZE))
			{
				printDado(registro);
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
void datPrint(string name, int x, int y) {
	fstream r;
	r.open(name, ios::in | ios::binary);
	
	int size = DATA_SIZE;
	int start = size * x;

	ifstream fileSize(name, ios::binary | ios::ate);
	
	size = fileSize.tellg() / size;
	fileSize.close();
	
	Dado registro;
	r.seekg(start, ios::beg);

	if (r.is_open())
	{
		while (x <= y && r.read((char*)&registro, DATA_SIZE))
		{
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

void datSwap(string name, int x, int y) {
	fstream r;
	r.open(name, ios::in | ios::out | ios::binary);
	
	int size = DATA_SIZE;
	int xPos = size * x;
	int yPos = size * y;

	ifstream fileSize(name, ios::binary | ios::ate);
	
	size = fileSize.tellg() / size;
	fileSize.close();
	
	Dado registroX;
	Dado registroY;

	if (r.is_open() &&( x < size && x >= 0) && (y < size && y >= 0))
	{
		r.seekg(xPos, ios::beg);
		r.read((char*)&registroX, DATA_SIZE);
		cout << "REGISTRO X: " << endl;
		printDado(registroX);
		
		r.seekg(yPos, ios::beg);
		r.read((char*)&registroY, DATA_SIZE);
		cout << "REGISTRO Y: " << endl;
		printDado(registroY);
		
		r.seekg(xPos, ios::beg);
		r.write((char*)&registroY, DATA_SIZE);
		
		r.seekg(yPos, ios::beg);
		r.write((char*)&registroX, DATA_SIZE);

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

void datInsert(string name, int x) {
	string tempFileName = "temp.dat";
    int size = DATA_SIZE;
    
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
	
	Dado registro;

	if (inputFile.is_open() &&( x < size && x >= 0) && tempFile.is_open())
	{
		string temp;
		
		inputFile.seekg(xPos, ios::beg);
		
		while(inputFile.read((char*)&registro, DATA_SIZE))
		{
			tempFile.write((char*)&registro, DATA_SIZE);
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
		strncpy(registro.seriesReference, temp.c_str(), sizeof(registro.seriesReference) - 1);
		registro.seriesReference[sizeof(registro.seriesReference) - 1] = '\0';
		
		cout << "Period:            ";
		getline(cin, temp);
		strncpy(registro.period, temp.c_str(), sizeof(registro.period) - 1);
		registro.period[sizeof(registro.period) - 1] = '\0';
		
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
		strncpy(registro.status, temp.c_str(), sizeof(registro.status) - 1);
		registro.status[sizeof(registro.status) - 1] = '\0';
		
		cout << "Units:             ";
		getline(cin, temp);
		strncpy(registro.units, temp.c_str(), sizeof(registro.units) - 1);
		registro.units[sizeof(registro.units) - 1] = '\0';
		
		cout << "Magnitude:         ";
		cin >> registro.magnitude;
		
		cout << "Subject:           ";
		cin.ignore();
		getline(cin, temp);
		strncpy(registro.subject, temp.c_str(), sizeof(registro.subject) - 1);
		registro.subject[sizeof(registro.subject) - 1] = '\0';
		
		cout << "Group:             ";
		getline(cin, temp);
		strncpy(registro.group, temp.c_str(), sizeof(registro.group) - 1);
		registro.group[sizeof(registro.group) - 1] = '\0';
		
		cout << "Series 1:          ";
		getline(cin, temp);
		strncpy(registro.seriesTitle1, temp.c_str(), sizeof(registro.seriesTitle1) - 1);
		registro.seriesTitle1[sizeof(registro.seriesTitle1) - 1] = '\0';
		
		cout << "Series 2:          ";
		getline(cin, temp);
		strncpy(registro.seriesTitle2, temp.c_str(), sizeof(registro.seriesTitle2) - 1);
		registro.seriesTitle2[sizeof(registro.seriesTitle2) - 1] = '\0';
		
		cout << "Series 3:          ";
		getline(cin, temp);
		strncpy(registro.seriesTitle3, temp.c_str(), sizeof(registro.seriesTitle3) - 1);
		registro.seriesTitle3[sizeof(registro.seriesTitle3) - 1] = '\0';
		
		cout << "Series 4:          ";
		getline(cin, temp);
		strncpy(registro.seriesTitle4, temp.c_str(), sizeof(registro.seriesTitle4) - 1);
		registro.seriesTitle4[sizeof(registro.seriesTitle4) - 1] = '\0';
		
		cout << "Series 5:          ";
		getline(cin, temp);
		strncpy(registro.seriesTitle5, temp.c_str(), sizeof(registro.seriesTitle5) - 1);
		registro.seriesTitle5[sizeof(registro.seriesTitle5) - 1] = '\0';
		
		inputFile.write((char*)&registro, DATA_SIZE);
		
		while(tempFile.read((char*)&registro, DATA_SIZE))
		{
			inputFile.write((char*)&registro, DATA_SIZE);
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

void datChange(string name, int x) {
	fstream r;
	r.open(name, ios::in | ios::out | ios::binary);
	
	int size = DATA_SIZE;
	int xPos = size * x;

	ifstream fileSize(name, ios::binary | ios::ate);
	
	size = fileSize.tellg() / size;
	fileSize.close();
	
	Dado registro;

	if (r.is_open() &&( x < size && x >= 0))
	{
		string temp;
		r.seekg(xPos, ios::beg);
		r.read((char*)&registro, DATA_SIZE);
		cout << "REGISTRO X: " << endl;
		printDado(registro);
		
		cout << "======================================" << endl;
		cout << "          INICIO DA MUDANCA           " << endl;
		cout << "======================================" << endl;
		cout << "Series Reference:  ";
		cin.ignore();
		getline(cin, temp);
		strncpy(registro.seriesReference, temp.c_str(), sizeof(registro.seriesReference) - 1);
		registro.seriesReference[sizeof(registro.seriesReference) - 1] = '\0';
		
		cout << "Period:            ";
		getline(cin, temp);
		strncpy(registro.period, temp.c_str(), sizeof(registro.period) - 1);
		registro.period[sizeof(registro.period) - 1] = '\0';
		
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
		strncpy(registro.status, temp.c_str(), sizeof(registro.status) - 1);
		registro.status[sizeof(registro.status) - 1] = '\0';
		
		cout << "Units:             ";
		getline(cin, temp);
		strncpy(registro.units, temp.c_str(), sizeof(registro.units) - 1);
		registro.units[sizeof(registro.units) - 1] = '\0';
		
		cout << "Magnitude:         ";
		cin >> registro.magnitude;
		
		cout << "Subject:           ";
		cin.ignore();
		getline(cin, temp);
		strncpy(registro.subject, temp.c_str(), sizeof(registro.subject) - 1);
		registro.subject[sizeof(registro.subject) - 1] = '\0';
		
		cout << "Group:             ";
		getline(cin, temp);
		strncpy(registro.group, temp.c_str(), sizeof(registro.group) - 1);
		registro.group[sizeof(registro.group) - 1] = '\0';
		
		cout << "Series 1:          ";
		getline(cin, temp);
		strncpy(registro.seriesTitle1, temp.c_str(), sizeof(registro.seriesTitle1) - 1);
		registro.seriesTitle1[sizeof(registro.seriesTitle1) - 1] = '\0';
		
		cout << "Series 2:          ";
		getline(cin, temp);
		strncpy(registro.seriesTitle2, temp.c_str(), sizeof(registro.seriesTitle2) - 1);
		registro.seriesTitle2[sizeof(registro.seriesTitle2) - 1] = '\0';
		
		cout << "Series 3:          ";
		getline(cin, temp);
		strncpy(registro.seriesTitle3, temp.c_str(), sizeof(registro.seriesTitle3) - 1);
		registro.seriesTitle3[sizeof(registro.seriesTitle3) - 1] = '\0';
		
		cout << "Series 4:          ";
		getline(cin, temp);
		strncpy(registro.seriesTitle4, temp.c_str(), sizeof(registro.seriesTitle4) - 1);
		registro.seriesTitle4[sizeof(registro.seriesTitle4) - 1] = '\0';
		
		cout << "Series 5:          ";
		getline(cin, temp);
		strncpy(registro.seriesTitle5, temp.c_str(), sizeof(registro.seriesTitle5) - 1);
		registro.seriesTitle5[sizeof(registro.seriesTitle5) - 1] = '\0';
		
		registro.isEndOfRun = 0;
		
		r.seekg(xPos, ios::beg);
		r.write((char*)&registro, DATA_SIZE);

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
void getFileByName(string &nome) {
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
		cout << "     EXTENSOES PERMITIDAS: '.csv'     " << endl;
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
bool folderHandler(string &nome) {
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

bool checkSort(int mode, string nome) {	
	int i = 0;
	fstream read;
	read.open(nome, ios::in | ios::binary);
	
	double value;
	Dado registro;
	
	read.read((char*)&registro, DATA_SIZE);
	value = registro.dataValue;
	
	while(read.read((char*)&registro, DATA_SIZE))
	{
		if(registro.dataValue < value && registro.dataValue != value && mode == 1)
		{
			read.close();
			return false;
			
		} else if (registro.dataValue > value && registro.dataValue != value && mode == 2)
		{
			read.close();
			return false;
		}
		
		value = registro.dataValue;
		i++;
	}
	read.close();
	
	return true;
}

void printDataValue(string arquivoDat) {
	cout << "======================================" << endl;
	cout << "           EXPORTACAO .TXT            " << endl;
	cout << "======================================" << endl;
	fstream read;
	ofstream output("./registro/dataValue.txt");
	read.open(arquivoDat, ios::in | ios::binary);
	
	Dado registro;
	
	while(read.read((char*)&registro, DATA_SIZE))
	{
		output << registro.dataValue << endl;
	}
	
	output.close();
	read.close();
	
	cout << "Documento Exportado: ./registro/dataValue.txt" << endl;
}

// Tela Principal
void telaMain(string &arquivoDat) {
	printFile();
	cout << "======================================" << endl;
	
	struct stat sb;
	if (stat(arquivoDat.c_str(), &sb) != 0) {
		cout << "         DOCUMENTO NAO EXISTE         " << endl;
		cout << "======================================" << endl;
		return;
	}
	
	cout << "          SELECIONE UMA OPCAO         " << endl;
	cout << "| Arquivo: " << arquivoDat << endl;
	cout << "======================================" << endl;
	cout << "| 1. Adicionar em Posicao            |" << endl;
	cout << "--------------------------------------" << endl;
	cout << "| 2. Visualizar Registros            |" << endl;
	cout << "--------------------------------------" << endl;
	cout << "| 3. Modificar Registro              |" << endl;
	cout << "--------------------------------------" << endl;
	cout << "| 4. Trocar Posicao de Registro      |" << endl;
	cout << "--------------------------------------" << endl;
	cout << "| 5. Imprimir Todos os Registros     |" << endl;
	cout << "--------------------------------------" << endl;
	cout << "| 6. Intercalacao Polifasica         |" << endl;
	cout << "--------------------------------------" << endl;
	cout << "| 7. Checar Ordenacao                |" << endl;
	cout << "--------------------------------------" << endl;
	cout << "| 8. Exportar dataValue              |" << endl;
	cout << "--------------------------------------" << endl;
	cout << "| 9. Importar Novo Arquivo .csv      |" << endl;
	cout << "--------------------------------------" << endl;
	cout << "| 0. Sair ou Mudar Arquivo .dat      |" << endl;
	cout << "--------------------------------------" << endl;
	
	int choice;
	cin >> choice;

	system("cls||clear");
	string name;
	string temp;
	string i;
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
		
		datInsert(arquivoDat, indexX);

		break;
	case 2:
		
		cout << "======================================" << endl;
		cout << "       DIGITE AS POSICOES X E Y       " << endl;
		cout << "======================================" << endl;
		cin >> indexX;
		cin >> indexY;

		system("cls||clear");
		
		datPrint(arquivoDat, indexX, indexY);

		break;
	case 3:
		cout << "======================================" << endl;
		cout << "      DIGITE A POSICAO DE EDICAO      " << endl;
		cout << "======================================" << endl;
		cin >> indexX;
		
		system("cls||clear");
		
		datChange(arquivoDat, indexX);
		
		break;
	case 4:
		
		cout << "======================================" << endl;
		cout << "     DIGITE AS POSICOES DE TROCA      " << endl;
		cout << "             X   &   Y                " << endl;
		cout << "======================================" << endl;
		cin >> indexX;
		cin >> indexY;

		system("cls||clear");
		
		datSwap(arquivoDat, indexX, indexY);

		break;
	case 5:
		system("cls||clear");
	
		//impressao
		datPrint(arquivoDat);
		
		break;
	case 6:
		cout << "======================================" << endl;
		cout << "        INTERCALACAO POLIFASICA       " << endl;
		cout << "======================================" << endl;
		cout << "|  Digite Nome Arquivo de Saida .dat  |" << endl;
		cout << "--------------------------------------" << endl;
		
		cin >> name;
		name = "./registro/" + name;
		
		cout << "======================================" << endl;
		cout << "|    Escolha o Modo de Ordenacao     |" << endl;
		cout << "| 1. Ascendente   | 2. Descendente   |" << endl;
		cout << "--------------------------------------" << endl;
		
		cin >> choice;
		
		polyphaseMerge(arquivoDat, name, AVAILABLE_MEMORY_SIZE, choice);
		
		break;
	case 7:		
		cout << "======================================" << endl;
		cout << "|  Escolha o Modo de Verificacao     |" << endl;
		cout << "| 1. Ascedente   | 2. Descendente    |" << endl;
		cout << "--------------------------------------" << endl;
		
		cin >> choice;
		
		if(checkSort(choice, arquivoDat))
		{
			cout << "======================================" << endl;
			cout << "       DOCUMENTO ESTA ORDENADO        " << endl;
			cout << "======================================" << endl;
		} else {
			cout << "======================================" << endl;
			cout << "     DOCUMENTO NAO ESTA ORDENADO      " << endl;
			cout << "======================================" << endl;
		}
		
		break;
	case 8:
		printDataValue(arquivoDat);
		break;
	case 9:
		// Converter CSV para binário
		cout << "======================================" << endl;
		cout << "    DIGITE O NOME DA ENTRADA .CSV     " << endl;
		cout << "======================================" << endl;
		cin >> temp;
		temp = "./registro/" + temp;
		
		cout << "======================================" << endl;
		cout << "     DIGITE O NOME DA SAIDA .DAT      " << endl;
		cout << "======================================" << endl;
		
		cin >> name;
		name = "./registro/" + name;
		
		csvReadWrite(temp, name);
		
		break;
	case 0:
		break;
		
	default:
		cout << "======================================" << endl;
		cout << "             OPCAO INVALIDA           " << endl;
		cout << "======================================" << endl;
		break;
	}
}

// Main
int main() {
	// Vars
	string arquivoCSV;
	string arquivoDAT;
	
	// Inicializando variaveis
	if (folderHandler(arquivoCSV)) {
		if(arquivoCSV != "exit"){
			// Converter CSV para binário
			cout << "======================================" << endl;
			cout << "     DIGITE O NOME DA SAIDA .DAT      " << endl;
			cout << "======================================" << endl;
			
			cin >> arquivoDAT;
			arquivoDAT = "./registro/" + arquivoDAT;
			csvReadWrite(arquivoCSV, arquivoDAT);
			
			// Verificar se o arquivo binário foi criado com sucesso
			ifstream check_file(arquivoDAT, ios::binary | ios::ate);
			if (!check_file.is_open()) {
				cerr << "Erro: Arquivo binario não foi criado!" << endl;
				return 1;
			}
			
			long long file_size = check_file.tellg();
			long long num_records = file_size / DATA_SIZE;
			
			check_file.close();
			
			cout << "Arquivo binario criado com sucesso!" << endl;
			cout << "Tamanho: " << file_size << " bytes" << endl;
			cout << "Registros: " << num_records << endl << endl;
			
			if (num_records == 0) {
				cerr << "Erro: Nenhum registro foi convertido!" << endl;
				return 1;
			}
			
			cout << "======================================" << endl;
			cout << "           FIM DA CONVERSAO           " << endl;
			cout << "======================================" << endl;
			cout << "| Digite qualquer valor para sair.   |" << endl;
			cout << "--------------------------------------" << endl;
			string i;
			cin >> i;
			
			system("cls||clear");
		} else
		{
			cout << "======================================" << endl;
			cout << "     DIGITE O NOME DO ARQUIVO .DAT    " << endl;
			cout << "======================================" << endl;
			
			cin >> arquivoDAT;
			arquivoDAT = "./registro/" + arquivoDAT;
			system("cls||clear");
		}
		
		// Loop de tela principal
		bool running = true;
		while (running)
		{
			telaMain(arquivoDAT);
			char continueChoice;
			
			cout << "======================================" << endl;
			printExit();
			cout << "======================================" << endl;
			cout << "           DESEJA CONTINUAR?          " << endl;
			cout << "(S) TELA INICIAL | (N) FECHA PROGRAMA " << endl;
			cout << "   (E) EDITAR ARQUIVO .DAT ABERTO     " << endl;
			cout << "======================================" << endl;
			cin >> continueChoice;
			system("cls||clear");
			
			if (continueChoice == 'N' || continueChoice == 'n')
			{
				running = false;
			} else if (continueChoice == 'E' || continueChoice == 'e')
			{
				cout << "======================================" << endl;
				cout << "         DIGITE NOVO ARQUIVO          " << endl;
				cout << "======================================" << endl;
				cin >> arquivoDAT;
				arquivoDAT = "./registro/" + arquivoDAT;
				system("cls||clear");
			}
		}
	}

	return 0;
}
