#include "indexBuffers.hpp"
#include "Renderer/openglErrors.hpp"
#include "Logs/logs.hpp"
#include "GL/glew.h"
#include <array>

namespace ph {

IndexBuffer createIndexBuffer()
{
	unsigned id;
	GLCheck( glGenBuffers(1, &id) );
	return {id};
}

void setData(IndexBuffer ibo, unsigned* indices, size_t arraySize)
{
	GLCheck( glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo.mID) );
	GLCheck( glBufferData(GL_ELEMENT_ARRAY_BUFFER, arraySize, indices, GL_STATIC_DRAW) );
}

void deleteIndexBuffer(IndexBuffer ibo)
{
	GLCheck( glDeleteBuffers(1, &ibo.mID) );
}

void bind(IndexBuffer ibo)
{
	GLCheck( glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo.mID) );
}

IndexBufferHolder::IndexBufferHolder()
{
	// create rectangle index buffer
	std::array<unsigned, 6> rectangleIndices = {
		0, 1, 3, // first rectangle
		1, 2, 3 // second rectangle
	};
	mRectangleIndexBuffer = createIndexBuffer();
	setData(mRectangleIndexBuffer, rectangleIndices.data(), rectangleIndices.size() * sizeof(unsigned));
}

IndexBuffer IndexBufferHolder::addAndGetIndexBuffer(const std::string& name, unsigned* data, size_t arraySize)
{
	IndexBuffer ibo = createIndexBuffer();
	setData(ibo, data, arraySize);
	mIndexBuffers.emplace_back(ibo);
	mNames.emplace_back(name);
	mReferenceCounters.emplace_back(1);
	return ibo;
}

IndexBuffer IndexBufferHolder::getIndexBuffer(const std::string& name)
{
	for(size_t i = 0; i < mNames.size(); ++i) {
		if(mNames[i] == name) {
			++mReferenceCounters[i];
			return mIndexBuffers[i];
		}
	}
	PH_LOG_ERROR("Index buffer of name \"" + name + "\" doesn't exist!");
}

void IndexBufferHolder::deleteIndexBuffer(IndexBuffer ibo)
{
	if(ibo.mID == mRectangleIndexBuffer.mID)
		return;

	for(size_t i = 0; i < mIndexBuffers.size(); ++i)
	{
		if(mIndexBuffers[i].mID == ibo.mID) {
			--mReferenceCounters[i];
			mNames.erase(mNames.begin() + i);
			mReferenceCounters.erase(mReferenceCounters.begin() + i);
			mIndexBuffers.erase(mIndexBuffers.begin() + i);
			return;
		}
	}
	PH_LOG_WARNING("You're trying to delete index buffer which doesn't exist");
}

}
