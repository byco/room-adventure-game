# room-adventure-game
A text-based adventure game written in C. The player must go from the starting room to the ending room. The rooms and their connections are randomly generated each game.

The player starts in the starting room and wins when they reach the ending room. They can navigate there by typing in room names; the program uses C-strings. The game can also return the current time, a feature that uses multithreading and mutexes.

# How to play
First compile the two files:
<tt>gcc -o adventure chaobe.adventure.c -lpthread</tt>
<tt>gcc -o buildrooms chaobe.buildrooms.c</tt>

Then build the rooms using <tt>buildrooms</tt>. This program will pick seven room names and designate each as a starting room, a mid room, or an ending room. Each room is connected to three to six other rooms, also generated each time you run this program. Run <tt>buildrooms</tt> again for a completely new room structure.

You can start the game with <tt>adventure</tt>. Type in the room names as appropriate. 

You can also type in <tt>time</tt> and it will print the current time and date (e.g. 08:02pm, Wednesday, May 15, 2019).

The game ends when you reach the ending room. A list of rooms previously traveled through will print out.

# Example
    $ adventure
    CURRENT LOCATION: Field
    POSSIBLE CONNECTIONS: Forest, Arctic, Desert, Hills, Marsh, Lake.
    WHERE TO? >Desert

    CURRENT LOCATION: Desert
    POSSIBLE CONNECTIONS: Forest, Field, Arctic, Marsh.
    WHERE TO? >Marsh

    YOU HAVE FOUND THE END ROOM. CONGRATULATIONS!
    YOU TOOK 2 STEPS. YOUR PATH TO VICTORY WAS:
    Desert
    Marsh
