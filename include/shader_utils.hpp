#ifndef SHADER_UTILS_HPP
#define SHADER_UTILS_HPP

#include <glad/glad.h>
#include <iostream>
#include <string>
#include <fstream>
#include <sstream>



std::string read_shader_file(const std::string& filepath);
uint32_t compile_shader(const std::string& filepath, GLenum shaderType);
uint32_t link_shader(uint32_t shader_program);

#endif // SHADER_UTILS_HPP