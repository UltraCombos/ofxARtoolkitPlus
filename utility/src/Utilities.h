#pragma once

#include "ofMain.h"

namespace
{
	void gaussian_elimination(float *input, int n)
	{
		// ported to c from pseudocode in
		// http://en.wikipedia.org/wiki/Gaussian_elimination
		float * A = input;
		int i = 0;
		int j = 0;
		int m = n - 1;
		while (i < m && j < n)
		{
			// Find pivot in column j, starting in row i:
			int maxi = i;
			for (int k = i + 1; k<m; k++)
			{
				if (fabs(A[k*n + j]) > fabs(A[maxi*n + j]))
				{
					maxi = k;
				}
			}
			if (A[maxi*n + j] != 0)
			{
				//swap rows i and maxi, but do not change the value of i
				if (i != maxi)
					for (int k = 0; k<n; k++)
					{
						float aux = A[i*n + k];
						A[i*n + k] = A[maxi*n + k];
						A[maxi*n + k] = aux;
					}
				//Now A[i,j] will contain the old value of A[maxi,j].
				//divide each entry in row i by A[i,j]
				float A_ij = A[i*n + j];
				for (int k = 0; k<n; k++)
				{
					A[i*n + k] /= A_ij;
				}
				//Now A[i,j] will have the value 1.
				for (int u = i + 1; u< m; u++)
				{
					//subtract A[u,j] * row i from row u
					float A_uj = A[u*n + j];
					for (int k = 0; k<n; k++)
					{
						A[u*n + k] -= A_uj*A[i*n + k];
					}
					//Now A[u,j] will be 0, since A[u,j] - A[i,j] * A[u,j] = A[u,j] - 1 * A[u,j] = 0.
				}
				i++;
			}
			j++;
		}
		//back substitution
		for (int i = m - 2; i >= 0; i--)
		{
			for (int j = i + 1; j<n - 1; j++)
			{
				A[i*n + m] -= A[i*n + j] * A[j*n + m];
				//A[i*n+j]=0;
			}
		}
	}

	ofMatrix4x4 findVectorHomographyForOpenGL(ofPoint src[4], ofPoint dst[4])
	{
		// create the equation system to be solved
		//
		// from: Multiple View Geometry in Computer Vision 2ed
		// Hartley R. and Zisserman A.
		//
		// x' = xH
		// where H is the homography: a 3 by 3 matrix
		// that transformed to inhomogeneous coordinates for each point
		// gives the following equations for each point:
		//
		// x' * (h31*x + h32*y + h33) = h11*x + h12*y + h13
		// y' * (h31*x + h32*y + h33) = h21*x + h22*y + h23
		//
		// as the homography is scale independent we can let h33 be 1 (indeed any of the terms)
		// so for 4 points we have 8 equations for 8 terms to solve: h11 - h32
		// after ordering the terms it gives the following matrix
		// that can be solved with gaussian elimination:
		float P[8][9] = {
			{ -src[0].x, -src[0].y, -1, 0, 0, 0, src[0].x*dst[0].x, src[0].y*dst[0].x, -dst[0].x }, // h11
			{ 0, 0, 0, -src[0].x, -src[0].y, -1, src[0].x*dst[0].y, src[0].y*dst[0].y, -dst[0].y }, // h12
			{ -src[1].x, -src[1].y, -1, 0, 0, 0, src[1].x*dst[1].x, src[1].y*dst[1].x, -dst[1].x }, // h13
			{ 0, 0, 0, -src[1].x, -src[1].y, -1, src[1].x*dst[1].y, src[1].y*dst[1].y, -dst[1].y }, // h21
			{ -src[2].x, -src[2].y, -1, 0, 0, 0, src[2].x*dst[2].x, src[2].y*dst[2].x, -dst[2].x }, // h22
			{ 0, 0, 0, -src[2].x, -src[2].y, -1, src[2].x*dst[2].y, src[2].y*dst[2].y, -dst[2].y }, // h23
			{ -src[3].x, -src[3].y, -1, 0, 0, 0, src[3].x*dst[3].x, src[3].y*dst[3].x, -dst[3].x }, // h31
			{ 0, 0, 0, -src[3].x, -src[3].y, -1, src[3].x*dst[3].y, src[3].y*dst[3].y, -dst[3].y }, // h32
		};
		gaussian_elimination(&P[0][0], 9);
		ofMatrix3x3 mat3(P[0][8], P[1][8], P[2][8],
			P[3][8], P[4][8], P[5][8],
			P[6][8], P[7][8], 1);
		mat3.invert();

		return ofMatrix4x4(mat3.a, mat3.d, mat3.g, 0.f,
			mat3.b, mat3.e, mat3.h, 0.f,
			mat3.c, mat3.f, mat3.i, 0.f,
			0.f, 0.f, 0.f, 0.f);
	}

	void toggleFullscreen(size_t width, size_t height)
	{
		ofToggleFullscreen();
		if (!(ofGetWindowMode() == OF_FULLSCREEN))
		{
			ofSetWindowShape(width, height);
			auto pos = ofVec2f(ofGetScreenWidth() - width, ofGetScreenHeight() - height) * 0.5f;
			ofSetWindowPosition(pos.x, pos.y);
		}
	}

	ofRectangle getCenteredRect(int srcWidth, int srcHeight, int otherWidth, int otherHeight, bool isFill = true)
	{
		ofRectangle other(0, 0, otherWidth, otherHeight);
		ofRectangle result; result.setFromCenter(other.getCenter(), srcWidth, srcHeight);
		float scaleBy = other.getHeight() / result.getHeight();
		auto aspectAspect = result.getAspectRatio() / other.getAspectRatio();
		if ((isFill && aspectAspect <= 1.0f) || (!isFill && aspectAspect >= 1.0f))
			scaleBy = other.getWidth() / result.getWidth();
		result.scaleFromCenter(scaleBy);
		return result;
	}


	void drawRectangle(float x, float y, float w, float h)
	{
		static ofVbo vbo;
		static vector<ofVec4f> vertices(4);
		vertices[0].set(x, y, 0, 1);
		vertices[1].set(x + w, y, 0, 1);
		vertices[2].set(x + w, y + h, 0, 1);
		vertices[3].set(x, y + h, 0, 1);
		static vector<ofFloatColor> colors(4);
		colors.assign(4, ofGetStyle().color);
		if (!vbo.getIsAllocated())
		{
			vector<ofVec2f> texCoords = { ofVec2f(0, 0), ofVec2f(1, 0), ofVec2f(1, 1), ofVec2f(0, 1) };
			vector<ofVec3f> normals(4, ofVec3f(0, 0, 1));
			vector<ofIndexType> indices = { 0, 2, 1, 0, 3, 2 };
			vbo.setVertexData(&vertices[0].x, 4, vertices.size(), GL_DYNAMIC_DRAW);
			vbo.setColorData(&colors[0].r, colors.size(), GL_DYNAMIC_DRAW);
			vbo.setTexCoordData(&texCoords[0].x, texCoords.size(), GL_STATIC_DRAW);
			vbo.setNormalData(&normals[0].x, normals.size(), GL_STATIC_DRAW);
			vbo.setIndexData(&indices[0], indices.size(), GL_STATIC_DRAW);
		}
		else
		{
			vbo.updateVertexData(&vertices[0].x, vertices.size());
			vbo.updateColorData(&colors[0].r, colors.size());
		}
		vbo.drawElements(GL_TRIANGLES, vbo.getNumIndices());
	}
	void drawRectangle(ofVec2f pos, float w, float h) { drawRectangle(pos.x, pos.y, w, h); }
	void drawRectangle(const ofRectangle& rect) { drawRectangle(rect.x, rect.y, rect.width, rect.height); }

}

using PixelSaverRef = std::shared_ptr<class PixelSaver>;

class PixelSaver
{
public:
	static PixelSaverRef create(string path = "print_screen", bool bNameAsSequence = false)
	{
		return PixelSaverRef(new PixelSaver(path, bNameAsSequence));
	}
	void savePixels(ofPixels pix)
	{
		std::lock_guard<mutex> guard(m);
		buffer.push_back(pix);
	}
	void saveTerxture(const ofTexture& tex)
	{
		std::lock_guard<mutex> guard(m);
		buffer.push_back(ofPixels());
		tex.readToPixels(buffer.back());
	}
	void saveFbo(const ofFbo& fbo)
	{
		saveTerxture(fbo.getTexture());
	}
	~PixelSaver()
	{
		is_thread_running = false;
		th.join();
	}

private:
	PixelSaver(string path, bool bNameAsSequence)
		:is_sequence(bNameAsSequence), path(path)
	{
		ofDirectory dir(path);
		if (!dir.exists()) dir.createDirectory(path);
		th = std::thread(&PixelSaver::threadFunction, this);
	}
	void threadFunction()
	{
		while (is_thread_running)
		{
			if (buffer.size() == 0) continue;

			ofPixels pix;
			{
				std::lock_guard<mutex> guard(m);
				pix = buffer[0];
				buffer.pop_front();
			}
			string file_path = path + ofVAArgsToString("/%s.png", ofGetTimestampString().c_str());
			if (is_sequence)
			{
				file_path = path + ofVAArgsToString("/sequence-%04d.png", sequence_index);
				sequence_index++;
			}
			ofSaveImage(pix, file_path);
		}
	}

	std::thread th;
	mutex m;
	std::deque<ofPixels> buffer;
	bool is_sequence;
	int sequence_index = 0;
	bool is_thread_running = true;
	string path;
};