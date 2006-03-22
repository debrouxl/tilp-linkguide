/*  snif2pkt - a packet decompiler for use with SniffUsb
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

	| packet header    | data												 |
	|				   | data header         |								 |
	| size		  | ty | size		 | code	 | data							 |
	|			  |    |			 |		 |								 |
	| 00 00 00 10 | 04 | 00 00 00 0A | 00 01 | 00 03 00 01 00 00 00 00 07 D0 |	
*/

#define TOKEN1	"-- URB_FUNCTION_BULK_OR_INTERRUPT_TRANSFER"
#define TOKEN2	"  TransferBufferMDL    = "
#define TOKEN3	"  UrbLink              = "
#define TOKEN4	"  PipeHandle           = 813f4184 [endpoint "
#define TOKEN5	"    00000000: "

typedef unsigned char	uint8_t;
typedef unsigned short	uint16_t;
typedef unsigned long	uint32_t;

typedef struct
{
	uint8_t		type;
	const char*	name;
	int			data_hdr;
	int			data;
} Packet;

typedef struct
{
	uint16_t		type;
	const char*		name;
} Opcode;

static const Packet packets[] = 
{
	{ 0x01, "Handshake", 0, 4}, { 0x02, "Response", 0, 4}, 
	{ 0x03, "Data", 1, 6}, { 0x04, "Last data", 1, 6}, 
	{ 0x05, "Acknowledge", 0, 2}, 
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

int is_a_packet_with_data_header(uint8_t id)
{
	int i;
  
  for(i=0; packets[i].name; i++)
    if(id == packets[i].type)
		if(packets[i].data_hdr)
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

const char* ep_way(int ep)
{
	if(ep == 0x01) return "TI>PC";
	else if(ep == 0x02) return "PC>TI";
	else return "??>??";
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

typedef struct
{
	uint8_t	ep;			// endpoint aka direction (0x81 = IN, 0x02 = OUT)
	uint8_t	size;		// #bytes
	uint8_t	data[256];	// data
} Urb;

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

Urb** buffer;

int add_pkt_type(uint8_t* array, uint8_t type, int *count)
{
	int i;
	
	for(i = 0; i < *count; i++)
		if(array[i] == type)
			return 0;

	array[++i] = type;
	*count = i;

	return i;
}

int add_data_code(uint16_t* array, uint16_t code, int *count)
{
	int i;
	
	for(i = 0; i < *count; i++)
		if(array[i] == code)
			return 0;

	array[i++] = code;
	*count = i;

	return i;
}

/*
  Format of data: 12 hexadecimal numbers with spaces
*/
int pkt_write(const char *filename, int nurbs)
{
	FILE *fo;
	int i;
	unsigned int j;

	uint8_t pkt_type_found[256] = { 0 };
	uint16_t data_code_found[256] = { 0 };
	int ptf=0, dcf=0;

	fo = fopen(filename, "wt");
    if(fo == NULL)
    {
        fprintf(stderr, "Unable to open this file: %s\n", filename);
        return -1;
    }

#if 0

	for(ptr = buffer; *ptr; ptr++)
	{
		Urb *u = *ptr;

		fprintf(fo, "%i: (%3i) ", u->ep, u->size);

		for(i = 0; i <u->size; i++)
			fprintf(fo, "%02X ", u->data[i]);
		fprintf(fo, "\n");
	}

#else
    
    fprintf(fo, "TI packet decompiler for D-USB, version 1.0\n");

	// process data
	for(i = 0; i < nurbs; i++)
    {
		Urb *u = buffer[i];
		uint32_t pkt_size;
		uint8_t pkt_type;
		uint32_t data_size;
		uint16_t data_code;
		static int concat = 0;

		pkt_size = u->data[3] | (u->data[2] << 8) | (u->data[1] << 16) | (u->data[0] << 24);
		pkt_type = u->data[4];

		fprintf(fo, "%08x (%02x) ", pkt_size, pkt_type);
		fprintf(fo, "\t\t\t\t");
		fprintf(fo, "| %s: %s\n", ep_way(u->ep), name_of_packet(pkt_type));

		add_pkt_type(pkt_type_found, pkt_type, &ptf);

		if((pkt_type == 1) || pkt_type == 2)
			data_size = u->data[8] | (u->data[7] << 8) | (u->data[6] << 16) | (u->data[5] << 24);
		else if(pkt_type == 5)
			data_size = u->data[6] | (u->data[5] << 8);
		
		if(is_a_packet_with_data_header(pkt_type) && !concat)
		{
			if(pkt_type == 0x03 && pkt_size == 0xFA)
				concat = 1;

			data_size = u->data[8] | (u->data[7] << 8) | (u->data[6] << 16) | (u->data[5] << 24);
			data_code = u->data[10] | (u->data[9] << 8);
			
			add_data_code(data_code_found, data_code, &dcf);
		
			fprintf(fo, "\t%08x {%04x} ", data_size, data_code);
			fprintf(fo, "\t\t");
			fprintf(fo, "| %s: %s\n", "CMD", name_of_data(data_code));

			for(j = 0; j < data_size && j < pkt_size-5; j++)
			{
				fill_buf(fo, u->data[11 + j], 0);
			}
			fill_buf(fo, 0, !0);
		}
		else if(concat && ((pkt_type == 0x03) || (pkt_type == 0x04)))
		{
			if(pkt_type == 0x04)
				concat = 0;

			for(j = 0; j < pkt_size; j++)
			{
				fill_buf(fo, u->data[5 + j], 0);
			}
			fill_buf(fo, 0, !0);
		}
		else
		{
			if(pkt_type == 5)
				fprintf(fo, "\t[%04x]\t\t", data_size >> 16);
			else
				fprintf(fo, "\t[%08x]", data_size);
			fprintf(fo, "\t\t\t\t\t");
			fprintf(fo, "|\n");
		}

		fprintf(fo, "\n");
	}

	fprintf(fo, "() Packet types found: ");
	for(i = 0; i < ptf; i++) fprintf(fo, "%02x ", pkt_type_found[i]);
	fprintf(fo, "\n");
	fprintf(fo, "{} Data codes found: ");
	for(i = 0; i < dcf; i++) fprintf(fo, "%04x ", data_code_found[i]);
	fprintf(fo, "\n");

#endif

	fclose(fo);
	return 0;
}

//"     00000000: 00 00 00 0a 04 00 00 00 04 00 12 00 00 07 d0"
int read_line(const char* str, uint8_t* data)
{
	const char *s = str + strlen(TOKEN5);
	unsigned int i;

	for(i = 0; (i <= strlen(s) / 3) && (i < 16); i++)
		sscanf(&s[3*i], "%02x", &data[i]);

	return i;
}

/*
	Packet can have up to 255 bytes and we always have a TI packet per URB. Easy !
 */
int snif_read(const char* filename, int* nurbs)
{
    FILE *fi;
	char str[1024];
	int ret = 0;
	int found = 0;
	int nlines = 0;
	int j = 0;
	int ep = -1;
    
    // open file
	fi = fopen(filename, "rt");
    if(fi == NULL)
    {
        fprintf(stderr, "Unable to open this file: %s\n", filename);
        return -1;
    }

	// first pass: count data
	while(!feof(fi))
	{
		fgets(str, sizeof(str), fi);

		if(!strncmp(str, TOKEN1, strlen(TOKEN1)))
			found = 1;

		if(found && !strncmp(str, TOKEN2, strlen(TOKEN2)))
		{
			for(fgets(str, sizeof(str), fi);
				strncmp(str, TOKEN3, strlen(TOKEN3)); 
				fgets(str, sizeof(str), fi))
			{
				nlines++;
			}
			found = 0;
		}

	}
    
	// allocate a NULL-terminated array of Urb's
    buffer = (Urb **)calloc(nlines + 1, sizeof(Urb *));
    if(buffer == NULL)
    {
        fprintf(stderr, "calloc error.\n");
        exit(-1);
    }

	// second pass: load data
    rewind(fi);
	while(!feof(fi))
	{
		fgets(str, sizeof(str), fi);

		if(!strncmp(str, TOKEN1, strlen(TOKEN1)))
		{
			found = 1;
			fgets(str, sizeof(str), fi);
			sscanf(str + strlen(TOKEN4), "0x%08x]", &ep);
		}

		if(found && !strncmp(str, TOKEN2, strlen(TOKEN2)))
		{
			uint8_t line[20], data[256];
			int size;
			int i = 0;

			for(fgets(str, sizeof(str), fi);
				strncmp(str, TOKEN3, strlen(TOKEN3)); 
				fgets(str, sizeof(str), fi))
			{
				str[strlen(str)-1] = 0;

				size = read_line(str, line);
				memcpy(&data[i], line, size);
				i += size;

				//printf("%s (%i)\n", str, size);
				printf(".");
			}
	
			if(i > 0)
			{
				Urb *urb = calloc(1, sizeof(Urb));
				urb->ep = ep & ~0x80;
				urb->size = i;
				memcpy(urb->data, data, 255);
				buffer[j++] = urb;
			}

			found = 0;
		}

	}

	*nurbs = j-1;
	fclose(fi);
    
	return 0;
}

int main(int argc, char **argv)
{
	char* filename = argv[1];
	char src_name[1024];
	char dst_name[1024];
	int nurbs;

	if(argc < 2)
    {
		fprintf(stderr, "Usage: log2pkt [file]\n");
		exit(0);
    }

	strcpy(src_name, filename);
    strcat(src_name, ".log");

	strcpy(dst_name, filename);
    strcat(dst_name, ".pkt");
    
	snif_read(src_name, &nurbs);
	pkt_write(dst_name,  nurbs);

	free(buffer);
  
	return 0;
}
