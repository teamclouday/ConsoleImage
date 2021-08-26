# ConsoleImage
Display image in console using specified characters

------

### Build Guide

__Requirements__:  
* C++17 compiler  
* cmake  
* GPU supporting OpenGL >= 3.3  

__Commands__:

```bash
mkdir build && cd build
cmake ..
make -j4
```

Or (on Windows)  
```bash
cmake --build . --config Release
```

Executable will be generated in folder `bin`

------

### Demo

<img src="assets/demo1.png" width="400" alt="demo1">

<img src="assets/demo2.png" width="400" alt="demo2">

<img src="assets/demo3.png" width="400" alt="demo3">

------

### Future Improvements

Add edge detection kernel to improve output visual