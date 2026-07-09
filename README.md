# Filters
- it allows you to play videos and select one of many filters/effects to apply to the video and code custome filters
- it is a project to learn shaders and how to apply them in a pipeline

## How To Build
the project uses CMake
```console
cmake -B build -S .
cmake --build build

cd build
./filters -V [VIDEO_NAME] -F [FILTER_NAME]
```

## How To Apply Filters
when you run
```console
./filters -V [VIDEO_NAME] -F [FILTER_NAME] -F [FILTER_NAME] ....
```
you can stack filters on top of each other for example you if you apply 
```console
./filters -V [VIDEO_NAME] -F vhs -F blur
```
then the vhs lines will be blurred .. they will be clear if applied in reverse order

and if you do
```console
./filters -V [VIDEO_NAME] -F bnw -F vhs
```
then the vhs edges will be colored .. they will be b&w if applied in reverse order you get the idea

### Supported Filter
- bnw  ----- black and white
- vhs  ----- Old VHS effect
- blur ----- Blurry effect
hoping to add more ;)