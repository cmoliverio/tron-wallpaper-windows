#include "shader.hpp"

Shader::Shader(const char* vertexPath, const char* fragmentPath)
{
    // 1. retrieve the vertex/fragment source code from filePath
    std::string vertexCode;
    std::string fragmentCode;
    std::ifstream vShaderFile;
    std::ifstream fShaderFile;
    // ensure ifstream objects can throw exceptions:
    vShaderFile.exceptions (std::ifstream::failbit | std::ifstream::badbit);
    fShaderFile.exceptions (std::ifstream::failbit | std::ifstream::badbit);
    try 
    {
        // open files
        vShaderFile.open(vertexPath);
        fShaderFile.open(fragmentPath);
        std::stringstream vShaderStream, fShaderStream;
        // read file's buffer contents into streams
        vShaderStream << vShaderFile.rdbuf();
        fShaderStream << fShaderFile.rdbuf();		
        // close file handlers
        vShaderFile.close();
        fShaderFile.close();
        // convert stream into string
        vertexCode   = vShaderStream.str();
        fragmentCode = fShaderStream.str();		
    }
    catch(std::ifstream::failure e)
    {
        std::cout << "ERROR::SHADER::FILE_NOT_SUCCESFULLY_READ" << std::endl;
    }
    const char* vShaderCode = vertexCode.c_str();
    const char* fShaderCode = fragmentCode.c_str();

    // vertex Shader
    uint32_t vertex = compile(
        GL_VERTEX_SHADER,
        vShaderCode
        );

    uint32_t fragment = compile(
        GL_FRAGMENT_SHADER,
        fShaderCode
    );

    // shader Program
    ID = glCreateProgram();
    glAttachShader(ID, vertex);
    glAttachShader(ID, fragment);
    glLinkProgram(ID);
    int32_t success = 0;
    glGetProgramiv(ID, GL_LINK_STATUS, &success);
    if(!success)
    {
        int32_t log_length = 0;
        glGetProgramiv(ID, GL_INFO_LOG_LENGTH, &log_length);
        std::vector<char> log(log_length + 1, '\0');
        glGetProgramInfoLog(ID, log_length, nullptr, log.data());
        std::cout << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n" << log.data() << std::endl;
        std::terminate();
    }
      
    // delete the shaders as they're linked into our program now and no longer necessary
    glDeleteShader(vertex);
    glDeleteShader(fragment);
}

uint32_t Shader::compile(GLenum shader_type, 
        const char* source)
{
    GLuint shader = glCreateShader(shader_type);
    glShaderSource(shader, 1, &source, nullptr);
    glCompileShader(shader);

    GLint success = GL_FALSE;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        GLint logLength = 0;
        glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &logLength);

        std::vector<char> log(logLength + 1, '\0');
        glGetShaderInfoLog(shader, logLength, nullptr, log.data());

        std::cerr
            << "ERROR::SHADER::"
            << (shader_type == GL_FRAGMENT_SHADER ? "FRAGMENT" : "VERTEX")
            << "::COMPILATION_FAILED\n"
            << log.data()
            << std::endl;

        std::terminate();
    }

    return shader;
}

void Shader::use() 
{ 
    glUseProgram(ID);
}  

void Shader::setBool(const std::string &name, bool value) const
{         
    glUniform1i(glGetUniformLocation(ID, name.c_str()), (int)value); 
}
void Shader::setInt(const std::string &name, int value) const
{ 
    glUniform1i(glGetUniformLocation(ID, name.c_str()), value); 
}
void Shader::setFloat(const std::string &name, float value) const
{ 
    glUniform1f(glGetUniformLocation(ID, name.c_str()), value); 
} 