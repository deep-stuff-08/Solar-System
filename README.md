![GitHub](https://img.shields.io/github/license/deep-stuff-08/Solar-System?style=plastic) ![GitHub](https://img.shields.io/badge/platforms-linux-success?style=plastic) ![GitHub](https://img.shields.io/badge/dependency-FreeType_|_GLEW_|_X11_|_OpenAL_|_ALUT_|_SOIL-orange?style=plastic)

# Solar-System

## Introduction

The project is meant to be a fun and interactive application for teaching and learning about our Solar System. It renders the entire Solar System with a beautiful initial animation. For movement you could use the mouse which controls a smooth arc-ball camera with the option to zoom in or out. You could select any planet and it would zoom in on the planet and display astronomical statistics about it.

## Effects Used

1. Arc-Ball Camera
1. Mouse Picking
1. Perlin Noise Planet Textures
1. Metaballs
1. Depht Of Field
1. Procedurally Generated Cubemap
1. Starfield
1. Font Rendering

## Requirements
To run the code you must have a working installation of:

1. FreeType 
1. GLEW
1. X11 Devel
1. OpenAL
1. ALUT
1. SOIL

## Run Instructions

Run the bash script 'run.sh' and if all dependencies are in place the code should run automatically

```bash
$ run.sh
```

## Limitations

Currently the code only work on Linux as the windowing is programmed in X11 but could theortically be run on any Operating System with a working X11 wrapper or implementatiion like WSL on Windows.
