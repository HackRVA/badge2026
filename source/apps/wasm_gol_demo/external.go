package main

//go:wasm-module Fb
//go:export clear
func FbClear()

//go:wasm-module Fb
//go:export color
func FbColor(color uint16)

//go:wasm-module Fb
//go:export move
func FbMove(x, y uint8)

//go:wasm-module Fb
//go:export rectangle
func FbRectangle(width, height uint8)

//go:wasm-module Fb
//go:export circle
func FbCircle(x, y, r int32)

//go:wasm-module Fb
//go:export filledCircle
func FbFilledCircle(x, y, r int32)

//go:wasm-module Fb
//go:export filledRectangle
func FbFilledRectangle(width, height uint8)

//go:wasm-module Fb
//go:export roundedRectangle
func FbRoundedRectangle(width, height, stroke uint8)

//go:wasm-module Fb
//go:export swapBuffers
func FbSwapBuffers()

//go:wasm-module Palette
//go:export drawGrid
func PaletteDrawGrid(x, y, tileSize uint8)

//go:wasm-module Palette
//go:export getColor
func PaletteGetColor(index uint8) uint16

//go:wasm-module Palette
//go:export getColorFromIndex
func PaletteGetColorFromIndex(index uint8) uint16

//go:wasm-module App
//go:export closeApp
func CloseApp()
