#include <iostream>
#include <vector>
#include <map>
#include <string>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
const GLuint WIDTH = 800, HEIGHT = 600;
float playerX = 400.0f;
const float playerY = 20.0f;
const float PLAYER_SPRITE_WIDTH = 100.0f;
const float PLAYER_SPEED = 10.0f;
const float PLAYER_SCREEN_X = (WIDTH / 2.0f) - (PLAYER_SPRITE_WIDTH / 2.0f);

struct Layer {
    std::string name_key;
    float speed;
    float y_pos;
    float height;
};

const char* vertexShaderSource = R"(
#version 330 core
layout (location = 0) in vec3 position;
layout (location = 1) in vec2 texCoord;

uniform mat4 model;
uniform mat4 projection;
uniform float u_TexScrollX;

out vec2 TexCoord;

void main() {
    gl_Position = projection * model * vec4(position, 1.0);
    TexCoord = vec2(texCoord.x + u_TexScrollX, 1.0 - texCoord.y);
}
)";

const char* fragmentShaderSource = R"(
#version 330 core
in vec2 TexCoord;
out vec4 color;
uniform sampler2D tex;

void main() {
    color = texture(tex, TexCoord);
}
)";

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);
GLuint loadTexture(const char* path);
GLuint setupShader();
GLuint createQuad();

int main() {
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* window = glfwCreateWindow(WIDTH, HEIGHT, "Parallax Scrolling", NULL, NULL);
    if (!window) {
        std::cerr << "Falha ao criar janela GLFW" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSetKeyCallback(window, key_callback);
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cerr << "Falha ao inicializar GLAD" << std::endl;
        return -1;
    }
    glViewport(0, 0, WIDTH, HEIGHT);

    GLuint shaderID = setupShader();
    GLuint quadVAO  = createQuad();
    glm::mat4 projection = glm::ortho(0.0f, (float)WIDTH, 0.0f, (float)HEIGHT);

    std::map<std::string, GLuint> textures = {
        {"ceu",       loadTexture("imagensVivencial2/ceu.png")},
        {"lua",       loadTexture("imagensVivencial2/lua.png")},
        {"trees_far", loadTexture("imagensVivencial2/arvore3.png")},
        {"trees_mid", loadTexture("imagensVivencial2/arvore2.png")},
        {"branches",  loadTexture("imagensVivencial2/arvore1.png")},
        {"player",    loadTexture("imagensVivencial2/gato.png")}
    };

    std::vector<Layer> layers = {
        {"ceu", 0.1f, 0.0f, (float)HEIGHT},
        {"lua", 0.2f, 250.0f, 450.0f},
        {"trees_far", 0.3f, 50.0f, 400.0f},
        {"trees_mid", 0.5f, 20.0f, 350.0f},
        {"branches", 0.8f, 0.0f, 400.0f}
    };

    glUseProgram(shaderID);
    glUniform1i(glGetUniformLocation(shaderID, "tex"), 0);
    GLint locScroll = glGetUniformLocation(shaderID, "u_TexScrollX");
    GLint locModel  = glGetUniformLocation(shaderID, "model");
    GLint locProj   = glGetUniformLocation(shaderID, "projection");
    glUniformMatrix4fv(locProj, 1, GL_FALSE, glm::value_ptr(projection));

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();
        glClearColor(0.1f, 0.1f, 0.2f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        glBindVertexArray(quadVAO);
        for (auto& layer : layers) {
            auto it = textures.find(layer.name_key);
            if (it == textures.end()) continue;
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, it->second);

            float scrollPix = (playerX - (WIDTH / 2.0f)) * layer.speed;
            float scrollUV  = scrollPix / WIDTH;
            glUniform1f(locScroll, scrollUV);

            glm::mat4 model = glm::mat4(1.0f);
            model = glm::translate(model, glm::vec3(0.0f, layer.y_pos, 0.0f));
            model = glm::scale(model, glm::vec3(WIDTH, layer.height, 1.0f));
            glUniformMatrix4fv(locModel, 1, GL_FALSE, glm::value_ptr(model));

            glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
        }

        glUniform1f(locScroll, 0.0f);
        auto itP = textures.find("player");
        if (itP != textures.end()) {
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, itP->second);
            glm::mat4 model = glm::mat4(1.0f);
            model = glm::translate(model, glm::vec3(PLAYER_SCREEN_X, playerY + 80.0f, 0.0f));
            model = glm::scale(model, glm::vec3(180.0f, 180.0f, 1.0f));
            glUniformMatrix4fv(locModel, 1, GL_FALSE, glm::value_ptr(model));
            glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
        }

        glfwSwapBuffers(window);
    }

    glDeleteVertexArrays(1, &quadVAO);
    glDeleteProgram(shaderID);
    for (auto& p : textures) glDeleteTextures(1, &p.second);
    glfwTerminate();
    return 0;
}

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    if (action == GLFW_PRESS || action == GLFW_REPEAT) {
        if (key == GLFW_KEY_LEFT)  playerX -= PLAYER_SPEED;
        if (key == GLFW_KEY_RIGHT) playerX += PLAYER_SPEED;
    }
}

GLuint setupShader() {
    GLint success;
    GLchar infoLog[512];
    GLuint vs = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vs, 1, &vertexShaderSource, NULL);
    glCompileShader(vs);
    glGetShaderiv(vs, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(vs, 512, NULL, infoLog);
        std::cerr << "Erro Vertex Shader:\n" << infoLog << std::endl;
    }
    GLuint fs = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fs, 1, &fragmentShaderSource, NULL);
    glCompileShader(fs);
    glGetShaderiv(fs, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(fs, 512, NULL, infoLog);
        std::cerr << "Erro Fragment Shader:\n" << infoLog << std::endl;
    }
    GLuint program = glCreateProgram();
    glAttachShader(program, vs);
    glAttachShader(program, fs);
    glLinkProgram(program);
    glGetProgramiv(program, GL_LINK_STATUS, &success);
    if (!success) {
        glGetProgramInfoLog(program, 512, NULL, infoLog);
        std::cerr << "Erro Shader Program:\n" << infoLog << std::endl;
    }
    glDeleteShader(vs);
    glDeleteShader(fs);
    return program;
}

GLuint createQuad() {
    float vertices[] = {
        0.0f, 0.0f, 0.0f,  0.0f, 0.0f,
        1.0f, 0.0f, 0.0f,  1.0f, 0.0f,
        0.0f, 1.0f, 0.0f,  0.0f, 1.0f,
        1.0f, 1.0f, 0.0f,  1.0f, 1.0f
    };
    GLuint VAO, VBO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
    return VAO;
}

GLuint loadTexture(const char* path) {
    std::cout << "Carregando imagem: " << path << std::endl;
    GLuint texID;
    glGenTextures(1, &texID);
    glBindTexture(GL_TEXTURE_2D, texID);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    int w, h, nch;
    unsigned char* data = stbi_load(path, &w, &h, &nch, 0);
    if (data) {
        GLenum fmt = (nch == 4 ? GL_RGBA : (nch == 3 ? GL_RGB : GL_RED));
        glTexImage2D(GL_TEXTURE_2D, 0, fmt, w, h, 0, fmt, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);
    } else {
        std::cerr << "Erro ao carregar: " << path
                  << " | STB: " << stbi_failure_reason() << std::endl;
    }
    stbi_image_free(data);
    glBindTexture(GL_TEXTURE_2D, 0);
    return texID;
}
