#define _CRT_SECURE_NO_WARNINGS  // لتجنب تحذيرات الأمان (اختياري إذا لم تعمل localtime_s)
#include <iostream>
#include <cmath>
#include <ctime>

#define GLEW_STATIC
#include <GL/glew.h>
#include <GLFW/glfw3.h>

const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;

// Callback for window resize
void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
    glViewport(0, 0, width, height);
}

// Convert degrees to radians
float degToRad(float deg) {
    return deg * 3.14159265359f / 180.0f;
}

// Get current time and compute hand angles (in degrees)
void getTimeAngles(float& hourAngle, float& minAngle, float& secAngle) {
    time_t now = time(nullptr);
    struct tm localTime;

    // Use localtime_s for safety (Windows)
    if (localtime_s(&localTime, &now) != 0) {
        // Fallback in case of error
        hourAngle = 0; minAngle = 0; secAngle = 0;
        return;
    }

    int hours = localTime.tm_hour % 12;
    int minutes = localTime.tm_min;
    int seconds = localTime.tm_sec;

    secAngle = seconds * 6.0f;
    minAngle = minutes * 6.0f + seconds * 0.1f;
    hourAngle = hours * 30.0f + minutes * 0.5f;
}

// Vertex shader
const char* vertexShaderSource = R"(
#version 330 core
layout (location = 0) in vec2 aPos;
uniform float uAngle;
void main() {
    float c = cos(uAngle);
    float s = sin(uAngle);
    vec2 rotatedPos = vec2(
        aPos.x * c - aPos.y * s,
        aPos.x * s + aPos.y * c
    );
    gl_Position = vec4(rotatedPos, 0.0, 1.0);
}
)";

// Fragment shader
const char* fragmentShaderSource = R"(
#version 330 core
out vec4 FragColor;
uniform vec4 uColor;
void main() {
    FragColor = uColor;
}
)";

int main() {
    // Initialize GLFW
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "Analog Clock", NULL, NULL);
    if (!window) {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    // Initialize GLEW
    glewExperimental = GL_TRUE;
    if (glewInit() != GLEW_OK) {
        std::cout << "Failed to initialize GLEW" << std::endl;
        return -1;
    }

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // Build shader program
    unsigned int vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
    glCompileShader(vertexShader);
    int success;
    char infoLog[512];
    glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
        std::cout << "Vertex shader compilation failed:\n" << infoLog << std::endl;
    }

    unsigned int fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
    glCompileShader(fragmentShader);
    glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(fragmentShader, 512, NULL, infoLog);
        std::cout << "Fragment shader compilation failed:\n" << infoLog << std::endl;
    }

    unsigned int shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);
    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
    if (!success) {
        glGetProgramInfoLog(shaderProgram, 512, NULL, infoLog);
        std::cout << "Shader linking failed:\n" << infoLog << std::endl;
    }
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    GLint angleLoc = glGetUniformLocation(shaderProgram, "uAngle");
    GLint colorLoc = glGetUniformLocation(shaderProgram, "uColor");
    // Create clock face (circle) using VBO/EBO
    const int SEGMENTS = 60;
    float radius = 0.8f;
    int vertexCount = SEGMENTS + 1;
    float* circleVertices = new float[vertexCount * 2];
    circleVertices[0] = 0.0f;
    circleVertices[1] = 0.0f;
    for (int i = 0; i < SEGMENTS; ++i) {
        float angle = 2.0f * 3.14159265359f * i / SEGMENTS;
        circleVertices[(i + 1) * 2] = radius * cos(angle);
        circleVertices[(i + 1) * 2 + 1] = radius * sin(angle);
    }

    unsigned int* indices = new unsigned int[SEGMENTS * 3];
    for (int i = 0; i < SEGMENTS; ++i) {
        indices[i * 3] = 0;
        indices[i * 3 + 1] = i + 1;
        indices[i * 3 + 2] = (i + 1) % SEGMENTS + 1;
    }

    unsigned int VAO_circle, VBO_circle, EBO_circle;
    glGenVertexArrays(1, &VAO_circle);
    glGenBuffers(1, &VBO_circle);
    glGenBuffers(1, &EBO_circle);

    glBindVertexArray(VAO_circle);
    glBindBuffer(GL_ARRAY_BUFFER, VBO_circle);
    glBufferData(GL_ARRAY_BUFFER, vertexCount * 2 * sizeof(float), circleVertices, GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO_circle);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, SEGMENTS * 3 * sizeof(unsigned int), indices, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glBindVertexArray(0);

    delete[] circleVertices;
    delete[] indices;

    // Second hand
    float secHandVertices[] = {
        -0.02f, 0.0f,  0.02f, 0.0f,  -0.02f, 0.7f,  0.02f, 0.7f
    };
    unsigned int handIndices[] = { 0, 1, 2, 1, 3, 2 };
    unsigned int VAO_sec, VBO_sec, EBO_sec;
    glGenVertexArrays(1, &VAO_sec);
    glGenBuffers(1, &VBO_sec);
    glGenBuffers(1, &EBO_sec);
    glBindVertexArray(VAO_sec);
    glBindBuffer(GL_ARRAY_BUFFER, VBO_sec);
    glBufferData(GL_ARRAY_BUFFER, sizeof(secHandVertices), secHandVertices, GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO_sec);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(handIndices), handIndices, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glBindVertexArray(0);

    // Minute hand
    float minHandVertices[] = {
        -0.03f, 0.0f,  0.03f, 0.0f,  -0.03f, 0.6f,  0.03f, 0.6f
    };
    unsigned int VAO_min, VBO_min, EBO_min;
    glGenVertexArrays(1, &VAO_min);
    glGenBuffers(1, &VBO_min);
    glGenBuffers(1, &EBO_min);
    glBindVertexArray(VAO_min);
    glBindBuffer(GL_ARRAY_BUFFER, VBO_min);
    glBufferData(GL_ARRAY_BUFFER, sizeof(minHandVertices), minHandVertices, GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO_min);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(handIndices), handIndices, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glBindVertexArray(0);

    // Hour hand
    float hourHandVertices[] = {
        -0.04f, 0.0f,  0.04f, 0.0f,  -0.04f, 0.5f,  0.04f, 0.5f
    };
    unsigned int VAO_hour, VBO_hour, EBO_hour;
    glGenVertexArrays(1, &VAO_hour);
    glGenBuffers(1, &VBO_hour);
    glGenBuffers(1, &EBO_hour);
    glBindVertexArray(VAO_hour);
    glBindBuffer(GL_ARRAY_BUFFER, VBO_hour);
    glBufferData(GL_ARRAY_BUFFER, sizeof(hourHandVertices), hourHandVertices, GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO_hour);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(handIndices), handIndices, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glBindVertexArray(0);

    // Markers (12,3,6,9) as lines
    float markerLines[] = {
        0.0f, 0.75f,   0.0f, 0.85f,
        0.75f, 0.0f,   0.85f, 0.0f,
        0.0f, -0.75f,  0.0f, -0.85f,
        -0.75f, 0.0f,  -0.85f, 0.0f
    };
    unsigned int VAO_markers, VBO_markers;
    glGenVertexArrays(1, &VAO_markers);
    glGenBuffers(1, &VBO_markers);
    glBindVertexArray(VAO_markers);
    glBindBuffer(GL_ARRAY_BUFFER, VBO_markers);
    glBufferData(GL_ARRAY_BUFFER, sizeof(markerLines), markerLines, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glBindVertexArray(0);

    // Main loop
    glClearColor(0.1f, 0.1f, 0.1f, 1.0f);

    float hourAngle, minAngle, secAngle;

    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();
        if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
            glfwSetWindowShouldClose(window, true);

        getTimeAngles(hourAngle, minAngle, secAngle);

        glClear(GL_COLOR_BUFFER_BIT);

        glUseProgram(shaderProgram);

        // Draw clock face
        glUniform1f(angleLoc, 0.0f);
        glUniform4f(colorLoc, 0.3f, 0.3f, 0.3f, 1.0f);
        glBindVertexArray(VAO_circle);
        glDrawElements(GL_TRIANGLES, SEGMENTS * 3, GL_UNSIGNED_INT, 0);

        // Draw markers
        glUniform4f(colorLoc, 1.0f, 1.0f, 1.0f, 1.0f);
        glBindVertexArray(VAO_markers);
        glDrawArrays(GL_LINES, 0, 8);

        // Draw second hand
        glUniform1f(angleLoc, degToRad(secAngle));
        glUniform4f(colorLoc, 1.0f, 0.0f, 0.0f, 0.8f);
        glBindVertexArray(VAO_sec);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

        // Draw minute hand
        glUniform1f(angleLoc, degToRad(minAngle));
        glUniform4f(colorLoc, 1.0f, 1.0f, 1.0f, 1.0f);
        glBindVertexArray(VAO_min);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

        // Draw hour hand
        glUniform1f(angleLoc, degToRad(hourAngle));
        glUniform4f(colorLoc, 0.9f, 0.9f, 0.9f, 1.0f);
        glBindVertexArray(VAO_hour);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

        glfwSwapBuffers(window);
    }

    // Cleanup
    glDeleteVertexArrays(1, &VAO_circle);
    glDeleteBuffers(1, &VBO_circle);
    glDeleteBuffers(1, &EBO_circle);
    glDeleteVertexArrays(1, &VAO_sec);
    glDeleteBuffers(1, &VBO_sec);
    glDeleteBuffers(1, &EBO_sec);
    glDeleteVertexArrays(1, &VAO_min);
    glDeleteBuffers(1, &VBO_min);
    glDeleteBuffers(1, &EBO_min);
    glDeleteVertexArrays(1, &VAO_hour);
    glDeleteBuffers(1, &VBO_hour);
    glDeleteBuffers(1, &EBO_hour);
    glDeleteVertexArrays(1, &VAO_markers);
    glDeleteBuffers(1, &VBO_markers);
    glDeleteProgram(shaderProgram);

    glfwTerminate();
    return 0;
}




