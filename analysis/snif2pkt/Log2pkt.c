/*  pkdecomp - an TI packet decompiler for insulating packets
 *  Copyright (C) 2005  Romain Lievin
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

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

/*
	Format:

	| packet header    | data header         | data							 |
	| size		  | ty | size		 | code	 |								 |
	|			  |    |			 |		 |								 |
	| 00 00 00 10 | 04 | 00 00 00 0A | 00 01 | 00 03 00 01 00 00 00 00 07 D0 |	
*/

typedef unsigned char	uint8_t;
typedef unsigned short	uint16_t;
typedef unsigned long	uint32_t;

typedef struct
{
	uint8_t		type;
	const char*	name;
	int			data;
} Packet;

typedef struct
{
	uint16_t		type;
	const char*		name;
} Opcode;

static const Packet packets[] = 
{
	{ 0x01, "Handshake", 0}, { 0x02, "Ack handshake", 0}, 
	{ 0x03, "Data Packet", 1}, { 0x04, "Data packet (last)", 1}, 
	{ 0x05, "Acknowledge", 0}, 
	{ 0 },
};

static const Opcode opcodes[] = 
{
	{ 0xaa00, "CTS"}, { 0xdd00, "EOT"}, { 0xee00, "ERR"}, { 0x0009, "RDIR"}, 
	{ 0x000a, "DIR"}, { 0x000b, "RTS"}, { 0x000c, "REQ"}, { 0x000d, "???"}, 
	{ 0x0010, "DEL"},
	{ 0 },
};

int is_a_packet(uint8_t id)
{
  int i;
  
  for(i=0; packets[i].name; i++)
    if(id == packets[i].type)
      break;
  return i;
}

const char* name_of_packet(uint8_t id)
{
	int i;
  
	for(i=0; packets[i].name; i++)
		if(id == packets[i].type)
			return packets[i].name;
	return "";
}

int is_a_packet_with_data(uint8_t id)
{
	int i;
  
  for(i=0; packets[i].name; i++)
    if(id == packets[i].type)
		if(packets[i].data)
			return 1;

  return 0;
}

int is_a_opcode(uint16_t id)
{
  int i;

  for(i=0; opcodes[i].name; i++)
    if(id == opcodes[i].type)
      break;

  return i;
}

const char* name_of_data(uint16_t id)
{
	int i;
  
	for(i=0; opcodes[i].name; i++)
		if(id == opcodes[i].type)
			return opcodes[i].name;
	return "";
}

typedef struct
{
	uint32_t	size;
	uint8_t		type;
} PktHdr;

typedef struct
{
	uint32_t	size;
	uint16_t	opcode;
} DataHdr;

#define WIDTH	12

int fill_buf(FILE *f, uint8_t data, int flush)
{
	static char buf[WIDTH];
	static unsigned int cnt = 0;
	unsigned int i, j;

	if(!flush)
		buf[cnt++] = data;

	if((cnt >= WIDTH) || flush)
	{
		//printf(".");
		fprintf(f, "    ");
		for(i = 0; i < cnt; i++)
			fprintf(f, "%02X ", 0xff & buf[i]);

		if(flush)
			for(j = i; j < WIDTH; j++)
				fprintf(f, "   ");

		fprintf(f, "| ");
		for(i = 0; i < cnt; i++)
			fprintf(f, "%c", isalnum(buf[i]) ? buf[i] : '.');

		fprintf(f, "\n");
		cnt = 0;
	}

	return 0;
}

/*
  Format of data: 8 hexadecimal numbers with spaces
*/
int pkdecomp(const char *filename, int resync)
{
    char src_name[1024];
    char dst_name[1024];
    FILE *fi, *fo;
    long file_size;
    struct stat st;
    unsigned char *buffer;
    int i;
	unsigned int j;
    int num_bytes;
	char str[256];
	int ret = 0;
    
	// build filenames
    strcpy(src_name, filename);
    strcat(src_name, ".log");
    
    strcpy(dst_name, filename);
    strcat(dst_name, ".pkt");
    
    stat(src_name, &st);
    file_size = st.st_size;
    
	// allocate buffer
    buffer = (unsigned char*)calloc(file_size/2, 1);
    memset(buffer, 0xff, file_size/2);
    if(buffer == NULL)
    {
        fprintf(stderr, "calloc error.\n");
        exit(-1);
    }
    
	// open files
    fi = fopen(src_name, "rt");
    fo = fopen(dst_name, "wt");
    
    if(fi == NULL)
    {
        fprintf(stderr, "Unable to open this file: %s\n", src_name);
        return -1;
    }

    fprintf(fo, "TI packet decompiler for D-USB, version 1.0\n");

	// skip comments
	fgets(str, sizeof(str), fi);
	fgets(str, sizeof(str), fi);
	fgets(str, sizeof(str), fi);

	// read source file
	for(i = 0; !feof(fi);)
    {
        for(j = 0; j < 16 && !feof(fi); j++)
		{
			fscanf(fi, "%02X", &(buffer[i+j]));
			fgetc(fi);
		}
        i += j;

        for(j=0; j<18 && !feof(fi); j++)
			fgetc(fi);
    }
    num_bytes = i-1; // -1 due to EOF char
    fprintf(stdout, "%i bytes read.\n", num_bytes);

	// process data
	for(i = 0; i < num_bytes;)
    {
		uint32_t pkt_size;
		uint8_t pkt_type;
		uint32_t data_size;
		uint16_t data_code;

		pkt_size = buffer[3+i] | (buffer[2+i] << 8) | (buffer[1+i] << 16) | (buffer[0+i] << 24);
		pkt_type = buffer[4+i];
		data_size = buffer[8+i] | (buffer[7+i] << 8) | (buffer[6+i] << 16) | (buffer[5+i] << 24);

		fprintf(fo, "%08x %02x ", pkt_size, pkt_type);
		fprintf(fo, "\t\t\t\t");
		fprintf(fo, "| %s\n", name_of_packet(pkt_type));
		
		if(is_a_packet_with_data(pkt_type))
		{
			data_code = buffer[10+i] | (buffer[9+i] << 8);
		
			fprintf(fo, "\t%08x %04x ", data_size, data_code);
			fprintf(fo, "\t\t");
			fprintf(fo, "|  %s\n", name_of_data(data_code));

			for(j = 0; j < data_size && j < pkt_size-5; j++)
			{
				fill_buf(fo, buffer[11 + i + j], 0);
			}
			fill_buf(fo, 0, !0);
		}		
		else
		{
			if(pkt_type == 5)
				fprintf(fo, "\t%04x\t\t", data_size >> 16);
			else
				fprintf(fo, "\t%08x", data_size);
			fprintf(fo, "\t\t\t\t\t");
			fprintf(fo, "|\n");
		}

		fprintf(fo, "\n");

		i += pkt_size + 5;
		printf("%i ", i);
	}

	if(ret < 0)
		fprintf(stdout, "Error %i\n", -ret);

	free(buffer);
	fclose(fi);
	fclose(fo);
    
	return ret;
}

int main(int argc, char **argv)
{
	int resync = 0;


	if(argc < 2)
    {
		fprintf(stderr, "Usage: log2pkt [file]\n");
		exit(0);
    }

	if(argc > 2)
		resync = !0;
  
	return pkdecomp(argv[1], resync);
}
