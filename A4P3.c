//Author Peter Adamson
//This program expects the following syntax when being run: ./a.out n file.txt < sample
//where n is the number of frames, file.txt is the memory file to be read, and sample is the list of logical addresses

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

//global variable declarations
int pageTable[256];
int indicatorTable[256];
int modified[256];
int pageFaults;
int dirtyPages;
int freeFrame;
int full;
int numOfFrames;

//function declarations
int findFreeFrame();

int main(int argc, char **argv)
{
	//variable set up
	pageFaults = 0;
	dirtyPages = 0;
	freeFrame = 0;
	full = 0;
	numOfFrames = atoi(argv[1]);
	int frames[numOfFrames * 256];
	long int logAddr;
	int i;
	int j;
	int k;
	int val;
	char str[100];
	char toSplit[10][10];
	char *character;
	char **tokens;
	char *toWrite;
	char toWriteConv;
	character = (char*)malloc(sizeof(char*));

	//open the file
	FILE *file = fopen(argv[2], "r+");

	//populate page table and indicator table
	for(i = 0; i < 256; i++)
	{
		pageTable[i] = 0;
		indicatorTable[i] = 0;
		modified[i] = 0;
	}

	//loop as long as there are still logical addresses to be read and the format is correct
	while(fgets(str, sizeof(str), stdin) != NULL)
	{
		//loop through the string and seperate based on whitespace
		j = 0;	//line value
		k = 0;	//length of word
		for(i = 0; i <= (strlen(str)); i++)
		{
			if(str[i] == '\0' || str[i] == ' ')	//we have the end or whitespace
			{
				toSplit[j][k] = '\0';	//null terminate string
				j = j + 1;	//start a new line (new word)
				k = 0;		//reset word length to 0
			}
			else	//still in a word
			{
				toSplit[j][k] = str[i];	//set the value on line to the character encountered
				k = k + 1;	//increase word length by 1
			}
		}
		char *readWrite = toSplit[0];
		logAddr = atoi(toSplit[1]);

		if(strcmp(readWrite, "W") == 0)	//we are writing
		{
			toWrite = toSplit[2];
			toWriteConv = toWrite[0];
		}

		fseek(file, logAddr, SEEK_SET);	//set the file stream to the appropriate location

		if(strcmp(readWrite, "R") == 0)	//we are reading
		{
			val = fgetc(file);		//get the character at the file stream location
			val = (char) val;		//cast the character to char
		}
		else	//we are writing
		{
			val = toWriteConv;
		}

		//get page number and offset of logical address
		int pageNum = logAddr / 256;	
		int offset = logAddr % 256;
		if(indicatorTable[pageNum] == 1)	//we have a valid page table entry
		{
			int physAddr = pageTable[pageNum] * 256 + offset;
			if(strcmp(readWrite, "W") == 0)	//we are writing
			{
				frames[physAddr] = val;
				modified[pageNum] == 1;
				fseek(file,logAddr, SEEK_SET);
				char *ptr = &toWriteConv;
				fwrite(ptr, 1, sizeof(ptr), file);
			}
			printf("%d->%d->%c\n",logAddr,physAddr,frames[physAddr]);
		}
		else					//the page is not loaded
		{	
			pageFaults = pageFaults + 1;
			int frameNum = findFreeFrame();
			indicatorTable[pageNum] = 1;
			pageTable[pageNum] = frameNum;
			int physAddr = frameNum * 256 + offset;
			int ch[255];

			//load the page
			for(i = 0; i < 256; i++)
			{
				fseek(file,pageNum*256 + i, SEEK_SET);
				int v = fgetc(file);
				v = (char) v;
				ch[i] = v;
			}

			//put the page into physical memory
			int count = 0;
			for(i = frameNum * 256; i < frameNum * 256 + 256; i++)
			{
				frames[i] = ch[count];
				count = count + 1;
			}
			if(strcmp(readWrite,"W") == 0)	//we are writing
			{
				frames[physAddr] = val;
				modified[pageNum] = 1;
				fseek(file,logAddr, SEEK_SET);
				char *ptr = &toWriteConv;
				fwrite(ptr, 1, sizeof(ptr), file);
			}
			printf("%d->%d->%c\n",logAddr,physAddr,frames[frameNum * 256 + offset]);
		}	
	}

	//close the file
	fclose(file);

	printf("total page faults: %d\n",pageFaults);

	//loop through remaining frames
	for(i = 0; i < 256; i++)
	{
		if(modified[i] == 1)	//page is dirty
		{
			dirtyPages = dirtyPages + 1;
		}
	}
	printf("number of dirty pages swapped out: %d\n",dirtyPages);
}

int findFreeFrame()
{
	int found;
	if(full == 0)	//still free frames
	{
		found = freeFrame;
		freeFrame = freeFrame + 1;
		if(freeFrame >= numOfFrames)	//loop freeframes
		{
			full = 1;
			freeFrame = 0;
		}
		return found;
	}
	else		//no free frames
	{
		int j;
		int index;
		for(j = 0; j < 256; j++)
		{
			if(pageTable[j] == freeFrame)	//we have found the entry to remove
			{
				pageTable[j] = 0;
				indicatorTable[j] = 0;
				if(modified[j] == 1)	//page is dirty
				{
					dirtyPages = dirtyPages + 1;
					modified[j] = 0;
				}
				break;
			}
		}
		found = freeFrame;
		freeFrame = freeFrame + 1;
		if(freeFrame >= numOfFrames)	//loop freeframes
		{
			freeFrame = 0;
		}
		return found;
	}
}
