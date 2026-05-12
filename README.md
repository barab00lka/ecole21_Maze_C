# Maze and Cave Generation Project

A maze with "thin walls" is a table of `n` rows by `m` columns. There may be walls between adjacent cells. The whole table is surrounded by outer walls.

The maze can be stored in a file containing:
- number of rows and columns
- two matrices: vertical walls (to the right of each cell) and horizontal walls (below each cell).

**Example file content (`4x4`)**
4 4
0 0 0 1
1 0 1 1
0 1 0 1
0 0 0 1

1 0 1 0
0 0 1 0
1 1 0 1
1 1 1 1
text


- `0` = no wall  
- `1` = wall exists  

---

## Solution and Maze Properties

- The **shortest path** from a start cell to an end cell uses moves up/down/left/right where no wall separates the cells. Path length = number of cells visited.
- **Maze flaws**:  
  - *Isolated areas* – parts unreachable from the rest.  
  - *Loops* – passages that can be walked in circles; walls in loops are not connected to the surrounding boundary.

---

## Cave Generation (Cellular Automaton)

Caves can be generated using a cellular automaton similar to Game of Life but simpler:

1. Randomly initialise each cell as "live" (free) or "dead" (wall).
2. Apply update rules:
   - **Birth limit** – a dead cell becomes live if the number of live neighbours ≥ birth limit.
   - **Death limit** – a live cell dies if the number of live neighbours < death limit.
3. Cells **outside** the cave are considered always live.

**Initial cave file format** (0 steps simulated):

n m
<matrix of 0/1>
text


Example `4x4`:

4 4
0 1 0 1
1 0 0 1
0 1 0 0
0 0 1 1
text


---

# Part 1 – Maze Project Implementation

Implement a program in **C** with the following features:

- **C language** – standard code style.
- All code inside `src/` folder.
- **Makefile** with targets: `all`, `install`, `uninstall`, `clean`, `tests`.  
  Installation directory: arbitrary (not the build directory).
- **Console‑based user interface**.
- **Load maze from file** (format described above).  
  Maximum maze size: **50×50**.
- **Render maze** in console pseudo‑graphics:
  - Wall thickness = **1 character**.
  - Cell size auto‑adjusted to fill the available screen area.

---

# Part 2 – Perfect Maze Generation

Add automatic generation of **perfect mazes** (exactly one unique path between any two cells, no isolation, no loops).

- **Algorithm**: **Eller’s algorithm**.
- **User input**: only number of rows and columns (both ≤50).
- **Output**: generated maze saved in the same file format and displayed on screen.
- **Unit tests** – full coverage of the generation module.

---

# Part 3 – Maze Solving

Allow the user to solve the currently displayed maze:

- User selects **start** and **end** cells.
- The **shortest path** is computed and displayed as a **line** 1 character thick, passing through the **middle** of each cell on the solution route.
- The solution line must have a **different colour** from walls and empty space (if terminal supports colour).
- **Unit tests** – full coverage of the solving module.

---

# Part 4 – Bonus: Cave Generation

Implement cave generation using a cellular automaton:

- **Load initial cave** from a file (format described above). Max size 50×50.
- **Render** the cave in console pseudo‑graphics.
- **User parameters**:
  - Birth limit (0–7)
  - Death limit (0–7)
  - Random initialisation chance (%)
  - Animation delay `N` milliseconds (for auto mode)
- **Boundary condition**: cells outside the cave = **always alive**.
- **Interaction modes**:
  - **Step‑by‑step** – one iteration per key press.
  - **Automatic** – iterations every `N` ms (user‑adjustable).
- **Cell size** automatically scaled to fill the screen.
- **Unit tests** – full coverage of the cave generation module.

---

## Building and Execution

| Make target | Action                        |
|-------------|-------------------------------|
| `all`       | Build the executable          |
| `install`   | Install to a chosen directory |
| `uninstall` | Remove installed files        |
| `clean`     | Remove build artefacts        |
| `tests`     | Run unit tests                |

---
## Part 5. Bonus. ML. Reinforcement learning

With the help of reinforcement learning, it is necessary to develop an algorithm for teaching an agent the shortest passage of mazes:

 - The user specifies a file which describes the maze and an ending point
 - The agent must be able to find a way out of the maze from any starting point
 - It is necessary to use the Q-learning method
 - The agent is trained on a single maze, which does not change either during the training process or during the testing phase; the endpoint is also fixed
 - The agent's training module must be fully covered by unit tests

It is required to provide the user with the opportunity to interact with a trained agent:

 -  The user defines the starting point
 -  The route built by the agent from a given point is displayed in accordance with the rules described above

Agent training and interaction modules should be developed in C language without using ready-made reinforcement learning libraries.
## Part 6. Bonus. Web-interface

Add a Web-based version of the user interface in any format (MPA, SPA) using appropriate frameworks. The Web interface must meet at least all the basic functional requirements from the parts above (Part 1-Part 3).
