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

#include "ComponentManager.hpp"
#include "EntityManager.hpp"

#include "PlayerInputSystem.hpp"
#include "PhysicsSystem.hpp"
#include "RenderSystem.hpp"
#include "UISystem.hpp"
 
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

static void cursor_position_callback(GLFWwindow* window, double xpos, double ypos)
{
    input_map->SetMousePosX(xpos);
    input_map->SetMousePosY(ypos);
}

void GenerateEntities(EntityManager& entity_manager, ComponentManager& component_manager);
// void LoadModels();

int main(int argv, char* args[])
{ 
    GLFWwindow* window;
    GLuint vertex_buffer;
    GLuint quad_render_vertex_shader, quad_render_fragment_shader, quad_render_program;
    GLuint ui_render_vertex_shader, ui_render_fragment_shader, ui_render_program;
 
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
    glfwSetCursorPosCallback(window, cursor_position_callback);
 
    glfwMakeContextCurrent(window);
    gladLoadGL(glfwGetProcAddress);
    glfwSwapInterval(1);

    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    if (glfwRawMouseMotionSupported())
    glfwSetInputMode(window, GLFW_RAW_MOUSE_MOTION, GLFW_TRUE);
    
    glEnable(GL_BLEND);
    glEnable(GL_DEPTH_TEST);
    glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
    
    glGenBuffers(1, &vertex_buffer);
    glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);

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
    
    const uint32_t num_entities = 6;
    EntityManager entity_manager(num_entities);
    ComponentManager component_manager(num_entities);
    GenerateEntities(entity_manager, component_manager);


    const uint32_t num_messages = 1024;
    const uint32_t num_systems = 4;
    MessageBus message_bus(num_messages, num_systems);

    PlayerInputSystem player_input_system(message_bus, *input_map);
    player_input_system.SetEntityManager(&entity_manager);
    player_input_system.SetComponentManager(&component_manager);
    PhysicsSystem physics_system(message_bus);
    physics_system.SetEntityManager(&entity_manager);
    physics_system.SetComponentManager(&component_manager);
    RenderSystem render_system(message_bus);
    render_system.SetEntityManager(&entity_manager);
    render_system.SetComponentManager(&component_manager);
    // UISystem ui_system(message_bus, window);
    // ui_system.SetEntityManager(&entity_manager);
    // ui_system.SetComponentManager(&component_manager);
    
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
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); 

        message_bus.Update();
        player_input_system.Update(delta_time);
        physics_system.Update(delta_time);
        render_system.Update(delta_time);
        // ui_system.Update(delta_time);

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
    transform.position[1] = 1;
    transform.position[2] = 0;
    transform.rotation[0] = 0;
    transform.rotation[1] = 0;
    transform.rotation[2] = 0;
    transform.scale[0] = 0;
    transform.scale[1] = 0;
    transform.scale[2] = 0;

    BoundingBox bounding_box;
    bounding_box.extent[0] = 2;
    bounding_box.extent[1] = 2;
    bounding_box.extent[2] = 2;

    entity_manager.SetEntityState(entity_id, EntityState::ACTIVE);
    entity_manager.SetEntitySignature(entity_id, PLAYER_INPUT_SYSTEM_SIGNATURE |
                                                    PHYSICS_SYSTEM_SIGNATURE |
                                                    COLLISION_SYSTEM_SIGNATURE);
    entity_manager.SetEntityTag(entity_id, "camera");

    component_manager.AddComponent<Transform>(entity_id, transform);

    entity_id++;
    
    // Wall Entity
    transform.position[0] = 0;
    transform.position[1] = 1.25;
    transform.position[2] = -10;
    transform.rotation[0] = 0;
    transform.rotation[1] = 0;
    transform.rotation[2] = 0;
    transform.scale[0] = 0;
    transform.scale[1] = 0;
    transform.scale[2] = 0;
    
    Quad quad;
    quad.extent[0] = 32;
    quad.extent[1] = 2.5;

    Texture texture;
    texture.texture_index = 2;
    texture.position[0] = 0;
    texture.position[1] = 0;
    texture.size[0] = 256;
    texture.size[1] = 256;
    texture.color[0] = 0;
    texture.color[1] = 1;
    texture.color[2] = 0;
    
    bounding_box.extent[0] = 32;
    bounding_box.extent[1] = 2.5;
    bounding_box.extent[2] = 2;

    entity_manager.SetEntityState(entity_id, EntityState::ACTIVE);
    entity_manager.SetEntitySignature(entity_id, RENDER_SYSTEM_SIGNATURE | 
                                                    PHYSICS_SYSTEM_SIGNATURE | 
                                                    COLLISION_SYSTEM_SIGNATURE);

    component_manager.AddComponent<Transform>(entity_id, transform);
    component_manager.AddComponent<Quad>(entity_id, quad);
    component_manager.AddComponent<Texture>(entity_id, texture);
    component_manager.AddComponent<BoundingBox>(entity_id, bounding_box);

    entity_id++;
    
    // Wall Entity
    transform.position[0] = 0;
    transform.position[1] = 1.25;
    transform.position[2] = 10;
    transform.rotation[0] = 0;
    transform.rotation[1] = 0;
    transform.rotation[2] = 0;
    transform.scale[0] = 0;
    transform.scale[1] = 0;
    transform.scale[2] = 0;
    
    quad.extent[0] = 32;
    quad.extent[1] = 2.5;
    
    texture.texture_index = 2;
    texture.position[0] = 0;
    texture.position[1] = 0;
    texture.size[0] = 256;
    texture.size[1] = 256;
    texture.color[0] = 0;
    texture.color[1] = 1;
    texture.color[2] = 0;
    
    bounding_box.extent[0] = 32;
    bounding_box.extent[1] = 2.5;
    bounding_box.extent[2] = 2;

    entity_manager.SetEntityState(entity_id, EntityState::ACTIVE);
    entity_manager.SetEntitySignature(entity_id, RENDER_SYSTEM_SIGNATURE | 
                                                    PHYSICS_SYSTEM_SIGNATURE | 
                                                    COLLISION_SYSTEM_SIGNATURE);

    component_manager.AddComponent<Transform>(entity_id, transform);
    component_manager.AddComponent<Quad>(entity_id, quad);
    component_manager.AddComponent<Texture>(entity_id, texture);
    component_manager.AddComponent<BoundingBox>(entity_id, bounding_box);

    entity_id++;
    
    // Wall Entity
    transform.position[0] = 16;
    transform.position[1] = 1.25;
    transform.position[2] = 0;
    transform.rotation[0] = 0;
    transform.rotation[1] = 90;
    transform.rotation[2] = 0;
    transform.scale[0] = 0;
    transform.scale[1] = 0;
    transform.scale[2] = 0;
    
    quad.extent[0] = 20;
    quad.extent[1] = 2.5;
    
    texture.texture_index = 2;
    texture.position[0] = 0;
    texture.position[1] = 0;
    texture.size[0] = 256;
    texture.size[1] = 256;
    texture.color[0] = 0;
    texture.color[1] = 1;
    texture.color[2] = 0;
    
    bounding_box.extent[0] = 2;
    bounding_box.extent[1] = 2.5;
    bounding_box.extent[2] = 20;

    entity_manager.SetEntityState(entity_id, EntityState::ACTIVE);
    entity_manager.SetEntitySignature(entity_id, RENDER_SYSTEM_SIGNATURE | 
                                                    PHYSICS_SYSTEM_SIGNATURE | 
                                                    COLLISION_SYSTEM_SIGNATURE);

    component_manager.AddComponent<Transform>(entity_id, transform);
    component_manager.AddComponent<Quad>(entity_id, quad);
    component_manager.AddComponent<Texture>(entity_id, texture);
    component_manager.AddComponent<BoundingBox>(entity_id, bounding_box);

    entity_id++;
    
    // Wall Entity
    transform.position[0] = -16;
    transform.position[1] = 1.25;
    transform.position[2] = 0;
    transform.rotation[0] = 0;
    transform.rotation[1] = 90;
    transform.rotation[2] = 0;
    transform.scale[0] = 0;
    transform.scale[1] = 0;
    transform.scale[2] = 0;
    
    quad.extent[0] = 20;
    quad.extent[1] = 2.5;
    
    texture.texture_index = 2;
    texture.position[0] = 0;
    texture.position[1] = 0;
    texture.size[0] = 256;
    texture.size[1] = 256;
    texture.color[0] = 0;
    texture.color[1] = 1;
    texture.color[2] = 0;
    
    bounding_box.extent[0] = 2;
    bounding_box.extent[1] = 2.5;
    bounding_box.extent[2] = 20;

    entity_manager.SetEntityState(entity_id, EntityState::ACTIVE);
    entity_manager.SetEntitySignature(entity_id, RENDER_SYSTEM_SIGNATURE | 
                                                    PHYSICS_SYSTEM_SIGNATURE | 
                                                    COLLISION_SYSTEM_SIGNATURE);

    component_manager.AddComponent<Transform>(entity_id, transform);
    component_manager.AddComponent<Quad>(entity_id, quad);
    component_manager.AddComponent<Texture>(entity_id, texture);
    component_manager.AddComponent<BoundingBox>(entity_id, bounding_box);

    entity_id++;

    // Test Label
    transform.position[0] = 0;
    transform.position[1] = 0;
    transform.position[2] = 0;
    transform.rotation[0] = 0;
    transform.rotation[1] = 0;
    transform.rotation[2] = 0;
    transform.scale[0] = 1;
    transform.scale[1] = 1;
    transform.scale[2] = 1;

    Label label;
    label.color[0] = 0;
    label.color[1] = 1;
    label.color[2] = 0;
    label.text = new char[10];
    snprintf(label.text, 10, "TEST");

    entity_manager.SetEntityState(entity_id, EntityState::ACTIVE);
    entity_manager.SetEntitySignature(entity_id, UI_SYSTEM_SIGNATURE);

    component_manager.AddComponent<Transform>(entity_id, transform);
    component_manager.AddComponent<Label>(entity_id, label);
}
