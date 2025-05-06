# badge2025
RVASec Badge 2025 Firmware

# Initial Setup

There are two ways you can run the badge software:

1) You can build the badge software and run it on hardware. This is helpful when seeing how well something works on the
   badge itself.
2) You can build a simulator that can run the badge software on your computer. This is *usually* more helpful when
   developing an app, since debugging tools on your computer are more sophisticated and easier to set up.

They need a few different things in order to set yourself up to build and run the badge software.

In both cases, you will need **CMake**, which is a build system / build system generator. (Version 3.13 or higher)

## Simulator

The simulator is intended to run on a Posix-y (that is, Linux or Mac) environment. Windows can build and run it,
although by using Windows Subsystem for Linux if your Linux subsystem has a window manager (e.g. X11).

To build the simulator, you will need a C compiler for your computer ("apt-get install build-essential"
on Debian based distros).  The simulator relies on SDL2
for graphics and keyboard/mouse/game controller support, so you will need to install SDL2.  For images,
libpng is needed.  ("apt-get install libsdl2-dev" package on Debian based distros, libpng-dev is usually already
present, on Mac, "brew install sdl2" and "brew install libpng").

### Prereqs

<details>
<summary>Linux</summary>

```bash
apt install build-essential
apt install cmake
apt install libsdl2-dev
apt install libpng-dev
```

</details>
<details>
<summary>Mac</summary>

```bash
brew install gcc
brew install cmake
brew install sdl2
brew install libpng
```

Make sure you have the directories that brew installs to in your `PATH` environment variable.

</details>

<details>
<summary>Windows</summary>

The simulator can run on windows through [wsl](https://learn.microsoft.com/en-us/windows/wsl/install) using [wslg](https://learn.microsoft.com/en-us/windows/wsl/tutorials/gui-apps)

> note: I think wslg is installed by default with wsl2

Once you have wsl setup, you should mostly follow the linux instructions.

You will likely need X11 (you don't necessarily need to run an entire desktop environment).

You can check if you can run graphical apps from wsl by installing `x11-apps`.

e.g.

```bash
apt install x11-apps
xeyes
```

</details>

### Running the Simulator

To generate the build directory:

```bash
bash ./run_cmake_sdl_sim.sh
```

This will create a directory called `build_sdl_sim`

```bash
cd build_sdl_sim
```

```bash
make
./source/badge2025_c
```

> note: when iterating on code, you only need to rerun the `make` command.  This will rebuild the binary with your new changes.



# Building for Hardware

The firmware build requires a git submodule:

```bash
git submodule update --init --recursive
```

> note the git submodule not required for building the simulator

For more info, see the [Pico SDK README](https://github.com/raspberrypi/pico-sdk).

When building the software for badge, you will need a *cross-compiler*. This takes code written on your computer and
compiles it into machine code for the badge. The compiler used is the ARM Embedded GCC compiler.

This may be available in your package manager (maybe called
"gcc-arm-none-eabi"), or
[here](https://developer.arm.com/tools-and-software/open-source-software/developer-tools/gnu-toolchain/gnu-rm/downloads).
If downloaded from the ARM site, you will need to take steps to add it to your
`$PATH` or equivalent environment
variable. (If it works, when you open a new terminal window, the
`arm-none-eabi-gcc` program can be run).

## Flashing the Badge

To flash your firmware to the badge, press and hold the small white button just to
the right of the screen on the badge, and connect the badge via micro usb cable to
your computer and release the button.  This will cause the badge to act as a USB
storage device, and you should see a filesystem mounted on your computer.  On linux this
will typically appear at `/media/*username*/RPI-RP2`. Copy the firmware to this
location:

```bash
cp source/badge2025_c.uf2 /media/$USER/RPI-RP2/
```

> note: on Windows WSL, you can run `explorer.exe .` from within linux to launch a file explorer with your linux dir opened. From here, you can copy the uf2 file to the mounted RPI-RP2 external media.

## Windows

[This link](https://community.element14.com/products/raspberry-pi/b/blog/posts/working-with-the-raspberry-pi-pico-with-windows-and-c-c)
has a bunch of useful information for getting started and installing prerequisites. Note that you don't need to do the
`PICO_SDK_PATH` setting portion, and when running and building this repository, you will want to use "NMake Makefiles"
instead of "Unix Makefiles" (unless you want to install and use `make` as well).

You can use Ninja, if you like, as well. \(Specify `-G Ninja` instead of Makefiles in the `cmake` command.\)

# Adding Your Own Apps

Apps are mostly contained within a single .c/.h file in the apps folder. Take a look at the comments inside the
`badge-app-template` files for help getting started.  See also
[BADGE-APP-HOWTO.md](https://github.com/HackRVA/badge2025/blob/main/BADGE-APP-HOWTO.md)

# Dir Layout

The overall structure of the repository is:
* `CMakeLists.txt` in the root directory is the main project definition. It includes subdirectories to add files/
  modules to the build. `pico_sdk_import.cmake` is provided by the Pico SDK.
* Code for apps and games is in the `source/apps` folder.
* Code for an interactive terminal (which may or may not be useful after the main application is running) is the
  `source/cli` folder. To run the CLI, hold the D-Pad left button down as the badge is starting. The display will
  show noise, and if you connect to the serial terminal that shows up on your computer, you can enter commands.
* Code that depends on Pico interfaces is within `source/hal/*_rp2040.c` files, with a platform agnostic header in the
  corresponding `source/hal/*.h` file. Code built for the simulator is in `source/hal/*_sim.c`.
* Generally helpful system code (main menus, screensavers, and the like) is in the `source/core` folder.
* Code for display buffers and drawing is in the `source/display` folder.

# Optional Editor Setup

Some notes about editor setup: [EDITOR-SETUP.md](./EDITOR-SETUP.md)
