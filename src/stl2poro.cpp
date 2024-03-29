/*
MIT License

Copyright (c) 2024 Nobuto NAKAMICHI

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

#include <iostream>
#include <ctime>
#include <vector>
#include <vtkSmartPointer.h>
#include <vtkSTLReader.h>
#include <vtkPolyData.h>
#include <vtkImageData.h>
#include <vtkImplicitPolyDataDistance.h>
#include <vtkDoubleArray.h>
#include <vtkPointData.h>
#include <vtkXMLImageDataWriter.h>
#include <omp.h>

// Function to read settings from config file
bool readConfigFile(const std::string& configFilePath, std::string& stlFilePath, std::string& outputCsvFileName,
                    std::string& outputVtkFilePath, std::vector<double>& boundsFactor, int& grid, int& axis,
                    double& thickness, bool& outputVtk, int& numThreads) {
    std::ifstream configFile(configFilePath);
    if (!configFile.is_open()) {
        std::cerr << "Error: Failed to open config file." << std::endl;
        return false;
    }

    std::string line;
    while (std::getline(configFile, line)) {
        std::istringstream iss(line);
        std::string key;
        if (std::getline(iss, key, '=')) {
            std::string value;
            if (std::getline(iss, value)) {
                if (key == "stlFilePath") {
                    stlFilePath = value;
                } else if (key == "outputCsvFileName") {
                    outputCsvFileName = value;
                } else if (key == "outputVtkFilePath") {
                    outputVtkFilePath = value;
                } else if (key == "boundsFactor") {
                    std::istringstream iss2(value);
                    double factor;
                    while (iss2 >> factor) {
                        boundsFactor.push_back(factor);
                    }
                } else if (key == "grid") {
                    grid = std::stoi(value);
                } else if (key == "axis") {
                    axis = std::stoi(value);
                } else if (key == "thickness") {
                    thickness = std::stod(value);
                } else if (key == "outputVtk") {
                    outputVtk = (value == "true" || value == "1");
                } else if (key == "numThreads") {
                    numThreads = std::stoi(value);
                }
            }
        }
    }

    configFile.close();
    
    // Check if stlFilePath exists
    std::ifstream stlFile(stlFilePath);
    if (!stlFile.is_open()) {
        std::cerr << "Error: stlFilePath does not exist." << std::endl;
        return false;
    }

    stlFile.close();
    
    return true;
}

vtkSmartPointer<vtkImageData> processSTLFile(vtkSmartPointer<vtkPolyData> polyData, const std::vector<double>& boundsFactor, int grid, int axis) {
    double original_bounds[6];
    polyData->GetBounds(original_bounds);

    std::vector<double> expandedBounds(6);
    for (size_t i = 0; i < 6; ++i) {
        expandedBounds[i] = original_bounds[i] * boundsFactor[i];
    }

    double pitch = (expandedBounds[axis * 2 + 1] - expandedBounds[axis * 2]) / grid;
    std::vector<double> meshPitch(3, pitch);
    std::vector<double> mins(3);
    for (size_t i = 0; i < 3; ++i) {
        mins[i] = expandedBounds[i * 2] + pitch / 2;
    }

    std::vector<double> cellDims(3);
    for (size_t i = 0; i < 3; ++i) {
        cellDims[i] = (expandedBounds[i * 2 + 1] - expandedBounds[i * 2]) / meshPitch[i];
    }

    vtkSmartPointer<vtkImageData> imageData = vtkSmartPointer<vtkImageData>::New();
    imageData->SetExtent(0, std::round(cellDims[0]-1), 0, std::round(cellDims[1]-1), 0, std::round(cellDims[2]-1));
    imageData->SetOrigin(mins[0], mins[1], mins[2]);
    imageData->SetSpacing((expandedBounds[1] - expandedBounds[0]) / cellDims[0], (expandedBounds[3] - expandedBounds[2]) / cellDims[1], (expandedBounds[5] - expandedBounds[4]) / cellDims[2]);

    // Print out information about the processed data
    std::cout << std::endl;
    std::cout << "Input data information:" << std::endl;
    std::cout << "STL number of verticies: " << polyData->GetNumberOfPoints() << std::endl;
    std::cout << "STL number of faces: " << polyData->GetNumberOfCells() << std::endl;
    std::cout << "Original bounds: ";
    for (int i = 0; i < 6; ++i){
        std::cout << original_bounds[i] << " ";
    }
    std::cout << std::endl;
    std::cout << "Expanded bounds: ";
    for (int i = 0; i < 6; ++i){
        std::cout << expandedBounds[i] << " ";
    }
    std::cout << std::endl;
    std::cout << "Mesh pitch: " << meshPitch[0] << " " << meshPitch[1] << " " << meshPitch[2] << std::endl;
    std::cout << std::endl;
    return imageData;
}

/**
 * @brief Calculate the Signed Distance Fields.
 * 
 * @param polyData The input polydata representing the geometry.
 * @param imageData The input image data representing the grid.
 * @return The output array storing the Signed Distance Fields.
 */
vtkSmartPointer<vtkDoubleArray> computeSDF(vtkSmartPointer<vtkPolyData> polyData, vtkSmartPointer<vtkImageData> imageData) {
    vtkSmartPointer<vtkImplicitPolyDataDistance> sdf = vtkSmartPointer<vtkImplicitPolyDataDistance>::New();
    sdf->SetInput(polyData);

    vtkSmartPointer<vtkDoubleArray> sdfOutput = vtkSmartPointer<vtkDoubleArray>::New();
    sdfOutput->SetName("SDF");

    int* dims = imageData->GetDimensions();
    double* spacing = imageData->GetSpacing();
    double* origin = imageData->GetOrigin();

    for (int k = 0; k < dims[2]; ++k) {
        for (int j = 0; j < dims[1]; ++j) {
            for (int i = 0; i < dims[0]; ++i) {
                double point[3];
                point[0] = origin[0] + i * spacing[0];
                point[1] = origin[1] + j * spacing[1];
                point[2] = origin[2] + k * spacing[2];
                // Signed Distance Fields
                double distance = sdf->FunctionValue(point);
                sdfOutput->InsertNextValue(distance);
            }
        }
    }

    return sdfOutput;
}

/**
 * @brief Calculate the porosity.
 * 
 * @param sdf The input array storing the Signed Distance Fields.
 * @param thickness The thickness parameter for normalization.
 * @return The output array storing the porosity.
 */
vtkSmartPointer<vtkDoubleArray> computePorosity(vtkSmartPointer<vtkDoubleArray> sdf, double thickness) {
    vtkSmartPointer<vtkDoubleArray> porosity = vtkSmartPointer<vtkDoubleArray>::New();
    porosity->SetName("porosity");

    int numPoints = sdf->GetNumberOfValues();
    for (int i = 0; i < numPoints; ++i) {
        double distance = sdf->GetValue(i);
        // Normalize SDF by thickness
        double normalizedDistance = distance / thickness;
        // Apply tanh function to get wall boundary distribution in range [-1, 1]
        double wallBoundaryFunction = 0.5 * std::tanh(normalizedDistance) + 0.5;
        porosity->InsertNextValue(wallBoundaryFunction);
    }

    return porosity;
}

/**
 * @brief Calculate Multi-grayscale Image.
 * 
 * @param sdf The input array storing the Signed Distance Fields.
 * @param delta The thickness parameter for interface.
 * @return The output array storing the multi-grayscale image.
 */
vtkSmartPointer<vtkDoubleArray> computeMultigrayscale(vtkSmartPointer<vtkDoubleArray> sdf, double delta) {
    vtkSmartPointer<vtkDoubleArray> multiImage = vtkSmartPointer<vtkDoubleArray>::New();
    multiImage->SetName("multiGrayscale");

    if (!(delta > 0 && delta <= 1)) {
        std::cout << "Error: In function computeMultigrayscale, delta must satisfy 0 < delta <= 1." << std::endl;
        return 0;
    }

    int numPoints = sdf->GetNumberOfValues();
    for (int i = 0; i < numPoints; ++i) {
        double distance = sdf->GetValue(i);
        double normalizedDistance;
        // Normalize SDF and calculate binary
        if (std::abs(distance) <= delta ) {
            normalizedDistance = 0.5*(1 + distance/2);
        }
        else if (distance < 0) {
            normalizedDistance = 0.0;
        }
        else {
            normalizedDistance = 1.0;
        }
        multiImage->InsertNextValue(normalizedDistance);
    }

    return multiImage;
}

/**
 * @brief Calculate Binary Image.
 * 
 * @param sdf The input array storing the Signed Distance Fields.
 * @return The output array storing the binary image.
 */
vtkSmartPointer<vtkDoubleArray> computeBinary(vtkSmartPointer<vtkDoubleArray> sdf) {
    vtkSmartPointer<vtkDoubleArray> binaryImage = vtkSmartPointer<vtkDoubleArray>::New();
    binaryImage->SetName("binary");

    int numPoints = sdf->GetNumberOfValues();
    for (int i = 0; i < numPoints; ++i) {
        double distance = sdf->GetValue(i);
        // Normalize SDF and calculate binary
        int normalizedDistance = std::round(0.49 + 0.5*distance/(std::abs(distance)+FLT_MIN));
        binaryImage->InsertNextValue(normalizedDistance);
    }

    return binaryImage;
}

void outputDataDetails(vtkImageData* imageData) {
    // Output data details
    int* dims = imageData->GetDimensions();
    double* spacing = imageData->GetSpacing();
    double* origin = imageData->GetOrigin();
    int* extent = imageData->GetExtent();

    std::cout << std::endl;
    std::cout << "Output data details:" << std::endl;

    std::cout << "Cell dimensions: " << dims[0] << " " << dims[1] << " " << dims[2] << std::endl; 

    std::cout << "Extent: " << extent[0] << " " << extent[1] << " "
              << extent[2] << " " << extent[3] << " "
              << extent[4] << " " << extent[5] << std::endl;

    std::cout << "Origin: " << origin[0] << " " << origin[1] << " " << origin[2] << std::endl;
    std::cout << "Spacing: " << spacing[0] << " " << spacing[1] << " " << spacing[2] << std::endl;
    std::cout << std::endl;

    vtkSmartPointer<vtkPointData> pointData = imageData->GetPointData();
    int numArrays = pointData->GetNumberOfArrays();

    for (int i = 0; i < numArrays; ++i) {
        vtkDataArray* dataArray = pointData->GetArray(i);
        const char* arrayName = dataArray->GetName();
        int arraySize = dataArray->GetNumberOfTuples();
        const char* arrayType = (dataArray->GetNumberOfComponents() == 1) ? "Scalar" : "Vector";

        std::cout << "Array Name: " << arrayName << ", Type: " << arrayType << ", Size: " << arraySize << std::endl;
    }

    std::cout << std::endl;
}

void writeImageDataToCSV(vtkImageData* imageData, const std::string& scalarDataName, const std::string& outputCsvFileName) {
    // Write cell dimensions to CSV
    int* dims = imageData->GetDimensions();
    double* spacing = imageData->GetSpacing();
    double* origin = imageData->GetOrigin();
    int* extent = imageData->GetExtent();

    // Get the point data
    vtkSmartPointer<vtkPointData> pointData = imageData->GetPointData();

    // Get the array with the specified name
    vtkDataArray* scalarDataArray = pointData->GetArray(scalarDataName.c_str());
    if (!scalarDataArray) {
        std::cerr << "Error: Scalar data array with name " << scalarDataName << " not found." << std::endl;
        return;
    }

    std::ofstream csvFile(outputCsvFileName);
    if (!csvFile.is_open()) {
        std::cerr << "Error: Failed to open CSV file for writing." << std::endl;
        return;
    }

    std::cout << "Get scalar value of " << scalarDataName << std::endl;
    std::cout << "Output to " << outputCsvFileName << "..." << std::flush;
    csvFile << dims[0] << "," << dims[1] << "," << dims[2] << std::endl;

    // Write cell coordinates and scalar values to CSV
    for (int iz = extent[4]; iz <= extent[5]; ++iz) {
        for (int iy = extent[2]; iy <= extent[3]; ++iy) {
            for (int ix = extent[0]; ix <= extent[1]; ++ix) {
                double scalarValue = scalarDataArray->GetComponent(ix+iy*ix+iz*iy*ix, 0);
                csvFile << ix << "," << iy << "," << iz << "," << scalarValue << std::endl;
            }
        }
    }

    csvFile.close();
    std::cout << "Completed " << std::endl;
}

void writeImageDataToFile(vtkImageData* imageData, const std::string& outputVtkFilePath) {
    std::cout << "Output to " << outputVtkFilePath << "...";
    vtkSmartPointer<vtkXMLImageDataWriter> writer = vtkSmartPointer<vtkXMLImageDataWriter>::New();
    writer->SetFileName(outputVtkFilePath.c_str());
    writer->SetInputData(imageData);
    writer->Write();
    std::cout << "Completed " << std::endl;
}

int main() {
    // Read settings from config file
    std::string configFilePath = "config.txt";
    std::string stlFilePath, outputCsvFileName, outputVtkFilePath;
    std::vector<double> boundsFactor;
    int grid, axis, numThreads;
    double thickness;
    bool outputVtk;

    if (!readConfigFile(configFilePath, stlFilePath, outputCsvFileName, outputVtkFilePath, boundsFactor, grid, axis,
                        thickness, outputVtk, numThreads)) {
        return 1;
    }

    vtkSmartPointer<vtkSTLReader> reader = vtkSmartPointer<vtkSTLReader>::New();
    reader->SetFileName(stlFilePath.c_str());
    reader->Update();

    vtkSmartPointer<vtkPolyData> polyData = reader->GetOutput();
    vtkSmartPointer<vtkImageData> imageData = processSTLFile(polyData, boundsFactor, grid, axis);

    // **************** calculate **********************
    // Set number of threads
    std::cout << "Max threads: " << omp_get_max_threads() << std::endl;
    omp_set_num_threads(numThreads);
    std::cout << "Number of threads set: " << numThreads << std::endl;
    std::cout << "Calculating ..." << std::flush;
    // Variable declarations for measuring the execution time of the computePorosity function
    double startTime, t1, t2, t3, endTime;
    double* spacing = imageData->GetSpacing();
    // Start measuring the execution time
    startTime = omp_get_wtime();
    vtkSmartPointer<vtkDoubleArray> sdf = computeSDF(polyData, imageData);
    t1 = omp_get_wtime();
    vtkSmartPointer<vtkDoubleArray> porosity = computePorosity(sdf, thickness*spacing[0]);
    t2 = omp_get_wtime();
    vtkSmartPointer<vtkDoubleArray> multiGrayscale = computeMultigrayscale(sdf, 1.0*spacing[0]);
    t3 = omp_get_wtime();
    vtkSmartPointer<vtkDoubleArray> binary = computeBinary(sdf);
    // End measuring the execution time
    endTime = omp_get_wtime();
    std::cout << "Completed" << std::endl;
    // Calculate the elapsed time in seconds
    double elapsedTimeSDF = t1 - startTime;
    double elapsedTimePorosity = t2 - t1;
    double elapsedTimeMultigrayscale = t3 - t2;
    double elapsedTimeBinary = endTime - t3;
    double totalElapsedTime = endTime - startTime;
    // Output the computation time
    std::cout << "computeSDF function took " << elapsedTimeSDF << " seconds." << std::endl;
    std::cout << "computePorosity function took " << elapsedTimePorosity << " seconds." << std::endl;
    std::cout << "computeMultigrayscale function took " << elapsedTimeMultigrayscale << " seconds." << std::endl;
    std::cout << "computeBinary function took " << elapsedTimeBinary << " seconds." << std::endl;
    std::cout << "Total computation time: " << totalElapsedTime << " seconds." << std::endl;
    // **************** calculate end **********************

    vtkSmartPointer<vtkPointData> pointData = imageData->GetPointData();
    pointData->AddArray(porosity);
    pointData->AddArray(sdf);
    pointData->AddArray(multiGrayscale);
    pointData->AddArray(binary);

    // Call the function to output data details
    outputDataDetails(imageData);

    // Write cell dimensions to CSV
    std::ofstream csvFile(outputCsvFileName);
    if (!csvFile.is_open()) {
        std::cerr << "Error: Failed to open CSV file for writing." << std::endl;
        return 1;
    }

    // Call the function to write image data to CSV
    writeImageDataToCSV(imageData, "porosity", outputCsvFileName);

    // Call the function to write image data to file if outputVtk is true
    if (outputVtk) {
        writeImageDataToFile(imageData, outputVtkFilePath);
    }

    return 0;
}
