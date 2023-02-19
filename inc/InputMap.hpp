#ifndef INPUT_MAP_HPP
#define INPUT_MAP_HPP

#include <stdint.h>

class InputMap
{
    public:
    InputMap(uint32_t m_num_inputs);
    ~InputMap();

    void AddInput(uint32_t input);
    void SetIsPressed(uint32_t input, bool is_pressed);
    bool IsPressed(uint32_t input);

    private:
    const uint32_t m_max_num_inupts;
    uint32_t m_num_inputs;
    uint32_t* m_input_index_map;
    bool* m_input_map;
};

#endif // INPUT_MAP_HPP