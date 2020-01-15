R"(
#version 330 core

layout (location = 0) in vec2 aPos;
layout (location = 1) in vec2 aTexCoords;

out vec2 texCoords;

layout (std140) uniform SharedData
{
	mat4 viewProjectionMatrix;
	mat4 guiViewProjectionMatrix;
};

void main()
{
	texCoords = aTexCoords;
	gl_Position = guiViewProjectionMatrix * vec4(aPos, 0, 1);	
}

)"
