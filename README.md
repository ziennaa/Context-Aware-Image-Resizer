# Seam Carving — Content-Aware Image Resizing

A C++ implementation of the Seam Carving algorithm for content-aware image resizing, based on the paper *"Seam Carving for Content-Aware Image Resizing"* by Shai Avidan and Ariel Shamir (SIGGRAPH 2007).

Unlike standard resizing which uniformly scales all pixels, seam carving identifies and removes the least visually important paths of pixels — preserving edges, objects, and high-detail regions while shrinking smooth or homogeneous areas.

---

## Demo

| Original | Seam Carving (−200px width) | Standard Resize |
|---|---|---|
| ![original](assets/original.jpg) | ![seam carved](assets/seam_carved.jpg) | ![standard](assets/standard.jpg) |

> Seam carving preserves the subject. Standard resize distorts it.

---

## How It Works

### The Core Idea

Every pixel in an image has an **energy value** — a measure of how visually important it is. Pixels on edges, faces, or high-contrast boundaries have high energy. Pixels in flat backgrounds, sky, or walls have low energy.

A **seam** is a connected path of pixels from the top of the image to the bottom (vertical seam) or left to right (horizontal seam), with exactly one pixel per row/column. To reduce image width by 1 pixel, we find and remove the seam with the lowest total energy.

```
Before removal:
Row 0:  A  B  C  D  E
Row 1:  F  G  H  I  J       Seam: C → H → M → R
Row 2:  K  L  M  N  O
Row 3:  P  Q  R  S  T

After removal (seam pixels deleted, rest shifted left):
Row 0:  A  B  D  E
Row 1:  F  G  I  J
Row 2:  K  L  N  O
Row 3:  P  Q  S  T
```

Repeating this process N times shrinks the image width by N pixels.

---

### Stage 1: Energy Calculation

For every pixel at position (y, x), energy is calculated using the **dual-gradient magnitude** — measuring color difference with horizontal and vertical neighbors:

```
dx² = (R_right − R_left)² + (G_right − G_left)² + (B_right − B_left)²
dy² = (R_down  − R_up)²  + (G_down  − G_up)²  + (B_down  − B_up)²

energy(y, x) = sqrt(dx² + dy²)
```

High energy = sharp color change = edge or important detail.
Low energy = flat region = safe to remove.

This project implements and compares **two energy functions**:
- **Simple Gradient** — uses only left/right neighbors (horizontal difference only)
- **Sobel Filter** — uses all 8 surrounding pixels with weighted kernels, giving cleaner edge detection

#### Sobel Filter (advanced energy):
```
Kernel Gx (horizontal edges):    Kernel Gy (vertical edges):
-1  0  +1                        +1  +2  +1
-2  0  +2                         0   0   0
-1  0  +1                        -1  -2  -1

energy = sqrt(Gx² + Gy²)
```

---

### Stage 2: Finding the Minimum Energy Seam (Dynamic Programming)

Finding the lowest-energy seam is a classic **optimal path problem** solved using dynamic programming.

#### Why not greedy?
A greedy approach always picks the lowest-energy neighboring pixel at each step. This is fast but does not guarantee the globally optimal seam — it can get "trapped" in a locally cheap path that leads to expensive pixels later.

#### Why DP works:
The minimum energy of a seam ending at pixel (y, x) depends only on the minimum energy seam ending at one of the three pixels directly above it. This is **optimal substructure** — the hallmark of DP.

```
dp[y][x] = energy[y][x] + min(
    dp[y−1][x−1],   // from upper-left
    dp[y−1][x],     // from directly above
    dp[y−1][x+1]    // from upper-right
)
```

Two tables are maintained:
- `dp[y][x]` — minimum total energy of any seam reaching pixel (y,x) from the top
- `parent[y][x]` — which column in the row above gave that minimum (used for backtracking)

#### Dry Run Example:
```
Energy map:          DP table:            Parent table:
3   4   1            3   4   1            -   -   -
6   1   8            9   2   9            1   2   1
5   2   3            7   4   5            1   1   1

Minimum in last row = 4 at column 1
Backtrack: (2,1) → parent=1 → (1,1) → parent=2 → (0,2)
Seam path: (0,2) → (1,1) → (2,1)   Total energy: 1+1+2 = 4
```

Time complexity: **O(W × H)** per seam removal.

---

### Stage 3: Seam Removal

Once the seam is identified, for each row we remove the seam pixel and shift remaining pixels left:

```cpp
for each row y:
    seamX = seam[y]
    for x from seamX to width-2:
        image[y][x] = image[y][x+1]
    width--
```

---

### Stage 4: Energy Recalculation

After every seam removal, neighboring pixels change — pixels that were not adjacent are now next to each other. This means the energy map must be updated.

**Simple approach:** Recalculate full energy map after each removal — O(W × H) per iteration.

**Optimized approach:** Only recalculate energy for pixels adjacent to the removed seam — reduces unnecessary computation significantly. Benchmarked in this project.

---

### Stage 5: Repeat

```
while (current_width > target_width):
    energy = computeEnergy(image)
    seam   = findMinSeam(energy)
    image  = removeSeam(image, seam)
```

---

## Features Implemented

| Feature | Status |
|---|---|
| Vertical seam removal (width reduction) | ✅ |
| Horizontal seam removal (height reduction) | ✅ |
| Simple gradient energy function | ✅ |
| Sobel filter energy function | ✅ |
| Energy map visualization (save as image) | ✅ |
| Seam path visualization (overlay on image) | ✅ |
| Object removal via low-energy mask | ✅ |
| Region protection via high-energy mask | ✅ |
| Full energy recalculation (baseline) | ✅ |
| Partial energy recalculation (optimized) | ✅ |
| Greedy seam finder (for comparison) | ✅ |
| CLI interface | ✅ |

---

## Algorithm Comparison: DP vs Greedy

This project implements both DP and greedy seam finding, allowing direct quality comparison.

| | DP | Greedy |
|---|---|---|
| Optimality | Globally optimal seam | Locally optimal only |
| Time complexity | O(W × H) | O(W + H) |
| Result quality | High | Degrades on complex images |
| Use case | Correct resizing | Fast approximation |

Greedy failure example: it chooses the cheapest next step but gets locked into a path that passes through high-energy pixels further down. DP considers all possible paths implicitly and always finds the true minimum.

---

## Benchmarks

All benchmarks run on [machine specs] using a [X × Y] test image.

### Seam removal time vs image size

| Image Size | Time per seam (ms) | 100 seams total (s) |
|---|---|---|
| 500 × 500 | - | - |
| 1000 × 1000 | - | - |
| 2000 × 2000 | - | - |

### Full vs partial energy recalculation

| Method | 100 seams (s) | 500 seams (s) |
|---|---|---|
| Full recalculation | - | - |
| Partial recalculation | - | - |
| Speedup | ~Xx | ~Xx |

### Energy function quality comparison

Subjective quality comparison (simple gradient vs Sobel) on images with:
- Hard horizontal edges
- Diagonal edges
- Low contrast backgrounds

*(Results with side-by-side images in `/assets/comparison/`)*

---

## Usage

```bash
# Build
g++ -O2 -o seamcarve main.cpp -std=c++17

# Reduce width by 200 pixels
./seamcarve input.jpg output.jpg --width -200

# Reduce height by 100 pixels
./seamcarve input.jpg output.jpg --height -100

# Use Sobel energy function
./seamcarve input.jpg output.jpg --width -200 --energy sobel

# Save energy map visualization
./seamcarve input.jpg output.jpg --width -200 --save-energy

# Remove object (provide mask image — black = remove, white = keep)
./seamcarve input.jpg output.jpg --mask mask.jpg
```

---

## Project Structure

```
seam-carving/
├── src/
│   ├── main.cpp          — CLI entry point
│   ├── image.cpp/.h      — Image loading/saving (stb_image)
│   ├── energy.cpp/.h     — Energy functions (gradient, Sobel)
│   ├── seam.cpp/.h       — DP seam finder, greedy seam finder
│   ├── carve.cpp/.h      — Seam removal, mask support
│   └── benchmark.cpp/.h  — Benchmarking utilities
├── assets/
│   ├── original.jpg
│   ├── seam_carved.jpg
│   └── comparison/
├── stb_image.h           — Single-header image library
├── stb_image_write.h
└── README.md
```

---

## Dependencies

- **stb_image** / **stb_image_write** — single-header image I/O library (no installation required)
- C++17 or later
- No other external dependencies

---

## References

1. Avidan, S., & Shamir, A. (2007). *Seam carving for content-aware image resizing*. ACM Transactions on Graphics (SIGGRAPH 2007). https://perso.crans.org/frenoy/matlab2012/seamcarving.pdf
2. Trekhleb — Content-aware image resizing in JavaScript. https://trekhleb.dev/blog/2021/content-aware-image-resizing-in-javascript/
3. Wikipedia — Seam carving. https://en.wikipedia.org/wiki/Seam_carving
4. Zucconi, A. (2023). Seam Carving. https://www.alanzucconi.com/2023/05/29/seam-carving/

---

## Future Work

- **Forward energy** — account for energy *created* by seam removal, not just energy removed (Rubinstein et al. 2008)
- **Real-time seam carving** — optimize for live interactive resizing
- **Seam insertion** — enlarge images by duplicating low-energy seams
- **Multi-size retargeting** — generate multiple target sizes from a single pass using the index map approach (Avidan 2007)
- **WASM port** — compile to WebAssembly for browser-based demo

---

Key insights:
- DP guarantees optimality because of overlapping subproblems and optimal substructure
- Image processing is just 2D array manipulation — energy is a matrix, seams are paths, removal is shifting
- Different energy functions produce visually different results, making benchmarking meaningful
- Partial energy recalculation is a significant optimization in practice

