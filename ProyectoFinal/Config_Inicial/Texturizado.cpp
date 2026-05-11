#include <iostream>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include "stb_image.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "Shader.h"
#include "Model.h"
#include <string>

void Inputs(GLFWwindow* window);

GLuint loadTexture(const char* path) {
    GLuint texID;
    glGenTextures(1, &texID);
    glBindTexture(GL_TEXTURE_2D, texID);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST_MIPMAP_NEAREST);

    int w, h, ch;
    stbi_set_flip_vertically_on_load(true);
    unsigned char* image = stbi_load(path, &w, &h, &ch, 4);

    if (image) {
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, image);
        glGenerateMipmap(GL_TEXTURE_2D);
        stbi_image_free(image);
    }
    else {
        std::cout << "ERROR cargando: " << path << std::endl;
        unsigned char fallback[] = { 255, 0, 255, 255 };
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, fallback);
    }
    return texID;
}

const GLint WIDTH = 1200, HEIGHT = 800;
float movX = 0.0f, movY = 0.0f, movZ = -5.0f, rot = 0.0f;
bool lucesOn = true;
bool lTeclaAnterior = false;
bool modoNoche = false;
bool nTeclaAnterior = false;
float esculturaAngulo = 0.0f;
float penduloTiempo = 0.0f;

// Para la máquina de estados: carro 1
enum EstadoCarro { ESPERANDO, AVANZANDO, GIRANDO, RETROCEDIENDO, ESTACIONADO };
EstadoCarro estadoCarro = ESPERANDO;

float carroPosX = 15.0f;
float carroPosZ = 20.0f;
float carroRot = 90.0f;
float carroVel = 0.0f;


// Máquina de estados: carro2
enum EstadoCarro2 { ESPERANDO2, AVANZANDO2, GIRANDO2, AVANZANDO2B, ESTACIONADO2 };
EstadoCarro2 estadoCarro2 = ESPERANDO2;

float carro2PosX = -38.0f;   // posición inicial
float carro2PosZ = 25.0f;
float carro2Rot = 90.0;

// Tecla para iniciar animaciones carros
bool oTeclaAnterior = false;
bool pTeclaAnterior = false;

// Máquina de estados: perro
enum EstadoPerro { PERRO_QUIETO, PERRO_ANIMANDO };
EstadoPerro estadoPerro = PERRO_QUIETO;

bool iTeclaAnterior = false;

float perroPosX = 0.0f;
float perroPosY = -1.0f;
float perroPosZ = 0.0f;
float perroRot = 0.0f;

float perroColaRot = 0.0f;
float perroCabRot = 0.0f;
float perroPataRot = 0.0f;

int  perroFase = 0;
bool perroStep = false;
int  perroTicks = 0;

// Máquina de estados: etiqueta
enum EstadoEtiqueta { ETIQUETA_QUIETA, ETIQUETA_TAMBALEANDO, ETIQUETA_CAYENDO, ETIQUETA_EN_SUELO, ETIQUETA_VOLVIENDO };
EstadoEtiqueta estadoEtiqueta = ETIQUETA_QUIETA;

bool tTeclaAnterior = false;

float etiquetaRotX = 0.0f;
float etiquetaRotZ = 0.0f;
float etiquetaOffY = 0.0f;

int  etiquetaTicks = 0;
bool etiquetaStep = false;

// Animacion 
void AnimacionCarro() {
    switch (estadoCarro) {
    case ESPERANDO:
        break;
    case AVANZANDO:
        carroPosX -= 0.30f;
        if (carroPosX <= 8.0f)
            estadoCarro = GIRANDO;
        break;
    case GIRANDO:
        carroRot -= 0.9f;
        if (carroRot <= 0.0f) {
            carroRot = 0.0f;
            estadoCarro = RETROCEDIENDO;
        }
        break;
    case RETROCEDIENDO:
        carroPosZ -= 0.4f;
        if (carroPosZ <= -2.0f) {
            carroPosZ = -2.0f;
            estadoCarro = ESTACIONADO;
        }
        break;
    case ESTACIONADO:
        break;
    }
}

void AnimacionCarro2() {
    switch (estadoCarro2) {

    case ESPERANDO2:
        break;

    case AVANZANDO2:
        carro2PosX += 0.40f;
        if (carro2PosX >= -45.0f) {
            estadoCarro2 = GIRANDO2;
        }
        break;

    case GIRANDO2:

        carro2Rot -= 1.1f;
        if (carro2Rot <= 30.0f) {
            carro2Rot = 30.0f;
            estadoCarro2 = AVANZANDO2B;
        }
        break;

    case AVANZANDO2B:

        carro2PosZ -= 0.8f;
        if (carro2PosZ <= -1.0f) {
            carro2PosZ = -1.0f;
            estadoCarro2 = ESTACIONADO2;
        }
        break;

    case ESTACIONADO2:
        break;
    }
}

void AnimacionPerro() {
    switch (estadoPerro) {

    case PERRO_QUIETO:
        break;

    case PERRO_ANIMANDO:
        // Fase inicial cola y cabeza
        if (perroFase == 0) {
            if (!perroStep) {
                perroColaRot += 1.5f;
                perroCabRot += 0.8f;
                if (perroColaRot >= 20.0f) { perroStep = true;  perroTicks++; }
            }
            else {
                perroColaRot -= 1.5f;
                perroCabRot -= 0.8f;
                if (perroColaRot <= -20.0f) { perroStep = false; perroTicks++; }
            }
            if (perroTicks >= 6) {
                perroColaRot = 0.0f;
                perroCabRot = 0.0f;
                perroStep = false;
                perroFase = 1;
            }
        }
        // Fase 1
        else if (perroFase == 1) {
            perroPataRot += 1.5f;
            if (perroPataRot >= 60.0f) { perroPataRot = 60.0f; perroFase = 2; }
        }
        // Fase 2
        else if (perroFase == 2) {
            perroPataRot -= 1.5f;
            if (perroPataRot <= 0.0f) { perroPataRot = 0.0f; estadoPerro = PERRO_QUIETO; }
        }
        break;
    }
}

void AnimacionEtiqueta() {
    switch (estadoEtiqueta) {

    case ETIQUETA_QUIETA:
        break;

    case ETIQUETA_TAMBALEANDO:
        if (!etiquetaStep) {
            etiquetaRotZ += 0.8f;
            if (etiquetaRotZ >= 12.0f) { etiquetaStep = true;  etiquetaTicks++; }
        }
        else {
            etiquetaRotZ -= 0.8f;
            if (etiquetaRotZ <= -12.0f) { etiquetaStep = false; etiquetaTicks++; }
        }
        if (etiquetaTicks >= 4) {
            etiquetaRotZ = 0.0f;
            etiquetaStep = false;
            estadoEtiqueta = ETIQUETA_CAYENDO;
        }
        break;

    case ETIQUETA_CAYENDO:
        etiquetaRotX += 1.0f;
        etiquetaOffY -= 0.012f;
        if (etiquetaRotX >= 90.0f) {
            etiquetaRotX = 90.0f;
            estadoEtiqueta = ETIQUETA_EN_SUELO;
        }
        break;

    case ETIQUETA_EN_SUELO:
        break;

    case ETIQUETA_VOLVIENDO:
        if (etiquetaRotX > 0.0f) etiquetaRotX -= 1.2f;
        if (etiquetaOffY < 0.0f) etiquetaOffY += 0.008f;
        if (etiquetaRotX <= 0.0f && etiquetaOffY >= 0.0f) {
            etiquetaRotX = 0.0f;
            etiquetaOffY = 0.0f;
            etiquetaTicks = 0;
            estadoEtiqueta = ETIQUETA_QUIETA;
        }
        break;
    }
}

int main() {
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);

    GLFWwindow* window = glfwCreateWindow(WIDTH, HEIGHT, "Museo con Texturas", nullptr, nullptr);
    if (!window) {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return EXIT_FAILURE;
    }
    glfwMakeContextCurrent(window);

    int screenWidth, screenHeight;
    glfwGetFramebufferSize(window, &screenWidth, &screenHeight);

    glewExperimental = GL_TRUE;
    if (GLEW_OK != glewInit()) {
        std::cout << "Failed to initialise GLEW" << std::endl;
        return EXIT_FAILURE;
    }

    glViewport(0, 0, screenWidth, screenHeight);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    Shader ourShader("Shader/modelLoading.vs", "Shader/modelLoading.frag");
    Shader lampshader("Shader/lamp.vs", "Shader/lamp.frag");
    Shader lightingShader("Shader/lighting.vs", "Shader/lighting.frag");

    Model estructura((char*)"Models/estructura5_1.obj");
    Model carro((char*)"Models/carro.obj");
    Model carro2((char*)"Models/carro2.obj");
    Model perro((char*)"Models/cuerpo.obj");
    Model pata((char*)"Models/pata.obj");
    Model cola((char*)"Models/cola.obj");
    Model cabeza((char*)"Models/cabeza.obj");
    Model baseP((char*)"Models/base.obj");

    float vertices[] = {
        -0.5f,-0.5f, 0.5f,   0,0,1,   0.0f,0.0f,
         0.5f,-0.5f, 0.5f,   0,0,1,   1.0f,0.0f,
         0.5f, 0.5f, 0.5f,   0,0,1,   1.0f,1.0f,
         0.5f, 0.5f, 0.5f,   0,0,1,   1.0f,1.0f,
        -0.5f, 0.5f, 0.5f,   0,0,1,   0.0f,1.0f,
        -0.5f,-0.5f, 0.5f,   0,0,1,   0.0f,0.0f,

        -0.5f,-0.5f,-0.5f,   0,0,-1,  0.0f,0.0f,
         0.5f,-0.5f,-0.5f,   0,0,-1,  1.0f,0.0f,
         0.5f, 0.5f,-0.5f,   0,0,-1,  1.0f,1.0f,
         0.5f, 0.5f,-0.5f,   0,0,-1,  1.0f,1.0f,
        -0.5f, 0.5f,-0.5f,   0,0,-1,  0.0f,1.0f,
        -0.5f,-0.5f,-0.5f,   0,0,-1,  0.0f,0.0f,

         0.5f,-0.5f, 0.5f,   1,0,0,   0.0f,0.0f,
         0.5f,-0.5f,-0.5f,   1,0,0,   1.0f,0.0f,
         0.5f, 0.5f,-0.5f,   1,0,0,   1.0f,1.0f,
         0.5f, 0.5f,-0.5f,   1,0,0,   1.0f,1.0f,
         0.5f, 0.5f, 0.5f,   1,0,0,   0.0f,1.0f,
         0.5f,-0.5f, 0.5f,   1,0,0,   0.0f,0.0f,

        -0.5f, 0.5f, 0.5f,  -1,0,0,   0.0f,0.0f,
        -0.5f, 0.5f,-0.5f,  -1,0,0,   1.0f,0.0f,
        -0.5f,-0.5f,-0.5f,  -1,0,0,   1.0f,1.0f,
        -0.5f,-0.5f,-0.5f,  -1,0,0,   1.0f,1.0f,
        -0.5f,-0.5f, 0.5f,  -1,0,0,   0.0f,1.0f,
        -0.5f, 0.5f, 0.5f,  -1,0,0,   0.0f,0.0f,

        -0.5f,-0.5f,-0.5f,   0,-1,0,  0.0f,0.0f,
         0.5f,-0.5f,-0.5f,   0,-1,0,  1.0f,0.0f,
         0.5f,-0.5f, 0.5f,   0,-1,0,  1.0f,1.0f,
         0.5f,-0.5f, 0.5f,   0,-1,0,  1.0f,1.0f,
        -0.5f,-0.5f, 0.5f,   0,-1,0,  0.0f,1.0f,
        -0.5f,-0.5f,-0.5f,   0,-1,0,  0.0f,0.0f,

        -0.5f, 0.5f,-0.5f,   0,1,0,   0.0f,0.0f,
         0.5f, 0.5f,-0.5f,   0,1,0,   1.0f,0.0f,
         0.5f, 0.5f, 0.5f,   0,1,0,   1.0f,1.0f,
         0.5f, 0.5f, 0.5f,   0,1,0,   1.0f,1.0f,
        -0.5f, 0.5f, 0.5f,   0,1,0,   0.0f,1.0f,
        -0.5f, 0.5f,-0.5f,   0,1,0,   0.0f,0.0f,
    };

    GLuint VBO, VAO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (GLvoid*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (GLvoid*)(3 * sizeof(GLfloat)));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (GLvoid*)(6 * sizeof(GLfloat)));
    glEnableVertexAttribArray(2);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    float lampVertices[] = {
        -0.5f,-0.5f,-0.5f,  0,0,-1,  0,0,
         0.5f,-0.5f,-0.5f,  0,0,-1,  1,0,
         0.5f, 0.5f,-0.5f,  0,0,-1,  1,1,
         0.5f, 0.5f,-0.5f,  0,0,-1,  1,1,
        -0.5f, 0.5f,-0.5f,  0,0,-1,  0,1,
        -0.5f,-0.5f,-0.5f,  0,0,-1,  0,0,

        -0.5f,-0.5f, 0.5f,  0,0,1,   0,0,
         0.5f,-0.5f, 0.5f,  0,0,1,   1,0,
         0.5f, 0.5f, 0.5f,  0,0,1,   1,1,
         0.5f, 0.5f, 0.5f,  0,0,1,   1,1,
        -0.5f, 0.5f, 0.5f,  0,0,1,   0,1,
        -0.5f,-0.5f, 0.5f,  0,0,1,   0,0,

        -0.5f, 0.5f, 0.5f, -1,0,0,   1,0,
        -0.5f, 0.5f,-0.5f, -1,0,0,   1,1,
        -0.5f,-0.5f,-0.5f, -1,0,0,   0,1,
        -0.5f,-0.5f,-0.5f, -1,0,0,   0,1,
        -0.5f,-0.5f, 0.5f, -1,0,0,   0,0,
        -0.5f, 0.5f, 0.5f, -1,0,0,   1,0,

         0.5f, 0.5f, 0.5f,  1,0,0,   1,0,
         0.5f, 0.5f,-0.5f,  1,0,0,   1,1,
         0.5f,-0.5f,-0.5f,  1,0,0,   0,1,
         0.5f,-0.5f,-0.5f,  1,0,0,   0,1,
         0.5f,-0.5f, 0.5f,  1,0,0,   0,0,
         0.5f, 0.5f, 0.5f,  1,0,0,   1,0,

        -0.5f,-0.5f,-0.5f,  0,-1,0,  0,1,
         0.5f,-0.5f,-0.5f,  0,-1,0,  1,1,
         0.5f,-0.5f, 0.5f,  0,-1,0,  1,0,
         0.5f,-0.5f, 0.5f,  0,-1,0,  1,0,
        -0.5f,-0.5f, 0.5f,  0,-1,0,  0,0,
        -0.5f,-0.5f,-0.5f,  0,-1,0,  0,1,

        -0.5f, 0.5f,-0.5f,  0,1,0,   0,1,
         0.5f, 0.5f,-0.5f,  0,1,0,   1,1,
         0.5f, 0.5f, 0.5f,  0,1,0,   1,0,
         0.5f, 0.5f, 0.5f,  0,1,0,   1,0,
        -0.5f, 0.5f, 0.5f,  0,1,0,   0,0,
        -0.5f, 0.5f,-0.5f,  0,1,0,   0,1
    };

    GLuint lampVAO, lampVBO;
    glGenVertexArrays(1, &lampVAO);
    glGenBuffers(1, &lampVBO);
    glBindVertexArray(lampVAO);
    glBindBuffer(GL_ARRAY_BUFFER, lampVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(lampVertices), lampVertices, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (GLvoid*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (GLvoid*)(3 * sizeof(GLfloat)));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (GLvoid*)(6 * sizeof(GLfloat)));
    glEnableVertexAttribArray(2);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    // Texturas
    GLuint texConcreto = loadTexture("images/concret.jpg");
    GLuint texConcreto2 = loadTexture("images/concret2.jpg");
    GLuint texMadera = loadTexture("images/madera.jpg");
    GLuint texMetal = loadTexture("images/metal.jpg");
    GLuint texRojo = loadTexture("images/rojo.jpg");
    GLuint texAmarillo = loadTexture("images/amarillo.jpg");
    GLuint texVidRojo = loadTexture("images/vidrio_rojo.jpg");
    GLuint texVidAmarillo = loadTexture("images/vidrio_amar.jpg");
    GLuint texVidRosa = loadTexture("images/vidrio_rosa.jpg");
    GLuint texVidAzul = loadTexture("images/vidrio_az.jpg");
    GLuint texAzul = loadTexture("images/azul.jpg");
    GLuint texAzul2 = loadTexture("images/azul2.jpg");
    GLuint texAzul3 = loadTexture("images/azul3.jpg");
    GLuint texVerde = loadTexture("images/verde.jpg");
    GLuint texPiedra = loadTexture("images/piedra.jpg");
    GLuint texPapel = loadTexture("images/papel.jpg");
    GLuint texNegro = loadTexture("images/negro.jpg");
    GLuint texGris = loadTexture("images/gris.jpg");
    GLuint texTerracota = loadTexture("images/terracota.jpg");
    GLuint texTierra = loadTexture("images/tierra.jpg");
    GLuint texHoja = loadTexture("images/planta.jpg");
    GLuint texArbusto = loadTexture("images/arbusto.jpg");
    GLuint texMAzul = loadTexture("images/MAzul.jpg");  // textura del perro

    glm::mat4 projection = glm::perspective(
        glm::radians(45.0f),
        (GLfloat)screenWidth / (GLfloat)screenHeight,
        0.1f, 100.0f);

    glm::vec3 lampPositions[] = {
        glm::vec3(3.15f,  4.15f,  7.3f),
        glm::vec3(13.3f,  4.15f,  7.3f),
        glm::vec3(-6.45f, 4.15f,  7.3f),
        glm::vec3(-16.45f,4.15f,  7.3f),
        glm::vec3(-24.5f, 4.15f,  6.3f),
        glm::vec3(17.0f,  4.15f,  1.7f),
        glm::vec3(16.8f,  4.15f, -8.6f),
        glm::vec3(16.8f,  4.15f,-33.18f),
        glm::vec3(3.4f,   4.15f,-36.8f),
        glm::vec3(24.8f,  4.15f,  9.4f),
        glm::vec3(30.9f,  4.15f,  7.0f),
        glm::vec3(-2.0f,  4.65f,  1.8f),
        glm::vec3(4.0f,   4.65f,  1.8f)
    };
    const int numLamps = sizeof(lampPositions) / sizeof(lampPositions[0]);

#define DRAW(m) \
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(m)); \
    glDrawArrays(GL_TRIANGLES, 0, 36);

#define USE_TEX(t) \
    glActiveTexture(GL_TEXTURE0); \
    glBindTexture(GL_TEXTURE_2D, t);

    
    while (!glfwWindowShouldClose(window)) {
        Inputs(window);
        glfwPollEvents();
        AnimacionCarro();
        AnimacionCarro2();
        AnimacionPerro();
        AnimacionEtiqueta();

        glClearColor(0.08f, 0.08f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glm::mat4 view = glm::mat4(1.0f);
        view = glm::translate(view, glm::vec3(movX, movY, movZ));
        view = glm::rotate(view, glm::radians(rot), glm::vec3(0.0f, 1.0f, 0.0f));

        glm::mat4 escenaBase = glm::mat4(1.0f);
        escenaBase = glm::translate(escenaBase, glm::vec3(movX, movY, movZ));
        escenaBase = glm::rotate(escenaBase, glm::radians(rot), glm::vec3(0.0f, 1.0f, 0.0f));

        lightingShader.Use();

        GLint modelLoc = glGetUniformLocation(lightingShader.Program, "model");
        GLint viewLoc = glGetUniformLocation(lightingShader.Program, "view");
        GLint projecLoc = glGetUniformLocation(lightingShader.Program, "projection");
        GLint texLoc = glGetUniformLocation(lightingShader.Program, "texture_diffuse");

        glUniformMatrix4fv(projecLoc, 1, GL_FALSE, glm::value_ptr(projection));
        glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
        glUniform1i(texLoc, 0);

        glUniform3f(glGetUniformLocation(lightingShader.Program, "viewPos"),
            -movX, -movY, -movZ);

        glUniform3f(glGetUniformLocation(lightingShader.Program, "material.ambient"), 2.3f, 2.3f, 2.3f);
        glUniform3f(glGetUniformLocation(lightingShader.Program, "material.diffuse"), 1.8f, 1.8f, 1.8f);
        glUniform3f(glGetUniformLocation(lightingShader.Program, "material.specular"), 1.4f, 1.4f, 1.4f);
        glUniform1f(glGetUniformLocation(lightingShader.Program, "material.shininess"), 2.0f);

        float intensidad = lucesOn ? 1.0f : 0.0f;
        glUniform1i(glGetUniformLocation(lightingShader.Program, "numLights"), numLamps);

        if (!modoNoche) {
            glUniform1i(glGetUniformLocation(lightingShader.Program, "numLights"), 1);
            glUniform3f(glGetUniformLocation(lightingShader.Program, "lights[0].position"), 0.0f, 10.0f, 0.0f);
            glUniform3f(glGetUniformLocation(lightingShader.Program, "lights[0].ambient"), 3.0f, 3.0f, 3.0f);
            glUniform3f(glGetUniformLocation(lightingShader.Program, "lights[0].diffuse"), 1.0f, 1.0f, 1.0f);
            glUniform3f(glGetUniformLocation(lightingShader.Program, "lights[0].specular"), 1.5f, 1.5f, 1.5f);
        }
        else {
            glUniform1i(glGetUniformLocation(lightingShader.Program, "numLights"), numLamps);
            intensidad = lucesOn ? 1.0f : 0.0f;
            for (int i = 0; i < numLamps; i++) {
                glm::vec3 lp = glm::vec3(escenaBase * glm::vec4(lampPositions[i], 1.0f));
                std::string b = "lights[" + std::to_string(i) + "]";
                glUniform3f(glGetUniformLocation(lightingShader.Program, (b + ".position").c_str()), lp.x, lp.y, lp.z);
                glUniform3f(glGetUniformLocation(lightingShader.Program, (b + ".ambient").c_str()),
                    lucesOn ? 0.15f : 0.02f, lucesOn ? 0.12f : 0.02f, lucesOn ? 0.08f : 0.02f);
                glUniform3f(glGetUniformLocation(lightingShader.Program, (b + ".diffuse").c_str()),
                    lucesOn ? 1.0f : 0.0f, lucesOn ? 0.85f : 0.0f, lucesOn ? 0.55f : 0.0f);
                glUniform3f(glGetUniformLocation(lightingShader.Program, (b + ".specular").c_str()),
                    lucesOn ? 0.8f : 0.0f, lucesOn ? 0.7f : 0.0f, lucesOn ? 0.5f : 0.0f);
            }
        }

        glBindVertexArray(VAO);
        glm::mat4 base, part;

        // Escultura 1 - arco con péndulo 
        penduloTiempo += 0.03f;
        float penduloAngulo = 25.0f * sin(penduloTiempo);

        USE_TEX(texRojo)
            base = glm::mat4(1.0f);
        base = glm::translate(base, glm::vec3(movX, movY, movZ));
        base = glm::rotate(base, glm::radians(rot), glm::vec3(0, 1, 0));
        base = glm::translate(base, glm::vec3(-18.55f, -0.89f, -20.0f));

        // Partes fijas
        part = glm::translate(base, glm::vec3(-0.8f, 1.2f, 0));
        part = glm::scale(part, glm::vec3(0.3f, 2.4f, 0.3f));  DRAW(part)
            part = glm::translate(base, glm::vec3(0.8f, 1.2f, 0));
        part = glm::scale(part, glm::vec3(0.3f, 2.4f, 0.3f));  DRAW(part)
            part = glm::translate(base, glm::vec3(0, 2.55f, 0));
        part = glm::scale(part, glm::vec3(1.9f, 0.3f, 0.3f));  DRAW(part)

            USE_TEX(texPiedra)
            part = glm::translate(base, glm::vec3(-0.8f, 0.07f, 0));
        part = glm::scale(part, glm::vec3(0.55f, 0.14f, 0.55f));  DRAW(part)
            part = glm::translate(base, glm::vec3(0.8f, 0.07f, 0));
        part = glm::scale(part, glm::vec3(0.55f, 0.14f, 0.55f));  DRAW(part)

            // Péndulo
            glm::mat4 pivote = glm::translate(base, glm::vec3(0.0f, 2.55f, 0.0f));
        pivote = glm::rotate(pivote, glm::radians(penduloAngulo), glm::vec3(1, 0, 0));
        pivote = glm::translate(pivote, glm::vec3(0.0f, -2.55f, 0.0f));

        USE_TEX(texRojo)
            part = glm::translate(pivote, glm::vec3(-0.4f, 1.5f, 0));
        part = glm::rotate(part, glm::radians(60.0f), glm::vec3(0, 1, 1));
        part = glm::scale(part, glm::vec3(0.08f, 0.9f, 0.08f));  DRAW(part)
            part = glm::translate(pivote, glm::vec3(0.4f, 1.5f, 0));
        part = glm::rotate(part, glm::radians(-60.0f), glm::vec3(0, 1, 1));
        part = glm::scale(part, glm::vec3(0.08f, 0.9f, 0.08f));  DRAW(part)

            USE_TEX(texAmarillo)
            part = glm::translate(pivote, glm::vec3(0, 1.5f, 0));
        part = glm::rotate(part, glm::radians(45.0f), glm::vec3(1, 0, 0));
        part = glm::scale(part, glm::vec3(0.55f, 0.55f, 0.55f));  DRAW(part)

            // Escultura 2 - planos piedra giratorios 
            esculturaAngulo += 0.3f;
        if (esculturaAngulo >= 360.0f) esculturaAngulo = 0.0f;

        base = glm::mat4(1.0f);
        base = glm::translate(base, glm::vec3(movX, movY, movZ));
        base = glm::rotate(base, glm::radians(rot), glm::vec3(0.0f, 1.0f, 0.0f));
        base = glm::translate(base, glm::vec3(3.5f, 1.5f, -22.0f));

        USE_TEX(texPiedra)
            part = glm::translate(base, glm::vec3(0.0f, 0.2f, 0.0f));
        part = glm::scale(part, glm::vec3(0.9f, 0.2f, 0.9f));  DRAW(part)

            USE_TEX(texConcreto)
            part = glm::translate(base, glm::vec3(0.0f, -1.0f, 0.0f));
        part = glm::scale(part, glm::vec3(2.0f, 2.5f, 1.2f));  DRAW(part)

            // Piezas giratorias
            glm::mat4 baseGiro = glm::rotate(base, glm::radians(esculturaAngulo), glm::vec3(0.0f, 1.0f, 0.0f));

        USE_TEX(texConcreto)
            part = glm::translate(baseGiro, glm::vec3(0.0f, 0.5f, 0.0f));
        part = glm::scale(part, glm::vec3(1.2f, 0.08f, 0.5f));  DRAW(part)

            USE_TEX(texConcreto2)
            part = glm::translate(baseGiro, glm::vec3(0.0f, 0.7f, 0.0f));
        part = glm::rotate(part, glm::radians(40.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        part = glm::scale(part, glm::vec3(1.1f, 0.08f, 0.45f));  DRAW(part)

            USE_TEX(texConcreto)
            part = glm::translate(baseGiro, glm::vec3(0.0f, 0.9f, 0.0f));
        part = glm::rotate(part, glm::radians(80.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        part = glm::scale(part, glm::vec3(1.0f, 0.08f, 0.4f));  DRAW(part)

            USE_TEX(texConcreto2)
            part = glm::translate(baseGiro, glm::vec3(0.0f, 1.1f, 0.0f));
        part = glm::rotate(part, glm::radians(120.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        part = glm::scale(part, glm::vec3(0.85f, 0.08f, 0.35f));  DRAW(part)

            USE_TEX(texConcreto)
            part = glm::translate(baseGiro, glm::vec3(0.0f, 1.3f, 0.0f));
        part = glm::rotate(part, glm::radians(160.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        part = glm::scale(part, glm::vec3(0.7f, 0.08f, 0.3f));  DRAW(part)

            USE_TEX(texConcreto2)
            part = glm::translate(baseGiro, glm::vec3(0.0f, 1.6f, 0.0f));
        part = glm::rotate(part, glm::radians(200.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        part = glm::scale(part, glm::vec3(0.25f, 0.25f, 0.25f));  DRAW(part)

            // Escultura 3 — vidrio 
            USE_TEX(texMetal)
            base = glm::mat4(1.0f);
        base = glm::translate(base, glm::vec3(movX, movY, movZ));
        base = glm::rotate(base, glm::radians(rot), glm::vec3(0, 1, 0));
        base = glm::translate(base, glm::vec3(-18.55f, 1.0f, -22.5f));

        USE_TEX(texVidAmarillo)
            part = glm::translate(base, glm::vec3(0, -0.5f, 0.3f));
        part = glm::rotate(part, glm::radians(55.0f), glm::vec3(0, 0, 1));
        part = glm::rotate(part, glm::radians(-18.0f), glm::vec3(0, 1, 0));
        part = glm::rotate(part, glm::radians(5.0f), glm::vec3(1, 0, 0));
        part = glm::scale(part, glm::vec3(3.4f, 0.13f, 0.72f));  DRAW(part)

            USE_TEX(texVidRosa)
            part = glm::translate(base, glm::vec3(-0.3f, -0.2f, 0.5f));
        part = glm::rotate(part, glm::radians(-32.0f), glm::vec3(0, 0, 1));
        part = glm::rotate(part, glm::radians(22.0f), glm::vec3(0, 1, 0));
        part = glm::rotate(part, glm::radians(-8.0f), glm::vec3(1, 0, 0));
        part = glm::scale(part, glm::vec3(3.1f, 0.11f, 0.68f));  DRAW(part)

            USE_TEX(texVidAzul)
            part = glm::translate(base, glm::vec3(0.6f, -0.5f, -0.4f));
        part = glm::rotate(part, glm::radians(-55.0f), glm::vec3(0, 0, 1));
        part = glm::rotate(part, glm::radians(-12.0f), glm::vec3(0, 1, 0));
        part = glm::rotate(part, glm::radians(18.0f), glm::vec3(1, 0, 0));
        part = glm::scale(part, glm::vec3(2.9f, 0.12f, 0.95f));  DRAW(part)

            USE_TEX(texVidRojo)
            part = glm::translate(base, glm::vec3(-0.2f, 0.55f, 0.6f));
        part = glm::rotate(part, glm::radians(82.0f), glm::vec3(0, 0, 1));
        part = glm::rotate(part, glm::radians(28.0f), glm::vec3(0, 1, 0));
        part = glm::rotate(part, glm::radians(12.0f), glm::vec3(1, 0, 0));
        part = glm::scale(part, glm::vec3(1.1f, 0.11f, 0.52f));  DRAW(part)

            // Escultura 4 - cruz metal
            USE_TEX(texPiedra)
            base = glm::mat4(1.0f);
        base = glm::translate(base, glm::vec3(movX, movY, movZ));
        base = glm::rotate(base, glm::radians(rot), glm::vec3(0, 1, 0));
        base = glm::translate(base, glm::vec3(-0.35f, -0.89f, -32.5f));

        part = glm::translate(base, glm::vec3(0, 0.07f, 0));
        part = glm::scale(part, glm::vec3(2.0f, 0.14f, 2.0f));  DRAW(part)

            USE_TEX(texRojo)
            part = glm::translate(base, glm::vec3(0, 2.2f, 0));
        part = glm::scale(part, glm::vec3(0.38f, 4.4f, 0.38f));  DRAW(part)
            part = glm::translate(base, glm::vec3(0, 3.0f, 0));
        part = glm::scale(part, glm::vec3(3.8f, 0.38f, 0.38f));  DRAW(part)

            USE_TEX(texAzul3)
            part = glm::translate(base, glm::vec3(0, 1.8f, 0));
        part = glm::rotate(part, glm::radians(90.0f), glm::vec3(0, 1, 0));
        part = glm::scale(part, glm::vec3(2.4f, 0.32f, 0.32f));  DRAW(part)

            USE_TEX(texAmarillo)
            part = glm::translate(base, glm::vec3(-1.9f, 3.0f, 0));
        part = glm::scale(part, glm::vec3(0.5f, 0.5f, 0.5f));  DRAW(part)
            part = glm::translate(base, glm::vec3(1.9f, 3.0f, 0));
        part = glm::scale(part, glm::vec3(0.5f, 0.5f, 0.5f));  DRAW(part)

            USE_TEX(texAzul)
            part = glm::translate(base, glm::vec3(0, 1.8f, 1.2f));
        part = glm::scale(part, glm::vec3(0.45f, 0.45f, 0.45f));  DRAW(part)
            part = glm::translate(base, glm::vec3(0, 1.8f, -1.2f));
        part = glm::scale(part, glm::vec3(0.45f, 0.45f, 0.45f));  DRAW(part)

            USE_TEX(texAzul2)
            part = glm::translate(base, glm::vec3(0, 4.62f, 0));
        part = glm::rotate(part, glm::radians(45.0f), glm::vec3(0, 1, 0));
        part = glm::scale(part, glm::vec3(0.5f, 0.5f, 0.5f));  DRAW(part)

            USE_TEX(texVerde)
            part = glm::translate(base, glm::vec3(0.6f, 2.5f, 0));
        part = glm::rotate(part, glm::radians(38.0f), glm::vec3(0, 0, 1));
        part = glm::scale(part, glm::vec3(1.8f, 0.12f, 0.32f));  DRAW(part)

            // Etiqueta obra
            USE_TEX(texMetal)
            base = glm::mat4(1.0f);
        base = glm::translate(base, glm::vec3(movX, movY, movZ));
        base = glm::rotate(base, glm::radians(rot), glm::vec3(0.0f, 1.0f, 0.0f));
        base = glm::translate(base, glm::vec3(-7.0f, -0.7f, -15.5f));
        // Animación de caída
        base = glm::translate(base, glm::vec3(0.0f, etiquetaOffY, 0.0f));
        base = glm::rotate(base, glm::radians(etiquetaRotZ), glm::vec3(0.0f, 0.0f, 1.0f));
        base = glm::rotate(base, glm::radians(etiquetaRotX), glm::vec3(1.0f, 0.0f, 0.0f));

        part = glm::translate(base, glm::vec3(0.0f, 0.04f, 0.0f));
        part = glm::scale(part, glm::vec3(0.55f, 0.06f, 0.55f));  DRAW(part)
            part = glm::translate(base, glm::vec3(0.0f, 0.85f, 0.0f));
        part = glm::scale(part, glm::vec3(0.05f, 1.6f, 0.05f));  DRAW(part)
            part = glm::translate(base, glm::vec3(0.0f, 1.68f, 0.04f));
        part = glm::scale(part, glm::vec3(0.5f, 0.04f, 0.04f));  DRAW(part)
            part = glm::translate(base, glm::vec3(0.0f, 1.22f, 0.04f));
        part = glm::scale(part, glm::vec3(0.5f, 0.04f, 0.04f));  DRAW(part)
            part = glm::translate(base, glm::vec3(-0.23f, 1.45f, 0.04f));
        part = glm::scale(part, glm::vec3(0.03f, 0.5f, 0.03f));  DRAW(part)
            part = glm::translate(base, glm::vec3(0.23f, 1.45f, 0.04f));
        part = glm::scale(part, glm::vec3(0.03f, 0.5f, 0.03f));  DRAW(part)

            USE_TEX(texPapel)
            part = glm::translate(base, glm::vec3(0.0f, 1.45f, 0.06f));
        part = glm::scale(part, glm::vec3(0.44f, 0.44f, 0.02f));  DRAW(part)

            USE_TEX(texMadera)
            part = glm::translate(base, glm::vec3(0.0f, 1.63f, 0.075f));
        part = glm::scale(part, glm::vec3(0.44f, 0.1f, 0.015f));  DRAW(part)

            USE_TEX(texConcreto2)
            part = glm::translate(base, glm::vec3(-0.04f, 1.52f, 0.075f));
        part = glm::scale(part, glm::vec3(0.3f, 0.025f, 0.012f));  DRAW(part)
            part = glm::translate(base, glm::vec3(-0.06f, 1.47f, 0.075f));
        part = glm::scale(part, glm::vec3(0.24f, 0.02f, 0.012f));  DRAW(part)
            part = glm::translate(base, glm::vec3(-0.05f, 1.42f, 0.075f));
        part = glm::scale(part, glm::vec3(0.26f, 0.02f, 0.012f));  DRAW(part)
            part = glm::translate(base, glm::vec3(-0.08f, 1.35f, 0.075f));
        part = glm::scale(part, glm::vec3(0.18f, 0.018f, 0.012f));  DRAW(part)

            // MACETA pequeña 
            USE_TEX(texTerracota)
            base = glm::mat4(1.0f);
        base = glm::translate(base, glm::vec3(movX, movY, movZ));
        base = glm::rotate(base, glm::radians(rot), glm::vec3(0.0f, 1.0f, 0.0f));
        base = glm::translate(base, glm::vec3(2.7f, -0.89f, -2.5f));

        part = glm::translate(base, glm::vec3(0.0f, 0.0f, 0.0f));
        part = glm::scale(part, glm::vec3(1.2f, 0.2f, 1.2f));  DRAW(part)
            part = glm::translate(base, glm::vec3(0.0f, 0.55f, 0.0f));
        part = glm::scale(part, glm::vec3(1.0f, 0.9f, 1.0f));  DRAW(part)
            part = glm::translate(base, glm::vec3(0.0f, 1.05f, 0.0f));
        part = glm::scale(part, glm::vec3(1.1f, 0.12f, 1.1f));  DRAW(part)

            USE_TEX(texTierra)
            part = glm::translate(base, glm::vec3(0.0f, 1.1f, 0.0f));
        part = glm::scale(part, glm::vec3(0.85f, 0.1f, 0.85f));  DRAW(part)

            USE_TEX(texHoja)
            part = glm::translate(base, glm::vec3(0.0f, 1.5f, 0.0f));
        part = glm::scale(part, glm::vec3(0.1f, 1.0f, 0.1f));  DRAW(part)
            part = glm::translate(base, glm::vec3(-0.4f, 1.8f, 0.0f));
        part = glm::rotate(part, glm::radians(-30.0f), glm::vec3(0.0f, 0.0f, 1.0f));
        part = glm::scale(part, glm::vec3(0.6f, 0.12f, 0.25f));  DRAW(part)
            part = glm::translate(base, glm::vec3(0.4f, 1.8f, 0.0f));
        part = glm::rotate(part, glm::radians(30.0f), glm::vec3(0.0f, 0.0f, 1.0f));
        part = glm::scale(part, glm::vec3(0.6f, 0.12f, 0.25f));  DRAW(part)
            part = glm::translate(base, glm::vec3(0.0f, 1.9f, 0.35f));
        part = glm::rotate(part, glm::radians(-25.0f), glm::vec3(1.0f, 0.0f, 0.0f));
        part = glm::scale(part, glm::vec3(0.25f, 0.12f, 0.55f));  DRAW(part)

            //  JARDINERA ext 
            USE_TEX(texConcreto)
            base = glm::mat4(1.0f);
        base = glm::translate(base, glm::vec3(movX, movY, movZ));
        base = glm::rotate(base, glm::radians(rot), glm::vec3(0.0f, 1.0f, 0.0f));
        base = glm::translate(base, glm::vec3(16.0f, -1.0f, -21.0f));
        base = glm::rotate(base, glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f));

        part = glm::translate(base, glm::vec3(0.0f, 0.6f, 0.0f));
        part = glm::scale(part, glm::vec3(3.0f, 1.2f, 1.2f));  DRAW(part)
            part = glm::translate(base, glm::vec3(0.0f, 0.06f, 0.0f));
        part = glm::scale(part, glm::vec3(3.2f, 0.12f, 1.35f));  DRAW(part)
            part = glm::translate(base, glm::vec3(0.0f, 1.26f, 0.0f));
        part = glm::scale(part, glm::vec3(3.15f, 0.12f, 1.3f));  DRAW(part)

            USE_TEX(texTierra)
            part = glm::translate(base, glm::vec3(0.0f, 1.30f, 0.0f));
        part = glm::scale(part, glm::vec3(2.8f, 0.08f, 1.0f));  DRAW(part)

            USE_TEX(texArbusto)
            part = glm::translate(base, glm::vec3(-0.9f, 1.75f, 0.0f));
        part = glm::scale(part, glm::vec3(0.8f, 0.75f, 0.8f));  DRAW(part)
            part = glm::translate(base, glm::vec3(-0.9f, 2.3f, 0.0f));
        part = glm::scale(part, glm::vec3(0.6f, 0.55f, 0.6f));  DRAW(part)
            part = glm::translate(base, glm::vec3(-0.9f, 2.75f, 0.0f));
        part = glm::scale(part, glm::vec3(0.4f, 0.4f, 0.4f));  DRAW(part)
            part = glm::translate(base, glm::vec3(0.9f, 1.75f, 0.0f));
        part = glm::scale(part, glm::vec3(0.8f, 0.75f, 0.8f));  DRAW(part)
            part = glm::translate(base, glm::vec3(0.9f, 2.3f, 0.0f));
        part = glm::scale(part, glm::vec3(0.6f, 0.55f, 0.6f));  DRAW(part)
            part = glm::translate(base, glm::vec3(0.9f, 2.75f, 0.0f));
        part = glm::scale(part, glm::vec3(0.4f, 0.4f, 0.4f));  DRAW(part)
            part = glm::translate(base, glm::vec3(0.0f, 1.9f, 0.0f));
        part = glm::scale(part, glm::vec3(0.1f, 1.2f, 0.1f));  DRAW(part)

            USE_TEX(texHoja)
            part = glm::translate(base, glm::vec3(0.0f, 2.5f, 0.0f));
        part = glm::rotate(part, glm::radians(20.0f), glm::vec3(0.0f, 0.0f, 1.0f));
        part = glm::scale(part, glm::vec3(0.7f, 0.12f, 0.3f));  DRAW(part)

            // Escultura 5 - Cubi XII David Smith 
            USE_TEX(texMetal)
            base = glm::mat4(1.0f);
        base = glm::translate(base, glm::vec3(movX, movY, movZ));
        base = glm::rotate(base, glm::radians(rot), glm::vec3(0, 1, 0));
        base = glm::translate(base, glm::vec3(0.8f, -0.89f, -21.0f));
        base = glm::rotate(base, glm::radians(-90.0f), glm::vec3(0.0f, 1.0f, 0.0f));

        USE_TEX(texPiedra)
            part = glm::translate(base, glm::vec3(-0.2f, 0.08f, 0));
        part = glm::scale(part, glm::vec3(2.2f, 0.16f, 2.2f));  DRAW(part)

            USE_TEX(texMetal)
            part = glm::translate(base, glm::vec3(-0.2f, 1.05f, 0));
        part = glm::scale(part, glm::vec3(0.4f, 2.15f, 0.4f));  DRAW(part)
            part = glm::translate(base, glm::vec3(0.3f, 2.0f, 0));
        part = glm::scale(part, glm::vec3(0.68f, 0.68f, 0.68f));  DRAW(part)
            part = glm::translate(base, glm::vec3(0.9f, 3.2f, 0));
        part = glm::rotate(part, glm::radians(30.0f), glm::vec3(0, 1, 0));
        part = glm::scale(part, glm::vec3(0.5f, 1.9f, 0.5f));  DRAW(part)
            part = glm::translate(base, glm::vec3(-0.85f, 2.4f, 0));
        part = glm::rotate(part, glm::radians(45.0f), glm::vec3(0, 0, 1));
        part = glm::scale(part, glm::vec3(1.0f, 1.0f, 0.85f));  DRAW(part)
            part = glm::translate(base, glm::vec3(-0.8f, 3.3f, 0));
        part = glm::rotate(part, glm::radians(45.0f), glm::vec3(0, 0, 1));
        part = glm::scale(part, glm::vec3(0.65f, 0.65f, 0.65f));  DRAW(part)
            part = glm::translate(base, glm::vec3(-0.75f, 4.0f, 0));
        part = glm::rotate(part, glm::radians(45.0f), glm::vec3(0, 0, 1));
        part = glm::scale(part, glm::vec3(0.5f, 0.5f, 0.5f));  DRAW(part)
            part = glm::translate(base, glm::vec3(0.12f, 4.5f, 0));
        part = glm::scale(part, glm::vec3(1.4f, 0.7f, 1.0f));  DRAW(part)

            // Escultura 6 - BLACKBOARD Louise Nevelson 
            USE_TEX(texNegro)
            base = glm::mat4(1.0f);
        base = glm::translate(base, glm::vec3(movX, movY, movZ));
        base = glm::rotate(base, glm::radians(rot), glm::vec3(0, 1, 0));
        base = glm::translate(base, glm::vec3(-16.5f, -0.89f, -21.0f));
        base = glm::rotate(base, glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f));

        part = glm::translate(base, glm::vec3(0, 2.5f, 0));
        part = glm::scale(part, glm::vec3(5.5f, 5.0f, 0.15f));  DRAW(part)

            // Caja 1
            part = glm::translate(base, glm::vec3(-1.8f, 0.7f, 0.12f));
        part = glm::scale(part, glm::vec3(2.0f, 1.4f, 0.12f));  DRAW(part)
            part = glm::translate(base, glm::vec3(-1.8f, 1.35f, 0.22f));
        part = glm::scale(part, glm::vec3(2.0f, 0.1f, 0.25f));  DRAW(part)
            part = glm::translate(base, glm::vec3(-1.8f, 0.05f, 0.22f));
        part = glm::scale(part, glm::vec3(2.0f, 0.1f, 0.25f));  DRAW(part)
            part = glm::translate(base, glm::vec3(-2.75f, 0.7f, 0.22f));
        part = glm::scale(part, glm::vec3(0.1f, 1.4f, 0.25f));  DRAW(part)
            part = glm::translate(base, glm::vec3(-0.85f, 0.7f, 0.22f));
        part = glm::scale(part, glm::vec3(0.1f, 1.4f, 0.25f));  DRAW(part)

            USE_TEX(texGris)
            part = glm::translate(base, glm::vec3(-2.3f, 0.7f, 0.28f));
        part = glm::scale(part, glm::vec3(0.6f, 1.1f, 0.1f));  DRAW(part)
            part = glm::translate(base, glm::vec3(-1.8f, 0.4f, 0.28f));
        part = glm::rotate(part, glm::radians(45.0f), glm::vec3(0, 0, 1));
        part = glm::scale(part, glm::vec3(1.5f, 0.35f, 0.1f));  DRAW(part)
            part = glm::translate(base, glm::vec3(-1.4f, 0.9f, 0.28f));
        part = glm::scale(part, glm::vec3(0.5f, 0.6f, 0.1f));  DRAW(part)
            part = glm::translate(base, glm::vec3(-1.9f, 1.1f, 0.28f));
        part = glm::scale(part, glm::vec3(0.8f, 0.1f, 0.1f));  DRAW(part)

            // Caja 2
            USE_TEX(texNegro)
            part = glm::translate(base, glm::vec3(1.2f, 0.7f, 0.12f));
        part = glm::scale(part, glm::vec3(1.5f, 1.4f, 0.12f));  DRAW(part)
            part = glm::translate(base, glm::vec3(1.2f, 1.35f, 0.22f));
        part = glm::scale(part, glm::vec3(1.5f, 0.1f, 0.25f));  DRAW(part)
            part = glm::translate(base, glm::vec3(1.2f, 0.05f, 0.22f));
        part = glm::scale(part, glm::vec3(1.5f, 0.1f, 0.25f));  DRAW(part)
            part = glm::translate(base, glm::vec3(0.5f, 0.7f, 0.22f));
        part = glm::scale(part, glm::vec3(0.1f, 1.4f, 0.25f));  DRAW(part)
            part = glm::translate(base, glm::vec3(1.9f, 0.7f, 0.22f));
        part = glm::scale(part, glm::vec3(0.1f, 1.4f, 0.25f));  DRAW(part)

            USE_TEX(texGris)
            part = glm::translate(base, glm::vec3(1.0f, 0.5f, 0.28f));
        part = glm::scale(part, glm::vec3(0.3f, 0.8f, 0.1f));  DRAW(part)
            part = glm::translate(base, glm::vec3(1.4f, 0.9f, 0.28f));
        part = glm::rotate(part, glm::radians(30.0f), glm::vec3(0, 0, 1));
        part = glm::scale(part, glm::vec3(0.7f, 0.35f, 0.1f));  DRAW(part)
            part = glm::translate(base, glm::vec3(1.2f, 0.3f, 0.28f));
        part = glm::scale(part, glm::vec3(0.9f, 0.2f, 0.1f));  DRAW(part)

            // Cajas fila media
            USE_TEX(texNegro)
            part = glm::translate(base, glm::vec3(-2.0f, 2.1f, 0.12f));
        part = glm::scale(part, glm::vec3(1.4f, 1.2f, 0.12f));  DRAW(part)
            part = glm::translate(base, glm::vec3(-2.0f, 2.65f, 0.22f));
        part = glm::scale(part, glm::vec3(1.4f, 0.1f, 0.25f));  DRAW(part)
            part = glm::translate(base, glm::vec3(-2.0f, 1.55f, 0.22f));
        part = glm::scale(part, glm::vec3(1.4f, 0.1f, 0.25f));  DRAW(part)
            part = glm::translate(base, glm::vec3(-2.65f, 2.1f, 0.22f));
        part = glm::scale(part, glm::vec3(0.1f, 1.2f, 0.25f));  DRAW(part)
            part = glm::translate(base, glm::vec3(-1.35f, 2.1f, 0.22f));
        part = glm::scale(part, glm::vec3(0.1f, 1.2f, 0.25f));  DRAW(part)
            part = glm::translate(base, glm::vec3(-2.1f, 2.1f, 0.28f));
        part = glm::scale(part, glm::vec3(0.35f, 0.9f, 0.1f));  DRAW(part)

            USE_TEX(texGris)
            part = glm::translate(base, glm::vec3(-1.75f, 2.4f, 0.28f));
        part = glm::rotate(part, glm::radians(60.0f), glm::vec3(0, 0, 1));
        part = glm::scale(part, glm::vec3(0.5f, 0.1f, 0.1f));  DRAW(part)

            USE_TEX(texNegro)
            part = glm::translate(base, glm::vec3(0.2f, 2.1f, 0.12f));
        part = glm::scale(part, glm::vec3(1.8f, 1.2f, 0.12f));  DRAW(part)
            part = glm::translate(base, glm::vec3(0.2f, 2.65f, 0.22f));
        part = glm::scale(part, glm::vec3(1.8f, 0.1f, 0.25f));  DRAW(part)
            part = glm::translate(base, glm::vec3(0.2f, 1.55f, 0.22f));
        part = glm::scale(part, glm::vec3(1.8f, 0.1f, 0.25f));  DRAW(part)
            part = glm::translate(base, glm::vec3(-0.7f, 2.1f, 0.22f));
        part = glm::scale(part, glm::vec3(0.1f, 1.2f, 0.25f));  DRAW(part)
            part = glm::translate(base, glm::vec3(1.1f, 2.1f, 0.22f));
        part = glm::scale(part, glm::vec3(0.1f, 1.2f, 0.25f));  DRAW(part)
            part = glm::translate(base, glm::vec3(0.3f, 2.3f, 0.28f));
        part = glm::scale(part, glm::vec3(0.6f, 0.6f, 0.1f));  DRAW(part)

            USE_TEX(texGris)
            part = glm::translate(base, glm::vec3(0.0f, 1.8f, 0.28f));
        part = glm::rotate(part, glm::radians(15.0f), glm::vec3(0, 0, 1));
        part = glm::scale(part, glm::vec3(1.0f, 0.15f, 0.1f));  DRAW(part)

            USE_TEX(texNegro)
            part = glm::translate(base, glm::vec3(2.3f, 2.1f, 0.12f));
        part = glm::scale(part, glm::vec3(1.0f, 1.2f, 0.12f));  DRAW(part)
            part = glm::translate(base, glm::vec3(2.3f, 2.65f, 0.22f));
        part = glm::scale(part, glm::vec3(1.0f, 0.1f, 0.25f));  DRAW(part)
            part = glm::translate(base, glm::vec3(2.3f, 1.55f, 0.22f));
        part = glm::scale(part, glm::vec3(1.0f, 0.1f, 0.25f));  DRAW(part)
            part = glm::translate(base, glm::vec3(1.85f, 2.1f, 0.22f));
        part = glm::scale(part, glm::vec3(0.1f, 1.2f, 0.25f));  DRAW(part)
            part = glm::translate(base, glm::vec3(2.75f, 2.1f, 0.22f));
        part = glm::scale(part, glm::vec3(0.1f, 1.2f, 0.25f));  DRAW(part)

            USE_TEX(texGris)
            part = glm::translate(base, glm::vec3(2.3f, 2.2f, 0.28f));
        part = glm::rotate(part, glm::radians(45.0f), glm::vec3(0, 0, 1));
        part = glm::scale(part, glm::vec3(0.7f, 0.7f, 0.1f));  DRAW(part)

            // Fila superior
            float cxs[] = { -1.8f, -0.6f, 0.6f, 1.8f };
        for (int i = 0; i < 4; i++) {
            float cx = cxs[i];
            USE_TEX(texNegro)
                part = glm::translate(base, glm::vec3(cx, 3.5f, 0.12f));
            part = glm::scale(part, glm::vec3(1.1f, 0.9f, 0.12f));  DRAW(part)
                part = glm::translate(base, glm::vec3(cx, 3.9f, 0.22f));
            part = glm::scale(part, glm::vec3(1.1f, 0.1f, 0.22f));  DRAW(part)
                part = glm::translate(base, glm::vec3(cx, 3.1f, 0.22f));
            part = glm::scale(part, glm::vec3(1.1f, 0.1f, 0.22f));  DRAW(part)
                part = glm::translate(base, glm::vec3(cx - 0.5f, 3.5f, 0.22f));
            part = glm::scale(part, glm::vec3(0.1f, 0.9f, 0.22f));  DRAW(part)
                part = glm::translate(base, glm::vec3(cx + 0.5f, 3.5f, 0.22f));
            part = glm::scale(part, glm::vec3(0.1f, 0.9f, 0.22f));  DRAW(part)
                float ang = (i % 2 == 0) ? 30.0f : -30.0f;
            USE_TEX(texGris)
                part = glm::translate(base, glm::vec3(cx + 0.1f, 3.5f, 0.28f));
            part = glm::rotate(part, glm::radians(ang), glm::vec3(0, 0, 1));
            part = glm::scale(part, glm::vec3(0.7f, 0.55f, 0.08f));  DRAW(part)
                part = glm::translate(base, glm::vec3(cx - 0.1f, 3.2f, 0.28f));
            part = glm::scale(part, glm::vec3(0.5f, 0.12f, 0.08f));  DRAW(part)
        }

        USE_TEX(texNegro)
            part = glm::translate(base, glm::vec3(-1.5f, 4.55f, 0.15f));
        part = glm::scale(part, glm::vec3(1.2f, 0.5f, 0.18f));  DRAW(part)
            part = glm::translate(base, glm::vec3(0.5f, 4.55f, 0.15f));
        part = glm::scale(part, glm::vec3(1.6f, 0.5f, 0.18f));  DRAW(part)
            part = glm::translate(base, glm::vec3(2.2f, 4.55f, 0.15f));
        part = glm::scale(part, glm::vec3(0.8f, 0.5f, 0.18f));  DRAW(part)

#undef DRAW
#undef USE_TEX

            glBindVertexArray(0);

       
        lightingShader.Use();

        glm::mat4 modelOBJ = glm::mat4(1.0f);
        modelOBJ = glm::translate(modelOBJ, glm::vec3(movX, movY, movZ));
        modelOBJ = glm::rotate(modelOBJ, glm::radians(rot), glm::vec3(0, 1, 0));
        modelOBJ = glm::translate(modelOBJ, glm::vec3(0.0f, -1.0f, -3.0f));
        modelOBJ = glm::scale(modelOBJ, glm::vec3(0.5f, 0.5f, 0.5f));

        glUniformMatrix4fv(glGetUniformLocation(lightingShader.Program, "projection"), 1, GL_FALSE, glm::value_ptr(projection));
        glUniformMatrix4fv(glGetUniformLocation(lightingShader.Program, "view"), 1, GL_FALSE, glm::value_ptr(view));
        glUniformMatrix4fv(glGetUniformLocation(lightingShader.Program, "model"), 1, GL_FALSE, glm::value_ptr(modelOBJ));

        estructura.Draw(lightingShader);

        // Carro 1
        glm::mat4 modelCarro = escenaBase;
        modelCarro = glm::translate(modelCarro, glm::vec3(carroPosX, -1.0f, carroPosZ));
        modelCarro = glm::rotate(modelCarro, glm::radians(carroRot), glm::vec3(0.0f, 1.0f, 0.0f));
        modelCarro = glm::scale(modelCarro, glm::vec3(0.5f, 0.5f, 0.5f));
        glUniformMatrix4fv(glGetUniformLocation(lightingShader.Program, "model"), 1, GL_FALSE, glm::value_ptr(modelCarro));
        carro.Draw(lightingShader);

        // Carro 2
        glm::mat4 modelCarro2 = escenaBase;
        modelCarro2 = glm::translate(modelCarro2, glm::vec3(carro2PosX, -1.0f, carro2PosZ));
        modelCarro2 = glm::rotate(modelCarro2, glm::radians(carro2Rot), glm::vec3(0.0f, 1.0f, 0.0f));
        modelCarro2 = glm::scale(modelCarro2, glm::vec3(0.5f, 0.5f, 0.5f));
        glUniformMatrix4fv(glGetUniformLocation(lightingShader.Program, "model"), 1, GL_FALSE, glm::value_ptr(modelCarro2));
        carro2.Draw(lightingShader);

        // Perro
        glm::mat4 perroBase, perroModel;

        perroBase = escenaBase;
        perroBase = glm::translate(perroBase, glm::vec3(perroPosX, perroPosY, perroPosZ));
        perroBase = glm::rotate(perroBase, glm::radians(perroRot), glm::vec3(0, 1, 0));
        perroBase = glm::translate(perroBase, glm::vec3(-19.5f, 0.5f, -6.0f));
        perroBase = glm::rotate(perroBase, glm::radians(270.0f), glm::vec3(0.0f, 1.0f, 0.0f));

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, texConcreto2);
        glUniformMatrix4fv(glGetUniformLocation(lightingShader.Program, "model"), 1, GL_FALSE, glm::value_ptr(perroBase));
        baseP.Draw(lightingShader);

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, texMAzul);
        glUniformMatrix4fv(glGetUniformLocation(lightingShader.Program, "model"), 1, GL_FALSE, glm::value_ptr(perroBase));
        perro.Draw(lightingShader);

        perroModel = perroBase;
        perroModel = glm::translate(perroModel, glm::vec3(0.0f, 0.0f, 0.02f));
        perroModel = glm::rotate(perroModel, glm::radians(perroCabRot), glm::vec3(0, 1, 0));
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, texMAzul);
        glUniformMatrix4fv(glGetUniformLocation(lightingShader.Program, "model"), 1, GL_FALSE, glm::value_ptr(perroModel));
        cabeza.Draw(lightingShader);

        
        perroModel = perroBase;
        perroModel = glm::rotate(perroModel, glm::radians(perroColaRot), glm::vec3(0, 0, 1));
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, texMAzul);
        glUniformMatrix4fv(glGetUniformLocation(lightingShader.Program, "model"), 1, GL_FALSE, glm::value_ptr(perroModel));
        cola.Draw(lightingShader);

        perroModel = perroBase;
        perroModel = glm::translate(perroModel, glm::vec3(
            0.0f + (perroPataRot * -0.017f),
            0.0f + (perroPataRot * 0.009f),
            0.0f));
        perroModel = glm::rotate(perroModel, glm::radians(perroPataRot), glm::vec3(0.0f, 0.0f, -1.0f));
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, texMAzul);
        glUniformMatrix4fv(glGetUniformLocation(lightingShader.Program, "model"), 1, GL_FALSE, glm::value_ptr(perroModel));
        pata.Draw(lightingShader);

        //  LUZ
        lightingShader.Use();
        glUniformMatrix4fv(glGetUniformLocation(lightingShader.Program, "projection"), 1, GL_FALSE, glm::value_ptr(projection));
        glUniformMatrix4fv(glGetUniformLocation(lightingShader.Program, "view"), 1, GL_FALSE, glm::value_ptr(view));
        glUniform3f(glGetUniformLocation(lightingShader.Program, "viewPos"), -movX, -movY, -movZ);
        glUniform1i(glGetUniformLocation(lightingShader.Program, "numLights"), numLamps);
        glUniform1i(glGetUniformLocation(lightingShader.Program, "texture_diffuse"), 0);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, texPapel);

        glBindVertexArray(lampVAO);
        for (int i = 0; i < numLamps; i++) {
            glm::mat4 lampModel = escenaBase;
            lampModel = glm::translate(lampModel, lampPositions[i]);
            lampModel = glm::scale(lampModel, glm::vec3(0.1f, 0.1f, 0.1f));
            glUniformMatrix4fv(glGetUniformLocation(lightingShader.Program, "model"), 1, GL_FALSE, glm::value_ptr(lampModel));
            glDrawArrays(GL_TRIANGLES, 0, 36);
        }
        glBindVertexArray(0);

        glfwSwapBuffers(window);
    }

    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteVertexArrays(1, &lampVAO);
    glDeleteBuffers(1, &lampVBO);
    glfwTerminate();
    return EXIT_SUCCESS;
}


void Inputs(GLFWwindow* window) {
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) glfwSetWindowShouldClose(window, true);
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) movX += 0.3f;
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) movX -= 0.3f;
    if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS) movY += 0.3f;
    if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS) movY -= 0.3f;
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) movZ -= 0.3f;
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) movZ += 0.3f;
    if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS) rot += 0.5f;
    if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS) rot -= 0.5f;

    // L — luces on/off
    bool lAhora = glfwGetKey(window, GLFW_KEY_L) == GLFW_PRESS;
    if (lAhora && !lTeclaAnterior) lucesOn = !lucesOn;
    lTeclaAnterior = lAhora;

    // N — modo noche
    bool nAhora = glfwGetKey(window, GLFW_KEY_N) == GLFW_PRESS;
    if (nAhora && !nTeclaAnterior) modoNoche = !modoNoche;
    nTeclaAnterior = nAhora;

    // P — carro 1
    bool pAhora = glfwGetKey(window, GLFW_KEY_P) == GLFW_PRESS;
    if (pAhora && !pTeclaAnterior && estadoCarro == ESPERANDO)
        estadoCarro = AVANZANDO;
    pTeclaAnterior = pAhora;

    // O — carro 2
    bool oAhora = glfwGetKey(window, GLFW_KEY_O) == GLFW_PRESS;
    if (oAhora && !oTeclaAnterior && estadoCarro2 == ESPERANDO2)
        estadoCarro2 = AVANZANDO2;
    oTeclaAnterior = oAhora;

    // I — perro
    bool iAhora = glfwGetKey(window, GLFW_KEY_I) == GLFW_PRESS;
    if (iAhora && !iTeclaAnterior && estadoPerro == PERRO_QUIETO) {
        perroFase = 0;
        perroStep = false;
        perroTicks = 0;
        perroColaRot = 0.0f;
        perroCabRot = 0.0f;
        perroPataRot = 0.0f;
        estadoPerro = PERRO_ANIMANDO;
    }
    iTeclaAnterior = iAhora;

    // T — etiqueta: caer / levantar
    bool tAhora = glfwGetKey(window, GLFW_KEY_T) == GLFW_PRESS;
    if (tAhora && !tTeclaAnterior) {
        if (estadoEtiqueta == ETIQUETA_QUIETA) {
            etiquetaTicks = 0;
            etiquetaStep = false;
            estadoEtiqueta = ETIQUETA_TAMBALEANDO;
        }
        else if (estadoEtiqueta == ETIQUETA_EN_SUELO) {
            estadoEtiqueta = ETIQUETA_VOLVIENDO;
        }
    }
    tTeclaAnterior = tAhora;
}