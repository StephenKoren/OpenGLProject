#include "glm_header.h"
#include "AntTweakBar.h"
#include "gl_core_4_4.h"
#include <GLFW\glfw3.h>
#include "Gizmos.h"
#include "tiny_obj_loader.h"
#include "stb_image.h"
#include "Vertex.h"

//Convenience function for attaching a single shader to a program.
bool LoadShader(const char* a_filename, GLenum a_shaderType, unsigned int* a_output) {
	bool succeeded = true;

	FILE* shaderFile = fopen(a_filename, "r");

	//Ensure the file was found
	if (shaderFile == 0)
		return false;
	else {
		//Enumerate file length
		fseek(shaderFile, 0, SEEK_END);
		int shaderFileLen = ftell(shaderFile);
		fseek(shaderFile, 0, SEEK_SET);

		//Create buffer and update the file length
		char *shaderSource = new char[shaderFileLen];
		shaderFileLen = fread(shaderSource, 1, shaderFileLen, shaderFile);

		int logLength = 0;

		//Create the shader handle
		unsigned int shaderHandle = glCreateShader(a_shaderType);

		//Compile the shader
		glShaderSource(shaderHandle, 1, &shaderSource, &shaderFileLen);
		glCompileShader(shaderHandle);

		//Catch for errors
		int success = GL_FALSE;
		glGetShaderiv(shaderHandle, GL_COMPILE_STATUS, &success);

		if (success == GL_FALSE) {
			glGetShaderiv(shaderHandle, GL_INFO_LOG_LENGTH, &logLength);
			char* log = new char[logLength];
			glGetShaderInfoLog(shaderHandle, logLength, NULL, log);
			printf("%s\n", log);
			delete[] log;
			succeeded = false;
		}

		if (succeeded)
			*a_output = shaderHandle;

		delete[] shaderSource;
		fclose(shaderFile);
	}
	return succeeded;
}

//Convenience function for attaching vertex, geometry and fragment shaders to a program.
bool LoadShader(const char* a_vertexFileName, const char* a_geometryFileName, const char* a_fragmentFileName, GLuint* a_result) {
	bool succeeded = true;

	//Make the program
	*a_result = glCreateProgram();

	//Create each type of shader using the code within the file
	if (a_vertexFileName != nullptr && a_vertexFileName != 0) {
		unsigned int vertexShader;
		LoadShader(a_vertexFileName, GL_VERTEX_SHADER, &vertexShader);
		glAttachShader(*a_result, vertexShader);
		glDeleteShader(vertexShader);
	}

	if (a_geometryFileName != nullptr && a_geometryFileName != 0) {
		unsigned int geometryShader;
		LoadShader(a_geometryFileName, GL_GEOMETRY_SHADER, &geometryShader);
		glAttachShader(*a_result, geometryShader);
		glDeleteShader(geometryShader);
	}

	if (a_fragmentFileName != nullptr && a_fragmentFileName != 0) {
		unsigned int fragmentShader;
		LoadShader(a_fragmentFileName, GL_FRAGMENT_SHADER, &fragmentShader);
		glAttachShader(*a_result, fragmentShader);
		glDeleteShader(fragmentShader);
	}

	glLinkProgram(*a_result);

	//Error handling 
	GLint success;
	glGetProgramiv(*a_result, GL_LINK_STATUS, &success);
	if (success == GL_FALSE) {
		GLint log_length;
		glGetProgramiv(*a_result, GL_INFO_LOG_LENGTH, &log_length);

		char* log = new char[log_length];
		glGetProgramInfoLog(*a_result, log_length, 0, log);

		printf("Error: Failed to link shader program!\n");
		printf("%s\n", log);

		delete[] log;
		succeeded = false;
	}
	return succeeded;
}

void LoadTexture(const char* a_filename, unsigned int* a_textureHandle) {
	int width, height, channels;
	unsigned char* data = stbi_load(a_filename, &width, &height, &channels, STBI_default);

	glGenTextures(1, a_textureHandle);
	glBindTexture(GL_TEXTURE_2D, *a_textureHandle);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
}

OpenGLData LoadOBJ(const char* filename){
	OpenGLData result = {};

	std::vector<tinyobj::shape_t> shapes;
	std::vector<tinyobj::material_t> materials;

	tinyobj::LoadObj(shapes, materials, filename);

	result.m_indexCount = shapes[0].mesh.indices.size();

	tinyobj::mesh_t* mesh = &shapes[0].mesh;

	std::vector<float> vertexData;
	vertexData.reserve(mesh->positions.size() + mesh->normals.size());

	vertexData.insert(vertexData.begin(), mesh->positions.begin(), mesh->positions.end());
	vertexData.insert(vertexData.begin(), mesh->normals.begin(), mesh->normals.end());

	glGenVertexArrays(1, &result.m_VAO);
	glGenBuffers(1, &result.m_VBO);
	glGenBuffers(1, &result.m_IBO);
	glBindVertexArray(result.m_VAO);

	glBindBuffer(GL_ARRAY_BUFFER, result.m_VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(float)* vertexData.size(), vertexData.data(), GL_STATIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, result.m_IBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned int)* mesh->indices.size(), mesh->indices.data(), GL_STATIC_DRAW);

	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, (void*)(sizeof(float)* mesh->positions.size()));

	glBindVertexArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

	return result;
}

void RenderPlane(const vec4 a_plane) {
	vec3 up = vec3(0, 1, 0);
	if (a_plane.xyz() == vec3(0, 1, 0))
	{
		up = vec3(1, 0, 0);
	}

	vec3 tangent = glm::normalize(glm::cross(a_plane.xyz(), up));
	vec3 bitangent = glm::normalize(glm::cross(a_plane.xyz(), tangent));

	vec3 p = a_plane.xyz() * a_plane.w;

	vec3 v0 = p + tangent + bitangent;
	vec3 v1 = p + tangent - bitangent;
	vec3 v2 = p - tangent - bitangent;
	vec3 v3 = p - tangent + bitangent;

	Gizmos::addTri(v0, v1, v2, vec4(1, 1, 0, 1));
	Gizmos::addTri(v0, v2, v3, vec4(1, 1, 0, 1));

	//Gizmos::addLine(p, p + a_plane.xyz() * 2, vec4(0, 1, 1, 1));
}

void OnMouseButton(GLFWwindow* window, int button, int pressed, int altKeys) {
	TwEventMouseButtonGLFW(button, pressed);
}

void OnMousePosition(GLFWwindow* window, double x, double y) {
	TwEventMousePosGLFW((int)x, (int)y);
}

void OnMouseScroll(GLFWwindow* window, double x, double y) {
	TwEventMouseWheelGLFW((int)y);
}

void OnKey(GLFWwindow* window, int key, int scanCode, int pressed, int modKeys) {
	TwEventKeyGLFW(key, pressed);
}

void OnChar(GLFWwindow* window, unsigned int c) {
	TwEventCharGLFW(c, GLFW_PRESS);
}

void OnWindowResize(GLFWwindow* window, int width, int height) {
	TwWindowSize(width, height);
	glViewport(0, 0, width, height);
}