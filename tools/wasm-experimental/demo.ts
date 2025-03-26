@external("Fb", "clear")
declare function FbClear(): void;

@external("Fb", "color")
declare function FbColor(color: u16): void;

@external("Fb", "move")
declare function FbMove(x: u8, y: u8): void;

@external("Fb", "rectangle")
declare function FbRectangle(width: u8, height: u8): void;

@external("Fb", "circle")
declare function FbCircle(x: i32, y: i32, r: i32): void;

@external("Fb", "filledCircle")
declare function FbFilledCircle(x: i32, y: i32, r: i32): void;

@external("Fb", "filledRectangle")
declare function FbFilledRectangle(width: u8, height: u8): void;

@external("Fb", "roundedRectangle")
declare function FbRoundedRectangle(width: u8, height: u8, stroke: u8): void;

@external("Fb", "swapBuffers")
declare function FbSwapBuffers(): void;

@external("Palette", "drawGrid")
declare function PaletteDrawGrid(x: u8, y: u8, tileSize: u8): void;

@external("Palette", "getColor")
declare function PaletteGetColor(index: u8): u16;

@external("Palette", "getColorFromIndex")
declare function PaletteGetColorFromIndex(index: u8): u16;

const SCREEN_WIDTH: u8 = 128;
const SCREEN_HEIGHT: u8 = 160;

const BUTTON_STATE_ADDR: usize = 0x0000;

const BUTTON_UP: u32 = 0x1;
const BUTTON_DOWN: u32 = 0x2;
const BUTTON_LEFT: u32 = 0x4;
const BUTTON_RIGHT: u32 = 0x8;
const BUTTON_A: u32 = 0x10;
const BUTTON_B: u32 = 0x20;

let currentInput: u32 = 0;
//function isButtonPressed(buttonMask: u32): bool {
//  return (load<u32>(BUTTON_STATE_ADDR) & buttonMask) != 0;
//}

class Rectangle {
  x: u8;
  y: u8;
  width: u8;
  height: u8;
  vx: i8;
  vy: i8;
  color: u16;

  constructor(x: u8, y: u8, width: u8, height: u8, color: u16, vx: i8, vy: i8) {
    this.x = x;
    this.y = y;
    this.width = width;
    this.height = height;
    this.color = color;
    this.vx = vx;
    this.vy = vy;
  }

  update(): void {
    this.x += this.vx;
    this.y += this.vy;

    if (this.x <= 0 || this.x + this.width >= SCREEN_WIDTH) this.vx *= -1;
    if (this.y <= 0 || this.y + this.height >= SCREEN_HEIGHT) this.vy *= -1;
  }

  render(): void {
    FbColor(this.color);
    FbMove(this.x, this.y);
    FbRectangle(this.width, this.height);
  }

  renderFilled(): void {
    FbColor(this.color);
    FbMove(this.x, this.y);
    FbFilledRectangle(this.width, this.height);
  }
}

class Circle {
  x: u8;
  y: u8;
  radius: u8;
  vx: i8;
  vy: i8;
  color: u16;

  constructor(x: u8, y: u8, radius: u8, color: u16, vx: i8, vy: i8) {
    this.x = x;
    this.y = y;
    this.radius = radius;
    this.color = color;
    this.vx = vx;
    this.vy = vy;
  }

  update(): void {
    this.x += this.vx;
    this.y += this.vy;

    if (this.x - this.radius <= 0 || this.x + this.radius >= SCREEN_WIDTH) this.vx *= -1;
    if (this.y - this.radius <= 0 || this.y + this.radius >= SCREEN_HEIGHT) this.vy *= -1;
  }

  render(): void {
    FbColor(this.color);
    FbMove(this.x, this.y);
    FbCircle(this.x, this.y, this.radius);
  }
  renderFilled(): void {
    FbColor(this.color);
    FbMove(this.x, this.y);
    FbFilledCircle(this.x, this.y, this.radius);
  }
}

//const red: u16 = 0b1111100000000000;
//const green: u16 = 0b0000011111100000;
//const blue: u16 = 0b0000000000011111;
//const yellow: u16 = 0b1111111111100000;
//const purple: u16 = 0b1111100000011111;
//const cyan: u16 = 0b0000011111111111;
//const white: u16 = 0b1111111111111111;
//const orange: u16 = 0b1111111000000000;

let rect1 = new Rectangle(10, 20, 30, 30, PaletteGetColorFromIndex(0), 1, 1);
let rect2 = new Rectangle(50, 50, 20, 20, PaletteGetColorFromIndex(1), -1, 1);
let rect3 = new Rectangle(90, 70, 25, 25, PaletteGetColorFromIndex(2), 2, -1);
let rect4 = new Rectangle(30, 90, 15, 15, PaletteGetColorFromIndex(3), 1, -2);
let rect5 = new Rectangle(20, 40, 25, 25, PaletteGetColorFromIndex(4), 1, 1);
let rect6 = new Rectangle(60, 20, 15, 15, PaletteGetColorFromIndex(5), -2, 1);
let rect7 = new Rectangle(70, 30, 20, 20, PaletteGetColorFromIndex(6), 1, -1);
let rect8 = new Rectangle(40, 60, 30, 30, PaletteGetColorFromIndex(7), -1, -1);

let circle1 = new Circle(80, 80, 15, PaletteGetColorFromIndex(8), 1, -1);
let circle2 = new Circle(40, 40, 10, PaletteGetColorFromIndex(9), -1, 1);
let circle3 = new Circle(60, 100, 12, PaletteGetColorFromIndex(10), 2, 2);
let circle4 = new Circle(50, 120, 18, PaletteGetColorFromIndex(11), -1, -1);
let circle5 = new Circle(30, 30, 20, PaletteGetColorFromIndex(12), 1, 1);
let circle6 = new Circle(90, 50, 15, PaletteGetColorFromIndex(13), -1, 2);

export function update(buttonMask: u32): void {
  currentInput = buttonMask;

  if ((currentInput & BUTTON_RIGHT) == 0) {
    rect1.update();
    rect2.update();
    rect3.update();
    rect4.update();
    rect5.update();
    rect6.update();
    rect7.update();
    rect8.update();
  }
  if ((currentInput & BUTTON_LEFT) == 0) {
    circle1.update();
    circle2.update();
    circle3.update();
    circle4.update();
    circle5.update();
    circle6.update();
  }
}

export function draw(): void {
  FbClear();
  PaletteDrawGrid(0, 0, 8);

  const stroke: u8 = 4
  FbMove(80 + stroke, 50 + stroke);
  FbColor(PaletteGetColorFromIndex(1));
  FbFilledRectangle(40 - stroke * 2, 40 - stroke * 2);
  FbMove(80, 50);
  FbColor(PaletteGetColorFromIndex(2));
  FbRoundedRectangle(40, 40, stroke);

  if ((currentInput & BUTTON_DOWN) == 0) {
    rect1.renderFilled();
    rect2.renderFilled();
    rect3.renderFilled();
    rect4.renderFilled();
    rect5.renderFilled();
    rect6.renderFilled();
    rect7.renderFilled();
    rect8.renderFilled();
  }

  if ((currentInput & BUTTON_UP) == 0) {
    circle1.renderFilled();
    circle2.renderFilled();
    circle3.renderFilled();
    circle4.renderFilled();
    circle5.renderFilled();
    circle6.renderFilled();
  }

  FbSwapBuffers();
}

