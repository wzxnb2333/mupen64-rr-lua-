/**
 * Mupen64 - flashram.c
 * Copyright (C) 2002 Hacktarux
 *
 * Mupen64 homepage: http://mupen64.emulation64.com
 * email address: hacktarux@yahoo.fr
 *
 * If you want to contribute to the project please contact
 * me first (maybe someone is already making what you are
 * planning to do).
 *
 *
 * This program is free software; you can redistribute it and/
 * or modify it under the terms of the GNU General Public Li-
 * cence as published by the Free Software Foundation; either
 * version 2 of the Licence, or any later version.
 *
 * This program is distributed in the hope that it will be use-
 * ful, but WITHOUT ANY WARRANTY; without even the implied war-
 * ranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public Licence for more details.
 *
 * You should have received a copy of the GNU General Public
 * Licence along with this program; if not, write to the Free
 * Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139,
 * USA.
 *
**/

#include <stdio.h>
#include <stdlib.h>

#include "../r4300/r4300.h"
#include "memory.h"
#include "savestates.h"
#include <shared/guifuncs.h>

int use_flashram;

typedef enum flashram_mode
{
	NOPES_MODE = 0,
	ERASE_MODE,
	WRITE_MODE,
	READ_MODE,
	STATUS_MODE
} Flashram_mode;

static int mode;
static unsigned long long status;
static unsigned long erase_offset, write_pointer;

void save_flashram_infos(char* buf)
{
	memcpy(buf + 0, &use_flashram, 4);
	memcpy(buf + 4, &mode, 4);
	memcpy(buf + 8, &status, 8);
	memcpy(buf + 16, &erase_offset, 4);
	memcpy(buf + 20, &write_pointer, 4);
}

void load_flashram_infos(char* buf)
{
	memcpy(&use_flashram, buf + 0, 4);
	memcpy(&mode, buf + 4, 4);
	memcpy(&status, buf + 8, 8);
	memcpy(&erase_offset, buf + 16, 4);
	memcpy(&write_pointer, buf + 20, 4);
}

void init_flashram()
{
	mode = NOPES_MODE;
	status = 0;
}

unsigned long flashram_status()
{
	return (status >> 32);
}

void flashram_command(unsigned long command)
{
	switch (command & 0xff000000)
	{
	case 0x4b000000:
		erase_offset = (command & 0xffff) * 128;
		break;
	case 0x78000000:
		mode = ERASE_MODE;
		status = 0x1111800800c20000LL;
		break;
	case 0xa5000000:
		erase_offset = (command & 0xffff) * 128;
		status = 0x1111800400c20000LL;
		break;
	case 0xb4000000:
		mode = WRITE_MODE;
		break;
	case 0xd2000000: // execute
		switch (mode)
		{
		case NOPES_MODE:
			break;
		case ERASE_MODE:
			{
				fseek(g_sram_file, 0, SEEK_SET);
				fread(flashram, 1, 0x20000, g_sram_file);

				for (int i = erase_offset; i < (erase_offset + 128); i++)
					flashram[i ^ S8] = 0xff;

				fseek(g_sram_file, 0, SEEK_SET);
				fwrite(flashram, 1, 0x20000, g_sram_file);
			}
			break;
		case WRITE_MODE:
			{
				fseek(g_sram_file, 0, SEEK_SET);
				fread(flashram, 1, 0x20000, g_sram_file);

				for (int i = 0; i < 128; i++)
					flashram[(erase_offset + i) ^ S8] =
						((unsigned char*)rdram)[(write_pointer + i) ^ S8];

				fseek(g_sram_file, 0, SEEK_SET);
				fwrite(flashram, 1, 0x20000, g_sram_file);
			}
			break;
		case STATUS_MODE:
			break;
		default:
			printf("unknown flashram command with mode:%x\n", (int)mode);
			stop = 1;
		}
		mode = NOPES_MODE;
		break;
	case 0xe1000000:
		mode = STATUS_MODE;
		status = 0x1111800100c20000LL;
		break;
	case 0xf0000000:
		mode = READ_MODE;
		status = 0x11118004f0000000LL;
		break;
	default:
		printf("unknown flashram command:%x\n", (int)command);
	//stop=1;
	}
}

void dma_read_flashram()
{
	int i;
	char* filename;
	FILE* f;

	switch (mode)
	{
	case STATUS_MODE:
		rdram[pi_register.pi_dram_addr_reg / 4] = (unsigned long)(status >> 32);
		rdram[pi_register.pi_dram_addr_reg / 4 + 1] = (unsigned long)(status);
		break;
	case READ_MODE:
		{
			fseek(g_fram_file, 0, SEEK_SET);
			fread(flashram, 1, 0x20000, g_fram_file);

			for (i = 0; i < (pi_register.pi_wr_len_reg & 0x0FFFFFF) + 1; i++)
				((unsigned char*)rdram)[(pi_register.pi_dram_addr_reg + i) ^ S8] =
					flashram[(((pi_register.pi_cart_addr_reg - 0x08000000) & 0xFFFF) * 2 + i) ^ S8];
			break;
		}
	default:
		printf("unknown dma_read_flashram:%x\n", mode);
		stop = 1;
	}
}

void dma_write_flashram()
{
	switch (mode)
	{
	case WRITE_MODE:
		write_pointer = pi_register.pi_dram_addr_reg;
		break;
	default:
		printf("unknown dma_read_flashram:%x\n", mode);
		stop = 1;
	}
}
