#include <cstdint>
#include <string>
#include <chrono>
#include <random>

const unsigned int START_ADDRESS = 0x200;
const unsigned int FONTSET_SIZE = 80;
const unsigned int FONTSET_START_ADDRESS = 0x050;
const unsigned int VIDEO_HEIGHT = 32;
const unsigned int VIDEO_WIDTH = 64;
const uint8_t fontset[FONTSET_SIZE] =
{
    0xF0, 0x90, 0x90, 0x90, 0xF0, // 0
	0x20, 0x60, 0x20, 0x20, 0x70, // 1
	0xF0, 0x10, 0xF0, 0x80, 0xF0, // 2
	0xF0, 0x10, 0xF0, 0x10, 0xF0, // 3
	0x90, 0x90, 0xF0, 0x10, 0x10, // 4
	0xF0, 0x80, 0xF0, 0x10, 0xF0, // 5
	0xF0, 0x80, 0xF0, 0x90, 0xF0, // 6
	0xF0, 0x10, 0x20, 0x40, 0x40, // 7
	0xF0, 0x90, 0xF0, 0x90, 0xF0, // 8
	0xF0, 0x90, 0xF0, 0x10, 0xF0, // 9
	0xF0, 0x90, 0xF0, 0x90, 0x90, // A
	0xE0, 0x90, 0xE0, 0x90, 0xE0, // B
	0xF0, 0x80, 0x80, 0x80, 0xF0, // C
	0xE0, 0x90, 0x90, 0x90, 0xE0, // D
	0xF0, 0x80, 0xF0, 0x80, 0xF0, // E
	0xF0, 0x80, 0xF0, 0x80, 0x80  // F
};

class chip8
{
    private:
        uint8_t registers[16] {};
        uint8_t memory [4096] {};
        uint16_t index_register {};
        uint16_t program_counter {};
        uint16_t stack[16] {};
        uint8_t stack_pointer {};
        uint8_t delay_timer {};
        uint8_t sound_timer {};

        uint16_t opcode {};

        std::default_random_engine randGen {};
        std::uniform_int_distribution<uint8_t> randByte {};

        typedef void (chip8::*chip8Func)();
        chip8Func table[0xF + 1]{&chip8::OP_NULL};
        chip8Func table0[0xE + 1]{&chip8::OP_NULL};
        chip8Func table8[0xE + 1]{&chip8::OP_NULL};
        chip8Func tableE[0xE + 1]{&chip8::OP_NULL};
        chip8Func tableF[0x65 + 1]{&chip8::OP_NULL};
    public:
        chip8();
        void load_rom(std::string filename);

        void OP_00E0_CLS();
        void OP_00EE_RET();
       
        void OP_1nnn_JP();
        void OP_2nnn_CAL();
        void OP_3xkk_SE();
        void OP_4xkk_SNE();
        void OP_5xy0_SE();
        void OP_6xkk_LD();
        void OP_7xkk_ADD();
        void OP_9xy0_SNE();
        void OP_Annn_LD();
        void OP_Bnnn_JP();
        void OP_Cxkk_RND();
        void OP_Dxyn_DRW();

        void OP_8xy0_LD();
        void OP_8xy1_OR();
        void OP_8xy2_AND();
        void OP_8xy3_XOR();
        void OP_8xy4_ADD();
        void OP_8xy5_SUB();
        void OP_8xy6_SHR();
        void OP_8xy7_SUBN();
        void OP_8xyE_SHL();
        
        void OP_Ex9E_SKP();
        void OP_ExA1_SKNP();
        void OP_Fx07_LD();
        void OP_Fx0A_LD();
        void OP_Fx15_LD();
        void OP_Fx18_LD();
        void OP_Fx1E_ADD();
        void OP_Fx29_LD();
        void OP_Fx33_LD();
        void OP_Fx55_LD();
        void OP_Fx65_LD();

        void Table0()
        {
            ((*this).*(table0[opcode & 0x000Fu]))();
        }

        void Table8()
        {
            ((*this).*(table8[opcode & 0x000Fu]))();
        }

        void TableE()
        {
            ((*this).*(tableE[opcode & 0x000Fu]))();
        }

        void TableF()
        {
            ((*this).*(tableF[opcode & 0x00FFu]))();
        }

        void OP_NULL() {}

        void cycle();

        uint32_t display_memory[64*32] {};
        uint8_t input_keys[16] {};

};
