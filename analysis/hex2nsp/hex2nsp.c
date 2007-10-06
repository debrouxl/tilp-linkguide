/*  hex2nsp - an D-USB packet decompiler
 *  Copyright (C) 2005-2007  Romain Lievin
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */


#include <conio.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

/*
	Format (see http://hackspire.unsads.com/USB_Protocol#Service_identifiers):

	54 FD SA SA SS SS DA DA DS DS DC DC SZ AK SQ CK [data part]
*/

typedef unsigned char	uint8_t;
typedef unsigned short	uint16_t;
typedef unsigned long	uint32_t;

/* */

typedef struct
{
	uint16_t	unused;
	uint16_t	src_addr;
	uint16_t	src_id;
	uint16_t	dst_addr;
	uint16_t	dst_id;
	uint16_t	data_sum;
	uint8_t		data_size;
	uint8_t		ack;
	uint8_t		seq;
	uint8_t		hdr_sum;
} Packet;

typedef struct
{
	uint16_t		type;
	const char*		name;
} ServiceId;

static const ServiceId sids[] = 
{
	{ 0x00FE, "Reception Acknowledgment" },
	{ 0x00FF, "Reception Ack" },
	{ 0x4002, "Echo" },
	{ 0x4003, "Device Address Request" },
	{ 0x4020, "Device Information" },
	{ 0x4021, "Screen Capture" },
	{ 0x4024, "Screen Capture w/ RLE" },
	{ 0x4050, "Login" },
	{ 0x4060, "File Management" },
	{ 0x4080, "OS Installation" },
	{ 0x40DE, "Service Deconnection" },
	{ 0 },
};

/* */

int is_a_sid(uint8_t id)
{
  int i;
  
  for(i=0; sids[i].name; i++)
    if(id == sids[i].type)
      break;
  return i;
}

const char* name_of_sid(uint8_t id)
{
	int i;
  
	for(i=0; sids[i].name; i++)
		if(id == sids[i].type)
			return sids[i].name;
	return "";
}

const char* ep_way(int ep)
{
	if(ep == 0x01) return "TI>PC";
	else if(ep == 0x02) return "PC>TI";
	else return "XX>XX";
}

/* */

int add_sid(uint16_t* array, uint16_t id, int *count)
{
	int i;
	
	for(i = 0; i < *count; i++)
		if(array[i] == id)
			return 0;

	array[++i] = id;
	*count = i;

	return i;
}

int add_addr(uint16_t* array, uint8_t addr, int *count)
{
	int i;
	
	for(i = 0; i < *count; i++)
		if(array[i] == addr)
			return 0;

	array[++i] = addr;
	*count = i;

	return i;
}

/* */

static FILE *hex = NULL;
static FILE *log = NULL;

int hex_read(unsigned char *data)
{
	static char line[256];
	static int idx = 0;
	int ret;

	if(feof(hex))
		return -1;

	ret = fscanf(hex, "%02X", data);
	if(ret < 1)
		return -1;
	fgetc(hex);
	idx++;

	if(idx >= 16)
	{
		int i;

		idx = 0;
		for(i = 0; (i < 67-49) && !feof(hex); i++)
			fgetc(hex);
	}
    
	return 0;
}

uint16_t sid_found[256] = { 0 };
uint16_t addr_found[256] = { 0 };
int sif=0, af=0;

int dusb_write(int dir, uint8_t data)
{
	static int array[20];
  	static int i = 0;
	static unsigned long state = 1;
	static uint16_t src_addr, src_id;
	static uint16_t dst_addr, dst_id;
	static uint8_t data_size, ack, sq;
	static int cnt;

  	if (log == NULL)
    		return -1;

	array[i++ % 16] = data;

	switch(state)	// Finite State Machine
	{
	case 1:
	case 2:
		break;

	case 3: 
		break;
	case 4: 
		src_addr = (array[3] << 8) | (array[2] << 0);
		fprintf(log, "%04x:", src_addr);
		break;

	case 5: 
		break;
	case 6: 
		src_id = (array[5] << 8) | (array[4] << 0);
		fprintf(log, "%04x->", src_id);
		break;

	case 7: 
		break;
	case 8: 
		dst_addr = (array[7] << 8) | (array[6] << 0);
		fprintf(log, "%04x:", dst_addr);
		break;

	case 9: 
		break;
	case 10: 
		dst_id = (array[9] << 8) | (array[8] << 0);
		fprintf(log, "%04x ", dst_id);
		break;
		
	case 11:	break;	// checksum
	case 12: break;
		
	case 13:
		data_size = array[12];
		break;

	case 14: 
		ack = array[13];
		fprintf(log, "AK=%02x ", ack);
		break;

	case 15:
		sq = array[13];
		fprintf(log, "SQ=%02x ", sq);

		fprintf(log, "(%i bytes)\n", data_size);
		cnt = 0;
		break;


	default:
		fprintf(log, "%02X ", data);

		if(!(++cnt % 12))
			fprintf(log, "\n\t\t");
		
		if(--data_size == 0)
		{
			fprintf(log, "\n");
			state = 0;
		}
		break;
	}

	if(state == 0)
	{
		fprintf(log, "\n");
		i = 0;
	}
	state++;	

  	return 0;
}

int dusb_decomp(const char *filename)
{
	char src_name[1024];
	char dst_name[1024];
	unsigned char data;
	int i;

	strcpy(src_name, filename);
    strcat(src_name, ".hex");

	strcpy(dst_name, filename);
    strcat(dst_name, ".pkt");
    
	hex = fopen(src_name, "rt");
    if(hex == NULL)
    {
        fprintf(stderr, "Unable to open this file: %s\n", src_name);
        return -1;
    }

	log = fopen(dst_name, "wt");
	if(log == NULL)
    {
        fprintf(stderr, "Unable to open this file: %s\n", dst_name);
        return -1;
    }

	{
		char line[256];

		fgets(line, sizeof(line), hex);
		fgets(line, sizeof(line), hex);
		fgets(line, sizeof(line), hex);
	}

	fprintf(log, "TI packet decompiler for D-USB, version 1.0\n");

	while(hex_read(&data) != -1)
	{
		dusb_write(0, data);
	}

	fprintf(log, "() Service IDs found: ");
	for(i = 0; i < sif; i++) fprintf(log, "%04x ", sid_found[i]);
	fprintf(log, "\n");
	fprintf(log, "() Addresses found: ");
	for(i = 0; i < af; i++) fprintf(log, "%04x ", addr_found[i]);
	fprintf(log, "\n");

	fclose(hex);

	return 0;
}

int main(int argc, char **argv)
{


	if(argc < 2)
    {
		fprintf(stderr, "Usage: hex2nsp [file]\n");
		exit(0);
    }

	return dusb_decomp(argv[1]);
}
