#define STB_IMAGE_IMPLEMENTATION
#include "../stb_image.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "../stb_image_write.h"
#include <iostream>
#include <vector>
#include <cmath>
#include <bits/stdc++.h>
using namespace std;
// vector for 2d arrays
// cmath for sqrt() and pow()
struct pixel{
    unsigned char r, g, b;
    // pixel DS custom
};
// stb - actual pixel data comes as flat 1d array not 2d grid
// image visually looks like this:
/*
(0,0)(0,1)(0,2)
(1,0)(1,1)(1,2)
(2,0)(2,1)(2,2)
*/
// but stb gives this in a straight line 1 d arrary
/*
[R, G, B, R, G, B, R, G, B, R, G, B, R, G, B, R, G, B, R, G, B, R, G, B, R, G, B]
 px00 px01 px02 px10 px11 px12 px20 px21 px22
*/
    vector<vector<pixel>>
    load_pixels(unsigned char *img, int w, int h)
{
    // 2d grid each cell holds a pixel - r, g, b
    // 1d array stb that i got convert it into 2d grid
    vector<vector<pixel>> pixels(h, vector<pixel>(w));
    /*
    vector<Pixel>(w)          // one row of w pixels
    vector<vector<Pixel>>(h, ...) // h rows of those
    */
    // pixels(h, vector<pixel>(w)) 
    // makes a grid of empty pixels
    for(int y = 0; y<h; y++)
    // y : row number 
    {
        for(int x = 0; x<w; x++)
        // c : col number
        {
            int i = (y*w+x) * 3;
            // after 1st row , w = 3 second row starts at 3 so thats why
            // converts 2d pos to 1d index with y*w+x * 3
            // why 3? cuz each pixel takes 3 slots i.e r g b
            // and stores r g b into a 2d grid
            pixels[y][x] = {img[i], img[i+1], img[i+2]};
            // r = img[i]
            // g = img[i+1]
            // b = img[i+2]
        }
    }
    return pixels;
    // entire 2d grid of pixels 
    // after calling loadpixels()
}

// energy of each pixel 
vector<vector<double>> compute_energy(vector<vector<pixel>> &pixels, int w, int h){
    vector<vector<double>> energy(h, vector<double>(w, 0));
    // returns a 2d grid of doubles 
    // same size as image
    // but each cell holds an energy value isntead of pixel
    // initialised to 0
    for(int y=0; y<h; y++){
        for(int x = 0; x<w; x++){
            // accessing each pixel in pixels
            pixel left = pixels[y][max(x-1, 0)];
            // max min is border handling - if youre at x = 0 // theres no left neighbour
            pixel right = pixels[y][min(x+1, w-1)];
            pixel up = pixels[max(y-1, 0)][x];
            pixel down = pixels[min(y+1, h-1)][x];
            double dx = pow(right.r - left.r, 2) + pow(right.g - left.g, 2) + pow(right.b - left.b, 2);
            double dy = pow(down.r - up.r, 2) + pow(down.g - up.g, 2) + pow(down.b - up.b, 2);
            energy[y][x] = sqrt(dy+dx);
        }
    }
    return energy;
}
/*/
[UP]
         ↑
[LEFT] [THIS] [RIGHT]
         ↓
       [DOWN]


       col:  0    1    2    3    4
       row0: .    .    .    .    .
       row1: .    .    .   [UP]  .
       row2: .    .  [LEFT][ME][RIGHT]
       row3: .    .    .  [DOWN] .


       col:  0    1    2
       row2:[ME][RIGHT] .
         ↑
       no left neighbor exists!

       Pixel left = pixels[y][max(x-1, 0)];
       Pixel right = pixels[y][min(x+1, w-1)];
       If x is the last column (x = w-1), then x+1 would go out of bounds. min(x+1, w-1) keeps it at the last valid column.

       This works for ANY image size — 3x3, 1000x1000, anything. The max/min just handles the 4 edges safely.

       */

// dp logic
vector<int> find_seam(vector<vector<double>>& energy, int w, int h){
    vector<vector<double>> dp(h, vector<double>(w, 0));
    vector<vector<int>> parent(h, vector<int>(w, -1));
    for(int x=0; x<w; x++){
        dp[0][x] = energy[0][x];
    }
    for(int y=1; y<h; y++){
        for(int x = 0; x<w; x++){
            double best = dp[y-1][x];
            int best_x = x;
            if(x-1 >= 0 && dp[y-1][x-1] < best){
                best = dp[y-1][x-1];
                best_x = x-1;
            }
            if(x+1 < w && dp[y-1][x+1] < best){
                best = dp[y-1][x+1];
                best_x = x+1;
            }
            dp[y][x] = energy[y][x] + best;
            parent[y][x] = best_x;
        }
    }
    int min_x = 0;
    for(int x =1; x<w; x++){
        if(dp[h-1][x] < dp[h-1][min_x]){
            min_x = x;
        }
    }
    vector<int> seam(h);
    seam[h-1] = min_x;
    for(int y=h-2; y>=0; y--){
        seam[y] = parent[y+1][seam[y+1]];
    }
    return seam;
}
void seam_remove(vector<vector<pixel>>& pixels, vector<int>& seam, int w, int h){
    for(int y=0; y<h; y++){
        int x = seam[y];
        for(int j=x; j<w-1; j++){
            pixels[y][j] = pixels[y][j+1];
        }
        pixels[y].pop_back();
    }
}
vector<vector<pixel>> transpose(vector<vector<pixel>> &pixels, int w, int h)
{
    vector<vector<pixel>> transposed(w, vector<pixel>(h));
    for (int y = 0; y < h; y++)
        for (int x = 0; x < w; x++)
            transposed[x][y] = pixels[y][x];
    return transposed;
}
int main(int argc, char *argv[]){
    if (argc < 5)
    {
        cout << "Usage: ./seamcarve input.png output.png <vertical_seams> <horizontal_seams>\n";
        cout << "Example: ./seamcarve images/test.png carved.png 200 100\n";
        return 1;
    }
    string input_path = argv[1];
    string output_path = argv[2];
    int vertical_seams = stoi(argv[3]);
    int horizontali_seams = stoi(argv[4]);
    int w, h, channels;
    unsigned char *img = stbi_load(input_path.c_str(), &w, &h, &channels, 3);
    if (!img)
    {
        cout << "Failed to load image\n";
        return 1;
    }
    auto pixels = load_pixels(img, w, h);
    auto energy = compute_energy(pixels, w, h);
    auto seam = find_seam(energy, w, h);
    // draw seam in red on original img
    for(int y=0; y<h; y++){
        int x = seam[y];
        int i = (y*w+x)*3;
        img[i] = 255;
        img[i+1] = 0;
        img[i+2] = 0;
    }
    stbi_write_png("seam.png", w, h, 3, img, w * 3);
    seam_remove(pixels, seam, w, h);
    w--;
    vector<unsigned char> out(w * h * 3);
    for (int y = 0; y < h; y++){
        for (int x = 0; x < w; x++)
        {
            int i = (y * w + x) * 3;
            out[i] = pixels[y][x].r;
            out[i + 1] = pixels[y][x].g;
            out[i + 2] = pixels[y][x].b;
        }
    }
    cout<<"TESTING\n";
    stbi_write_png("removed.png", w, h, 3, out.data(), w * 3);
    cout << "TESTING SUCCESSFUL Seam removed. New width: " << w << "\n";
    int seams_to_remove = vertical_seams;
    for(int i=0; i<seams_to_remove; i++){
        auto energy = compute_energy(pixels, w, h);
        auto seam = find_seam(energy, w, h);
        seam_remove(pixels, seam, w, h);
        w--;
    }
    cout << "Vertical seams done. Width: " << w << "\n";

    // --- remove 100 horizontal seams ---
    int horizontal_seams = horizontali_seams;
    pixels = transpose(pixels, w, h);
    swap(w, h);

    for (int i = 0; i < horizontal_seams; i++)
    {
        auto energy = compute_energy(pixels, w, h);
        auto seam = find_seam(energy, w, h);
        seam_remove(pixels, seam, w, h);
        w--;
    }

    pixels = transpose(pixels, w, h);
    swap(w, h);
    h -= horizontal_seams; // fix h after transposing back
    cout << "Horizontal seams done. Height: " << h << "\n";
    vector<unsigned char> outi(w * h * 3);
    for (int y = 0; y < h; y++)
        for (int x = 0; x < w; x++)
        {
            int i = (y * w + x) * 3;
            outi[i] = pixels[y][x].r;
            outi[i + 1] = pixels[y][x].g;
            outi[i + 2] = pixels[y][x].b;
        }
    stbi_write_png(output_path.c_str(), w, h, 3, outi.data(), w * 3);
    cout << "Done verticval seeam. Final width: " << w << "\n";

    double max_energy = 0;
    // this is for normalisation
    // because each pixel should be b/w 0 to 255
    for(auto& row: energy){
        for(auto val : row){
            max_energy = max(max_energy, val);
        }
    }
    // save energy map as grey scale image
    vector<unsigned char> energy_img(w * h * 3);
    // new image, stb needs a flat 1d array
    for (int y = 0; y < h; y++)
    {
        for (int x = 0; x < w; x++)
        {
            unsigned char val = (unsigned char)(energy[y][x] / max_energy * 255);
            // normalisation formulla
            // energy[y][x] / max_energy   → gives you a number between 0.0 and 1.0
            // × 255                        → stretches it to 0 to 255
            // (unsigned char)              → converts double to integer
            /*
            pixel energy  = 600
            max energy    = 1200
            600/1200      = 0.5
            0.5 * 255     = 127   ← medium grey
            */
            int i = (y*w+x) * 3;
            energy_img[i] = energy_img[i+1] = energy_img[i+2] = val;
            // all same values  why??
            // because u want greyscale image 
            // in greyscal r g b r alwys same
        }
    }
    stbi_write_png("energy.png", w, h, 3, energy_img.data(), w*3);
    cout<<"energy map saved to energy.png\n";
    stbi_image_free(img);
    return 0;
}
