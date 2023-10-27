#include <fstream>
#include <cstring>
#include "chip8.hpp"



chip8::chip8() : randGen(std::chrono::system_clock::now().time_since_epoch().count())
{
    program_counter = START_ADDRESS;
    for (unsigned int i = 0; i < FONTSET_SIZE; ++i)
    {
        memory[FONTSET_START_ADDRESS + i] = fontset[i];
    }
    
    randByte = std::uniform_int_distribution<uint8_t>(0, 255U);

    table[0x0] = &chip8::Table0;
    table[0x1] = &chip8::OP_1nnn_JP;
    table[0x2] = &chip8::OP_2nnn_CAL;
    table[0x3] = &chip8::OP_3xkk_SE;
    table[0x4] = &chip8::OP_4xkk_SNE;
    table[0x5] = &chip8::OP_5xy0_SE;
    table[0x6] = &chip8::OP_6xkk_LD;
    table[0x7] = &chip8::OP_7xkk_ADD;
    table[0x8] = &chip8::Table8;
    table[0x9] = &chip8::OP_9xy0_SNE;
    table[0xA] = &chip8::OP_Annn_LD;
    table[0xB] = &chip8::OP_Bnnn_JP;
    table[0xC] = &chip8::OP_Cxkk_RND;
    table[0xD] = &chip8::OP_Dxyn_DRW;
    table[0xE] = &chip8::TableE;
    table[0xF] = &chip8::TableF;

    table0[0x0] = &chip8::OP_00E0_CLS;
    table0[0xE] = &chip8::OP_00EE_RET;

    table8[0x0] = &chip8::OP_8xy0_LD;
    table8[0x1] = &chip8::OP_8xy1_OR;
    table8[0x2] = &chip8::OP_8xy2_AND;
    table8[0x3] = &chip8::OP_8xy3_XOR;
    table8[0x4] = &chip8::OP_8xy4_ADD;
    table8[0x5] = &chip8::OP_8xy5_SUB;
    table8[0x6] = &chip8::OP_8xy6_SHR;
    table8[0x7] = &chip8::OP_8xy7_SUBN;
    table8[0xE] = &chip8::OP_8xyE_SHL;

    tableE[0x1] = &chip8::OP_ExA1_SKNP;
    tableE[0xE] = &chip8::OP_Ex9E_SKP;

    tableF[0x07] = &chip8::OP_Fx07_LD;
    tableF[0x0A] = &chip8::OP_Fx0A_LD;
    tableF[0x15] = &chip8::OP_Fx15_LD;
    tableF[0x18] = &chip8::OP_Fx18_LD;
    tableF[0x1E] = &chip8::OP_Fx1E_ADD;
    tableF[0x29] = &chip8::OP_Fx29_LD;
    tableF[0x33] = &chip8::OP_Fx33_LD;
    tableF[0x55] = &chip8::OP_Fx55_LD;
    tableF[0x65] = &chip8::OP_Fx65_LD;

}

void chip8::load_rom(std::string filename)
{
    //Read as binary and move pointer to eof
    std::ifstream file(filename, std::ios::binary | std::ios::ate);

    if(file.is_open())
    {
        //Get size of file and create memory buffer of size
        std::streampos size = file.tellg();
        char* buffer = new char[size];

        //Move to start of file and read contents into buffer
        file.seekg(0, std::ios::beg);
        file.read(buffer,size);
        file.close();

        //Copy into chip8 memory and delete buffer
        for(long i = 0; i < size; ++i)
        {
            memory[START_ADDRESS + i] = buffer[i];
        }

        delete[] buffer;
    }
}

void chip8::OP_00E0_CLS()
{
    std::memset(display_memory, 0, sizeof(display_memory));
}

void chip8::OP_00EE_RET()
{
    stack_pointer -= 1;
    program_counter = stack[stack_pointer];
}

void chip8::OP_1nnn_JP()
{
    uint16_t address = opcode & 0x0FFFu;
    program_counter = address;
}

void chip8::OP_2nnn_CAL()
{
    uint16_t address = opcode & 0x0FFFu;
    stack[stack_pointer] = program_counter;
    stack_pointer += 1;
    program_counter = address;
}

void chip8::OP_3xkk_SE()
{
    uint8_t Vx = (opcode & 0x0F00u) >> 8u;
    uint8_t byte = opcode & 0x00FFu;

    if (registers[Vx] == byte)
    {
        program_counter += 2;
    }
}

void chip8::OP_4xkk_SNE()
{
    uint8_t Vx = (opcode & 0x0F00u) >> 8u;
    uint8_t byte = opcode & 0x00FFu;

    if (registers[Vx] != byte)
    {
        program_counter += 2;
    }
}

void chip8::OP_5xy0_SE()
{
    uint8_t Vx = (opcode & 0x0F00u) >> 8u;
    uint8_t Vy = (opcode & 0x00F0u) >> 4u;

    if(registers[Vx] == registers[Vy])
    {
        program_counter += 2;
    }
}

void chip8::OP_6xkk_LD()
{
    uint8_t Vx = (opcode & 0x0F00u) >> 8u;
    uint8_t byte = (opcode & 0x00FFu);

    registers[Vx] = byte;
}

void chip8::OP_7xkk_ADD()
{
    uint8_t Vx = (opcode & 0x0F00u) >> 8u;
    uint8_t byte = (opcode & 0x00FFu);

    registers[Vx] += byte;
}

void chip8::OP_8xy0_LD()
{
    uint8_t Vx = (opcode & 0x0F00u) >> 8u;
    uint8_t Vy = (opcode & 0x00F0u) >> 4u;

    registers[Vx] = registers[Vy];

}

void chip8::OP_8xy1_OR()
{
    uint8_t Vx = (opcode & 0x0F00u) >> 8u;
    uint8_t Vy = (opcode & 0x00F0u) >> 4u;

    registers[Vx] = registers[Vx] | registers[Vy];
}

void chip8::OP_8xy2_AND()
{
    uint8_t Vx = (opcode & 0x0F00u) >> 8u;
    uint8_t Vy = (opcode & 0x00F0u) >> 4u;

    registers[Vx] = registers[Vx] & registers[Vy];
}

void chip8::OP_8xy3_XOR()
{
    uint8_t Vx = (opcode & 0x0F00u) >> 8u;
    uint8_t Vy = (opcode & 0x00F0u) >> 4u;

    registers[Vx] = registers[Vx] ^ registers[Vy];
}

void chip8::OP_8xy4_ADD()
{
    uint8_t Vx = (opcode & 0x0F00u) >> 8u;
    uint8_t Vy = (opcode & 0x00F0u) >> 4u;

    uint16_t temp_result = registers[Vx] + registers[Vy];
    uint8_t carry = (temp_result & 0xFF00) >> 8u;
    uint8_t result = (temp_result & 0x00FF);

    if(carry != 0)
    {
        registers[15] = 1;
    }else
    {
        registers[15] = 0;
    }
    registers[Vx] = result;
}

void chip8::OP_8xy5_SUB()
{
    uint8_t Vx = (opcode & 0x0F00u) >> 8u;
    uint8_t Vy = (opcode & 0x00F0u) >> 4u;

    if(registers[Vx] > registers[Vy])
    {
        registers[15] = 1;
    }
    else
    {
        registers[15] = 0;
    }

    registers[Vx] = registers[Vx] - registers[Vy];
}

void chip8::OP_8xy6_SHR()
{
    uint8_t Vx = (opcode & 0x0F00u) >> 8u;
    if(registers[Vx] & 0x01u == 1)
    {
        registers[15] = 1;
    }
    else
    {
        registers[15] = 0;
    }

    registers[Vx] = registers[Vx] >> 1;
}

void chip8::OP_8xy7_SUBN()
{
    uint8_t Vx = (opcode & 0x0F00u) >> 8u;
    uint8_t Vy = (opcode & 0x00F0u) >> 4u;

    if(registers[Vy] > registers[Vx])
    {
        registers[15] = 1;
    }
    else
    {
        registers[15] = 0;
    }

    registers[Vx] = registers[Vy] - registers[Vx];
}

// void chip8::OP_8xyE_SHL()
// {
//     uint8_t Vx = (opcode & 0x0F00u) >> 8u;

//     if(registers[Vx] & 0x40 == 1)
//     {
//         registers[15] = 1;
//     }
//     else
//     {
//         registers[15] = 0;
//     }

//     registers[Vx] = registers[Vx] << 1;
    
// }

void chip8::OP_8xyE_SHL()
{
	uint8_t Vx = (opcode & 0x0F00u) >> 8u;

	// Save MSB in VF
	registers[0xF] = (registers[Vx] & 0x80u) >> 7u;

	registers[Vx] <<= 1;
}


void chip8::OP_9xy0_SNE()
{
    uint8_t Vx = (opcode & 0x0F00u) >> 8u;
    uint8_t Vy = (opcode & 0x00F0u) >> 4u;

    if(registers[Vx] != registers[Vy])
    {
        program_counter += 2;
    }
}

void chip8::OP_Annn_LD()
{
    uint16_t address = (opcode & 0x0FFFu);
    index_register = address;
}

void chip8::OP_Bnnn_JP()
{
    uint16_t address = (opcode & 0x0FFFu);
    program_counter = address + registers[0];
}

void chip8::OP_Cxkk_RND()
{
    uint8_t Vx = (opcode & 0x0F00u) >> 8u;
    uint8_t byte = (opcode & 0x00FFu);

    registers[Vx] = randByte(randGen) & byte;
}

// void chip8::OP_Dxyn_DRW()
// {
//     uint8_t Vx = (opcode & 0x0F00u) >> 8u;
//     uint8_t Vy = (opcode & 0x00F0u) >> 4u;
//     uint8_t height = (opcode & 0x000Fu);

//     uint8_t xPos = registers[Vx] % VIDEO_WIDTH;
//     uint8_t yPos = registers[Vy] % VIDEO_HEIGHT;

//     registers[15] = 0;
//     for(unsigned int row = 0; row < height; row++)
//     {
//         uint8_t spriteByte = memory[index_register + row];

//         for(unsigned int col = 0; col < 8; col++)
//         {
//             uint8_t spritePixel = spriteByte & (0x80u >> col);
//             uint32_t* screenPixel = &display_memory[(yPos + row) * VIDEO_WIDTH + (xPos + col)];

//             if(spritePixel)
//             {
//                 if(*screenPixel == 0xFFFFFFFF)
//                 {
//                     registers[15] = 1;
//                 }
//                 *screenPixel ^= 0xFFFFFFFF;
//             }
//         }
//     }
// }
void chip8::OP_Dxyn_DRW()
{
	uint8_t Vx = (opcode & 0x0F00u) >> 8u;
	uint8_t Vy = (opcode & 0x00F0u) >> 4u;
	uint8_t height = opcode & 0x000Fu;

	// Wrap if going beyond screen boundaries
	uint8_t xPos = registers[Vx] % VIDEO_WIDTH;
	uint8_t yPos = registers[Vy] % VIDEO_HEIGHT;

	registers[0xF] = 0;

	for (unsigned int row = 0; row < height; ++row)
	{
		uint8_t spriteByte = memory[index_register + row];

		for (unsigned int col = 0; col < 8; ++col)
		{
			uint8_t spritePixel = spriteByte & (0x80u >> col);
			uint32_t* screenPixel = &display_memory[(yPos + row) * VIDEO_WIDTH + (xPos + col)];

			// Sprite pixel is on
			if (spritePixel)
			{
				// Screen pixel also on - collision
				if (*screenPixel == 0xFFFFFFFF)
				{
					registers[0xF] = 1;
				}

				// Effectively XOR with the sprite pixel
				*screenPixel ^= 0xFFFFFFFF;
			}
		}
	}
}

void chip8::OP_Ex9E_SKP()
{
    uint8_t Vx = (opcode & 0x0F00u) >> 8u;
    uint8_t key = registers[Vx];

    if (input_keys[key])
    {
        program_counter += 2;
    }

}

void chip8::OP_ExA1_SKNP()
{
    uint8_t Vx = (opcode & 0x0F00u) >> 8u;
    uint8_t key = registers[Vx];

    if (!input_keys[key])
    {
        program_counter += 2;
    }
}

void chip8::OP_Fx07_LD()
{
    uint8_t Vx = (opcode & 0x0F00u) >> 8u;
    registers[Vx] = delay_timer;

}

void chip8::OP_Fx0A_LD()
{
    uint8_t Vx = (opcode & 0x0F00u) >> 8u;

	if (input_keys[0])
	{
		registers[Vx] = 0;
	}
	else if (input_keys[1])
	{
		registers[Vx] = 1;
	}
	else if (input_keys[2])
	{
		registers[Vx] = 2;
	}
	else if (input_keys[3])
	{
		registers[Vx] = 3;
	}
	else if (input_keys[4])
	{
		registers[Vx] = 4;
	}
	else if (input_keys[5])
	{
		registers[Vx] = 5;
	}
	else if (input_keys[6])
	{
		registers[Vx] = 6;
	}
	else if (input_keys[7])
	{
		registers[Vx] = 7;
	}
	else if (input_keys[8])
	{
		registers[Vx] = 8;
	}
	else if (input_keys[9])
	{
		registers[Vx] = 9;
	}
	else if (input_keys[10])
	{
		registers[Vx] = 10;
	}
	else if (input_keys[11])
	{
		registers[Vx] = 11;
	}
	else if (input_keys[12])
	{
		registers[Vx] = 12;
	}
	else if (input_keys[13])
	{
		registers[Vx] = 13;
	}
	else if (input_keys[14])
	{
		registers[Vx] = 14;
	}
	else if (input_keys[15])
	{
		registers[Vx] = 15;
	}
	else
	{
		program_counter -= 2;
	}
}
        
void chip8::OP_Fx15_LD()
{
    uint8_t Vx = (opcode & 0x0F00u) >> 8u;
    delay_timer = registers[Vx];
}

void chip8::OP_Fx18_LD()
{
    uint8_t Vx = (opcode & 0x0F00u) >> 8u;
    sound_timer = registers[Vx];
}

void chip8::OP_Fx1E_ADD()
{
    uint8_t Vx = (opcode & 0x0F00u) >> 8u;
    index_register += registers[Vx];
}

void chip8::OP_Fx29_LD()
{
    uint8_t Vx = (opcode & 0x0F00u) >> 8u;
    uint8_t digit = registers[Vx];
    index_register = FONTSET_START_ADDRESS + (5 * digit);
}

void chip8::OP_Fx33_LD()
{
     uint8_t Vx = (opcode & 0x0F00u) >> 8u;
     uint8_t val = registers[Vx];
     memory[index_register + 2] = val%10;
     val = val/10;
     memory[index_register + 1] = val%10;
     val = val/10;
     memory[index_register] = val;
}

void chip8::OP_Fx55_LD()
{
    uint8_t Vx = (opcode & 0x0F00u) >> 8u;
    for(uint8_t i = 0; i < Vx + 1; i++)
    {
        memory[index_register + i] = registers[i];
    }
}

void chip8::OP_Fx65_LD()
{
    uint8_t Vx = (opcode & 0x0F00u) >> 8u;
    for(uint8_t i = 0; i < Vx + 1; i++)
    {
        registers[i] = memory[index_register + i];
    }
}

void chip8::cycle()
{
    opcode = memory[program_counter] << 8u | memory[program_counter+1];
    program_counter += 2;
    ((*this).*(table[(opcode & 0xF000u) >> 12u])) ();

    if(delay_timer > 0)
    {
        delay_timer -= 1;
    }

    if(sound_timer > 0)
    {
        sound_timer -= 1;
    }
}