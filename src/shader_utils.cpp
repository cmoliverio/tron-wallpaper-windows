#include <shader_utils.hpp>

// Helper to read shader file into a string
std::string read_shader_file(const std::string& filepath) {
    std::ifstream file(filepath);
    if (!file.is_open()) {
        std::cerr << "ERROR::SHADER::FILE_NOT_READABLE: " << filepath << std::endl;
        return "";
    }
    std::stringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
}

// Function to compile a shader from file
uint32_t compile_shader(const std::string& filepath, GLenum shaderType) {
    std::string shaderCode = read_shader_file(filepath);
    const char* shaderSource = shaderCode.c_str();

    uint32_t shader = glCreateShader(shaderType);
    glShaderSource(shader, 1, &shaderSource, nullptr);
    glCompileShader(shader);

    int32_t success;
    char infoLog[512];
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(shader, 512, nullptr, infoLog);
        std::cerr << "ERROR::SHADER::" 
                  << (shaderType == GL_VERTEX_SHADER ? "VERTEX" : "FRAGMENT")
                  << "::COMPILATION_FAILED\n" 
                  << filepath << "\n" << infoLog << std::endl;
    }

    return shader;
}

uint32_t link_shader(uint32_t shader_program) 
{
    int32_t success;
    char infoLog[512];
    glLinkProgram(shader_program);
    
    glGetShaderiv(shader_program, GL_LINK_STATUS, &success);
    if (!success) { 
        glGetProgramInfoLog(shader_program, 512, NULL, infoLog);
        std::cerr << "ERROR::SHADER::LINKING\n" 
                  << infoLog << std::endl;
    }

    return success;
}