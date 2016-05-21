#pragma once
#define RASTER_WIDTH 500
#define RASTER_HEIGHT  500
#define NUM_PIXELS (RASTER_WIDTH * RASTER_HEIGHT)
#define NEAR_PLANE 0.1f
#define FAR_PLANE 100.0f

struct UV
{
	float U, V;
};

struct RasterPoint
{
	float x, y;
};


struct Matrix4x4
{
	float matrix[4][4];
};

struct SIMPLE_VERTEX
{
	float vertex[3];
	float uv[3];
	float normals[3];
};

//
//struct Grid
//{
//	Vertex points[44];
//};
//
//struct Cube
//{
//	Vertex points[36];
//};

