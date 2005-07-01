#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define TOKEN1	"<payloadbytes>"
#define TOKEN2	"</payloadbytes>"

// Format: <payloadbytes>08680000</payloadbytes>

int main(int argc, char **argv)
{
	char *filename = arg[1];
	char src_name[1024];
    char dst_name[1024];
	FILE *fi;
	FILE *fo;
	
	printf("argc = %i\n", argc);
	if(argc < 2)
		return -1;

	printf("argv = %s\n", argv[1]);

	strcpy(src_name, filename);
    strcat(src_name, ".xml");
    
    strcpy(dst_name, filename);
    strcat(dst_name, ".log");

	fi = fopen(argv[1], "rt");
	fo = fopen(argv[2], "wt");
	if(fi == NULL || fo == NULL)
		return -1;

	while(!feof(fi))
	  {
	    char line[4096];
		char *p, *q;
		unsigned int i;
		char data[3];
		char ascii[4096];
		char value;

	    fgets(line, sizeof(line), fi);
	    line[strlen(line)-1] = '\0';

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
			ascii[i] = value;

			//fprintf(stdout, "%s ", data);
			fprintf(fo,     "%s ", data);
		}

		for(i = 0; i < strlen(p) / 2; i++)
		{
			//fprintf(stdout, "%c", isalnum(ascii[i]) ? ascii[i] : '.');
			fprintf(fo,     "%c", isalnum(ascii[i]) ? ascii[i] : '.');
		}

		//fprintf(stdout, "\n");
		fprintf(fo,     "\n");
	  }

	fclose(fi);
	fclose(fo);

	return 0;
}
