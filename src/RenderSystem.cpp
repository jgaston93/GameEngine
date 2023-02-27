#include <vector>
#include <algorithm>
#include <cstdio>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include <glad/gl.h>
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

#include "RenderSystem.hpp"

static const char* quad_vertex_shader_text =
"#version 330\n"
"uniform mat4 M;\n"
"uniform mat4 V;\n"
"uniform mat4 P;\n"
"layout(location = 0)in vec3 vPos;\n"
"layout(location = 1)in vec2 vTexCoord;\n"
"out vec2 fTexCoord;\n"
"out vec4 fFragPos;\n"
"void main()\n"
"{\n"
"    gl_Position = P * V * M * vec4(vPos, 1.0);\n"
"    fTexCoord = vTexCoord;\n"
"    fFragPos = M * vec4(vPos, 1.0);\n"
"}\n";
 
static const char* quad_fragment_shader_text =
"#version 330\n"
"uniform bool use_light;\n"
"in vec2 fTexCoord\n;"
"in vec4 fFragPos;\n"
"out vec4 fragColor;\n"
"uniform sampler2D quadTexture;\n"
"void main()\n"
"{\n"
"    vec3 light_pos[25];\n"
"    light_pos[0] = vec3(-10.0, 2.5, 0.0);\n"
"    light_pos[1] = vec3(-5.0, 2.5, 0.0);\n"
"    light_pos[2] = vec3(0.0, 2.5, 0.0);\n"
"    light_pos[3] = vec3(5.0, 2.5, 0.0);\n"
"    light_pos[4] = vec3(10.0, 2.5, 0.0);\n"
"    light_pos[5] = vec3(-10.0, 5, -10.0);\n"
"    light_pos[6] = vec3(-5.0, 5, -10.0);\n"
"    light_pos[7] = vec3(0.0, 5, -10.0);\n"
"    light_pos[8] = vec3(5.0, 5, -10.0);\n"
"    light_pos[9] = vec3(10.0, 5, -10.0);\n"
"    light_pos[10] = vec3(-10.0, 2.5, -10.0);\n"
"    light_pos[11] = vec3(-5.0, 2.5, -10.0);\n"
"    light_pos[12] = vec3(0.0, 2.5, -10.0);\n"
"    light_pos[13] = vec3(5.0, 2.5, -10.0);\n"
"    light_pos[14] = vec3(10.0, 2.5, -10.0);\n"
"    light_pos[15] = vec3(-10.0, 0, -10.0);\n"
"    light_pos[16] = vec3(-5.0, 0, -10.0);\n"
"    light_pos[17] = vec3(0.0, 0, -10.0);\n"
"    light_pos[18] = vec3(5.0, 0, -10.0);\n"
"    light_pos[19] = vec3(10.0, 0, -10.0);\n"
"    light_pos[20] = vec3(-10.0, -2.5, -10.0);\n"
"    light_pos[21] = vec3(-5.0, -2.5, -10.0);\n"
"    light_pos[22] = vec3(0.0, -2.5, -10.0);\n"
"    light_pos[23] = vec3(5.0, -2.5, -10.0);\n"
"    light_pos[24] = vec3(10.0, -2.5, -10.0);\n"
"    float diffuseStrength = 0.0;\n"
"    for(int i = 0; i < 25; i++)\n"
"    {\n"
"       float distance = sqrt(pow(fFragPos.x - light_pos[i].x, 2) + pow(fFragPos.y - light_pos[i].y, 2) + pow(fFragPos.z - light_pos[i].z, 2));\n"
"       if(distance < 3)\n"
"       {\n"
"           float diffuse = 1 - (distance / 3);\n"
"           if(diffuse > diffuseStrength) diffuseStrength = diffuse;\n"
"       }\n"
"    }\n"
"    float ambientStrength = 0.1;\n"
"    diffuseStrength = clamp(diffuseStrength, 0.0, 1.0);\n"
"    vec3 diffuse = diffuseStrength * vec3(0.5, 0.5, 0.5);\n"
"    vec3 ambient = ambientStrength * vec3(1.0, 1.0, 1.0);\n"
"    if(use_light)\n"
"    {\n"
"       fragColor = (vec4(ambient, 1.0) + vec4(diffuse, 1.0)) * texture(quadTexture, fTexCoord);\n"
"    }\n"
"    else\n"
"    {\n"
"       fragColor = vec4(ambient, 1.0) * texture(quadTexture, fTexCoord);\n"
"    }\n"
"    if(fragColor.w == 0) discard;\n"
"}\n";

static const char* xray_vertex_shader_text =
"#version 330\n"
"uniform mat4 M;\n"
"uniform mat4 V;\n"
"uniform mat4 P;\n"
"layout(location = 0)in vec3 vPos;\n"
"layout(location = 1)in vec2 vTexCoord;\n"
"out vec2 fTexCoord;\n"
"out vec4 fFragPos;\n"
"out vec4 fFragPosCamera;\n"
"void main()\n"
"{\n"
"    gl_Position = P * V * M * vec4(vPos, 1.0);\n"
"    fTexCoord = vTexCoord;\n"
"    fFragPos = M * vec4(vPos, 1.0);\n"
"    fFragPosCamera = V * M * vec4(vPos, 1.0);\n"
"}\n";
 
static const char* xray_fragment_shader_text =
"#version 330\n"
"uniform bool use_light;\n"
"uniform sampler2D quadTexture;\n"
"out vec4 fNormal;\n"
"in vec2 fTexCoord\n;"
"in vec4 fFragPos;\n"
"in vec4 fFragPosCamera;\n"
"out vec4 fragColor;\n"
"void main()\n"
"{\n"
"    vec3 light_pos[25];\n"
"    light_pos[0] = vec3(-10.0, 2.5, 0.0);\n"
"    light_pos[1] = vec3(-5.0, 2.5, 0.0);\n"
"    light_pos[2] = vec3(0.0, 2.5, 0.0);\n"
"    light_pos[3] = vec3(5.0, 2.5, 0.0);\n"
"    light_pos[4] = vec3(10.0, 2.5, 0.0);\n"
"    light_pos[5] = vec3(-10.0, 5, -10.0);\n"
"    light_pos[6] = vec3(-5.0, 5, -10.0);\n"
"    light_pos[7] = vec3(0.0, 5, -10.0);\n"
"    light_pos[8] = vec3(5.0, 5, -10.0);\n"
"    light_pos[9] = vec3(10.0, 5, -10.0);\n"
"    light_pos[10] = vec3(-10.0, 2.5, -10.0);\n"
"    light_pos[11] = vec3(-5.0, 2.5, -10.0);\n"
"    light_pos[12] = vec3(0.0, 2.5, -10.0);\n"
"    light_pos[13] = vec3(5.0, 2.5, -10.0);\n"
"    light_pos[14] = vec3(10.0, 2.5, -10.0);\n"
"    light_pos[15] = vec3(-10.0, 0, -10.0);\n"
"    light_pos[16] = vec3(-5.0, 0, -10.0);\n"
"    light_pos[17] = vec3(0.0, 0, -10.0);\n"
"    light_pos[18] = vec3(5.0, 0, -10.0);\n"
"    light_pos[19] = vec3(10.0, 0, -10.0);\n"
"    light_pos[20] = vec3(-10.0, -2.5, -10.0);\n"
"    light_pos[21] = vec3(-5.0, -2.5, -10.0);\n"
"    light_pos[22] = vec3(0.0, -2.5, -10.0);\n"
"    light_pos[23] = vec3(5.0, -2.5, -10.0);\n"
"    light_pos[24] = vec3(10.0, -2.5, -10.0);\n"
"    float diffuseStrength = 0.0;\n"
"    for(int i = 0; i < 25; i++)\n"
"    {\n"
"       float distance = sqrt(pow(fFragPos.x - light_pos[i].x, 2) + pow(fFragPos.y - light_pos[i].y, 2) + pow(fFragPos.z - light_pos[i].z, 2));\n"
"       if(distance < 3)\n"
"       {\n"
"           float diffuse = 1 - (distance / 3);\n"
"           if(diffuse > diffuseStrength) diffuseStrength = diffuse;\n"
"       }\n"
"    }\n"
"    float ambientStrength = 0.1;\n"
"    diffuseStrength = clamp(diffuseStrength, 0.0, 1.0);\n"
"    vec3 diffuse = diffuseStrength * vec3(0.5, 0.5, 0.5);\n"
"    vec3 ambient = ambientStrength * vec3(1.0, 1.0, 1.0);\n"

"    float vision_angle = radians(15.0);\n"
"    float camera_distance = sqrt(pow(fFragPosCamera.x, 2) + pow(fFragPosCamera.y, 2) + pow(fFragPosCamera.z, 2));\n"
"    float radius = tan(vision_angle) * camera_distance;\n"
"    if(radius > .5) radius = .5;\n"
"    vec3 look_center = vec3(0.0, 0.0, -camera_distance);\n"
"    float look_center_distance = sqrt(pow(look_center.x - fFragPosCamera.x, 2) + pow(look_center.y - fFragPosCamera.y, 2) + pow(look_center.z - fFragPosCamera.z, 2));\n"
"    float opacity = 1.0;\n"
"    if(look_center_distance < radius) discard;\n"

"    if(use_light)\n"
"    {\n"
"       fragColor = (vec4(ambient, 1.0) + vec4(diffuse, 1.0)) * texture(quadTexture, fTexCoord);\n"
"    }\n"
"    else\n"
"    {\n"
"       fragColor = vec4(ambient, 1.0) * texture(quadTexture, fTexCoord);\n"
"    }\n"
"    if(fragColor.w == 0) discard;\n"
"}\n";

struct VertexData
{
    float x;
    float y;
    float z;
    float u;
    float v;
};

RenderSystem::RenderSystem(MessageBus& message_bus, InputMap& input_map) : 
    System(message_bus, RENDER_SYSTEM_SIGNATURE),
    m_input_map(input_map),
    m_zoom_on(false),
    m_xray_on(false)
{
    uint32_t texture_index = 0;
    glGenTextures(1, &m_textures[texture_index]);
    glBindTexture(GL_TEXTURE_2D, m_textures[texture_index]);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    int32_t num_channels;
    stbi_set_flip_vertically_on_load(true);
    unsigned char *data = stbi_load("assets/BrickTexture.png", &m_texture_sizes[texture_index][0], &m_texture_sizes[texture_index][1], &num_channels, 0);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, m_texture_sizes[texture_index][0], m_texture_sizes[texture_index][1], 0, GL_RGB, GL_UNSIGNED_BYTE, data);
    glGenerateMipmap(GL_TEXTURE_2D);
    stbi_image_free(data);
    
    texture_index++;
    glGenTextures(1, &m_textures[texture_index]);
    glBindTexture(GL_TEXTURE_2D, m_textures[texture_index]);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    data = stbi_load("assets/AtlasTexture.png", &m_texture_sizes[texture_index][0], &m_texture_sizes[texture_index][1], &num_channels, 0);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, m_texture_sizes[texture_index][0], m_texture_sizes[texture_index][1], 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
    glGenerateMipmap(GL_TEXTURE_2D);
    stbi_image_free(data);
    
    texture_index++;
    glGenTextures(1, &m_textures[texture_index]);
    glBindTexture(GL_TEXTURE_2D, m_textures[texture_index]);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    data = stbi_load("assets/WallTexture.png", &m_texture_sizes[texture_index][0], &m_texture_sizes[texture_index][1], &num_channels, 0);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, m_texture_sizes[texture_index][0], m_texture_sizes[texture_index][1], 0, GL_RGB, GL_UNSIGNED_BYTE, data);
    glGenerateMipmap(GL_TEXTURE_2D);
    stbi_image_free(data);
    
    texture_index++;
    glGenTextures(1, &m_textures[texture_index]);
    glBindTexture(GL_TEXTURE_2D, m_textures[texture_index]);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    data = stbi_load("assets/CarpetTexture.png", &m_texture_sizes[texture_index][0], &m_texture_sizes[texture_index][1], &num_channels, 0);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, m_texture_sizes[texture_index][0], m_texture_sizes[texture_index][1], 0, GL_RGB, GL_UNSIGNED_BYTE, data);
    glGenerateMipmap(GL_TEXTURE_2D);
    stbi_image_free(data);
    
    texture_index++;
    glGenTextures(1, &m_textures[texture_index]);
    glBindTexture(GL_TEXTURE_2D, m_textures[texture_index]);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    data = stbi_load("assets/CeilingTexture.png", &m_texture_sizes[texture_index][0], &m_texture_sizes[texture_index][1], &num_channels, 0);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, m_texture_sizes[texture_index][0], m_texture_sizes[texture_index][1], 0, GL_RGB, GL_UNSIGNED_BYTE, data);
    glGenerateMipmap(GL_TEXTURE_2D);
    stbi_image_free(data);
    
    texture_index++;
    glGenTextures(1, &m_textures[texture_index]);
    glBindTexture(GL_TEXTURE_2D, m_textures[texture_index]);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    data = stbi_load("assets/GroundTextureAtlas.png", &m_texture_sizes[texture_index][0], &m_texture_sizes[texture_index][1], &num_channels, 0);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, m_texture_sizes[texture_index][0], m_texture_sizes[texture_index][1], 0, GL_RGB, GL_UNSIGNED_BYTE, data);
    glGenerateMipmap(GL_TEXTURE_2D);
    stbi_image_free(data);
    
    texture_index++;
    glGenTextures(1, &m_textures[texture_index]);
    glBindTexture(GL_TEXTURE_2D, m_textures[texture_index]);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    data = stbi_load("assets/EnemyTexture.png", &m_texture_sizes[texture_index][0], &m_texture_sizes[texture_index][1], &num_channels, 0);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, m_texture_sizes[texture_index][0], m_texture_sizes[texture_index][1], 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
    glGenerateMipmap(GL_TEXTURE_2D);
    stbi_image_free(data);
    
    // Normal Shader
    GLuint quad_render_vertex_shader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(quad_render_vertex_shader, 1, &quad_vertex_shader_text, NULL);
    glCompileShader(quad_render_vertex_shader);

    GLint isCompiled = 0;
    glGetShaderiv(quad_render_vertex_shader, GL_COMPILE_STATUS, &isCompiled);
    if(isCompiled == GL_FALSE)
    {
        GLint maxLength = 0;
        glGetShaderiv(quad_render_vertex_shader, GL_INFO_LOG_LENGTH, &maxLength);

        // The maxLength includes the NULL character
        std::vector<GLchar> errorLog(maxLength);
        glGetShaderInfoLog(quad_render_vertex_shader, maxLength, &maxLength, &errorLog[0]);
        printf("%s\n", errorLog.data());

        // Provide the infolog in whatever manor you deem best.
        // Exit with failure.
        glDeleteShader(quad_render_vertex_shader); // Don't leak the shader.
        return;
    }
 
    GLuint quad_render_fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(quad_render_fragment_shader, 1, &quad_fragment_shader_text, NULL);
    glCompileShader(quad_render_fragment_shader);

    isCompiled = 0;
    glGetShaderiv(quad_render_fragment_shader, GL_COMPILE_STATUS, &isCompiled);
    if(isCompiled == GL_FALSE)
    {
        GLint maxLength = 0;
        glGetShaderiv(quad_render_fragment_shader, GL_INFO_LOG_LENGTH, &maxLength);

        // The maxLength includes the NULL character
        std::vector<GLchar> errorLog(maxLength);
        glGetShaderInfoLog(quad_render_fragment_shader, maxLength, &maxLength, &errorLog[0]);
        printf("%s\n", errorLog.data());

        // Provide the infolog in whatever manor you deem best.
        // Exit with failure.
        glDeleteShader(quad_render_fragment_shader); // Don't leak the shader.
        return;
    }
 
    m_shader_program = glCreateProgram();
    glAttachShader(m_shader_program, quad_render_vertex_shader);
    glAttachShader(m_shader_program, quad_render_fragment_shader);
    glLinkProgram(m_shader_program);
    
    GLint status; 
    glGetProgramiv( m_shader_program, GL_LINK_STATUS, &status ); 
    if( GL_FALSE == status ) {
        GLint maxLength = 0;
        glGetProgramiv(m_shader_program, GL_INFO_LOG_LENGTH, &maxLength);

        // The maxLength includes the NULL character
        std::vector<GLchar> errorLog(maxLength);
        glGetProgramInfoLog(m_shader_program, maxLength, &maxLength, &errorLog[0]);
        printf("%s\n", errorLog.data());

        // Provide the infolog in whatever manor you deem best.
        // Exit with failure.
        glDeleteProgram(m_shader_program); // Don't leak the shader.
        return;
    }

    glDeleteShader(quad_render_vertex_shader);
    glDeleteShader(quad_render_fragment_shader);

    GLuint xray_vertex_shader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(xray_vertex_shader, 1, &xray_vertex_shader_text, NULL);
    glCompileShader(xray_vertex_shader);

    isCompiled = 0;
    glGetShaderiv(xray_vertex_shader, GL_COMPILE_STATUS, &isCompiled);
    if(isCompiled == GL_FALSE)
    {
        GLint maxLength = 0;
        glGetShaderiv(xray_vertex_shader, GL_INFO_LOG_LENGTH, &maxLength);

        // The maxLength includes the NULL character
        std::vector<GLchar> errorLog(maxLength);
        glGetShaderInfoLog(xray_vertex_shader, maxLength, &maxLength, &errorLog[0]);
        printf("%s\n", errorLog.data());

        // Provide the infolog in whatever manor you deem best.
        // Exit with failure.
        glDeleteShader(xray_vertex_shader); // Don't leak the shader.
        return;
    }
 
    GLuint xray_fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(xray_fragment_shader, 1, &xray_fragment_shader_text, NULL);
    glCompileShader(xray_fragment_shader);

    isCompiled = 0;
    glGetShaderiv(xray_fragment_shader, GL_COMPILE_STATUS, &isCompiled);
    if(isCompiled == GL_FALSE)
    {
        GLint maxLength = 0;
        glGetShaderiv(xray_fragment_shader, GL_INFO_LOG_LENGTH, &maxLength);

        // The maxLength includes the NULL character
        std::vector<GLchar> errorLog(maxLength);
        glGetShaderInfoLog(xray_fragment_shader, maxLength, &maxLength, &errorLog[0]);
        printf("%s\n", errorLog.data());

        // Provide the infolog in whatever manor you deem best.
        // Exit with failure.
        glDeleteShader(xray_fragment_shader); // Don't leak the shader.
        return;
    }
 
    m_xray_program = glCreateProgram();
    glAttachShader(m_xray_program, xray_vertex_shader);
    glAttachShader(m_xray_program, xray_fragment_shader);
    glLinkProgram(m_xray_program);
    
    status; 
    glGetProgramiv( m_xray_program, GL_LINK_STATUS, &status ); 
    if( GL_FALSE == status ) {
        GLint maxLength = 0;
        glGetProgramiv(m_xray_program, GL_INFO_LOG_LENGTH, &maxLength);

        // The maxLength includes the NULL character
        std::vector<GLchar> errorLog(maxLength);
        glGetProgramInfoLog(m_xray_program, maxLength, &maxLength, &errorLog[0]);
        printf("%s\n", errorLog.data());

        // Provide the infolog in whatever manor you deem best.
        // Exit with failure.
        glDeleteProgram(m_xray_program); // Don't leak the shader.
        return;
    }

    glDeleteShader(xray_vertex_shader);
    glDeleteShader(xray_fragment_shader);
    
    m_m_location = glGetUniformLocation(m_shader_program, "M");
    m_v_location = glGetUniformLocation(m_shader_program, "V");
    m_p_location = glGetUniformLocation(m_shader_program, "P");
    
    m_xray_m_location = glGetUniformLocation(m_xray_program, "M");
    m_xray_v_location = glGetUniformLocation(m_xray_program, "V");
    m_xray_p_location = glGetUniformLocation(m_xray_program, "P");
}

RenderSystem::~RenderSystem()
{

}

void RenderSystem::HandleMessage(Message message)
{
    if(message.message_type == MessageType::XRAY)
    {
        m_xray_on = message.message_data;
    }
    else if(message.message_type == MessageType::ZOOM)
    {
        m_zoom_on = message.message_data;
    }
}

void RenderSystem::HandleEntity(uint32_t entity_id, float delta_time)
{
    Transform& transform = m_component_manager->GetComponent<Transform>(entity_id);
    Quad& quad = m_component_manager->GetComponent<Quad>(entity_id);
    Texture& texture = m_component_manager->GetComponent<Texture>(entity_id);

    float half_width = quad.extent[0] / 2;
    float half_height = quad.extent[1] / 2;

    float u1 = texture.position[0] / m_texture_sizes[texture.texture_index][0];
    float v1 = texture.position[1] / m_texture_sizes[texture.texture_index][1];
    float u2 = (texture.position[0] + texture.size[0]) / m_texture_sizes[texture.texture_index][0];
    float v2 = (texture.position[1] + texture.size[1]) / m_texture_sizes[texture.texture_index][1];

    VertexData vertices[4];
    vertices[0] = { -half_width,      half_height,      0, 
                    u1,               v2 };
    vertices[1] = { -half_width,      -half_height,     0, 
                    u1,               v1 };
    vertices[2] = {  half_width,      half_height,      0, 
                    u2,               v2 };
    vertices[3] = {  half_width,      -half_height,     0, 
                    u2,               v1 };

    // Rotation
    mat4x4 rotation_matrix;
    mat4x4_identity(rotation_matrix);
    mat4x4_rotate_Z(rotation_matrix, rotation_matrix, transform.rotation[2] * M_PI / 180.0);
    mat4x4_rotate_Y(rotation_matrix, rotation_matrix, transform.rotation[1] * M_PI / 180.0);
    mat4x4_rotate_X(rotation_matrix, rotation_matrix, transform.rotation[0] * M_PI / 180.0);

    // Translation
    mat4x4 translation_matrix;
    mat4x4_identity(translation_matrix);
    mat4x4_identity(translation_matrix);
    mat4x4_translate(translation_matrix, transform.position[0], transform.position[1], transform.position[2]);

    // Model Matrix
    mat4x4 model_matrix;
    mat4x4_mul(model_matrix, translation_matrix, rotation_matrix);
                
    // View Matrix
    mat4x4 view_matrix;
    mat4x4_look_at(view_matrix, m_eye, m_look, m_up);
    
    // Perspective Matrix
    mat4x4 perspective_matrix;
    if(m_zoom_on)
    {
        mat4x4_perspective(perspective_matrix, 30 * M_PI / 180.0, 4 / 3, 0.1, 300);
    }
    else
    {
        mat4x4_perspective(perspective_matrix, 60 * M_PI / 180.0, 4 / 3, 0.1, 300);
    }

    glBindTexture(GL_TEXTURE_2D, m_textures[texture.texture_index]);

    if(m_entity_manager->GetEntitySignature(entity_id) & XRAY_SYSTEM_SIGNATURE && m_xray_on)
    {
        glUseProgram(m_xray_program);
        glUniformMatrix4fv(m_xray_m_location, 1, GL_FALSE, (const GLfloat*) model_matrix);
        glUniformMatrix4fv(m_xray_v_location, 1, GL_FALSE, (const GLfloat*) view_matrix);
        glUniformMatrix4fv(m_xray_p_location, 1, GL_FALSE, (const GLfloat*) perspective_matrix);
        uint32_t use_light_location = glGetUniformLocation(m_xray_program, "use_light");
        glUniform1ui(use_light_location, texture.use_light);
    }
    else
    {
        glUseProgram(m_shader_program);
        glUniformMatrix4fv(m_m_location, 1, GL_FALSE, (const GLfloat*) model_matrix);
        glUniformMatrix4fv(m_v_location, 1, GL_FALSE, (const GLfloat*) view_matrix);
        glUniformMatrix4fv(m_p_location, 1, GL_FALSE, (const GLfloat*) perspective_matrix);
        uint32_t use_light_location = glGetUniformLocation(m_shader_program, "use_light");
        glUniform1ui(use_light_location, texture.use_light);
    }

    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), &vertices, GL_DYNAMIC_DRAW);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
}


void RenderSystem::Update(float delta_time)
{
    uint32_t camera_entity_id = m_entity_manager->GetEntityId("player");
    Transform& camera_transform = m_component_manager->GetComponent<Transform>(camera_entity_id);

    m_eye[0] = camera_transform.position[0];
    m_eye[1] = camera_transform.position[1];
    m_eye[2] = camera_transform.position[2];

    // Rotation
    mat4x4 rotation_matrix;
    mat4x4_identity(rotation_matrix);
    mat4x4_rotate_Z(rotation_matrix, rotation_matrix, camera_transform.rotation[2] * M_PI / 180.0);
    mat4x4_rotate_Y(rotation_matrix, rotation_matrix, camera_transform.rotation[1] * M_PI / 180.0);
    mat4x4_rotate_X(rotation_matrix, rotation_matrix, camera_transform.rotation[0] * M_PI / 180.0);

    vec4 forward = { 0, 0, -1 , 0};
    vec4 result;

    mat4x4_mul_vec4(result, rotation_matrix, forward);

    m_look[0] = m_eye[0] + result[0];
    m_look[1] = m_eye[1] + result[1];
    m_look[2] = m_eye[2] + result[2];
 
    int32_t vpos_location = glGetAttribLocation(m_shader_program, "vPos");
    int32_t vtexcoord_location = glGetAttribLocation(m_shader_program, "vTexCoord");
 
    glEnableVertexAttribArray(vpos_location);
    glVertexAttribPointer(vpos_location, 3, GL_FLOAT, GL_FALSE,
                          sizeof(VertexData), (void*) 0);
    glEnableVertexAttribArray(vtexcoord_location);
    glVertexAttribPointer(vtexcoord_location, 2, GL_FLOAT, GL_FALSE,
                          sizeof(VertexData), (void*) (sizeof(float) * 3));

    uint32_t num_entities = m_entity_manager->GetNumEntities();
    
    for(uint32_t i = 0; i < num_entities; i++)
    {
        if(m_entity_manager->GetEntityState(i) == EntityState::ACTIVE &&
            (m_entity_manager->GetEntitySignature(i) & m_system_signature))
        {
            HandleEntity(i, delta_time);
        }
    }
}