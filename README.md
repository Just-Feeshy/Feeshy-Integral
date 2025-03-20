# SDF Raymarching Application

A **semi**-cross-platform application made for rendering scenes using raymarching via signed distance fields utilizing OpenGL. If I decide to further my work, I will focus on utilizing on compute shaders instead of purely fragments. However, I don't have a proper PC to do so. In this project, we are only focused on distanced based raymarching.

<p align="center">
  <img src="https://github.com/Just-Feeshy/Feeshy-Integral/blob/Init/screenshots/take-1.png" style="width:75%;">
</p>

## Presentation Slides
<p align="center">
  <a href="https://docs.google.com/presentation/d/1r-cLMqXjYzPNCFv5gTDhlm2inaUlTxu8MPqP6Pjr23E/edit?usp=sharing">
    <img src="https://github.com/Just-Feeshy/Feeshy-Integral/blob/Init/screenshots/presentation.png">
  </a>
</p>

[//]: <> (Add further stuff such as video or paper)



## Specifications and Supported Platforms
### MacOS (Arm64)
* **OpenGL Version**: 4.1 Metal
* **GLSL Version**: 4.1
### Emscripten
* **OpenGL Version**: ES 3.0 (WebGL 2.0)
* **GLSL Version**: ES 3.0

## Building the Project (From Source)
Regarding prerequisites, [premake5](https://premake.github.io) is required to build the project. Once premake is installed, this command can be used to build the project:
```
premake5 --os=<target> --file=build/premake5.lua gmake
```

### Desktop
**BEFORE BUILDING**: make sure to have [Haxe 4.3](https://haxe.org/download/list/) to compile SDL into a static library, in which Haxe can be installed via **Homebrew**. Once Haxe is installed, go to the build directory and run the script via:
```
cd build
haxe build.hxml
```
And SDL used to produce a static library.

To run the project:
```
./bin/program
```

## Controls:

| Key   | Description                            |
|-------|----------------------------------------|
| W     | Move forward                           |
| S     | Move backwards                         |
| A     | Move left                              |
| D     | Move right                             |
| Q     | Move Up                                |
| E     | Move Down                              |
| Escape| Tab Out                                |
