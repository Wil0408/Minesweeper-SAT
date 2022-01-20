# Minesweeper-SAT
## Game Rule
- Find all squares without mines.
- Each square is either a mine or a number of 1~8, which implys how many mines around the square.
- You can right click the mouse to add flag on the grid and mark the grid with mine.
[Give it a try](https://minesweeper.online/)

<p float="left">
    <img src="https://i.imgur.com/89ykPkY.png" width=500>
</p>

## Problem formulation
Given a plane with some existing squares with a number or a mine (marked "X"), Can we verify the plane is valid?

## NP-completeness
### NP
We can check every square with number and verify that the adjacent mines number is equal to the square number. Assume we are given a plane with n squares, then we need to check at most 8n squares. Therefore, Checking the Minesweeper consistency takes O(n), which is polynomial.
MineSweeper problem is Non-deterministic polynomial.

### NP-hard
Reduce Minesweeper problem to a well-known NP-hard problem SAT (Boolean satisfiability problem). Detail see slides.

## Usage
```
make
./MineSat ./input/<input file> ./output/<output file>
```

## input format
```
3 3    // plane with 3*3 (x-dir*y-dir) squares
1 -2 -2    // -1 stands for mine
-2 3 -2    // -2 stands for unknown
-2 -2 -2    // 1~8 is the number for implying neighbor mines number
```

## output format
```
3 3    // plane with 3*3 (x-dir*y-dir) squares
SAT    // Satisfiable
1 X X     // X means mine
1 3 3 
0 1 X 
```
If SAT, the solver will return a valid solution.

## Applicaiton
Given a plane with only one soluiton, our solver must find the solution.

<p float="left">
  <img src="https://i.imgur.com/obHeCVi.png" width=300 hspace="20">
  <img src="https://i.imgur.com/Rks5xkQ.png" width=300 hspace="20">
</p>

```
// input
10 12
-2 0 -2 1 -2 -2 -2 3 -2 -2 5 -2
-2 -2 -2 -2 3 -2 -2 -2 5 -2 -2 -2
-2 -2 2 -2 -2 2 -2 2 -2 -2 -2 -2
2 -2 -2 3 -2 3 0 -2 2 5 -2 4
-2 -2 4 3 -2 -2 -2 -2 4 -2 -2 2
-2 -2 -2 -2 2 4 -2 -2 -2 -2 -2 -2 
2 -2 -2 -2 1 -2 -2 -2 5 -2 -2 -2
1 -2 -2 2 -2 -2 -2 -2 -2 2 -2 1
1 2 -2 -2 -2 -2 3 3 4 -2 4 -2
1 -2 1 -2 -2 1 -2 -2 2 2 -2 -2
```

```
// output
10 12
SAT
0 0 1 1 2 X X 3 X X 5 X 
0 0 1 X 3 3 3 X 5 X X X 
1 1 2 3 X 2 1 2 X 5 X X 
2 X 2 3 X 3 0 1 2 5 X 4 
3 X 4 3 X 3 2 3 4 X X 2 
3 X X 3 2 4 X X X X 3 1 
2 X X 3 1 X X 6 5 3 1 0 
1 3 X 2 2 3 4 X X 2 2 1 
1 2 2 1 1 X 3 3 4 X 4 X 
1 X 1 0 1 1 2 X 2 2 X X 
```



