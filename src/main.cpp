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
#include "AISystem.hpp"
 
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

void mouse_button_callback(GLFWwindow* window, int button, int action, int mods)
{
    bool is_pressed = input_map->IsPressed(button);
    if(action == GLFW_PRESS)
    {
        is_pressed = true;
    }
    else if(action == GLFW_RELEASE)
    {
        is_pressed = false;
    }
    input_map->SetIsPressed(button, is_pressed);
}

static void cursor_position_callback(GLFWwindow* window, double xpos, double ypos)
{
    input_map->SetMousePosX(xpos);
    input_map->SetMousePosY(ypos);
}

void GenerateEntities(EntityManager& entity_manager, ComponentManager& component_manager);
void GenerateFloor(EntityManager& entity_manager, ComponentManager& component_manager, uint32_t& entity_id, vec3 offset,
                        uint32_t num_windows, float window_width, float window_height, float wall_total_width, float wall_height, bool my_building);

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
    glfwSetMouseButtonCallback(window, mouse_button_callback);
    glfwSetCursorPosCallback(window, cursor_position_callback);
    glfwSetCursorPos(window, 0, 0);
 
    glfwMakeContextCurrent(window);
    gladLoadGL(glfwGetProcAddress);
    glfwSwapInterval(1);

    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    if (glfwRawMouseMotionSupported())
    glfwSetInputMode(window, GLFW_RAW_MOUSE_MOTION, GLFW_TRUE);

    glEnable(GL_CULL_FACE);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    
    glGenBuffers(1, &vertex_buffer);
    glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);

    uint32_t num_inputs = 13;
    uint32_t input_list[] = { GLFW_KEY_LEFT,
                                GLFW_KEY_RIGHT,
                                GLFW_KEY_UP,
                                GLFW_KEY_DOWN,
                                GLFW_KEY_W,
                                GLFW_KEY_S,
                                GLFW_KEY_A,
                                GLFW_KEY_D,
                                GLFW_KEY_E,
                                GLFW_KEY_Q,
                                GLFW_KEY_SPACE,
                                GLFW_MOUSE_BUTTON_RIGHT,
                                GLFW_MOUSE_BUTTON_LEFT };

    input_map = new InputMap(num_inputs);
    for(uint32_t i = 0; i < num_inputs; i++)
    {
        input_map->AddInput(input_list[i]);
    }
    
    const uint32_t num_entities = 177;
    EntityManager entity_manager(num_entities);
    ComponentManager component_manager(num_entities);
    GenerateEntities(entity_manager, component_manager);


    const uint32_t num_messages = 1024;
    const uint32_t num_systems = 5;
    MessageBus message_bus(num_messages, num_systems);

    PlayerInputSystem player_input_system(message_bus, *input_map);
    player_input_system.SetEntityManager(&entity_manager);
    player_input_system.SetComponentManager(&component_manager);
    PhysicsSystem physics_system(message_bus);
    physics_system.SetEntityManager(&entity_manager);
    physics_system.SetComponentManager(&component_manager);
    RenderSystem render_system(message_bus, *input_map);
    render_system.SetEntityManager(&entity_manager);
    render_system.SetComponentManager(&component_manager);
    UISystem ui_system(message_bus, window);
    ui_system.SetEntityManager(&entity_manager);
    ui_system.SetComponentManager(&component_manager);
    AISystem ai_system(message_bus);
    ai_system.SetEntityManager(&entity_manager);
    ai_system.SetComponentManager(&component_manager);

    
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
        glClearColor(0.5, 0.5, 0.5, 1.0);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); 

        message_bus.Update();
        ai_system.Update(delta_time);
        player_input_system.Update(delta_time);
        physics_system.Update(delta_time);
        glEnable(GL_DEPTH_TEST);
        render_system.Update(delta_time);
        glDisable(GL_DEPTH_TEST);
        ui_system.Update(delta_time);

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
    bounding_box.extent[0] = 0.15;
    bounding_box.extent[1] = 1.5;
    bounding_box.extent[2] = 0.15;

    PlayerInput player_input;
    player_input.score = 0;
    player_input.timer = 60;
    player_input.state = PlayerState::INIT;

    entity_manager.SetEntityState(entity_id, EntityState::ACTIVE);
    entity_manager.SetEntitySignature(entity_id, PLAYER_INPUT_SYSTEM_SIGNATURE |
                                                    PHYSICS_SYSTEM_SIGNATURE |
                                                    COLLISION_SYSTEM_SIGNATURE);
    entity_manager.SetEntityTag(entity_id, "player");

    component_manager.AddComponent<Transform>(entity_id, transform);
    component_manager.AddComponent<BoundingBox>(entity_id, bounding_box);
    component_manager.AddComponent<PlayerInput>(entity_id, player_input);

    entity_id++;

    uint32_t num_windows = 5;
    float window_width = 0.6;
    float window_height = 1;
    float wall_total_width = 32;
    float wall_height = 2.5;

    // Front collision box
    transform.position[0] = 0;
    transform.position[1] = wall_height / 3 / 2;
    transform.position[2] = -1;
    transform.rotation[0] = 0;
    transform.rotation[1] = 0;
    transform.rotation[2] = 0;
    transform.scale[0] = 0;
    transform.scale[1] = 0;
    transform.scale[2] = 0;
    
    bounding_box.extent[0] = wall_total_width;
    bounding_box.extent[1] = wall_height / 3;
    bounding_box.extent[2] = .1;

    entity_manager.SetEntityState(entity_id, EntityState::ACTIVE);
    entity_manager.SetEntitySignature(entity_id, COLLISION_SYSTEM_SIGNATURE);

    component_manager.AddComponent<Transform>(entity_id, transform);
    component_manager.AddComponent<BoundingBox>(entity_id, bounding_box);

    entity_id++;

    vec3 my_floor_offset = { 0, 0, 0 };
    GenerateFloor(entity_manager, component_manager, entity_id, my_floor_offset, num_windows, window_width, window_width, wall_total_width, wall_height, true);

    vec3 other_floor_offset = { 0, wall_height, -10 };
    GenerateFloor(entity_manager, component_manager, entity_id, other_floor_offset, num_windows, window_width, window_width, wall_total_width, wall_height, false);
    other_floor_offset[1] -= wall_height;
    GenerateFloor(entity_manager, component_manager, entity_id, other_floor_offset, num_windows, window_width, window_width, wall_total_width, wall_height, false);
    other_floor_offset[1] -= wall_height;
    GenerateFloor(entity_manager, component_manager, entity_id, other_floor_offset, num_windows, window_width, window_width, wall_total_width, wall_height, false);
    other_floor_offset[1] -= wall_height;
    GenerateFloor(entity_manager, component_manager, entity_id, other_floor_offset, num_windows, window_width, window_width, wall_total_width, wall_height, false);

    
    // Ground Entity
    transform.position[0] = 0;
    transform.position[1] = other_floor_offset[1] - 0.2;
    transform.position[2] = 0;
    transform.rotation[0] = -90;
    transform.rotation[1] = 0;
    transform.rotation[2] = 0;
    transform.scale[0] = 0;
    transform.scale[1] = 0;
    transform.scale[2] = 0;
    
    Quad quad;
    quad.extent[0] = 10000;
    quad.extent[1] = 10000;
    
    Texture texture;
    texture.texture_index = 5;
    texture.position[0] = 0;
    texture.position[1] = 0;
    texture.size[0] = 256;
    texture.size[1] = 85;
    texture.use_light = false;

    entity_manager.SetEntityState(entity_id, EntityState::ACTIVE);
    entity_manager.SetEntitySignature(entity_id, RENDER_SYSTEM_SIGNATURE);

    component_manager.AddComponent<Transform>(entity_id, transform);
    component_manager.AddComponent<Quad>(entity_id, quad);
    component_manager.AddComponent<Texture>(entity_id, texture);

    entity_id++;

    // Road Entity
    transform.position[0] = 0;
    transform.position[1] = other_floor_offset[1] - 0.1;
    transform.position[2] = -6;
    transform.rotation[0] = -90;
    transform.rotation[1] = 0;
    transform.rotation[2] = 0;
    transform.scale[0] = 0;
    transform.scale[1] = 0;
    transform.scale[2] = 0;
    
    quad.extent[0] = 10000;
    quad.extent[1] = 5;
    
    texture.texture_index = 5;
    texture.position[0] = 0;
    texture.position[1] = 172;
    texture.size[0] = 256 * 1000;
    texture.size[1] = 85;
    texture.use_light = false;

    entity_manager.SetEntityState(entity_id, EntityState::ACTIVE);
    entity_manager.SetEntitySignature(entity_id, RENDER_SYSTEM_SIGNATURE);

    component_manager.AddComponent<Transform>(entity_id, transform);
    component_manager.AddComponent<Quad>(entity_id, quad);
    component_manager.AddComponent<Texture>(entity_id, texture);

    entity_id++;

    // Sidewalk Entity
    transform.position[0] = 0;
    transform.position[1] = other_floor_offset[1] - 0.1;
    transform.position[2] = -8.8;
    transform.rotation[0] = -90;
    transform.rotation[1] = 0;
    transform.rotation[2] = 0;
    transform.scale[0] = 0;
    transform.scale[1] = 0;
    transform.scale[2] = 0;
    
    quad.extent[0] = 10000;
    quad.extent[1] = .6;
    
    texture.texture_index = 5;
    texture.position[0] = 0;
    texture.position[1] = 85;
    texture.size[0] = 256 * 1000;
    texture.size[1] = 85;
    texture.use_light = false;

    entity_manager.SetEntityState(entity_id, EntityState::ACTIVE);
    entity_manager.SetEntitySignature(entity_id, RENDER_SYSTEM_SIGNATURE);

    component_manager.AddComponent<Transform>(entity_id, transform);
    component_manager.AddComponent<Quad>(entity_id, quad);
    component_manager.AddComponent<Texture>(entity_id, texture);

    entity_id++;

    // Timer Label
    // transform.position[0] = (640 / 2) - (15 * 2) / 2;
    // transform.position[1] = 480 - 25;
    // transform.position[2] = 0;
    // transform.rotation[0] = 0;
    // transform.rotation[1] = 0;
    // transform.rotation[2] = 0;
    // transform.scale[0] = 1;
    // transform.scale[1] = 1;
    // transform.scale[2] = 1;

    // Label label;
    // label.color[0] = 0;
    // label.color[1] = 1;
    // label.color[2] = 0;
    // label.text = new char[10];
    // snprintf(label.text, 10, "00");

    // entity_manager.SetEntityState(entity_id, EntityState::INACTIVE);
    // entity_manager.SetEntitySignature(entity_id, UI_SYSTEM_TEXT_SIGNATURE);
    // entity_manager.SetEntityTag(entity_id, "timer_entity");

    // component_manager.AddComponent<Transform>(entity_id, transform);
    // component_manager.AddComponent<Label>(entity_id, label);

    // entity_id++;

    // Timer Label
    transform.position[0] = (640 / 2) - (15 * 2) / 2;
    transform.position[1] = 480 - 25;
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
    snprintf(label.text, 10, "00");

    entity_manager.SetEntityState(entity_id, EntityState::INACTIVE);
    entity_manager.SetEntitySignature(entity_id, UI_SYSTEM_TEXT_SIGNATURE);
    entity_manager.SetEntityTag(entity_id, "timer_entity");

    component_manager.AddComponent<Transform>(entity_id, transform);
    component_manager.AddComponent<Label>(entity_id, label);

    entity_id++;

    // Title Label
    transform.position[0] = (640 / 2) - ((15 * 11 * 2) / 2);
    transform.position[1] = (480 / 2) - ((22 * 2) / 2) + 22 * 3;
    transform.position[2] = 0;
    transform.rotation[0] = 0;
    transform.rotation[1] = 0;
    transform.rotation[2] = 0;
    transform.scale[0] = 2;
    transform.scale[1] = 2;
    transform.scale[2] = 2;

    label.color[0] = 0;
    label.color[1] = 1;
    label.color[2] = 0;
    label.text = new char[12];
    snprintf(label.text, 12, "XRAY SNIPER");

    entity_manager.SetEntityState(entity_id, EntityState::ACTIVE);
    entity_manager.SetEntitySignature(entity_id, UI_SYSTEM_TEXT_SIGNATURE);
    entity_manager.SetEntityTag(entity_id, "title_entity");

    component_manager.AddComponent<Transform>(entity_id, transform);
    component_manager.AddComponent<Label>(entity_id, label);

    entity_id++;

    // Win Label
    transform.position[0] = (640 / 2) - ((15 * 3 * 2) / 2);
    transform.position[1] = (480 / 2) - ((22 * 2) / 2) + 22 * 3;
    transform.position[2] = 0;
    transform.rotation[0] = 0;
    transform.rotation[1] = 0;
    transform.rotation[2] = 0;
    transform.scale[0] = 2;
    transform.scale[1] = 2;
    transform.scale[2] = 2;

    label.color[0] = 0;
    label.color[1] = 1;
    label.color[2] = 0;
    label.text = new char[12];
    snprintf(label.text, 12, "WIN");

    entity_manager.SetEntityState(entity_id, EntityState::INACTIVE);
    entity_manager.SetEntitySignature(entity_id, UI_SYSTEM_TEXT_SIGNATURE);
    entity_manager.SetEntityTag(entity_id, "win_entity");

    component_manager.AddComponent<Transform>(entity_id, transform);
    component_manager.AddComponent<Label>(entity_id, label);

    entity_id++;

    // Lose Label
    transform.position[0] = (640 / 2) - ((15 * 4 * 2) / 2);
    transform.position[1] = (480 / 2) - ((22 * 2) / 2) + 22 * 3;
    transform.position[2] = 0;
    transform.rotation[0] = 0;
    transform.rotation[1] = 0;
    transform.rotation[2] = 0;
    transform.scale[0] = 2;
    transform.scale[1] = 2;
    transform.scale[2] = 2;

    label.color[0] = 0;
    label.color[1] = 1;
    label.color[2] = 0;
    label.text = new char[12];
    snprintf(label.text, 12, "LOSE");

    entity_manager.SetEntityState(entity_id, EntityState::INACTIVE);
    entity_manager.SetEntitySignature(entity_id, UI_SYSTEM_TEXT_SIGNATURE);
    entity_manager.SetEntityTag(entity_id, "lose_entity");

    component_manager.AddComponent<Transform>(entity_id, transform);
    component_manager.AddComponent<Label>(entity_id, label);

    entity_id++;

    // Crosshair 
    transform.position[0] = 640 / 2;
    transform.position[1] = 480 / 2;
    transform.position[2] = 0;
    transform.rotation[0] = 0;
    transform.rotation[1] = 0;
    transform.rotation[2] = 0;
    transform.scale[0] = 1;
    transform.scale[1] = 1;
    transform.scale[2] = 1;

    texture.position[0] = 0;
    texture.position[1] = 0;
    texture.size[0] = 256;
    texture.size[1] = 256;

    quad.extent[0] = 640 / 2;
    quad.extent[1] = 480 / 2;

    entity_manager.SetEntityState(entity_id, EntityState::INACTIVE);
    entity_manager.SetEntitySignature(entity_id, UI_SYSTEM_IMAGE_SIGNATURE);
    entity_manager.SetEntityTag(entity_id, "crosshair");

    component_manager.AddComponent<Transform>(entity_id, transform);
    component_manager.AddComponent<Texture>(entity_id, texture);
    component_manager.AddComponent<Quad>(entity_id, quad);

    entity_id++;

    // Bullets
    for(uint32_t i = 0; i < 32; i++)
    {
        transform.position[0] = 0;
        transform.position[1] = 0;
        transform.position[2] = 0;
        transform.rotation[0] = 0;
        transform.rotation[1] = 0;
        transform.rotation[2] = 0;
        transform.scale[0] = 0;
        transform.scale[1] = 0;
        transform.scale[2] = 0;
        
        texture.texture_index = 2;
        texture.position[0] = 0;
        texture.position[1] = 257;
        texture.size[0] = 63;
        texture.size[1] = 63;

        quad.extent[0] = 0.1;
        quad.extent[1] = 0.1;

        bounding_box.extent[0] = 0.1;
        bounding_box.extent[1] = 0.1;
        bounding_box.extent[2] = 0.1;

        RigidBody rigid_body;
        rigid_body.velocity[0] = 0;
        rigid_body.velocity[1] = 0;
        rigid_body.velocity[2] = 0;
        rigid_body.acceleration[0] = 0;
        rigid_body.acceleration[1] = 0;
        rigid_body.acceleration[2] = 0;

        entity_manager.SetEntityState(entity_id, EntityState::INACTIVE);
        entity_manager.SetEntitySignature(entity_id, PHYSICS_SYSTEM_SIGNATURE |
                                                        COLLISION_SYSTEM_SIGNATURE);
        char bullet_tag[10];
        sprintf(bullet_tag, "bullet_%d", i);
        entity_manager.SetEntityTag(entity_id, bullet_tag);

        component_manager.AddComponent<Transform>(entity_id, transform);
        component_manager.AddComponent<Texture>(entity_id, texture);
        component_manager.AddComponent<Quad>(entity_id, quad);

        entity_id++;
    }

    printf("Num Entities: %d\n", entity_id);
}


void GenerateFloor(EntityManager& entity_manager, ComponentManager& component_manager, uint32_t& entity_id, vec3 offset,
                        uint32_t num_windows, float window_width, float window_height, float wall_total_width, float wall_height, bool my_building)
{
    
    float window_interval = wall_total_width / (num_windows + 1);
    float outer_wall_segment_width = window_interval - window_width / 2;
    float inner_wall_segment_width = window_interval - window_width;

    float wall_pos_x = -(wall_total_width / 2) + (outer_wall_segment_width / 2);
    float window_pos_x = -(wall_total_width / 2) + window_interval;

    float front_wall_offset = 1;
    float back_wall_offset = -1;
    if(my_building)
    {
        front_wall_offset = -1;
        back_wall_offset = 1;
    }

    // Front Wall Entities
    for(uint32_t i = 0; i < num_windows + 1; i++)
    {    
        float wall_width = inner_wall_segment_width;
        if(i == 0 || i == num_windows)
        {
            wall_width = outer_wall_segment_width;
        }
        Transform transform;
        transform.position[0] = wall_pos_x + offset[0];
        transform.position[1] = wall_height / 2 + offset[1];
        transform.position[2] = front_wall_offset + offset[2];
        transform.rotation[0] = 0;
        transform.rotation[1] = 0;
        transform.rotation[2] = 0;
        transform.scale[0] = 0;
        transform.scale[1] = 0;
        transform.scale[2] = 0;

        if(i == 0 || i == num_windows - 1)
        {
            wall_pos_x += outer_wall_segment_width / 2 + window_width + inner_wall_segment_width / 2;
        }
        else
        {
            wall_pos_x += window_width + inner_wall_segment_width;
        }
        
        Quad quad;
        quad.extent[0] = wall_width;
        quad.extent[1] = wall_height;

        if(my_building)
        {
            quad.normal[0] = 0;
            quad.normal[1] = 0;
            quad.normal[2] = 1;
        }
        else
        {
            quad.normal[0] = 0;
            quad.normal[1] = 0;
            quad.normal[2] = -1;
        }

        Texture texture;
        
        if(my_building)
        {
            texture.texture_index = 2;
            texture.position[0] = 0;
            texture.position[1] = 0;
            texture.size[0] = 256;
            texture.size[1] = 256;
            texture.use_light = true;
        }
        else
        {
            texture.texture_index = 0;
            texture.position[0] = 0;
            texture.position[1] = 0;
            texture.size[0] = 64 * wall_width;
            texture.size[1] = 64 * 2;
            texture.use_light = false;
        }

        entity_manager.SetEntityState(entity_id, EntityState::ACTIVE);
        uint32_t signature = RENDER_SYSTEM_SIGNATURE;
        if(!my_building)
        {
            signature |= XRAY_SYSTEM_SIGNATURE;
        }
        entity_manager.SetEntitySignature(entity_id, signature);

        component_manager.AddComponent<Transform>(entity_id, transform);
        component_manager.AddComponent<Quad>(entity_id, quad);
        component_manager.AddComponent<Texture>(entity_id, texture);

        entity_id++;
    }

    // Front Window Entities
    for(uint32_t i = 0; i < num_windows; i++)
    {
        // Bottom Wall
        Transform transform;
        transform.position[0] = window_pos_x + offset[0];
        transform.position[1] = wall_height / 3 / 2 + offset[1];
        transform.position[2] = front_wall_offset + offset[2];
        transform.rotation[0] = 0;
        transform.rotation[1] = 0;
        transform.rotation[2] = 0;
        transform.scale[0] = 0;
        transform.scale[1] = 0;
        transform.scale[2] = 0;

        Quad quad;
        quad.extent[0] = window_width;
        quad.extent[1] = wall_height / 3;

        if(my_building)
        {
            quad.normal[0] = 0;
            quad.normal[1] = 0;
            quad.normal[2] = 1;
        }
        else
        {
            quad.normal[0] = 0;
            quad.normal[1] = 0;
            quad.normal[2] = -1;
        }

        Texture texture;
        if(my_building)
        {
            texture.texture_index = 2;
            texture.position[0] = 0;
            texture.position[1] = 0;
            texture.size[0] = 256;
            texture.size[1] = 256 / 3;
            texture.use_light = true;
        }
        else
        {
            texture.texture_index = 0;
            texture.position[0] = 0;
            texture.position[1] = 0;
            texture.size[0] = 64;
            texture.size[1] = 64;
            texture.use_light = false;
        }

        entity_manager.SetEntityState(entity_id, EntityState::ACTIVE);
        uint32_t signature = RENDER_SYSTEM_SIGNATURE;
        if(!my_building)
        {
            signature |= XRAY_SYSTEM_SIGNATURE;
        }
        entity_manager.SetEntitySignature(entity_id, signature);

        component_manager.AddComponent<Transform>(entity_id, transform);
        component_manager.AddComponent<Quad>(entity_id, quad);
        component_manager.AddComponent<Texture>(entity_id, texture);

        entity_id++;

        // Top Wall
        transform.position[0] = window_pos_x + offset[0];
        transform.position[1] = wall_height - (wall_height / 3 / 2) + offset[1];
        transform.position[2] = front_wall_offset + offset[2];
        transform.rotation[0] = 0;
        transform.rotation[1] = 0;
        transform.rotation[2] = 0;
        transform.scale[0] = 0;
        transform.scale[1] = 0;
        transform.scale[2] = 0;

        quad.extent[0] = window_width;
        quad.extent[1] = wall_height / 3;

        if(my_building)
        {
            quad.normal[0] = 0;
            quad.normal[1] = 0;
            quad.normal[2] = 1;
        }
        else
        {
            quad.normal[0] = 0;
            quad.normal[1] = 0;
            quad.normal[2] = -1;
        }

        if(my_building)
        {
            texture.texture_index = 2;
            texture.position[0] = 0;
            texture.position[1] = 256 - (256 / 3);
            texture.size[0] = 256;
            texture.size[1] = 256 / 3;
            texture.use_light = true;
        }
        else
        {
            texture.texture_index = 0;
            texture.position[0] = 0;
            texture.position[1] = 0;
            texture.size[0] = 64;
            texture.size[1] = 64;
            texture.use_light = false;
        }

        entity_manager.SetEntityState(entity_id, EntityState::ACTIVE);
        signature = RENDER_SYSTEM_SIGNATURE;
        if(!my_building)
        {
            signature |= XRAY_SYSTEM_SIGNATURE;
        }
        entity_manager.SetEntitySignature(entity_id, signature);

        component_manager.AddComponent<Transform>(entity_id, transform);
        component_manager.AddComponent<Quad>(entity_id, quad);
        component_manager.AddComponent<Texture>(entity_id, texture);

        entity_id++;

        // Window
        transform.position[0] = window_pos_x + offset[0];
        transform.position[1] = wall_height / 3 + wall_height / 3 / 2 + offset[1];
        transform.position[2] = front_wall_offset + offset[2];
        transform.rotation[0] = 0;
        transform.rotation[1] = 0;
        transform.rotation[2] = 0;
        transform.scale[0] = 0;
        transform.scale[1] = 0;
        transform.scale[2] = 0;

        if(my_building)
        {
            printf("%f %f %f\n", transform.position[0], transform.position[1], transform.position[2]);
        }

        quad.extent[0] = window_width;
        quad.extent[1] = wall_height / 3;

        if(my_building)
        {
            quad.normal[0] = 0;
            quad.normal[1] = 0;
            quad.normal[2] = 1;
        }
        else
        {
            quad.normal[0] = 0;
            quad.normal[1] = 0;
            quad.normal[2] = -1;
        }

        texture.texture_index = 1;
        texture.position[0] = 0;
        texture.position[1] = 320;
        texture.size[0] = 128;
        texture.size[1] = 192;

        entity_manager.SetEntityState(entity_id, EntityState::ACTIVE);
        signature = RENDER_SYSTEM_SIGNATURE;
        if(!my_building)
        {
            signature |= XRAY_SYSTEM_SIGNATURE;
        }
        entity_manager.SetEntitySignature(entity_id, signature);

        component_manager.AddComponent<Transform>(entity_id, transform);
        component_manager.AddComponent<Quad>(entity_id, quad);
        component_manager.AddComponent<Texture>(entity_id, texture);

        entity_id++;

        window_pos_x += window_interval;
    }
    
    // Wall Entity
    Transform transform;
    transform.position[0] = 0 + offset[0];
    transform.position[1] = wall_height / 2 + offset[1];
    transform.position[2] = back_wall_offset + offset[2];
    transform.rotation[0] = 0;
    if(my_building)
    {
        transform.rotation[1] = 180;
    }
    else
    {
        transform.rotation[1] = 0;
    }
    transform.rotation[2] = 0;
    transform.scale[0] = 0;
    transform.scale[1] = 0;
    transform.scale[2] = 0;
    
    Quad quad;
    quad.extent[0] = wall_total_width;
    quad.extent[1] = wall_height;

    if(my_building)
    {
        quad.normal[0] = 0;
        quad.normal[1] = 0;
        quad.normal[2] = -1;
    }
    else
    {
        quad.normal[0] = 0;
        quad.normal[1] = 0;
        quad.normal[2] = 1;
    }
    
    Texture texture;
    texture.texture_index = 2;
    texture.position[0] = 0;
    texture.position[1] = 0;
    texture.size[0] = 256;
    texture.size[1] = 256;
    texture.use_light = true;
    
    BoundingBox bounding_box;
    bounding_box.extent[0] = wall_total_width;
    bounding_box.extent[1] = wall_height;
    bounding_box.extent[2] = 1;

    entity_manager.SetEntityState(entity_id, EntityState::ACTIVE);
    entity_manager.SetEntitySignature(entity_id, RENDER_SYSTEM_SIGNATURE |
                                                    COLLISION_SYSTEM_SIGNATURE);
    entity_manager.SetEntityTag(entity_id, "back_wall");

    component_manager.AddComponent<Transform>(entity_id, transform);
    component_manager.AddComponent<Quad>(entity_id, quad);
    component_manager.AddComponent<Texture>(entity_id, texture);
    component_manager.AddComponent<BoundingBox>(entity_id, bounding_box);

    entity_id++;
    
    // Side Wall Entity
    transform.position[0] = (wall_total_width / 2) + offset[0];
    transform.position[1] = wall_height / 2 + offset[1];
    transform.position[2] = 0 + offset[2];
    transform.rotation[0] = 0;
    transform.rotation[1] = -90;
    transform.rotation[2] = 0;
    transform.scale[0] = 0;
    transform.scale[1] = 0;
    transform.scale[2] = 0;
    
    quad.extent[0] = 2;
    quad.extent[1] = wall_height;
    quad.normal[0] = -1;
    quad.normal[1] = 0;
    quad.normal[2] = 0;
    
    texture.texture_index = 2;
    texture.position[0] = 0;
    texture.position[1] = 0;
    texture.size[0] = 256;
    texture.size[1] = 256;
    texture.use_light = true;
    
    bounding_box.extent[0] = 1;
    bounding_box.extent[1] = wall_height;
    bounding_box.extent[2] = 2;

    entity_manager.SetEntityState(entity_id, EntityState::ACTIVE);
    entity_manager.SetEntitySignature(entity_id, RENDER_SYSTEM_SIGNATURE |
                                                    COLLISION_SYSTEM_SIGNATURE);
    entity_manager.SetEntityTag(entity_id, "side_wall");

    component_manager.AddComponent<Transform>(entity_id, transform);
    component_manager.AddComponent<Quad>(entity_id, quad);
    component_manager.AddComponent<Texture>(entity_id, texture);
    component_manager.AddComponent<BoundingBox>(entity_id, bounding_box);

    entity_id++;
    
    // Side Wall Entity
    transform.position[0] = -(wall_total_width / 2) + offset[0];
    transform.position[1] = wall_height / 2 + offset[1];
    transform.position[2] = 0 + offset[2];
    transform.rotation[0] = 0;
    transform.rotation[1] = 90;
    transform.rotation[2] = 0;
    transform.scale[0] = 0;
    transform.scale[1] = 0;
    transform.scale[2] = 0;
    
    quad.extent[0] = 2;
    quad.extent[1] = wall_height;
    quad.normal[0] = 1;
    quad.normal[1] = 0;
    quad.normal[2] = 0;
    
    texture.texture_index = 2;
    texture.position[0] = 0;
    texture.position[1] = 0;
    texture.size[0] = 256;
    texture.size[1] = 256;
    texture.use_light = true;
    
    bounding_box.extent[0] = 1;
    bounding_box.extent[1] = wall_height;
    bounding_box.extent[2] = 2;

    entity_manager.SetEntityState(entity_id, EntityState::ACTIVE);
    entity_manager.SetEntitySignature(entity_id, RENDER_SYSTEM_SIGNATURE |
                                                    COLLISION_SYSTEM_SIGNATURE);
    entity_manager.SetEntityTag(entity_id, "side_wall");

    component_manager.AddComponent<Transform>(entity_id, transform);
    component_manager.AddComponent<Quad>(entity_id, quad);
    component_manager.AddComponent<Texture>(entity_id, texture);
    component_manager.AddComponent<BoundingBox>(entity_id, bounding_box);

    entity_id++;
    
    // Floor Entity
    transform.position[0] = 0 + offset[0];
    transform.position[1] = 0 + offset[1];
    transform.position[2] = 0 + offset[2];
    transform.rotation[0] = -90;
    transform.rotation[1] = 0;
    transform.rotation[2] = 0;
    transform.scale[0] = 0;
    transform.scale[1] = 0;
    transform.scale[2] = 0;
    
    quad.extent[0] = wall_total_width;
    quad.extent[1] = 2;
    quad.normal[0] = 0;
    quad.normal[1] = 1;
    quad.normal[2] = 0;
    
    texture.texture_index = 3;
    texture.position[0] = 0;
    texture.position[1] = 0;
    texture.size[0] = 128 * 50;
    texture.size[1] = 128 * 5;
    texture.use_light = true;

    entity_manager.SetEntityState(entity_id, EntityState::ACTIVE);
    entity_manager.SetEntitySignature(entity_id, RENDER_SYSTEM_SIGNATURE);

    component_manager.AddComponent<Transform>(entity_id, transform);
    component_manager.AddComponent<Quad>(entity_id, quad);
    component_manager.AddComponent<Texture>(entity_id, texture);

    entity_id++;
    
    // Ceiling Entity
    transform.position[0] = 0 + offset[0];
    transform.position[1] = wall_height + offset[1];
    transform.position[2] = 0 + offset[2];
    transform.rotation[0] = 90;
    transform.rotation[1] = 0;
    transform.rotation[2] = 0;
    transform.scale[0] = 0;
    transform.scale[1] = 0;
    transform.scale[2] = 0;
    
    quad.extent[0] = wall_total_width;
    quad.extent[1] = 2;
    quad.normal[0] = 0;
    quad.normal[1] = -1;
    quad.normal[2] = 0;
    
    texture.texture_index = 4;
    texture.position[0] = 0;
    texture.position[1] = 0;
    texture.size[0] = 128 * 50;
    texture.size[1] = 128 * 5;
    texture.use_light = true;

    entity_manager.SetEntityState(entity_id, EntityState::ACTIVE);
    entity_manager.SetEntitySignature(entity_id, RENDER_SYSTEM_SIGNATURE);

    component_manager.AddComponent<Transform>(entity_id, transform);
    component_manager.AddComponent<Quad>(entity_id, quad);
    component_manager.AddComponent<Texture>(entity_id, texture);

    entity_id++;

    vec3 enemy_extent = { 0.75, 1.5, .75 };

    if(!my_building)
    {
        for(uint32_t i = 0; i < 1; i++)
        {
            // Enemy Entity
            transform.position[0] = (rand() % (int)(wall_total_width - enemy_extent[0])) - ((wall_total_width - enemy_extent[0]) / 2) + offset[0];
            transform.position[1] = enemy_extent[1] / 2 + offset[1];
            transform.position[2] = 0 + offset[2];
            transform.rotation[0] = 0;
            transform.rotation[1] = 0;
            transform.rotation[2] = 0;
            transform.scale[0] = 0;
            transform.scale[1] = 0;
            transform.scale[2] = 0;
            
            quad.extent[0] = enemy_extent[0];
            quad.extent[1] = enemy_extent[1];
            float speed = ((rand() % 1) == 1) ? 1 : -1;

            texture.texture_index = 6;
            texture.position[0] = 0;
            texture.position[1] = speed > 0 ? 0 : 128;
            texture.size[0] = 64;
            texture.size[1] = 128;
            texture.use_light = true;

            BoundingBox bounding_box;
            bounding_box.extent[0] = enemy_extent[0];
            bounding_box.extent[1] = enemy_extent[1];
            bounding_box.extent[2] = enemy_extent[2];

            RigidBody rigid_body;
            rigid_body.velocity[0] = speed;
            rigid_body.velocity[1] = 0;
            rigid_body.velocity[2] = 0;
            rigid_body.acceleration[0] = 0;
            rigid_body.acceleration[1] = 0;
            rigid_body.acceleration[2] = 0;

            Animation animation;
            animation.counter = 0;
            animation.current_frame = 0;
            animation.num_frames = 4;
            animation.paused = false;
            animation.speed = 0.25;

            AIData ai_data;
            ai_data.speed = speed;
            ai_data.alive = true;
            ai_data.initial_height = transform.position[1];
            ai_data.position[0] = transform.position[0];
            ai_data.position[1] = transform.position[1];
            ai_data.position[2] = transform.position[2];
            ai_data.rotation[0] = transform.rotation[0];
            ai_data.rotation[1] = transform.rotation[1];
            ai_data.rotation[2] = transform.rotation[2];

            entity_manager.SetEntityState(entity_id, EntityState::ACTIVE);
            entity_manager.SetEntitySignature(entity_id, RENDER_SYSTEM_SIGNATURE | 
                                                            PHYSICS_SYSTEM_SIGNATURE |
                                                            COLLISION_SYSTEM_SIGNATURE |
                                                            AI_SYSTEM_SIGNATURE);
            entity_manager.SetEntityTag(entity_id, "enemy");

            component_manager.AddComponent<Transform>(entity_id, transform);
            component_manager.AddComponent<Quad>(entity_id, quad);
            component_manager.AddComponent<Texture>(entity_id, texture);
            component_manager.AddComponent<BoundingBox>(entity_id, bounding_box);
            component_manager.AddComponent<RigidBody>(entity_id, rigid_body);
            component_manager.AddComponent<Animation>(entity_id, animation);
            component_manager.AddComponent<AIData>(entity_id, ai_data);

            entity_id++;
        }
    }
}