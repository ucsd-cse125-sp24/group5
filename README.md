# Vivaldi - CSE 125 Team 5


\[Insert creative game description here\]


## Development Setup

Our code is cross-platform! ~~Unless you're on Linux~~ We support developing and building Vivaldi on both Windows and macOS, and the build instructions will be (or at least, should be) identical on both platforms.

### Setting up your environment
1. [Download](https://visualstudio.microsoft.com/) Visual Studio (NOT Visual Studio Code!) from Microsoft's website. Make sure you specify to download CMake in the installation options.
   1. If you forgot to select to install CMake while setting up Visual Studio, you can still select the option by opening up Visual Studio Installer on your machine.
2. If you don't want to install CMake from Microsoft, you can also download it from [here](https://cmake.org/) on CMake's official website.
3. Download whatever IDE/text editor you'd like, Visual Studio, Visual Studio Code, CLion, ~~Microsoft Word~~, Vim, it doesn't matter.
4. Clone the repository onto your machine, warning: the repo is quite fat as we included the source code for several libaries in order to make cross-platform development as smooth as possible.
```
git clone https://github.com/ucsd-cse125-sp24/group5.git
```
5. If you want IDE integration with the code, make sure to load the CMake project associated with the `CMakeLists.txt` in the repository's root directory.


### Building Vivaldi from the command line

From the repository root, run the following.
```sh
$ mkdir build
$ cd build
$ cmake ..
$ cmake --build .
```

### Running Vivaldi

TBD, if we even get here lol

### Developing Vivaldi

