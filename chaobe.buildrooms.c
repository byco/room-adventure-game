// DESCRIPTION: Combined with chaobe.adventure.c, this program runs a text-based adventure game.
// The user can use the interface in adventure.c to run the game. This file builds room structures
// with which the user can navigate within the Adventure game, in addition to saving descriptions of
// the rooms and how they are connected to one another.

// necessary libraries used
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

// one room is assigned to be the start room (where the player begins the game), one room is assigned
// to be the end room (where the player ends the game) and all others are mid rooms
enum ROOM_TYPE
{
    START_ROOM,
    MID_ROOM,
    END_ROOM
};

// hardcoded room names
char* ROOM_NAME[10] =
        {
                "Hills",
                "Beach",
                "Lake",
                "Field",
                "Desert",
                "Jungle",
                "Ocean",
                "Marsh",
                "Arctic",
                "Forest"
        };

// hardcoded room file names
char* ROOM_FILES[10] =
        {
                "Hills_Room",
                "Beach_Room",
                "Lake_Room",
                "Field_Room",
                "Desert_Room",
                "Jungle_Room",
                "Ocean_Room",
                "Marsh_Room",
                "Arctic_Room",
                "Forest_Room"
        };        

// this struct collects necessary data about the rooms (randomized during each time chaobe.buildrooms is run)
struct ROOM
{
    int connectionsNum;
    struct ROOM* connections[6];
    enum ROOM_TYPE roomType;
    char name[25];
    char fileName[25];
};

// global constants needed for each run of chaobe.buildrooms
// roomList is the data for all 10 default rooms; pickedRooms is the shorter list of rooms the game will use
// pickedRooms is randomly generated each time chaobe.buildrooms is run
struct ROOM roomList[10];
char folder[256];
struct ROOM pickedRooms[7];

// pick a randomRoom - use this only within pickedRooms (roomList is a longer list)
int randomRoom()
{
	return rand() % 7;
}

// initialize roomList elements and pick 7 out of 10 rooms to initialize for pickedRooms
void createRoomList()
{
    // by default, set number of connections to 0, copy hardcoded names and file names, and assign rooms as midrooms
    int k;
    for(k = 0; k < 10; k++)
    {
        roomList[k].connectionsNum = 0;
        strcpy(roomList[k].name, ROOM_NAME[k]);
        strcpy(roomList[k].fileName, ROOM_FILES[k]);
        roomList[k].roomType = MID_ROOM;
    }

    // now pick 7 random rooms and make sure there are no duplicates
    // rerun loop if there are duplicates

    int i;
    for(i = 0; i < 7; i++)
    {
        int dup = 0;
        do
        {
            dup = 0;
            pickedRooms[i] = roomList[rand() % 10];
            int j;

            for(j = 0; j < i; j++)
            {
                if(strcmp(pickedRooms[i].name,pickedRooms[j].name) == 0)
                {
                    dup++;
                }
            }
        } while(dup != 0);
    }

    // testing
    /*printf("pickedrooms:\n");

    int j;
    for(j = 0; j < 7; j++)
    {
        printf("%s\n",pickedRooms[j].name);
    }*/

    // random rooms will be assigned to be start and end rooms

    int rand1 = randomRoom();
    int rand2 = rand1;

    while(rand2 == rand1)
    {
        rand2 = randomRoom();
    }

    pickedRooms[rand1].roomType = START_ROOM;
    pickedRooms[rand2].roomType = END_ROOM;

}

// helper function to check if two rooms are already connected
int isConnected(int r1, int r2)
{
    int c1 = pickedRooms[r1].connectionsNum;

    // check that r2 isn't the same room as r1
    if(strcmp(pickedRooms[r1].name, pickedRooms[r2].name)==0)
    {
        return 1;
    }

    // go through to see that r2 isn't already in r1's connections
    int k;
    for(k = 0; k < c1; k++)
    {
        if(strcmp(pickedRooms[r1].connections[k]->name, pickedRooms[r2].name) == 0)
        {
            return 1;
        }
    }

    // rooms not connected - return 0
    return 0;

}

// connect specified room (passed as parameter) to a random room
void connectRooms(int room)
{
    int r1 = room;

    int r2 = randomRoom();
    int c2 = pickedRooms[r2].connectionsNum;

    // connect rooms if rooms aren't already connected
    if(isConnected(r1,r2) == 0 && pickedRooms[r1].connectionsNum < 6 && pickedRooms[r2].connectionsNum < 6)
    {
        pickedRooms[r1].connections[pickedRooms[r1].connectionsNum] = &pickedRooms[r2];
        pickedRooms[r2].connections[c2] = &pickedRooms[r1];
        // remember to update the number of connections or else program will overwrite
	pickedRooms[r1].connectionsNum++;
        pickedRooms[r2].connectionsNum++;
    }
}

// check if more connections between rooms are needed or not
int isGraphFull()
{
    // go through room list and check that each room has at least 3 connections
    // if room with too few connections found, return 0 (FALSE)
    int z;
    for(z = 0; z < 7; z++)
    {
        if(pickedRooms[z].connectionsNum < 3)
        {
            return 0;
        }
    }
        return 1;
}

// this function will create the room graph (set how rooms are connected to one another)
void roomGraph()
{
    createRoomList();

    // add more random connections if there are any rooms with fewer than 3 connections
    while(isGraphFull()==0)
    {
        int z;
        for (z = 0; z < 7; z++)
        {
            connectRooms(z);
        }
    }
}

// use graph data to write room info into room file
void createFiles(char* folderName)
{
    FILE* roomFile;
    int j;
    
    // go through all rooms' data so each room gets its own file
    for(j = 0; j < 7; j++)
    {
	// name the file based on hardcoded file name
        char fullName[256];
        sprintf(fullName, "%s/%s", folderName, pickedRooms[j].fileName);

	// now open file to write room name, connections, room type
        roomFile = fopen(fullName, "w");

        fprintf(roomFile, "ROOM NAME: %s\n",pickedRooms[j].name);

	// loop to write connections data
        int k;
        for(k = 0; k < pickedRooms[j].connectionsNum; k++)
        {
            fprintf(roomFile, "CONNECTION %i: %s\n", k + 1, pickedRooms[j].connections[k]->name);
        }

        // workaround to print room type (which happens to be not a string)

        if(pickedRooms[j].roomType == START_ROOM)
        {
            fprintf(roomFile, "ROOM TYPE: %s\n", "START_ROOM");
        }

        else if(pickedRooms[j].roomType == MID_ROOM)
        {
            fprintf(roomFile, "ROOM TYPE: %s\n", "MID_ROOM");
        }

        else if(pickedRooms[j].roomType == END_ROOM)
        {
            fprintf(roomFile, "ROOM TYPE: %s\n", "END_ROOM");
        }

        fclose(roomFile);
    }

}

// create new directory to hold room files
void createDir()
{
    // new directory will add a pid at the end of its name
    struct stat st = {0};
    int pid = getpid();
    char folderName[256];
    char prefix[256] = "chaobe.rooms.";

    sprintf(folderName, "%s%d", prefix, pid);

    if(stat(folderName, &st) == -1)
    {
        mkdir(folderName, 0777);
    }

    createFiles(folderName);
}

int main() {
    // seed so rand() can be more random
    srand(time(NULL));
    roomGraph();

    // testing
    /*int z;
    for(z = 0; z < 7; z++) {
        printf("%s has %i connections\n", pickedRooms[z].name, pickedRooms[z].connectionsNum);
        int y;
        for (y = 0; y < pickedRooms[z].connectionsNum; y++) {
            printf("%s ", pickedRooms[z].connections[y]->name);
        }
        printf("\n\n");
    }*/

    // after creating room graph, create directory and add room data files
    createDir();

    return 0;
}
