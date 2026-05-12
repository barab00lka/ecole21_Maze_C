A maze with "thin walls" is a table of n rows by m columns size. There may be "walls" between the cells of a table. The table as a whole is also surrounded by "walls".

The maze can be stored in a file as a number of rows and columns, as well as two matrices containing the positions of vertical and horizontal walls respectively.

The first matrix shows the wall to the right of each cell, and the second - the wall at the bottom.

An example of such a file:

4 4
0 0 0 1
1 0 1 1
0 1 0 1
0 0 0 1

1 0 1 0
0 0 1 0
1 1 0 1
1 1 1 1

The solution to a maze is the shortest path from a given starting point (table cell) to the ending one.

When traversing a maze, you can move to neighboring cells that are not separated by a "wall" from the current cell and that are on the top, bottom, right or left. A route is considered the shortest if it passes through the smallest number of cells.

Maze flaws include isolated areas and loops.
An isolated area is a part of the maze with passages that you cannot access from the rest of the maze. 
A loop is a part of the maze with passages that can be walked in "circles". The walls in the loops are not connected to the walls surrounding the maze.

In many games there is a need for branching locations, such as caves. They can be created by generation using the cellular automaton. This kind of generation uses an idea similar to the Game of Life you are already familiar with. The idea of the proposed algorithm consists of implementing only two steps: first, the whole field is filled randomly with walls - i.e., for each cell it is randomly determined whether it will be free or impassable - and then the map state is updated several times according to the conditions, similar to the birth/death ones in the Game of Life.

The rules are simpler than in the Game of Life - there are two special variables, one for "birth" of "dead" cells (the "birth" limit) and one for destruction of "live" cells (the "death" limit). If "live" cells are surrounded by "live" cells, the number of which is less than the "death" limit, they "die". In the same way, if "dead" cells are next to "live" cells, the number of which is greater than the "birth" limit, they become "live".

A cave that has passed 0 simulation steps (only initialized) can be stored in the file as a number of rows and columns, as well as a matrix containing the positions of "live" and "dead" cells.

An example of such a file:

4 4
0 1 0 1
1 0 0 1
0 1 0 0
0 0 1 1

Part 1. Implementation of the Maze project
You need to implement a Maze program that can generate and render perfect mazes and caves:

The program must be developed in C language
When writing a program, you must use the standard code writing style for the selected language
The program code must be located in the src folder
The program must be built with Makefile which contains standard set of targets for GNU-programs: all, install, uninstall, clean, tests. Installation directory could be arbitrary, except the building one
The program should implement a console-based user interface
The program has an option to load the maze from a file, which is set in the format described above
Maximum size of the maze is 50x50
The loaded maze must be rendered on the screen in the console in pseudo-graphics
"Wall" thickness is 1 character
The size of the maze cells themselves is calculated so that the maze occupies the entire field allotted to it.

Part 2. Generation of a perfect maze
Add the ability to automatically generate a perfect maze.
A maze is considered perfect if it is possible to get from each point to any other point in exactly one way.
You must generate the maze according to Eller's algorithm
The generated maze must not have isolations and loops
Prepare full coverage of the perfect maze generation module with unit-tests
The user enters only the dimensionality of the maze: the number of rows and columns

The generated maze must be saved in the file format described above

The created maze should be displayed on the screen as specified in the first part
Part 3. Solving the maze
Add the ability to show the solution to any maze currently shown on the screen:

The user sets the starting and ending points
The route, which is the solution, must be displayed with "a line" 1 character thick, passing through the middle of all the cells in the maze through which the solution runs.
The color of the solution line must be different from the color of the walls, and the field
Prepare full coverage of the maze solving module with unit-tests
Part 4. Bonus. Cave Generation
Add cave generation using a cellular automaton:

The user specifies the file that describes the cave according to the format described above
Maximum size of the cave is 50x50
The loaded cave must be rendered on the screen in the console in pseudo-graphics
The user sets the limits for "birth" and "death" of a cell, as well as the chance for the starting initialization of the cell
The "birth" and "death" limits can have values from 0 to 7
Cells outside the cave are considered alive
There should be a step-by-step mode for rendering the results of the algorithm in two variants:
The next step option will lead to rendering the next iteration of the algorithm
The automatic work option starts rendering iterations of the algorithm with a frequency of 1 step in N milliseconds, where the number of milliseconds N is set separately by the user
The size of cells in pixels is calculated so that the cave occupies the entire field allotted to it
Prepare full coverage of the cave generation module with unit-tests

