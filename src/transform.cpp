#include <glad/glad.h>
#include <glfw/glfw3.h>
#include <cstdio>
#include <string>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "stb_image.h"


#include "shader.h"

#define OGL_VERSION_MAJOR           4
#define OGL_VERSION_MINOR           6

//////////////////////////////////////////////////////////
static const char g_strVertShader[] = R"(
    #version 460 core
    layout (location = 0) in vec3 aPos;
    layout (location = 1) in vec3 aColor;
    layout (location = 2) in vec2 aTexCoord;
    
    out vec3 ourColor;
    out vec2 TexCoord;
    
    void main()
    {
        gl_Position = vec4(aPos, 1.0);
        ourColor = aColor;
        TexCoord = aTexCoord;
    }

)";
static const char g_fragVertShader[] = R"(
    #version 460 core
    out vec4 FragColor;

    in vec3 ourColor;
    in vec2 TexCoord;

    uniform sampler2D texture1;
    uniform sampler2D texture2;

    void main()
    {
        FragColor = mix(texture(texture1, TexCoord), texture(texture2, TexCoord), 0.2);
    }

)";

float vertices[] = {
    //     ---- 位置 ----       ---- 颜色 ----     - 纹理坐标 -
         0.5f,  0.5f, 0.0f,   1.0f, 0.0f, 0.0f,   1.0f, 1.0f,   // 右上
         0.5f, -0.5f, 0.0f,   0.0f, 1.0f, 0.0f,   1.0f, 0.0f,   // 右下
        -0.5f, -0.5f, 0.0f,   0.0f, 0.0f, 1.0f,   0.0f, 0.0f,   // 左下
        -0.5f,  0.5f, 0.0f,   1.0f, 1.0f, 0.0f,   0.0f, 1.0f    // 左上
};

unsigned int indices[] = { // 注意索引从0开始! 
    0, 1, 3, // 第一个三角形
    1, 2, 3  // 第二个三角形
};

/////////////////////////////////////////////


void processInput(GLFWwindow* pWindow);
int initGLFW();
GLFWwindow* createMainWindow(int width, int height, const std::string& title);
bool initGlad();
Shader* createShader(const std::string& strVertex, const std::string& strFrag);

/////////////////////////////////////////////////

int main(int argc, char* argv[])
{
    if (initGLFW())
        return -1;
    GLFWwindow* pMainWindow = createMainWindow(800, 600, "TITLE");
    if (!pMainWindow)
        return -2;
    glfwMakeContextCurrent(pMainWindow);
    if (!initGlad())
        return -3;
    auto pShader = createShader(g_strVertShader, g_fragVertShader);
    if (!pShader)
        return -4;

    //initialize
    GLuint VAO, VBO, IBO, texture1, texture2;
    {
        glGenVertexArrays(1, &VAO);
        glBindVertexArray(VAO);
        glGenBuffers(1, &VBO);
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), nullptr);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (const void*)(3 * sizeof(float)));
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (const void*)(6 * sizeof(float)));
        glEnableVertexAttribArray(2);
        glGenBuffers(1, &IBO);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IBO);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

        stbi_set_flip_vertically_on_load(true);

        glGenTextures(1, &texture1);
        glBindTexture(GL_TEXTURE_2D, texture1);
        //set texture paramerters
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        //load and generate texture
        int imgWidth, imgHeight, nrChannels;
        unsigned char* pData = stbi_load("D:\\workspace\\OpenGLSampleCode\\res\\container.jpg", &imgWidth, &imgHeight, &nrChannels, 0);
        assert(pData);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, imgWidth, imgHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, pData);
        glGenerateMipmap(GL_TEXTURE_2D);
        stbi_image_free(pData);


        glGenTextures(1, &texture2);
        glBindTexture(GL_TEXTURE_2D, texture2);
        //set texture paramerters
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        pData = stbi_load("D:\\workspace\\OpenGLSampleCode\\res\\awesomeface.png", &imgWidth, &imgHeight, &nrChannels, 0);
        assert(pData);
        //png image has alpa channel, so we should use GL_RGBA
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, imgWidth, imgHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, pData);
        glGenerateMipmap(GL_TEXTURE_2D);
        stbi_image_free(pData);

        pShader->use();
        pShader->setUniform("texture1", 0);
        pShader->setUniform("texture2", 1);
    }

    glEnable(GL_COLOR_BUFFER_BIT);
    glClearColor(0.5, 0.5, 0.5, 0);
    glfwMakeContextCurrent(pMainWindow);
    while (!glfwWindowShouldClose(pMainWindow)) {
        processInput(pMainWindow);

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, texture1);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, texture2);
        pShader->use();
        glBindVertexArray(VAO);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr);

        glfwSwapBuffers(pMainWindow);
        glfwPollEvents();
    }

    pShader->unuse();
    delete pShader;

    return 0;
}



///////////////////////////////////////////////////////////

void processInput(GLFWwindow* pWindow)
{
    if (glfwGetKey(pWindow, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
        glfwSetWindowShouldClose(pWindow, true);
    }
}

int initGLFW()
{
    if (GLFW_TRUE != glfwInit())
        return -1;
    glfwSetErrorCallback([](int errCode, const char* strErr) {
        char errorString[1024] = { 0 };
        sprintf(errorString, "GLFW ERROR: %d %s\r\n", errCode, strErr);
#ifdef _WIN32
        OutputDebugString(errorString);
#else
        printf(errorString);
#endif
    });

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, OGL_VERSION_MAJOR);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, OGL_VERSION_MINOR);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_COMPAT_PROFILE, GL_TRUE);
#endif
    glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);

    return 0;
}

GLFWwindow* createMainWindow(int width, int height, const std::string& title)
{
    GLFWwindow* pMainWindow = glfwCreateWindow(width, height, title.c_str(), NULL, NULL);
    if (!pMainWindow) {
        glfwTerminate();
        return nullptr;
    }

    glfwMakeContextCurrent(pMainWindow);
    glfwSetFramebufferSizeCallback(pMainWindow, [](GLFWwindow* pWindow, int width, int height) {
        glViewport(0, 0, width, height);
    });

    return pMainWindow;
}

bool initGlad()
{
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        printf("Failed to load opengl");
        return false;
    }
    return true;
}

Shader* createShader(const std::string& strVertex, const std::string& strFrag)
{
    std::string errLog;
    Shader* pShader = new Shader();
    if (!pShader->attachShaderSource(GL_VERTEX_SHADER, strVertex, &errLog)) {
        printf("log is: %s", errLog.c_str());
        delete pShader;
        return nullptr;
    }
    errLog.clear();
    if (!pShader->attachShaderSource(GL_FRAGMENT_SHADER, strFrag, &errLog)) {
        printf("log is: %s", errLog.c_str());
        delete pShader;
        return nullptr;
    }
    errLog.clear();
    if (!pShader->compile(&errLog)) {
        printf("log is: %s", errLog.c_str());
        delete pShader;
        return nullptr;
    }
    return pShader;
}
