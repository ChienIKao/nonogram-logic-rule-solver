# Nonogram — Logic-Rule-Based Solver

以邏輯規則推導取代盲目搜尋的 Nonogram 求解器。核心想法是：在進入代價高昂的回溯搜尋之前，
盡可能用邏輯規則確定格點狀態，並儘早偵測出矛盾盤面。

> **Chien-I Kao** and **Kuo-Chan Huang**, *A New Logic-Rule-Based Method for Solving Nonograms*,
> 2025 Taiwan Computer Game Association (TCGA) Workshop on Computer Games, Taiwan, May 2025.
> **🏆 2025 佳作論文獎（Honorable Mention Paper Award），台灣電腦對局學會**

本研究為國科會大專學生研究計畫 `113-2813-C-142-002-E` 的成果，計畫主持人：高健壹。

---

## Problem

Nonogram（又稱 Paint by Numbers、Hanjie）是 NP-Complete 問題。求解流程通常分兩階段：

1. **Line solving / Propagation** — 對每一行每一列反覆推導可確定的格點，直到無法再推進
2. **Backtracking** — 邏輯推導窮盡後，只能猜點並回溯

第二階段的代價遠高於第一階段。因此關鍵在於：**第一階段能確定多少格點，以及能多早判定一個盤面無解。**
本專案針對這兩件事重新設計邏輯規則。

---

## Method

### 極左極右界（Leftmost / Rightmost）

對每條線的每個線索，維護它在合法排列下可能落在的最左與最右位置。
兩者收斂重疊處即為可確定的格點。

| 函式 | 作用 |
|---|---|
| `RLmost_init()` | 初始化每條線所有線索的左右界 |
| `RLmost()` | 依當前盤面收斂左右界 |
| `Update_leftmost()` / `Update_rightmost()` | 單一線索的界線更新 |

### 矛盾偵測（Conflict Rules）

在推導過程中即時判定盤面無解，避免無謂的深入搜尋：

**RLmost 階段**
- `Total > 25 - start`（Total 為含間隔的線索總長）
- `start + 1 < Total`（Total 累計至當前線索）
- 線索長度與可用區間不符
- 已填格點無法對應到同一個線索

**Leftmost / Rightmost 階段**
- 已到最後一格，但線索尚未配置完畢

### 傳播與快取

`Propagate()` 以 64-bit bitboard 表示每條線，用 `__builtin_ffsll` 走訪待更新的線，
並以 hash 表快取「線索 + 線狀態 → 推導結果」，避免重複計算相同的 line solving。

盤面為 25×25，因此 50 條線（25 列 + 25 行）剛好可用單一 `uint64_t` 的位元遮罩追蹤。

---

## Project Structure

```text
.
├── src/
│   ├── logicSolve.cpp    # 邏輯規則推導與矛盾偵測（本研究主體，約 1,170 行）
│   ├── solver.cpp        # 求解流程控制與計時
│   ├── fullyprobe.cpp    # Fully Probing
│   ├── hash.cpp          # line solving 結果快取
│   ├── board.cpp         # 盤面表示與 bitboard 操作
│   ├── fileManager.cpp   # 題目讀取與盤面輸出
│   ├── helper.cpp        # 命令列參數
│   └── main.cpp
├── include/
├── input.txt             # 1,000 題 25×25 測資
└── Makefile
```

---

## Build & Run

需求：`g++`（C++11）、GNU Make。編譯選項為 `-O3`。

```bash
make
```

```bash
bin/nonogram
```

預設求解 `input.txt` 的第 1 至 1000 題，輸出盤面到 `output.txt`，逐題統計寫入 `log.txt`。

### 參數

| 參數 | 說明 | 預設 |
|---|---|---|
| `-S`, `--start` | 起始題號 | 1 |
| `-E`, `--end` | 結束題號 | 1000 |
| `-I`, `--input` | 輸入檔 | `input.txt` |
| `-O`, `--output` | 輸出檔 | `output.txt` |
| `-L`, `--log` | 記錄檔 | `log.txt` |
| `-M`, `--method` | 猜點策略（1–7） | — |
| `--show-config` | 顯示目前設定 | — |

```bash
bin/nonogram --start 1 --end 100 --input input.txt
```

程式會印出每題耗時、平均確定格點數與總時間。

### 輸入格式

每題以 `$編號` 起始，接著 25 行列線索、25 行行線索，每行的數字以 tab 分隔：

```text
$1
1	2	2	2	1	1	1	1	1
1	2	1	7	2	1
...
```

---

## Attribution

`logicSolve.cpp` 的邏輯規則推導與矛盾偵測是本研究的貢獻。

其餘模組建立在國立臺中教育大學高效能計算實驗室的共用 Nonogram 程式基礎之上，
其中 `hash.cpp` 與共用版本幾乎相同，`board.cpp`、`fullyprobe.cpp` 則有不同程度的改寫。
共用基礎由實驗室歷屆成員累積而成，在此致謝。

Fully Probing 的概念參考：

> I-Chen Wu, Der-Johng Sun, Lung-Ping Chen, Kan-Yueh Chen, Ching-Hua Kuo, Hao-Hua Kang,
> and Hung-Hsuan Lin, *An Efficient Approach to Solving Nonograms*,
> IEEE Transactions on Computational Intelligence and AI in Games, vol. 5, no. 3, Sept. 2013.
