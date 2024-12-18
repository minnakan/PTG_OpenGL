#include <glad/glad.h>
#include <GLFW/glfw3.h>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image/stb_image.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/noise.hpp>

#include "BruteForceTerrain.h"
#include "TerrainManager.h"

#include <Shader.h>
#include <Camera.h>

#include <iostream>
#include <vector>
#include <cmath>



using namespace std;

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void processInput(GLFWwindow* window);


// settings
const unsigned int SCR_WIDTH = 1920;
const unsigned int SCR_HEIGHT = 1080;

// camera
Camera camera(glm::vec3(0.0f, 0.0f, 3.0f));
float lastX = SCR_WIDTH / 2.0f;
float lastY = SCR_HEIGHT / 2.0f;
bool firstMouse = true;

// timing
float deltaTime = 0.0f;	// time between current frame and last frame
float lastFrame = 0.0f;




int main()
{
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
    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "Terrains", NULL, NULL);
    if (window == NULL)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetScrollCallback(window, scroll_callback);

    // tell GLFW to capture our mouse
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    // glad: load all OpenGL function pointers
    // ---------------------------------------
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    // configure global opengl state
    // -----------------------------
    glEnable(GL_DEPTH_TEST);

    // build and compile our shader zprogram
    // ------------------------------------
    
    Shader terrainShader(RESOURCES_PATH"TerrainVertex.vert", RESOURCES_PATH"TerrainFrag.frag");

    TerrainManager terrainManager;
    terrainManager.Initialize(&terrainShader,RESOURCES_PATH"TextureTiles/",RESOURCES_PATH"DetailMap1.jpg");

    terrainManager.GenerateChunks(32.0f, 256,64.f);
    terrainManager.SetLightingProperties(1.0f, 0.0f, 0.1f, 1.0f, 10.0f);


    BruteForceTerrain terrain;
    terrain.shaderProgramme = &terrainShader;
    //terrain.LoadTexture(RESOURCES_PATH"Grass1/aerial_grass_rock_diff_4k.jpg");
    terrain.LoadtextureTiles(RESOURCES_PATH"TextureTiles/");
    terrain.LoadDetailMap(RESOURCES_PATH"DetailMap1.jpg");
    //terrain.LoadLightMap(RESOURCES_PATH"Lightmaps/lightmapTest.jpg", 257);
    terrain.repeatDetailMap = 1.f;
    terrain.GenerateTerrainFractal_FF(0, 255, 32, 256,true, 0.1f,0.1f);
    // Random seed based on time
    //srand(static_cast<unsigned int>(time(0)));
    //terrain.GenerateTerrainFractal_MPD(256.0f, 1.0f, 256, 0.5f);
    //terrain.GenerateTerrainContinuous(32.0f, 256, 0, 0);

    terrain.SetLightingType(SLOPE_LIGHT);
    
    terrain.CustomizeSlopeLighting(1, 0, 0.1f, 1.0f, 10.0f);
    terrain.CalculateLighting();

    BruteForceTerrain terrain2;
    terrain2.shaderProgramme = &terrainShader;
    terrain2.LoadtextureTiles(RESOURCES_PATH"TextureTiles/");
    terrain2.LoadDetailMap(RESOURCES_PATH"DetailMap1.jpg");
    terrain2.repeatDetailMap = 1.f;
    terrain2.StitchTerrainMPD(terrain.GetVertices(),
        terrain.GetIndices(),
        terrain.GetSize(),
        0,
        0.5f,256,1, terrain.GetXOffset(), terrain.GetZOffset());
    terrain2.SetLightingType(SLOPE_LIGHT);
    //terrain2.GenerateTerrainContinuous(32.0f, 256, 1, 0);

    terrain2.CustomizeSlopeLighting(1, 0, 0.1f, 1.0f, 10.0f);
    terrain2.CalculateLighting();

    /*BruteForceTerrain terrain3;
    terrain3.shaderProgramme = &terrainShader;
    terrain3.LoadtextureTiles(RESOURCES_PATH"TextureTiles/");
    terrain3.LoadDetailMap(RESOURCES_PATH"DetailMap1.jpg");
    terrain3.repeatDetailMap = 1.f;
    terrain3.StitchTerrainMPD(terrain2.GetVertices(),
        terrain2.GetIndices(),
        terrain2.GetSize(),
        0,
        0.5f, 256, 1, terrain2.GetXOffset(), terrain2.GetZOffset());
    terrain3.SetLightingType(SLOPE_LIGHT);

    terrain3.CustomizeSlopeLighting(1, 0, 0.1f, 1.0f, 10.0f);
    terrain3.CalculateLighting();*/


    
    //terrain.SetLightingType(HEIGHT_BASED); // Switch to height-based lighting
    //terrain.CalculateLighting();

    // render loop
    // -----------
    while (!glfwWindowShouldClose(window))
    {
        // per-frame time logic
        // --------------------
        float currentFrame = static_cast<float>(glfwGetTime());
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        // input
        // -----
        processInput(window);

        // render
        // ------
        glClearColor(0.5f, 0.8f, 0.92f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glm::mat4 view = camera.GetViewMatrix();
        glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 1000.0f);
        glm::mat4 model = glm::mat4(1.0f);

        terrainShader.use();

        //Wireframe
        //glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

        glUniformMatrix4fv(glGetUniformLocation(terrainShader.ID, "model"), 1, GL_FALSE, glm::value_ptr(model));
        glUniformMatrix4fv(glGetUniformLocation(terrainShader.ID, "view"), 1, GL_FALSE, glm::value_ptr(view));
        glUniformMatrix4fv(glGetUniformLocation(terrainShader.ID, "projection"), 1, GL_FALSE, glm::value_ptr(projection));

        
        terrainManager.Render();


        //terrain.Render();
        //terrain2.Render();
        //terrain3.Render();
   

        // glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
        // -------------------------------------------------------------------------------
        glfwSwapBuffers(window);
        glfwPollEvents();
    }
    // glfw: terminate, clearing all previously allocated GLFW resources.
    // ------------------------------------------------------------------
    glfwTerminate();
    return 0;
}

// process all input: query GLFW whether relevant keys are pressed/released this frame and react accordingly
// ---------------------------------------------------------------------------------------------------------
void processInput(GLFWwindow* window)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        camera.ProcessKeyboard(FORWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        camera.ProcessKeyboard(BACKWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        camera.ProcessKeyboard(LEFT, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        camera.ProcessKeyboard(RIGHT, deltaTime);

    
}

// glfw: whenever the window size changed (by OS or user resize) this callback function executes
// ---------------------------------------------------------------------------------------------
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    // make sure the viewport matches the new window dimensions; note that width and 
    // height will be significantly larger than specified on retina displays.
    glViewport(0, 0, width, height);
}


// glfw: whenever the mouse moves, this callback is called
// -------------------------------------------------------
void mouse_callback(GLFWwindow* window, double xposIn, double yposIn)
{
    float xpos = static_cast<float>(xposIn);
    float ypos = static_cast<float>(yposIn);

    if (firstMouse)
    {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos; // reversed since y-coordinates go from bottom to top

    lastX = xpos;
    lastY = ypos;

    camera.ProcessMouseMovement(xoffset, yoffset);
}

// glfw: whenever the mouse scroll wheel scrolls, this callback is called
// ----------------------------------------------------------------------
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
    camera.ProcessMouseScroll(static_cast<float>(yoffset));
}