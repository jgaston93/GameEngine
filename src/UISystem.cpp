#include <vector>
#include <algorithm>
#include <cstdio>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include "UISystem.hpp"

static const char* ui_vertex_shader_text =
"#version 330\n"
"uniform mat4 MVP;\n"
"layout(location = 0)in vec3 vPos;\n"
"layout(location = 1)in vec3 vColor;\n"
"layout(location = 2)in vec2 vTexCoord;\n"
"out vec3 fColor;\n"
"out vec2 fTexCoord;\n"
"void main()\n"
"{\n"
"    gl_Position = vec4(vPos, 1.0);\n"
"    fColor = vColor;\n"
"    fTexCoord = vTexCoord;\n"
"}\n";
 
static const char* ui_fragment_shader_text =
"#version 330\n"
"in vec3 fColor\n;"
"in vec2 fTexCoord;\n"
"out vec4 fragColor;\n"
"uniform sampler2D uiTexture;\n"
"void main()\n"
"{\n"
"    fragColor = texture(uiTexture, fTexCoord);\n"
"}\n";

const vec2 alpha_start_index = { 0, 490 };
const vec2 numeric_start_index = { 0, 459 };
const vec2 character_extent = { 15, 22 };
const uint32_t character_stride = 15;
const vec2 texture_size = { 512, 512 };

struct UIVertexData
{
    float x;
    float y;
    float z;
    float r;
    float g;
    float b;
    float u;
    float v;
};

UISystem::UISystem(MessageBus& message_bus, GLFWwindow* window) : 
    System(message_bus, UI_SYSTEM_SIGNATURE),
    m_window(window)
{   
    glGenTextures(1, &m_ui_texture);
    glBindTexture(GL_TEXTURE_2D, m_ui_texture);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    int32_t width, height, num_channels;
    stbi_set_flip_vertically_on_load(true);
    unsigned char *data = stbi_load("assets/UITexture.png", &width, &height, &num_channels, 0);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
    glGenerateMipmap(GL_TEXTURE_2D);
    stbi_image_free(data);

    GLuint ui_render_vertex_shader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(ui_render_vertex_shader, 1, &ui_vertex_shader_text, NULL);
    glCompileShader(ui_render_vertex_shader);

    GLint isCompiled = 0;
    glGetShaderiv(ui_render_vertex_shader, GL_COMPILE_STATUS, &isCompiled);
    if(isCompiled == GL_FALSE)
    {
        GLint maxLength = 0;
        glGetShaderiv(ui_render_vertex_shader, GL_INFO_LOG_LENGTH, &maxLength);

        // The maxLength includes the NULL character
        std::vector<GLchar> errorLog(maxLength);
        glGetShaderInfoLog(ui_render_vertex_shader, maxLength, &maxLength, &errorLog[0]);
        printf("%s\n", errorLog.data());

        // Provide the infolog in whatever manor you deem best.
        // Exit with failure.
        glDeleteShader(ui_render_vertex_shader); // Don't leak the shader.
        return;
    }
 
    GLuint ui_render_fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(ui_render_fragment_shader, 1, &ui_fragment_shader_text, NULL);
    glCompileShader(ui_render_fragment_shader);

    isCompiled = 0;
    glGetShaderiv(ui_render_fragment_shader, GL_COMPILE_STATUS, &isCompiled);
    if(isCompiled == GL_FALSE)
    {
        GLint maxLength = 0;
        glGetShaderiv(ui_render_fragment_shader, GL_INFO_LOG_LENGTH, &maxLength);

        // The maxLength includes the NULL character
        std::vector<GLchar> errorLog(maxLength);
        glGetShaderInfoLog(ui_render_fragment_shader, maxLength, &maxLength, &errorLog[0]);
        printf("%s\n", errorLog.data());

        // Provide the infolog in whatever manor you deem best.
        // Exit with failure.
        glDeleteShader(ui_render_fragment_shader); // Don't leak the shader.
        return;
    }
 
    m_shader_program = glCreateProgram();
    glAttachShader(m_shader_program, ui_render_vertex_shader);
    glAttachShader(m_shader_program, ui_render_fragment_shader);
    glLinkProgram(m_shader_program);
    glDeleteShader(ui_render_vertex_shader);
    glDeleteShader(ui_render_fragment_shader);

}

UISystem::~UISystem()
{

}

void UISystem::HandleMessage(Message message)
{

}

void UISystem::HandleEntity(uint32_t entity_id, float delta_time)
{
    Transform& transform = m_component_manager->GetComponent<Transform>(entity_id);
    Label& label = m_component_manager->GetComponent<Label>(entity_id);

    vec2 vertices_positions[4] = { { transform.position[0],                                            transform.position[1] + character_extent[1] * transform.scale[1] },
                                   { transform.position[0],                                            transform.position[1]                                            },
                                   { transform.position[0] + character_extent[0] * transform.scale[0], transform.position[1] + character_extent[1] * transform.scale[1] },
                                   { transform.position[0] + character_extent[0] * transform.scale[0], transform.position[1]                                            } };

    int width, height;
    glfwGetFramebufferSize(m_window, &width, &height);
    uint32_t label_length = strlen(label.text);

    for(uint32_t i = 0; i < label_length; i++)
    {
        char character = label.text[i];
        uint32_t character_index = 0;
        vec2 texture_start_index = { 0, 0 };
        
        if('A' <= character && character <= 'Z')
        {
            character_index = character - 'A';
            texture_start_index[0] = alpha_start_index[0] + character_index * character_stride;
            texture_start_index[1] = alpha_start_index[1];
        }
        else if('0' <= character && character <= '9')
        {
            character_index = character - '0';
            texture_start_index[0] = numeric_start_index[0] + character_index * character_stride;
            texture_start_index[1] = numeric_start_index[1];
        }
        else if(character == ' ')
        {
            character_index = 26;
            texture_start_index[0] = alpha_start_index[0] + character_index * character_stride;
            texture_start_index[1] = alpha_start_index[1];
        }
        
        UIVertexData vertices[4];

        vertices[0] = { 2.0f * (vertices_positions[0][0] + character_stride * i * transform.scale[0]) / width - 1.0f, 2.0f * vertices_positions[0][1] / height - 1.0f, 0,
                        label.color[0], label.color[1], label.color[2],
                        texture_start_index[0] / texture_size[0], (texture_start_index[1] + character_extent[1]) / texture_size[1] };
                        
        vertices[1] = { 2.0f * (vertices_positions[1][0] + character_stride * i * transform.scale[0]) / width - 1.0f, 2.0f * vertices_positions[1][1] / height - 1.0f, 0,
                        label.color[0], label.color[1], label.color[2],
                        texture_start_index[0] / texture_size[0], texture_start_index[1] / texture_size[1] };

        vertices[2] = { 2.0f * (vertices_positions[2][0] + character_stride * i * transform.scale[0]) / width - 1.0f, 2.0f * vertices_positions[2][1] / height - 1.0f, 0,
                        label.color[0], label.color[1], label.color[2],
                        (texture_start_index[0] + character_extent[0]) / texture_size[0], (texture_start_index[1] + character_extent[1]) / texture_size[1] };
                        
        vertices[3] = { 2.0f * (vertices_positions[3][0] + character_stride * i * transform.scale[0]) / width - 1.0f, 2.0f * vertices_positions[3][1] / height - 1.0f, 0,
                        label.color[0], label.color[1], label.color[2],
                        (texture_start_index[0] + character_extent[0]) / texture_size[0],  texture_start_index[1] / texture_size[1] };

        glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), &vertices, GL_DYNAMIC_DRAW);
        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    }

}

void UISystem::Update(float delta_time)
{
    glBindTexture(GL_TEXTURE_2D, m_ui_texture);

    glUseProgram(m_shader_program);
 
    uint32_t texture_location = glGetUniformLocation(m_shader_program, "uiTexture");
    uint32_t vpos_location = glGetAttribLocation(m_shader_program, "vPos");
    uint32_t vcolor_location = glGetAttribLocation(m_shader_program, "vColor");
    uint32_t vtex_coord_location = glGetAttribLocation(m_shader_program, "vTexCoord");
 
    glEnableVertexAttribArray(vpos_location);
    glVertexAttribPointer(vpos_location, 3, GL_FLOAT, GL_FALSE,
                          sizeof(UIVertexData), (void*) 0);
    glEnableVertexAttribArray(vcolor_location);
    glVertexAttribPointer(vcolor_location, 3, GL_FLOAT, GL_FALSE,
                          sizeof(UIVertexData), (void*) (sizeof(float) * 3));
    glEnableVertexAttribArray(vtex_coord_location);
    glVertexAttribPointer(vtex_coord_location, 2, GL_FLOAT, GL_FALSE,
                          sizeof(UIVertexData), (void*) (sizeof(float) * 6));

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