#include <iostream>
#include <fstream>
#include <vector>
#include <iomanip>
#include <cstdint>
#include <unordered_map>
#include <string>
#include <sstream>

using namespace std;

class RV32ISimulator {
private:
    uint32_t pc;
    uint32_t regs[32];
    unordered_map<uint32_t, uint8_t> memory;

    uint32_t read_word(uint32_t addr) {
        uint32_t b0 = memory[addr];
        uint32_t b1 = memory[addr + 1];
        uint32_t b2 = memory[addr + 2];
        uint32_t b3 = memory[addr + 3];
        return b0 | (b1 << 8) | (b2 << 16) | (b3 << 24);
    }

    void write_word(uint32_t addr, uint32_t val) {
        memory[addr] = val & 0xFF;
        memory[addr + 1] = (val >> 8) & 0xFF;
        memory[addr + 2] = (val >> 16) & 0xFF;
        memory[addr + 3] = (val >> 24) & 0xFF;
    }

public:
    RV32ISimulator() {
        pc = 0x00000000;
        for (int i = 0; i < 32; ++i) regs[i] = 0;
    }

    bool load_program(const string& filename) {
        ifstream file(filename, ios::binary);
        if (!file) {
            cerr<<"No se pudo abrir el archivo "<<filename<<endl;
            return false;
        }

        uint32_t addr = 0x00000000;
        char byte;
        while (file.get(byte)) {
            memory[addr++] = static_cast<uint8_t>(byte);
        }
        cout << "\"" << filename << "\" cargado a memoria." << endl;
        return true;
    }

    // Paso de ciclo
    void step() {
        if (memory.find(pc) == memory.end()) {
            cout << "Error: PC apunta a memoria no inicializada (x" << hex << pc << ")" << endl;
            return;
        }

        // 1. Fetch
        uint32_t inst = read_word(pc);
        pc += 4; // Avanzar PC

        // 2. Decode
        uint32_t opcode = inst & 0x7F;
        uint32_t rd     = (inst >> 7) & 0x1F;
        uint32_t funct3 = (inst >> 12) & 0x7;
        uint32_t rs1    = (inst >> 15) & 0x1F;
        uint32_t rs2    = (inst >> 20) & 0x1F;
        uint32_t funct7 = (inst >> 25) & 0x7F;

        // ImmExtend
        int32_t immI = ((int32_t)inst) >> 20;
        int32_t immS = (((int32_t)(inst & 0xFE000000)) >> 20) | ((inst >> 7) & 0x1F);
        int32_t immB = (((int32_t)(inst & 0x80000000)) >> 19) | ((inst & 0x7E000000) >> 20) |
                       ((inst >> 7) & 0x1E) | ((inst << 4) & 0x800);
        int32_t immU = inst & 0xFFFFF000;
        int32_t immJ = (((int32_t)(inst & 0x80000000)) >> 11) | (inst & 0xFF000) |
                       ((inst >> 9) & 0x800) | ((inst >> 20) & 0x7FE);

        // 3. Execute
        switch (opcode) {
            case 0x33: // R-type
                if (funct3 == 0x0) {
                    if (funct7 == 0x00) regs[rd] = regs[rs1] + regs[rs2];      // add
                    else if (funct7 == 0x20) regs[rd] = regs[rs1] - regs[rs2]; // sub
                }
                break;
            case 0x13: // I-type ALU
                if (funct3 == 0x0) regs[rd] = regs[rs1] + immI;            // addi
                else if (funct3 == 0x6) regs[rd] = regs[rs1] | immI;       // ori
                else if (funct3 == 0x1) regs[rd] = regs[rs1] << (immI & 0x1F); // slli
                break;
            case 0x37: // U-type (lui)
                regs[rd] = immU;
                break;
            case 0x23: // S-type
                if (funct3 == 0x2) write_word(regs[rs1] + immS, regs[rs2]); // sw
                break;
            case 0x03: // I-type Load
                if (funct3 == 0x2) regs[rd] = read_word(regs[rs1] + immI); // lw
                break;
            case 0x63: // B-type
                if (funct3 == 0x0) { // beq
                    if (regs[rs1] == regs[rs2]) pc = (pc - 4) + immB;
                }
                break;
            case 0x6F: // J-type (jal)
                regs[rd] = pc;
                pc = (pc - 4) + immJ;
                break;
            default:
                cout << "Instruccion no implementada u opcode invalido: x" << hex << opcode << endl;
                break;
        }

        regs[0] = 0;
        cout << "Ejecutando instruccion." << endl;
    }

    void print_pc() {
        cout << "pc x" << setfill('0') << setw(8) << hex << pc << dec << endl;
    }

    void print_regs(const vector<int>& target_regs) {
        for (int r : target_regs) {
            cout << "x" << dec << r << "=0x" << setfill('0') << setw(8) << hex << regs[r] << dec << endl;
        }
    }

    void print_mem(uint32_t start, uint32_t end) {
        cout << "Memoria (x" << hex << start << "-x" << end << "): ";
        for (uint32_t addr = start; addr <= end; ++addr) {
            cout << setfill('0') << setw(2) << hex << (int)memory[addr] << " ";
        }
        cout << dec << endl;
    }
};

// Función principal: CPULator pero con instrucciones fijas y en vivo basicamente
int main(int argc, char* argv[]) {
    if (argc < 2) {
        cerr << "Uso: " << argv[0] << " programa.bin" << endl;
        return 1;
    }

    RV32ISimulator sim;
    if (!sim.load_program(argv[1])) {
        return 1;
    }

    string command_line;
    while (true) {
        cout << "> ";
        if (!getline(cin, command_line)) break;

        stringstream ss(command_line);
        string cmd;
        ss>> cmd;

        if (cmd == "step") {
            sim.step();
        }
        else if (cmd == "pc") {
            sim.print_pc();
        }
        else if (cmd == "regs") {
            string reg;
            vector<int> r_list;
            while (ss >> reg) {
                if (reg[0] == 'x') {
                    r_list.push_back(stoi(reg.substr(1)));
                }
            }
            sim.print_regs(r_list);
        }
        else if (cmd == "mem") {
            string s_start, s_end;
            if (ss >> s_start >> s_end) {
                uint32_t start = stoul(s_start, nullptr, 16);
                uint32_t end = stoul(s_end, nullptr, 16);
                sim.print_mem(start, end);
            }
        }
        else if (cmd == "exit") {
            cout << "See you next time..." << endl;
            cout << "Funciona" << endl;
            break;
        }
    }
    return 0;
}