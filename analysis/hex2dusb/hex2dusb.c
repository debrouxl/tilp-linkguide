/*  hex2dusb - an D-USB packet decompiler
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


/* !!! Not working yet !!! */

#include <conio.h>
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
	{ 0x01, "Buffer Size Request", 0, 4 }, 
	{ 0x02, "Buffer Size Allocation", 0, 4 }, 
	{ 0x03, "Virtual Packet Data with Continuation", 1, 6 }, 
	{ 0x04, "Virtual Packet Data Final", 1, 6 }, 
	{ 0x05, "Virtual Packet Data Acknowledgement", 0, 2 }, 
	{ 0 },
};

static const Opcode opcodes[] = 
{
	{ 0x0001, "Ping / Set Mode" }, 
	{ 0x0002, "Begin OS Transfer" },
	{ 0x0003, "Acknowledgement of OS Transfer" },
	{ 0x0005, "OS Data" },
	{ 0x0006, "Acknowledgement of EOT" },
	{ 0x0007, "Parameter Request"}, 
	{ 0x0008, "Parameter Data"}, 
	{ 0x0009, "Request Directory Listing" },
	{ 0x000a, "Variable Header" },
	{ 0x000b, "Request to Send" },
	{ 0x000c, "Request Variable" },
	{ 0x000D, "Variable Contents" },
	{ 0x000e, "Parameter Set"},
	{ 0x0010, "Delete Variable"}, 
	{ 0x0011, "Unknown"}, 
	{ 0x0012, "Acknowledgement of Mode Setting"}, 
	{ 0xaa00, "Acknowledgement of Data"}, 
	{ 0xbb00, "Acknowledgement of Parameter Request"},	
	{ 0xdd00, "End of Transmission"}, 
	{ 0xee00, "Error"},
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

	return "unknown";
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
	uint8_t		ep;			// endpoint aka direction (0x81 = IN, 0x02 = OUT)
	uint32_t	size;		// size of packet
	uint8_t		data[2048];	// data
	uint32_t	len;		// length of data

	int			frag;		// fragmented
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

Urb** urbs;

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

	for(ptr = urbs; *ptr; ptr++)
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
		Urb *u = urbs[i];
		uint32_t pkt_size = 0;
		uint8_t pkt_type = 0;
		uint32_t data_size = 0;
		uint16_t data_code = 0;
		static int concat = 0;
		static uint32_t buf_size = 0x3ff;

		pkt_size = u->data[3] | (u->data[2] << 8) | (u->data[1] << 16) | (u->data[0] << 24);
		pkt_type = u->data[4];

		fprintf(fo, "%08x (%02x) ", pkt_size, pkt_type);
		fprintf(fo, "\t\t\t\t\t\t\t");
		fprintf(fo, "| %s: %s %s\n", ep_way(u->ep), name_of_packet(pkt_type), u->frag ? "(fragmented)" : "");

		add_pkt_type(pkt_type_found, pkt_type, &ptf);

		if((pkt_type == 1) || pkt_type == 2)
		{
			data_size = u->data[8] | (u->data[7] << 8) | (u->data[6] << 16) | (u->data[5] << 24);
			if(pkt_type == 2)
				buf_size = data_size;
		}
		else if(pkt_type == 5)
			data_size = u->data[6] | (u->data[5] << 8);
		
		if(is_a_packet_with_data_header(pkt_type) && !concat)
		{
			//if(pkt_type == 0x03 && (pkt_size == 0xFA || pkt_size == 0x3FF))
			if((pkt_type == 0x03) && (pkt_size == buf_size))
				concat = 1;

			data_size = u->data[8] | (u->data[7] << 8) | (u->data[6] << 16) | (u->data[5] << 24);
			data_code = u->data[10] | (u->data[9] << 8);
			
			add_data_code(data_code_found, data_code, &dcf);
		
			fprintf(fo, "\t%08x {%04x} ", data_size, data_code);
			fprintf(fo, "\t\t\t\t\t");
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
			if(pkt_type == 0x05)
				fprintf(fo, "\t[%04x]\t", data_size);
			else
				fprintf(fo, "\t[%08x]", data_size);
			fprintf(fo, "\t\t\t\t\t\t\t");
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

// parse file on a per-urb basis
static Urb* load_urb(FILE *fi)
{
	int found = 0;
	char str[1024];
	unsigned int ep = -1;
	unsigned int nbytes;
	Urb *urb;

	if(feof(fi))
		return NULL;
	
	urb = calloc(1, sizeof(Urb));

	for(nbytes = 0; found < 2 && !feof(fi);)
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
			uint8_t line[20], data[2048];
			unsigned int size;

			memset(data, 0, sizeof(data));

			for(fgets(str, sizeof(str), fi);
				strncmp(str, TOKEN3, strlen(TOKEN3)); 
				fgets(str, sizeof(str), fi))
			{
				str[strlen(str)-1] = 0;

				size = read_line(str, line);
				memcpy(&data[nbytes], line, size);
				nbytes += size;

				//printf("%s (%i)\n", str, size);
				//printf(".");
			}

			if(!nbytes)
				continue;

			urb->size = (data[3] | (data[2] << 8) | (data[1] << 16) | (data[0] << 24)) + 5;

			if(nbytes > 0)
			{
				urb->ep = ep & ~0x80;
				urb->len = nbytes;
				memcpy(urb->data, data, urb->len);
			}

			found = 2;
		}
	}

	return urb;
}

/*
	Packet can have up to 255 bytes and we often have a TI packet per URB but
	this is not always the case (especially with TiConnect&Titanium).
 */
int snif_read(const char* filename, int* nurbs)
{
    FILE *fi;
	char str[1024];
	int ret = 0;
	int found = 0;
	int nlines = 0;
	int i = 0;
	int j = 0;
	int k = 0;

	Urb *prev = NULL;
	int concat = 0;
   
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
    urbs = (Urb **)calloc(nlines + 1, sizeof(Urb *));
    if(urbs == NULL)
    {
        fprintf(stderr, "calloc error.\n");
        exit(-1);
    }

	fprintf(stdout, "Reading data:\n");

	// second pass: load data
    rewind(fi);
	while(!feof(fi))
	{
		Urb *tmp, *urb;
		
		urb = load_urb(fi);
		if(!urb) break;
		printf(".");

		// Big blocks of data on Titanium are fragmented. Re-assemble here
		if(urb->len < urb->size)
		{
			urb->frag = 1;
			i++;
			//printf("!! <%08x-%08x> \n", urb->len, urb->size);
			
			do
			{
				tmp = load_urb(fi);
				if(!tmp) break;
				printf("*");
				k++;
				
				memcpy(urb->data + urb->len, tmp->data, tmp->len);
				urb->len += tmp->len;

				free(tmp);
			} while(urb->len < urb->size);

			//printf("$ <%08x-%08x> \n", urb->len, urb->size);
		}
		//else
			//printf(<%08x-%08x> \n", urb->len, urb->size);

		urbs[j++] = urb;
	}

 	*nurbs = --j;
	fclose(fi);

	printf("\n");
	fprintf(stdout, "Processed %i blocks into %i packets (%i packets were fragmented into %i blocks).\n", j+k, j, i, k);
    
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

	free(urbs);

	//while(!kbhit());
  
	return 0;
}
