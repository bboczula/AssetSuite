# AssetSuite
## Overview
A simple and convinient library for loading game assets (like images and meshes) from disc to local memory.It's main design principle is that it has to be both convinient to use and easy to read and understand, and not necessarily extremly fast. You should use this library if you simply want to get your work done on your engine and don't care about the details, or if you are studying image formats like BMP or PNG.

## Dependencies
The library requires C++17 because it uses the `filesystem` header. It also requires C++20 because of the `span` usage.

## Usage
First of all, you need to declare Asset Suite manager. Asset Suite manages its memory internally, so once this class goes out of scope, the memory will be cleared.
```cpp
AssetSuite::Manager assetManager;
```
Then you need to load and decode image or mesh from the disc, to the memory. By default, the appropriate decoder will be used based on the file extension, however, you can provide an option which decoder to use. You can either use two different functions or a convinent one that does both of those things.
```cpp
auto errorCode = assetManager.ImageLoadAndDecode("girl_with_pearl_earring.bmp");
```
Finally, you need to retrieve the image from memory. You can do that by using the `Get` function. You need to provide the output format of the image, as well as a vector of bytes where the output will be copied to. You will also get some information about the image in the `ImageDescriptor`, like image dimensions.
```cpp
AssetSuite::ImageDescriptor imageDescriptor = {};
std::vector<BYTE> imageOutput;
errorCode = assetManager.ImageGet(AssetSuite::OutputFormat::RGB8, imageOutput, imageDescriptor);
```
The procedure is very similar for meshes, but when you get one, you need to provide the name of the mesh as well, since usually you can have multiple meshes in one file.
```cpp
errorCode = assetManager.MeshLoadAndDecode("wavefront_sample.obj");

std::vector<FLOAT> meshOutput;
AssetSuite::MeshDescriptor meshDescriptor;
errorCode = assetManager.MeshGet("Plane_Plane\r", AssetSuite::MeshOutputFormat::POSITION, meshOutput, meshDescriptor);
```

## Integration

## Installation

## License
This library is provided as is, and it uses the MIT license.
