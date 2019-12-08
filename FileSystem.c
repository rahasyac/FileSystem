// Name: Fnu Rahasya Chandan
// UTA Id: 100954962
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define DIRECTORY_ENTRIES 40
#define ENTRY_SIZE 128
#define FILENAME_SIZE 123
#define DIRECTORY_SIZE (DIRECTORY_ENTRIES * ENTRY_SIZE)

#define INITIAL_STATUS '\0' //empty position
#define RESUABLE_STATUS '1'
#define DELETED_STATUS '2' //file is deleted, can recorvered
#define USED_STATUS '3' //file is existing

#define BUFFER_SIZE 101

//file entry structure
typedef struct FILE_ENTRY{
	
	//file name
	char name[FILENAME_SIZE];

	char status;

	int size;

} FileEntry;

//directory structure
typedef struct DIRECTORY{
	
	FileEntry fileEntries[DIRECTORY_ENTRIES];
	
} Directory;


char* createOrOpenFile(char* filename, int* repositorySize, Directory *directory);

/* position to add file*/
int findEmptyPos(Directory *directory, int size);

/* position by file name*/
int findPosByFilename(Directory *directory, char* filename);

/* the size of file*/
int getFileSize(char* filename);

/* position of content*/
int getPositionOfContent(int pos, Directory *directory);

/*process put command*/
void processPut(char* localfile, char* repositoryName , int repositorySize, char* content, Directory *directory);

void saveRepository(char* repositoryName , int repositorySize, char* content, Directory *directory);

/*display file names and sizes*/
void processList(Directory *directory);

/*process get command*/
void processGet(char* repositoryfile, char* repositoryName , int repositorySize, char* content, Directory *directory);

/*process delete command*/
void processDelete(char* repositoryfile, char* repositoryName , int repositorySize, char* content, Directory *directory);

/*process recover command*/
void processRecover(char* repositoryfile, char* repositoryName , int repositorySize, char* content, Directory *directory);

/*process remove command*/
void processRemove(char* repositoryfile, char* repositoryName , int repositorySize, char* content, Directory *directory);


int processCommand(char buffer[], char* repositoryName , int repositorySize, char* content, Directory *directory);

void process(char* repositoryName, int repositorySize, char* content, Directory* directory);


int main(int argc, char *argv[]){

	int size; //repository size

	//directory memory
	Directory directory;

	//repository file content
	char* content;

	if (argc != 3)
	{
		printf("Usage: %s RepositoryName Size\n", argv[0]);
		return 0;
	}

	size = atoi(argv[2]);

	//validate size
	if (size < 1)
	{
		printf("Invalid size %d\n", size);
		return 0;
	}

	//file with fixed size
	content = createOrOpenFile(argv[1], &size, &directory);

	process(argv[1], size, content, &directory);

	free (content);

	return 0;
}


char* createOrOpenFile(char* filename, int* repositorySize, Directory *directory){
	
	FILE *out; //file for creating if not existing
	char ch = '\0'; //empty
	int i;
	char* content = NULL;

    FILE* fp = fopen(filename, "r"); 
    if (fp == NULL) { 

		// repository file content
		content = (char*)malloc((*repositorySize + DIRECTORY_SIZE) * sizeof(char));
       
		//open file for writing
		out = fopen(filename, "w");

		for (i = 0; i < *repositorySize + DIRECTORY_SIZE; i++)
		{
			content[i] = ch;			
		}

		if( !fwrite(content, (*repositorySize + DIRECTORY_SIZE), 1, out))
		{
			printf("Could not create %s file\n", filename);
			exit(EXIT_FAILURE);
		}

		//close file
		fclose(out);

		//read directory
		fp = fopen(filename, "r"); 

		//move to directory position
		fseek(fp, *repositorySize, SEEK_SET);

		//read directory
		fread((void*)directory, sizeof(Directory), 1, fp);    

		// closing the file 
		fclose(fp);

    } else{

		//move to end file
		fseek(fp, 0L, SEEK_END); 

		//size of file
		*repositorySize = ftell(fp); 

		*repositorySize = *repositorySize - DIRECTORY_SIZE;

		content = (char*)malloc((*repositorySize) * sizeof(char));

		//start position
		fseek(fp, 0, SEEK_SET);

		//read content
		fread((void*)content, (*repositorySize), 1, fp);    

		//read directory
		fread((void*)directory, sizeof(Directory), 1, fp);    

		//closing the file
		fclose(fp); 
	}	

	return content;
}

int findEmptyPos(Directory *directory, int size){
	int i;

	for (i = 0; i < DIRECTORY_ENTRIES; i++)
	{
		if (directory->fileEntries[i].status == INITIAL_STATUS || 
			(directory->fileEntries[i].status == RESUABLE_STATUS && directory->fileEntries[i].size == size))
		{
			return i;
		}
	}
	return -1;
}

int findPosByFilename(Directory *directory, char* filename){

	int i;

	for (i = 0; i < DIRECTORY_ENTRIES; i++)
	{
		if (strcmp(directory->fileEntries[i].name, filename) == 0)
		{
			return i;
		}
	}
	return -1;
}


int getFileSize(char* filename){

	int size;

	// opening the file in read mode 
    FILE* fp = fopen(filename, "r"); 
    if (fp == NULL) { 

		size = -1;

	}else{

		fseek(fp, 0L, SEEK_END); 
  
		//size of the file
		size = ftell(fp); 

		//closing the file
		fclose(fp);
	}

	return size;
}


int getPositionOfContent(int pos, Directory *directory){

	int contentPos = 0;
    
	int i;

	for (i = 0; i < pos; i++)
	{
		contentPos += directory->fileEntries[i].size;
	}

	return contentPos;
}

void processPut(char* localfile, char* repositoryName , int repositorySize, char* content, Directory *directory){

	int pos;
	int fileSize = getFileSize(localfile);
	char ch;
	int contentPos;
	int i;

	// opening the file in read mode 
    FILE* fp = fopen(localfile, "r"); 
    if (fp == NULL) { 

		printf("%s file Not Found!\n", localfile);

	}else{

		pos = findEmptyPos(directory, fileSize);

		if (pos == -1)
		{
			printf("There is not enough space to put %s file\n", localfile);
		}else{

			contentPos = getPositionOfContent(pos, directory);

			if (contentPos + fileSize > repositorySize)
			{
				printf("There is not enough space to put %s file\n", localfile);
			}else{
				strcpy(directory->fileEntries[pos].name, localfile);
				directory->fileEntries[pos].size = fileSize;
				directory->fileEntries[pos].status = USED_STATUS;

				i = contentPos;
				
				ch = fgetc(fp); 
				while (ch != EOF) 
				{ 
					content[i++] = ch;
					ch = fgetc(fp); 
				} 

				printf("The %s file has been put into the repository file\n", localfile);
			}
		}

		fclose(fp);		
	}
}


void saveRepository(char* repositoryName , int repositorySize, char* content, Directory *directory){
	
	// opening the file in write mode 
    FILE* out = fopen(repositoryName, "w"); 

    if (out == NULL) { 

		printf("Could not save %s file\n", repositoryName);

	}else{


		if( !fwrite(content, repositorySize, 1, out))
		{
			printf("Could not write %s file\n", repositoryName);
			exit(EXIT_FAILURE);
		}

		if( !fwrite((void*)directory, DIRECTORY_SIZE, 1, out))
		{
			printf("Could not write %s file\n", repositoryName);
			exit(EXIT_FAILURE);
		}

		//close file
		fclose(out);

		printf("%s has been saved successfully\n", repositoryName);
	}	
}


void processList(Directory *directory){

	int i;
	int numFiles = 0;
    
	for (i = 0; i < DIRECTORY_ENTRIES; i++)
	{
		if (directory->fileEntries[i].status == USED_STATUS)
		{
			printf("%s %d\n", directory->fileEntries[i].name, directory->fileEntries[i].size);
			numFiles ++;
		}
	}

	printf("Number of files: %d\n", numFiles);
}

void processGet(char* repositoryfile, char* repositoryName , int repositorySize, char* content, Directory *directory){

	char* fileContent;

	int pos = findPosByFilename(directory, repositoryfile);

	if (pos == -1)
	{
		printf("%s file not found!\n", repositoryfile);
	}else if (directory->fileEntries[pos].status == DELETED_STATUS)
	{
		printf("%s file have been deleted (recoverable)\n", repositoryfile);
	}else if (directory->fileEntries[pos].status == USED_STATUS)
	{

		// opening the file in write mode 
		FILE* out = fopen(repositoryfile, "w"); 

		if (out == NULL) { 

			printf("Could not create %s file\n", repositoryfile);

		}else{

			fileContent = (char*)malloc(directory->fileEntries[pos].size * sizeof(char));

			strncpy(fileContent, content + getPositionOfContent(pos, directory), 
				directory->fileEntries[pos].size);

			if( !fwrite(fileContent, directory->fileEntries[pos].size, 1, out))
			{
				printf("Could not write %s file\n", repositoryName);
				exit(EXIT_FAILURE);
			}

			fclose(out);

			free(fileContent);

			printf("The %s file has been created (content from the repository file)\n", repositoryfile);
		}
	}else{
		printf("%s file not found!\n", repositoryfile);
	}
}

void processDelete(char* repositoryfile, char* repositoryName , int repositorySize, char* content, Directory *directory){

	int pos = findPosByFilename(directory, repositoryfile);

	if (pos == -1)
	{
		printf("%s file not found!\n", repositoryfile);
	}else if (directory->fileEntries[pos].status == USED_STATUS)
	{
		directory->fileEntries[pos].status = DELETED_STATUS;
		printf("%s file have been deleted (recoverable)\n", repositoryfile);
	}else if (directory->fileEntries[pos].status == DELETED_STATUS)
	{
		printf("%s file have been deleted already\n", repositoryfile);
	}else{
		printf("%s file not found!\n", repositoryfile);
	}
}

void processRecover(char* repositoryfile, char* repositoryName , int repositorySize, char* content, Directory *directory){

	int pos = findPosByFilename(directory, repositoryfile);

	if (pos == -1)
	{
		printf("%s file not found!\n", repositoryfile);
	}else if (directory->fileEntries[pos].status == DELETED_STATUS)
	{
		directory->fileEntries[pos].status = USED_STATUS;
		printf("%s file have been recovered\n", repositoryfile);
	}else if (directory->fileEntries[pos].status == USED_STATUS)
	{
		printf("%s file existing or have been recovered already\n", repositoryfile);
	}else{
		printf("%s file not found!\n", repositoryfile);
	}
}

void processRemove(char* repositoryfile, char* repositoryName , int repositorySize, char* content, Directory *directory){

	int pos = findPosByFilename(directory, repositoryfile);

	if (pos == -1)
	{
		printf("%s file not found!\n", repositoryfile);
	}else if (directory->fileEntries[pos].status == DELETED_STATUS ||
		directory->fileEntries[pos].status == USED_STATUS)
	{
		directory->fileEntries[pos].status = RESUABLE_STATUS;
		printf("%s file have been removed\n", repositoryfile);
	}else if (directory->fileEntries[pos].status == RESUABLE_STATUS)
	{
		printf("%s file have been removed already\n", repositoryfile);
	}else{
		printf("%s file not found!\n", repositoryfile);
	}
}


int processCommand(char buffer[], char* repositoryName , int repositorySize, char* content, Directory *directory){

	char* command = strtok(buffer, " ");
	char* filename = strtok(NULL, " ");

	if (strcmp(command, "put") == 0)
	{
		processPut(filename, repositoryName, repositorySize, content, directory);
	}else if (strcmp(command, "get") == 0)
	{
		processGet(filename, repositoryName, repositorySize, content, directory);
	}else if (strcmp(command, "save") == 0)
	{
		saveRepository(repositoryName, repositorySize, content, directory);
	}else if (strcmp(command, "list") == 0)
	{
		processList(directory);
	}else if (strcmp(command, "delete") == 0)
	{
		processDelete(filename, repositoryName, repositorySize, content, directory);
	}else if (strcmp(command, "recover") == 0)
	{
		processRecover(filename, repositoryName, repositorySize, content, directory);
	}else if (strcmp(command, "remove") == 0)
	{
		processRemove(filename, repositoryName, repositorySize, content, directory);
	}else if (strcmp(command, "quit") == 0)
	{
		return 0;	
	}
	return 1;
}

void process(char* repositoryName, int repositorySize, char* content, Directory* directory){

	char buffer[BUFFER_SIZE];
	int status = 1;

	while (status == 1)
	{
		fgets(buffer, BUFFER_SIZE, stdin);

		buffer[strlen(buffer) - 1] = '\0';//remove character

		status = processCommand(buffer, repositoryName, repositorySize, content, directory);

		printf("\n");
	}
}
