# Filters
- it allows you to play videos and select one of many filters/effects to apply to the video and code custome filters
- it is a project to learn shaders and how to apply them in a pipeline

## How To Build
- the project uses CMake
```console
cmake -B build -S .
cmake --build build

cd build
./filters -v [video_name] -s [script]
```