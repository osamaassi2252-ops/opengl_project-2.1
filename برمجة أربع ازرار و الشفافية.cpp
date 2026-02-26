#include <iostream>
#include <cstdlib>
#include <ctime>

// 1. تضمين المكتبات
#define GLEW_STATIC
#include <GL/glew.h>
#include <GLFW/glfw3.h>

// إعدادات النافذة
const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;

// --- 2. كود المظللات (معدل لإزاحة الرؤوس واستقبال لون موحد) ---

// Vertex Shader: يستقبل إزاحة (offset) لتطبيقها على جميع رؤوس الشكل
const char* vertexShaderSource = "#version 330 core\n"
"layout (location = 0) in vec3 aPos;\n"
"uniform vec3 offset;\n"                // متغير الإزاحة (X, Y, Z)
"void main()\n"
"{\n"
"   gl_Position = vec4(aPos + offset, 1.0);\n"
"}\0";

// Fragment Shader: يستقبل لون الشكل (مع ألفا للشفافية)
const char* fragmentShaderSource = "#version 330 core\n"
"out vec4 FragColor;\n"
"uniform vec4 objectColor;\n"
"void main()\n"
"{\n"
"   FragColor = objectColor;\n"
"}\n\0";

// متغيرات التحكم
bool transparencyEnabled = true;
float triangle1OffsetX = 0.0f;          // إزاحة المثلث الأول على X
float triangle2OffsetZ = 0.0f;          // إزاحة المثلث الثاني على Z
float color1[3] = { 1.0f, 0.5f, 0.2f };   // برتقالي
float color2[3] = { 0.2f, 0.5f, 1.0f };   // أزرق فاتح

// دالة معالجة تغيير حجم النافذة
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    glViewport(0, 0, width, height);
}

// دالة معالجة المدخلات (الأزرار)
void processInput(GLFWwindow* window)
{
    // وضع الرسم (خطوط / تعبئة) - كما في الكود الأصلي
    if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    else
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

    // زر 1: تشغيل / إيقاف الشفافية
    static bool key1Pressed = false;
    if (glfwGetKey(window, GLFW_KEY_1) == GLFW_PRESS && !key1Pressed)
    {
        transparencyEnabled = !transparencyEnabled;
        if (transparencyEnabled)
        {
            glEnable(GL_BLEND);
            std::cout << "Transparency ON\n";
        }
        else
        {
            glDisable(GL_BLEND);
            std::cout << "Transparency OFF\n";
        }
        key1Pressed = true;
    }
    if (glfwGetKey(window, GLFW_KEY_1) == GLFW_RELEASE)
        key1Pressed = false;

    // زر 2: تغيير لون المثلث الأول عشوائياً
    static bool key2Pressed = false;
    if (glfwGetKey(window, GLFW_KEY_2) == GLFW_PRESS && !key2Pressed)
    {
        color1[0] = static_cast<float>(rand()) / RAND_MAX;
        color1[1] = static_cast<float>(rand()) / RAND_MAX;
        color1[2] = static_cast<float>(rand()) / RAND_MAX;
        std::cout << "Triangle 1 color changed to ("
            << color1[0] << ", " << color1[1] << ", " << color1[2] << ")\n";
        key2Pressed = true;
    }
    if (glfwGetKey(window, GLFW_KEY_2) == GLFW_RELEASE)
        key2Pressed = false;

    // زر 3: تحريك المثلث الأول على المحور X
    static bool key3Pressed = false;
    static bool moveDirectionX = true;
    if (glfwGetKey(window, GLFW_KEY_3) == GLFW_PRESS && !key3Pressed)
    {
        if (moveDirectionX)
            triangle1OffsetX += 0.2f;
        else
            triangle1OffsetX -= 0.2f;

        if (triangle1OffsetX > 1.0f || triangle1OffsetX < -1.0f)
            moveDirectionX = !moveDirectionX;

        std::cout << "Triangle 1 offset X = " << triangle1OffsetX << "\n";
        key3Pressed = true;
    }
    if (glfwGetKey(window, GLFW_KEY_3) == GLFW_RELEASE)
        key3Pressed = false;

    // زر 4: تحريك المثلث الثاني على المحور Z
    static bool key4Pressed = false;
    static bool moveDirectionZ = true;
    if (glfwGetKey(window, GLFW_KEY_4) == GLFW_PRESS && !key4Pressed)
    {
        if (moveDirectionZ)
            triangle2OffsetZ -= 0.2f;   // أقرب للكاميرا
        else
            triangle2OffsetZ += 0.2f;   // أبعد عن الكاميرا

        if (triangle2OffsetZ < -1.5f || triangle2OffsetZ > 1.5f)
            moveDirectionZ = !moveDirectionZ;

        std::cout << "Triangle 2 offset Z = " << triangle2OffsetZ << "\n";
        key4Pressed = true;
    }
    if (glfwGetKey(window, GLFW_KEY_4) == GLFW_RELEASE)
        key4Pressed = false;
}
int main()
{
    // تهيئة البذرة العشوائية
    srand(static_cast<unsigned>(time(nullptr)));

    // --- 3. تهيئة GLFW ---
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT,
        "Two Shapes with Controls (no GLM)", NULL, NULL);
    if (!window)
    {
        std::cout << "Failed to create GLFW window\n";
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    // --- 4. تهيئة GLEW ---
    glewExperimental = GL_TRUE;
    if (glewInit() != GLEW_OK)
    {
        std::cout << "Failed to initialize GLEW\n";
        return -1;
    }

    // --- تفعيل اختبار العمق والشفافية ---
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // --- 5. بناء برنامج الشيدر ---
    // تجميع Vertex Shader
    unsigned int vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
    glCompileShader(vertexShader);
    int success;
    char infoLog[512];
    glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
        std::cout << "Vertex shader compilation failed:\n" << infoLog << "\n";
    }

    // تجميع Fragment Shader
    unsigned int fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
    glCompileShader(fragmentShader);
    glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(fragmentShader, 512, NULL, infoLog);
        std::cout << "Fragment shader compilation failed:\n" << infoLog << "\n";
    }

    // الربط
    unsigned int shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);
    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
    if (!success)
    {
        glGetProgramInfoLog(shaderProgram, 512, NULL, infoLog);
        std::cout << "Shader program linking failed:\n" << infoLog << "\n";
    }
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    // --- 6. تعريف بيانات المثلثين (بإحداثيات Z مختلفة) ---
    // المثلث الأول: Z = -0.5 (أقرب للكاميرا)
    float vertices1[] = {
        -0.5f, -0.5f, -0.5f,
         0.5f, -0.5f, -0.5f,
         0.0f,  0.5f, -0.5f
    };
    // المثلث الثاني: Z = 0.5 (أبعد)
    float vertices2[] = {
        -0.5f, -0.5f,  0.5f,
         0.5f, -0.5f,  0.5f,
         0.0f,  0.5f,  0.5f
    };

    // --- 7. إعداد VAO و VBO لكل مثلث ---
    unsigned int VBO[2], VAO[2];
    glGenVertexArrays(2, VAO);
    glGenBuffers(2, VBO);

    // المثلث الأول
    glBindVertexArray(VAO[0]);
    glBindBuffer(GL_ARRAY_BUFFER, VBO[0]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices1), vertices1, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    // المثلث الثاني
    glBindVertexArray(VAO[1]);
    glBindBuffer(GL_ARRAY_BUFFER, VBO[1]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices2), vertices2, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    // --- 8. مواقع المتغيرات الموحدة (uniforms) في الشيدر ---
    GLint offsetLoc = glGetUniformLocation(shaderProgram, "offset");
    GLint colorLoc = glGetUniformLocation(shaderProgram, "objectColor");

    // --- 9. حلقة الرسم ---
    while (!glfwWindowShouldClose(window))
    {
        // معالجة المدخلات
        processInput(window);
        // تنظيف الشاشة ومسخ العمق
        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glUseProgram(shaderProgram);

        // رسم المثلث الأول (بالإزاحة على X)
        glUniform3f(offsetLoc, triangle1OffsetX, 0.0f, 0.0f);
        glUniform4f(colorLoc,
            color1[0], color1[1], color1[2],
            transparencyEnabled ? 0.6f : 1.0f);   // ألفا 0.6 إذا الشفافية مفعلة
        glBindVertexArray(VAO[0]);
        glDrawArrays(GL_TRIANGLES, 0, 3);

        // رسم المثلث الثاني (بالإزاحة على Z)
        glUniform3f(offsetLoc, 0.0f, 0.0f, triangle2OffsetZ);
        glUniform4f(colorLoc,
            color2[0], color2[1], color2[2],
            transparencyEnabled ? 0.4f : 1.0f);   // ألفا مختلفة
        glBindVertexArray(VAO[1]);
        glDrawArrays(GL_TRIANGLES, 0, 3);

        // تبديل المخازن ومعالجة الأحداث
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // --- 10. تنظيف ---
    glDeleteVertexArrays(2, VAO);
    glDeleteBuffers(2, VBO);
    glDeleteProgram(shaderProgram);

    glfwTerminate();
    return 0;
}