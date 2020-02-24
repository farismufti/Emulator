/*
Author: Faris Mufti (19044237)
Created 06 Feb. 2020
Revised: 27 Feb. 2020
Description: Chimera 8080 Microprocessor Emulator
User advice: none
*/

#include "stdafx.h"
#include <winsock2.h>

#pragma comment(lib, "wsock32.lib")


#define STUDENT_NUMBER    "19044237"

#define IP_ADDRESS_SERVER "127.0.0.1"

#define PORT_SERVER 0x1984 // We define a port that we are going to use.
#define PORT_CLIENT 0x1985 // We define a port that we are going to use.

#define WORD  unsigned short
#define DWORD unsigned long
#define BYTE  unsigned char

#define MAX_FILENAME_SIZE 500
#define MAX_BUFFER_SIZE   500

SOCKADDR_IN server_addr;
SOCKADDR_IN client_addr;

SOCKET sock;  // This is our socket, it is the handle to the IO address to read/write packets

WSADATA data;

char InputBuffer [MAX_BUFFER_SIZE];

char hex_file [MAX_BUFFER_SIZE];
char trc_file [MAX_BUFFER_SIZE];

//////////////////////////
//   Registers          //
//////////////////////////

#define FLAG_I  0x80
#define FLAG_N  0x20
#define FLAG_V  0x10
#define FLAG_Z  0x08
#define FLAG_C  0x01
#define REGISTER_A	6
#define REGISTER_H	5
#define REGISTER_L	4
#define REGISTER_E	3
#define REGISTER_D	2
#define REGISTER_C	1
#define REGISTER_B	0
#define REGISTER_M  7 //added
#define REGISTER_X  8 //added
#define REGISTER_FL 9 //added
WORD IndexRegister;

BYTE Registers[10]; //changed
BYTE Flags;
WORD ProgramCounter;
WORD StackPointer;


////////////
// Memory //
////////////

#define MEMORY_SIZE	65536

BYTE Memory[MEMORY_SIZE];

#define TEST_ADDRESS_1  0x01FA
#define TEST_ADDRESS_2  0x01FB
#define TEST_ADDRESS_3  0x01FC
#define TEST_ADDRESS_4  0x01FD
#define TEST_ADDRESS_5  0x01FE
#define TEST_ADDRESS_6  0x01FF
#define TEST_ADDRESS_7  0x0200
#define TEST_ADDRESS_8  0x0201
#define TEST_ADDRESS_9  0x0202
#define TEST_ADDRESS_10  0x0203
#define TEST_ADDRESS_11  0x0204
#define TEST_ADDRESS_12  0x0205


///////////////////////
// Control variables //
///////////////////////

bool memory_in_range = true;
bool halt = false;


///////////////////////
// Disassembly table //
///////////////////////

char opcode_mneumonics[][14] =
{
"BRA rel      ", 
"BCC rel      ", 
"BCS rel      ", 
"BNE rel      ", 
"BEQ rel      ", 
"BVC rel      ", 
"BVS rel      ", 
"BMI rel      ", 
"BPL rel      ", 
"BGE rel      ", 
"BLE rel      ", 
"BLS rel      ", 
"BHI rel      ", 
"ILLEGAL     ", 
"RTN impl     ", 
"ILLEGAL     ", 

"ST abs       ", 
"PSH  ,A      ", 
"POP A,       ", 
"ILLEGAL     ", 
"ILLEGAL     ", 
"CLC impl     ", 
"SEC impl     ", 
"CLI impl     ", 
"STI impl     ", 
"SEV impl     ", 
"CLV impl     ", 
"DEX impl     ", 
"INX impl     ", 
"NOP impl     ", 
"WAI impl     ", 
"ILLEGAL     ", 

"ST abs,X     ", 
"PSH  ,s      ", 
"POP s,       ", 
"ILLEGAL     ", 
"ILLEGAL     ", 
"ADI  #       ", 
"SBI  #       ", 
"CPI  #       ", 
"ANI  #       ", 
"XRI  #       ", 
"MVI  #,B     ", 
"MVI  #,C     ", 
"MVI  #,D     ", 
"MVI  #,E     ", 
"MVI  #,L     ", 
"MVI  #,H     ", 

"ILLEGAL     ", 
"PSH  ,B      ", 
"POP B,       ", 
"JPR abs      ", 
"CCC abs      ", 
"CCS abs      ", 
"CNE abs      ", 
"CEQ abs      ", 
"CVC abs      ", 
"CVS abs      ", 
"CMI abs      ", 
"CPL abs      ", 
"CHI abs      ", 
"CLE abs      ", 
"ILLEGAL     ", 
"ILLEGAL     ", 

"ILLEGAL     ", 
"PSH  ,C      ", 
"POP C,       ", 
"TST abs      ", 
"INC abs      ", 
"DEC abs      ", 
"RCR abs      ", 
"RCL abs      ", 
"SAL abs      ", 
"ASR abs      ", 
"NOT abs      ", 
"ROL abs      ", 
"ROR abs      ", 
"ILLEGAL     ", 
"LDX  #       ", 
"LODS  #      ", 

"STOX abs     ", 
"PSH  ,D      ", 
"POP D,       ", 
"TST abs,X    ", 
"INC abs,X    ", 
"DEC abs,X    ", 
"RCR abs,X    ", 
"RCL abs,X    ", 
"SAL abs,X    ", 
"ASR abs,X    ", 
"NOT abs,X    ", 
"ROL abs,X    ", 
"ROR abs,X    ", 
"ILLEGAL     ", 
"LDX abs      ", 
"LODS abs     ", 

"STOX abs,X   ", 
"PSH  ,E      ", 
"POP E,       ", 
"TSTA A,A     ", 
"INCA A,A     ", 
"DECA A,A     ", 
"RCRA A,A     ", 
"RCLA A,A     ", 
"SALA A,A     ", 
"ASRA A,A     ", 
"NOTA A,A     ", 
"ROLA A,A     ", 
"RORA A,A     ", 
"ILLEGAL     ", 
"LDX abs,X    ", 
"LODS abs,X   ", 

"ILLEGAL     ", 
"PSH  ,L      ", 
"POP L,       ", 
"ILLEGAL     ", 
"TAS impl     ", 
"TSA impl     ", 
"ILLEGAL     ", 
"ILLEGAL     ", 
"MOVE A,A     ", 
"MOVE B,A     ", 
"MOVE C,A     ", 
"MOVE D,A     ", 
"MOVE E,A     ", 
"MOVE L,A     ", 
"MOVE H,A     ", 
"MOVE M,A     ", 

"ILLEGAL     ", 
"PSH  ,H      ", 
"POP H,       ", 
"ILLEGAL     ", 
"ILLEGAL     ", 
"SWI impl     ", 
"RTI impl     ", 
"ILLEGAL     ", 
"MOVE A,B     ", 
"MOVE B,B     ", 
"MOVE C,B     ", 
"MOVE D,B     ", 
"MOVE E,B     ", 
"MOVE L,B     ", 
"MOVE H,B     ", 
"MOVE M,B     ", 

"ADC A,B      ", 
"SBC A,B      ", 
"CMP A,B      ", 
"IOR A,B      ", 
"AND A,B      ", 
"XOR A,B      ", 
"BT A,B       ", 
"ILLEGAL     ", 
"MOVE A,C     ", 
"MOVE B,C     ", 
"MOVE C,C     ", 
"MOVE D,C     ", 
"MOVE E,C     ", 
"MOVE L,C     ", 
"MOVE H,C     ", 
"MOVE M,C     ", 

"ADC A,C      ", 
"SBC A,C      ", 
"CMP A,C      ", 
"IOR A,C      ", 
"AND A,C      ", 
"XOR A,C      ", 
"BT A,C       ", 
"ILLEGAL     ", 
"MOVE A,D     ", 
"MOVE B,D     ", 
"MOVE C,D     ", 
"MOVE D,D     ", 
"MOVE E,D     ", 
"MOVE L,D     ", 
"MOVE H,D     ", 
"MOVE M,D     ", 

"ADC A,D      ", 
"SBC A,D      ", 
"CMP A,D      ", 
"IOR A,D      ", 
"AND A,D      ", 
"XOR A,D      ", 
"BT A,D       ", 
"LD  #        ", 
"MOVE A,E     ", 
"MOVE B,E     ", 
"MOVE C,E     ", 
"MOVE D,E     ", 
"MOVE E,E     ", 
"MOVE L,E     ", 
"MOVE H,E     ", 
"MOVE M,E     ", 

"ADC A,E      ", 
"SBC A,E      ", 
"CMP A,E      ", 
"IOR A,E      ", 
"AND A,E      ", 
"XOR A,E      ", 
"BT A,E       ", 
"LD abs       ", 
"MOVE A,L     ", 
"MOVE B,L     ", 
"MOVE C,L     ", 
"MOVE D,L     ", 
"MOVE E,L     ", 
"MOVE L,L     ", 
"MOVE H,L     ", 
"MOVE M,L     ", 

"ADC A,L      ", 
"SBC A,L      ", 
"CMP A,L      ", 
"IOR A,L      ", 
"AND A,L      ", 
"XOR A,L      ", 
"BT A,L       ", 
"LD abs,X     ", 
"MOVE A,H     ", 
"MOVE B,H     ", 
"MOVE C,H     ", 
"MOVE D,H     ", 
"MOVE E,H     ", 
"MOVE L,H     ", 
"MOVE H,H     ", 
"MOVE M,H     ", 

"ADC A,H      ", 
"SBC A,H      ", 
"CMP A,H      ", 
"IOR A,H      ", 
"AND A,H      ", 
"XOR A,H      ", 
"BT A,H       ", 
"ILLEGAL     ", 
"MOVE A,M     ", 
"MOVE B,M     ", 
"MOVE C,M     ", 
"MOVE D,M     ", 
"MOVE E,M     ", 
"MOVE L,M     ", 
"MOVE H,M     ", 
"MOVE -,-     ", 

"ADC A,M      ", 
"SBC A,M      ", 
"CMP A,M      ", 
"IOR A,M      ", 
"AND A,M      ", 
"XOR A,M      ", 
"BT A,M       ", 
"ILLEGAL     ", 
"ILLEGAL     ", 
"ILLEGAL     ", 
"JMP abs      ", 
"ILLEGAL     ", 
"ILLEGAL     ", 
"ILLEGAL     ", 
"ILLEGAL     ", 
"ILLEGAL     ", 

}; 

////////////////////////////////////////////////////////////////////////////////
//                           Simulator/Emulator (Start)                       //
////////////////////////////////////////////////////////////////////////////////
BYTE fetch()
{
	BYTE byte = 0;

	if ((ProgramCounter >= 0) && (ProgramCounter <= MEMORY_SIZE))
	{
		memory_in_range = true;
		byte = Memory[ProgramCounter];
		ProgramCounter++;
	}
	else
	{
		memory_in_range = false;
	}
	return byte;
}

void set_flag_n(BYTE inReg) //Sets negative flag
{
	BYTE reg;
	reg = inReg;

	if ((reg & 0x80) != 0) //msbit set
	{
		Flags = Flags | FLAG_N;
	}
	else
	{
		Flags = Flags & (0xFF - FLAG_N);
	}
}

void set_flag_v(BYTE in1, BYTE in2, BYTE out1) //Sets overflow flag
{
	BYTE reg1in;
	BYTE reg2in;
	BYTE regOut;

	reg1in = in1;
	reg2in = in2;
	regOut = out1;

	if ((((reg1in & 0x80) == 0x80) && ((reg2in & 0x80) == 0x80) && ((regOut & 0x80) != 0x80)) //overflow
		|| (((reg1in & 0x80) != 0x80) && ((reg2in & 0x80) != 0x80) && ((regOut & 0x80) == 0x80))) //overflow
	{
		Flags = Flags | FLAG_V;
	}
	else
	{
		Flags = Flags & (0xFF - FLAG_V);
	}
}

void set_flag_z(BYTE inReg) //Sets zero flag
{
	BYTE reg; 
	reg = inReg; 

	if (reg == 0) //msbit set 
	{ 
		Flags = Flags | FLAG_Z;
	}
	else 
	{ 
		Flags = Flags & (0xFF - FLAG_Z);
	}
}

void set_zn_flags(WORD result) //Sets the zero and negative flags
{
	set_flag_z(result);
	set_flag_n(result);
}

WORD getAddressAbs() //Returns absolute memory address
{
	BYTE HB = 0;
	BYTE LB = 0;
	WORD address = 0;
	HB = fetch();
	LB = fetch();
	address = ((WORD)HB << 8) + LB;
	return address; 
}

WORD getAddressAbsX() //Returns indexed absolute memory address
{
	BYTE HB = 0;
	BYTE LB = 0;
	WORD address = 0;
	HB = fetch();
	LB = fetch();
	address = ((WORD)HB << 8) + LB;
	address = address + IndexRegister;
	return address;
}

void push8(BYTE inReg)
{
	StackPointer--;
	Memory[StackPointer] = inReg;
}

BYTE pop8()
{
	BYTE reg;
	reg = Memory[StackPointer];
	StackPointer++;
	return reg;
}

void push16(WORD inReg)
{
	BYTE LB, HB;
	LB = (BYTE)inReg;
	HB = (BYTE)(inReg >> 8);
	StackPointer--;
	Memory[StackPointer] = HB;
	StackPointer--;
	Memory[StackPointer] = LB;
}

BYTE negate(BYTE inReg)
{
	WORD result = ~inReg + 1;
	set_zn_flags(result);
	return (BYTE)result;
}

void Group_1(BYTE opcode)
{
	
	BYTE LB = 0;
	BYTE HB = 0;
	WORD address = 0;
	WORD data = 0;
	WORD temp_word;
	BYTE param1;
	BYTE param2;
	WORD offset;
	BYTE saved_flags;
	BYTE NF;
	BYTE VF;
	BYTE ZF;
	BYTE CF;
	BYTE temp;

	if ((Flags & FLAG_N) == FLAG_N)
	{
		NF = 1;
	}
	else
	{
		NF = 0;
	}

	if ((Flags & FLAG_V) == FLAG_V)
	{
		VF = 1;
	}
	else
	{
		VF = 0;
	}

	if ((Flags & FLAG_Z) == FLAG_Z)
	{
		ZF = 1;
	}
	else
	{
		ZF = 0;
	}

	if ((Flags & FLAG_C) == FLAG_C)
	{
		CF = 1;
	}
	else
	{
		CF = 0;
	}

	switch (opcode) {

	case 0xB7: //LD Immidiate
		data = fetch();
		Registers[REGISTER_A] = data;
		set_zn_flags(Registers[REGISTER_A]);
		Flags = Flags & (0xFF - FLAG_C);
		break;

	case 0xC7: //LD Absolute
		address = getAddressAbs();

		if (address >= 0 && address < MEMORY_SIZE)
		{
			Registers[REGISTER_A] = Memory[address];
			set_zn_flags(Registers[REGISTER_A]);
			Flags = Flags & (0xFF - FLAG_C);
		}
		break;

	case 0xD7: //LD abs,X
		address = getAddressAbsX();

		if (address >= 0 && address < MEMORY_SIZE)
		{
			Registers[REGISTER_A] = Memory[address];
			set_zn_flags(Registers[REGISTER_A]);
			Flags = Flags & (0xFF - FLAG_C);
		}
		break;

	case 0x10: //ST Absolute
		address = getAddressAbs();

		if (address >= 0 && address < MEMORY_SIZE)
		{
			Memory[address] = Registers[REGISTER_A];
			set_flag_z(Registers[REGISTER_A]);
			set_flag_n(Registers[REGISTER_A]);
			Flags = Flags & (0xFF - FLAG_C);
		}
		break;

	case 0x20: //ST abs,X
		address = getAddressAbsX();

		if (address >= 0 && address < MEMORY_SIZE)
		{
			Memory[address] = Registers[REGISTER_A];
			set_zn_flags(Registers[REGISTER_A]);
			Flags = Flags & (0xFF - FLAG_C);
		}
		break;

	case 0x2A: //MVI Immidiate, B
		data = fetch();
		Registers[REGISTER_B] = data;
		set_zn_flags(Registers[REGISTER_B]);
		Flags = Flags & (0xFF - FLAG_C);
		break;

	case 0x2B: //MVI Immidiate, C
		data = fetch();
		Registers[REGISTER_C] = data;
		set_zn_flags(Registers[REGISTER_C]);
		Flags = Flags & (0xFF - FLAG_C);
		break;

	case 0x2C: //MVI Immidiate, D
		data = fetch();
		Registers[REGISTER_D] = data;
		set_zn_flags(Registers[REGISTER_D]);
		Flags = Flags & (0xFF - FLAG_C);
		break;

	case 0x2D: //MVI Immidiate, E
		data = fetch();
		Registers[REGISTER_E] = data;
		set_zn_flags(Registers[REGISTER_E]);
		Flags = Flags & (0xFF - FLAG_C);
		break;

	case 0x2E: //MVI Immidiate, L
		data = fetch();
		Registers[REGISTER_L] = data;
		set_zn_flags(Registers[REGISTER_L]);
		Flags = Flags & (0xFF - FLAG_C);
		break;

	case 0x2F: //MVI Immidiate, H
		data = fetch();
		Registers[REGISTER_H] = data;
		set_zn_flags(Registers[REGISTER_H]);
		Flags = Flags & (0xFF - FLAG_C);
		break;

	case 0x4F: //LODS Immidiate
		data = fetch();
		StackPointer = data << 8, StackPointer += fetch();
		break;

	case 0x5F: //LODS Absolute
		address = getAddressAbs();

		if (address >= 0 && address < MEMORY_SIZE - 1)
		{
			StackPointer = (WORD)Memory[address] << 8;
			StackPointer += Memory[address + 1];
		}
		break;

	case 0x6F: //LODS abs, X
		address = getAddressAbsX();

		if (address >= 0 && address < MEMORY_SIZE - 1)
		{
			StackPointer = (WORD)Memory[address] << 8;
			StackPointer += Memory[address + 1];
		}
		break;

	case 0x4E: //LDX Immidiate
		data = fetch();
		Registers[REGISTER_X] = data;
		break;

	case 0x5E: //LDX Absolute
		data = fetch();
		Registers[REGISTER_X] = data;
		break;

	case 0x6E: //LDX abs,X
		data = fetch();
		Registers[REGISTER_X] = data;
		break;

	case 0x50: //STOX Absolute
		data = fetch();
		data = Registers[REGISTER_X];
		break;

	case 0x60: //STOX abs,X
		data = fetch();
		data = Registers[REGISTER_X];
		break;


		//Flags
	case 0x90: // ADC A,B
		param1 = Registers[REGISTER_A];
		param2 = Registers[REGISTER_B];
		temp_word = (WORD)Registers[REGISTER_A] + (WORD)Registers[REGISTER_B];
		if ((Flags & FLAG_C) != 0)
		{
			temp_word++;
		}

		if (temp_word >= 0x100)
		{
			Flags = Flags | FLAG_C; //Set carry flag
		}
		else
		{
			Flags = Flags & (0xFF - FLAG_C); //Clear carry flag
		}
		set_zn_flags((BYTE)temp_word);
		set_flag_v(param1, param2, temp_word);
		Registers[REGISTER_A] = (BYTE)temp_word;
		break;

	case 0xA0: // ADC, A-C
		param1 = Registers[REGISTER_A];
		param2 = Registers[REGISTER_C];
		temp_word = (WORD)Registers[REGISTER_A] + (WORD)Registers[REGISTER_C];
		if ((Flags & FLAG_C) != 0)
		{
			temp_word++;
		}

		if (temp_word >= 0x100)
		{
			Flags = Flags | FLAG_C; //Set carry flag
		}
		else
		{
			Flags = Flags & (0xFF - FLAG_C); //Clear carry flag
		}
		set_zn_flags((BYTE)temp_word);
		set_flag_v(param1, param2, temp_word);
		Registers[REGISTER_A] = (BYTE)temp_word;
		break;

	case 0xB0: // ADC A,D
		param1 = Registers[REGISTER_A];
		param2 = Registers[REGISTER_D];
		temp_word = (WORD)Registers[REGISTER_A] + (WORD)Registers[REGISTER_D];
		if ((Flags & FLAG_C) != 0)
		{
			temp_word++;
		}

		if (temp_word >= 0x100)
		{
			Flags = Flags | FLAG_C; //Set carry flag
		}
		else
		{
			Flags = Flags & (0xFF - FLAG_C); //Clear carry flag
		}
		set_zn_flags((BYTE)temp_word);
		set_flag_v(param1, param2, temp_word);
		Registers[REGISTER_A] = (BYTE)temp_word;
		break;

	case 0xC0: // ADC A,E
		param1 = Registers[REGISTER_A];
		param2 = Registers[REGISTER_E];
		temp_word = (WORD)Registers[REGISTER_A] + (WORD)Registers[REGISTER_E];
		if ((Flags & FLAG_C) != 0)
		{
			temp_word++;
		}

		if (temp_word >= 0x100)
		{
			Flags = Flags | FLAG_C; //Set carry flag
		}
		else
		{
			Flags = Flags & (0xFF - FLAG_C); //Clear carry flag
		}
		set_zn_flags((BYTE)temp_word);
		set_flag_v(param1, param2, temp_word);
		Registers[REGISTER_A] = (BYTE)temp_word;
		break;

	case 0xD0: // ADC A,L
		param1 = Registers[REGISTER_A];
		param2 = Registers[REGISTER_L];
		temp_word = (WORD)Registers[REGISTER_A] + (WORD)Registers[REGISTER_L];
		if ((Flags & FLAG_C) != 0)
		{
			temp_word++;
		}

		if (temp_word >= 0x100)
		{
			Flags = Flags | FLAG_C; //Set carry flag
		}
		else
		{
			Flags = Flags & (0xFF - FLAG_C); //Clear carry flag
		}
		set_zn_flags((BYTE)temp_word);
		set_flag_v(param1, param2, temp_word);
		Registers[REGISTER_A] = (BYTE)temp_word;
		break;

	case 0xE0: // ADC A,H
		param1 = Registers[REGISTER_A];
		param2 = Registers[REGISTER_H];
		temp_word = (WORD)Registers[REGISTER_A] + (WORD)Registers[REGISTER_H];
		if ((Flags & FLAG_C) != 0)
		{
			temp_word++;
		}

		if (temp_word >= 0x100)
		{
			Flags = Flags | FLAG_C; //Set carry flag
		}
		else
		{
			Flags = Flags & (0xFF - FLAG_C); //Clear carry flag
		}
		set_zn_flags((BYTE)temp_word);
		set_flag_v(param1, param2, temp_word);
		Registers[REGISTER_A] = (BYTE)temp_word;
		break;

	case 0xF0: // ADC A,M
		param1 = Registers[REGISTER_A];
		param2 = Registers[REGISTER_M];
		temp_word = (WORD)Registers[REGISTER_A] + (WORD)Registers[REGISTER_M];
		if ((Flags & FLAG_C) != 0)
		{
			temp_word++;
		}

		if (temp_word >= 0x100)
		{
			Flags = Flags | FLAG_C; //Set carry flag
		}
		else
		{
			Flags = Flags & (0xFF - FLAG_C); //Clear carry flag
		}
		set_zn_flags((BYTE)temp_word);
		set_flag_v(param1, param2, temp_word);
		Registers[REGISTER_A] = (BYTE)temp_word;
		break;

	case 0x92: //CMP, A-B
		param1 = Registers[REGISTER_A];
		param2 = Registers[REGISTER_B];
		temp_word = (WORD)param1 - (WORD)param2;

		if (temp_word >= 0x100)
		{
			Flags = Flags | FLAG_C; //Set carry flag
		}
		else
		{
			Flags = Flags & (0xFF - FLAG_C); //Clear carry flag
		}
		set_zn_flags((BYTE)temp_word);
		set_flag_v(param1, -param2, temp_word);
		break;

	case 0xA2: //CMP, A-C
		param1 = Registers[REGISTER_A];
		param2 = Registers[REGISTER_C];
		temp_word = (WORD)param1 - (WORD)param2;

		if (temp_word >= 0x100)
		{
			Flags = Flags | FLAG_C; //Set carry flag
		}
		else
		{
			Flags = Flags & (0xFF - FLAG_C); //Clear carry flag
		}
		set_zn_flags((BYTE)temp_word);
		set_flag_v(param1, -param2, temp_word);
		break;

	case 0xB2: //CMP, A-D
		param1 = Registers[REGISTER_A];
		param2 = Registers[REGISTER_C];
		temp_word = (WORD)param1 - (WORD)param2;

		if (temp_word >= 0x100)
		{
			Flags = Flags | FLAG_C; //Set carry flag
		}
		else
		{
			Flags = Flags & (0xFF - FLAG_C); //Clear carry flag
		}
		set_zn_flags((BYTE)temp_word);
		set_flag_v(param1, -param2, temp_word);
		break;

	case 0xC2: //CMP, A-E
		param1 = Registers[REGISTER_A];
		param2 = Registers[REGISTER_E];
		temp_word = (WORD)param1 - (WORD)param2;

		if (temp_word >= 0x100)
		{
			Flags = Flags | FLAG_C; //Set carry flag
		}
		else
		{
			Flags = Flags & (0xFF - FLAG_C); //Clear carry flag
		}
		set_zn_flags((BYTE)temp_word);
		set_flag_v(param1, -param2, temp_word);
		break;

	case 0xD2: //CMP, A-L
		param1 = Registers[REGISTER_A];
		param2 = Registers[REGISTER_L];
		temp_word = (WORD)param1 - (WORD)param2;

		if (temp_word >= 0x100)
		{
			Flags = Flags | FLAG_C; //Set carry flag
		}
		else
		{
			Flags = Flags & (0xFF - FLAG_C); //Clear carry flag
		}
		set_zn_flags((BYTE)temp_word);
		set_flag_v(param1, -param2, temp_word);
		break;

	case 0xE2: //CMP, A-H
		param1 = Registers[REGISTER_A];
		param2 = Registers[REGISTER_H];
		temp_word = (WORD)param1 - (WORD)param2;

		if (temp_word >= 0x100)
		{
			Flags = Flags | FLAG_C; //Set carry flag
		}
		else
		{
			Flags = Flags & (0xFF - FLAG_C); //Clear carry flag
		}
		set_zn_flags((BYTE)temp_word);
		set_flag_v(param1, -param2, temp_word);
		break;

	case 0xF2: //CMP, A-M
		param1 = Registers[REGISTER_A];
		param2 = Registers[REGISTER_M];
		temp_word = (WORD)param1 - (WORD)param2;

		if (temp_word >= 0x100)
		{
			Flags = Flags | FLAG_C; //Set carry flag
		}
		else
		{
			Flags = Flags & (0xFF - FLAG_C); //Clear carry flag
		}
		set_zn_flags((BYTE)temp_word);
		set_flag_v(param1, -param2, temp_word);
		break;

	case 0x75: //TSA
		Registers[REGISTER_A] = Flags;
		break;

	case 0x74: //TAS
		Flags = Registers[REGISTER_A];
		break;

	case 0x15: //CLC
		Flags = Flags & 0xFE;
		break;

	case 0x16: //SEC
		Flags = Flags | 0x01;
		break;

	case 0x17: //CLI
		Flags = Flags & 0x7F;
		break;

	case 0x18: //STI
		Flags = Flags | 0x80;
		break;

	case 0x19: //SEV
		Flags = Flags | 0x10;
		break;

	case 0x1A: //CLV
		Flags = Flags & 0xEF;
		break;

		//Stack
	case 0x11: //PSH,A
		if ((StackPointer >= 1) && (StackPointer < MEMORY_SIZE))
		{
			Memory[StackPointer] = Registers[REGISTER_A];
			StackPointer--;
		}
		break;

	case 0x21: //PSH,FL (Status register)
		if ((StackPointer >= 1) && (StackPointer < MEMORY_SIZE))
		{
			Memory[StackPointer] = Registers[REGISTER_FL];
			StackPointer--;
		}
		break;

	case 0x31: //PSH,B
		if ((StackPointer >= 1) && (StackPointer < MEMORY_SIZE))
		{
			Memory[StackPointer] = Registers[REGISTER_B];
			StackPointer--;
		}
		break;

	case 0x41: //PSH,C
		if ((StackPointer >= 1) && (StackPointer < MEMORY_SIZE))
		{
			Memory[StackPointer] = Registers[REGISTER_C];
			StackPointer--;
		}
		break;

	case 0x51: //PSH,D
		if ((StackPointer >= 1) && (StackPointer < MEMORY_SIZE))
		{
			Memory[StackPointer] = Registers[REGISTER_D];
			StackPointer--;
		}
		break;

	case 0x61: //PSH,E
		if ((StackPointer >= 1) && (StackPointer < MEMORY_SIZE))
		{
			Memory[StackPointer] = Registers[REGISTER_E];
			StackPointer--;
		}
		break;

	case 0x71: //PSH,L
		if ((StackPointer >= 1) && (StackPointer < MEMORY_SIZE))
		{
			Memory[StackPointer] = Registers[REGISTER_L];
			StackPointer--;
		}
		break;

	case 0x81: //PSH,H
		if ((StackPointer >= 1) && (StackPointer < MEMORY_SIZE))
		{
			Memory[StackPointer] = Registers[REGISTER_H];
			StackPointer--;
		}
		break;

	case 0xFA: //JMP Absolute
		address = getAddressAbs();
		ProgramCounter = address;
		break;

	case 0x33: //JPR Absolute
		address = getAddressAbs();
		
		if ((StackPointer >= 2) && (StackPointer < MEMORY_SIZE))
		{
			Memory[StackPointer] = (BYTE)(ProgramCounter & 0xFF);
			StackPointer--;
			Memory[StackPointer] = (BYTE)((ProgramCounter >> 8) & 0xFF);
			StackPointer--;
		}
		ProgramCounter = address;
		break;

	case 0x0E: //RTN ***
		LB = pop8();
		HB = pop8();
		ProgramCounter = ((WORD)HB << 8) + LB;
		break;

	case 0x00: //BRA Relative
		LB = fetch();
		offset = (WORD)LB;

		if ((offset & 0x80) != 0)
		{
			offset += 0xFF00;  //offset = offset + 0xFF00
		}
		address = ProgramCounter + offset;
		ProgramCounter = address;
		break;

	case 0x12: //POP,A
		Registers[REGISTER_A] = Memory[0];
		break;

	case 0x22: //POP,FL
		Registers[REGISTER_FL] = Memory[0];
		break;

	case 0x32: //POP,B
		Registers[REGISTER_B] = Memory[0];
		break;

	case 0x42: //POP,C
		Registers[REGISTER_C] = Memory[0];
		break;

	case 0x52: //POP,D
		Registers[REGISTER_D] = Memory[0];
		break;

	case 0x62: //POP,E
		Registers[REGISTER_E] = Memory[0];
		break;

	case 0x72: //POP,L
		Registers[REGISTER_L] = Memory[0];
		break;

	case 0x82: //POP,H
		Registers[REGISTER_H] = Memory[0];
		break;

	case 0x01: //BCC
		LB = fetch();

		if((Flags & FLAG_C) == 0)
		{
			offset = (WORD)LB;

			if ((offset & 0x80) != 0)
			{
				offset += 0xFF00; //offset = offset + 0xFF00
			}
			address = ProgramCounter + offset; 
			ProgramCounter = address;
		}
		break;

	case 0x02: //BCS
		LB = fetch();
		
		if ((Flags & FLAG_C) == FLAG_C)
		{
			offset = (WORD)LB;
			if ((offset & 0x80) != 0)
			{ //need to sign extend 
				offset = offset + 0xFF00;
			} 
			address = ProgramCounter + offset;
			ProgramCounter = address;
		}
		break;

	case 0x03: //BNE
		LB = fetch();

		if ((Flags & FLAG_Z) == 0)
		{
			offset = (WORD)LB;

			if ((offset & 0x80) != 0)
			{
				offset += 0xFF00; //offset = offset + 0xFF00
			}
			address = ProgramCounter + offset;
			ProgramCounter = address;
		}
		break;

	case 0x04: //BEQ
		LB = fetch();

		if ((Flags & FLAG_Z) == FLAG_Z)
		{
			offset = (WORD)LB;

			if ((offset & 0x80) != 0)
			{
				offset += 0xFF00; //offset = offset + 0xFF00
			}
			address = ProgramCounter + offset;
			ProgramCounter = address;
		}
		break;

	case 0x05: //BVC
		LB = fetch();

		if ((Flags & FLAG_V) == 0)
		{
			offset = (WORD)LB;

			if ((offset & 0x80) != 0)
			{
				offset += 0xFF00; //offset = offset + 0xFF00
			}
			address = ProgramCounter + offset;
			ProgramCounter = address;
		}
		break;

	case 0x06: //BVS
		LB = fetch();

		if ((Flags & FLAG_V) == FLAG_V)
		{
			offset = (WORD)LB;

			if ((offset & 0x80) != 0)
			{
				offset += 0xFF00; //offset = offset + 0xFF00
			}
			address = ProgramCounter + offset;
			ProgramCounter = address;
		}
		break;

	case 0x07: //BMI
		LB = fetch();

		if((Flags & FLAG_C) == 0)
		{
			offset = (WORD)LB;

			if ((offset & 0x80) != 0)
			{
				offset += 0xFF00; //offset = offset + 0xFF00
			}
			address = ProgramCounter + offset; 
			ProgramCounter = address;
		}

	case 0x08: //BPL
		if ((Flags & FLAG_N) != FLAG_N)
		{
			ProgramCounter = getAddressAbs(); 
		}
		break;

	case 0x09: //BGE
		LB = fetch();

		if ((NF ^ VF) == 0)
		{
			offset = (WORD)LB;

			if ((offset & 0x80) != 0)
			{
				offset += 0xFF00; //offset = offset + 0xFF00
			}
			address = ProgramCounter + offset;
			ProgramCounter = address;
		}

	case 0x0A: //BLE
		LB = fetch();

		if ((ZF | NF ^ VF) == 1) //***Check if brackets are needed***
		{
			offset = (WORD)LB;

			if ((offset & 0x80) != 0)
			{
				offset += 0xFF00; //offset = offset + 0xFF00
			}
			address = ProgramCounter + offset;
			ProgramCounter = address;
		}

	case 0x0B: //BLS
		LB = fetch();

		if ((CF | ZF) == 1)
		{
			offset = (WORD)LB;

			if ((offset & 0x80) != 0)
			{
				offset += 0xFF00; //offset = offset + 0xFF00
			}
			address = ProgramCounter + offset;
			ProgramCounter = address;
		}

	case 0x0C: //BHI
		LB = fetch();

		if ((CF | ZF) == 1)
		{
			offset = (WORD)LB;

			if ((offset & 0x80) != 0)
			{
				offset += 0xFF00; //offset = offset + 0xFF00
			}
			address = ProgramCounter + offset;
			ProgramCounter = address;
		}

	case 0x34: //CCC
		Flags = Flags & FLAG_C;
		break;

	case 0x35: //CCS
		Flags = Flags | FLAG_C;
		break;

	case 0x36: //CNE
		
		if ((Flags & FLAG_Z) != 0)
		{
			ProgramCounter = getAddressAbs();
		}
		break;

	case 0x37: //CEQ
		
		if ((Flags & FLAG_Z) == 0)
		{
			ProgramCounter = getAddressAbs(); 
		}
		break;

	case 0x38: //CCC
		Flags = Flags & FLAG_V;
		break;

	case 0x39: //CVS 
		HB = fetch();
		LB = fetch();

		if ((Flags & FLAG_V) == FLAG_V) //Checks if overflow flag is set
		{
			address += (WORD)((WORD)HB << 8) + LB;
			if (address >= 0 && address < MEMORY_SIZE)
			{
				//Push return address onto the stack 
				if ((StackPointer >= 2) && (StackPointer < MEMORY_SIZE))
				{
					Memory[StackPointer] = (BYTE)(ProgramCounter & 0xFF);
					StackPointer--;
					Memory[StackPointer] = (BYTE)((ProgramCounter >> 8) & 0xFF);
					StackPointer--;
				}
				ProgramCounter = (WORD)address;
			}
		}
		break;

	case 0x3A: //CMI
		
		if ((Flags & FLAG_N) == 1)
		{
			ProgramCounter = getAddressAbs();  
		}
		break;

	case 0x3B: //CPL
		
		if ((Flags & FLAG_N) == 0)
		{
			ProgramCounter = getAddressAbs(); 
		}
		break;

	case 0x3C: //CHI
		LB = fetch();

		if ((CF | ZF) == 1)
		{
			offset = (WORD)LB;

			if ((offset & 0x80) != 0)
			{
				offset += 0xFF00; //offset = offset + 0xFF00
			}
			address = ProgramCounter + offset;
			ProgramCounter = address;
		}
		break;

	case 0x3D: //CLE
		LB = fetch();

		if ((CF | ZF) == 0)
		{
			offset = (WORD)LB;

			if ((offset & 0x80) != 0)
			{
				offset += 0xFF00; //offset = offset + 0xFF00
			}
			address = ProgramCounter + offset;
			ProgramCounter = address;
		}
		break;

	case 0x64: //INCA, A
		Registers[REGISTER_A]++;
		set_zn_flags(Registers[REGISTER_A]);
		break;

	case 0x65: //DECA, A
		Registers[REGISTER_A]--;
		set_zn_flags(Registers[REGISTER_A]);
		break;

	case 0x1C: //INX *** was index register,, dex
		Registers[REGISTER_X]++;
		set_flag_z(Registers[REGISTER_X]);
		break;

	case 0x94: //AND, A-B
		Registers[REGISTER_A] = Registers[REGISTER_A] & Registers[REGISTER_B];
		set_zn_flags(Registers[REGISTER_A]);
		break;

	case 0xA4: //AND, A-C
		Registers[REGISTER_A] = Registers[REGISTER_A] & Registers[REGISTER_C];
		set_zn_flags(Registers[REGISTER_A]);
		break;

	case 0xB4: //AND, A-D
		Registers[REGISTER_A] = Registers[REGISTER_A] & Registers[REGISTER_D];
		set_zn_flags(Registers[REGISTER_A]);
		break;

	case 0xC4: //AND, A-E
		Registers[REGISTER_A] = Registers[REGISTER_A] & Registers[REGISTER_E];
		set_zn_flags(Registers[REGISTER_A]);
		break;

	case 0xD4: //AND, A-L
		Registers[REGISTER_A] = Registers[REGISTER_A] & Registers[REGISTER_L];
		set_zn_flags(Registers[REGISTER_A]);
		break;

	case 0xE4: //AND, A-H
		Registers[REGISTER_A] = Registers[REGISTER_A] & Registers[REGISTER_H];
		set_zn_flags(Registers[REGISTER_A]);
		break;

	case 0xF4: //AND, A-M
		Registers[REGISTER_A] = Registers[REGISTER_A] & Registers[REGISTER_M];
		set_zn_flags(Registers[REGISTER_A]);
		break;

	case 0x96: //BT, A-B
		temp = Registers[REGISTER_A] & Registers[REGISTER_B];
		set_zn_flags(temp);
		break;

	case 0xA6: //BT, A-C
		temp = Registers[REGISTER_A] & Registers[REGISTER_C];
		set_zn_flags(temp);
		break;

	case 0xB6: //BT, A-D
		temp = Registers[REGISTER_A] & Registers[REGISTER_D];
		set_zn_flags(temp);
		break;

	case 0xC6: //BT, A-E
		temp = Registers[REGISTER_A] & Registers[REGISTER_E];
		set_zn_flags(temp);
		break;

	case 0xD6: //BT, A-L
		temp = Registers[REGISTER_A] & Registers[REGISTER_L];
		set_zn_flags(temp);
		break;

	case 0xE6: //BT, A-H
		temp = Registers[REGISTER_A] & Registers[REGISTER_H];
		set_zn_flags(temp);
		break;

	case 0xF6: //BT, A-M
		temp = Registers[REGISTER_A] & Registers[REGISTER_M];
		set_zn_flags(temp);
		break;

	case 0x44: //INC Absolute
		address = getAddressAbs();
		Memory[address]++;
		set_zn_flags(Memory[address]);
		break;

	case 0x54: //INC abs,X
		address = getAddressAbsX();
		Memory[address]++;
		set_zn_flags(Memory[address]);
		break;

	case 0x1B: //DEX ***
		Registers[REGISTER_X]--;
		set_flag_z(Registers[REGISTER_X]);
		break;

	case 0x91: //SBC, A-B
		param1 = Registers[REGISTER_A];
		param2 = Registers[REGISTER_B];
		temp_word = (WORD)Registers[REGISTER_A] - (WORD)Registers[REGISTER_B];
		if ((Flags & FLAG_C) != 0)
		{
			temp_word--;
		}

		if (temp_word >= 0x100)
		{
			Flags = Flags | FLAG_C; //Set carry flag
		}
		else
		{
			Flags = Flags & (0xFF - FLAG_C); //Clear carry flag
		}
		set_zn_flags((BYTE)temp_word);
		set_flag_v(param1, -param2, temp_word);
		Registers[REGISTER_A] = (BYTE)temp_word;
		break;

	case 0xA1: //SBC, A-C
		param1 = Registers[REGISTER_A];
		param2 = Registers[REGISTER_C];
		temp_word = (WORD)Registers[REGISTER_A] - (WORD)Registers[REGISTER_C];
		if ((Flags & FLAG_C) != 0)
		{
			temp_word--;
		}

		if (temp_word >= 0x100)
		{
			Flags = Flags | FLAG_C; //Set carry flag
		}
		else
		{
			Flags = Flags & (0xFF - FLAG_C); //Clear carry flag
		}
		set_zn_flags((BYTE)temp_word);
		set_flag_v(param1, -param2, temp_word);
		Registers[REGISTER_A] = (BYTE)temp_word;
		break;

	case 0xB1: //SBC, A-D
		param1 = Registers[REGISTER_A];
		param2 = Registers[REGISTER_D];
		temp_word = (WORD)Registers[REGISTER_A] - (WORD)Registers[REGISTER_D];
		if ((Flags & FLAG_C) != 0)
		{
			temp_word--;
		}

		if (temp_word >= 0x100)
		{
			Flags = Flags | FLAG_C; //Set carry flag
		}
		else
		{
			Flags = Flags & (0xFF - FLAG_C); //Clear carry flag
		}
		set_zn_flags((BYTE)temp_word);
		set_flag_v(param1, -param2, temp_word);
		Registers[REGISTER_A] = (BYTE)temp_word;
		break;

	case 0xC1: //SBC, A-E
		param1 = Registers[REGISTER_A];
		param2 = Registers[REGISTER_E];
		temp_word = (WORD)Registers[REGISTER_A] - (WORD)Registers[REGISTER_E];
		if ((Flags & FLAG_C) != 0)
		{
			temp_word--;
		}

		if (temp_word >= 0x100)
		{
			Flags = Flags | FLAG_C; //Set carry flag
		}
		else
		{
			Flags = Flags & (0xFF - FLAG_C); //Clear carry flag
		}
		set_zn_flags((BYTE)temp_word);
		set_flag_v(param1, -param2, temp_word);
		Registers[REGISTER_A] = (BYTE)temp_word;
		break;

	case 0xD1: //SBC, A-L
		param1 = Registers[REGISTER_A];
		param2 = Registers[REGISTER_L];
		temp_word = (WORD)Registers[REGISTER_A] - (WORD)Registers[REGISTER_L];
		if ((Flags & FLAG_C) != 0)
		{
			temp_word--;
		}

		if (temp_word >= 0x100)
		{
			Flags = Flags | FLAG_C; //Set carry flag
		}
		else
		{
			Flags = Flags & (0xFF - FLAG_C); //Clear carry flag
		}
		set_zn_flags((BYTE)temp_word);
		set_flag_v(param1, -param2, temp_word);
		Registers[REGISTER_A] = (BYTE)temp_word;
		break;

	case 0xE1: //SBC, A-H
		param1 = Registers[REGISTER_A];
		param2 = Registers[REGISTER_H];
		temp_word = (WORD)Registers[REGISTER_A] - (WORD)Registers[REGISTER_H];
		if ((Flags & FLAG_C) != 0)
		{
			temp_word--;
		}

		if (temp_word >= 0x100)
		{
			Flags = Flags | FLAG_C; //Set carry flag
		}
		else
		{
			Flags = Flags & (0xFF - FLAG_C); //Clear carry flag
		}
		set_zn_flags((BYTE)temp_word);
		set_flag_v(param1, -param2, temp_word);
		Registers[REGISTER_A] = (BYTE)temp_word;
		break;

	case 0xF1: //SBC, A-M
		address = (WORD)Registers[REGISTER_H] << 8 + Registers[REGISTER_L];
		param1 = Registers[REGISTER_A];
		param2 = Registers[REGISTER_M];
		temp_word = (WORD)Registers[REGISTER_A] - (WORD)Registers[REGISTER_M];
		if ((Flags & FLAG_C) != 0)
		{
			temp_word--;
		}

		if (temp_word >= 0x100)
		{
			Flags = Flags | FLAG_C; //Set carry flag
		}
		else
		{
			Flags = Flags & (0xFF - FLAG_C); //Clear carry flag
		}
		set_zn_flags((BYTE)temp_word);
		set_flag_v(param1, -param2, temp_word);
		Registers[REGISTER_A] = (BYTE)temp_word;
		break;

	case 0x93: //IOR, A-B
		Registers[REGISTER_A] = Registers[REGISTER_A] | Registers[REGISTER_B];
		set_zn_flags(Registers[REGISTER_A]);
		break;

	case 0xA3: //IOR, A-C
		Registers[REGISTER_A] = Registers[REGISTER_A] | Registers[REGISTER_C];
		set_zn_flags(Registers[REGISTER_A]);
		break;

	case 0xB3: //IOR, A-D
		Registers[REGISTER_A] = Registers[REGISTER_A] | Registers[REGISTER_D];
		set_zn_flags(Registers[REGISTER_A]);
		break;

	case 0xC3: //IOR, A-E
		Registers[REGISTER_A] = Registers[REGISTER_A] | Registers[REGISTER_E];
		set_zn_flags(Registers[REGISTER_A]);
		break;

	case 0xD3: //IOR, A-L
		Registers[REGISTER_A] = Registers[REGISTER_A] | Registers[REGISTER_L];
		set_zn_flags(Registers[REGISTER_A]);
		break;

	case 0xE3: //IOR, A-H
		Registers[REGISTER_A] = Registers[REGISTER_A] | Registers[REGISTER_H];
		set_zn_flags(Registers[REGISTER_A]);
		break;

	case 0xF3: //IOR, A-M
		Registers[REGISTER_A] = Registers[REGISTER_A] | Registers[REGISTER_M];
		set_zn_flags(Registers[REGISTER_A]);
		break;

	case 0x6A: //NOTA, A
		Registers[REGISTER_A] = ~Registers[REGISTER_A];
		set_zn_flags(Registers[REGISTER_A]);
		Flags = Flags & (0xFF - FLAG_C);
		break;

	case 0x4A: //NOT Absolute
		address = getAddressAbs();
		
		Memory[address] = ~Memory[address];
		set_zn_flags(Memory[address]);
		Flags = Flags & (0xFF - FLAG_C);
		break;
	
	case 0x5A: ///NOT abs,X
		address = getAddressAbsX();

		Memory[address] = ~Memory[address];
		set_zn_flags(Memory[address]);
		Flags = Flags & (0xFF - FLAG_C);
		break;

	case 0x95: //XOR, A-B
		Registers[REGISTER_A] = Registers[REGISTER_A] ^ Registers[REGISTER_B];
		set_zn_flags(Registers[REGISTER_A]);
		break;

	case 0xA5: //XOR, A-C
		Registers[REGISTER_A] = Registers[REGISTER_A] ^ Registers[REGISTER_C];
		set_zn_flags(Registers[REGISTER_A]);
		break;

	case 0xB5: //XOR, A-D
		Registers[REGISTER_A] = Registers[REGISTER_A] ^ Registers[REGISTER_D];
		set_zn_flags(Registers[REGISTER_A]);
		break;

	case 0xC5: //XOR, A-E
		Registers[REGISTER_A] = Registers[REGISTER_A] ^ Registers[REGISTER_E];
		set_zn_flags(Registers[REGISTER_A]);
		break;

	case 0xD5: //XOR, A-L
		Registers[REGISTER_A] = Registers[REGISTER_A] ^ Registers[REGISTER_L];
		set_zn_flags(Registers[REGISTER_A]);
		break;

	case 0xE5: //XOR, A-H
		Registers[REGISTER_A] = Registers[REGISTER_A] ^ Registers[REGISTER_H];
		set_zn_flags(Registers[REGISTER_A]);
		break;

	case 0xF5: //XOR, A-M
		Registers[REGISTER_A] = Registers[REGISTER_A] ^ Registers[REGISTER_M];
		set_zn_flags(Registers[REGISTER_A]);
		break;

	case 0x25: //ADI
		data = fetch();
		data = data + Registers[REGISTER_C];
		break;

	case 0x26: //SBI
		data = fetch();
		data = data - Registers[REGISTER_C];
		break;

	case 0x45: //DEC Absolute
		address = getAddressAbs();
		Memory[address]--;
		set_zn_flags(Memory[address]);
		break;

	case 0x55: //DEC abs,X
		address = getAddressAbsX();
		Memory[address]--;
		set_zn_flags(Memory[address]);
		break;

	case 0x28: //ANI ***check***
		data = fetch();
		Registers[REGISTER_A] = Registers[REGISTER_A] & data;
		set_zn_flags(Registers[REGISTER_A]);
		break;

	case 0x29: //XRI ***check***
		data = fetch();
		Registers[REGISTER_A] = Registers[REGISTER_A] ^ data;
		set_zn_flags(Registers[REGISTER_A]);
		break;

	case 0x43: ///TST Absolute
		address = getAddressAbs();
		set_zn_flags(Memory[address]);
		break;

	case 0x53: //TST abs,X
		address = getAddressAbsX();
		set_zn_flags(Memory[address]);
		break;

	case 0x63: //TSTA
		set_zn_flags(Registers[REGISTER_A]);
		break;

	case 0x47: //RCL Absolute
		address = getAddressAbs();
		saved_flags = Flags;

		if ((Memory[address] & 0x80) == 0x80)
		{
			Flags = Flags | FLAG_C;
		}
		else
		{
			Flags = Flags & (0xFF - FLAG_C);
		}
		
		Memory[address] = Memory[address] | 0x01;

		if ((saved_flags & FLAG_C) == FLAG_C)
		{
			Memory[address] = Memory[address] | 0x01;
		}
		set_zn_flags(Memory[address]);
		break;

	case 0x57: //RCL abs,X
		address = getAddressAbsX();
		saved_flags = Flags;

		if ((Memory[address] & 0x80) == 0x80)
		{
			Flags = Flags | FLAG_C;
		}
		else
		{
			Flags = Flags & (0xFF - FLAG_C);
		}

		Memory[address] = Memory[address] | 0x01;

		if ((saved_flags & FLAG_C) == FLAG_C)
		{
			Memory[address] = Memory[address] | 0x01;
		}
		set_zn_flags(Memory[address]);
		break;

	case 0x67: //RCLA
		saved_flags = Flags;

		if ((Registers[REGISTER_A] & 0x80) == 0x80)
		{
			Flags = Flags | FLAG_C;
		}
		else
		{
			Flags = Flags & (0xFF - FLAG_C);
		}

		Registers[REGISTER_A] = (Registers[REGISTER_A] << 1) & 0xFE;

		if ((saved_flags & FLAG_C) == FLAG_C)
		{
			Registers[REGISTER_A] = Registers[REGISTER_A] | 0x01;
		}
		set_zn_flags(Registers[REGISTER_A]);
		break;

	case 0x48: //SAL Absolute
		address = getAddressAbs();

		if ((Memory[address] & 0x80) == 0x80)
		{
			Flags = Flags | FLAG_C;
		}
		else
		{
			Flags = Flags & (0xFF - FLAG_C);
		}
		Memory[address] = (Memory[address] << 1) & 0xFE;
		set_zn_flags(Memory[address]);
		break;

	case 0x58: //SAL abs,X
		address = getAddressAbsX();

		if ((Memory[address] & 0x80) == 0x80)
		{
			Flags = Flags | FLAG_C;
		}
		else
		{
			Flags = Flags & (0xFF - FLAG_C);
		}
		Memory[address] = (Memory[address] << 1) & 0xFE;
		set_zn_flags(Memory[address]);
		break;

	case 0x68: //SALA
		if ((Registers[REGISTER_A] & 0x80) == 0x80)
		{
			Flags = Flags | FLAG_C;
		}
		else
		{
			Flags = Flags & (0xFF - FLAG_C);
		}
		Registers[REGISTER_A] = (Registers[REGISTER_A] << 1) & 0xFE;
		set_zn_flags(Registers[REGISTER_A]);
		break;

	case 0x49: //ASR Absolute
		address = getAddressAbs();
		
		if ((Memory[address] & 0x01) == 0x01)
		{
			Flags = Flags | FLAG_C;
		}
		else
		{
			Flags = Flags & (0xFF - FLAG_C);
		}

		Memory[address] = (Memory[address] >> 1) & 0x7F;
		set_zn_flags(Memory[address]);
		break;

	case 0x59: //ASR abs,X
		address = getAddressAbsX();

		if ((Memory[address] & 0x01) == 0x01)
		{
			Flags = Flags | FLAG_C;
		}
		else
		{
			Flags = Flags & (0xFF - FLAG_C);
		}

		Memory[address] = (Memory[address] >> 1) & 0x7F;
		set_zn_flags(Memory[address]);
		break;

	case 0x69: //ASRA
		if ((Registers[REGISTER_A] & 0x01) == 0x01)
		{
			Flags = Flags | FLAG_C;
		}
		else
		{
			Flags = Flags & (0xFF - FLAG_C);
		}

		Registers[REGISTER_A] = (Registers[REGISTER_A] >> 1) & 0x7F;
		set_zn_flags(Registers[REGISTER_A]);

		if ((Registers[REGISTER_A] & 0x40) == 0x40)
		{
			Registers[REGISTER_A] = Registers[REGISTER_A] | 0x80;
		}
		break;

	case 0x1E: //WAI
		halt = true;
		break;

	case 0x1d: //NOP
		halt = true;
		break;

	case 0x85: //SWI
		Flags = Flags | FLAG_I;
		push8(Registers[REGISTER_A]);
		push8(Registers[REGISTER_B]);
		push16(ProgramCounter);
		push8(Registers[REGISTER_C]);
		push8(Registers[REGISTER_D]);
		push8(Flags);
		break;

	case 0x86: //RTI *****
		LB = pop8();
		HB = pop8();
		ProgramCounter = ((WORD)HB << 8) + LB;
		break;
	}
}

void Group_2_Move(BYTE opcode)
{
	int destination;
	int source;

	int destReg = 0;
	int sourceReg = 0;

	WORD address;

	destination = opcode & 0x0F;

	switch (destination)
	{
	case 0x08:
		destReg = REGISTER_A;
		break;

	case 0x09:
		destReg = REGISTER_B;
		break;

	case 0x0A:
		destReg = REGISTER_C;
		break;

	case 0x0B:
		destReg = REGISTER_D;
		break;

	case 0x0C:
		destReg = REGISTER_E;
		break;

	case 0x0D:
		destReg = REGISTER_L;
		break;

	case 0x0E:
		destReg = REGISTER_H;
		break;

	case 0x0F:
		destReg = REGISTER_M;
		break;
	}

	source = (opcode >> 4) & 0x0F;

	switch (source)
	{
	case 0x07:
		sourceReg = REGISTER_A;
		break;

	case 0x08:
		sourceReg = REGISTER_B;
		break;

	case 0x09:
		sourceReg = REGISTER_C;
		break;

	case 0x0A:
		sourceReg = REGISTER_D;
		break;

	case 0x0C:
		sourceReg = REGISTER_E;
		break;

	case 0x0D:
		sourceReg = REGISTER_L;
		break;

	case 0x0E:
		sourceReg = REGISTER_H;
		break;
	}

	if (sourceReg == REGISTER_M)
	{
		address = (WORD)Registers[REGISTER_H] << 8 + (WORD)Registers[REGISTER_L];
		Registers[REGISTER_M] = Memory[address];
	}

	Registers[destReg] = Registers[sourceReg];

	if (destReg == REGISTER_M)
	{
		address = (WORD)Registers[REGISTER_H] << 8 + (WORD)Registers[REGISTER_L];
		Memory[address] = Registers[REGISTER_M];
	}
}



void execute(BYTE opcode)
{
	if (((opcode >= 0x78) && (opcode <= 0x7F))
		|| ((opcode >= 0x88) && (opcode <= 0x8F))
		|| ((opcode >= 0x98) && (opcode <= 0x9F))
		|| ((opcode >= 0xA8) && (opcode <= 0xAF))
		|| ((opcode >= 0xB8) && (opcode <= 0xBF))
		|| ((opcode >= 0xC8) && (opcode <= 0xCF))
		|| ((opcode >= 0xD8) && (opcode <= 0xDF))
		|| ((opcode >= 0xE8) && (opcode <= 0xEF)))
	{
		Group_2_Move(opcode);
	}
	else
	{
		Group_1(opcode);
	}
}

void emulate()
{
	BYTE opcode;
	int sanity;
	
	ProgramCounter = 0;
	halt = false;
	memory_in_range = true;
	sanity = 0;
	
	printf("                    A  B  C  D  E  L  H  X    SP\n");

	while ((!halt) && (memory_in_range)) {
		
		sanity++;
		if (sanity > 500) halt = true;
		printf("%04X ", ProgramCounter);           // Print current address
		opcode = fetch();
		execute(opcode);

		printf("%s  ", opcode_mneumonics[opcode]);  // Print current opcode

		printf("%02X ", Registers[REGISTER_A]);
		printf("%02X ", Registers[REGISTER_B]);
		printf("%02X ", Registers[REGISTER_C]);
		printf("%02X ", Registers[REGISTER_D]);
		printf("%02X ", Registers[REGISTER_E]);
		printf("%02X ", Registers[REGISTER_L]);
		printf("%02X ", Registers[REGISTER_H]);
		printf("%04X ", IndexRegister);
		printf("%04X ", StackPointer);              // Print Stack Pointer

		if ((Flags & FLAG_I) == FLAG_I)	
		{
			printf("I=1 ");
		}
		else
		{
			printf("I=0 ");
		}
		if ((Flags & FLAG_N) == FLAG_N)	
		{
			printf("N=1 ");
		}
		else
		{
			printf("N=0 ");
		}
		if ((Flags & FLAG_V) == FLAG_V)	
		{
			printf("V=1 ");
		}
		else
		{
			printf("V=0 ");
		}
		if ((Flags & FLAG_Z) == FLAG_Z)	
		{
			printf("Z=1 ");
		}
		else
		{
			printf("Z=0 ");
		}
		if ((Flags & FLAG_C) == FLAG_C)	
		{
			printf("C=1 ");
		}
		else
		{
			printf("C=0 ");
		}

		printf("\n");  // New line
	}

	printf("\n");  // New line
}


////////////////////////////////////////////////////////////////////////////////
//                            Simulator/Emulator (End)                        //
////////////////////////////////////////////////////////////////////////////////


void initialise_filenames() {
	int i;

	for (i=0; i<MAX_FILENAME_SIZE; i++) {
		hex_file [i] = '\0';
		trc_file [i] = '\0';
	}
}


int find_dot_position(char *filename) {
	int  dot_position;
	int  i;
	char chr;

	dot_position = 0;
	i = 0;
	chr = filename[i];

	while (chr != '\0') {
		if (chr == '.') {
			dot_position = i;
		}
		i++;
		chr = filename[i];
	}

	return (dot_position);
}


int find_end_position(char *filename) {
	int  end_position;
	int  i;
	char chr;

	end_position = 0;
	i = 0;
	chr = filename[i];

	while (chr != '\0') {
		end_position = i;
		i++;
		chr = filename[i];
	}

	return (end_position);
}


bool file_exists(char *filename) {
	bool exists;
	FILE *ifp;

	exists = false;

	if ( ( ifp = fopen( filename, "r" ) ) != NULL ) {
		exists = true;

		fclose(ifp);
	}

	return (exists);
}


void create_file(char *filename) {
	FILE *ofp;

	if ( ( ofp = fopen( filename, "w" ) ) != NULL ) {
		fclose(ofp);
	}
}


bool getline(FILE *fp, char *buffer) {
	bool rc;
	bool collect;
	char c;
	int  i;

	rc = false;
	collect = true;

	i = 0;
	while (collect) {
		c = getc(fp);

		switch (c) {
		case EOF:
			if (i > 0) {
				rc = true;
			}
			collect = false;
			break;

		case '\n':
			if (i > 0) {
				rc = true;
				collect = false;
				buffer[i] = '\0';
			}
			break;

		default:
			buffer[i] = c;
			i++;
			break;
		}
	}

	return (rc);
}


void load_and_run(int args,_TCHAR** argv) {
	char chr;
	int  ln;
	int  dot_position;
	int  end_position;
	long i;
	FILE *ifp;
	long address;
	long load_at;
	int  code;

	//Prompt for the .hex file

	printf("\n");
	printf("Enter the hex filename (.hex): ");

	if(args == 2){
		ln = 0;
		chr = argv[1][ln];
		while (chr != '\0')
		{
			if (ln < MAX_FILENAME_SIZE)
			{
				hex_file [ln] = chr;
				trc_file [ln] = chr;
				ln++;
			}
			chr = argv[1][ln];
		}
	} else {
		ln = 0;
		chr = '\0';
		while (chr != '\n') {
			chr = getchar();

			switch(chr) {
			case '\n':
				break;
			default:
				if (ln < MAX_FILENAME_SIZE)	{
					hex_file [ln] = chr;
					trc_file [ln] = chr;
					ln++;
				}
				break;
			}
		}

	}
	// Tidy up the file names

	dot_position = find_dot_position(hex_file);
	if (dot_position == 0) {
		end_position = find_end_position(hex_file);

		hex_file[end_position + 1] = '.';
		hex_file[end_position + 2] = 'h';
		hex_file[end_position + 3] = 'e';
		hex_file[end_position + 4] = 'x';
		hex_file[end_position + 5] = '\0';
	} else {
		hex_file[dot_position + 0] = '.';
		hex_file[dot_position + 1] = 'h';
		hex_file[dot_position + 2] = 'e';
		hex_file[dot_position + 3] = 'x';
		hex_file[dot_position + 4] = '\0';
	}

	dot_position = find_dot_position(trc_file);
	if (dot_position == 0) {
		end_position = find_end_position(trc_file);

		trc_file[end_position + 1] = '.';
		trc_file[end_position + 2] = 't';
		trc_file[end_position + 3] = 'r';
		trc_file[end_position + 4] = 'c';
		trc_file[end_position + 5] = '\0';
	} else {
		trc_file[dot_position + 0] = '.';
		trc_file[dot_position + 1] = 't';
		trc_file[dot_position + 2] = 'r';
		trc_file[dot_position + 3] = 'c';
		trc_file[dot_position + 4] = '\0';
	}

	if (file_exists(hex_file)) {
		// Clear Registers and Memory

		Registers[REGISTER_A] = 0;
		Registers[REGISTER_B] = 0;
		Registers[REGISTER_C] = 0;
		Registers[REGISTER_D] = 0;
		Registers[REGISTER_E] = 0;
		Registers[REGISTER_L] = 0;
		Registers[REGISTER_H] = 0;
		IndexRegister = 0;
		Flags = 0;
		ProgramCounter = 0;
		StackPointer = 0;

		for (i=0; i<MEMORY_SIZE; i++) {
			Memory[i] = 0x00;
		}

		// Load hex file

		if ( ( ifp = fopen( hex_file, "r" ) ) != NULL ) {
			printf("Loading file...\n\n");

			load_at = 0;

			while (getline(ifp, InputBuffer)) {
				if (sscanf(InputBuffer, "L=%x", &address) == 1) {
					load_at = address;
				} else if (sscanf(InputBuffer, "%x", &code) == 1) {
					if ((load_at >= 0) && (load_at <= MEMORY_SIZE)) {
						Memory[load_at] = (BYTE)code;
					}
					load_at++;
				} else {
					printf("ERROR> Failed to load instruction: %s \n", InputBuffer);
				}
			}

			fclose(ifp);
		}

		// Emulate

		emulate();
	} else {
		printf("\n");
		printf("ERROR> Input file %s does not exist!\n", hex_file);
		printf("\n");
	}
}

void building(int args,_TCHAR** argv){
	char buffer[1024];
	load_and_run(args,argv);
	sprintf(buffer, "0x%02X,0x%02X,0x%02X,0x%02X,0x%02X,0x%02X,0x%02X,0x%02X,0x%02X,0x%02X,0x%02X,0x%02X", 
		Memory[TEST_ADDRESS_1],
		Memory[TEST_ADDRESS_2],
		Memory[TEST_ADDRESS_3],
		Memory[TEST_ADDRESS_4], 
		Memory[TEST_ADDRESS_5],
		Memory[TEST_ADDRESS_6], 
		Memory[TEST_ADDRESS_7],
		Memory[TEST_ADDRESS_8], 
		Memory[TEST_ADDRESS_9], 
		Memory[TEST_ADDRESS_10],
		Memory[TEST_ADDRESS_11],
		Memory[TEST_ADDRESS_12]
		);
	sendto(sock, buffer, strlen(buffer), 0, (SOCKADDR *)&server_addr, sizeof(SOCKADDR));
}



void test_and_mark() {
	char buffer[1024];
	bool testing_complete;
	int  len = sizeof(SOCKADDR);
	char chr;
	int  i;
	int  j;
	bool end_of_program;
	long address;
	long load_at;
	int  code;
	int  mark;
	int  passed;

	printf("\n");
	printf("Automatic Testing and Marking\n");
	printf("\n");

	testing_complete = false;

	sprintf(buffer, "Test Student %s", STUDENT_NUMBER);
	sendto(sock, buffer, strlen(buffer), 0, (SOCKADDR *)&server_addr, sizeof(SOCKADDR));

	while (!testing_complete) {
		memset(buffer, '\0', sizeof(buffer));

		if (recvfrom(sock, buffer, sizeof(buffer)-1, 0, (SOCKADDR *)&client_addr, &len) != SOCKET_ERROR) {
			printf("Incoming Data: %s \n", buffer);

			//if (strcmp(buffer, "Testing complete") == 1)
			if (sscanf(buffer, "Testing complete %d", &mark) == 1) {
				testing_complete = true;
				printf("Current mark = %d\n", mark);

			}else if (sscanf(buffer, "Tests passed %d", &passed) == 1) {
				//testing_complete = true;
				printf("Passed = %d\n", passed);

			} else if (strcmp(buffer, "Error") == 0) {
				printf("ERROR> Testing abnormally terminated\n");
				testing_complete = true;
			} else {
				// Clear Registers and Memory

		Registers[REGISTER_A] = 0;
		Registers[REGISTER_B] = 0;
		Registers[REGISTER_C] = 0;
		Registers[REGISTER_D] = 0;
		Registers[REGISTER_E] = 0;
		Registers[REGISTER_L] = 0;
		Registers[REGISTER_H] = 0;
		IndexRegister = 0;
				Flags = 0;
				ProgramCounter = 0;
				StackPointer = 0;
				for (i=0; i<MEMORY_SIZE; i++) {
					Memory[i] = 0;
				}

				// Load hex file

				i = 0;
				j = 0;
				load_at = 0;
				end_of_program = false;
				FILE *ofp;
				fopen_s(&ofp ,"branch.txt", "a");

				while (!end_of_program) {
					chr = buffer[i];
					switch (chr) {
					case '\0':
						end_of_program = true;

					case ',':
						if (sscanf(InputBuffer, "L=%x", &address) == 1) {
							load_at = address;
						} else if (sscanf(InputBuffer, "%x", &code) == 1) {
							if ((load_at >= 0) && (load_at <= MEMORY_SIZE)) {
								Memory[load_at] = (BYTE)code;
								fprintf(ofp, "%02X\n", (BYTE)code);
							}
							load_at++;
						} else {
							printf("ERROR> Failed to load instruction: %s \n", InputBuffer);
						}
						j = 0;
						break;

					default:
						InputBuffer[j] = chr;
						j++;
						break;
					}
					i++;
				}
				fclose(ofp);
				// Emulate

				if (load_at > 1) {
					emulate();
					// Send and store results
					sprintf(buffer, "%02X%02X %02X%02X %02X%02X %02X%02X %02X%02X %02X%02X", 
						Memory[TEST_ADDRESS_1],
						Memory[TEST_ADDRESS_2],
						Memory[TEST_ADDRESS_3],
						Memory[TEST_ADDRESS_4], 
						Memory[TEST_ADDRESS_5],
						Memory[TEST_ADDRESS_6], 
						Memory[TEST_ADDRESS_7],
						Memory[TEST_ADDRESS_8], 
						Memory[TEST_ADDRESS_9], 
						Memory[TEST_ADDRESS_10],
						Memory[TEST_ADDRESS_11],
						Memory[TEST_ADDRESS_12]
						);
					sendto(sock, buffer, strlen(buffer), 0, (SOCKADDR *)&server_addr, sizeof(SOCKADDR));
				}
			}
		}
	}
}


int _tmain(int argc, _TCHAR* argv[])
{
	char chr;
	char dummy;

	printf("\n");
	printf("Microprocessor Emulator\n");
	printf("UWE Computer and Network Systems Assignment 1\n");
	printf("\n");

	initialise_filenames();

	if (WSAStartup(MAKEWORD(2, 2), &data) != 0) return(0);

	sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);  // Here we create our socket, which will be a UDP socket (SOCK_DGRAM).
	if (!sock) {	
		// Creation failed! 
	}

	memset(&server_addr, 0, sizeof(SOCKADDR_IN));
	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = inet_addr(IP_ADDRESS_SERVER);
	server_addr.sin_port = htons(PORT_SERVER);

	memset(&client_addr, 0, sizeof(SOCKADDR_IN));
	client_addr.sin_family = AF_INET;
	client_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
	client_addr.sin_port = htons(PORT_CLIENT);

	chr = '\0';
	while ((chr != 'e') && (chr != 'E'))
	{
		printf("\n");
		printf("Please select option\n");
		printf("L - Load and run a hex file\n");
		printf("T - Have the server test and mark your emulator\n");
		printf("E - Exit\n");
		if(argc == 2){ building(argc,argv); exit(0);}
		printf("Enter option: ");
		chr = getchar();
		if (chr != 0x0A)
		{
			dummy = getchar();  // read in the <CR>
		}
		printf("\n");

		switch (chr)
		{
		case 'L':
		case 'l':
			load_and_run(argc,argv);
			break;

		case 'T':
		case 't':
			test_and_mark();
			break;

		default:
			break;
		}
	}

	closesocket(sock);
	WSACleanup();

	return 0;
}