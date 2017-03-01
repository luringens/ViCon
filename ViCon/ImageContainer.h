#pragma once
#include <vector>
#include "gl3w/gl3w.h"

class ImageContainer
{
public:
	ImageContainer(int width, int height);
	
	void SetRgbaBuffer(int* newbuffer, int height, int width);
	void SetBgraBuffer(int* newbuffer, int height, int width);

	GLuint& GetTexture() const { return *texture; }

	int Height() const { return height; }
	int Width()  const { return width;  }

	~ImageContainer();
private:
	int height;
	int width;
	std::vector<int> buffer;
	GLuint* texture;
};

