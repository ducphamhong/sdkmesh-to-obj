# sdkmesh-to-obj

A tool help to convert DirectX sdkmesh to Wavefront .obj file

## How to build

Prerequisites
-   [CMake](https://cmake.org/download/)
-   [Visual Studio](https://visualstudio.microsoft.com/downloads/)  (2017 or higher if you want to build on Windows OS)

Generate project VC
```console
    cmake -S . -B ./PrjVisualStudio -G "Visual Studio 15 2017" -A WIN32
```

-   Open the generated solution `PrjVisualStudio/SDKMESH_EXPORTER.sln`

-   Once the solution opens, right click the **SDKMeshObjExporter** project, click **"Set as StartUp Project"** and build tool.

## Usage

```console
    SDKMeshObjExporter.exe -i=INPUT.sdkmesh -o=OUTPUT.obj
```
