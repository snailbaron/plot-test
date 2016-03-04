#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <cmath>
#include <tclap/CmdLine.h>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/gtc/matrix_transform.hpp>

static const int WINDOW_WIDTH = 800;
static const int WINDOW_HEIGHT = 600;
static const char WINDOW_TITLE[] = "plot";

static GLuint g_program;
static glm::mat4 g_transform = glm::ortho<float>(-1.0f, 1.0f, -1.0f, 1.0f);
static float g_zoom = 1.0;
static bool g_moving = false;
static float g_x = 0.0f, g_y = 0.0f;

void RecalcTransform()
{
    std::cout << "Zoom: " << g_zoom << std::endl;
    std::cout << "Position: (" << g_x << ", " << g_y << ")" << std::endl;

    g_transform = glm::ortho<float>(g_x - g_zoom, g_x + g_zoom, g_y - g_zoom, g_y + g_zoom);
    GLint transformUniformLocation = glGetUniformLocation(g_program, "Transform");
    glUniformMatrix4fv(transformUniformLocation, 1, GL_FALSE, &g_transform[0][0]);
}

void DebugCallback(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* message, const void* userParam)
{
    std::cerr << "OpenGL Error: " << message << std::endl;
}

void ScrollCallback(GLFWwindow *window, double xoffset, double yoffset)
{
    std::cout << "Scroll: (" << xoffset << ", " << yoffset << ")" << std::endl;

    float minZoom = 0.001f, maxZoom = 1000.0f;
    g_zoom *= (float)pow(1.5, -yoffset);
    if (g_zoom < minZoom) g_zoom = minZoom;
    if (g_zoom > maxZoom) g_zoom = maxZoom;
    
    RecalcTransform();
}

void MouseButtonCallback(GLFWwindow *window, int button, int action, int mods)
{
    if (button == GLFW_MOUSE_BUTTON_LEFT) {
        switch (action) {
            case GLFW_PRESS:   g_moving = true;  break;
            case GLFW_RELEASE: g_moving = false; break;
        }
    }
}

void MouseMoveCallback(GLFWwindow *window, double xpos, double ypos)
{
    float x = (float)xpos, y = (float)ypos;
    static float xprev = x, yprev = y;
    float dx = x - xprev, dy = y - yprev;
    xprev = x;
    yprev = y;
    if (g_moving) {
        g_x -= dx * g_zoom / 1000.f;
        g_y += dy * g_zoom / 1000.f;
    }
    RecalcTransform();        
}

GLuint CompileShader(GLenum shaderType, const std::string &fileName)
{
    std::cout << "Compiling shader from file: " << fileName << std::endl;

    // Grab shader source code into string
    std::ifstream ifs(fileName);
    if (!ifs.is_open()) {
        std::cerr << "Failed to open shader source file: " << fileName << std::endl;
    }
    std::string source( (std::istreambuf_iterator<char>(ifs)), (std::istreambuf_iterator<char>()) );
    ifs.close();

    const char *sourcePtr = source.c_str();
    const GLint sourceLen = static_cast<GLint>(source.length());

    GLuint shader = glCreateShader(shaderType);
    glShaderSource(shader, 1, &sourcePtr, &sourceLen);
    glCompileShader(shader);
    GLint compileSuccess;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &compileSuccess);
    std::cout << "Compilation status: " << (compileSuccess ? "OK" : "FAIL") << std::endl;
    GLint infoLogLen;
    glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &infoLogLen);
    if (infoLogLen > 0) {
        std::vector<GLchar> infoLog(infoLogLen);
        glGetShaderInfoLog(shader, infoLogLen, nullptr, &infoLog[0]);
        std::cout << "Compilation log:" << std::endl << &infoLog[0] << std::endl;
    } else {
        std::cout << "Compilation log is empty" << std::endl;
    }
    return shader;
}

int main(int argc, char *argv[])
{
    // Command line arguments
    std::string infile;

    // Parse command line
    try {
        TCLAP::CmdLine cmd("plotter");
        TCLAP::ValueArg<std::string> infileArg("i", "input", "Input file with chart points", true, "", "string", cmd);
        cmd.parse(argc, argv);
        infile = infileArg.getValue();
    } catch (TCLAP::CmdLineParseException &e) {
        std::cerr << "Command line error: " << e.error() << std::endl;
        return 1;
    }

    // Initialize GLFW
    if (glfwInit() != GL_TRUE) {
        return 1;
    }

    // Create GLFW window and OpenGL context
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    GLFWwindow *window = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, WINDOW_TITLE, nullptr, nullptr);
    if (!window) {
        return 1;
    }
    glfwMakeContextCurrent(window);

    glfwSetScrollCallback(window, ScrollCallback);
    glfwSetMouseButtonCallback(window, MouseButtonCallback);
    glfwSetCursorPosCallback(window, MouseMoveCallback);

    // Initiallize GLEW with experimental drivers support
    glewExperimental = true;
    GLenum glewErr = glewInit();
    if (glewErr != GLEW_OK) {
        std::cerr << "GLEW Error: " << glewGetErrorString(glewErr) << std::endl;
        return 1;
    }

    std::cout << "Initialization done" << std::endl;

    glPointSize(3.0f);


    glDebugMessageCallback(DebugCallback, nullptr);

    std::cout << "Building shader program" << std::endl;

    GLuint vertexShader = CompileShader(GL_VERTEX_SHADER, "../../src/plot.vert");
    GLuint fragmentShader = CompileShader(GL_FRAGMENT_SHADER, "../../src/plot.frag");
    g_program = glCreateProgram();
    glAttachShader(g_program, vertexShader);
    glAttachShader(g_program, fragmentShader);
    glLinkProgram(g_program);
    GLint linkSuccess;
    glGetProgramiv(g_program, GL_LINK_STATUS, &linkSuccess);
    std::cout << "Link status: " << (linkSuccess ? "OK" : "FAIL") << std::endl;
    GLint linkLogLen;
    glGetProgramiv(g_program, GL_INFO_LOG_LENGTH, &linkLogLen);
    if (linkLogLen > 0) {
        std::vector<GLchar> linkLog(linkLogLen);
        glGetProgramInfoLog(g_program, linkLogLen, nullptr, &linkLog[0]);
        std::cout << "Link log:" << std::endl << &linkLog[0] << std::endl;
    } else {
        std::cout << "Link log is empty" << std::endl;
    }
    glUseProgram(g_program);   
    
    std::cout << "Shader program is built and used" << std::endl;

    std::cout << "Reading points from input file" << std::endl;

    // Read chart points from input file
    float minX=0.f, maxX=0.f, minY=0.f, maxY=0.f;   // I'm lazy
    std::ifstream ifs(infile, std::ios::binary);
    if (!ifs.is_open()) {
        std::cerr << "Failed to open input file: " << infile << std::endl;
        return 1;
    }
    GLsizei pointCount;
    ifs.read((char*)&pointCount, 4);
    std::vector<float> points;
    for (GLsizei i = 0; i < pointCount; i++) {
        float x, y;
        ifs.read((char*)&x, 4);
        ifs.read((char*)&y, 4);

        points.push_back(x);
        points.push_back(y);

        if (x < minX) minX = x;
        if (x > maxX) maxX = x;
        if (y < minY) minY = y;
        if (y > maxY) maxY = y;
    }

    std::cout << "Points read into memory" << std::endl;

    GLuint vao;
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    std::cout << "Generating and filling OpenGL buffer" << std::endl;
    GLuint vertBuffer;
    glGenBuffers(1, &vertBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, vertBuffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * points.size(), &points[0], GL_STATIC_DRAW);
    std::cout << "Buffer created and filled" << std::endl;
    
    GLint vertPos = glGetAttribLocation(g_program, "VertexPosition");
    glEnableVertexAttribArray(vertPos);
    glBindVertexBuffer(0, vertBuffer, 0, sizeof(float) * 2);
    glVertexAttribFormat(vertPos, 2, GL_FLOAT, GL_FALSE, 0);
    glVertexAttribBinding(vertPos, 0);

    //glm::mat4 transform = glm::ortho<float>(minX - 1.f, maxX + 1.f, minY - 1.f, maxY + 1.f);
    //glm::mat4 transform = glm::ortho<float>(-2.0f, 100.0f, -2.0f, 2.0f);
    RecalcTransform();
    GLint transformUniformLocation = glGetUniformLocation(g_program, "Transform");
    glUniformMatrix4fv(transformUniformLocation, 1, GL_FALSE, &g_transform[0][0]);

    // Main loop
    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glDrawArrays(GL_POINTS, 0, pointCount);

        glfwSwapBuffers(window);
    }

    
    // Cleanup GLFW resources
    glfwDestroyWindow(window);
    glfwTerminate();
}