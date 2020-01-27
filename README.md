# Raytracer
[![Try it now](https://img.shields.io/badge/Try%20It-Online-blue.svg)](https://bwrsandman.github.io/raytracer)

*by Sandy Carter and Anne van Ede*

![image](https://user-images.githubusercontent.com/1013356/73201242-e1c54f80-4138-11ea-8e63-15cc56f2785b.png)

## Pipeline

![Screenshot from 2020-01-27 16-28-55](https://user-images.githubusercontent.com/1013356/73202738-f9520780-413b-11ea-8873-fb202db178ac.png)


## Building
Building the project requires CMake and a package manager.
There are many options for package managers.

### vcpkg

Follow the installation steps at https://github.com/Microsoft/vcpkg

Make sure to set the global environment variable `VCPKG_DEFAULT_TRIPLET=x64-windows`

Install the dependencies
```
vcpkg install sdl2 glslang spirv-cross
```

When generating the cmake project use the vcpkg toolchain file.

### msys2

Install the following packages

```bash
pacman -S mingw-w64-x86_64-sdl2 mingw-w64-x86_64-glslang mingw-w64-x86_64-spirv-cross
```
