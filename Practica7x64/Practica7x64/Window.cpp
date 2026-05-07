#include "Window.h"

Window::Window()
{
    width = 800;
    height = 600;
    muevex = 0.0f;
}

Window::Window(GLint windowWidth, GLint windowHeight)
{
    width = windowWidth;
    height = windowHeight;
    muevex = 2.0f;
}

int Window::Initialise()
{
    if (!glfwInit()) {
        printf("Falló inicializar GLFW\n");
        glfwTerminate();
        return 1;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);

    mainWindow = glfwCreateWindow(width, height, "PracticaXX:Nombre de la practica", NULL, NULL);
    if (!mainWindow) {
        printf("Fallo en crearse la ventana con GLFW\n");
        glfwTerminate();
        return 1;
    }

    glfwGetFramebufferSize(mainWindow, &bufferWidth, &bufferHeight);
    glfwMakeContextCurrent(mainWindow);

    createCallbacks();
    glewExperimental = GL_TRUE;
    if (glewInit() != GLEW_OK) {
        printf("Falló inicialización de GLEW\n");
        glfwDestroyWindow(mainWindow);
        glfwTerminate();
        return 1;
    }

    glEnable(GL_DEPTH_TEST);
    glViewport(0, 0, bufferWidth, bufferHeight);

    glfwSetWindowUserPointer(mainWindow, this);

    // Estado inicial del carro
    carWheelAng = 0.0f;
    carHoodAng = 0.0f;
    carPosZ = 0.0f;
    carYaw = 0.0f;

    return 0; // éxito
}

void Window::createCallbacks()
{
    glfwSetKeyCallback(mainWindow, ManejaTeclado);
    glfwSetCursorPosCallback(mainWindow, ManejaMouse);
}

GLfloat Window::getXChange()
{
    GLfloat theChange = xChange;
    xChange = 0.0f;
    return theChange;
}

GLfloat Window::getYChange()
{
    GLfloat theChange = yChange;
    yChange = 0.0f;
    return theChange;
}

void Window::ManejaTeclado(GLFWwindow* window, int key, int /*code*/, int action, int /*mode*/)
{
    Window* theWindow = static_cast<Window*>(glfwGetWindowUserPointer(window));

    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
        glfwSetWindowShouldClose(window, GL_TRUE);

    if (key == GLFW_KEY_Y) theWindow->muevex += 1.0f;
    if (key == GLFW_KEY_U) theWindow->muevex -= 1.0f;

    if (key == GLFW_KEY_E) theWindow->rotax += 10.0f;
    if (key == GLFW_KEY_R) theWindow->rotay += 10.0f;
    if (key == GLFW_KEY_T) theWindow->rotaz += 10.0f;

    if (key == GLFW_KEY_F) theWindow->articulacion1 += 10.0f;
    if (key == GLFW_KEY_G) theWindow->articulacion2 += 10.0f;
    if (key == GLFW_KEY_H) theWindow->articulacion3 += 10.0f;
    if (key == GLFW_KEY_J) theWindow->articulacion4 += 10.0f;
    if (key == GLFW_KEY_K) theWindow->articulacion5 += 10.0f;
    if (key == GLFW_KEY_L) theWindow->articulacion6 += 10.0f;

    if (key == GLFW_KEY_D && action == GLFW_PRESS) {
        const char* key_name = glfwGetKeyName(GLFW_KEY_D, 0);
        (void)key_name; // evitar warning si no lo usas
    }

    if (key >= 0 && key < 1024)
    {
        if (action == GLFW_PRESS)   theWindow->keys[key] = true;
        else if (action == GLFW_RELEASE) theWindow->keys[key] = false;
    }

    if (key == GLFW_KEY_F && action == GLFW_PRESS)  theWindow->articulacion1 += 10.0f;
    if (key == GLFW_KEY_V && action == GLFW_PRESS)  theWindow->articulacion1 -= 10.0f;

    if (key == GLFW_KEY_G && action == GLFW_PRESS)  theWindow->articulacion2 += 10.0f;
    if (key == GLFW_KEY_B && action == GLFW_PRESS)  theWindow->articulacion2 -= 10.0f;

    if (key == GLFW_KEY_H && action == GLFW_PRESS)  theWindow->articulacion3 += 10.0f;
    if (key == GLFW_KEY_N && action == GLFW_PRESS)  theWindow->articulacion3 -= 10.0f;

    if (key == GLFW_KEY_J && action == GLFW_PRESS)  theWindow->articulacion4 += 10.0f;
    if (key == GLFW_KEY_M && action == GLFW_PRESS)  theWindow->articulacion4 -= 10.0f;

    if (key == GLFW_KEY_U && action == GLFW_PRESS)  theWindow->articulacion5 += 10.0f;
    if (key == GLFW_KEY_Y && action == GLFW_PRESS)  theWindow->articulacion5 -= 10.0f;
}

void Window::ManejaMouse(GLFWwindow* window, double xPos, double yPos)
{
    Window* theWindow = static_cast<Window*>(glfwGetWindowUserPointer(window));

    if (theWindow->mouseFirstMoved)
    {
        theWindow->lastX = (GLfloat)xPos;
        theWindow->lastY = (GLfloat)yPos;
        theWindow->mouseFirstMoved = false;
    }

    theWindow->xChange = (GLfloat)xPos - theWindow->lastX;
    theWindow->yChange = theWindow->lastY - (GLfloat)yPos;

    theWindow->lastX = (GLfloat)xPos;
    theWindow->lastY = (GLfloat)yPos;
}

void Window::ProcessLampToggle(LampState& lamp, float /*dt*/)
{
    static bool latch = false;
    const bool pressed = getsKeys()[GLFW_KEY_L]; // tecla L para conmutar
    if (pressed) {
        if (!latch) { lamp.on = !lamp.on; latch = true; }
    }
    else {
        latch = false;
    }
}

Window::~Window()
{
    glfwDestroyWindow(mainWindow);
    glfwTerminate();
}

