#ifndef USE_DIRECTX11
#include "main.h"
#include "texture.h"
#include <glad/glad.h>
#include <cstdio>
#include <cstring>
#include <map>
#include <string>

static std::map<std::string, int> loadedTexture;

// Convert "rom:/path" to "rom/path"
static std::string ResolvePath(const char* filename)
{
	std::string path(filename);
	// Replace "rom:/" with "rom/"
	if (path.rfind("rom:/", 0) == 0) {
		path = "rom/" + path.substr(5);
	}
	return path;
}

unsigned int LoadTexture(const char* FileName)
{
	std::string resolved = ResolvePath(FileName);

	// Check cache
	auto it = loadedTexture.find(resolved);
	if (it != loadedTexture.end()) {
		return it->second;
	}

	// Open TGA file
	FILE* fp = fopen(resolved.c_str(), "rb");
	if (!fp) {
		printf("Failed to open texture: %s\n", resolved.c_str());
		return 0;
	}

	// Read TGA header (18 bytes)
	unsigned char header[18];
	fread(header, 1, 18, fp);

	int width = header[12] | (header[13] << 8);
	int height = header[14] | (header[15] << 8);
	int bpp = header[16] / 8;

	// Read image data
	int imageSize = width * height * bpp;
	unsigned char* imageData = new unsigned char[imageSize];
	fread(imageData, 1, imageSize, fp);
	fclose(fp);

	// Swap R and B channels (TGA is BGR)
	for (int i = 0; i < imageSize; i += bpp) {
		unsigned char tmp = imageData[i];
		imageData[i] = imageData[i + 2];
		imageData[i + 2] = tmp;
	}

	// Create GL texture
	GLuint texture;
	glGenTextures(1, &texture);
	glBindTexture(GL_TEXTURE_2D, texture);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

	GLenum format = (bpp == 4) ? GL_RGBA : GL_RGB;
	glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, imageData);

	glBindTexture(GL_TEXTURE_2D, 0);
	delete[] imageData;

	loadedTexture[resolved] = texture;
	return texture;
}

void UnloadTexture(unsigned int texture)
{
	if (texture != 0) {
		glDeleteTextures(1, &texture);
	}
}

void UnloadTextures()
{
	for (auto& pair : loadedTexture) {
		GLuint tex = pair.second;
		glDeleteTextures(1, &tex);
	}
	loadedTexture.clear();
}

#endif // !USE_DIRECTX11
