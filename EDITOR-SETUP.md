# Language Server

Code completions and other language server nice-to-haves are possible.
Typically you need clangd installed and you need `compile-commands.json` somewhere where the language server can find it.

> in vscode, i believe cmake-tools handles installing clangd

A `compile-commands.json` file is output by building either the sdl simulator or the badge firmware.  You can symlink this file to the root of the repo (there may be a better way to do this).

If you're building the simulator:

`ln -s ./build_sdl_simulator/compile_commands.json .`

If you're building the firmware:

`ln -s ./build_sdl_simulator/compile_commands.json .`

> this file should not be git commited. 

# VSCODE

## Extensions

These extensions are recommended:

* [ms-vscode.cpptools](https://marketplace.visualstudio.com/items?itemName=ms-vscode.cpptools)
* [ms-vscode.cmake-tools](https://marketplace.visualstudio.com/items?itemName=ms-vscode.cmake-tools)

### Additional Extensions
In previous years [cortex-debug](https://marketplace.visualstudio.com/items?itemName=marus25.cortex-debug) was used for debugging some of the hardware related code.

## Compile/Run Button

If you like having a one button option to build/run, adding the following two files will provide `run simulator` and `build firmware` in the "Run and Debug" tab.

<details>
<summary>`./.vscode/launch.json` </summary>

```json
{
    "version": "0.2.0",
    "configurations": [
        {
            "name": "Run SDL Simulation",
            "type": "cppdbg",
            "request": "launch",
            "program": "${workspaceFolder}/build_sdl_sim/source/badge2025_c",
            "cwd": "${workspaceFolder}/build_sdl_sim",
            "preLaunchTask": "Build SDL Sim",
            "stopAtEntry": false,
            "externalConsole": false
        },
        {
          "name": "Build Firmware",
          "type": "cppdbg",
          "request": "launch",
          "preLaunchTask": "Build Pico",
          "cwd": "${workspaceFolder}/build",
          "program": "${workspaceFolder}/build/source/badge2025_c.elf",
          "stopAtEntry": false,
          "internalConsoleOptions": "neverOpen",
          "externalConsole": false
        }
    ]
}
```
</details>

<details>
<summary>`./.vscode/tasks.json`</summary>

```json
{
  "version": "2.0.0",
  "tasks": [
    {
      "label": "Configure SDL Sim",
      "type": "shell",
      "command": "bash",
      "args": [
        "./run_cmake_sdl_sim.sh"
      ],
      "problemMatcher": [],
      "group": {
        "kind": "build",
        "isDefault": false
      }
    },
    {
      "label": "Build SDL Sim",
      "type": "shell",
      "command": "make",
      "options": {
        "cwd": "${workspaceFolder}/build_sdl_sim"
      },
      "problemMatcher": [],
      "group": {
        "kind": "build",
        "isDefault": true
      },
      "dependsOn": "Configure SDL Sim"
    },
    {
      "label": "Configure Pico",
      "type": "shell",
      "command": "bash",
      "args": ["-c", "./run_cmake.sh"],
      "options": {
        "cwd": "${workspaceFolder}"
      },
      "problemMatcher": [],
      "group": {
        "kind": "build",
        "isDefault": false
      }
    },
    {
      "label": "Build Pico",
      "type": "shell",
      "command": "bash",
      "args": ["-c", "make -C build"],
      "options": {
        "cwd": "${workspaceFolder}"
      },
      "problemMatcher": ["$gcc"],
      "group": {
        "kind": "build",
        "isDefault": true
      },
      "dependsOn": "Configure Pico"
    }
  ]
}
```
</details>

## Windows Users
[VS Code integration with WSL](https://learn.microsoft.com/en-us/windows/wsl/tutorials/wsl-vscode)

