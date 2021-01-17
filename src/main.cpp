#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <stb_image.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <learnopengl/filesystem.h>
#include <learnopengl/shader_m.h>
#include <rg/Camera.h>
#include <rg/Function.h>
#include <learnopengl/model.h>
#include <rg/Function.h>

#include <iostream>

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void processInput(GLFWwindow *window);
unsigned int loadTexture(const char *path);
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);
unsigned int loadCubemap(vector<std::string> &faces);

// settings
const unsigned int SCR_WIDTH = 1200;
const unsigned int SCR_HEIGHT = 1000;

float lastX = (float)SCR_WIDTH / 2.0;
float lastY = (float)SCR_HEIGHT / 2.0;
bool firstMouse = true;

// timing
float deltaTime = 0.0f;
float lastFrame = 0.0f;

Function function = Function();

// ProgramState
struct ProgramState {
    float kernel = 0.01f;
    bool ImGuiEnabled = false;
    bool open = false;
    bool effect = false;
    float speed = 0.01f;
    int start = -1;
    Camera camera;
    ProgramState()
    : camera(glm::vec3(25.0f, 5.0f, 0.0f)) {}
    glm::vec3 elevatorPosition = glm::vec3(-9.8f, -6.0f, 7.6f);
    glm::vec3 doorPosition = glm::vec3(-4.8f, 2.32f, -3.28f);
    glm::mat4 view;

    void SaveToDisk(std::string path);
    void LoadFromDisk(std::string path);
};

void ProgramState::SaveToDisk(std::string path) {
    // open ofstream on this path
    std::ofstream out(path);
    out << ImGuiEnabled << '\n'
        << camera.Position.x << '\n'
        << camera.Position.y << '\n'
        << camera.Position.z << '\n'
        << camera.Front.x << '\n'
        << camera.Front.y << '\n'
        << camera.Front.z << '\n'
        << camera.Yaw << '\n'
        << camera.Pitch << '\n'
        << open << '\n';
}

void ProgramState::LoadFromDisk(std::string path) {
    // open ifstream on this path
    std::ifstream in(path);
    if (in) {
        in >> ImGuiEnabled
           >> camera.Position.x
           >> camera.Position.y
           >> camera.Position.z
           >> camera.Front.x
           >> camera.Front.y
           >> camera.Front.z
           >> camera.Yaw
           >> camera.Pitch
           >> open;
    }
}

ProgramState* programState;
void ElevatorImGui(ProgramState* programState);

int main() {
    // glfw: initialize and configure
    // ------------------------------
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    // glfw window creation
    // --------------------
    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "LearnOpenGL", NULL, NULL);
    if (window == NULL) {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetScrollCallback(window, scroll_callback);
    glfwSetKeyCallback(window, key_callback);

    // glad: load all OpenGL function pointers
    // ---------------------------------------
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    // stbi_set_flip_vertically_on_load(true);

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    (void)io;

    // ImGui
    ImGui::StyleColorsDark();
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 330 core");

    // configure global opengl state
    // -----------------------------
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);

    programState = new ProgramState;
    programState->LoadFromDisk("resources/programState.txt");

    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    if (programState->ImGuiEnabled) {
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
    }

    // build and compile shaders
    // -------------------------
    Shader shader(FileSystem::getPath("resources/shaders/vertexShader.vs").c_str(),
                  FileSystem::getPath("resources/shaders/fragmentShader.fs").c_str());
    Shader lightShader(FileSystem::getPath("resources/shaders/lightCube.vs").c_str(),
                       FileSystem::getPath("resources/shaders/lightCube.fs").c_str());
    Shader skyboxShader(FileSystem::getPath("resources/shaders/skybox.vs").c_str(),
                        FileSystem::getPath("resources/shaders/skybox.fs").c_str());
    Shader shaderCubeMaps(FileSystem::getPath("resources/shaders/cubemaps.vs").c_str(),
                          FileSystem::getPath("resources/shaders/cubemaps.fs").c_str());
    Shader screenShader(FileSystem::getPath("resources/shaders/framebuffer.vs").c_str(),
                        FileSystem::getPath("resources/shaders/framebuffer.fs").c_str());
    Shader screenShaderNext(FileSystem::getPath("resources/shaders/framebuffer.vs").c_str(),
                            FileSystem::getPath("resources/shaders/framebufferEffect.fs").c_str());
    Shader lightingShader(FileSystem::getPath("resources/shaders/multi_lights.vs").c_str(),
                          FileSystem::getPath("resources/shaders/multi_lights.fs").c_str());
    Model sofaModel(FileSystem::getPath("resources/objects/sofa/sofa2.obj").c_str());
    Model chairModel(FileSystem::getPath("resources/objects/chair/Wooden Chair.obj").c_str());
    Model stairsModel(FileSystem::getPath("resources/objects/stairs/staircase_180_long.obj").c_str());
    Model tableModel(FileSystem::getPath("resources/objects/table/wood.table.obj").c_str());
    Model deskModel(FileSystem::getPath("resources/objects/desk/CoffeeTable1.obj").c_str());
    Model tvModel(FileSystem::getPath("resources/objects/tv/TV set N140418.obj").c_str());
    Model bedModel(FileSystem::getPath("resources/objects/bed/Bed actual design apriori S N230720.obj").c_str());
    Model lockerModel(FileSystem::getPath("resources/objects/locker/Locker 1.obj").c_str());
    Model bedsideTableModel(FileSystem::getPath("resources/objects/bedside_table/Locker 2.obj").c_str());
    Model elevatorModel(FileSystem::getPath("resources/objects/elevator/untitled.obj").c_str());

    // set up vertex data (and buffer(s)) and configure vertex attributes
    // ------------------------------------------------------------------
    float cubeVertices[] = {
            -0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  0.0f,  0.0f,
            0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  1.0f,  0.0f,
            0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  1.0f,  1.0f,
            0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  1.0f,  1.0f,
            -0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  0.0f,  1.0f,
            -0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  0.0f,  0.0f,

            -0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  0.0f,  0.0f,
            0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  1.0f,  0.0f,
            0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  1.0f,  1.0f,
            0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  1.0f,  1.0f,
            -0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  0.0f,  1.0f,
            -0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  0.0f,  0.0f,

            -0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,  1.0f,  0.0f,
            -0.5f,  0.5f, -0.5f, -1.0f,  0.0f,  0.0f,  1.0f,  1.0f,
            -0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,  0.0f,  1.0f,
            -0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,  0.0f,  1.0f,
            -0.5f, -0.5f,  0.5f, -1.0f,  0.0f,  0.0f,  0.0f,  0.0f,
            -0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,  1.0f,  0.0f,

            0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  1.0f,  0.0f,
            0.5f,  0.5f, -0.5f,  1.0f,  0.0f,  0.0f,  1.0f,  1.0f,
            0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,  0.0f,  1.0f,
            0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,  0.0f,  1.0f,
            0.5f, -0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  0.0f,  0.0f,
            0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  1.0f,  0.0f,

            -0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,  0.0f,  1.0f,
            0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,  1.0f,  1.0f,
            0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,  1.0f,  0.0f,
            0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,  1.0f,  0.0f,
            -0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,  0.0f,  0.0f,
            -0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,  0.0f,  1.0f,

            -0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,  0.0f,  1.0f,
            0.5f,   0.5f, -0.5f,  0.0f,  1.0f,  0.0f,  1.0f,  1.0f,
            0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  1.0f,  0.0f,
            0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  1.0f,  0.0f,
            -0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  0.0f,  0.0f,
            -0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,  0.0f,  1.0f
    };

    float floorVertices[] = {
        -7.0f, 0.0f, -5.0f, 0.0f, 1.0f, 0.0f, 0.0f, 2.0f,
        7.0f, 0.0f, -5.0f, 0.0f, 1.0f, 0.0f, 2.0f, 2.0f,
        -7.0f, 0.0f, 5.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f,

        7.0f, 0.0f, -5.0f, 0.0f, -1.0f, 0.0f, 2.0f, 2.0f,
        -7.0f, 0.0f, 5.0f, 0.0f, -1.0f, 0.0f, 0.0f, 0.0f,
        7.0f, 0.0f, 5.0f, 0.0f, -1.0f, 0.0f, 2.0f, 0.0f
    };

    float skyboxVertices[] = {
            -1.0f,  1.0f, -1.0f,
            -1.0f, -1.0f, -1.0f,
            1.0f, -1.0f, -1.0f,
            1.0f, -1.0f, -1.0f,
            1.0f,  1.0f, -1.0f,
            -1.0f,  1.0f, -1.0f,

            -1.0f, -1.0f,  1.0f,
            -1.0f, -1.0f, -1.0f,
            -1.0f,  1.0f, -1.0f,
            -1.0f,  1.0f, -1.0f,
            -1.0f,  1.0f,  1.0f,
            -1.0f, -1.0f,  1.0f,

            1.0f, -1.0f, -1.0f,
            1.0f, -1.0f,  1.0f,
            1.0f,  1.0f,  1.0f,
            1.0f,  1.0f,  1.0f,
            1.0f,  1.0f, -1.0f,
            1.0f, -1.0f, -1.0f,

            -1.0f, -1.0f,  1.0f,
            -1.0f,  1.0f,  1.0f,
            1.0f,  1.0f,  1.0f,
            1.0f,  1.0f,  1.0f,
            1.0f, -1.0f,  1.0f,
            -1.0f, -1.0f,  1.0f,

            -1.0f,  1.0f, -1.0f,
            1.0f,  1.0f, -1.0f,
            1.0f,  1.0f,  1.0f,
            1.0f,  1.0f,  1.0f,
            -1.0f,  1.0f,  1.0f,
            -1.0f,  1.0f, -1.0f,

            -1.0f, -1.0f, -1.0f,
            -1.0f, -1.0f,  1.0f,
            1.0f, -1.0f, -1.0f,
            1.0f, -1.0f, -1.0f,
            -1.0f, -1.0f,  1.0f,
            1.0f, -1.0f,  1.0f
    };

    float quadVertices[] = {
            // positions   // texCoords
            -1.0f,  1.0f,  0.0f, 1.0f,
            -1.0f, -1.0f,  0.0f, 0.0f,
            1.0f, -1.0f,  1.0f, 0.0f,

            -1.0f,  1.0f,  0.0f, 1.0f,
            1.0f, -1.0f,  1.0f, 0.0f,
            1.0f,  1.0f,  1.0f, 1.0f
    };

    glm::vec3 pointLightPositions[] = {
            glm::vec3(0.0f, 1.9f, 3.6f),
            glm::vec3(0.4f, 1.9f, 4.0f),
            glm::vec3(0.0f, 1.9f, 4.4f),
            glm::vec3(-0.4f, 1.9f, 4.0f),
            // glm::vec3(-5.5f, 4.55f, -2.65f),

            glm::vec3(0.0f, 3.9f, 3.6f),
            glm::vec3(0.4f, 3.9f, 4.0f),
            glm::vec3(0.0f, 3.9f, 4.4f),
            glm::vec3(-0.4f, 3.9f, 4.0f),

            glm::vec3(0.0f, 5.9f, 3.6f),
            glm::vec3(0.4f, 5.9f, 4.0f),
            glm::vec3(0.0f, 5.9f, 4.4f),
            glm::vec3(-0.4f, 5.9f, 4.0f),

            glm::vec3(4.0f, 1.9f, 3.6f),
            glm::vec3(4.4f, 1.9f, 4.0f),
            glm::vec3(4.0f, 1.9f, 4.4f),
            glm::vec3(-4.4f, 1.9f, 4.0f),

            glm::vec3(4.0f, 3.9f, 3.6f),
            glm::vec3(4.4f, 3.9f, 4.0f),
            glm::vec3(4.0f, 3.9f, 4.4f),
            glm::vec3(-4.4f, 3.9f, 4.0f),

            glm::vec3(4.0f, 5.9f, 3.6f),
            glm::vec3(4.4f, 5.9f, 4.0f),
            glm::vec3(4.0f, 5.9f, 4.4f),
            glm::vec3(-4.4f, 5.9f, 4.0f)
    };

    // cube VAO
    unsigned int cubeVAO, cubeVBO;

    glGenVertexArrays(1, &cubeVAO);
    glGenBuffers(1, &cubeVBO);
    glBindVertexArray(cubeVAO);
    glBindBuffer(GL_ARRAY_BUFFER, cubeVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(cubeVertices), &cubeVertices, GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);

    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));

    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));

    glBindVertexArray(0);

    // floor VAO
    unsigned int floorVAO, floorVBO;
    glGenVertexArrays(1, &floorVAO);
    glGenBuffers(1, &floorVBO);
    glBindVertexArray(floorVAO);
    glBindBuffer(GL_ARRAY_BUFFER, floorVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(floorVertices), &floorVertices, GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);

    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));

    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));

    glBindVertexArray(0);

    // lightCube VAO
    unsigned int lightVAO;
    glGenVertexArrays(1, &lightVAO);
    glBindVertexArray(lightVAO);

    glBindBuffer(GL_ARRAY_BUFFER, cubeVBO);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    // skybox VAO
    unsigned int skyboxVAO, skyboxVBO;
    glGenVertexArrays(1, &skyboxVAO);
    glGenBuffers(1, &skyboxVBO);
    glBindVertexArray(skyboxVAO);

    glBindBuffer(GL_ARRAY_BUFFER, skyboxVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(skyboxVertices), &skyboxVertices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    // screen quad VAO
    unsigned int quadVAO, quadVBO;
    glGenVertexArrays(1, &quadVAO);
    glGenBuffers(1, &quadVBO);
    glBindVertexArray(quadVAO);

    glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
    glEnableVertexAttribArray(1);

    // load textures
    // -------------
    unsigned int wall = loadTexture(FileSystem::getPath("resources/textures/wall.jpg").c_str());
    unsigned int floor = loadTexture(FileSystem::getPath("resources/textures/floor.png").c_str());
    unsigned int tile = loadTexture(FileSystem::getPath("resources/textures/tile.png").c_str());
    unsigned int stone = loadTexture(FileSystem::getPath("resources/textures/stone.jpg").c_str());
    unsigned int wood = loadTexture(FileSystem::getPath("resources/textures/Wooden_Chair_default.png").c_str());
    unsigned int glass = loadTexture(FileSystem::getPath("resources/textures/glass.jpg").c_str());

    vector<std::string> faces {
        FileSystem::getPath("resources/textures/space/right.jpg").c_str(),
        FileSystem::getPath("resources/textures/space/left.jpg").c_str(),
        FileSystem::getPath("resources/textures/space/top.jpg").c_str(),
        FileSystem::getPath("resources/textures/space/bottom.jpg").c_str(),
        FileSystem::getPath("resources/textures/space/front.jpg").c_str(),
        FileSystem::getPath("resources/textures/space/back.jpg").c_str()
    };

    unsigned int cubemapTexture = loadCubemap(faces);

    // shader configuration
    // --------------------
    shader.use();
    shader.setInt("texture1", 0);

    skyboxShader.use();
    skyboxShader.setInt("skybox", 0);

    shaderCubeMaps.use();
    shader.setInt("skybox", 0);

    screenShader.use();
    screenShader.setInt("screenTexture", 0);
    // screenShader.setFloat("zoom", programState->zoom);

    screenShaderNext.use();
    screenShaderNext.setInt("screenTexture", 0);

    lightingShader.use();
    lightingShader.setInt("material.diffuse", 0);
    lightingShader.setInt("material.specular", 1);

    // create framebuffer object
    unsigned int fbo;
    glGenFramebuffers(1, &fbo);

    // activate framebuffer object
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);

    // create a color attachment texture
    unsigned int textureColorBuffer;
    // create
    glGenTextures(1, &textureColorBuffer);
    // activate
    glBindTexture(GL_TEXTURE_2D, textureColorBuffer);
    // texture in which we render whole picture
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, SCR_WIDTH, SCR_HEIGHT, 0,
                 GL_RGB, GL_UNSIGNED_BYTE, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    // attach texture attachment to the framebuffer
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, textureColorBuffer, 0);

    // glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH24_STENCIL8, SCR_WIDTH, SCR_HEIGHT, 0,
    //              GL_DEPTH_STENCIL, GL_UNSIGNED_INT_24_8, NULL);
    // -> ovako se ne radi (kao teksture) zbog perfomansi, nema konverzije, samo se vrsi upis u memoriju
    // koriste se samo u fazi depth i blending
    // tri bafera za boju, dubinu (24 byte) i stencil buffer (8 byte)

    // create a render buffer object for depth and stencil attachment
    unsigned int rbo;
    // create
    glGenRenderbuffers(1, &rbo);
    // activate
    glBindRenderbuffer(GL_RENDERBUFFER, rbo);
    // setup render buffer as depth and stencil buffer
    // use a single renderbuffer object for both a depth and stencil buffer
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, SCR_WIDTH, SCR_HEIGHT);
    // attachment
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, rbo);
    // checking
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE) {
        std::cout << "OKAY!\n";
    }
    // da bi smo renderovali ponovo na ekran gasi se frejmbafer
    // turn off the framebuffer
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    // glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

    // using second fragment shader in some moment

    // render loop
    // -----------
    while (!glfwWindowShouldClose(window)) {
        // per-frame time logic
        // --------------------
        float currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        // input
        // -----
        processInput(window);

        // render
        // ------
        glBindFramebuffer(GL_FRAMEBUFFER, fbo);
        // check this function
        glViewport(0, 0, SCR_WIDTH, SCR_HEIGHT);
        glEnable(GL_DEPTH_TEST);

        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

        // BEGIN DRAW SCENE
        shader.use();

        lightingShader.use();
        lightingShader.setVec3("viewPos", programState->camera.Position);
        lightingShader.setFloat("material.shininess", 32.0f);

        lightingShader.setVec3("dirLight.direction", -0.2f, -1.0f, -1.0f);
        lightingShader.setVec3("dirLight.ambient", 0.05f, 0.05f, 0.05f);
        lightingShader.setVec3("dirLight.diffuse", 0.4f, 0.4f, 0.4f);
        lightingShader.setVec3("dirLight.specular", 0.5f, 0.5f, 0.5f);

        lightingShader.setVec3("pointLights[0].position", pointLightPositions[0]);
        lightingShader.setVec3("pointLights[0].ambient", 0.05f, 0.05f, 0.05f);
        lightingShader.setVec3("pointLights[0].diffuse", 0.5f, 0.5f, 0.5f);
        lightingShader.setVec3("pointLights[0].specular", 0.8f, 0.8f, 0.8f);
        lightingShader.setFloat("pointLights[0].constant", 1.0f);
        lightingShader.setFloat("pointLights[0].linear", 0.09);
        lightingShader.setFloat("pointLights[0].quadratic", 0.032);

        lightingShader.setVec3("pointLights[1].position", pointLightPositions[1]);
        lightingShader.setVec3("pointLights[1].ambient", 0.05f, 0.05f, 0.05f);
        lightingShader.setVec3("pointLights[1].diffuse", 0.5f, 0.5f, 0.5f);
        lightingShader.setVec3("pointLights[1].specular", 0.8f, 0.8f, 0.8f);
        lightingShader.setFloat("pointLights[1].constant", 1.0f);
        lightingShader.setFloat("pointLights[1].linear", 0.09);
        lightingShader.setFloat("pointLights[1].quadratic", 0.032);

        lightingShader.setVec3("pointLights[2].position", pointLightPositions[2]);
        lightingShader.setVec3("pointLights[2].ambient", 0.05f, 0.05f, 0.05f);
        lightingShader.setVec3("pointLights[2].diffuse", 0.5f, 0.5f, 0.5f);
        lightingShader.setVec3("pointLights[2].specular", 0.8f, 0.8f, 0.8f);
        lightingShader.setFloat("pointLights[2].constant", 1.0f);
        lightingShader.setFloat("pointLights[2].linear", 0.09);
        lightingShader.setFloat("pointLights[2].quadratic", 0.032);

        lightingShader.setVec3("pointLights[3].position", pointLightPositions[3]);
        lightingShader.setVec3("pointLights[3].ambient", 0.05f, 0.05f, 0.05f);
        lightingShader.setVec3("pointLights[3].diffuse", 0.5f, 0.5f, 0.5f);
        lightingShader.setVec3("pointLights[3].specular", 0.8f, 0.8f, 0.8f);
        lightingShader.setFloat("pointLights[3].constant", 1.0f);
        lightingShader.setFloat("pointLights[3].linear", 0.09);
        lightingShader.setFloat("pointLights[3].quadratic", 0.032);

//        lightingShader.setVec3("pointLights[4].position", pointLightPositions[4]);
//        lightingShader.setVec3("pointLights[4].ambient", 0.05f, 0.05f, 0.05f);
//        lightingShader.setVec3("pointLights[4].diffuse", 0.5f, 0.5f, 0.5f);
//        lightingShader.setVec3("pointLights[4].specular", 0.8f, 0.8f, 0.8f);
//        lightingShader.setFloat("pointLights[4].constant", 1.0f);
//        lightingShader.setFloat("pointLights[4].linear", 0.09);
//        lightingShader.setFloat("pointLights[4].quadratic", 0.032);
//
//        lightingShader.setVec3("pointLights[5].position", pointLightPositions[5]);
//        lightingShader.setVec3("pointLights[5].ambient", 0.05f, 0.05f, 0.05f);
//        lightingShader.setVec3("pointLights[5].diffuse", 0.5f, 0.5f, 0.5f);
//        lightingShader.setVec3("pointLights[5].specular", 0.8f, 0.8f, 0.8f);
//        lightingShader.setFloat("pointLights[5].constant", 1.0f);
//        lightingShader.setFloat("pointLights[5].linear", 0.09);
//        lightingShader.setFloat("pointLights[5].quadratic", 0.032);
//
//        lightingShader.setVec3("pointLights[6].position", pointLightPositions[6]);
//        lightingShader.setVec3("pointLights[6].ambient", 0.05f, 0.05f, 0.05f);
//        lightingShader.setVec3("pointLights[6].diffuse", 0.5f, 0.5f, 0.5f);
//        lightingShader.setVec3("pointLights[6].specular", 0.8f, 0.8f, 0.8f);
//        lightingShader.setFloat("pointLights[6].constant", 1.0f);
//        lightingShader.setFloat("pointLights[6].linear", 0.09);
//        lightingShader.setFloat("pointLights[6].quadratic", 0.032);
//
//        lightingShader.setVec3("pointLights[7].position", pointLightPositions[7]);
//        lightingShader.setVec3("pointLights[7].ambient", 0.05f, 0.05f, 0.05f);
//        lightingShader.setVec3("pointLights[7].diffuse", 0.5f, 0.5f, 0.5f);
//        lightingShader.setVec3("pointLights[7].specular", 0.8f, 0.8f, 0.8f);
//        lightingShader.setFloat("pointLights[7].constant", 1.0f);
//        lightingShader.setFloat("pointLights[7].linear", 0.09);
//        lightingShader.setFloat("pointLights[7].quadratic", 0.032);
//
//        lightingShader.setVec3("pointLights[8].position", pointLightPositions[8]);
//        lightingShader.setVec3("pointLights[8].ambient", 0.05f, 0.05f, 0.05f);
//        lightingShader.setVec3("pointLights[8].diffuse", 0.5f, 0.5f, 0.5f);
//        lightingShader.setVec3("pointLights[8].specular", 0.8f, 0.8f, 0.8f);
//        lightingShader.setFloat("pointLights[8].constant", 1.0f);
//        lightingShader.setFloat("pointLights[8].linear", 0.09);
//        lightingShader.setFloat("pointLights[8].quadratic", 0.032);
//
//        lightingShader.setVec3("pointLights[9].position", pointLightPositions[9]);
//        lightingShader.setVec3("pointLights[9].ambient", 0.05f, 0.05f, 0.05f);
//        lightingShader.setVec3("pointLights[9].diffuse", 0.5f, 0.5f, 0.5f);
//        lightingShader.setVec3("pointLights[9].specular", 0.8f, 0.8f, 0.8f);
//        lightingShader.setFloat("pointLights[9].constant", 1.0f);
//        lightingShader.setFloat("pointLights[9].linear", 0.09);
//        lightingShader.setFloat("pointLights[9].quadratic", 0.032);
//
//        lightingShader.setVec3("pointLights[10].position", pointLightPositions[10]);
//        lightingShader.setVec3("pointLights[10].ambient", 0.05f, 0.05f, 0.05f);
//        lightingShader.setVec3("pointLights[10].diffuse", 0.5f, 0.5f, 0.5f);
//        lightingShader.setVec3("pointLights[10].specular", 0.8f, 0.8f, 0.8f);
//        lightingShader.setFloat("pointLights[10].constant", 1.0f);
//        lightingShader.setFloat("pointLights[10].linear", 0.09);
//        lightingShader.setFloat("pointLights[10].quadratic", 0.032);
//
//        lightingShader.setVec3("pointLights[11].position", pointLightPositions[11]);
//        lightingShader.setVec3("pointLights[11].ambient", 0.05f, 0.05f, 0.05f);
//        lightingShader.setVec3("pointLights[11].diffuse", 0.5f, 0.5f, 0.5f);
//        lightingShader.setVec3("pointLights[11].specular", 0.8f, 0.8f, 0.8f);
//        lightingShader.setFloat("pointLights[11].constant", 1.0f);
//        lightingShader.setFloat("pointLights[11].linear", 0.09);
//        lightingShader.setFloat("pointLights[11].quadratic", 0.032);
//
//        lightingShader.setVec3("pointLights[12].position", pointLightPositions[12]);
//        lightingShader.setVec3("pointLights[12].ambient", 0.05f, 0.05f, 0.05f);
//        lightingShader.setVec3("pointLights[12].diffuse", 0.5f, 0.5f, 0.5f);
//        lightingShader.setVec3("pointLights[12].specular", 0.8f, 0.8f, 0.8f);
//        lightingShader.setFloat("pointLights[12].constant", 1.0f);
//        lightingShader.setFloat("pointLights[12].linear", 0.09);
//        lightingShader.setFloat("pointLights[12].quadratic", 0.032);
//
//        lightingShader.setVec3("pointLights[13].position", pointLightPositions[13]);
//        lightingShader.setVec3("pointLights[13].ambient", 0.05f, 0.05f, 0.05f);
//        lightingShader.setVec3("pointLights[13].diffuse", 0.5f, 0.5f, 0.5f);
//        lightingShader.setVec3("pointLights[13].specular", 0.8f, 0.8f, 0.8f);
//        lightingShader.setFloat("pointLights[13].constant", 1.0f);
//        lightingShader.setFloat("pointLights[13].linear", 0.09);
//        lightingShader.setFloat("pointLights[13].quadratic", 0.032);
//
//        lightingShader.setVec3("pointLights[14].position", pointLightPositions[14]);
//        lightingShader.setVec3("pointLights[14].ambient", 0.05f, 0.05f, 0.05f);
//        lightingShader.setVec3("pointLights[14].diffuse", 0.5f, 0.5f, 0.5f);
//        lightingShader.setVec3("pointLights[14].specular", 0.8f, 0.8f, 0.8f);
//        lightingShader.setFloat("pointLights[14].constant", 1.0f);
//        lightingShader.setFloat("pointLights[14].linear", 0.09);
//        lightingShader.setFloat("pointLights[14].quadratic", 0.032);

//        lightingShader.setVec3("spotLight.position", programState->camera.Position);
//        lightingShader.setVec3("spotLight.direction", programState->camera.Front);
//        lightingShader.setVec3("spotLight.ambient", 0.0f, 0.0f, 0.0f);
//        lightingShader.setVec3("spotLight.diffuse", 1.0f, 1.0f, 1.0f);
//        lightingShader.setVec3("spotLight.specular", 1.0f, 1.0f, 1.0f);
//        lightingShader.setFloat("spotLight.constant", 1.0f);
//        lightingShader.setFloat("spotLight.linear", 0.09);
//        lightingShader.setFloat("spotLight.quadratic", 0.032);
//        lightingShader.setFloat("spotLight.cutOff", glm::cos(glm::radians(12.5f)));
//        lightingShader.setFloat("spotLight.outerCutOff", glm::cos(glm::radians(15.0f)));

        // camera
        programState->view = programState->camera.GetViewMatrix();
        glm::mat4 projection = glm::perspective(glm::radians(programState->camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
        glm::mat4 model = glm::mat4(1.0f);

        lightingShader.setMat4("projection", projection);
        lightingShader.setMat4("view", programState->view);
        lightingShader.setMat4("model", model);

        // sofa
        shader.setMat4("view", programState->view);
        shader.setMat4("projection", projection);
        model = glm::mat4(1.0f);
        function.loadSofa(sofaModel, model, shader);

        // chairs
        shader.setMat4("view", programState->view);
        shader.setMat4("projection", projection);
        model = glm::mat4(1.0f);
        function.loadFirstChair(chairModel, model, shader);

        shader.setMat4("view", programState->view);
        shader.setMat4("projection", projection);
        model = glm::mat4(1.0f);
        function.loadSecondChair(chairModel, model, shader);

        shader.setMat4("view", programState->view);
        shader.setMat4("projection", projection);
        model = glm::mat4(1.0f);
        function.loadThirdChair(chairModel, model, shader);

        // table
        shader.setMat4("view", programState->view);
        shader.setMat4("projection", projection);
        model = glm::mat4(1.0f);
        function.loadTable(tableModel, model, shader);

        // stairs
        shader.setMat4("view", programState->view);
        shader.setMat4("projection", projection);
        model = glm::mat4(1.0f);
        function.loadStairs(stairsModel, model, shader);

        // desk
        shader.setMat4("view", programState->view);
        shader.setMat4("projection", projection);
        model = glm::mat4(1.0f);
        function.loadDesk(deskModel, model, shader);

        // tv
        shader.setMat4("view", programState->view);
        shader.setMat4("projection", projection);
        model = glm::mat4(1.0f);
        function.loadTv(tvModel, model, shader);

        // bed
        shader.setMat4("view", programState->view);
        shader.setMat4("projection", projection);
        model = glm::mat4(1.0f);
        function.loadBed(bedModel, model, shader);

        // locker
        shader.setMat4("view", programState->view);
        shader.setMat4("projection", projection);
        model = glm::mat4(1.0f);
        function.loadLocker(lockerModel, model, shader);

        // bedside_tables
        shader.setMat4("view", programState->view);
        shader.setMat4("projection", projection);
        model = glm::mat4(1.0f);
        function.loadFirstBedsideTable(bedsideTableModel, model, shader);

        shader.setMat4("view", programState->view);
        shader.setMat4("projection", projection);
        model = glm::mat4(1.0f);
        function.loadSecondBedsideTable(bedsideTableModel, model, shader);

        // elevator
        shader.setMat4("view", programState->view);
        shader.setMat4("projection", projection);
        model = glm::mat4(1.0f);
        function.loadElevator(elevatorModel, model, shader, programState->elevatorPosition, programState->speed * deltaTime, programState->start);

        // elevatorDoor
        glBindVertexArray(cubeVAO);
        glBindTexture(GL_TEXTURE_2D, glass);
        function.settingUpElevatorDoor(shader, model, programState->doorPosition, programState->open, programState->speed * deltaTime, programState->start);
        glBindVertexArray(0);

        // floor
        glBindVertexArray(floorVAO);
        function.settingUpFloor(shader, model, floor);
        glBindVertexArray(0);

        // wall
        glBindVertexArray(cubeVAO);
        function.settingUpWall(shader, model, tile, wall, 0.5f);
        function.settingUpPillar(shader, model, stone);
        function.settingUpWall(shader, model, tile, wall, 6.5f);
        glBindVertexArray(0);

        // tiles
        glBindVertexArray(cubeVAO);
        glBindTexture(GL_TEXTURE_2D, wood);
        function.settingUpTilesInPillar(shader, model);
        glBindTexture(GL_TEXTURE_2D, tile);
        function.settingUpTilesInWall(shader, model);
        glBindVertexArray(0);

        // roof
        glBindVertexArray(cubeVAO);
        glBindTexture(GL_TEXTURE_2D, tile);
        function.settingUpRoof(shader, model);
        glBindVertexArray(0);

        // light
        lightShader.use();
        lightShader.setMat4("projection", projection);
        lightShader.setMat4("view", programState->view);
        glBindVertexArray(lightVAO);
        function.settingUpLight(lightShader, model);
        glBindVertexArray(0);

        if (programState->ImGuiEnabled) {
            ElevatorImGui(programState);
        }

        // window
        shaderCubeMaps.use();
        programState->view = programState->camera.GetViewMatrix();
        projection = glm::perspective(glm::radians(programState->camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
        shaderCubeMaps.setMat4("view", programState->view);
        shaderCubeMaps.setMat4("projection", projection);
        shaderCubeMaps.setVec3("cameraPos", programState->camera.Position);

        glBindVertexArray(floorVAO);
        glActiveTexture(GL_TEXTURE0);
        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(0.0f, 6.0f, 0.0f));
        shaderCubeMaps.setMat4("model", model);
        glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapTexture);
        glDrawArrays(GL_TRIANGLES, 0, 36);
        glBindVertexArray(0);

        // skybox draw
        glDepthMask(GL_FALSE);
        glDepthFunc(GL_LEQUAL);
        skyboxShader.use();
        skyboxShader.setMat4("view", glm::mat4(glm::mat3(programState->view)));
        skyboxShader.setMat4("projection", projection);

        glBindVertexArray(skyboxVAO);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapTexture);
        glDrawArrays(GL_TRIANGLES, 0, 36);
        glBindVertexArray(0);
        glDepthMask(GL_TRUE);
        glDepthFunc(GL_LESS);

        // kernel effects
        if (programState->effect) {
            programState->kernel = 0.01f;
            screenShaderNext.use();
        }
        else if (!programState->effect) {
            programState->kernel = programState->kernel + 0.01f >= 1.0f ? 1.0f : programState->kernel + 0.01f;
            screenShader.use();
            screenShader.setFloat("kernel", programState->kernel);
        }

        // camera movement
        if (programState->start == 1) {
            programState->view = glm::translate(programState->view, glm::vec3(programState->camera.Position.x,
                                                                              programState->camera.Position.y = programState->camera.Position.y + ((float)glfwGetTime() * deltaTime * programState->speed) - (-9.81 / 2.0f) *  deltaTime * programState->speed *  deltaTime * programState->speed >= 8.32f ?
                                                                                                                8.32f : programState->camera.Position.y + ((float)glfwGetTime() * deltaTime * programState->speed) - (-9.81 / 2.0f) * deltaTime * programState->speed * deltaTime * programState->speed,
                                                                              programState->camera.Position.z));
        }
        else if (programState->start == 0) {
            programState->view = glm::translate(programState->view, glm::vec3(programState->camera.Position.x,
                                                                              programState->camera.Position.y = programState->camera.Position.y - ((float)glfwGetTime() * deltaTime * programState->speed) - (-9.81 / 2.0f) *  deltaTime * programState->speed *  deltaTime * programState->speed <= 2.0f ?
                                                                                                                2.0f : programState->camera.Position.y - ((float)glfwGetTime() * deltaTime * programState->speed) - (-9.81 / 2.0f) * deltaTime * programState->speed * deltaTime * programState->speed,
                                                                              programState->camera.Position.z));
        }
        // END DRAW SCENE

        // unbind framebuffer
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        // disable depth test
        glDisable(GL_DEPTH_TEST); 
        // clear all relevant buffers
        // glClearColor(1.0f, 1.0f, 1.0f, 1.0f); 
        glClear(GL_COLOR_BUFFER_BIT);

        // map on screen : bottom left -> top right
        glBindVertexArray(quadVAO);
        glBindTexture(GL_TEXTURE_2D, textureColorBuffer);
        glDrawArrays(GL_TRIANGLES, 0, 6);

        // glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
        // -------------------------------------------------------------------------------
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    programState->SaveToDisk("resources/programState.txt");

    // ImGui CleanUp
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    delete programState;

    // optional: de-allocate all resources once they've outlived their purpose:
    // ------------------------------------------------------------------------
    glDeleteVertexArrays(1, &cubeVAO);
    glDeleteVertexArrays(1, &floorVAO);
    glDeleteVertexArrays(1, &lightVAO);
    glDeleteVertexArrays(1, &skyboxVAO);
    glDeleteVertexArrays(1, &quadVAO);
    glDeleteBuffers(1, &cubeVBO);
    glDeleteBuffers(1, &floorVBO);
    glDeleteBuffers(1, &skyboxVBO);
    glDeleteBuffers(1, &quadVBO);

    glfwTerminate();

    return 0;
}

void ElevatorImGui(ProgramState* programState) {
    // ImGui Frame init
    static bool entry = false;
    static int op = -1;

    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
    {
        // static float f = 0.0f;
        ImGui::Begin("Lift");

        ImGui::Text("Opcije za lift");
        ImGui::Checkbox("Otvori", &programState->open);
//        ImGui::SliderFloat("float", &f, 0.0f, 1.0f);
//
//        if (ImGui::Button("Change effect")) {
//            programState->effect = !programState->effect;
//        }
//            counter++;
//        ImGui::SameLine();
//        ImGui::Text("counter = %d", counter);

        ImGui::Text("Application average: %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
        ImGui::Text("Camera position: %.3f, %.3f %.3f", programState->camera.Position.x, programState->camera.Position.y, programState->camera.Position.z);

        ImGui::End();
    }

    if (programState->open && programState->doorPosition.z == -2.0f) {
        ImGui::Begin("Lift");

        ImGui::Checkbox("Ulaz", &entry);

        ImGui::End();
    }

    if (entry) {
        programState->open = false;
        function.entryElevator(programState->camera, programState->camera.Position.y, programState->camera.Position.z);
        ImGui::Begin("Lift", &entry);
        if (programState->doorPosition.z == -3.28f) {
            ImGui::Text("\nSpratovi: ");
            if (ImGui::RadioButton("Prvi sprat", &op, 1)) {
                if (programState->camera.Position.y < 6.0) {
                    // programState->camera = Camera(glm::vec3(programState->camera.Position.x, 8.0f, programState->camera.Position.z));
                    programState->start = 1;
                } else {
                    op = -1;
                }
            }
            if (ImGui::RadioButton("Prizemlje", &op, 0)) {
                if (programState->camera.Position.y >= 6.0f) {
                    // programState->camera = Camera(glm::vec3(programState->camera.Position.x, 2.0f, programState->camera.Position.z));
                    programState->start = 0;
                } else {
                    op = -1;
                }
            }
            ImGui::Text("\nIzlaz: ");
            if (ImGui::Button("Izadji iz lifta!")) {
                programState->start = -1;
                programState->open = true;
                function.leaveElevator(programState->camera, programState->camera.Position.x + 4.0f,
                                           programState->camera.Position.y, programState->camera.Position.z);
                entry = false;
            }
        }


        ImGui::End();
    }

    // ImGui render
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
   if (key == GLFW_KEY_SPACE && action == GLFW_PRESS) {
       programState->effect = false;
   }

    bool retVal = function.validPosition(programState->camera.Position.x, programState->camera.Position.y, programState->camera.Position.z);
    if (retVal) {
        if (key == GLFW_KEY_TAB && action == GLFW_PRESS) {
            programState->ImGuiEnabled = !programState->ImGuiEnabled;
            if (programState->ImGuiEnabled) {
                glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
            } else {
                glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
            }
        }
    }
}

// process all input: query GLFW whether relevant keys are pressed/released this frame and react accordingly
// ---------------------------------------------------------------------------------------------------------
void processInput(GLFWwindow *window) {
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
        glfwSetWindowShouldClose(window, true);
    }
    if (!programState->ImGuiEnabled) {
        if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
            programState->camera.ProcessKeyboard(FORWARD, deltaTime);
        }
        if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
            programState->camera.ProcessKeyboard(BACKWARD, deltaTime);
        }
        if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
            programState->camera.ProcessKeyboard(LEFT, deltaTime);
        }
        if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
            programState->camera.ProcessKeyboard(RIGHT, deltaTime);
        }
    }
}

// glfw: whenever the window size changed (by OS or user resize) this callback function executes
// ---------------------------------------------------------------------------------------------
void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
    // make sure the viewport matches the new window dimensions; note that width and
    // height will be significantly larger than specified on retina displays.
    glViewport(0, 0, width, height);
}

// glfw: whenever the mouse moves, this callback is called
// -------------------------------------------------------
void mouse_callback(GLFWwindow* window, double xpos, double ypos) {
    if (firstMouse) {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos; // reversed since y-coordinates go from bottom to top

    lastX = xpos;
    lastY = ypos;

    if (!programState->ImGuiEnabled) {
        programState->camera.ProcessMouseMovement(xoffset, yoffset);
    }
}

// glfw: whenever the mouse scroll wheel scrolls, this callback is called
// ----------------------------------------------------------------------
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset) {
    programState->camera.ProcessMouseScroll(yoffset);
    programState->effect = true;
}

unsigned int loadCubemap(vector<std::string> &faces) {
    unsigned int textureID;
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);

    int width, height, nrChannels, n = faces.size();
    for (unsigned int i = 0; i < n; ++i) {
        unsigned char *data = stbi_load(faces[i].c_str(), &width, &height, &nrChannels, 0);
        if (data) {
            glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB, width, height,
                         0, GL_RGB, GL_UNSIGNED_BYTE, data);
        }
        else {
            std::cerr << "Cube map texture face - load failure!\n";

            return -1;
        }
        stbi_image_free(data);
    }

    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

    return textureID;
}

// utility function for loading a 2D texture from file
// ---------------------------------------------------
unsigned int loadTexture(char const *path) {
    unsigned int textureID;
    glGenTextures(1, &textureID);

    int width, height, nrComponents;
    unsigned char *data = stbi_load(path, &width, &height, &nrComponents, 0);
    if (data) {
        GLenum format;
        if (nrComponents == 1) {
            format = GL_RED;
        } else if (nrComponents == 3) {
            format = GL_RGB;
        } else if (nrComponents == 4) {
            format = GL_RGBA;
        }

        glBindTexture(GL_TEXTURE_2D, textureID);
        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        stbi_image_free(data);
    }
    else {
        std::cout << "Texture failed to load at path: " << path << std::endl;
        stbi_image_free(data);
    }

    return textureID;
}
