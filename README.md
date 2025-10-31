## ZBS — Simple Build System

**ZBS** is a minimal build system in C++ so... yeah.

### Installation

``` bash
git clone https://github.com/TheZetmix/zbs.git
cd zbs
g++ main.cpp -o zbs
```

### Usage

Create a `.zbsconfig` file in your project root:

``` ini
[config my_app]
[sources]
main.cpp
utils.cpp

[link]
ncurses
raylib

[output]
my_app

[compiler]
g++

[flags]
-std=c++20
-O2
```

Build your project:

``` bash
./zbs
```

It prints build commands and compiles your project.

## `.zbsconfig` structure

[config `name`] — new configuration

[sources] — source files

[link] — libraries for -l (checked via pkg-config)

[output] — output executable name

[compiler] — compiler (default: g++)

[flags] — additional compiler flags
