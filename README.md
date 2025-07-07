# SDF Raymarching Application

A **semi**-cross-platform application made for rendering scenes using raymarching via signed distance fields utilizing OpenGL. If I decide to further my work, I will focus on utilizing on compute shaders instead of purely fragments.

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
