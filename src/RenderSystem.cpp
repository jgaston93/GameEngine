#include <vector>
#include <algorithm>
#include <cstdio>

#include <glad/gl.h>
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

#include "RenderSystem.hpp"

static const char* quad_vertex_shader_text =
"#version 330\n"
"uniform mat4 MVP;\n"
"layout(location = 0)in vec3 vPos;\n"
"layout(location = 1)in vec3 vColor;\n"
"out vec3 fColor;\n"
"void main()\n"
"{\n"
"    gl_Position = MVP * vec4(vPos, 1.0);\n"
"    fColor = vColor;\n"
"}\n";
 
static const char* quad_fragment_shader_text =
"#version 330\n"
"in vec3 fColor\n;"
"out vec3 fragColor;\n"
"void main()\n"
"{\n"
"    fragColor = fColor;\n"
"}\n";

RenderSystem::RenderSystem(MessageBus& message_bus) : 
    System(message_bus, RENDER_SYSTEM_SIGNATURE)
{
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
    glDeleteShader(quad_render_vertex_shader);
    glDeleteShader(quad_render_fragment_shader);
    
    m_mvp_location = glGetUniformLocation(m_shader_program, "MVP");
}

RenderSystem::~RenderSystem()
{

}

void RenderSystem::HandleMessage(Message message)
{

}

void RenderSystem::HandleEntity(uint32_t entity_id, float delta_time)
{
    Transform& transform = m_component_manager->GetComponent<Transform>(entity_id);
    Quad& quad = m_component_manager->GetComponent<Quad>(entity_id);
    Texture& texture = m_component_manager->GetComponent<Texture>(entity_id);

    float half_width = quad.extent[0] / 2;
    float half_height = quad.extent[1] / 2;

    VertexData vertices[4];
    vertices[0] = { -half_width,  half_height, 0, texture.color[0], texture.color[1], texture.color[2] };
    vertices[1] = { -half_width, -half_height, 0, texture.color[0], texture.color[1], texture.color[2] };
    vertices[2] = {  half_width,  half_height, 0, texture.color[0], texture.color[1], texture.color[2] };
    vertices[3] = {  half_width, -half_height, 0, texture.color[0], texture.color[1], texture.color[2] };

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
    mat4x4_perspective(perspective_matrix, 65 * M_PI / 180.0, 4 / 3, 1, 300);

    // Model View Perspective Matrix
    mat4x4 model_view_matrix;
    mat4x4_mul(model_view_matrix, view_matrix, model_matrix);
    mat4x4_mul(model_view_matrix, perspective_matrix, model_view_matrix);

    glUniformMatrix4fv(m_mvp_location, 1, GL_FALSE, (const GLfloat*) model_view_matrix);

    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), &vertices, GL_DYNAMIC_DRAW);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
}


void RenderSystem::Update(float delta_time)
{
    uint32_t camera_entity_id = m_entity_manager->GetEntityId("camera");
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
    
    glUseProgram(m_shader_program);
 
    uint32_t vpos_location = glGetAttribLocation(m_shader_program, "vPos");
    uint32_t vcolor_location = glGetAttribLocation(m_shader_program, "vColor");
 
    glEnableVertexAttribArray(vpos_location);
    glVertexAttribPointer(vpos_location, 3, GL_FLOAT, GL_FALSE,
                          sizeof(VertexData), (void*) 0);
    glEnableVertexAttribArray(vcolor_location);
    glVertexAttribPointer(vcolor_location, 3, GL_FLOAT, GL_FALSE,
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