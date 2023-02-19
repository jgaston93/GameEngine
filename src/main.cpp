#include <glad/gl.h>
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#include <stdlib.h>
#include <stdio.h>
#include <chrono>
#include <thread>
#include <vector>
#include <cstdlib>

#include "json.hpp"

#include "linmath.h"
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include "ComponentManager.hpp"
#include "EntityManager.hpp"

#include "PlayerInputSystem.hpp"
#include "PhysicsSystem.hpp"
#include "RenderSystem.hpp"

static const char* vertex_shader_text =
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
 
static const char* fragment_shader_text =
"#version 330\n"
"in vec3 fColor\n;"
"out vec3 fragColor;\n"
"void main()\n"
"{\n"
"    fragColor = fColor;\n"
"}\n";
 
static void error_callback(int error, const char* description)
{
    fprintf(stderr, "Error: %s\n", description);
}


InputMap* input_map;

static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
    {
        glfwSetWindowShouldClose(window, GLFW_TRUE);
    }
    bool is_pressed = input_map->IsPressed(key);
    if(action == GLFW_PRESS)
    {
        is_pressed = true;
    }
    else if(action == GLFW_RELEASE)
    {
        is_pressed = false;
    }
    input_map->SetIsPressed(key, is_pressed);
}

void GenerateEntities(EntityManager& entity_manager, ComponentManager& component_manager);
// void LoadModels();

int main(int argv, char* args[])
{ 
    GLFWwindow* window;
    GLuint vertex_buffer, vertex_shader, fragment_shader, program;
    GLint mvp_location, vpos_location, vcolor_location;
 
    glfwSetErrorCallback(error_callback);
 
    if (!glfwInit())
        exit(EXIT_FAILURE);
 
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
 
    window = glfwCreateWindow(640, 480, "Untitled", NULL, NULL);
    if (!window)
    {
        glfwTerminate();
        exit(EXIT_FAILURE);
    }
 
    glfwSetKeyCallback(window, key_callback);
 
    glfwMakeContextCurrent(window);
    gladLoadGL(glfwGetProcAddress);
    glfwSwapInterval(1);
    
    glEnable(GL_BLEND);
    glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
    glGenBuffers(1, &vertex_buffer);
    glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);    
 
    vertex_shader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertex_shader, 1, &vertex_shader_text, NULL);
    glCompileShader(vertex_shader);

    GLint isCompiled = 0;
    glGetShaderiv(vertex_shader, GL_COMPILE_STATUS, &isCompiled);
    if(isCompiled == GL_FALSE)
    {
        GLint maxLength = 0;
        glGetShaderiv(vertex_shader, GL_INFO_LOG_LENGTH, &maxLength);

        // The maxLength includes the NULL character
        std::vector<GLchar> errorLog(maxLength);
        glGetShaderInfoLog(vertex_shader, maxLength, &maxLength, &errorLog[0]);

        // Provide the infolog in whatever manor you deem best.
        // Exit with failure.
        glDeleteShader(vertex_shader); // Don't leak the shader.
        return -1;
    }
 
    fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragment_shader, 1, &fragment_shader_text, NULL);
    glCompileShader(fragment_shader);

    isCompiled = 0;
    glGetShaderiv(fragment_shader, GL_COMPILE_STATUS, &isCompiled);
    if(isCompiled == GL_FALSE)
    {
        GLint maxLength = 0;
        glGetShaderiv(fragment_shader, GL_INFO_LOG_LENGTH, &maxLength);

        // The maxLength includes the NULL character
        std::vector<GLchar> errorLog(maxLength);
        glGetShaderInfoLog(fragment_shader, maxLength, &maxLength, &errorLog[0]);
        printf("%s\n", errorLog.data());

        // Provide the infolog in whatever manor you deem best.
        // Exit with failure.
        glDeleteShader(fragment_shader); // Don't leak the shader.
        return -1;
    }
 
    program = glCreateProgram();
    glAttachShader(program, vertex_shader);
    glAttachShader(program, fragment_shader);
    glLinkProgram(program);
    glUseProgram(program);
 
    mvp_location = glGetUniformLocation(program, "MVP");
    vpos_location = glGetAttribLocation(program, "vPos");
    vcolor_location = glGetAttribLocation(program, "vColor");
 
    glEnableVertexAttribArray(vpos_location);
    glVertexAttribPointer(vpos_location, 3, GL_FLOAT, GL_FALSE,
                          sizeof(VertexData), (void*) 0);
    glEnableVertexAttribArray(vcolor_location);
    glVertexAttribPointer(vcolor_location, 3, GL_FLOAT, GL_FALSE,
                          sizeof(VertexData), (void*) (sizeof(float) * 3));

    uint32_t num_inputs = 9;
    uint32_t input_list[] = { GLFW_KEY_LEFT,
                                GLFW_KEY_RIGHT,
                                GLFW_KEY_UP,
                                GLFW_KEY_DOWN,
                                GLFW_KEY_W,
                                GLFW_KEY_S,
                                GLFW_KEY_A,
                                GLFW_KEY_D,
                                GLFW_KEY_SPACE };

    input_map = new InputMap(num_inputs);
    for(uint32_t i = 0; i < num_inputs; i++)
    {
        input_map->AddInput(input_list[i]);
    }
    
    const uint32_t num_entities = 3;
    EntityManager entity_manager(num_entities);
    ComponentManager component_manager(num_entities);
    GenerateEntities(entity_manager, component_manager);


    const uint32_t num_messages = 1024;
    const uint32_t num_systems = 3;
    MessageBus message_bus(num_messages, num_systems);

    PlayerInputSystem player_input_system(message_bus, *input_map);
    player_input_system.SetEntityManager(&entity_manager);
    player_input_system.SetComponentManager(&component_manager);
    PhysicsSystem physics_system(message_bus);
    physics_system.SetEntityManager(&entity_manager);
    physics_system.SetComponentManager(&component_manager);
    RenderSystem render_system(message_bus, mvp_location);
    render_system.SetEntityManager(&entity_manager);
    render_system.SetComponentManager(&component_manager);
    
    std::chrono::time_point<std::chrono::steady_clock> prev_time = std::chrono::steady_clock::now();
    uint32_t num_frames = 0;
    uint32_t MS_PER_FRAME = 16;

    bool is_running = true;

    while (!glfwWindowShouldClose(window))
    {
        std::chrono::time_point<std::chrono::steady_clock> current_time = std::chrono::steady_clock::now();
        float delta_time = std::chrono::duration_cast<std::chrono::milliseconds>(current_time - prev_time).count() * 1e-3;
        prev_time = current_time;

        glfwPollEvents();
        
        float ratio;
        int width, height;

        glfwGetFramebufferSize(window, &width, &height);
        ratio = width / (float) height;

        glViewport(0, 0, width, height);
        glClearColor(0, 0, 0, 1.0);
        glClear(GL_COLOR_BUFFER_BIT); 

        message_bus.Update();
        player_input_system.Update(delta_time);
        physics_system.Update(delta_time);
        render_system.Update(delta_time);

        glfwSwapBuffers(window);
        
        // Calculate time to sleep and sleep if necessary
        std::chrono::time_point<std::chrono::steady_clock> next_frame_time = current_time + std::chrono::milliseconds(MS_PER_FRAME);
        std::this_thread::sleep_until(next_frame_time);
    }
 
    glfwDestroyWindow(window);
 
    glfwTerminate();
    exit(EXIT_SUCCESS);
    
    return 0;
}


void GenerateEntities(EntityManager& entity_manager, ComponentManager& component_manager)
{
    uint32_t entity_id = 0;

    // Camera Entity
    Transform transform;
    transform.position[0] = 0;
    transform.position[1] = 0;
    transform.position[2] = 10;
    transform.rotation[0] = 0;
    transform.rotation[1] = 0;
    transform.rotation[2] = 0;
    transform.scale[0] = 0;
    transform.scale[1] = 0;
    transform.scale[2] = 0;

    entity_manager.SetEntityState(entity_id, EntityState::ACTIVE);
    entity_manager.SetEntityTag(entity_id, "camera");

    component_manager.AddComponent<Transform>(entity_id, transform);

    entity_id++;
    
    // Player Test Entity
    transform.position[0] = -10;
    transform.position[1] = 0;
    transform.position[2] = -10;
    transform.rotation[0] = 0;
    transform.rotation[1] = 0;
    transform.rotation[2] = 0;
    transform.scale[0] = 0;
    transform.scale[1] = 0;
    transform.scale[2] = 0;

    Quad quad;
    quad.extent[0] = 1;
    quad.extent[1] = 1;

    Texture texture;
    texture.color[0] = 0;
    texture.color[1] = 1;
    texture.color[2] = 0;

    BoundingBox bounding_box;
    bounding_box.extent[0] = 1;
    bounding_box.extent[1] = 1;
    bounding_box.extent[2] = 1;

    RigidBody rigid_body;
    rigid_body.velocity[0] = 0;
    rigid_body.velocity[1] = 0;
    rigid_body.velocity[2] = 0;
    rigid_body.acceleration[0] = 0;
    rigid_body.acceleration[1] = 0;
    rigid_body.acceleration[2] = 0;

    entity_manager.SetEntityState(entity_id, EntityState::ACTIVE);
    entity_manager.SetEntitySignature(entity_id, RENDER_SYSTEM_SIGNATURE | 
                                                    PHYSICS_SYSTEM_SIGNATURE | 
                                                    COLLISION_SYSTEM_SIGNATURE |
                                                    PLAYER_INPUT_SYSTEM_SIGNATURE);

    component_manager.AddComponent<Transform>(entity_id, transform);
    component_manager.AddComponent<Quad>(entity_id, quad);
    component_manager.AddComponent<Texture>(entity_id, texture);
    component_manager.AddComponent<BoundingBox>(entity_id, bounding_box);
    component_manager.AddComponent<RigidBody>(entity_id, rigid_body);

    entity_id++;
    
    // Test Entity
    transform.position[0] = 0;
    transform.position[1] = 0;
    transform.position[2] = -10;
    transform.rotation[0] = 0;
    transform.rotation[1] = 0;
    transform.rotation[2] = 0;
    transform.scale[0] = 0;
    transform.scale[1] = 0;
    transform.scale[2] = 0;
    
    quad.extent[0] = 5;
    quad.extent[1] = 5;
    
    texture.color[0] = 1;
    texture.color[1] = 0;
    texture.color[2] = 0;
    
    bounding_box.extent[0] = 5;
    bounding_box.extent[1] = 5;
    bounding_box.extent[2] = 1;

    entity_manager.SetEntityState(entity_id, EntityState::ACTIVE);
    entity_manager.SetEntitySignature(entity_id, RENDER_SYSTEM_SIGNATURE | 
                                                    PHYSICS_SYSTEM_SIGNATURE | 
                                                    COLLISION_SYSTEM_SIGNATURE);

    component_manager.AddComponent<Transform>(entity_id, transform);
    component_manager.AddComponent<Quad>(entity_id, quad);
    component_manager.AddComponent<Texture>(entity_id, texture);
    component_manager.AddComponent<BoundingBox>(entity_id, bounding_box);
}
