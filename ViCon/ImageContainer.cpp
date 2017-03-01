#include "ImageContainer.h"

ImageContainer::ImageContainer(int width, int height)
{
	this->width  = width;
	this->height = height;

	std::vector<int> vector;
	vector.reserve(width*height);

	for (auto value : vector) value = 0;
	
}

ImageContainer::~ImageContainer()
{
		
}

void ImageContainer::SetRgbaBuffer(int* newbuffer, int height, int width)
{
	buffer.resize(height*width);
	for (auto i = 0; i < height*width; i++)
		buffer[i] = newbuffer[i];
}

void ImageContainer::SetBgraBuffer(int* newbuffer, int height, int width)
{
	// Swap Red and Blue before saving as RGBA
	// Also set alpha to 255 to make it opaque
	for (auto i = 0; i < width * height; i++)
	 newbuffer[i] = newbuffer[i] & 0xff00ff00 |
		(newbuffer[i] & 0xff) << 16 |
		(newbuffer[i] & 0xff0000) >> 16 |
		0xff000000;
	SetRgbaBuffer(newbuffer, height, width);
}
