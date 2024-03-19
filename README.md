# STL to Porosity Converter

This project provides a C++ code for converting STL files into level set distributions stored in orthogonal grids. The level set distribution, calculated using Signed Distance Fields (SDF), represents the distance from a geometric surface. The resulting data can be utilized for various applications such as computational fluid dynamics (CFD), computer-aided design (CAD), and medical image processing.

## Installation

1. **Clone the repository:**

   ```sh
   git clone https://github.com/username/repository.git
   ```

2. **Build using CMake:**

   ```sh
   cmake -S . -B build
   cmake --build build
   ```

## Usage

1. **Configuration:**

   Modify the `config.txt` file to specify the input STL file path, output CSV file name, output VTK file path, bounds factor, grid resolution, axis, thickness parameter, output VTK flag, and number of threads.

2. **Execution:**

   Run the compiled executable:

   ```sh
   ./build/src/STLToPorosity
   ```

   The output CSV file containing cell dimensions and scalar values will be generated. If the `outputVtk` option is set to true, an additional VTK file will be created.
