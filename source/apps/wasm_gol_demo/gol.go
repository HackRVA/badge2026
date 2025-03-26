package main

const (
	BUTTON_UP    = 0x1
	BUTTON_DOWN  = 0x2
	BUTTON_LEFT  = 0x4
	BUTTON_RIGHT = 0x8
	BUTTON_A     = 0x10
	BUTTON_B     = 0x20
)

const (
	gridWidth  = 30
	gridHeight = 20
	cellSize   = 8
)

var (
	grid         [gridHeight][gridWidth]bool
	nextGrid     [gridHeight][gridWidth]bool
	frameCount          = 0
	currentInput uint32 = 0
	isFirstRun          = true
)

func initGrid() {
	grid[1][2] = true
	grid[2][3] = true
	grid[3][1] = true
	grid[3][2] = true
	grid[3][3] = true
}

func countNeighbors(x, y int) int {
	count := 0
	for dy := -1; dy <= 1; dy++ {
		for dx := -1; dx <= 1; dx++ {
			if dx == 0 && dy == 0 {
				continue
			}
			nx, ny := x+dx, y+dy
			if nx >= 0 && nx < gridWidth && ny >= 0 && ny < gridHeight && grid[ny][nx] {
				count++
			}
		}
	}
	return count
}

func step() {
	for y := 0; y < gridHeight; y++ {
		for x := 0; x < gridWidth; x++ {
			neighbors := countNeighbors(x, y)
			if grid[y][x] {
				nextGrid[y][x] = neighbors == 2 || neighbors == 3
			} else {
				nextGrid[y][x] = neighbors == 3
			}
		}
	}

	for y := 0; y < gridHeight; y++ {
		for x := 0; x < gridWidth; x++ {
			grid[y][x] = nextGrid[y][x]
		}
	}
}

//go:export update
func update(buttonMask uint32) {
	if isFirstRun {
		isFirstRun = false
		initGrid()
	}
	currentInput = buttonMask
	frameCount++
	if frameCount%10 == 0 {
		step()
	}
	if currentInput&BUTTON_B != 0 {
		CloseApp()
	}
}

//go:export draw
func draw() {
	FbClear()

	for y := 0; y < gridHeight; y++ {
		for x := 0; x < gridWidth; x++ {
			if grid[y][x] {
				FbColor(PaletteGetColorFromIndex(8))
				FbMove(uint8(x*cellSize), uint8(y*cellSize))
				FbFilledRectangle(cellSize, cellSize)
			}
		}
	}

	FbSwapBuffers()
}

func main() {
}
