import { useState, useRef, useCallback, useEffect } from "react";

// ─── Types ────────────────────────────────────────────────────────────────────
interface Pixel { r: number; g: number; b: number; }
type Grid = Pixel[][];

// ─── Algorithm ────────────────────────────────────────────────────────────────
function imageDataToGrid(data: Uint8ClampedArray, w: number, h: number): Grid {
  const grid: Grid = [];
  for (let y = 0; y < h; y++) {
    grid[y] = [];
    for (let x = 0; x < w; x++) {
      const i = (y * w + x) * 4;
      grid[y][x] = { r: data[i], g: data[i + 1], b: data[i + 2] };
    }
  }
  return grid;
}

function gridToImageData(grid: Grid, w: number, h: number): Uint8ClampedArray {
  const data = new Uint8ClampedArray(w * h * 4);
  for (let y = 0; y < h; y++) {
    for (let x = 0; x < w; x++) {
      const i = (y * w + x) * 4;
      data[i]     = grid[y][x].r;
      data[i + 1] = grid[y][x].g;
      data[i + 2] = grid[y][x].b;
      data[i + 3] = 255;
    }
  }
  return data;
}

function computeEnergy(grid: Grid, w: number, h: number, mask?: boolean[][]): number[][] {
  const energy: number[][] = [];
  for (let y = 0; y < h; y++) {
    energy[y] = [];
    for (let x = 0; x < w; x++) {
      if (mask && mask[y]?.[x]) { energy[y][x] = -1e9; continue; }
      const left  = grid[y][Math.max(x - 1, 0)];
      const right = grid[y][Math.min(x + 1, w - 1)];
      const up    = grid[Math.max(y - 1, 0)][x];
      const down  = grid[Math.min(y + 1, h - 1)][x];
      const dx = (right.r-left.r)**2 + (right.g-left.g)**2 + (right.b-left.b)**2;
      const dy = (down.r-up.r)**2   + (down.g-up.g)**2   + (down.b-up.b)**2;
      energy[y][x] = Math.sqrt(dx + dy);
    }
  }
  return energy;
}

function findSeam(energy: number[][], w: number, h: number): number[] {
  const dp: number[][] = Array.from({ length: h }, () => new Array(w).fill(Infinity));
  const parent: number[][] = Array.from({ length: h }, () => new Array(w).fill(-1));
  for (let x = 0; x < w; x++) dp[0][x] = energy[0][x];
  for (let y = 1; y < h; y++) {
    for (let x = 0; x < w; x++) {
      let best = dp[y-1][x], bestX = x;
      if (x > 0     && dp[y-1][x-1] < best) { best = dp[y-1][x-1]; bestX = x-1; }
      if (x < w - 1 && dp[y-1][x+1] < best) { best = dp[y-1][x+1]; bestX = x+1; }
      dp[y][x] = energy[y][x] + best;
      parent[y][x] = bestX;
    }
  }
  let minX = 0;
  for (let x = 1; x < w; x++) if (dp[h-1][x] < dp[h-1][minX]) minX = x;
  const seam = new Array(h);
  seam[h-1] = minX;
  for (let y = h - 2; y >= 0; y--) seam[y] = parent[y+1][seam[y+1]];
  return seam;
}

function removeSeam(grid: Grid, seam: number[], w: number, h: number, mask?: boolean[][]): { grid: Grid; mask?: boolean[][] } {
  const newGrid: Grid = [];
  const newMask: boolean[][] | undefined = mask ? [] : undefined;
  for (let y = 0; y < h; y++) {
    newGrid[y] = [];
    if (newMask) newMask[y] = [];
    for (let x = 0; x < w; x++) {
      if (x === seam[y]) continue;
      newGrid[y].push(grid[y][x]);
      if (newMask && mask) newMask[y].push(mask[y][x]);
    }
  }
  return { grid: newGrid, mask: newMask };
}

function transpose(grid: Grid, w: number, h: number): Grid {
  const t: Grid = [];
  for (let x = 0; x < w; x++) {
    t[x] = [];
    for (let y = 0; y < h; y++) t[x][y] = grid[y][x];
  }
  return t;
}

function hasMaskedPixels(mask: boolean[][]): boolean {
  return mask.some(row => row.some(v => v));
}

// ─── App ──────────────────────────────────────────────────────────────────────
type Mode = "resize" | "remove";

export default function App() {
  const [originalImage, setOriginalImage] = useState<HTMLImageElement | null>(null);
  const [originalUrl, setOriginalUrl]     = useState("");
  const [carvedUrl, setCarvedUrl]         = useState("");
  const [vertSeams, setVertSeams]         = useState(100);
  const [horizSeams, setHorizSeams]       = useState(0);
  const [loading, setLoading]             = useState(false);
  const [progress, setProgress]           = useState(0);
  const [origDims, setOrigDims]           = useState({ w: 0, h: 0 });
  const [newDims, setNewDims]             = useState({ w: 0, h: 0 });
  const [mode, setMode]                   = useState<Mode>("resize");
  const [drawing, setDrawing]             = useState(false);
  const [brushSize, setBrushSize]         = useState(20);

  const fileRef    = useRef<HTMLInputElement>(null);
  const canvasRef  = useRef<HTMLCanvasElement>(null);
  const maskRef    = useRef<boolean[][]>([]);
  const isDrawing  = useRef(false);

  // draw image + red mask overlay on canvas
  const redrawCanvas = useCallback(() => {
    const canvas = canvasRef.current;
    const img = originalImage;
    if (!canvas || !img) return;
    const ctx = canvas.getContext("2d")!;
    ctx.drawImage(img, 0, 0, canvas.width, canvas.height);
    const mask = maskRef.current;
    if (mask.length === 0) return;
    const scaleX = canvas.width / img.width;
    const scaleY = canvas.height / img.height;
    const imgData = ctx.getImageData(0, 0, canvas.width, canvas.height);
    for (let y = 0; y < img.height; y++) {
      for (let x = 0; x < img.width; x++) {
        if (!mask[y]?.[x]) continue;
        const cx = Math.floor(x * scaleX);
        const cy = Math.floor(y * scaleY);
        const i = (cy * canvas.width + cx) * 4;
        imgData.data[i]     = 255;
        imgData.data[i + 1] = 0;
        imgData.data[i + 2] = 0;
        imgData.data[i + 3] = 160;
      }
    }
    ctx.putImageData(imgData, 0, 0);
  }, [originalImage]);

  useEffect(() => {
    if (mode === "remove" && originalImage) redrawCanvas();
  }, [mode, originalImage, redrawCanvas]);

  const onUpload = useCallback((e: React.ChangeEvent<HTMLInputElement>) => {
    const file = e.target.files?.[0];
    if (!file) return;
    const url = URL.createObjectURL(file);
    const img = new Image();
    img.onload = () => {
      setOriginalImage(img);
      setOriginalUrl(url);
      setCarvedUrl("");
      setOrigDims({ w: img.width, h: img.height });
      setNewDims({ w: img.width, h: img.height });
      maskRef.current = Array.from({ length: img.height }, () => new Array(img.width).fill(false));
    };
    img.src = url;
  }, []);

  const paintMask = useCallback((e: React.MouseEvent<HTMLCanvasElement>) => {
    if (!isDrawing.current || !originalImage) return;
    const canvas = canvasRef.current!;
    const rect   = canvas.getBoundingClientRect();
    const scaleX = originalImage.width  / canvas.width;
    const scaleY = originalImage.height / canvas.height;
    const cx = e.clientX - rect.left;
    const cy = e.clientY - rect.top;
    const ix = Math.floor(cx * scaleX);
    const iy = Math.floor(cy * scaleY);
    const r  = Math.ceil(brushSize * scaleX / 2);
    const mask = maskRef.current;
    for (let dy = -r; dy <= r; dy++) {
      for (let dx = -r; dx <= r; dx++) {
        const nx = ix + dx, ny = iy + dy;
        if (nx >= 0 && nx < originalImage.width && ny >= 0 && ny < originalImage.height) {
          if (dx*dx + dy*dy <= r*r) mask[ny][nx] = true;
        }
      }
    }
    redrawCanvas();
  }, [originalImage, brushSize, redrawCanvas]);

  const clearMask = () => {
    if (!originalImage) return;
    maskRef.current = Array.from({ length: originalImage.height }, () => new Array(originalImage.width).fill(false));
    redrawCanvas();
  };

  const onCarve = useCallback(async () => {
    if (!originalImage) return;
    setLoading(true);
    setProgress(0);
    setCarvedUrl("");
    await new Promise(r => setTimeout(r, 50));

    const offscreen = document.createElement("canvas");
    offscreen.width  = originalImage.width;
    offscreen.height = originalImage.height;
    const ctx = offscreen.getContext("2d")!;
    ctx.drawImage(originalImage, 0, 0);
    const imageData = ctx.getImageData(0, 0, originalImage.width, originalImage.height);

    let grid = imageDataToGrid(imageData.data, originalImage.width, originalImage.height);
    let cw = originalImage.width;
    let ch = originalImage.height;

    if (mode === "remove") {
      // object removal: keep removing until no masked pixels left
      let mask = maskRef.current.map(row => [...row]);
      let iter = 0;
      while (hasMaskedPixels(mask) && cw > 1) {
        const energy = computeEnergy(grid, cw, ch, mask);
        const seam   = findSeam(energy, cw, ch);
        const result = removeSeam(grid, seam, cw, ch, mask);
        grid = result.grid;
        mask = result.mask!;
        cw--;
        iter++;
        if (iter % 5 === 0) {
          setProgress(Math.min(99, iter));
          await new Promise(r => setTimeout(r, 0));
        }
      }
    } else {
      // resize mode
      const chunkSize = 10;
      // vertical seams
      for (let done = 0; done < vertSeams; done += chunkSize) {
        const batch = Math.min(chunkSize, vertSeams - done);
        for (let i = 0; i < batch; i++) {
          const energy = computeEnergy(grid, cw, ch);
          const seam   = findSeam(energy, cw, ch);
          const result = removeSeam(grid, seam, cw, ch);
          grid = result.grid;
          cw--;
        }
        setProgress(Math.round(((done + batch) / (vertSeams + horizSeams)) * 100));
        await new Promise(r => setTimeout(r, 0));
      }
      // horizontal seams — transpose, carve, transpose back
      if (horizSeams > 0) {
        grid = transpose(grid, cw, ch);
        let tmp = cw; cw = ch; ch = tmp;
        for (let done = 0; done < horizSeams; done += chunkSize) {
          const batch = Math.min(chunkSize, horizSeams - done);
          for (let i = 0; i < batch; i++) {
            const energy = computeEnergy(grid, cw, ch);
            const seam   = findSeam(energy, cw, ch);
            const result = removeSeam(grid, seam, cw, ch);
            grid = result.grid;
            cw--;
          }
          setProgress(Math.round(((vertSeams + done + batch) / (vertSeams + horizSeams)) * 100));
          await new Promise(r => setTimeout(r, 0));
        }
        grid = transpose(grid, cw, ch);
        tmp = cw; cw = ch; ch = tmp;
      }
    }

    const out = document.createElement("canvas");
    out.width = cw; out.height = ch;
    const outCtx = out.getContext("2d")!;
    const outData = outCtx.createImageData(cw, ch);
    outData.data.set(gridToImageData(grid, cw, ch));
    outCtx.putImageData(outData, 0, 0);
    setCarvedUrl(out.toDataURL());
    setNewDims({ w: cw, h: ch });
    setLoading(false);
    setProgress(100);
  }, [originalImage, mode, vertSeams, horizSeams]);

  const onDownload = () => {
    const a = document.createElement("a");
    a.href = carvedUrl;
    a.download = "seam_carved.png";
    a.click();
  };

  const maxVert  = originalImage ? Math.floor(originalImage.width  * 0.6) : 100;
  const maxHoriz = originalImage ? Math.floor(originalImage.height * 0.6) : 100;

  return (
    <div style={s.root}>
      <header style={s.header}>
        <div style={s.pill}>SIGGRAPH 2007 · Avidan & Shamir</div>
        <h1 style={s.title}>Seam Carving</h1>
        <p style={s.subtitle}>
          Content-aware image resizing — removes the least important pixels,<br />
          preserving what matters.
        </p>
      </header>

      <main style={s.main}>
        {/* Upload */}
        <div style={s.dropzone} onClick={() => fileRef.current?.click()}>
          <input ref={fileRef} type="file" accept="image/*" onChange={onUpload} style={{ display: "none" }} />
          {originalUrl
            ? <span style={s.dropText}>✓ Image loaded — click to change</span>
            : <>
                <span style={s.dropIcon}>↑</span>
                <span style={s.dropText}>Upload an image</span>
                <span style={s.dropHint}>PNG, JPG, WEBP</span>
              </>
          }
        </div>

        {originalImage && (
          <>
            {/* Mode tabs */}
            <div style={s.tabs}>
              <button style={{ ...s.tab, ...(mode === "resize" ? s.tabActive : {}) }} onClick={() => setMode("resize")}>Resize</button>
              <button style={{ ...s.tab, ...(mode === "remove" ? s.tabActive : {}) }} onClick={() => setMode("remove")}>Object Removal</button>
            </div>

            {mode === "resize" && (
              <div style={s.controls}>
                <div style={s.sliderRow}>
                  <label style={s.label}>Width reduction <span style={s.val}>{vertSeams}px</span></label>
                  <input type="range" min={0} max={maxVert} value={vertSeams}
                    onChange={e => setVertSeams(Number(e.target.value))} style={s.slider} disabled={loading} />
                  <div style={s.sliderMeta}><span>0px</span><span>{maxVert}px max</span></div>
                </div>
                <div style={s.sliderRow}>
                  <label style={s.label}>Height reduction <span style={s.val}>{horizSeams}px</span></label>
                  <input type="range" min={0} max={maxHoriz} value={horizSeams}
                    onChange={e => setHorizSeams(Number(e.target.value))} style={s.slider} disabled={loading} />
                  <div style={s.sliderMeta}><span>0px</span><span>{maxHoriz}px max</span></div>
                </div>
                <div style={s.dimPreview}>
                  {origDims.w} × {origDims.h} → {origDims.w - vertSeams} × {origDims.h - horizSeams}
                </div>
              </div>
            )}

            {mode === "remove" && (
              <div style={s.controls}>
                <p style={s.hint}>Paint over the object you want removed. Seams will carve through it.</p>
                <div style={s.sliderRow}>
                  <label style={s.label}>Brush size <span style={s.val}>{brushSize}px</span></label>
                  <input type="range" min={5} max={80} value={brushSize}
                    onChange={e => setBrushSize(Number(e.target.value))} style={s.slider} />
                </div>
                <div style={s.canvasWrap}>
                  <canvas
                    ref={canvasRef}
                    width={Math.min(originalImage.width, 800)}
                    height={Math.round(originalImage.height * Math.min(800, originalImage.width) / originalImage.width)}
                    style={{ ...s.maskCanvas, cursor: drawing ? "crosshair" : "default" }}
                    onMouseDown={() => { isDrawing.current = true; setDrawing(true); }}
                    onMouseUp={() => { isDrawing.current = false; setDrawing(false); }}
                    onMouseLeave={() => { isDrawing.current = false; setDrawing(false); }}
                    onMouseMove={paintMask}
                  />
                </div>
                <button style={s.clearBtn} onClick={clearMask}>Clear mask</button>
              </div>
            )}

            {/* Carve button */}
            <button style={{ ...s.carveBtn, opacity: loading ? 0.6 : 1 }} onClick={onCarve} disabled={loading}>
              {loading ? `Carving… ${progress}%` : mode === "remove" ? "Remove Object" : "Carve Image"}
            </button>

            {loading && (
              <div style={s.progressWrap}>
                <div style={{ ...s.progressBar, width: `${progress}%` }} />
              </div>
            )}
          </>
        )}

        {/* Before / After */}
        {(originalUrl || carvedUrl) && (
          <div style={s.comparison}>
            {originalUrl && (
              <div style={s.imageCard}>
                <div style={s.cardLabel}>Original</div>
                <img src={originalUrl} style={s.img} alt="original" />
                <div style={s.cardMeta}>{origDims.w} × {origDims.h}</div>
              </div>
            )}
            {carvedUrl && (
              <div style={s.imageCard}>
                <div style={s.cardLabel}>{mode === "remove" ? "Object Removed" : "Seam Carved"}</div>
                <img src={carvedUrl} style={s.img} alt="result" />
                <div style={s.cardMeta}>{newDims.w} × {newDims.h}</div>
              </div>
            )}
          </div>
        )}

        {carvedUrl && (
          <button style={s.downloadBtn} onClick={onDownload}>Download result</button>
        )}
      </main>

      <footer style={s.footer}>
        Built with C++ · Ported to TypeScript · Dynamic Programming · O(W×H) per seam
      </footer>
    </div>
  );
}

// ─── Styles ───────────────────────────────────────────────────────────────────
const s: Record<string, React.CSSProperties> = {
  root: { minHeight: "100vh", background: "#0a0a0a", color: "#f0f0f0", fontFamily: "'Inter','Helvetica Neue',sans-serif", display: "flex", flexDirection: "column", alignItems: "center" },
  header: { textAlign: "center", padding: "64px 24px 40px", maxWidth: 640 },
  pill: { display: "inline-block", fontSize: 11, letterSpacing: "0.12em", textTransform: "uppercase", color: "#888", border: "1px solid #333", borderRadius: 100, padding: "4px 12px", marginBottom: 20 },
  title: { fontSize: "clamp(2.4rem,6vw,4rem)", fontWeight: 700, letterSpacing: "-0.03em", margin: "0 0 16px", lineHeight: 1.1 },
  subtitle: { fontSize: 15, color: "#888", lineHeight: 1.7, margin: 0 },
  main: { width: "100%", maxWidth: 900, padding: "0 24px 80px", display: "flex", flexDirection: "column", alignItems: "center", gap: 28 },
  dropzone: { width: "100%", border: "1px dashed #333", borderRadius: 12, padding: "48px 24px", display: "flex", flexDirection: "column", alignItems: "center", gap: 8, cursor: "pointer" },
  dropIcon: { fontSize: 32, color: "#555" },
  dropText: { fontSize: 15, color: "#ccc" },
  dropHint: { fontSize: 12, color: "#555" },
  tabs: { display: "flex", gap: 4, background: "#111", borderRadius: 10, padding: 4 },
  tab: { background: "transparent", border: "none", color: "#666", padding: "8px 20px", borderRadius: 8, cursor: "pointer", fontSize: 13, fontWeight: 500, transition: "all 0.15s" },
  tabActive: { background: "#1e1e1e", color: "#fff" },
  controls: { width: "100%", display: "flex", flexDirection: "column", gap: 16, alignItems: "center" },
  sliderRow: { width: "100%", maxWidth: 500, display: "flex", flexDirection: "column", gap: 8 },
  label: { fontSize: 13, color: "#aaa", display: "flex", justifyContent: "space-between" },
  val: { color: "#fff", fontVariantNumeric: "tabular-nums" },
  slider: { width: "100%", accentColor: "#fff", cursor: "pointer" },
  sliderMeta: { display: "flex", justifyContent: "space-between", fontSize: 11, color: "#555" },
  dimPreview: { fontSize: 12, color: "#666", fontVariantNumeric: "tabular-nums" },
  hint: { fontSize: 13, color: "#666", textAlign: "center", maxWidth: 400 },
  canvasWrap: { width: "100%", display: "flex", justifyContent: "center" },
  maskCanvas: { borderRadius: 8, border: "1px solid #222", maxWidth: "100%" },
  clearBtn: { background: "transparent", border: "1px solid #333", color: "#666", borderRadius: 6, padding: "6px 16px", fontSize: 12, cursor: "pointer" },
  carveBtn: { background: "#fff", color: "#000", border: "none", borderRadius: 8, padding: "12px 36px", fontSize: 14, fontWeight: 600, cursor: "pointer", letterSpacing: "0.01em" },
  progressWrap: { width: "100%", height: 2, background: "#222", borderRadius: 1, overflow: "hidden" },
  progressBar: { height: "100%", background: "#fff", transition: "width 0.3s ease" },
  comparison: { width: "100%", display: "grid", gridTemplateColumns: "1fr 1fr", gap: 16 },
  imageCard: { display: "flex", flexDirection: "column", gap: 8 },
  cardLabel: { fontSize: 11, letterSpacing: "0.1em", textTransform: "uppercase", color: "#666" },
  img: { width: "100%", borderRadius: 8, display: "block", border: "1px solid #1a1a1a" },
  cardMeta: { fontSize: 12, color: "#555", fontVariantNumeric: "tabular-nums" },
  downloadBtn: { background: "transparent", color: "#888", border: "1px solid #333", borderRadius: 8, padding: "10px 28px", fontSize: 13, cursor: "pointer" },
  footer: { marginTop: "auto", padding: 24, fontSize: 11, color: "#444", letterSpacing: "0.05em", textAlign: "center" },
};