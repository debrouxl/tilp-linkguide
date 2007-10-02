/*  xml2hex - a SnoopyPro xml to hex converter
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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <process.h>

#define TOKEN1	"<payloadbytes>"
#define TOKEN2	"</payloadbytes>"

// Format: <payloadbytes>08680000</payloadbytes>

int fill_buf(FILE *f, char data, int flush)
{
	static char buf[16];
	static unsigned int cnt = 0;
	unsigned int i, j;

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
	int toggle = 0;
	
	if(argc < 2)
	{
		fprintf(stderr, "Usage: xml2hex [file]\n");
		return -1;
	}

	strcpy(src_name, filename);
    strcat(src_name, ".xml");
    
    strcpy(dst_name, filename);
    strcat(dst_name, ".hex");

	fi = fopen(src_name, "rt");
	fo = fopen(dst_name, "wt");
	if(fi == NULL || fo == NULL)
		return -1;

	fprintf(fo, "XML2HEX converter for use with SnoopyPro, version 1.0\n");
	fprintf(fo, "Copyright (c) 2005, Romain Lievin\n");
	fprintf(fo, "\n");

	while(!feof(fi))
	  {
	    char line[4096];
		char *p, *q;
		unsigned int i;
		char data[3];
		char value;

	    fgets(line, sizeof(line), fi);
	    line[strlen(line)-1] = '\0';

		if(strlen(line) == 0)
			continue;

		if(!strcmp(line, "<function>BULK_OR_INTERRUPT_TRANSFER</function>"))
			toggle = 1;

		if(!toggle)
			continue;

		if(!strstr(line, TOKEN1))
			continue;

		if(strlen(line) <= strlen(TOKEN1)+strlen(TOKEN2))
			continue;

		p = line + strlen(TOKEN1);
		q = strchr(p, '<');
		if(q != NULL)
			*q = '\0';

		for(i = 0; i < strlen(p) / 2; i++)
		{
			data[0] = p[2*i];
			data[1] = p[2*i+1];
			data[2] = '\0';

			sscanf(data, "%02X", &value);
			//fprintf(stdout, "%02x ", 0xff & value);
			fill_buf(fo, value, 0);
		}
	  }
	fill_buf(fo, 0, !0);

	fclose(fi);
	fclose(fo);

#if 0
	{
		char *args[4] = { 0 };
		
		//execl("LOG2PKT.EXE", "LOG2PKT.EXE", dst_name, NULL);

		args[0] = "LOG2PKT.EXE";
		args[1] = dst_name;
		args[2] = NULL;

		execv("LOG2PKT.EXE", args);
	}
#endif

	return 0;
}
