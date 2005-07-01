/* RS232 analyser
 * Copyright 2000 by R. Lievin
 */

#include <windows.h>
#include <stdio.h>
#include <conio.h>
#include <string.h>

#define BUFFER_SIZE 4096

#define VERSION "1.1"

int main(int argc, char **argv)
{
	DCB dcb;
	HANDLE hCom;
	BOOL fSuccess;
	char comPort[256] = "COM1";
	DWORD i;
	int ch;
	int c;
	COMMTIMEOUTS cto;
	FILE *txt;
	char filename[256] = "test.log";
	int j=0;
	int buffer[17];
	int k;
	int t = !0;
	int nBytes = 0;

	if(argc < 3)
	{
		printf("Command line: rs_sampler [port] [filename]\n");
		printf("port=COM1..4\n");
		printf("filename=test for example but without file extension.\n");
	}
	else
	{
		printf("Port: %s\n", argv[1]);
		strcpy(comPort, strupr(argv[1]));

		strcpy(filename, "test");
		if(strcmp(argv[2], ""))
		{
			strcpy(filename, argv[2]);
		}
		strcat(filename, ".txt");
		printf("Filename: %s\n", filename);
	}

	if( (txt  = fopen(filename, "wt" )) == NULL )
	{
		printf("Unable to open this file: <%s>\n", filename);
		return -1;
	}

	hCom = CreateFile(comPort, GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, 0, NULL);

	if(hCom == INVALID_HANDLE_VALUE)
	{
		printf("CreateFile failed with error %d.\n", GetLastError());
		return 1;
	}

	fSuccess = GetCommState(hCom, &dcb);
	if(!fSuccess)
	{
		printf("GetCommState failed with error %d.\n", GetLastError());
		return 2;
	}

	fSuccess = SetupComm(hCom, BUFFER_SIZE, BUFFER_SIZE);	// setup buffer size
	if(!fSuccess)
	{
		printf("SetCommState failed with error %d.\n", GetLastError());
		return 3;
	}

	dcb.BaudRate = CBR_57600;				// 9600 bauds
	dcb.fBinary = TRUE;						// Binary mode
	dcb.fParity = FALSE;					// Parity checking diabled
	dcb.fOutxCtsFlow = FALSE;				// No output flow control
	dcb.fOutxDsrFlow = FALSE;				// Idem
	dcb.fDtrControl = DTR_CONTROL_ENABLE;	// Provide power supply
	dcb.fDsrSensitivity = FALSE;			// ignore DSR status
	dcb.fOutX = FALSE;						// no XON/XOFF flow control
	dcb.fInX = FALSE;						// idem
	dcb.fErrorChar = FALSE;					// no replacement
	dcb.fNull = FALSE;						// don't discard null chars
	dcb.fRtsControl = RTS_CONTROL_ENABLE;	// Provide power supply
	
	dcb.ByteSize = 8;						// 8 bits
	dcb.Parity = NOPARITY;					// no parity checking
	dcb.StopBits = ONESTOPBIT;				// 1 stop bit


	fSuccess = SetCommState(hCom, &dcb);
	if(!fSuccess)
	{
		printf("SetCommState failed with error %d.\n", GetLastError());
		return 3;
	}

		fSuccess=GetCommTimeouts(hCom,&cto);
		if(!fSuccess)
	{
		printf("SetCommTimeouts failed with error %d.\n", GetLastError());
		return 33;
	}
	
	cto.ReadIntervalTimeout = MAXDWORD;
    cto.ReadTotalTimeoutMultiplier = 0;
    cto.ReadTotalTimeoutConstant = 60000;
    cto.WriteTotalTimeoutMultiplier = 0;
    cto.WriteTotalTimeoutConstant = 60000;

    fSuccess=SetCommTimeouts(hCom,&cto);
	if(!fSuccess)
	{
		printf("SetCommTimeouts failed with error %d.\n", GetLastError());
		return 33;
	}

	printf("Serial port %s successfully reconfigured.\n", comPort);
	printf("The program will wait until a byte is received or a key pressed...\n");

	fprintf(txt, "RS232 receiver, version %s, ??/??/2000\n", VERSION);
	fprintf(txt, "Copyright (c) 1999-2001, Romain Lievin\n");
	fprintf(txt, "\n");

	fSuccess = PurgeComm(hCom, PURGE_TXCLEAR | PURGE_RXCLEAR);
	if(!fSuccess)
	{
		printf("PurgeComm failed with error %d.\n", GetLastError());
		return 33;
	}

	j=0;
	do
	{
		fSuccess = ReadFile(hCom, &ch, 1, &i, NULL);
		if(!fSuccess)
		{
			printf("ReadFile failed with error %d.\n", GetLastError());
			return 4;
		}

		if(fSuccess && i == 0)
		{
			printf("Timeout\n");
			fclose(txt);
			return 0;
		}

		if(fSuccess && i == 1)
		{
			if(t)
			{
				cto.ReadIntervalTimeout = MAXDWORD;
				cto.ReadTotalTimeoutMultiplier = 0;
				cto.ReadTotalTimeoutConstant = 5000;
				cto.WriteTotalTimeoutMultiplier = 0;
				cto.WriteTotalTimeoutConstant = 5000;
	
				fSuccess=SetCommTimeouts(hCom,&cto);
				if(!fSuccess)
				{
					printf("SetCommTimeouts failed with error %d.\n", GetLastError());
					return 33;
				}
				t=0;
			}

			buffer[j++]=ch & 0xff;
			fprintf(txt, "%02X ", ch & 0xff);
			//printf("%02X ", ch & 0xff);
			//if(!(nBytes++ % 256)) printf(".");

			if(j > 15)
			{
				fprintf(txt, "| ");
				for(k=0; k<16; k++)
				{
					c = buffer[k];
					if( (c < 32) || (c > 127) )
						fprintf(txt, " ");
					else
						fprintf(txt, "%c", c);
				}
				fprintf(txt, "\n");
				j=0;
			} 
		}
	}
	while(!_kbhit());
	printf("Done. Closing file.\n");

	fclose(txt);
	return 0;
}