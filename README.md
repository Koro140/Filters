# Filters
- it allows you to play videos and select one of many filters/effects to apply to the video and code custom filters
- it is a project to learn shaders and how to apply them in a pipeline

## How To Install
- to clone this repository
```console
git clone --recurse-submodules https://github.com/Koro140/Filters.git
```
- this project is dependant on SDL3 and ffmpeg libraries
if you are on linux (fedora for example):
```console
sudo dnf install SDL3-devel ffmpeg-devel
```
if you are on Windows then run the vcpkg command to install and compile the libraries
```console
vcpkg install
```

## How To Build
- the project uses CMake
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
./filters -V [VIDEO_NAME] -F bnw -F chroma
```
then the chromatic a lines will be blurred .. they will be clear if applied in reverse order

and if you do
```console
./filters -V [VIDEO_NAME] -F bnw -F vhs
```
then the vhs edges will be colored .. they will be b&w if applied in reverse order you get the idea

### Supported Filter
```console
- bnw      --- black and white
- vhs      --- Old VHS effect
- blur     --- Blurry effect
- glitch   --- Glitching effect
- chroma   --- Chromatic Aberration effect
- scanline --- Old CRT TV lines effect
- film     --- Film grain effect
- vignette --- Vignette effect
- pixel    --- Pixelation effect
- evil     --- Negative colors effect
```
hoping to add more ;)