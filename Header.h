#pragma once

#include <iostream>
#include <chrono>
#include <thread>
#include <fstream>

uint8_t  V[16];
unsigned I; //index register
unsigned short PC = 0x0; //program counter/instruction pointer

bool gfx[64 * 32];//vram
unsigned char memory[4096];//ram

unsigned char delay_timer;
unsigned char sound_timer;

unsigned short stack[16];
uint8_t sp;
unsigned char key[16];
bool draw = false;

bool program = true;

unsigned char chars[] = {
	'1', '2', '3', 'C',
	'4', '5,', '6', 'D',
	'7', '8', '9', 'E',
	'A', '0', 'B', 'F'
};

unsigned char font[] = {
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

bool load_rom(std::string path) {
	std::ifstream f(path, std::ifstream::in | std::ifstream::binary);

	if (!f.is_open()) {
		std::cout << "its not open";
		return false;
	}

	char c;
	int a = 512; //0x200 == 512, 0x200 is where chip8 program starts

	for (int i = 0x200; f.get(c); i++) {
		if (a >= 4096) {
			std::cout << "bigger than 4096";
			return false;
		}

		memory[i] = (uint8_t)c;
		a++;
	}
	//f.close();
	return true;
}

void executeInstr(short instr) {
	short first = (instr >> 12) & 0x00F;
	short reg;
	short last = instr & 0x000F;
	unsigned short value, NNN, regx, regy, newLoc, newreg;
	regx = (instr >> 8) & 0x0F;
	regy = (instr >> 4) & 0x0F;
	short SecondToLast = (instr >> 4) & 0x000F;
	switch (first) {
	case 0:
		if (instr == 0x00E0) {
			memset(gfx, 0, sizeof(gfx));
			draw = true;
			//clear screen, set all pixels to 0
		}
		else if (instr == 0x00EE) {
			//std::cout << "ee\n";

			sp--;
			PC = stack[sp];
			PC += 2;
			//return from a subroutine, The interpreter sets the program counter to the address at the top of the stack, then subtracts 1 from the stack pointer.
		}
		break;
	case 1: //1nnn - JP addr
		newLoc = instr & 0x0FFF;
		PC = newLoc;
		std::cout << "JMP INSTRUCTION" << std::endl;
		std::cout << "PC: " << std::hex << PC << std::endl;
		std::cout << "JP TO : " << std::hex << newLoc << std::endl;
		std::cout <<"INSTR: " << std::hex << instr << std::endl;
		std::dec;
		break;
	case 2: //2nnn - CALL addr, nnn= addr
		stack[sp] = PC;
		sp++;
		PC = instr & 0x0FFF; // since first bit is 1 & 0 and not 1 & 1 it just becomes 0 so 2nnn becomes 0nnn
		break;
	case 3://3xkk skip next instruction if Vx == kk
		reg = (instr >> 8) & 0x0F;
		value = instr & 0x0FF;
		if (V[reg] == value) {
			PC += 2;
		}
		break;
	case 4://The interpreter compares register Vx to kk, and if they are not equal, increments the program counter by 2.
		reg = (instr >> 8) & 0x0F;
		value = instr & 0x0FF;
		if (V[reg] != value) {
			PC += 2;
		}
		break;
	case 5://The interpreter compares register Vx to register Vy, and if they are equal, increments the program counter by 2.
		regy = (instr >> 4) & 0x0F;
		regx = (instr >> 8) & 0x0F;
		if (V[regx] == V[regy]) {
			PC += 2;
		}
		break;
	case 6://The interpreter puts the value kk into register Vx.
		reg = (instr >> 8) & 0x0F;
		value = instr & 0x0FF;
		std::cout << "value: " << value << std::endl;
		V[reg] = value;
		break;
	case 7://Adds the value kk to the value of register Vx, then stores the result in Vx. 
		reg = (instr >> 8) & 0x0F;
		value = instr & 0x0FF;
		V[reg] = V[reg] + value;
		break;
	case 8:
	
		switch (last) {
		case 0: //Stores the value of register Vy in register Vx. 8xy0
			regy = (instr >> 4) & 0x0F;
			regx = (instr >> 8) & 0x0F;
			V[regx] = V[regy];
			break;
		case 1://Performs a bitwise OR on the values of Vx and Vy, then stores the result in Vx. A bitwise OR compares the corrseponding bits from two values, and if either bit is 1, then the same bit in the result is also 1. Otherwise, it is 0. 
			regy = (instr >> 4) & 0x0F;
			regx = (instr >> 8) & 0x0F;
			newreg = V[regx] | V[regy];
			V[regx] = newreg;
			break;
		case 2://Performs a bitwise AND on the values of Vx and Vy, then stores the result in Vx. A bitwise AND compares the corrseponding bits from two values, and if both bits are 1, then the same bit in the result is also 1. Otherwise, it is 0. 
			regy = (instr >> 4) & 0x0F;
			regx = (instr >> 8) & 0x0F;
			newreg = V[regx] & V[regy];
			V[regx] = newreg;
			break;
		case 3://Performs a bitwise exclusive OR on the values of Vx and Vy, then stores the result in Vx. An exclusive OR compares the corrseponding bits from two values, and if the bits are not both the same, then the corresponding bit in the result is set to 1. Otherwise, it is 0. 
			regy = (instr >> 4) & 0x0F;
			regx = (instr >> 8) & 0x0F;
			newreg = V[regx] ^ V[regy];
			V[regx] = newreg;
			break;
		case 4://The values of Vx and Vy are added together. If the result is greater than 8 bits (i.e., > 255,) VF is set to 1, otherwise 0. Only the lowest 8 bits of the result are kept, and stored in Vx.
			regy = (instr >> 4) & 0x0F;
			regx = (instr >> 8) & 0x0F;
			newreg = (uint8_t)V[regy] + (uint8_t)V[regx];
			if (newreg > 255) {

				V[0xF] = 1;
			}
			else {
				V[0xF] = 0;
			}
			V[regx] = (int8_t)newreg;
			break;
		case 5://If Vx > Vy, then VF is set to 1, otherwise 0. Then Vy is subtracted from Vx, and the results stored in Vx.
			regy = (instr >> 4) & 0x0F;
			regx = (instr >> 8) & 0x0F;
			if (V[regx] > V[regy]) {
				V[0xF] = 1;
			}
			else {
				V[0xF] = 0;
			}
			V[regx] = (uint8_t)V[regx] - (uint8_t)V[regy];
			break;
		case 6://Store the value of register VY shifted right one bit in register VX Set register VF to the least significant bit prior to the shift VY is unchanged
			regx = (instr >> 8) & 0x0F;
			regy = (instr >> 4) & 0x0F;
			V[0xF] = V[regx] & 0x1;
			V[regx] = V[regy] >> 0x1;
			V[regx] = (uint8_t)V[regx];
			break;
		case 7://Set register VX to the value of VY minus VX Set VF to 00 if a borrow occurs Set VF to 01 if a borrow does not occur
			regy = (instr >> 4) & 0x0F;
			regx = (instr >> 8) & 0x0F;
			if (V[regx] < V[regy]) {
				V[0xF] = 1;
			}
			else {
				V[0xF] = 0;
			}
			V[regx] = (uint8_t)V[regy] - (uint8_t)V[regx];
			break;
		case 0xE:/*Store the value of register VY shifted left one bit in register VX¹
						Set register VF to the most significant bit prior to the shift
						VY is unchanged*/
			regx = (instr >> 8) & 0x0F;
			regy = (instr >> 4) & 0x0F;
			V[0xF] = (uint8_t)(V[regx] >> 7);
			V[regx] = V[regy] << 0x1;
			V[regx] = (uint8_t)V[regx];
			break;
		default:
			std::cout << "NOT FOUND: " << std::hex << instr << std::endl;
			std::dec;
			break;
		}

		break;
	case 9://Skip the following instruction if the value of register VX is not equal to the value of register VY
		regx = (instr >> 8) & 0x0F;
		regy = (instr >> 4) & 0x0F;
		if (V[regx] != V[regy]) {
			PC += 2;
		}
		break;
	case 0xA://Store memory address NNN in register I
		NNN = instr & 0x0FFF;
		I = NNN;
		break;
	case 0xB://Jump to address NNN + V0
		NNN = instr & 0x0FFF;
		newLoc = NNN + V[0];
		std::cout << "New Loc: " << std::hex << newLoc << std::endl;
		std::cout << "NNN: " << std::hex << NNN << std::endl;
		std::cout << "V0: " << std::hex << V[0] << std::endl;
		PC += newLoc;
		std::cout << "PC: " << std::hex << PC << std::endl;
		break;
	case 0xC://Set VX to a random number with a mask of NN
		regx = (instr >> 8) & 0x0F;
		value = instr & 0x00FF;
		V[regx] = (value & 2);
		break;
	case 0xD:/*Draw a sprite at position VX, VY with N bytes of sprite data starting at the address stored in I
			   Set VF to 01 if any set pixels are changed to unset, and 00 otherwise*/
	{
		//std::cout << "Drawing\n";
		regx = (instr >> 8) & 0x0F;
		regy = (instr >> 4) & 0x0F;
		int x = V[regx] % 64;
		int y = V[regy] % 32;

		int wt = 8;
		int N = instr & 0x000F; // N ==hieght
		V[0xF] = 0;
		for (int i = 0; i < N; i++) {
			int pixel = memory[I + i];
			for (int A = 0; A < wt; A++) {
				if (pixel & (0x80 >> x)) {
					int pix = ((x + A) + ((y + i) * 64)) % 2048;
					//std::cout << pix << std::endl;
					if (gfx[pix] == 1) {
						V[0xF] = 1;
					}
					gfx[pix] ^= 1;
				}
			}
		}
		draw = true;
		break;
	}
	case 0xE://EX9E & EXA1
		regx = (instr >> 8) & 0x0F;
		//TODO
		break;
	case 0xF://Store the current value of the delay timer in register VX
		switch (last) {
		case 7://Store the current value of the delay timer in register VX
			regx = (instr >> 8) & 0x0F;
			V[regx] = delay_timer;
			break;
		case 0xA://Wait for a keypress and store the result in register VX
			//TODO
			break;
		case 0x8://Set the sound timer to the value of register VX
			sound_timer = V[regx];
			break;
		case 0xE://Add the value stored in register VX to register I
			I = V[regx] + I;
			break;
		case 9://Set I to the memory address of the sprite data corresponding to the hexadecimal digit stored in register VX
			//TOOD
			break;
		case 3://Store the binary-coded decimal equivalent of the value stored in register VX at addresses I, I + 1, and I + 2
			//TODO
			break;
		case 5:
			switch (SecondToLast) {
			case 1://set delay_timer to regx
				delay_timer = V[regx];
				break;
			case 5://Store the values of registers V0 to VX inclusive in memory starting at address I, I is set to I + X + 1 after operation²
				//TODO
				break;
			case 6://Fill registers V0 to VX inclusive with the values stored in memory starting at address I, I is set to I + X + 1 after operation²
				//TODO
				break;
			default:
				std::cout << "NOT FOUND: " << std::hex << instr << std::endl;
				std::dec;
				break;
			}
			break;
		default:
			std::cout << "NOT FOUND: " << std::hex << instr << std::endl;
			std::dec;
			break;
		}


		break;
	default:
		std::cout << "NOT FOUND: " << std::hex<<instr<<std::endl;
		std::dec;
		break;
	}

}
