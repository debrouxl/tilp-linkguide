/*  log2hex - a SniffUsb log to hex converter
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
	Format:

	| packet header    | data												 |
	|				   | data header         |								 |
	| size		  | ty | size		 | code	 | data							 |
	|			  |    |			 |		 |								 |
	| 00 00 00 10 | 04 | 00 00 00 0A | 00 01 | 00 03 00 01 00 00 00 00 07 D0 |	
*/

#define TOKEN0  "-- URB_FUNCTION_CONTROL_TRANSFER"
#define TOKEN1	"-- URB_FUNCTION_BULK_OR_INTERRUPT_TRANSFER"
#define TOKEN2	"  TransferBufferMDL    = "
#define TOKEN3	"  UrbLink              = "
#define TOKEN4	"  PipeHandle           = 813f4184 [endpoint "
#define TOKEN5	"    00000000: "

typedef unsigned char	uint8_t;
typedef unsigned short	uint16_t;
typedef unsigned long	uint32_t;

int fill_buf(FILE *f, char data, int flush)
{
	static char buf[16];
	static unsigned int cnt = 0;
	unsigned int i, j;

	if(!flush)
		buf[cnt++] = data;

	if(cnt > 15 || flush)
	{
		printf(".");
		for(i = 0; i < cnt; i++)
			fprintf(f, "%02X ", 0xff & buf[i]);

		if(flush)
			for(j = i; j < 16; j++)
				fprintf(f, "   ");

		fprintf(f, "| ");
		for(i = 0; i < cnt; i++)
			fprintf(f, "%c", isalnum(buf[i]) ? buf[i] : '.');

		fprintf(f, "\n");
		cnt = 0;
	}

	return 0;
}


int main(int argc, char **argv)
{
	char *filename = argv[1];
	char src_name[1024];
    char dst_name[1024];
	FILE *fi;
	FILE *fo;

	if(argc < 2)
	{
		fprintf(stderr, "Usage: log2hex [file]\n");
		return -1;
	}

	strcpy(src_name, filename);
    strcat(src_name, ".log");
    
    strcpy(dst_name, filename);
    strcat(dst_name, ".hex");

	fi = fopen(src_name, "rt");
	fo = fopen(dst_name, "wt");
	if(fi == NULL || fo == NULL)
		return -1;

	fprintf(fo, "LOG2HEX converter for use with SniffUsb, version 1.0\n");
	fprintf(fo, "Copyright (c) 2005, Romain Lievin\n");
	fprintf(fo, "\n");
    
	while(!feof(fi))
	  {
	    char line[4096];
		char *p;
		unsigned int i;
		char data[3];
		char value;

	    fgets(line, sizeof(line), fi);
	    line[strlen(line)-1] = '\0';

		if(strlen(line) < 13)
			continue;

		if(strstr(line, TOKEN1))
		{
			int end = 0;

			while(!feof(fi))
			{
				fgets(line, sizeof(line), fi);
				line[strlen(line)-1] = '\0';

				if((line[0] != ' ') || (line[1] != ' ') || (line[2] != ' ') || (line[3] != ' '))
					continue;
				else
					break;
			}

			while(!feof(fi))
			{
				if((line[0] != ' ') || (line[1] != ' ') || (line[2] != ' ') || (line[3] != ' '))
					break;

				p = line + 14;

				for(i = 0; i < strlen(p);)
				{
					data[0] = p[i++];
					data[1] = p[i++];
					data[2] = p[i++];
					data[2] = '\0';	

					sscanf(data, "%02X", &value);
					fill_buf(fo, value, 0);
				}
			
				fgets(line, sizeof(line), fi);
				line[strlen(line)-1] = '\0';
			}
		}
	}
	fill_buf(fo, 0, !0);

	fclose(fi);
	fclose(fo);
  
	return 0;
}
