#include <stdio.h>
#include <dlfcn.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

unsigned int char2hex(unsigned char* a_data, unsigned int* a_type)
{
	unsigned int dword=0;
	unsigned int power=0;

	while(power < 8)
	{
		switch(a_data[power])
		{
			case '0': dword|=0 * (1 << (4*(7-power))); break;
			case '1': dword|=1 * (1 << (4*(7-power))); break;
			case '2': dword|=2 * (1 << (4*(7-power))); break;
			case '3': dword|=3 * (1 << (4*(7-power))); break;
			case '4': dword|=4 * (1 << (4*(7-power))); break;
			case '5': dword|=5 * (1 << (4*(7-power))); break;
			case '6': dword|=6 * (1 << (4*(7-power))); break;
			case '7': dword|=7 * (1 << (4*(7-power))); break;
			case '8': dword|=8 * (1 << (4*(7-power))); break;
			case '9': dword|=9 * (1 << (4*(7-power))); break;
			case 'a':case 'A': dword|=0xA * (1 << (4*(7-power))); break;
			case 'b':case 'B': dword|=0xB * (1 << (4*(7-power))); break;
			case 'c':case 'C': dword|=0xC * (1 << (4*(7-power))); break;
			case 'd':case 'D': dword|=0xD * (1 << (4*(7-power))); break;
			case 'e':case 'E': dword|=0xE * (1 << (4*(7-power))); break;
			case 'f':case 'F': dword|=0xF * (1 << (4*(7-power))); break;
			default: dword>>=4*(8-power); *a_type=power; return dword;
		}

		++power;
	}

	*a_type=8;
	return dword;
}

void searchText(const char* needle, int needleLen, unsigned int startAddr, unsigned int endAddr)
{
	int searchResultCounter=0;
	for(unsigned int cAddress=startAddr;cAddress<endAddr;cAddress+=sizeof(unsigned int))
	{
		printf("\33[2K0x%08X\r", cAddress);
		int charCounter=0;
		while(charCounter < needleLen)//arbitrary max string length
		{
			if(cAddress+charCounter < endAddr)
			{
				//Get the current letter in memory
				unsigned char tempLetter=*((unsigned char*)(startAddr+charCounter));
				if((tempLetter >= 0x61) && (tempLetter <= 0x7A))
					tempLetter-=0x20;

				//Check against the haystack
				if(tempLetter == (unsigned char)needle[charCounter])
				{
					//
					++charCounter;
				}
				else
					break;
			}
		}

		if(charCounter == needleLen)
		{
			printf("%d: 0x%08X\n", searchResultCounter, startAddr);
			++searchResultCounter;
		}

		if(searchResultCounter == 200)
			break;
	}
	printf("\n");
}

void printBrowser(unsigned int startAddr, unsigned int endAddr)
{
	unsigned char strContent[11];
	strContent[10]=0;
	int drawChars=8;
	int searchResultCounter=0;
	int byteCounter=0;
	for(unsigned int cAddress=startAddr;cAddress<endAddr;cAddress+=sizeof(unsigned char))
	{
		//beginning of line
		if(byteCounter == 0)
			printf("0x%08X\t", cAddress);

		//content
		strContent[byteCounter]=*((unsigned char*)(cAddress));
		printf("%02X", strContent[byteCounter]);

		//for the print later
		if((strContent[byteCounter] <= 0x20) || (strContent[byteCounter] == 0xFF))
			strContent[byteCounter]='.';

		//end of line
		if(byteCounter == drawChars)
		{
			//reset the counter
			byteCounter=0;

			//print the ascii form
			printf("\t%s\n", strContent);
		}
		else
			++byteCounter;
	}

	//end of browser
	printf("\n");
}

void inject()
{
	printf("Injected...\n");

	//file info
	char fname[16]="/proc/self/maps\0";
	FILE* fd=fopen(fname, "r");
	printf("[FILE] %c%s\n", (fd != NULL)? '+':'-', fname);

	//Mem Range
	unsigned int startAddr=0x00000000;
	unsigned int endAddr=0x00000000;

	//read the file
	if(fd != NULL)
	{
		//get the file size
		fseek(fd, 0, SEEK_END);
		size_t fSize=ftell(fd);
		fseek(fd, 0, SEEK_SET);

		//Allocate a buffer
		int rowCounter=0;
		int MAX_LINE_SIZE=256;
		int linesRead=0;//Are the lines read, not how many lines read
		char* buffer=(char*)malloc(MAX_LINE_SIZE*sizeof(char));

		do
		{
			bzero(buffer, MAX_LINE_SIZE);

			//get the current line
			char readBuffer[MAX_LINE_SIZE];
			if(fgets(readBuffer, sizeof(readBuffer), fd) != 0)
				linesRead=sscanf(readBuffer, "%[^\n]s", buffer);
			else
				linesRead=0;

			//EOF or error
			if(linesRead <= 0)
				break;

			//Print each line
			//printf("buffer: %s\n", buffer);

			//Starting Address Init
			int addrSize=8;
			unsigned int startType=0;
			unsigned char* strStartAddr=(unsigned char*)malloc(sizeof(unsigned char)*(addrSize+1));//terminating null char
			bzero(strStartAddr, (addrSize+1));

			//Get the Starting Address
			memcpy(strStartAddr, buffer, addrSize);
			startAddr=char2hex(strStartAddr, &startType);

			//Ending Address Init
			unsigned int endType=0;
			unsigned char* strEndAddr=(unsigned char*)malloc(sizeof(unsigned char)*(addrSize+1));//terminating null char
			bzero(strEndAddr, (addrSize+1));

			//Get the Ending Address
			memcpy(strEndAddr, &buffer[addrSize+1], addrSize);
			endAddr=char2hex(strEndAddr, &endType);

			++rowCounter;

		} while((linesRead > 0) && (ftell(fd) < fSize));

		//EOF
		free(buffer);
		fclose(fd);
	}
	else
		printf("Proc map not found\n");

	//Print the mem range
	printf("[MEM] 0x%08X - 0x%08X\n", startAddr, endAddr);

	/*int needleLen=6;
	char needle[6]={'T', 'I', 'C', 'K', 'E', 'R'};
	searchText(needle, needleLen, startAddr, endAddr);*/
	printBrowser(startAddr, endAddr);

	//done
	printf("Exiting\n");
}

__attribute__((constructor))
void loadMsg()
{
	inject();
}
