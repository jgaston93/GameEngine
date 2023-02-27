#include <string.h>

#include "InputMap.hpp"

InputMap::InputMap(uint32_t num_inputs) : m_max_num_inupts(num_inputs),
                                          m_num_inputs(0),
                                          m_input_map(new bool[num_inputs]),
                                          m_input_index_map(new uint32_t[num_inputs]),
                                          m_mouse_pos_x(0),
                                          m_mouse_pos_y(0)
{
    memset(m_input_map, 0, num_inputs * sizeof(bool));
    memset(m_input_index_map, 0, num_inputs * sizeof(uint32_t));
}

InputMap::~InputMap()
{
    delete[] m_input_map;
    delete[] m_input_index_map;
}


void InputMap::AddInput(uint32_t input)
{
    m_input_index_map[m_num_inputs++] = input;
}

void InputMap::SetIsPressed(uint32_t input, bool is_pressed)
{
    for(uint32_t i = 0; i < m_num_inputs; i++)
    {
        if(m_input_index_map[i] == input)
        {
            m_input_map[i] = is_pressed;
        }
    }
}

bool InputMap::IsPressed(uint32_t input)
{
    bool is_pressed = false;

    for(uint32_t i = 0; i < m_num_inputs; i++)
    {
        if(m_input_index_map[i] == input)
        {
            is_pressed = m_input_map[i];
        }
    }

    return is_pressed;
}

void InputMap::SetMousePosX(double pos_x)
{
    m_mouse_pos_x = pos_x;
}

void InputMap::SetMousePosY(double pos_y)
{
    m_mouse_pos_y = pos_y;
}

double InputMap::GetMousePosX()
{
    return m_mouse_pos_x;
}

double InputMap::GetMousePosY()
{
    return m_mouse_pos_y;
}
