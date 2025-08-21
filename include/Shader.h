#ifndef SHADER_H
#define SHADER_H

#include <string>
#include <glm.hpp>

class Shader {
public:
    unsigned int ID;
    Shader(const char* vertexPath, const char* fragmentPath);

    void use() const;
    void setMat4(const std::string &name, const glm::mat4 &mat) const;

    void setInt(const std::string &name, int value) const;

private:
    std::string loadFile(const char* path);
    void checkCompileErrors(unsigned int shader, std::string type);
};

#endif
