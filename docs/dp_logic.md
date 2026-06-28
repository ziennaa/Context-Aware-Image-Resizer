Row 0:  [3]  [4]  [1]  [2]
Row 1:  [6]  [1]  [8]  [3]
Row 2:  [5]  [2]  [3]  [1]

You want to find the cheapest path from top to bottom. One pixel per row.
You need to find a path top→bottom where each step can only go diagonally one step left/right or straight down. Like this:

↓  or  ↘  or  ↙
double best = dp[y-1][x] — what is this?
calculating cost to reach pixel y,x 
you're looking at the row above y-1 and asking, which of these 3 pixels
is the cheapest to come from ?

Row y-1:  ...  [?]  [?]  [?]  ...
                      ↑
              assume came from here first (same x)

Row y:    ...  [?] [ME]  [?]  ...

best = dp[y-1][x]
means 
assume the best prev pixel is directly above me
best_x = x remembers where that came from!

Row y-1:  [x-1]  [x]  [x+1]
            ↖     ↑     ↗
Row y:           [ME]

if(x->= 0 && dp[y-1][x-1] < best){
    best = dp[y-1][x-];
    best_X = x-1;
}

if (x+1 < w && dp[y-1][x+1] < best) {
    best = dp[y-1][x+1];
    best_x = x+1;
}

After these two checks, best holds the minimum of all 3 options above, and best_x remembers which column it came from.

The x-1 >= 0 check is just making sure you don't go out of bounds on the left edge. Same for x+1 < w on the right edge.


dp[y][x] = energy+best
now you know the cheapest way to reach y,x it's 
energy of the current pixel
plus the cheapest path that led here ??


energy:          dp after filling:
3  4  1  2       3   4   1   2      ← row 0: just copy
6  1  8  3       9   2   9   4      ← row 1: each = energy + min above
5  2  3  1       7   4   5   3      ← row 2: same

For dp[1][1] = energy[1][1] + min(dp[0][0], dp[0][1], dp[0][2])


parent[y][x] = bestx
This stores WHERE you came from. So later you can trace back the path.

parent table for above example:
-   -   -   -     ← row 0: no parent
0   2   1   2     ← row 1: each cell stores which column above it came from
1   1   1   2     ← row 2

why h and not h-1 ??
arrays : 0 indexed 

Last row of dp contains the total cost of the cheapest seam ending at each column. You just scan across and find which column has the smallest total. That's where your seam ends.

Last row dp:  7   4   5   3
                          ↑
                     min is here (index 3)

Backtracking to get the seam path
vector<int> seam(h);      // stores one x-coordinate per row
seam[h-1] = min_x;        // start at the minimum in last row
for (int y = h-2; y >= 0; y--)
    seam[y] = parent[y+1][seam[y+1]];

energy:        
3  4  1  2     
6  1  8  3     
5  2  3  1

dp row 0:  3  4  1  2

dp:                  parent:
3   4   1   2        -   -   -   -
9   2   9   4        0   2   2   2
7   4   5   5        1   1   1   3


seam[2] = 1                        ← min_x, start here

seam[1] = parent[2][1] = 1        ← row 2, col 1 came from row 1, col 1

seam[0] = parent[1][1] = 2        ← row 1, col 1 came from row 0, col 2

Row 0: remove pixel at column 2   (energy was 1)
Row 1: remove pixel at column 1   (energy was 1)
Row 2: remove pixel at column 1   (energy was 2)

Total energy = 1 + 1 + 2 = 4  ✓ matches dp[2][1]

col:  0  1  2  3
row0: .  .  X  .     ← remove col 2
row1: .  X  .  .     ← remove col 1
row2: .  X  .  .     ← remove col 1