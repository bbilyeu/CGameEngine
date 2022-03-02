#ifndef SHADERS_H_INCLUDED
#define SHADERS_H_INCLUDED

namespace CGameEngine
{
	/// bindless texture array shaders ////////////////////////////////

	static const char* V450_VERT_SRC = R"(
	#version 450
	#extension GL_ARB_separate_shader_objects : enable // break up shaders ( https://www.khronos.org/registry/OpenGL/extensions/ARB/ARB_separate_shader_objects.txt )
	#extension GL_ARB_shader_storage_buffer_object : require // SHADER_STORAGE_BUFFER

	// uniforms / shader_storage_buffer object
	layout(std140, binding = 0) buffer CB0 { mat4 Transforms[]; };

	// view & projection in one
	uniform mat4 ViewProjection;

	// input
	layout(location = 0) in vec3 In_v3Pos;
	layout(location = 1) in vec2 In_v2TexCoord;
	layout(location = 2) in uint In_uiDrawID;

	// output
	out DrawBlock
	{
		vec2 v2TexCoord;
		flat uint iDrawID;
	} Out;

	void main()
	{
		mat4 World = Transforms[In_uiDrawID];
		vec4 worldPos = World * vec4(In_v3Pos, 1.0);
		gl_Position = ViewProjection * worldPos;

		// inverting y-axis so images are drawn "naturally" (bottom left = 0,0)
		//Out.v2TexCoord = vec2(In_v2TexCoord.x, 1.0 - In_v2TexCoord.y);
		Out.v2TexCoord = In_v2TexCoord;
		Out.iDrawID = In_uiDrawID;
	})";

	static const char* V450_FRAG_SRC = R"(
	#version 450
	#extension GL_ARB_shader_storage_buffer_object : require
	#extension GL_ARB_bindless_texture : require

	struct TexAddress
	{
        sampler2DArray arr;
        float slice;
	};

	layout (std430, binding = 1) buffer CB1 { TexAddress texAddress[]; };

	// input
	in DrawBlock
	{
		vec2 v2TexCoord;
		flat uint iDrawID;
	} In;


	// output
	layout(location = 0) out vec4 Out_v4Color;

	vec4 Texture(TexAddress addr, vec2 uv) { return texture(addr.arr, vec3(uv, addr.slice)); }

	void main()
	{
		int DrawID = int(In.iDrawID);
		Out_v4Color = vec4(Texture(texAddress[DrawID], In.v2TexCoord).xyz, 1.0f);
		//Out_v4Color = vec4(255, 255, 255, 255); // white
	})";

	/// debug renderer shaders ////////////////////////////////////////////////

	static const char* DEBUG_RENDERER_VERT_SRC = R"(
    #version 330

    layout(location = 0) in vec3 vertexPosition;
    layout(location = 1) in vec4 vertexColor;
    uniform mat4 projection;

    out Vertex
	{
		vec3 v3Pos;
		vec4 v4Color;
	} Out;

    void main() {
        // set the x,y,z position on the screen
        gl_Position = projection * vec4(vertexPosition, 1.0);

        //fragmentPosition = vertexPosition;
        Out.v4Color = vertexColor;
    })";

    static const char* DEBUG_RENDERER_FRAG_SRC = R"(
    #version 330
    // the fragment shader operates on each pixel in a given polygon

    in Vertex
	{
		vec3 v3Pos;
		vec4 v4Color;
	} In;

    // this is the 3 component float vector that gets output to the screen for each pixel
    out vec4 color;

    void main()
    {
		vec4 newColor;
		newColor.r = float(In.v4Color.r / 255.0f);
		newColor.g = float(In.v4Color.g / 255.0f);
		newColor.b = float(In.v4Color.b / 255.0f);
		newColor.a = 1.0f;
		color = newColor;
	})";
    //void main() { color = vec4(1.0f, 1.0f, 1.0f, 1.0f); })";

	/// default shaders ///////////////////////////////////////////////////////

    static const char* DEFAULT_VERT_SRC = R"(
    #version 330
    // vertex shader

    // input data from the VBO. Each vertex is 2 floats
    layout(location = 0) in vec2 vertexPosition;
    layout(location = 1) in vec4 vertexColor;
    layout(location = 2) in vec2 vertexUV;

    out vec2 fragmentPosition;
    out vec4 fragmentColor;
    out vec2 fragmentUV;

    // matrices
    uniform mat4 projection;
    //uniform mat4 model;

    void main()
    {
        // Set the x,y position on the screen
        //gl_Position = translation * rotation * scaling * vec4(vertexPosition.xy, 0.0, 1.0);
        //gl_Position = translation * rotation * vec4(vertexPosition.xy, 0.0, 1.0);
        //gl_Position = projection * model * vec4(vertexPosition, 0.0f, 1.0f);
        gl_Position = projection * vec4(vertexPosition, 0.0f, 1.0f);

        // the z position is zero since we are in 2D
        //gl_Position.z = 0.0f;

        //Indicate that the coordinates are normalized
        //gl_Position.w = 1.0f;

        fragmentPosition = vertexPosition;
        fragmentColor = vertexColor;
        fragmentUV = vec2(vertexUV.x, 1.0f - vertexUV.y);
    })";

    static const char* DEFAULT_FRAG_SRC = R"(
    #version 330
    // fragment shader

    in vec2 fragmentPosition;
    in vec4 fragmentColor;
    in vec2 fragmentUV;

    // This is the 3 component float vector that
    // gets outputted to the screen for each pixel.
    out vec4 color;

    uniform sampler2D image;

    void main()
    {
        color = fragmentColor * texture(image, fragmentUV);
    })";

    /// gui shaders ///////////////////////////////////////////////////////////

    static const char* DEFAULT_GUI_VERT_SRC = R"(
    #version 330 core
    // GUI vertex shader

    // input data from the VBO. Each vertex is 2 floats
    layout(location = 0) in vec2 vertexPosition;
    layout(location = 1) in vec4 vertexColor;
    layout(location = 2) in vec2 vertexUV;

    out vec2 fragmentPosition;
    out vec4 fragmentColor;
    out vec2 fragmentUV;

    // projection matrix
    uniform mat4 p;

    void main()
    {
        // Set the x,y position on the screen
        gl_Position = p * vec4(vertexPosition, 0.0f, 1.0f);

        fragmentPosition = vertexPosition;
        fragmentColor = vertexColor;
        fragmentUV = vec2(vertexUV.x, 1.0f - vertexUV.y);
    })";
}

#endif // SHADERS_H_INCLUDED
