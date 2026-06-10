#define STB_IMAGE_IMPLEMENTATION
#include "../stb_image.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "../stb_image_write.h"
#include <iostream>
#include <vector>
#include <cmath>
#include <chrono>
#include <bits/stdc++.h>
using namespace std;
using namespace chrono;
// vector for 2d arrays
// cmath for sqrt() and pow()
struct pixel
{
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
    for (int y = 0; y < h; y++)
    // y : row number
    {
        for (int x = 0; x < w; x++)
        // c : col number
        {
            int i = (y * w + x) * 3;
            // after 1st row , w = 3 second row starts at 3 so thats why
            // converts 2d pos to 1d index with y*w+x * 3
            // why 3? cuz each pixel takes 3 slots i.e r g b
            // and stores r g b into a 2d grid
            pixels[y][x] = {img[i], img[i + 1], img[i + 2]};
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
vector<vector<double>> compute_energy(vector<vector<pixel>> &pixels, int w, int h, vector<vector<bool>> &mask)
{
    vector<vector<double>> energy(h, vector<double>(w, 0));
    // returns a 2d grid of doubles
    // same size as image
    // but each cell holds an energy value isntead of pixel
    // initialised to 0
    for (int y = 0; y < h; y++)
    {
        for (int x = 0; x < w; x++)
        {
            // accessing each pixel in pixels
            pixel left = pixels[y][max(x - 1, 0)];
            // max min is border handling - if youre at x = 0 // theres no left neighbour
            pixel right = pixels[y][min(x + 1, w - 1)];
            pixel up = pixels[max(y - 1, 0)][x];
            pixel down = pixels[min(y + 1, h - 1)][x];
            double dx = pow(right.r - left.r, 2) + pow(right.g - left.g, 2) + pow(right.b - left.b, 2);
            double dy = pow(down.r - up.r, 2) + pow(down.g - up.g, 2) + pow(down.b - up.b, 2);
            energy[y][x] = sqrt(dy + dx);
            if (mask[y][x])
                energy[y][x] = -1e9;
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

vector<vector<double>> compute_energy_sobel(vector<vector<pixel>> &pixels, int w, int h, vector<vector<bool>> &mask)
{
    vector<vector<double>> energy(h, vector<double>(w, 0));
    for (int y = 0; y < h; y++)
    {
        for (int x = 0; x < w; x++)
        {
            // get all 8 neighbors (clamped at borders)
            pixel tl = pixels[max(y - 1, 0)][max(x - 1, 0)];         // top-left
            pixel tm = pixels[max(y - 1, 0)][x];                     // top-middle
            pixel tr = pixels[max(y - 1, 0)][min(x + 1, w - 1)];     // top-right
            pixel ml = pixels[y][max(x - 1, 0)];                     // middle-left
            pixel mr = pixels[y][min(x + 1, w - 1)];                 // middle-right
            pixel bl = pixels[min(y + 1, h - 1)][max(x - 1, 0)];     // bottom-left
            pixel bm = pixels[min(y + 1, h - 1)][x];                 // bottom-middle
            pixel br = pixels[min(y + 1, h - 1)][min(x + 1, w - 1)]; // bottom-right

            // sobel kernels applied to each channel
            // Gx kernel: -1 0 +1 / -2 0 +2 / -1 0 +1
            // Gy kernel: +1 +2 +1 / 0 0 0 / -1 -2 -1
            // auto gx = [&](int l, int r)
            //{ return -l + r; };
            // auto gy = [&](int t, int b)
            //{ return -t + b; };

            double gx_r = -tl.r - 2 * ml.r - bl.r + tr.r + 2 * mr.r + br.r;
            double gx_g = -tl.g - 2 * ml.g - bl.g + tr.g + 2 * mr.g + br.g;
            double gx_b = -tl.b - 2 * ml.b - bl.b + tr.b + 2 * mr.b + br.b;

            double gy_r = -tl.r - 2 * tm.r - tr.r + bl.r + 2 * bm.r + br.r;
            double gy_g = -tl.g - 2 * tm.g - tr.g + bl.g + 2 * bm.g + br.g;
            double gy_b = -tl.b - 2 * tm.b - tr.b + bl.b + 2 * bm.b + br.b;

            double gx_mag = gx_r * gx_r + gx_g * gx_g + gx_b * gx_b;
            double gy_mag = gy_r * gy_r + gy_g * gy_g + gy_b * gy_b;

            energy[y][x] = sqrt(gx_mag + gy_mag);
            if (mask[y][x])
                energy[y][x] = -1e9;
        }
    }
    return energy;
}
// dp logic
vector<int> find_seam(vector<vector<double>> &energy, int w, int h)
{
    vector<vector<double>> dp(h, vector<double>(w, 0));
    vector<vector<int>> parent(h, vector<int>(w, -1));
    for (int x = 0; x < w; x++)
    {
        dp[0][x] = energy[0][x];
    }
    for (int y = 1; y < h; y++)
    {
        for (int x = 0; x < w; x++)
        {
            double best = dp[y - 1][x];
            int best_x = x;
            if (x - 1 >= 0 && dp[y - 1][x - 1] < best)
            {
                best = dp[y - 1][x - 1];
                best_x = x - 1;
            }
            if (x + 1 < w && dp[y - 1][x + 1] < best)
            {
                best = dp[y - 1][x + 1];
                best_x = x + 1;
            }
            dp[y][x] = energy[y][x] + best;
            parent[y][x] = best_x;
        }
    }
    int min_x = 0;
    for (int x = 1; x < w; x++)
    {
        if (dp[h - 1][x] < dp[h - 1][min_x])
        {
            min_x = x;
        }
    }
    vector<int> seam(h);
    seam[h - 1] = min_x;
    for (int y = h - 2; y >= 0; y--)
    {
        seam[y] = parent[y + 1][seam[y + 1]];
    }
    return seam;
}
void seam_remove(vector<vector<pixel>> &pixels,
                 vector<vector<bool>> &mask,
                 vector<int> &seam, int w, int h)
{
    for (int y = 0; y < h; y++)
    {
        int x = seam[y];
        for (int j = x; j < w - 1; j++)
        {
            pixels[y][j] = pixels[y][j + 1];
            mask[y][j] = mask[y][j + 1];
        }
        pixels[y].pop_back();
        mask[y].pop_back();
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
int main(int argc, char *argv[])
{
    if (argc < 5)
    {
        cout << "Usage: ./seamcarve input.png output.png <vertical_seams> <horizontal_seams>\n";
        return 1;
    }
    string input_path = argv[1];
    string output_path = argv[2];
    int vertical_seams = stoi(argv[3]);
    int horizontal_seams = stoi(argv[4]);
    // after parsing argv, add:
    bool use_sobel = (argc >= 6 && string(argv[5]) == "--sobel");

    int w, h, channels;
    unsigned char *img = stbi_load(input_path.c_str(), &w, &h, &channels, 3);
    if (!img)
    {
        cout << "Failed to load: " << input_path << "\n";
        return 1;
    }

    cout << "Loaded: " << w << "x" << h << "\n";
    auto pixels = load_pixels(img, w, h);
    stbi_image_free(img);
    auto start = high_resolution_clock::now();
    // after loading image, before loops
    if (vertical_seams >= w || horizontal_seams >= h)
    {
        cout << "Error: can't remove more seams than image dimensions\n";
        cout << "Max vertical seams: " << w - 1 << "\n";
        cout << "Max horizontal seams: " << h - 1 << "\n";
        return 1;
    }
    // in main, add mask_path as optional 6th argument (or 7th if --sobel is 6th)
    // simplest: just hardcode mask loading for now, add CLI later

    // load mask if provided
    string mask_path = "";
    for (int i = 5; i < argc; i++)
    {
        string arg = argv[i];
        if (arg != "--sobel")
            mask_path = arg;
    }

    vector<vector<bool>> mask(h, vector<bool>(w, false));
    if (mask_path != "")
    {
        int mw, mh, mc;
        unsigned char *mask_img = stbi_load(mask_path.c_str(), &mw, &mh, &mc, 3);
        if (mask_img && mw == w && mh == h)
        {
            for (int y = 0; y < h; y++)
                for (int x = 0; x < w; x++)
                {
                    int i = (y * w + x) * 3;
                    // white pixel in mask = remove this region
                    if (mask_img[i] > 128)
                        mask[y][x] = true;
                }
            stbi_image_free(mask_img);
            cout << "Mask loaded: " << mask_path << "\n";
        }
    }
    // vertical seams
    for (int i = 0; i < vertical_seams; i++)
    {
        auto energy = use_sobel ? compute_energy_sobel(pixels, w, h, mask)
                                : compute_energy(pixels, w, h, mask);
        auto seam = find_seam(energy, w, h);
        seam_remove(pixels, mask, seam, w, h);
        w--;
    }
    cout << "Vertical done. Width: " << w << "\n";
    auto v_end = high_resolution_clock::now();
    double v_time = duration<double>(v_end - start).count();
    cout << "Vertical done: " << vertical_seams << " seams in "
         << v_time << "s ("
         << (v_time / vertical_seams * 1000) << "ms per seam)\n";

    // transpose mask too
    vector<vector<bool>> mask_t(w, vector<bool>(h, false));
    for (int y = 0; y < h; y++)
        for (int x = 0; x < w; x++)
            mask_t[x][y] = mask[y][x];
    mask = mask_t;
    // horizontal seams
    pixels = transpose(pixels, w, h);
    swap(w, h);
    auto h_start = high_resolution_clock::now();
    for (int i = 0; i < horizontal_seams; i++)
    {
        auto energy = use_sobel ? compute_energy_sobel(pixels, w, h, mask)
                                : compute_energy(pixels, w, h, mask);
        auto seam = find_seam(energy, w, h);
        seam_remove(pixels, mask, seam, w, h);
        w--;
    }
    auto h_end = high_resolution_clock::now();
    double h_time = duration<double>(h_end - h_start).count();
    if (horizontal_seams > 0)
        cout << "Horizontal done: " << horizontal_seams << " seams in "
             << h_time << "s ("
             << (h_time / horizontal_seams * 1000) << "ms per seam)\n";
    pixels = transpose(pixels, w, h);
    swap(w, h);
    h -= horizontal_seams;
    cout << "Horizontal done. Height: " << h << "\n";

    // save output
    vector<unsigned char> out(w * h * 3);
    for (int y = 0; y < h; y++)
        for (int x = 0; x < w; x++)
        {
            int i = (y * w + x) * 3;
            out[i] = pixels[y][x].r;
            out[i + 1] = pixels[y][x].g;
            out[i + 2] = pixels[y][x].b;
        }
    stbi_write_png(output_path.c_str(), w, h, 3, out.data(), w * 3);
    cout << "Saved: " << output_path << " (" << w << "x" << h << ")\n";
    return 0;
}