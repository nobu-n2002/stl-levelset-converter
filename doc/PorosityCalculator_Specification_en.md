# Porosity Calculator Specification

## Overview

The Porosity Calculator is a software tool designed to calculate the porosity of geometrical structures represented by STL files using Signed Distance Fields (SDF). This document outlines the functionality, input requirements, output formats, and other relevant details of the Porosity Calculator.

## License

The Porosity Calculator is released under the MIT License. The license details are as follows:

```txt
MIT License

Copyright (c) 2024 Nobuto NAKAMICHI

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

...
```

## Configuration File (config.txt)

The Porosity Calculator requires a configuration file named "config.txt" to specify input parameters. The format of the configuration file is as follows:

- `stlFilePath`: Path to the STL file containing the geometry.
- `outputCsvFileName`: Path to the CSV file where output data will be written.
- `outputVtkFilePath`: Path to the VTK file where output data will be written (optional).
- `boundsFactor`: A list of six floating-point values representing scaling factors for adjusting the bounding box size.
- `grid`: Integer value specifying the grid resolution.
- `axis`: Integer value specifying the axis for grid generation.
- `thickness`: Floating-point value representing the thickness parameter for normalization.
- `outputVtk`: Boolean value (true/false) indicating whether to output data in VTK format.
- `numThreads`: Integer value specifying the number of threads to use for parallel computation.

Example configuration file:

```txt
stlFilePath = example.stl
outputCsvFileName = output.csv
outputVtkFilePath = output.vtk
boundsFactor = 1.2 1.2 1.2 1.2 1.2 1.2
grid = 100
axis = 0
thickness = 1.5
outputVtk = true
numThreads = 4
```

## Input Requirements

The Porosity Calculator requires the following inputs:

- STL file representing the geometry of the structure.
- Configuration file specifying input parameters.

## Output Formats

The Porosity Calculator produces the following output:

- CSV file containing cell dimensions and scalar values.
- VTK file (optional) containing the processed data.

## Functionality

The Porosity Calculator performs the following steps:

1. Reads input parameters from the configuration file.
2. Loads the STL file and processes the geometry.
3. Calculates the porosity using Signed Distance Fields (SDF).
4. Generates output data in CSV and/or VTK format.
5. Provides information about the processed data.

## Code Specification

## Used Libraries

- VTK (Visualization Toolkit)
- OpenMP

### Functions and Classes

1. `bool readConfigFile(const std::string& configFilePath, std::string& stlFilePath, std::string& outputCsvFileName, std::string& outputVtkFilePath, std::vector<double>& boundsFactor, int& grid, int& axis, double& thickness, bool& outputVtk, int& numThreads)`
   - Description: Reads necessary parameters from the specified configuration file.
   - Arguments:
     - `configFilePath`: Path to the configuration file
     - `stlFilePath`: Path to the STL file
     - `outputCsvFileName`: Output path for the CSV file
     - `outputVtkFilePath`: Output path for the VTK file
     - `boundsFactor`: List of bounds factors
     - `grid`: Number of grid cells
     - `axis`: Index of the axis
     - `thickness`: Thickness value
     - `outputVtk`: Flag for VTK file output
     - `numThreads`: Number of threads
   - Returns: Boolean value indicating whether the configuration file was successfully read.

2. `vtkSmartPointer<vtkImageData> processSTLFile(vtkSmartPointer<vtkPolyData> polyData, const std::vector<double>& boundsFactor, int grid, int axis)`
   - Description: Processes the polygon data read from the STL file and generates VTK format image data.
   - Arguments:
     - `polyData`: Polygon data read from the STL file
     - `boundsFactor`: List of bounds factors
     - `grid`: Number of grid cells
     - `axis`: Index of the axis
   - Returns: Processed VTK format image data.

3. `vtkSmartPointer<vtkDoubleArray> computePorosity(vtkSmartPointer<vtkPolyData> polyData, vtkSmartPointer<vtkImageData> imageData, double thickness)`
   - Description: Calculates the porosity of the polygon data using Signed Distance Fields.
   - Arguments:
     - `polyData`: Input polygon data
     - `imageData`: Input image data
     - `thickness`: Thickness parameter
   - Returns: Calculated porosity data.

4. `void outputDataDetails(vtkImageData* imageData)`
   - Description: Outputs detailed information about the output data.
   - Arguments:
     - `imageData`: Output image data

5. `void writeImageDataToCSV(vtkImageData* imageData, const std::string& outputCsvFileName)`
   - Description: Writes image data to a CSV file.
   - Arguments:
     - `imageData`: Output image data
     - `outputCsvFileName`: Output path for the CSV file

6. `void writeImageDataToFile(vtkImageData* imageData, const std::string& outputVtkFilePath)`
   - Description: Writes image data to a VTK file.
   - Arguments:
     - `imageData`: Output image data
     - `outputVtkFilePath`: Output path for the VTK file

7. `int main()`
   - Description: Main function. Reads the configuration file, processes polygon data from the STL file, calculates porosity, and outputs it to files.

---

Please let me know if you need further clarification or additional details.
