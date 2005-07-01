/*  pkdecomp - an TI packet decompiler for insulating packets
 *  Copyright (C) 2002, 2005  Romain Lievin
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
#include <sys/stat.h>

#define WIDTH	12

static const unsigned char machine_id[] =
{
  0x00, 0x02, 0x03, 0x05, 0x06, 0x07, 0x08, 0x08, 0x09, 0x23,
  0x00, 0x82, 0x83, 0x85, 0x86, 0x74, 0x98, 0x88, 0x89, 0x73,
  0xff
};
static const unsigned char command_id[] =
{
	0x06, 0x09, 0x15, 0x2d, 0x36, 0x47, 0x56, 0x5A, 0x68, 0x6D, 
	0x74, 0x78, 0x87, 0x92, 0xA2, 0xB7, 0xC9, 
	0xff
};

static const char string_id[][8] =
{
	"VAR", "CTS", "XDP", "FLH", "SKP", "SID", "ACK", "ERR", "RDY", "SCR", "RID",
	"CNT", "KEY", "EOT", "REQ", "IND", "RTS", 
	""
};

static const int cmd_with_data[] =
{
	!0, 0, !0, 0, !0, !0, 0, 0, 0, 0, 0, 0, 0, 0, !0, !0, !0, -1
};

int is_a_machine_id(unsigned char id)
{
  int i;
  
  for(i=0; machine_id[i] != 0xff; i++)
    if(id == machine_id[i])
      break;
  return i;
}

int is_a_command_id(unsigned char id)
{
  int i;

  for(i=0; command_id[i] != 0xff; i++)
    if(id == command_id[i])
      break;

  return i;
}


/*
  Format of data: 8 hexadecimal numbers with spaces
*/
int pkdecomp(const char *filename)
{
    char src_name[1024];
    char dst_name[1024];
    FILE *fi, *fo;
    long file_size;
    struct stat st;
    unsigned char *buffer;
    int i, j, k, l;
    int num_bytes;
	char str[17];
	int mid, cid;
    
    strcpy(src_name, filename);
    strcat(src_name, ".log");
    
    strcpy(dst_name, filename);
    strcat(dst_name, ".pkt");
    
    stat(src_name, &st);
    file_size = st.st_size;
    
    buffer = (unsigned char*)calloc(file_size/2, 1);
    memset(buffer, 0xff, file_size/2);
    if(buffer == NULL)
    {
        fprintf(stderr, "calloc error.\n");
        exit(-1);
    }
    
    fi = fopen(src_name, "rt");
    fo = fopen(dst_name, "wt");
    
    if(fi == NULL)
    {
        fprintf(stderr, "Unable to open this file: %s\n", src_name);
        return -1;
    }

    fprintf(fo, "TI packet decompiler, version 1.1\n");
    
    i = 0;
    while(!feof(fi))
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

    /*
    for(i=0; i<num_bytes; i++)
      fprintf(stdout, "%02X ", buffer[i]);
    */
    
    for(i = 0; i < num_bytes;)
    {
        int index;
        unsigned int length;

        if(is_a_machine_id(mid = buffer[i]) != -1)
			fprintf(fo, "\n%02X ", buffer[i++]);
        else
		{
		  fprintf(stderr, "Error 1!\n");
		  fclose(fi);
		  fclose(fo);
		  return -1;
		}      

        index = is_a_command_id(cid = buffer[i]);
        if(index != -1)
			fprintf(fo, "%02X ", buffer[i++]);
        else
		{
			fprintf(stderr, "Error 2: index=%i, data=%02X !\n", index, buffer[i-1]);
			fclose(fi);
			fclose(fo);
			return -1;
		}      

        length = buffer[i];
        length |= buffer[i+1] << 8;
        fprintf(fo, "%02X ", buffer[i++]);
        fprintf(fo, "%02X ", buffer[i++]);

		for(l = 4; l <= WIDTH; l++)
			fprintf(fo, "   ");
		fprintf(fo, "  | ");
		fprintf(fo, "%s", string_id[is_a_command_id(cid)]);

        if(cmd_with_data[index])
		{
			fprintf(fo, "\n");
			for(j = 0; j < length; /*j++*/ )
			{
				fprintf(fo, "\t");
				for(k = 0; k < WIDTH && j < length; k++, j++)
				{
					if(buffer[i] >= 0x20 && buffer[i] <= 0x7f)
						str[k] = buffer[i] ;
					else
						str[k] = '.';
					fprintf(fo, "%02X ", buffer[i++]);
				}
				str[k] = '\0';

				for(l = k; l < WIDTH; l++)
					fprintf(fo, "   ");

				fprintf(fo, " | %s", str);
				fprintf(fo, "\n");
			}

			fprintf(fo, "\t%02X ", buffer[i++]);
			fprintf(fo, "%02X ", buffer[i++]);
		}
    }
    
	fclose(fi);
	fclose(fo);
    
	return 0;
}

int main(int argc, char **argv)
{
	if(argc < 2)
    {
		fprintf(stderr, "Usage: log2pkt [file]\n");
		exit(0);
    }
  
	return pkdecomp(argv[1]);
}
