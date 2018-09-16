#include <glad/glad.h>
#include <glfw/glfw3.h>
#include <cstdio>
#include <string>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>


#include "shader.h"

#define OGL_VERSION_MAJOR           4
#define OGL_VERSION_MINOR           6

//////////////////////////////////////////////////////////
static const char g_strVertShader[] = R"(
    #version 460 core
    layout (location = 0) in vec3 aPos;
    void main()
    {
        gl_Position = vec4(aPos.x, aPos.y, aPos.z, 1.0);
    }

)";
static const char g_fragVertShader[] = R"(
    #version 460 core
    out vec4 FragColor;

    uniform vec4 ourColor;    

    void main()
    {
        FragColor = ourColor;
    }

)";


float vertices[] = {
    0.5f, 0.5f, 0.0f,   // 右上角
    0.5f, -0.5f, 0.0f,  // 右下角
    -0.5f, -0.5f, 0.0f, // 左下角
    -0.5f, 0.5f, 0.0f   // 左上角
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


    //generate buffers
    GLuint g_vao, g_vbo, g_ebo;

    {
        //generate and bind a vao object, then all vbo calls will also bind to it
        //it is a switcher or group that we could combine our vbo operations and switch it together.
        glGenVertexArrays(1, &g_vao);
        glBindVertexArray(g_vao);
        //generate a vbo object
        glGenBuffers(1, &g_vbo);
        //bind it as array buffer, the target GL_ARRAY_BUFFER means position, color and etc
        //it contains many elements in this array
        glBindBuffer(GL_ARRAY_BUFFER, g_vbo);
        glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 3, (void*)0);
        glEnableVertexAttribArray(0);

        glGenBuffers(1, &g_ebo);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, g_ebo);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindVertexArray(0);

        //glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    }

    auto pFnRender = [&]() {
        //glDrawArrays(GL_TRIANGLES, 0, 3);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr);
    };

    glEnable(GL_COLOR_BUFFER_BIT);
    //glEnable(GL_DEPTH_TEST);  //should not use depth test, otherwise, you will got an empty screen.
    glfwMakeContextCurrent(pMainWindow);
    while (!glfwWindowShouldClose(pMainWindow)) {
        processInput(pMainWindow);

        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        pShader->use();
        pShader->setUniform("ourColor", glm::vec4(0.f, sin(glfwGetTime()) / 2 + 0.5f, 0.f, 1.f));
        glBindVertexArray(g_vao);
        pFnRender();

        glfwSwapBuffers(pMainWindow);
        glfwPollEvents();
    }

    glDeleteVertexArrays(1, &g_vao);
    glDeleteBuffers(1, &g_vbo);
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
