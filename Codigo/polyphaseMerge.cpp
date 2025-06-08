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

// Struct dado para armazenar dados do .csv
struct dado {
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
    dado() : dataValue(0.0), magnitude(0), isEndOfRun(0) {
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
    bool operator>(const dado& other) const {
        if (this->isEndOfRun == 1 && other.isEndOfRun == 0) {
            return true;
        }
        if (this->isEndOfRun == 0 && other.isEndOfRun == 1) {
            return false;
        }
        return this->dataValue > other.dataValue;
    }

    bool operator<(const dado& other) const {
        if (this->isEndOfRun == 0 && other.isEndOfRun == 1) {
            return true;
        }
        if (this->isEndOfRun == 1 && other.isEndOfRun == 0) {
            return false;
        }
        return this->dataValue < other.dataValue;
    }

    bool operator==(const dado& other) const {
        if (this->isEndOfRun != other.isEndOfRun) {
            return false;
        }
        if (this->isEndOfRun == 1) {
            return true;
        }
        return this->dataValue == other.dataValue;
    }

    bool operator<=(const dado& other) const {
        return (*this < other) || (*this == other);
    }
    
    bool operator>=(const dado& other) const {
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

// Define o Tamanho de Dado
const long long DATA_SIZE = sizeof(dado);
// Memoria RAM Disponivel para a Intercalacao
const unsigned int AVAILABLE_MEMORY_SIZE = 50 * 1024 * 1024;

// Lê múltiplos registros de uma vez para reduzir chamadas de sistema
int blockRead(ifstream& arquivo, dado* buffer, size_t bufferSize) {
    arquivo.read((char*)buffer, bufferSize * DATA_SIZE);
    streamsize bytes_read = arquivo.gcount();
    int records_read = bytes_read / DATA_SIZE;

    // Check stream state flags
    if (arquivo.eof()) {
        cout << "blockRead - Stream EOF bit set." << std::endl;
    }
    if (arquivo.fail()) {
        cout << "blockRead - Stream FAIL bit set." << std::endl;
    }
    if (arquivo.bad()) {
        cout << "blockRead - Stream BAD bit set." << std::endl;
    }
    
    arquivo.clear();    
    
    return records_read;
}

// Funcao para Escrever Blocos de Registros do Buffer para um Arquivo
void blockWrite(std::ofstream& arquivo, dado* buffer, size_t num_registros) {
    for (size_t i = 0; i < num_registros; ++i) {
        arquivo.write((char*)&buffer[i], DATA_SIZE);
    }
}

// --- Tape Management Structure ---
// Abstrai uma "fita" do algoritmo de intercalação polifásica
struct PolyphaseTape {
    string filename;
    mutable fstream stream;
    long long actualRuns; // Sequências reais de dados ordenados
    long long dummyRuns;  // Sequências fictícias para balanceamento Fibonacci

    PolyphaseTape(const string& fname) : filename(fname), actualRuns(0), dummyRuns(0) {}

    ~PolyphaseTape() {
        if (stream.is_open()) {
            stream.close();
        }
    }
    
    // Method to rewind stream for a new merge pass (or after initial write)
    void rewind_and_clear_flags() {
        if (stream.is_open()) {
            stream.clear();
            stream.seekg(0);
            stream.seekp(0);
        }
    }
    
    // Lê um registro completo ou falha - não permite leituras parciais
    bool readRecord(dado& record) {
        if (!stream.is_open()) {
            cerr << "ERROR: Attempted to read from closed tape stream: " << filename << endl;
            return false;
        }

        stream.read((char*)&record, DATA_SIZE);
        streamsize bytes_read = stream.gcount();

        if (bytes_read < DATA_SIZE) { 
            if (stream.eof()) {
                cout << "blockRead - Stream EOF bit set." << endl; 
            } else if (stream.fail() && !stream.eof()) {
                cerr << "Read error on tape " << filename << " (gcount: " << bytes_read << ")" << endl;
            }
            stream.clear();
            return false;
        }
        return true;
    }

    bool writeRecord(const dado& record) {
        if (!stream.is_open()) {
            cerr << "ERROR: Attempted to write to closed tape stream: " << filename << endl;
            return false;
        }
            
        if (!stream.write((char*)&record, DATA_SIZE)) {
            cerr << "ERROR: Failed to write record to tape '" << filename << "'. Stream state: " << stream.rdstate() << " Error: " << strerror(errno) << endl;
            stream.clear();
            return false;
        }
        streampos after_write_pos = stream.tellp();
        return true;
    }
    
    // Add a method to get the current file size
    long long getFileSize() const {
        if (filename.empty()) {
            return -1; // No filename set
        }
        try {
            return std::filesystem::file_size(filename);
        } catch (const std::filesystem::filesystem_error& e) {
            if (stream.is_open()) {
                std::streampos currentPos = stream.tellg(); 
                stream.seekg(0, std::ios::end);
                long long size = stream.tellg();
                stream.seekg(currentPos);
                return size;
            }
            return -1;
        }
    }
};

// Gerencia leitura sequencial de uma fita durante intercalação
class mergeFileIn {
public:
    PolyphaseTape* tape;
    dado currentReg;
    bool hasData; // Indica se currentReg contém dados válidos da sequência atual

    mergeFileIn(PolyphaseTape* t) : tape(t), hasData(false) {}

    // Para na primeira marca EOR encontrada - define limite da sequência lógica
    bool loadNextDataRecord() {
        currentReg = dado();
        hasData = false;

        dado tempReg;
        bool read_success = tape->readRecord(tempReg);

        if (!read_success) {
            currentReg = dado(); 
            return false; 
        }

        if (tempReg.isEndOfRun == 1) {
            currentReg = dado(); 
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
            tape->stream.clear(); // Ensure stream flags are clear for a dummy run that might be processed from an exhausted tape.
            hasData = false; // No data to provide.
            currentReg = dado(); // Ensure currentReg is cleared.
            cout << "Tape " << tape->filename << " logically exhausted. No new run prepared.\n";
            return;
        }

        if (tape->actualRuns > 0) {
            hasData = loadNextDataRecord(); // Load the first data record of the new run
            if (!hasData) {
                cerr << "WARNING: Expected actual run on tape " << tape->filename << " but could not load first data record (immediate EOR/EOF or corruption). At run " << " (phase loop 'i')" << endl;
            } else {
            }
            tape->actualRuns--; 
        } else {
            hasData = false; 
            currentReg = dado(); 
            tape->dummyRuns--; 
        }
    }

    // Returns true if a data record was successfully loaded.
    bool advance() {
        if (!hasData) {
            return false;
        }
        hasData = loadNextDataRecord(); 
        return hasData; 
    }
    
    dado& getCurrentRecord() {
        return currentReg;
    }

    bool hasRecords() const {
        return hasData;
    }

    bool isEndOfCurrentRun() const {
        return !hasData; // If hasData is false, the current logical run is exhausted.
    }
};

// Calcula distribuição Fibonacci necessária para balancear intercalação polifásica
long long calculateInitialDummyRuns(long long total_actual_runs_from_input_file, long long fib_targets[3]) {
    if (total_actual_runs_from_input_file == 0) {
        fib_targets[0] = fib_targets[1] = fib_targets[2] = 0;
        cout << "DEBUG: Fibonacci targets: 0, 0, 0 (No actual runs)" << endl;
        return 0;
    }

    long long fib_a = 0; 
    long long fib_b = 1; 
    long long fib_c = 1; 

    int k_index = 2; 

    while (fib_c < total_actual_runs_from_input_file) {
        long long next_fib_c = fib_b + fib_c;
        fib_a = fib_b;
        fib_b = fib_c;
        fib_c = next_fib_c;
        k_index++;
    }

    fib_targets[0] = fib_c; 
    fib_targets[1] = fib_b; 
    fib_targets[2] = fib_a; 

    long long total_fib_runs_at_current_level = fib_targets[0] + fib_targets[1] + fib_targets[2];

    cout << "calculateInitialDummyRuns called with total_actual_runs: " << total_actual_runs_from_input_file << endl;
    cout << "k_index (F_k): " << k_index << std::endl;
    cout << "Calculated Fibonacci targets (t0,t1,t2): " << fib_targets[0] << ", "
              << fib_targets[1] << ", " << fib_targets[2] << endl;
    cout << "Total runs at target Fib level (sum of targets): " << total_fib_runs_at_current_level << endl;

    long long num_dummies_needed = total_fib_runs_at_current_level - total_actual_runs_from_input_file;
    cout << "Total dummy runs needed for system: " << num_dummies_needed << endl;

    return num_dummies_needed;
}

//Removev Arquivos Temporarios Utilizados
void cleanTemp() {
    cout << "Cleaning temporary files..." << endl;
    if (remove("./registro/t0.dat") != 0 && errno != ENOENT) cerr << "Error removing t0.dat: " << strerror(errno) << endl;
    if (remove("./registro/t1.dat") != 0 && errno != ENOENT) cerr << "Error removing t1.dat: " << strerror(errno) << endl;
    if (remove("./registro/t2.dat") != 0 && errno != ENOENT) cerr << "Error removing t2.dat: " << strerror(errno) << endl;
    if (remove("./registro/temp_initial_runs.dat") != 0 && errno != ENOENT) cerr << "Error removing t2.dat: " << strerror(errno) << endl;
    cout << "Temporary files cleaned." << endl;
}

// --- Funcao Principal: Intercalacao Polifasica ---
// Funcao que Realiza a Intercalacao Polifasica
void polyphaseMerge(string arquivo_entrada, string arquivo_saida, int tamanho_memoria_disponivel_bytes, int mode) {
    // Para uma Intercalacao 3-Way, NAO MUDAR
    const int NUM_ARQUIVOS_AUXILIARES = 3;
    string nomes_arquivos_auxiliares[NUM_ARQUIVOS_AUXILIARES];

    // Local dos Arquivos Temporarios Utilizados na Intercalacao
    nomes_arquivos_auxiliares[0] = "./registro/t0.dat";
    nomes_arquivos_auxiliares[1] = "./registro/t1.dat";
    nomes_arquivos_auxiliares[2] = "./registro/t2.dat";

    long long initial_total_actual_runs = 0;

    // Local do Arquivo Inicial que Prepara a Intercalacao (Distribuicao das Dummys)
    string temp_initial_runs_filename = "./registro/temp_initial_runs.dat";
    ofstream temp_initial_runs_stream;

    dado* buffer_memoria = nullptr;

    PolyphaseTape* tapes[NUM_ARQUIVOS_AUXILIARES];
    for(int i = 0; i < NUM_ARQUIVOS_AUXILIARES; ++i) {
        tapes[i] = new PolyphaseTape(nomes_arquivos_auxiliares[i]);
        if (remove(tapes[i]->filename.c_str()) != 0 && errno != ENOENT) {
            cerr << "AVISO: Nao foi possivel remover o arquivo auxiliar antigo (pode nao existir): " << tapes[i]->filename << ": " << strerror(errno) << endl;
        }
    }

    size_t capacidade_buffer_registros = tamanho_memoria_disponivel_bytes / DATA_SIZE;

    cout << "Capacidade do buffer de registros calculada: " << capacidade_buffer_registros << endl;
    if (capacidade_buffer_registros == 0) {
        cerr << "Tamanho do buffer muito pequeno para um registro. Deve ser no minimo " << DATA_SIZE << " bytes." << endl;
        // Limpeza adequada se a alocacao do buffer falhar ou fornecer
        if (buffer_memoria) delete[] buffer_memoria;
        for(int i = 0; i < NUM_ARQUIVOS_AUXILIARES; ++i) delete tapes[i];
        return;
    }

    buffer_memoria = new dado[capacidade_buffer_registros]; // Aloca aqui

    // --- 1. Fase de Criacao e Distribuicao de Runs Iniciais ---
    // Prepara o Arquivo de Entrada para a Intercalacao
    cout << "Fase 1: Criando runs iniciais e distribuindo para Intercalacao Polifasica..." << endl;
    ifstream entrada_original;
    entrada_original.open(arquivo_entrada, ios::in | ios::binary);

    if (!entrada_original.is_open()) {
        cerr << "Erro ao abrir o arquivo de entrada: " << arquivo_entrada << endl;
        if (buffer_memoria) delete[] buffer_memoria;
        for(int i = 0; i < NUM_ARQUIVOS_AUXILIARES; ++i) delete tapes[i];
        return;
    }

    entrada_original.seekg(0, ios::end);
    long long total_records_in_source_file = entrada_original.tellg() / DATA_SIZE;
    entrada_original.clear();
    entrada_original.seekg(0);
    cout << "Total de registros no arquivo de entrada: " << total_records_in_source_file << endl;

    if (total_records_in_source_file == 0) {
        cout << "Arquivo de entrada esta vazio. Nada para ordenar." << endl;
        entrada_original.close();
        if (buffer_memoria) delete[] buffer_memoria;
        for(int i = 0; i < NUM_ARQUIVOS_AUXILIARES; ++i) delete tapes[i];
        cleanTemp();
        return;
    }

    temp_initial_runs_stream.open(temp_initial_runs_filename, std::ios::out | std::ios::binary);
    if (!temp_initial_runs_stream.is_open()) {
        cerr << "ERRO FATAL: Nao foi possivel abrir o arquivo temporario de runs iniciais: " << temp_initial_runs_filename << endl;
        if (buffer_memoria) delete[] buffer_memoria;
        for(int i = 0; i < NUM_ARQUIVOS_AUXILIARES; ++i) delete tapes[i];
        return;
    }

    long long total_actual_runs_physically_created = 0;
    cout << "Iniciando loop de criacao de runs iniciais (Fase 1: Escrevendo para arquivo temp)..." << endl;

    int lidos = blockRead(entrada_original, buffer_memoria, capacidade_buffer_registros);

    // Distribui os Arquivos Dummy para a Distribuicao Logica
    while (lidos != 0) {

        if (mode == 1) { // Ascendente
            sort(buffer_memoria, buffer_memoria + lidos);
        } else { // Descendente
            sort(buffer_memoria, buffer_memoria + lidos, greater<dado>());
        }

        temp_initial_runs_stream.write((char*)buffer_memoria, lidos * DATA_SIZE);

        dado eor_marker;
        eor_marker.setAsEndOfRun();
        temp_initial_runs_stream.write((char*)&eor_marker, DATA_SIZE);

        total_actual_runs_physically_created++;
        lidos = blockRead(entrada_original, buffer_memoria, capacidade_buffer_registros);
    }

    cout << "Fim do arquivo de entrada atingido ou sem mais registros para ler para runs iniciais." << endl;

    entrada_original.close();
    temp_initial_runs_stream.close();
    cout << "\nFim da Fase 1. Total de " << total_actual_runs_physically_created << " runs criadas no arquivo temporario." << endl << endl;
    initial_total_actual_runs = total_actual_runs_physically_created;

    // --- Calcula os Alvos de Fibonacci ---
    long long fib_targets[3];
    calculateInitialDummyRuns(total_actual_runs_physically_created, fib_targets);

    // --- FASE 2: Distribui runs do arquivo temporario para fitas auxiliares ---
    ifstream temp_initial_runs_reader_stream(temp_initial_runs_filename, ios::in | ios::binary);
    if (!temp_initial_runs_reader_stream.is_open()) {
        cerr << "ERRO FATAL: Nao foi possivel abrir o arquivo temporario de runs iniciais para leitura (fase de distribuicao)." << endl;
        if (buffer_memoria) delete[] buffer_memoria;
        for(int j = 0; j < NUM_ARQUIVOS_AUXILIARES; ++j) delete tapes[j];
        return;
    }

    ofstream distrib_tapes_write_streams[NUM_ARQUIVOS_AUXILIARES];
    for (int i = 0; i < NUM_ARQUIVOS_AUXILIARES; ++i) {
        distrib_tapes_write_streams[i].open(nomes_arquivos_auxiliares[i], ios::out | ios::binary | ios::trunc);
        if (!distrib_tapes_write_streams[i].is_open()) {
            cerr << "ERRO FATAL: Nao foi possivel abrir a fita de distribuicao " << i << " para escrita: " << strerror(errno) << endl;
            if (buffer_memoria) delete[] buffer_memoria;
            for(int j = 0; j < NUM_ARQUIVOS_AUXILIARES; ++j) delete tapes[j];
            return;
        }
        tapes[i]->actualRuns = 0; // Reinicia contadores logicos para distribuicao
        tapes[i]->dummyRuns = 0;
    }

    // Declara input_tape1_idx e input_tape2_idx aqui para este escopo
    int input_tape1_idx = 0;
    int input_tape2_idx = 1;

    long long runs_for_tape0 = fib_targets[1];
    long long runs_for_tape1 = total_actual_runs_physically_created - runs_for_tape0;

    runs_for_tape0 = std::max(0LL, runs_for_tape0);
    runs_for_tape1 = std::max(0LL, runs_for_tape1);

    cout << "Iniciando Fase 2: Distribuindo runs do arquivo temp para fitas auxiliares..." << endl;
    cout << "Runs determinadas para distribuir: Fita " << input_tape1_idx << " recebe " << runs_for_tape0
              << " runs, Fita " << input_tape2_idx << " recebe " << runs_for_tape1 << " runs." << endl;

    for (int i = 0; i < total_actual_runs_physically_created; ++i) {
        vector<dado> current_run_data;
        dado record;

        while (temp_initial_runs_reader_stream.read((char*)&record, DATA_SIZE) && record.isEndOfRun != 1) {
            streamsize bytes_read = temp_initial_runs_reader_stream.gcount();

            if (bytes_read < DATA_SIZE) { // EOF ou erro de leitura
                temp_initial_runs_reader_stream.clear(); // Limpa flags para futuras leituras, se possivel
            } else if (record.isEndOfRun != 1) {
                current_run_data.push_back(record);
            }
        }

        int target_tape_for_this_run;
        if (tapes[input_tape1_idx]->actualRuns < runs_for_tape0) {
            target_tape_for_this_run = input_tape1_idx;
        } else {
            target_tape_for_this_run = input_tape2_idx;
        }

        if (!current_run_data.empty()) {
            distrib_tapes_write_streams[target_tape_for_this_run].write((char*)current_run_data.data(), current_run_data.size() * DATA_SIZE);
        }

        dado eor_marker_dist;
        eor_marker_dist.setAsEndOfRun();
        distrib_tapes_write_streams[target_tape_for_this_run].write((char*)&eor_marker_dist, DATA_SIZE);

        tapes[target_tape_for_this_run]->actualRuns++;
    }

    temp_initial_runs_reader_stream.close();
    remove(temp_initial_runs_filename.c_str());

    // Fecha todos os fluxos de distribuicao (eram ofstream)
    for (int i = 0; i < NUM_ARQUIVOS_AUXILIARES; ++i) {
        if (distrib_tapes_write_streams[i].is_open()) {
            distrib_tapes_write_streams[i].close();
        }
    }

    // Agora, calcula explicitamente as runs dummy finais apos todas as runs reais terem sido distribuidas
    tapes[0]->dummyRuns = fib_targets[0] - tapes[0]->actualRuns;
    tapes[1]->dummyRuns = fib_targets[1] - tapes[1]->actualRuns;
    tapes[2]->dummyRuns = fib_targets[2] - tapes[2]->actualRuns;

    tapes[0]->dummyRuns = std::max(0LL, tapes[0]->dummyRuns);
    tapes[1]->dummyRuns = std::max(0LL, tapes[1]->dummyRuns);
    tapes[2]->dummyRuns = std::max(0LL, tapes[2]->dummyRuns);


    cout << "Runs iniciais criadas e distribuidas. Runs reais (tapes[i]->actualRuns): "
              << tapes[0]->actualRuns << ", " << tapes[1]->actualRuns << ", " << tapes[2]->actualRuns << endl;
    cout << "Runs dummy iniciais (tapes[i]->dummyRuns): "
              << tapes[0]->dummyRuns << ", " << tapes[1]->dummyRuns << ", " << tapes[2]->dummyRuns << endl;
    cout << "Total de runs (Reais + Dummy) por fita: "
              << (tapes[0]->actualRuns + tapes[0]->dummyRuns) << ", "
              << (tapes[1]->actualRuns + tapes[1]->dummyRuns) << ", "
              << (tapes[2]->actualRuns + tapes[2]->dummyRuns) << std::endl;

    std::cout << "Tamanhos dos arquivos apos distribuicao inicial:" << std::endl;
    for (int i = 0; i < NUM_ARQUIVOS_AUXILIARES; ++i) {
        cout << "  " << tapes[i]->filename << ": " << tapes[i]->getFileSize() << " bytes" << endl;
    }

    // --- ABRE TODAS AS FITAS COMO FSTREAM PARA AS FASES DE INTERCALACAO ---
    for(int i = 0; i < NUM_ARQUIVOS_AUXILIARES; ++i) {
        tapes[i]->stream.open(tapes[i]->filename, ios::in | ios::out | ios::binary);
        if (!tapes[i]->stream.is_open()) {
            cerr << "ERRO FATAL: Nao foi possivel abrir a fita " << i << " para leitura/escrita nas fases de intercalacao: " << strerror(errno) << endl;
            if (buffer_memoria) delete[] buffer_memoria;
            for (int j = 0; j < NUM_ARQUIVOS_AUXILIARES; ++j) delete tapes[j];
            return;
        }
        tapes[i]->rewind_and_clear_flags(); // Garante que todas as fitas comecem no inicio para a primeira passagem de intercalacao
    }

    // --- 2. Fases de Intercalacao ---
    cout << "\nIniciando fases de intercalacao..." << endl;

    int inputTapeIdx[2];
    int outputTapeIdx;
    int prevOutputTapeIdx = 2; // Para a primeira fase de intercalacao, t2 e a saida inicial.

    inputTapeIdx[0] = 0;
    inputTapeIdx[1] = 1;
    outputTapeIdx = 2;

    int fase_num = 0;
    bool endMerge = false;

    while (!endMerge) {
        fase_num++;

        int tapes_with_actual_data_runs_count = 0;
        int tape_holding_final_result_idx = -1;

        for (int i = 0; i < NUM_ARQUIVOS_AUXILIARES; ++i) {
            if (tapes[i]->actualRuns > 0) {
                tapes_with_actual_data_runs_count++;
                tape_holding_final_result_idx = i;
            }
        }

        if (tapes_with_actual_data_runs_count == 1 &&
            tapes[tape_holding_final_result_idx]->actualRuns == 1 &&
            tapes[tape_holding_final_result_idx]->dummyRuns == 0) {

            bool all_others_empty = true;
            for (int i = 0; i < NUM_ARQUIVOS_AUXILIARES; ++i) {
                if (i != tape_holding_final_result_idx) {
                    if (tapes[i]->actualRuns != 0 || tapes[i]->dummyRuns != 0) {
                        all_others_empty = false;
                        i = NUM_ARQUIVOS_AUXILIARES;
                    }
                }
            }

            if (all_others_empty) {
                outputTapeIdx = tape_holding_final_result_idx;
                endMerge = true;
            }
        }

        if (!endMerge) {
            if (tapes_with_actual_data_runs_count == 0 && initial_total_actual_runs > 0) {
                cerr << "ERRO GRAVE: Todas as runs reais perdidas durante a intercalacao! Saindo do loop." << endl;
                break;
            }

            long long runs_to_merge_this_pass = std::min(
                tapes[inputTapeIdx[0]]->actualRuns + tapes[inputTapeIdx[0]]->dummyRuns,
                tapes[inputTapeIdx[1]]->actualRuns + tapes[inputTapeIdx[1]]->dummyRuns
            );

            if (runs_to_merge_this_pass == 0) {
                cerr << "ERRO GRAVE: runs_to_merge_this_pass e 0, mas a ordenacao nao esta completa. Isso indica um erro de logica na contagem de runs ou determinacao de fase. Saindo do loop." << endl;
                break;
            }

            // --- Gerencia as posicoes da fita e as flags para esta fase ---
            tapes[outputTapeIdx]->rewind_and_clear_flags();

            // Fita de Entrada 0:
            if (inputTapeIdx[0] == prevOutputTapeIdx) {
                tapes[inputTapeIdx[0]]->rewind_and_clear_flags();
            } else {
                tapes[inputTapeIdx[0]]->stream.clear();
            }

            // Fita de Entrada 1:
            if (inputTapeIdx[1] == prevOutputTapeIdx) {
                tapes[inputTapeIdx[1]]->rewind_and_clear_flags();
            } else {
                tapes[inputTapeIdx[1]]->stream.clear();
            }

            prevOutputTapeIdx = outputTapeIdx; // Atualiza prevOutputTapeIdx para a proxima iteracao

            tapes[outputTapeIdx]->actualRuns = 0;
            tapes[outputTapeIdx]->dummyRuns = 0;

            cout << "\nInicio da Fase " << fase_num << "." << endl;
            cout << "  Fitas de Entrada: t" << inputTapeIdx[0] << " (R:" << tapes[inputTapeIdx[0]]->actualRuns << ",D:" << tapes[inputTapeIdx[0]]->dummyRuns << ")"
                      << ", t" << inputTapeIdx[1] << " (R:" << tapes[inputTapeIdx[1]]->actualRuns << ",D:" << tapes[inputTapeIdx[1]]->dummyRuns << ")" << endl;
            cout << "  Fita de Saida: t" << outputTapeIdx << endl;
            cout << "  Runs para intercalar nesta passagem (logico): " << runs_to_merge_this_pass << endl;

            mergeFileIn* entradas_merge[2];
            entradas_merge[0] = new mergeFileIn(tapes[inputTapeIdx[0]]);
            entradas_merge[1] = new mergeFileIn(tapes[inputTapeIdx[1]]);

            long long actual_runs_produced_this_phase = 0;
            long long dummy_runs_produced_this_phase = 0;

            for (long long i = 0; i < runs_to_merge_this_pass; ++i) {
                cout << "Intercalando run " << i + 1 << " de " << runs_to_merge_this_pass << endl;

                bool this_output_run_is_actual = false;

                if (tapes[inputTapeIdx[0]]->actualRuns > 0) {
                    entradas_merge[0]->prepareForNewLogicalRun();
                    if (entradas_merge[0]->hasRecords()) {
                        this_output_run_is_actual = true;
                    }
                } else if (tapes[inputTapeIdx[0]]->dummyRuns > 0) {
                    entradas_merge[0]->prepareForNewLogicalRun();
                }
                if (tapes[inputTapeIdx[1]]->actualRuns > 0) {
                    entradas_merge[1]->prepareForNewLogicalRun();
                    if (entradas_merge[1]->hasRecords()) {
                        this_output_run_is_actual = true;
                    }
                } else if (tapes[inputTapeIdx[1]]->dummyRuns > 0) {
                    entradas_merge[1]->prepareForNewLogicalRun();
                }

                while (entradas_merge[0]->hasRecords() || entradas_merge[1]->hasRecords()) {
                    dado record_to_write;
                    bool record_taken = false;

                    if (entradas_merge[0]->hasRecords() && entradas_merge[1]->hasRecords()) {
                        if (mode == 1) { // Ascendente
                            if (entradas_merge[0]->getCurrentRecord() <= entradas_merge[1]->getCurrentRecord()) {
                                record_to_write = entradas_merge[0]->getCurrentRecord();
                                record_taken = true;
                                entradas_merge[0]->advance();
                            } else {
                                record_to_write = entradas_merge[1]->getCurrentRecord();
                                record_taken = true;
                                entradas_merge[1]->advance();
                            }
                        } else { // Descendente
                            if (entradas_merge[0]->getCurrentRecord() >= entradas_merge[1]->getCurrentRecord()) {
                                record_to_write = entradas_merge[0]->getCurrentRecord();
                                record_taken = true;
                                entradas_merge[0]->advance();
                            } else {
                                record_to_write = entradas_merge[1]->getCurrentRecord();
                                record_taken = true;
                                entradas_merge[1]->advance();
                            }
                        }
                    } else if (entradas_merge[0]->hasRecords()) {
                        record_to_write = entradas_merge[0]->getCurrentRecord();
                        record_taken = true;
                        entradas_merge[0]->advance();
                    } else if (entradas_merge[1]->hasRecords()) {
                        record_to_write = entradas_merge[1]->getCurrentRecord();
                        record_taken = true;
                        entradas_merge[1]->advance();
                    }

                    if (record_taken) {
                        if (!record_to_write.endMarker()) {
                            tapes[outputTapeIdx]->writeRecord(record_to_write);
                        }
                    }
                }

                if (this_output_run_is_actual) {
                    dado eor_marker;
                    eor_marker.setAsEndOfRun();
                    tapes[outputTapeIdx]->writeRecord(eor_marker);
                    actual_runs_produced_this_phase++;
                } else {
                    dummy_runs_produced_this_phase++;
                }
            }

            delete entradas_merge[0];
            delete entradas_merge[1];

            // Garante que a fita de saida nao tenha arquivos fantasmas ou remanescentes
            try {
                long long current_write_pos = tapes[outputTapeIdx]->stream.tellp();
                filesystem::resize_file(tapes[outputTapeIdx]->filename, current_write_pos);
                cout << "Fita de saida " << outputTapeIdx << " truncada para " << current_write_pos << " bytes." << endl;
            } catch (const filesystem::filesystem_error& e) {
                cerr << "Falha ao truncar fita de saida " << outputTapeIdx << ": " << e.what() << endl;
            }

            tapes[outputTapeIdx]->actualRuns = actual_runs_produced_this_phase;
            tapes[outputTapeIdx]->dummyRuns = dummy_runs_produced_this_phase;
            cout << "DEBUG: Fita de saida " << outputTapeIdx << " agora tem "
                      << tapes[outputTapeIdx]->actualRuns << " runs reais e "
                      << tapes[outputTapeIdx]->dummyRuns << " runs dummy." << std::endl;

            cout << "Tamanhos dos arquivos apos intercalacao/copia para a Fase " << fase_num << ":" << endl;
            for (int i = 0; i < NUM_ARQUIVOS_AUXILIARES; ++i) { // Condicao de loop corrigida aqui
                cout << "  " << tapes[i]->filename << ": " << tapes[i]->getFileSize() << " bytes" << endl;
            }

            int exhausted_input_tape_idx = -1;
            if (tapes[inputTapeIdx[0]]->actualRuns == 0 && tapes[inputTapeIdx[0]]->dummyRuns == 0) {
                exhausted_input_tape_idx = inputTapeIdx[0];
            } else if (tapes[inputTapeIdx[1]]->actualRuns == 0 && tapes[inputTapeIdx[1]]->dummyRuns == 0) {
                exhausted_input_tape_idx = inputTapeIdx[1];
            }

            if (exhausted_input_tape_idx == -1) {
                cerr << "ERRO GRAVE: Nenhuma fita de entrada foi esgotada apos a fase de intercalacao. Saindo do loop." << endl;
                break;
            }

            // Encontra a proxima fita de entrada disponivel
            int next_input_tape_idx = -1;
            for (int i = 0; i < NUM_ARQUIVOS_AUXILIARES; ++i) {
                if (i != exhausted_input_tape_idx && i != prevOutputTapeIdx) { // Usa prevOutputTapeIdx
                    next_input_tape_idx = i;
                    i = NUM_ARQUIVOS_AUXILIARES;
                }
            }

            inputTapeIdx[0] = next_input_tape_idx;
            inputTapeIdx[1] = prevOutputTapeIdx; // Isso era outputTapeIdx, agora e prevOutputTapeIdx (saida anterior)
            outputTapeIdx = exhausted_input_tape_idx;

            cout << "Fim da Fase " << fase_num << ". Configuracao da proxima fase: Fitas de Entrada: t" << inputTapeIdx[0] << ", t" << inputTapeIdx[1] << "; Fita de Saida: t" << outputTapeIdx << std::endl;
        }

    } // Fim do loop principal de intercalacao

    // --- 3. Copia Final para o Arquivo de Saida ---
    cout << "\nIniciando copia final para o arquivo de saida: " << arquivo_saida << endl;
    ofstream saida_final;
    saida_final.open(arquivo_saida, ios::out | ios::binary | ios::trunc); // Usa ios::trunc para garantir arquivo limpo

    if (!saida_final.is_open()) {
        cerr << "Erro ao abrir o arquivo de saida final: " << arquivo_saida << endl;
        // Limpeza e saida
        if (buffer_memoria) delete[] buffer_memoria;
        for(int i = 0; i < NUM_ARQUIVOS_AUXILIARES; ++i) delete tapes[i];
        return;
    }

    PolyphaseTape* final_sorted_tape = tapes[outputTapeIdx];
    final_sorted_tape->rewind_and_clear_flags();

    long long records_copied = 0;
    dado temp_dado;

    // Le e copia os registros ate exceto EOR
    while(final_sorted_tape->stream.read((char*)&temp_dado, DATA_SIZE)) {
        if (temp_dado.isEndOfRun == 0) {
            saida_final.write(reinterpret_cast<char*>(&temp_dado), DATA_SIZE);
            records_copied++;
        }
    }

    saida_final.close();

    cout << "Total de registros copiados para a saida final: " << records_copied << endl;
    cout << "Intercalacao polifasica concluida com sucesso." << endl;

    // --- Limpeza ---
    if (buffer_memoria) {
        delete[] buffer_memoria;
        buffer_memoria = nullptr; // Boa pratica
    }
    for(int i = 0; i < NUM_ARQUIVOS_AUXILIARES; ++i) {
        delete tapes[i];
        tapes[i] = nullptr; // Boa pratica
    }

    // Verifica se a intercalacao foi bem-sucedida
    ifstream check_sorted("./registro/dados_ordenados.dat", ios::binary | ios::ate);
    if (!check_sorted.is_open()) {
        cerr << "Erro: Arquivo ordenado nao foi criado!" << endl;
    } else {
        long long sorted_size = check_sorted.tellg();
        long long sorted_records = sorted_size / DATA_SIZE;
        check_sorted.close();

        cout << "\nResultado da intercalacao:" << endl;
        cout << "Arquivo ordenado: " << sorted_size << " bytes" << endl;
        cout << "Registros ordenados: " << sorted_records << endl;

        if (sorted_records == 0) {
            cerr << "ERRO CRITICO: Todos os registros foram perdidos na intercalacao!" << endl;
        }
    }

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
	ofstream fileText;
	fileText.open("dados_entrada.txt");
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
		dado registro = {};

		// Copiar campos na ordem correta (garantir que não exceda limites dos arrays)
		strncpy(registro.seriesReference, fields[0].c_str(), sizeof(registro.seriesReference) - 1);
		registro.seriesReference[sizeof(registro.seriesReference) - 1] = '\0';
		
		strncpy(registro.period, fields[1].c_str(), sizeof(registro.period) - 1);
		registro.period[sizeof(registro.period) - 1] = '\0';

		// Data value com tratamento de erro
		if(!fields[2].empty()){
			try {
				registro.dataValue = stod(fields[2]);
			} catch (const std::exception& e) {
				registro.dataValue = 0.0; // Valor padrão se conversão falhar
			}
		} else {
			registro.dataValue = 0.0; // Valor padrão para campos vazios
		}
		
		fileText << registro.dataValue << endl;
		
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
	cout << "DEBUG: CSV conversion complete. Final binary file size: " << saveFile.tellp() << " bytes." << endl;

	openFile.close();
	saveFile.close();
	fileText.close();
	
	cout << "Total de linhas processadas: " << total_lines_processed << endl;
	cout << "Registros convertidos do CSV: " << records_written << endl;
	cout << "Linhas com problemas de formatacao (corrigidas): " << problematic_lines << endl;
	
	// Calcular estatísticas
	double success_rate = (double)records_written / total_lines_processed * 100.0;
	cout << "Taxa de sucesso: " << success_rate << "%" << endl;
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
	r.open(name, ios::in | ios::binary);

	ifstream fileSize(name, ios::binary | ios::ate);
	int size = DATA_SIZE;
	size = fileSize.tellg() / size;
	fileSize.close();

	dado registro;
	
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
void datPrint(string name, int x, int y)
{
	fstream r;
	r.open(name, ios::in | ios::binary);
	
	int size = DATA_SIZE;
	int start = size * x;

	ifstream fileSize(name, ios::binary | ios::ate);
	
	size = fileSize.tellg() / size;
	fileSize.close();
	
	dado registro;
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

void datSwap(string name, int x, int y)
{
	fstream r;
	r.open(name, ios::in | ios::out | ios::binary);
	
	int size = DATA_SIZE;
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

void datInsert(string name, int x)
{
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
	
	dado registro;

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

void datChange(string name, int x)
{
	fstream r;
	r.open(name, ios::in | ios::out | ios::binary);
	
	int size = DATA_SIZE;
	int xPos = size * x;

	ifstream fileSize(name, ios::binary | ios::ate);
	
	size = fileSize.tellg() / size;
	fileSize.close();
	
	dado registro;

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

bool checkSort(int mode, string nome)
{	
	int i = 0;
	fstream read;
	read.open(nome, ios::in | ios::binary);
	
	double value;
	dado registro;
	
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

void printDataValue(string arquivoDat)
{
	cout << "======================================" << endl;
	cout << "           EXPORTACAO .TXT            " << endl;
	cout << "======================================" << endl;
	fstream read;
	ofstream output("./registro/dataValue.txt");
	read.open(arquivoDat, ios::in | ios::binary);
	
	dado registro;
	
	while(read.read((char*)&registro, DATA_SIZE))
	{
		output << registro.dataValue << endl;
	}
	
	output.close();
	read.close();
	
	cout << "Documento Exportado: ./registro/dataValue.txt" << endl;
}

// Tela Principal
void telaMain(string &arquivoDat)
{
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
	cout << "| 5. Imprimir Registro Completo      |" << endl;
	cout << "--------------------------------------" << endl;
	cout << "| 6. Intercalacao Polifasica         |" << endl;
	cout << "--------------------------------------" << endl;
	cout << "| 7. Checar Ordenacao                |" << endl;
	cout << "--------------------------------------" << endl;
	cout << "| 8. Exportar dataValue              |" << endl;
	cout << "--------------------------------------" << endl;
	cout << "| 9. Sair ou Mudar Arquivo .dat      |" << endl;
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
		break;
	default:
		cout << "======================================" << endl;
		cout << "             OPCAO INVALIDA           " << endl;
		cout << "======================================" << endl;
		break;
	}
}

// Main
int main()
{
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
