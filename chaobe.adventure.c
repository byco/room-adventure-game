// DESCRIPTION: Along with chaobe.buildrooms.c, this file creates a text-based Adventure game. The user
// can navigate through a randomly-generated set of rooms using the interface provided and can request
// time information (given using threads).

// libraries used
#include <pthread.h>
#include <time.h>
#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

// for time thread - data will go into the timeFile generated
pthread_mutex_t time_mutex;
char* timeFile = "currentTime.txt";

// this function gets the current time and writes it to currentTime.txt
void* createTimeFile()
{
	FILE* tf;
	char currentTimeStr[256];
	time_t currentTime;
	struct tm* timeData;
	memset(currentTimeStr, '\0', sizeof(currentTimeStr));

	time(&currentTime);
	// localtime() takes raw time data and converts it to tm structure info
	timeData = localtime(&currentTime);
	// strftime() formats time as string so we can print it
	// note that %a = abbreviated weekday name, %A = full weekday name (CASE MATTERS!)
	strftime(currentTimeStr, 256, "%I:%M%P, %A, %B %d, %Y", timeData);

	tf = fopen(timeFile, "w");

	fprintf(tf, "%s\n", currentTimeStr);
	fclose(tf);
	return NULL;
	
}

// this function locks the time thread, creates a new time file used later for reading, unlocks the thread and joins the new thread
int timeThread()
{
	pthread_t tt;
	pthread_mutex_lock(&time_mutex);

	// create new thread and make sure new time file is created
	// note that I am passing createTimeFile, NOT createTimeFile() because function needs type void*(*)
	if( pthread_create(&tt, NULL, createTimeFile, NULL) != 0)
	{
		// error new thread is not created
		return 1;
	}

	// unlocked thread mutex
	pthread_mutex_unlock(&time_mutex);
	pthread_join(tt, NULL);
	return 0;
}

// this function opens the time file and prints its contents to the terminal
void readTime()
{
	FILE* tf;
	char buffer1[256];
	
	// clear buffer before reading next char
	memset(buffer1, '\0', sizeof(buffer1));

	tf = fopen(timeFile, "r");

	// if the file is found and readable
	if(tf != NULL)
	{
		// read to end of file
		while( fgets(buffer1, 256, tf) != NULL )
		{
			printf("%s\n", buffer1);
		}
	}

	fclose(tf);
}

// need to reuse and adapt some structures and functions from buildrooms.c to build rooms
// most rooms are assigned a mid-room status, one is a start-room, another one is an end-room
// rooms have already been assigned in chaobe.buildrooms
enum ROOM_TYPE
{
    START_ROOM,
    MID_ROOM,
    END_ROOM
};

// struct that holds room data
struct ROOM
{
    int connectionsNum;
    struct ROOM* connections[6];
    enum ROOM_TYPE roomType;
    char name[25];
    char fileName[25];
};

// list of rooms that will be used in the game
struct ROOM pickedRooms[7];

// using data read from room files, initilize the rooms
void initRooms()
{
	int i;
	for (i = 0; i < 7; i++)
	{
		// make sure room names don't have garbage in them
		memset(pickedRooms[i].name, '\0', sizeof(pickedRooms[i].name));
		pickedRooms[i].connectionsNum = 0;
		
		// by default, connections should be NULL (add them later)
		int j;
		for (j = 0; j < 6; j++)
		{
			pickedRooms[i].connections[j] = NULL;
		}
	}
}

// this is a helper function that converts a string representing a room name to its current room position within pickedRooms
int nameToRoom(char* roomName)
{
	int i;
	// search for roomName within pickedRooms
	// if found, return the room position
	for(i = 0; i < 7; i++)
	{
		if(strcmp(pickedRooms[i].name,roomName)==0)
		{
			return i;
		}
	}
	// if the room name is not found
	return -1;
}

// add connections between rooms based on room file data
void addConnection(int r1, int r2)
{
	int c1 = pickedRooms[r1].connectionsNum;

	pickedRooms[r1].connections[c1] = &pickedRooms[r2];
	pickedRooms[r1].connectionsNum++;
}

// adapted from 2.4 Manipulating Directories
// this function reads from the room files and begins recreating the rooms and their graph
void getData()
{
	// to be used later for collecting time data
	time_t lastTime;
	time_t newestTime = 0; 

	char prefix[25] = "chaobe.rooms.";
	char newestDir[256];
	memset(newestDir, '\0', sizeof(newestDir));

	DIR* currentDir;
	struct dirent *dirFiles;
	struct stat dirInfo;

	// buffers will be used to check data found in room files
	char buffer1[256];
        char buffer2[256];
	struct stat* buffer3;
	buffer3 = malloc(sizeof(struct stat));
	dirFiles = malloc(sizeof(struct dirent));

	// open current directory
	currentDir = opendir(".");

	if(currentDir > 0)
	{
		// read all files within directory
		while((dirFiles = readdir(currentDir)) != NULL)
		{
			if(strstr(dirFiles->d_name, prefix) != NULL)
			{
				// get the last modified date stamp of file
				stat(dirFiles->d_name, buffer3);
				lastTime = buffer3->st_mtime;
			
				// if this folder is newer than the last folder checked
				// replace name and redirect to this folder as the newest
				if(lastTime > newestTime)
				{
					newestTime = lastTime;
					memset(newestDir, '\0', sizeof(newestDir));
					strcpy(newestDir, dirFiles->d_name);
				}

			}
		}


	}
	closedir(currentDir);

	// now get file names to fill room list

	initRooms();

	FILE* roomFile;
	int fileNum = 0;
	struct dirent *nsd;
	DIR* roomDir;

	// open the newest directory
	if((roomDir = opendir(newestDir)) != NULL)
	{
		while((nsd = readdir(roomDir)) != NULL)
		{
			// read the file names and copy them into the room list
			if(strlen(nsd->d_name) > 2)
			{
				strcpy(pickedRooms[fileNum].fileName, nsd->d_name);
				fileNum++;
			}
		}
	}

	// remember to change directories to access room files!
	
	chdir(newestDir);

	// loop through the room files to read them individually
	int i;
	for(i = 0; i < 7; i++)
	{
		// open each file and make sure the buffer is clear
		roomFile = fopen(pickedRooms[i].fileName, "r");
		memset(buffer1, '\0', sizeof(buffer1));

		// fgets lines read from file
		while( fgets(buffer1, sizeof(buffer1), roomFile) != NULL )
		{
			// if the file is formatted correctly, there should be 3 strings on each line
			// assign these strings to s1, s2, s3 for parsing
			char s1[25], s2[25], s3[25];
			sscanf(buffer1, "%s %s %s", s1, s2, s3);
		
			// if the second string in the line is NAME, assign this string to room name
			if(strcmp(s2,"NAME:") == 0)
			{
				strcpy(pickedRooms[i].name,s3);
			}

			// if the third string in the line is MID_ROOM, label the room as a MID_ROOM roomType
			if(strcmp(s3,"MID_ROOM") == 0)
			{
				pickedRooms[i].roomType = MID_ROOM; 

			}

			// if the third string in the line is START_ROOM, label the room as a START_ROOM roomType
			if(strcmp(s3,"START_ROOM") == 0)
			{
				pickedRooms[i].roomType = START_ROOM;
			}

			// if the third string in the line is END_ROOM, label the room as an END_ROOM roomType
			if(strcmp(s3,"END_ROOM") == 0)
			{
				pickedRooms[i].roomType = END_ROOM;
			}

		}
	
	}

	// now that we have named all rooms, we can go through the file again to collect connections data
        int j;
        for(j = 0; j < 7; j++)
        {
		roomFile = fopen(pickedRooms[j].fileName, "r");
		memset(buffer2, '\0', sizeof(buffer2));
		// be sure to rewind to start at the beginning of the file, since we've already gone through the file once
		rewind(roomFile);
	
		// reread lines in file
		while( fgets(buffer2, sizeof(buffer2), roomFile) != NULL )
		{
			// if the file is formatted correctly, it should have 3 strings
			// check that the first string is CONNECTION
			// if it is CONNECTION, then assign the third string in the line as a connection of the room's
			char s1[25], s2[25], s3[25];
			sscanf(buffer2, "%s %s %s", s1, s2, s3);
		
			if(strcmp(s1,"CONNECTION")==0)
			{
				addConnection(j,nameToRoom(s3));
			}
		}
	}
	// remember to go back to main directory! (in case currentTime.txt needs to be outputted)
	chdir("..");
}

// this helper function returns the position of the start room of this game
int findStartRoom()
{
	// go through each room and see if the room's roomType is START_ROOM
	// if so, return the position
	int i;
	for(i = 0; i < 7; i++)
	{
		if(pickedRooms[i].roomType == START_ROOM)
		{
			return i;
		}
	}
}

// this function provides the interface with which the player interacts
void startGame()
{
	// keep track of room player is in
	struct ROOM currentRoom;
	int currentRoomPos;

	// keep track of steps
	// stepRec contains the path of the user
	int stepRec[100];
	int steps = 0;
	
	// buffer for user input
	char buffer1[256];

	// counter to check if game is finished yet
	int endReached = 0;
	// find start room
	stepRec[steps] = findStartRoom();
	
	// keep looping through with the menu until the player finds the END_ROOM
	do
	{
		currentRoomPos = stepRec[steps];
		currentRoom = pickedRooms[currentRoomPos];

		// text for game begins here - print to terminal
		printf("CURRENT LOCATION: %s\n", currentRoom.name);

		int i;
		// loop to print connections - variable number of connections
		printf("POSSIBLE CONNECTIONS:");
		for(i = 0; i < currentRoom.connectionsNum - 1; i++)
		{
			printf(" %s,", currentRoom.connections[i]->name);
		}

		// note formatting - no comma following the last connection name
		printf(" %s.\n", currentRoom.connections[i]->name);

		// now ask user for next room
		memset(buffer1, '\0', sizeof(buffer1));
		printf("WHERE TO? >");
		scanf("%255s", buffer1);
		printf("\n");

		// look for room within list of room connections
		// update variable if room is found
		int roomFound = 0;
		int j;
		for(j = 0; j < currentRoom.connectionsNum; j++)
		{
			// if room name matches with room connection
			if(strcmp(buffer1, currentRoom.connections[j]->name) == 0)
			{
				// update steps and current path list and find data for next room
				roomFound++;
				steps++;
				stepRec[steps] = nameToRoom(buffer1);

				currentRoomPos = stepRec[steps];
				currentRoom = pickedRooms[currentRoomPos];

				// if the new room happens to be the END_ROOM
				if(currentRoom.roomType == END_ROOM)
				{
					// print ending message for game
					printf("YOU HAVE FOUND THE END ROOM. CONGRATULATIONS!\n");
					printf("YOU TOOK %i STEPS. YOUR PATH TO VICTORY WAS:\n", steps);
					
					// print path (list of rooms) that the player visited
					int k;
					for(k = 1; k < steps + 1; k++)
					{
						printf("%s\n", pickedRooms[stepRec[k]].name);
					}
					// end the loop now the game is won
					endReached++;
				}
			}
		}

		// if user types in time instead of a room name
		if(strcmp(buffer1, "time") == 0 && roomFound == 0)
		{
			// check that timeThread is valid and time file has been created
			if( timeThread() == 0)
			{
				// now read the time file and output the time to terminal
				readTime();

			}
		}

		// if user input is not time or a valid room name
		// print an error message and run menu again
		else if(roomFound == 0)
		{
			printf("HUH? I DON’T UNDERSTAND THAT ROOM. TRY AGAIN.\n\n");
		}
		// for TESTING
		// endReached = 1;
	} while(endReached == 0);

}

int main()
{
	// first read the data from the last modified chaobe.rooms directory, then initialize the rooms and rooms graph
	getData();
	// begin game menu. will run until game is won.	
	startGame();

	// testing
	/*int i;
	for (i = 0; i < 7; i++)
	{
		printf("\nfile name %i: %s\n", i+1, pickedRooms[i].fileName);
		printf("room name: %s\n", pickedRooms[i].name);
		printf("connections: %i\n", pickedRooms[i].connectionsNum);
		printf("room type: %i\n", pickedRooms[i].roomType);
		
		int j;
		for (j = 0; j < pickedRooms[i].connectionsNum; j++)
		{
			printf("%i: %s\n",j+1,pickedRooms[i].connections[j]->name);
		}
	}*/

	return 0;
}
