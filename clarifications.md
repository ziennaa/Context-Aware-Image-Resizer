where is path stored ?
parent array !
while filling DP table for every pixel 
you also storw which pixel above it gave the minimum
then after finding minimum in the last row
you trace back upward using parent points

"Why only 3 options (down-left, down, down-right)

Yes, adjacency. A seam must be a connected path — you can't teleport. From pixel (y,x) you can only reach (y+1, x-1), (y+1, x), or (y+1, x+1). That's the definition of a valid seam.

How does DP deal with shifting?
each row : array!
seam passes through coloum 3 in this row
[A] [B] [C] [D] [E]
         ^remove

[A] [B] [D] [E]

width here decreases by 1

How to artificially make masked region low energy

if (mask[y][x] == REMOVE) energy[y][x] = -1e9;
if (mask[y][x] == PROTECT) energy[y][x] = 1e9;

The mask is just a 2D boolean array the user paints over. You set the energy values manually before the DP step runs.


"What do dp and parent arrays do?"

vector<vector<double>> dp(height, vector<double>(width));
dp[y][x] = minimum total energy of any seam starting from row 0 and reaching pixel (y,x)

vector<vector<int>> parent(height, vector<int>(width));
parent[y][x] = which column (x-1, x, or x+1) in the row above gave that minimum. Used for backtracking the path.

project DP or greedy?"
DP. Always use DP. Greedy is only mentioned as a comparison — it's faster but gives wrong results. Your project uses DP which gives the optimal minimum energy seam guaranteed.


forward energy ?
how do you end up at higher energy after removing low energy?

Forward energy — how do you end up at higher energy after removing low energy?"
Imagine this: you remove a seam through a boring sky region. But now the two sides that join together create a new sharp edge that didn't exist before. The new neighbouring pixels clash visually. Forward energy accounts for this by penalizing seams that create new edges when removed. It's an advanced optimization — don't implement it in v1, mention it as future work.

so what happens is !
when youre about to remove a seam 
before removing it
you simulate happens 
you check if i remove this pixel
what new edge gets created b/w its left n right

 Before:  A | B | C | D
Remove B: A | C | D
New edge created between A and C

forward energy : adds cost of that new A-C edge into seam's energy score
so seam that creat ugly new eges become more expensive and gers avoided 

 THIS IS TO BE ADDED!!!!! ITS SOOO WOWOWOWOW
 but how does it account for this ?? like how do yk by seaming through u get clashh?
 once formed ?? does the image keeps on chekcing itself or what??
 what if u get an image that always leads to some clash??
 then itll infinite loop??\


 enerygy formula

 dx² = (R_right - R_left)² + (G_right - G_left)² + (B_right - B_left)²
 dy² = (R_down - R_up)² + (G_down - G_up)² + (B_down - B_up)²
energy = sqrt(dx² + dy²)

basically this is distance formual only ??
this is eucledians distance in color spce bw neighbouring pixels
high distance = colors are very different = edge proeserve
low distance = colors are simillar = boring region = remove 

if colours change sharpely !! - high energy !! - important pixel !!preserve it
flat sky - all similalr - low enerygy - safe to remove

realtime?
processing fast enough that as you drag a slider the image resizes live on the screen

should i do it's impleemntation later ?? like yk how u resize a window typa in cpp locally ig ???
alsoo does this seaming thing works on gif n all too ??


"Sobel vs simple gradient — which should I use?"
Start with simple gradient (just left-right difference). Then add Sobel as a second energy function and compare results. Simple gradient: uses 2 neighbors. Sobel: uses all 8 surrounding pixels, weighted. Sobel gives cleaner edge detection. Comparing both = one strong benchmark bullet.


mEnergy = 1(IR-mR)2 +(IG - mG)2 + (IB -mB)2 + (rR -mR)2 + (rG -mG)2 + (rB - mB)2
formual
mEnergy - Energy (importance) of the middle pixel ([0..626] if rounded)
lR - Red channel value for the left pixel ([0..255])
mR - Red channel value for the middle pixel ([0..255])
rR - Red channel value for the right pixel ([0..255])
lG - Green channel value for the left pixel ([0..255])


this also i didnt get ??? like what it si ?? same formulaa ??

also if i'm gonna simulate greedy? how will that implementation look like ??


also if i do resizing thing what if someone does it diagnoally i.e both hoerizontal and veritcal idk if ill even do this ?? but im curious can both happen at same time ??


greedy implementation ??
```
vector<int> findGreedySeam(energy){
    vector<int> seam(height);
    int mixX = 0;
    for(int x = 1; x<width; x++){
        if(energy[0][x] < energy[0][minX]) minX = x;
    }
    seam[0] = minX;
    for(int y=1; y<height; y++){
        int x = seam[y-1];
        int bextX = x;
        if (x-1 >= 0 && energy[y][x-1] < energy[y][bestX]) bestX = x-1;
        if (x+1 < width && energy[y][x+1] < energy[y][bestX]) bestX = x+1;
        seam[y] = bestX;
    }
    return seam;
}
```

seam carving on gifs

gif : frames 
so apply seam carving to each fram
indepenedently
But the seams would flicker between frames unless you use the same seamfor every frame


Not truly simultaneously — computers are sequential. But you can alternate: remove one vertical seam, then one horizontal seam, then repeat. The original paper actually discusses the optimal order of removing horizontal vs vertical seams as a separate DP problem — finding the best sequence of H and V removals. That's called optimal seam order and it's genuinely interesting. Way too advanced for v1 but worth one line in your future work section.
