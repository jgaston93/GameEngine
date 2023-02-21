#ifndef UI_SYSTEM_HPP
#define UI_SYSTEM_HPP

#include <glad/gl.h>
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

#include "System.hpp"
#include "Signatures.hpp"

class UISystem : public System
{
    public:
    UISystem(MessageBus& message_bus, GLFWwindow* window);
    ~UISystem();
    
    void HandleMessage(Message message);
    void HandleEntity(uint32_t entity_id, float delta_time);
    void Update(float delta_time);

    private:
    GLFWwindow* m_window;
    uint32_t m_shader_program;
    uint32_t m_ui_texture;
};

#endif // UI_SYSTEM_HPP