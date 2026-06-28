#ifndef USE_DIRECTX11
// ================================================
// Renderer.cpp
// 描画インターフェースの実装
//
// 両プラットフォーム（NX64, Win64）でOpenGL 4.5を使用している。
// プラットフォーム固有の処理がある場合は、
// #if PLATFORM_NX / #elif PLATFORM_WIN64 で分岐する。
//
// このファイルでは描画に必要なOpenGLリソース（シェーダ、VAO、VBO）を
// 内部で管理し、外部には Renderer.h のインターフェースだけを公開する。
// ================================================

#include "Renderer.h"
#include "main.h"

// ================================================
// 内部変数（外部に公開しない）
// ================================================
namespace
{
	GLuint s_ShaderProgramId    = 0;
	GLuint s_VertexShaderId     = 0;
	GLuint s_FragmentShaderId   = 0;
	GLuint s_VertexArrayObject  = 0;
	GLuint s_VertexBuffer       = 0;

	// 一度に描画できる最大頂点数
	const int MAX_VERTICES = 256;

	// 画面オフセット（シェイク用）
	float s_OffsetX = 0.0f;
	float s_OffsetY = 0.0f;

	// 現在のテクスチャID（呼び出し側が保存・復元できるように追跡）
	unsigned int s_CurrentTextureId = 0;


	// ================================================
	// シェーダソース（両プラットフォーム共通）
	// ================================================

	// 頂点シェーダ
	const char* VertexShaderSource =
		"#version 450\n"
		"precision highp float;\n"
		"uniform mat4 uProjection;\n"
		"layout( location = 0 ) in vec3 inPosition;\n"
		"layout( location = 1 ) in vec4 inColor;\n"
		"layout( location = 2 ) in vec2 inTexCoord;\n"
		"out vec4 vColor;\n"
		"out vec2 vTexCoord;\n"
		"void main() {\n"
		"    vColor = inColor;\n"
		"    vTexCoord = inTexCoord;\n"
		"    gl_Position = vec4(inPosition, 1.0) * uProjection;\n"
		"}\n";

	// フラグメントシェーダ
	const char* FragmentShaderSource =
		"#version 450\n"
		"precision highp float;\n"
		"uniform sampler2D uSampler;\n"
		"uniform bool uTextureEnable;\n"
		"in vec4 vColor;\n"
		"in vec2 vTexCoord;\n"
		"out vec4 outColor;\n"
		"void main() {\n"
		"    if(uTextureEnable)\n"
		"        outColor = vColor * texture(uSampler, vTexCoord);\n"
		"    else\n"
		"        outColor = vColor;\n"
		"}\n";


	// ================================================
	// DrawMode → GLenum 変換
	// ================================================
	GLenum ToGLMode(DrawMode mode)
	{
		switch (mode)
		{
		case DrawMode::Triangles:     return GL_TRIANGLES;
		case DrawMode::TriangleStrip: return GL_TRIANGLE_STRIP;
		case DrawMode::Lines:         return GL_LINES;
		case DrawMode::LineLoop:      return GL_LINE_LOOP;
		default:                      return GL_TRIANGLES;
		}
	}
}


// ================================================
// 初期化
// ================================================
void Renderer::Initialize()
{
	// --- レンダーステート ---
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glDisable(GL_DEPTH_TEST);

	// --- シェーダコンパイル ---
	{
		GLint result;
		GLchar shaderLog[1024];
		GLsizei shaderLogSize;

		// 頂点シェーダ
		s_VertexShaderId = glCreateShader(GL_VERTEX_SHADER);
		PAL_ASSERT(s_VertexShaderId != 0, "Failed to create vertex shader");
		glShaderSource(s_VertexShaderId, 1, &VertexShaderSource, 0);
		glCompileShader(s_VertexShaderId);
		glGetShaderiv(s_VertexShaderId, GL_COMPILE_STATUS, &result);
		if (!result)
		{
			glGetShaderInfoLog(s_VertexShaderId, sizeof(shaderLog), &shaderLogSize, shaderLog);
			PAL_LOG("Failed to compile vertex shader: %s\n", shaderLog);
			PAL_ASSERT(false, "Vertex shader compilation failed");
		}

		// フラグメントシェーダ
		s_FragmentShaderId = glCreateShader(GL_FRAGMENT_SHADER);
		PAL_ASSERT(s_FragmentShaderId != 0, "Failed to create fragment shader");
		glShaderSource(s_FragmentShaderId, 1, &FragmentShaderSource, 0);
		glCompileShader(s_FragmentShaderId);
		glGetShaderiv(s_FragmentShaderId, GL_COMPILE_STATUS, &result);
		if (!result)
		{
			glGetShaderInfoLog(s_FragmentShaderId, sizeof(shaderLog), &shaderLogSize, shaderLog);
			PAL_LOG("Failed to compile fragment shader: %s\n", shaderLog);
			PAL_ASSERT(false, "Fragment shader compilation failed");
		}

		// リンク
		s_ShaderProgramId = glCreateProgram();
		PAL_ASSERT(s_ShaderProgramId != 0, "Failed to create shader program");
		glAttachShader(s_ShaderProgramId, s_VertexShaderId);
		glAttachShader(s_ShaderProgramId, s_FragmentShaderId);
		glLinkProgram(s_ShaderProgramId);
		glUseProgram(s_ShaderProgramId);
	}

	// --- プロジェクション行列 ---
	// nn::util と Win64 スタブで OrthographicRightHanded のメモリレイアウトが
	// 一致しないため、float[16] で直接組んでアップロードする
	{
		const float w = (float)SCREEN_WIDTH;
		const float h = (float)-SCREEN_HEIGHT;
		float fprojection[16] = {
			2.0f / w, 0.0f,     0.0f, 0.0f,
			0.0f,     2.0f / h, 0.0f, 0.0f,
			0.0f,     0.0f,    -1.0f, 0.0f,
		   -1.0f,     1.0f,     0.0f, 1.0f,
		};
		glUniformMatrix4fv(
			glGetUniformLocation(s_ShaderProgramId, "uProjection"),
			1, GL_TRUE, fprojection);
	}

	// --- VAO / VBO ---
	{
		glGenVertexArrays(1, &s_VertexArrayObject);
		glBindVertexArray(s_VertexArrayObject);

		glEnableVertexArrayAttrib(s_VertexArrayObject, 0);
		glEnableVertexArrayAttrib(s_VertexArrayObject, 1);
		glEnableVertexArrayAttrib(s_VertexArrayObject, 2);

		glVertexAttribFormat(0, 3, GL_FLOAT, GL_FALSE, 0);
		glVertexAttribFormat(1, 4, GL_FLOAT, GL_FALSE, sizeof(Float3));
		glVertexAttribFormat(2, 2, GL_FLOAT, GL_FALSE, sizeof(Float3) + sizeof(Float4));

		glVertexAttribBinding(0, 0);
		glVertexAttribBinding(1, 0);
		glVertexAttribBinding(2, 0);

		glGenBuffers(1, &s_VertexBuffer);
		glBindBuffer(GL_ARRAY_BUFFER, s_VertexBuffer);
		glBufferData(GL_ARRAY_BUFFER, sizeof(VERTEX_3D) * MAX_VERTICES, 0, GL_DYNAMIC_DRAW);

		glBindVertexArray(0);
	}
}


// ================================================
// 終了処理
// ================================================
void Renderer::Finalize()
{
	// シェーダ削除
	glDetachShader(s_ShaderProgramId, s_VertexShaderId);
	glDetachShader(s_ShaderProgramId, s_FragmentShaderId);
	glDeleteShader(s_VertexShaderId);
	glDeleteShader(s_FragmentShaderId);
	glDeleteProgram(s_ShaderProgramId);

	// VAO / VBO 削除
	glDeleteVertexArrays(1, &s_VertexArrayObject);
	glDeleteBuffers(1, &s_VertexBuffer);
}


// ================================================
// フレーム制御
// ================================================
void Renderer::BeginFrame()
{
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void Renderer::BeginFrame(float r, float g, float b)
{
	glClearColor(r, g, b, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void Renderer::EndFrame()
{
	SwapBuffers();
}


// ================================================
// 描画
// ================================================
void Renderer::DrawVertices(const VERTEX_3D* vertices, int count, DrawMode mode)
{
	// Apply screen offset (for shake etc.)
	if (s_OffsetX != 0.0f || s_OffsetY != 0.0f)
	{
		static VERTEX_3D offsetVerts[256];
		int n = (count < MAX_VERTICES) ? count : MAX_VERTICES;
		for (int i = 0; i < n; i++)
		{
			offsetVerts[i] = vertices[i];
			offsetVerts[i].Position.x += s_OffsetX;
			offsetVerts[i].Position.y += s_OffsetY;
		}
		glBindBuffer(GL_ARRAY_BUFFER, s_VertexBuffer);
		glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(VERTEX_3D) * n, offsetVerts);
	}
	else
	{
		glBindBuffer(GL_ARRAY_BUFFER, s_VertexBuffer);
		glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(VERTEX_3D) * count, vertices);
	}

	glBindVertexArray(s_VertexArrayObject);
	glBindVertexBuffer(0, s_VertexBuffer, 0, sizeof(VERTEX_3D));

	glDrawArrays(ToGLMode(mode), 0, count);

	glBindVertexArray(0);
}


// ================================================
// テクスチャ制御
// ================================================
void Renderer::SetTexture(unsigned int textureId)
{
	s_CurrentTextureId = textureId;
	if (textureId == 0)
	{
		glUniform1i(glGetUniformLocation(s_ShaderProgramId, "uTextureEnable"), 0);
		glBindTexture(GL_TEXTURE_2D, 0);
	}
	else
	{
		glUniform1i(glGetUniformLocation(s_ShaderProgramId, "uTextureEnable"), 1);
		glBindTexture(GL_TEXTURE_2D, textureId);
	}
}

unsigned int Renderer::GetCurrentTexture()
{
	return s_CurrentTextureId;
}


// ================================================
// 線の太さ
// ================================================
void Renderer::SetLineWidth(float width)
{
	glLineWidth(width);
}


// ================================================
// 画面オフセット
// ================================================
void Renderer::SetScreenOffset(float x, float y)
{
	s_OffsetX = x;
	s_OffsetY = y;
}

#endif // !USE_DIRECTX11
