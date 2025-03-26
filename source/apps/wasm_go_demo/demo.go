package main

const (
	BUTTON_UP    = 0x1
	BUTTON_DOWN  = 0x2
	BUTTON_LEFT  = 0x4
	BUTTON_RIGHT = 0x8
	BUTTON_A     = 0x10
	BUTTON_B     = 0x20
)

var currentInput uint32 = 0

//go:export update
func update(buttonMask uint32) {
	currentInput = buttonMask
	if currentInput&BUTTON_B != 0 {
		CloseApp()
	}
}

//go:export draw
func draw() {
	FbClear()
	PaletteDrawGrid(0, 0, 8)

	FbMove(80, 50)
	FbColor(PaletteGetColorFromIndex(2))
	FbRoundedRectangle(40, 40, 4)

	if currentInput&BUTTON_UP != 0 {
		FbColor(PaletteGetColorFromIndex(8))
		FbFilledCircle(64, 20, 10)
	}
	if currentInput&BUTTON_DOWN != 0 {
		FbColor(PaletteGetColorFromIndex(9))
		FbFilledCircle(64, 140, 10)
	}
	if currentInput&BUTTON_LEFT != 0 {
		FbColor(PaletteGetColorFromIndex(10))
		FbFilledCircle(20, 80, 10)
	}
	if currentInput&BUTTON_RIGHT != 0 {
		FbColor(PaletteGetColorFromIndex(11))
		FbFilledCircle(108, 80, 10)
	}
	if currentInput&BUTTON_A != 0 {
		FbColor(PaletteGetColorFromIndex(12))
		FbFilledCircle(30, 30, 8)
	}
	if currentInput&BUTTON_B != 0 {
		FbColor(PaletteGetColorFromIndex(13))
		FbFilledCircle(98, 130, 8)
	}

	FbSwapBuffers()
}

func main() {}
