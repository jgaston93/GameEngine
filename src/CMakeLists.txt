add_executable(Untitled main.cpp
                             gl.c
                             MessageBus.cpp
                             System.cpp
                             ComponentManager.cpp
                             EntityManager.cpp
                             InputMap.cpp
                             PlayerInputSystem.cpp
                             PhysicsSystem.cpp
                             RenderSystem.cpp)

target_include_directories(Untitled PUBLIC ${CMAKE_SOURCE_DIR}/inc
                                                ${CMAKE_SOURCE_DIR}/inc/Components
                                                ${CMAKE_SOURCE_DIR}/inc/Systems
                                                ${CMAKE_SOURCE_DIR}/inc/Utils
                                                ${CMAKE_SOURCE_DIR}/inc/MessageBus)

target_link_options(Untitled PUBLIC -mwindows -static-libgcc -static-libstdc++ -static)
target_link_libraries(Untitled glfw OpenGL::GL)