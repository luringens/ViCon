#include "ImageContainer.h"

ImageContainer::ImageContainer(int width, int height)
	: height(height), width(width)
{
	buffer.reserve(width * height);

	glGenTextures(1, &texture);
	glBindTexture(GL_TEXTURE_2D, texture);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR); // Linear Filtering
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR); // Linear Filtering
}

ImageContainer::~ImageContainer()
{
		
}

void ImageContainer::CreateTexture()
{
	glBindTexture(GL_TEXTURE_2D, GetTexture());
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height,
		0, GL_RGBA, GL_UNSIGNED_BYTE, static_cast<GLvoid*>(buffer.data()));
}

void ImageContainer::SetRgbaBuffer(int* newbuffer, int height, int width)
{
	buffer.resize(height*width*sizeof(int));
	for (auto i = 0; i < height*width * sizeof(int); i++)
		buffer[i] = newbuffer[i];
	CreateTexture();
}

void ImageContainer::SetBgraBuffer(int* newbuffer, int height, int width)
{
	// Swap Red and Blue before saving as RGBA
	// Also set alpha to 255 to make it opaque
	/*for (auto i = 0; i < width * height; i++)
	 newbuffer[i] = newbuffer[i] & 0xff00ff00 |
		(newbuffer[i] & 0xff) << 16 |
		(newbuffer[i] & 0xff0000) >> 16 |
		0xff000000;*/
	SetRgbaBuffer(newbuffer, height, width);
}
