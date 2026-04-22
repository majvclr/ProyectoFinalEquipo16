#include <iostream>
#include <cmath>

// GLEW
#include <GL/glew.h>

// GLFW
#include <GLFW/glfw3.h>

// Other Libs
#include "stb_image.h"

// GLM Mathematics
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

//Load Models
#include "SOIL2/SOIL2.h"


// Other includes
#include "Shader.h"
#include "Camera.h"
#include "Model.h"


// Function prototypes
void KeyCallback(GLFWwindow *window, int key, int scancode, int action, int mode);
void MouseCallback(GLFWwindow *window, double xPos, double yPos);
void DoMovement();
void Animation();

// Window dimensions
const GLuint WIDTH = 800, HEIGHT = 600;
int SCREEN_WIDTH, SCREEN_HEIGHT;

// Camera
Camera  camera(glm::vec3(0.0f, 0.0f, 3.0f));
GLfloat lastX = WIDTH / 2.0;
GLfloat lastY = HEIGHT / 2.0;
bool keys[1024];
bool firstMouse = true;
// Light attributes
glm::vec3 lightPos(0.0f, 0.0f, 0.0f);
bool active;

// Positions of the point lights
glm::vec3 pointLightPositions[] = {
	glm::vec3(0.0f,2.0f, 0.0f),
	glm::vec3(0.0f,0.0f, 0.0f),
	glm::vec3(0.0f,0.0f,  0.0f),
	glm::vec3(0.0f,0.0f, 0.0f)
};

float vertices[] = {
	 -0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
		0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
		0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
		0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
	   -0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
	   -0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,

	   -0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,
		0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,
		0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,
		0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,
	   -0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,
	   -0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,

	   -0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,
	   -0.5f,  0.5f, -0.5f, -1.0f,  0.0f,  0.0f,
	   -0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,
	   -0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,
	   -0.5f, -0.5f,  0.5f, -1.0f,  0.0f,  0.0f,
	   -0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,

		0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,
		0.5f,  0.5f, -0.5f,  1.0f,  0.0f,  0.0f,
		0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,
		0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,
		0.5f, -0.5f,  0.5f,  1.0f,  0.0f,  0.0f,
		0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,

	   -0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,
		0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,
		0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,
		0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,
	   -0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,
	   -0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,

	   -0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,
		0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,
		0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,
		0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,
	   -0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,
	   -0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f
};


glm::vec3 Light1 = glm::vec3(0);
//Anim
float rotBall = 0.0f;
float rotDog = 0.0f;
int dogAnim = 0;
float FLegs = 0.0f;
float RLegs = 0.0f;
float head = 0.0f;
float tail = 0.0f;



//KeyFrames
float dogPosX , dogPosY , dogPosZ  ;

#define MAX_FRAMES 9
int i_max_steps = 190;
int i_curr_steps = 0;
typedef struct _frame {
	
	float rotDog;
	float rotDogInc;
	float dogPosX;
	float dogPosY;
	float dogPosZ;
	float incX;
	float incY;
	float incZ;

	float head;
	float headInc;

	float Flegs;
	float FlegsInc;

	float Rlegs;
	float RlegsInc;

	float tail;
	float tailInc;

}FRAME;

FRAME KeyFrame[MAX_FRAMES];
int FrameIndex = 0;			//introducir datos
bool play = false;
int playIndex = 0;

void saveFrame(void)  //Guarda la posición donde está el modelo
{

	printf("frameindex %d\n", FrameIndex);

	KeyFrame[FrameIndex].dogPosX = dogPosX;
	KeyFrame[FrameIndex].dogPosY = dogPosY;
	KeyFrame[FrameIndex].dogPosZ = dogPosZ;

	KeyFrame[FrameIndex].rotDog = rotDog;

	KeyFrame[FrameIndex].head = head;

	KeyFrame[FrameIndex].Flegs = FLegs;
	KeyFrame[FrameIndex].Rlegs = RLegs;
	KeyFrame[FrameIndex].tail = tail;

	FrameIndex++;
}

void resetElements(void)  // Regresa a la primera foto
{
	dogPosX = KeyFrame[0].dogPosX;
	dogPosY = KeyFrame[0].dogPosY;
	dogPosZ = KeyFrame[0].dogPosZ;

	rotDog = KeyFrame[0].rotDog;

	head = KeyFrame[0].head; 

	FLegs = KeyFrame[0].Flegs;
	RLegs = KeyFrame[0].Rlegs;
	tail = KeyFrame[0].tail;

}
void interpolation(void)
{

	KeyFrame[playIndex].incX = (KeyFrame[playIndex + 1].dogPosX - KeyFrame[playIndex].dogPosX) / i_max_steps;
	KeyFrame[playIndex].incY = (KeyFrame[playIndex + 1].dogPosY - KeyFrame[playIndex].dogPosY) / i_max_steps;
	KeyFrame[playIndex].incZ = (KeyFrame[playIndex + 1].dogPosZ - KeyFrame[playIndex].dogPosZ) / i_max_steps;

	KeyFrame[playIndex].rotDogInc = (KeyFrame[playIndex + 1].rotDog - KeyFrame[playIndex].rotDog) / i_max_steps;

	KeyFrame[playIndex].headInc = (KeyFrame[playIndex + 1].head - KeyFrame[playIndex].head) / i_max_steps;

	KeyFrame[playIndex].FlegsInc = (KeyFrame[playIndex + 1].Flegs - KeyFrame[playIndex].Flegs) / i_max_steps;
	KeyFrame[playIndex].RlegsInc = (KeyFrame[playIndex + 1].Rlegs - KeyFrame[playIndex].Rlegs) / i_max_steps;
	KeyFrame[playIndex].tailInc = (KeyFrame[playIndex + 1].tail - KeyFrame[playIndex].tail) / i_max_steps;
}



// Deltatime
GLfloat deltaTime = 0.0f;	// Time between current frame and last frame
GLfloat lastFrame = 0.0f;  	// Time of last frame

int main()
{
	// Init GLFW
	glfwInit();
	// Set all the required options for GLFW
	/*glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
	glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);*/

	// Create a GLFWwindow object that we can use for GLFW's functions
	GLFWwindow* window = glfwCreateWindow(WIDTH, HEIGHT, "Animacion maquina de estados", nullptr, nullptr);

	if (nullptr == window)
	{
		std::cout << "Failed to create GLFW window" << std::endl;
		glfwTerminate();

		return EXIT_FAILURE;
	}

	glfwMakeContextCurrent(window);

	glfwGetFramebufferSize(window, &SCREEN_WIDTH, &SCREEN_HEIGHT);

	// Set the required callback functions
	glfwSetKeyCallback(window, KeyCallback);
	glfwSetCursorPosCallback(window, MouseCallback);

	// GLFW Options
	//glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

	// Set this to true so GLEW knows to use a modern approach to retrieving function pointers and extensions
	glewExperimental = GL_TRUE;
	// Initialize GLEW to setup the OpenGL Function pointers
	if (GLEW_OK != glewInit())
	{
		std::cout << "Failed to initialize GLEW" << std::endl;
		return EXIT_FAILURE;
	}

	// Define the viewport dimensions
	glViewport(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);



	Shader lightingShader("Shader/lighting.vs", "Shader/lighting.frag");
	Shader lampShader("Shader/lamp.vs", "Shader/lamp.frag");


	////models
	//Model DogBody((char*)"Models/DogBody.obj");
	//Model HeadDog((char*)"Models/HeadDog.obj");
	//Model DogTail((char*)"Models/TailDog.obj");
	//Model F_RightLeg((char*)"Models/F_RightLegDog.obj");
	//Model F_LeftLeg((char*)"Models/F_LeftLegDog.obj");
	//Model B_RightLeg((char*)"Models/B_RightLegDog.obj");
	//Model B_LeftLeg((char*)"Models/B_LeftLegDog.obj");
	//Model Piso((char*)"Models/piso.obj");
	//Model Ball((char*)"Models/ball.obj");

	// Modelos Fachada
	Model AguaF((char*)"ModelosFachada/AguaF.obj");
	Model Arcos((char*)"ModelosFachada/Arcos.obj");
	Model Bancas((char*)"ModelosFachada/Bancas.obj");
	Model BarandalZ((char*)"ModelosFachada/BarandalZ.obj");
	Model Campana((char*)"ModelosFachada/Campana.obj");
	Model Faros((char*)"ModelosFachada/FarosF.obj");
	Model Fuente((char*)"ModelosFachada/Fuente.obj");
	Model Letras((char*)"ModelosFachada/Letras.obj");
	Model ParedF1((char*)"ModelosFachada/ParedF1.obj");
	Model ParedF2((char*)"ModelosFachada/ParedF2.obj");
	Model ParedFA1((char*)"ModelosFachada/ParedFA1.obj");
	Model ParedFA2((char*)"ModelosFachada/ParedFA2.obj");
	Model ParedFA3((char*)"ModelosFachada/ParedFA3.obj");
	Model ParedFA4((char*)"ModelosFachada/ParedFA4.obj");
	Model ParedFA5((char*)"ModelosFachada/ParedFA5.obj");
	Model ParedLD((char*)"ModelosFachada/ParedLD.obj");
	Model ParedLF((char*)"ModelosFachada/ParedLF.obj");
	Model ParedLF1((char*)"ModelosFachada/ParedLF1.obj");
	Model ParedLI((char*)"ModelosFachada/ParedLI.obj");
	Model ParedT((char*)"ModelosFachada/ParedT.obj");
	Model PisoZoo((char*)"ModelosFachada/Piso.obj");
	Model Pasto((char*)"ModelosFachada/PisoPasto.obj");
	Model PlataformaZ((char*)"ModelosFachada/PlataformaZ.obj");
	Model Reloj((char*)"ModelosFachada/Reloj.obj");
	Model Soportes((char*)"ModelosFachada/Soportes.obj");
	Model TechoF((char*)"ModelosFachada/TechoF.obj");
	Model TorreI((char*)"ModelosFachada/TorreI.obj");
	Model TorreS((char*)"ModelosFachada/TorreS.obj");
	Model Trabes((char*)"Modelos/Trabes.obj");
	Model VidrioF((char*)"ModelosFachada/VidrioF.obj");

	//Modelos Acuario
	Model AguaA((char*)"ModelosAcuario/AguaA.obj");
	Model Algas((char*)"ModelosAcuario/Algas.obj");
	Model ArenaA((char*)"ModelosAcuario/ArenaA.obj");
	Model CartelT((char*)"ModelosAcuario/CartelT.obj");
	Model Coral((char*)"ModelosAcuario/Coral.obj");
	Model Hongo1((char*)"ModelosAcuario/Hongo1.obj");
	Model Hongo2((char*)"ModelosAcuario/Hongo2.obj");
	Model Hongo3((char*)"ModelosAcuario/Hongo3.obj");
	Model Hongo4((char*)"ModelosAcuario/Hongo4.obj");
	Model Hongo5((char*)"ModelosAcuario/Hongo5.obj");
	Model Hongo6((char*)"ModelosAcuario/Hongo6.obj");
	Model ParedA((char*)"ModelosAcuario/ParedA.obj");
	Model RocasA((char*)"ModelosAcuario/RocasA.obj");
	Model VidrioA((char*)"ModelosAcuario/VidrioA.obj");

	// Modelos Polar
	Model AguaP((char*)"ModelosPolar/AguaP.obj");
	Model BarandalP((char*)"ModelosPolar/BarandalP.obj");
	Model CartelP((char*)"ModelosPolar/CartelP.obj");
	Model EstanqueP((char*)"ModelosPolar/EstanqueP.obj");
	Model Nieve((char*)"ModelosPolar/Nieve.obj");
	Model PlataformaP((char*)"ModelosPolar/PlataformaP.obj");

	// Modelos Sabana
	Model AguaJ((char*)"ModelosSabana/AguaJ.obj");
	Model BarandalJ((char*)"ModelosSabana/BarandalJ.obj");
	Model BaseJ((char*)"ModelosSabana/BaseJ.obj");
	Model CartelJ((char*)"ModelosSabana/CartelJ.obj");
	Model PastoSeco((char*)"ModelosSabana/PastoSeco.obj");
	Model RocaJ1((char*)"ModelosSabana/RocaJ1.obj");
	Model RocaJ2((char*)"ModelosSabana/RocaJ2.obj");
	Model RocaJ3((char*)"ModelosSabana/RocaJ3.obj");
	Model RocaJ4((char*)"ModelosSabana/RocaJ4.obj");

	// Modelos Selva
	Model BarandalTg((char*)"ModelosSelva/BarandalTg.obj");
	Model CartelTg((char*)"ModelosSelva/CartelTg.obj");
	Model CasaTg((char*)"ModelosSelva/CasaTg.obj");
	Model Palma1((char*)"ModelosSelva/Palma1.obj");
	Model Palma2((char*)"ModelosSelva/Palma2.obj");
	Model Palma3((char*)"ModelosSelva/Palma3.obj");
	Model ParedTg((char*)"ModelosSelva/ParedTg.obj");
	Model ParedTg1((char*)"ModelosSelva/ParedTg1.obj");
	Model ParedTg2((char*)"ModelosSelva/ParedTg2.obj");
	Model PisoTg((char*)"ModelosSelva/PisoTg.obj");
	Model TechoTg((char*)"ModelosSelva/TechoTg.obj");
	Model TroncoP1((char*)"ModelosSelva/TroncoP1.obj");
	Model TroncoP2((char*)"ModelosSelva/TroncoP2.obj");
	Model TroncoP3((char*)"ModelosSelva/TroncoP3.obj");
	Model Volcan((char*)"ModelosSelva/Volcan.obj");

	//KeyFrames
	for (int i = 0; i < MAX_FRAMES; i++) //Incremento de cada keyframe
	{
		KeyFrame[i].dogPosX = 0;
		KeyFrame[i].dogPosY = 0;
		KeyFrame[i].dogPosZ = 0;
		KeyFrame[i].incX = 0;
		KeyFrame[i].incY = 0;
		KeyFrame[i].incZ = 0;
		KeyFrame[i].rotDog = 0;
		KeyFrame[i].rotDogInc = 0;

		KeyFrame[i].head = 0;
		KeyFrame[i].headInc = 0;

		KeyFrame[i].Flegs = 0;
		KeyFrame[i].FlegsInc = 0;
		KeyFrame[i].Rlegs = 0;
		KeyFrame[i].RlegsInc = 0;
		KeyFrame[i].tail = 0;
		KeyFrame[i].tailInc = 0;
	}


	// First, set the container's VAO (and VBO)
	GLuint VBO, VAO, EBO;
	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);


	glBindVertexArray(VAO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);


	// Position attribute
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), (GLvoid*)0);
	glEnableVertexAttribArray(0);
	// normal attribute
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(1);

	// Set texture units
	lightingShader.Use();
	glUniform1i(glGetUniformLocation(lightingShader.Program, "material.difuse"), 0);
	glUniform1i(glGetUniformLocation(lightingShader.Program, "material.specular"), 1);


	glm::mat4 projection = glm::perspective(camera.GetZoom(), (GLfloat)SCREEN_WIDTH / (GLfloat)SCREEN_HEIGHT, 0.1f, 100.0f);


	// TEXTURAS 

	GLuint texAgua = SOIL_load_OGL_texture("Texturas/Agua.jpg", SOIL_LOAD_AUTO, SOIL_CREATE_NEW_ID, SOIL_FLAG_INVERT_Y);
	GLuint texAlga = SOIL_load_OGL_texture("Texturas/Alga.jpg", SOIL_LOAD_AUTO, SOIL_CREATE_NEW_ID, SOIL_FLAG_INVERT_Y);
	GLuint texArena = SOIL_load_OGL_texture("Texturas/Arena.jpg", SOIL_LOAD_AUTO, SOIL_CREATE_NEW_ID, SOIL_FLAG_INVERT_Y);
	GLuint texBambu = SOIL_load_OGL_texture("Texturas/Bambu.jpg", SOIL_LOAD_AUTO, SOIL_CREATE_NEW_ID, SOIL_FLAG_INVERT_Y);
	GLuint texCoral = SOIL_load_OGL_texture("Texturas/Coral.jpg", SOIL_LOAD_AUTO, SOIL_CREATE_NEW_ID, SOIL_FLAG_INVERT_Y);
	GLuint texEstanque = SOIL_load_OGL_texture("Texturas/Estanque.jpg", SOIL_LOAD_AUTO, SOIL_CREATE_NEW_ID, SOIL_FLAG_INVERT_Y);
	GLuint texHongo = SOIL_load_OGL_texture("Texturas/Hongo.jpg", SOIL_LOAD_AUTO, SOIL_CREATE_NEW_ID, SOIL_FLAG_INVERT_Y);
	GLuint texLadrillo = SOIL_load_OGL_texture("Texturas/Ladrillo.jpg", SOIL_LOAD_AUTO, SOIL_CREATE_NEW_ID, SOIL_FLAG_INVERT_Y);
	GLuint texLadrillo1 = SOIL_load_OGL_texture("Texturas/Ladrillo1.jpg", SOIL_LOAD_AUTO, SOIL_CREATE_NEW_ID, SOIL_FLAG_INVERT_Y);
	GLuint texMadera = SOIL_load_OGL_texture("Texturas/Madera.jpg", SOIL_LOAD_AUTO, SOIL_CREATE_NEW_ID, SOIL_FLAG_INVERT_Y);
	GLuint texMetalBlanco = SOIL_load_OGL_texture("Texturas/Metal blanco.jpg", SOIL_LOAD_AUTO, SOIL_CREATE_NEW_ID, SOIL_FLAG_INVERT_Y);
	GLuint texMetalDorado = SOIL_load_OGL_texture("Texturas/Metal dorado.jpg", SOIL_LOAD_AUTO, SOIL_CREATE_NEW_ID, SOIL_FLAG_INVERT_Y);
	GLuint texMetalNegro = SOIL_load_OGL_texture("Texturas/Metal negro.jpg", SOIL_LOAD_AUTO, SOIL_CREATE_NEW_ID, SOIL_FLAG_INVERT_Y);
	GLuint texMetalVerde = SOIL_load_OGL_texture("Texturas/Metal verde.jpg", SOIL_LOAD_AUTO, SOIL_CREATE_NEW_ID, SOIL_FLAG_INVERT_Y);
	GLuint texNieve = SOIL_load_OGL_texture("Texturas/Nieve.jpg", SOIL_LOAD_AUTO, SOIL_CREATE_NEW_ID, SOIL_FLAG_INVERT_Y);
	GLuint texPalma = SOIL_load_OGL_texture("Texturas/Palma.jpg", SOIL_LOAD_AUTO, SOIL_CREATE_NEW_ID, SOIL_FLAG_INVERT_Y);
	GLuint texPasto = SOIL_load_OGL_texture("Texturas/Pasto.jpg", SOIL_LOAD_AUTO, SOIL_CREATE_NEW_ID, SOIL_FLAG_INVERT_Y);
	GLuint texPastoSeco = SOIL_load_OGL_texture("Texturas/PastoSeco.jpg", SOIL_LOAD_AUTO, SOIL_CREATE_NEW_ID, SOIL_FLAG_INVERT_Y);
	GLuint texPastoSelva = SOIL_load_OGL_texture("Texturas/PastoSelva.jpg", SOIL_LOAD_AUTO, SOIL_CREATE_NEW_ID, SOIL_FLAG_INVERT_Y);
	GLuint texPiedra = SOIL_load_OGL_texture("Texturas/Piedra.jpg", SOIL_LOAD_AUTO, SOIL_CREATE_NEW_ID, SOIL_FLAG_INVERT_Y);
	GLuint texPiso = SOIL_load_OGL_texture("Texturas/Piso.jpg", SOIL_LOAD_AUTO, SOIL_CREATE_NEW_ID, SOIL_FLAG_INVERT_Y);
	GLuint texPlastico = SOIL_load_OGL_texture("Texturas/Plastico.jpg", SOIL_LOAD_AUTO, SOIL_CREATE_NEW_ID, SOIL_FLAG_INVERT_Y);
	GLuint texTronco = SOIL_load_OGL_texture("Texturas/Tronco.jpg", SOIL_LOAD_AUTO, SOIL_CREATE_NEW_ID, SOIL_FLAG_INVERT_Y);
	GLuint texVidrio = SOIL_load_OGL_texture("Texturas/Vidrio.jpg", SOIL_LOAD_AUTO, SOIL_CREATE_NEW_ID, SOIL_FLAG_INVERT_Y);
	GLuint texVolcan = SOIL_load_OGL_texture("Texturas/Volcan.jpg", SOIL_LOAD_AUTO, SOIL_CREATE_NEW_ID, SOIL_FLAG_INVERT_Y);


	// OpenGL options
	glEnable(GL_DEPTH_TEST);

	// Game loop
	while (!glfwWindowShouldClose(window))
	{

		// Calculate deltatime of current frame
		GLfloat currentFrame = glfwGetTime();
		deltaTime = currentFrame - lastFrame;
		lastFrame = currentFrame;

		// Check if any events have been activiated (key pressed, mouse moved etc.) and call corresponding response functions
		glfwPollEvents();
		DoMovement();
		Animation();

		// Clear the colorbuffer
		glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);



		glm::mat4 modelTemp = glm::mat4(1.0f); //Temp



		// Use cooresponding shader when setting uniforms/drawing objects
		lightingShader.Use();

		glUniform1i(glGetUniformLocation(lightingShader.Program, "diffuse"), 0);
		//glUniform1i(glGetUniformLocation(lightingShader.Program, "specular"),1);

		GLint viewPosLoc = glGetUniformLocation(lightingShader.Program, "viewPos");
		glUniform3f(viewPosLoc, camera.GetPosition().x, camera.GetPosition().y, camera.GetPosition().z);


		// Directional light
		glUniform3f(glGetUniformLocation(lightingShader.Program, "dirLight.direction"), 0.0f, -0.5f, -0.3f);
		glUniform3f(glGetUniformLocation(lightingShader.Program, "dirLight.ambient"), 0.5f, 0.5f, 0.5f);
		glUniform3f(glGetUniformLocation(lightingShader.Program, "dirLight.diffuse"), 1.0f, 1.0f, 1.0f);
		glUniform3f(glGetUniformLocation(lightingShader.Program, "dirLight.specular"), 0.3f, 0.3f, 0.3f);


		// Point light 1
		glm::vec3 lightColor;
		lightColor.x = abs(sin(glfwGetTime() * Light1.x));
		lightColor.y = abs(sin(glfwGetTime() * Light1.y));
		lightColor.z = sin(glfwGetTime() * Light1.z);


		glUniform3f(glGetUniformLocation(lightingShader.Program, "pointLights[0].position"), pointLightPositions[0].x, pointLightPositions[0].y, pointLightPositions[0].z);
		glUniform3f(glGetUniformLocation(lightingShader.Program, "pointLights[0].ambient"), lightColor.x, lightColor.y, lightColor.z);
		glUniform3f(glGetUniformLocation(lightingShader.Program, "pointLights[0].diffuse"), lightColor.x, lightColor.y, lightColor.z);
		glUniform3f(glGetUniformLocation(lightingShader.Program, "pointLights[0].specular"), 1.0f, 0.2f, 0.2f);
		glUniform1f(glGetUniformLocation(lightingShader.Program, "pointLights[0].constant"), 1.0f);
		glUniform1f(glGetUniformLocation(lightingShader.Program, "pointLights[0].linear"), 0.045f);
		glUniform1f(glGetUniformLocation(lightingShader.Program, "pointLights[0].quadratic"), 0.075f);


		// SpotLight
		glUniform3f(glGetUniformLocation(lightingShader.Program, "spotLight.position"), camera.GetPosition().x, camera.GetPosition().y, camera.GetPosition().z);
		glUniform3f(glGetUniformLocation(lightingShader.Program, "spotLight.direction"), camera.GetFront().x, camera.GetFront().y, camera.GetFront().z);
		glUniform3f(glGetUniformLocation(lightingShader.Program, "spotLight.ambient"), 0.2f, 0.2f, 0.8f);
		glUniform3f(glGetUniformLocation(lightingShader.Program, "spotLight.diffuse"), 0.2f, 0.2f, 0.8f);
		glUniform3f(glGetUniformLocation(lightingShader.Program, "spotLight.specular"), 0.0f, 0.0f, 0.0f);
		glUniform1f(glGetUniformLocation(lightingShader.Program, "spotLight.constant"), 1.0f);
		glUniform1f(glGetUniformLocation(lightingShader.Program, "spotLight.linear"), 0.3f);
		glUniform1f(glGetUniformLocation(lightingShader.Program, "spotLight.quadratic"), 0.7f);
		glUniform1f(glGetUniformLocation(lightingShader.Program, "spotLight.cutOff"), glm::cos(glm::radians(12.0f)));
		glUniform1f(glGetUniformLocation(lightingShader.Program, "spotLight.outerCutOff"), glm::cos(glm::radians(18.0f)));


		// Set material properties
		glUniform1f(glGetUniformLocation(lightingShader.Program, "material.shininess"), 5.0f);

		// Create camera transformations
		glm::mat4 view;
		view = camera.GetViewMatrix();

		// Get the uniform locations
		GLint modelLoc = glGetUniformLocation(lightingShader.Program, "model");
		GLint viewLoc = glGetUniformLocation(lightingShader.Program, "view");
		GLint projLoc = glGetUniformLocation(lightingShader.Program, "projection");

		// Pass the matrices to the shader
		glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
		glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));


		glm::mat4 model(1);



		//Carga de modelo 
		//view = camera.GetViewMatrix();
		//model = glm::mat4(1);
		//glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
		//Piso.Draw(lightingShader);

		//model = glm::mat4(1);
		//glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
		//glUniform1i(glGetUniformLocation(lightingShader.Program, "transparency"), 0);
		////Body
		//modelTemp = model = glm::translate(model, glm::vec3(dogPosX, dogPosY, dogPosZ));
		//modelTemp = model = glm::rotate(model, glm::radians(rotDog), glm::vec3(0.0f, 1.0f, 0.0f));
		//glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
		//DogBody.Draw(lightingShader);
		////Head
		//model = modelTemp;
		//model = glm::translate(model, glm::vec3(0.0f, 0.093f, 0.208f));
		//model = glm::rotate(model, glm::radians(head), glm::vec3(0.0f, 0.0f, 1.0f));
		//glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
		//HeadDog.Draw(lightingShader);
		////Tail 
		//model = modelTemp;
		//model = glm::translate(model, glm::vec3(0.0f, 0.026f, -0.288f));
		//model = glm::rotate(model, glm::radians(tail), glm::vec3(0.0f, 0.0f, -1.0f));
		//glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
		//DogTail.Draw(lightingShader);
		////Front Left Leg
		//model = modelTemp;
		//model = glm::translate(model, glm::vec3(0.112f, -0.044f, 0.074f));
		//model = glm::rotate(model, glm::radians(FLegs), glm::vec3(-1.0f, 0.0f, 0.0f));
		//glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
		//F_LeftLeg.Draw(lightingShader);
		////Front Right Leg
		//model = modelTemp;
		//model = glm::translate(model, glm::vec3(-0.111f, -0.055f, 0.074f));
		//model = glm::rotate(model, glm::radians(FLegs), glm::vec3(1.0f, 0.0f, 0.0f));
		//glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
		//F_RightLeg.Draw(lightingShader);
		////Back Left Leg
		//model = modelTemp;
		//model = glm::translate(model, glm::vec3(0.082f, -0.046, -0.218));
		//model = glm::rotate(model, glm::radians(RLegs), glm::vec3(1.0f, 0.0f, 0.0f));
		//glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
		//B_LeftLeg.Draw(lightingShader);
		////Back Right Leg
		//model = modelTemp;
		//model = glm::translate(model, glm::vec3(-0.083f, -0.057f, -0.231f));
		//model = glm::rotate(model, glm::radians(RLegs), glm::vec3(-1.0f, 0.0f, 0.0f));
		//glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
		//B_RightLeg.Draw(lightingShader);

		//model = glm::mat4(1);
		//glEnable(GL_BLEND);//Avtiva la funcionalidad para trabajar el canal alfa
		//glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		//glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
		//glUniform1i(glGetUniformLocation(lightingShader.Program, "transparency"), 1);
		//model = glm::rotate(model, glm::radians(rotBall), glm::vec3(0.0f, 1.0f, 0.0f));
		//glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
		//Ball.Draw(lightingShader);
		//glDisable(GL_BLEND);  //Desactiva el canal alfa 


		// Dibujar Zoologico

		lightingShader.Use();
		glUniform1i(glGetUniformLocation(lightingShader.Program, "material.diffuse"), 0);
		glUniform1f(glGetUniformLocation(lightingShader.Program, "material.shininess"), 32.0f);


		auto drawModelWithTexture = [&](Model& model, GLuint texture, glm::vec3 position = glm::vec3(0.0f), glm::vec3 scale = glm::vec3(1.0f)) {
			glm::mat4 modelMat = glm::mat4(1.0f);
			modelMat = glm::translate(modelMat, position);
			modelMat = glm::scale(modelMat, scale);
			glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(modelMat));

			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, texture);
			glUniform1i(glGetUniformLocation(lightingShader.Program, "material.diffuse"), 0);

			model.Draw(lightingShader);
			};

		// Alga
		drawModelWithTexture(Algas, texAlga);

		// Arena
		glBindTexture(GL_TEXTURE_2D, texArena);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

		lightingShader.setVec2("textureScale", glm::vec2(3.0, 3.0f));
		drawModelWithTexture(ArenaA, texArena);

		// Bambu
		glBindTexture(GL_TEXTURE_2D, texBambu);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

		lightingShader.setVec2("textureScale", glm::vec2(3.0f, 3.0f));
		drawModelWithTexture(CasaTg, texBambu);

		//Coral
		drawModelWithTexture(Coral, texCoral);

		//Estamque
		drawModelWithTexture(ParedA, texEstanque);
		glBindTexture(GL_TEXTURE_2D, texEstanque);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

		lightingShader.setVec2("textureScale", glm::vec2(3.0f, 3.0f));
		drawModelWithTexture(CasaTg, texBambu);

		// Hongo
		drawModelWithTexture(Hongo1, texHongo);
		drawModelWithTexture(Hongo2, texHongo);
		drawModelWithTexture(Hongo3, texHongo);
		drawModelWithTexture(Hongo4, texHongo);
		drawModelWithTexture(Hongo5, texHongo);
		drawModelWithTexture(Hongo6, texHongo);

		// Ladrillo (paredes, techo, torre, trabes normales, etc.)
		glBindTexture(GL_TEXTURE_2D, texLadrillo1);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

		lightingShader.setVec2("textureScale", glm::vec2(1.5f, 1.5f));
		drawModelWithTexture(ParedF1, texLadrillo1);

		lightingShader.setVec2("textureScale", glm::vec2(10.0f, 10.0f));
		drawModelWithTexture(ParedLD, texLadrillo1);
		drawModelWithTexture(ParedLI, texLadrillo1);

		lightingShader.setVec2("textureScale", glm::vec2(7.0f, 7.0f));
		drawModelWithTexture(ParedT, texLadrillo1);

		lightingShader.setVec2("textureScale", glm::vec2(4.0f, 4.0f));
		drawModelWithTexture(ParedF2, texLadrillo1);

		lightingShader.setVec2("textureScale", glm::vec2(3.0f, 3.0f));
		drawModelWithTexture(TechoF, texLadrillo1);

		lightingShader.setVec2("textureScale", glm::vec2(270.0f, 20.0f));
		drawModelWithTexture(Trabes, texLadrillo1);

		lightingShader.setVec2("textureScale", glm::vec2(3.0f, 3.0f));
		drawModelWithTexture(BarandalTg, texLadrillo1);

		lightingShader.setVec2("textureScale", glm::vec2(1.5f, 1.5f));
		drawModelWithTexture(ParedTg2, texLadrillo1);

		lightingShader.setVec2("textureScale", glm::vec2(3.5f, 3.5f));
		drawModelWithTexture(ParedTg, texLadrillo1);

		glBindTexture(GL_TEXTURE_2D, texLadrillo);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

		lightingShader.setVec2("textureScale", glm::vec2(2.0, 2.0f));
		drawModelWithTexture(ParedFA1, texLadrillo);

		lightingShader.setVec2("textureScale", glm::vec2(1.5f, 1.5f));
		drawModelWithTexture(ParedFA2, texLadrillo);

		lightingShader.setVec2("textureScale", glm::vec2(1.5f, 1.5f));
		drawModelWithTexture(ParedFA3, texLadrillo);

		lightingShader.setVec2("textureScale", glm::vec2(1.5f, 1.5f));
		drawModelWithTexture(ParedFA4, texLadrillo);

		lightingShader.setVec2("textureScale", glm::vec2(1.5f, 1.5f));
		drawModelWithTexture(ParedFA5, texLadrillo);

		lightingShader.setVec2("textureScale", glm::vec2(1.5f, 1.5f));
		drawModelWithTexture(ParedLF1, texLadrillo);

		lightingShader.setVec2("textureScale", glm::vec2(1.5f, 1.5f));
		drawModelWithTexture(ParedLF, texLadrillo);

		lightingShader.setVec2("textureScale", glm::vec2(2.0f, 2.0f));
		drawModelWithTexture(TorreI, texLadrillo);

		lightingShader.setVec2("textureScale", glm::vec2(2.0f, 2.0f));
		drawModelWithTexture(TorreS, texLadrillo);

		lightingShader.setVec2("textureScale", glm::vec2(1.0f, 1.0f));
		drawModelWithTexture(ParedTg1, texLadrillo);

		//// Madera
		drawModelWithTexture(Bancas, texMadera);
		drawModelWithTexture(BarandalJ, texMadera);

		// Metal blanco
		drawModelWithTexture(BarandalZ, texMetalBlanco);

		// Metal dorado
		drawModelWithTexture(Campana, texMetalDorado);

		// Metal Negro
		drawModelWithTexture(Letras, texMetalNegro);
		drawModelWithTexture(Soportes, texMetalNegro);

		////Metal Verde
		drawModelWithTexture(BarandalP, texMetalVerde);
		drawModelWithTexture(Faros, texMetalVerde);
		drawModelWithTexture(CartelT, texMetalVerde);
		drawModelWithTexture(CartelP, texMetalVerde);
		drawModelWithTexture(CartelJ, texMetalVerde);
		drawModelWithTexture(CartelTg, texMetalVerde);
		drawModelWithTexture(TechoTg, texMetalVerde);

		// Nieve
		//drawModelWithTexture(Nieve, texNieve);
		drawModelWithTexture(PlataformaP, texNieve);

		// Palma
		drawModelWithTexture(Palma1, texPalma);
		drawModelWithTexture(Palma2, texPalma);
		drawModelWithTexture(Palma3, texPalma);

		// Pasto
		glBindTexture(GL_TEXTURE_2D, texPasto);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

		lightingShader.setVec2("textureScale", glm::vec2(8.0, 8.0f));
		drawModelWithTexture(Pasto, texPasto);

		// Pasto Seco
		glBindTexture(GL_TEXTURE_2D, texPastoSeco);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

		lightingShader.setVec2("textureScale", glm::vec2(5.0, 5.0f));
		drawModelWithTexture(PastoSeco, texPastoSeco);

		// Pasto Selva
		glBindTexture(GL_TEXTURE_2D, texPastoSelva);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

		lightingShader.setVec2("textureScale", glm::vec2(4.0, 4.0f));
		drawModelWithTexture(PisoTg, texPastoSelva);

		// Piedra

		glBindTexture(GL_TEXTURE_2D, texPiedra);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

		lightingShader.setVec2("textureScale", glm::vec2(1.5, 1.5f));
		drawModelWithTexture(Arcos, texPiedra);
		drawModelWithTexture(PlataformaZ, texPiedra);
		drawModelWithTexture(RocasA, texPiedra);
		drawModelWithTexture(EstanqueP, texPiedra);
		drawModelWithTexture(BaseJ, texPiedra);
		drawModelWithTexture(RocaJ1, texPiedra);
		drawModelWithTexture(RocaJ2, texPiedra);
		drawModelWithTexture(RocaJ3, texPiedra);
		drawModelWithTexture(RocaJ4, texPiedra);
		drawModelWithTexture(Fuente, texPiedra);

		// Piso
		drawModelWithTexture(PisoZoo, texPiso);

		// Plastico
		drawModelWithTexture(Reloj, texPlastico);

		// Tronco
		drawModelWithTexture(TroncoP1, texTronco);
		drawModelWithTexture(TroncoP2, texTronco);
		drawModelWithTexture(TroncoP3, texTronco);

		// Volcan
		drawModelWithTexture(Volcan, texVolcan);

		// Vidrio
		glUniform1i(glGetUniformLocation(lightingShader.Program, "transparency"), 1);
		drawModelWithTexture(VidrioA, texVidrio);
		drawModelWithTexture(VidrioF, texVidrio);

		// Agua
		drawModelWithTexture(AguaA, texAgua);
		drawModelWithTexture(AguaP, texAgua);
		drawModelWithTexture(AguaJ, texAgua);
		drawModelWithTexture(AguaF, texAgua);

		glDisable(GL_BLEND);

		glBindVertexArray(0);
	

		// Also draw the lamp object, again binding the appropriate shader
		lampShader.Use();
		// Get location objects for the matrices on the lamp shader (these could be different on a different shader)
		modelLoc = glGetUniformLocation(lampShader.Program, "model");
		viewLoc = glGetUniformLocation(lampShader.Program, "view");
		projLoc = glGetUniformLocation(lampShader.Program, "projection");

		// Set matrices
		glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
		glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));
		model = glm::mat4(1);
		model = glm::translate(model, lightPos);
		model = glm::scale(model, glm::vec3(0.2f)); // Make it a smaller cube
		glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
		// Draw the light object (using light's vertex attributes)
		
		model = glm::mat4(1);
		model = glm::translate(model, pointLightPositions[0]);
		model = glm::scale(model, glm::vec3(0.2f)); // Make it a smaller cube
		glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
		glBindVertexArray(VAO);
		glDrawArrays(GL_TRIANGLES, 0, 36);
		
		glBindVertexArray(0);

		
		// Swap the screen buffers
		glfwSwapBuffers(window);
	}

	
	

	// Terminate GLFW, clearing any resources allocated by GLFW.
	glfwTerminate();



	return 0;
}

// Moves/alters the camera positions based on user input
void DoMovement()
{
	//Dog Controls
	
	if (keys[GLFW_KEY_Z])
	{

		head += 0.25f;

	}

	if (keys[GLFW_KEY_X])
	{

		head -= 0.25f;

	}

	if (keys[GLFW_KEY_P])
	{

		FLegs += 0.25f;

	}

	if (keys[GLFW_KEY_O])
	{

		FLegs -= 0.25f;

	}

	if (keys[GLFW_KEY_N])
	{

		RLegs += 0.25f;

	}

	if (keys[GLFW_KEY_M])
	{

		RLegs -= 0.25f;

	}

	if (keys[GLFW_KEY_B])
	{

		tail += 0.25f;

	}

	if (keys[GLFW_KEY_V])
	{

		tail -= 0.25f;

	}


	if (keys[GLFW_KEY_2])
	{
		
			rotDog += 1.0f;

	}

	if (keys[GLFW_KEY_3])
	{
		
			rotDog -= 1.0f;

	}
			
	if (keys[GLFW_KEY_H])
	{
		dogPosZ += 0.01;
	}

	if (keys[GLFW_KEY_Y])
	{
		dogPosZ -= 0.01;
	}

	if (keys[GLFW_KEY_G])
	{
		dogPosX -= 0.01;
	}

	if (keys[GLFW_KEY_J])
	{
		dogPosX += 0.01;
	}

	// Camera controls
	if (keys[GLFW_KEY_W] || keys[GLFW_KEY_UP])
	{
		camera.ProcessKeyboard(FORWARD, deltaTime);

	}

	if (keys[GLFW_KEY_S] || keys[GLFW_KEY_DOWN])
	{
		camera.ProcessKeyboard(BACKWARD, deltaTime);


	}

	if (keys[GLFW_KEY_A] || keys[GLFW_KEY_LEFT])
	{
		camera.ProcessKeyboard(LEFT, deltaTime);


	}

	if (keys[GLFW_KEY_D] || keys[GLFW_KEY_RIGHT])
	{
		camera.ProcessKeyboard(RIGHT, deltaTime);


	}

	if (keys[GLFW_KEY_T])
	{
		pointLightPositions[0].x += 0.01f;
	}
	if (keys[GLFW_KEY_G])
	{
		pointLightPositions[0].x -= 0.01f;
	}

	if (keys[GLFW_KEY_Y])
	{
		pointLightPositions[0].y += 0.01f;
	}

	if (keys[GLFW_KEY_H])
	{
		pointLightPositions[0].y -= 0.01f;
	}
	if (keys[GLFW_KEY_U])
	{
		pointLightPositions[0].z -= 0.1f;
	}
	if (keys[GLFW_KEY_J])
	{
		pointLightPositions[0].z += 0.01f;
	}
	
}

// Is called whenever a key is pressed/released via GLFW
void KeyCallback(GLFWwindow *window, int key, int scancode, int action, int mode)
{

	if (keys[GLFW_KEY_L])
	{
		if (play == false && (FrameIndex > 1))
		{

			resetElements();
			//First Interpolation				
			interpolation();

			play = true;
			playIndex = 0;
			i_curr_steps = 0;
		}
		else
		{
			play = false;
		}

	}

	if (keys[GLFW_KEY_K])
	{
		if (FrameIndex < MAX_FRAMES)
		{
			saveFrame();
		}

	}



	if (GLFW_KEY_ESCAPE == key && GLFW_PRESS == action)
	{
		glfwSetWindowShouldClose(window, GL_TRUE);
	}

	if (key >= 0 && key < 1024)
	{
		if (action == GLFW_PRESS)
		{
			keys[key] = true;
		}
		else if (action == GLFW_RELEASE)
		{
			keys[key] = false;
		}
	}

	if (keys[GLFW_KEY_SPACE])
	{
		active = !active;
		if (active)
		{
			Light1 = glm::vec3(0.2f, 0.8f, 1.0f);
			
		}
		else
		{
			Light1 = glm::vec3(0);//Cuado es solo un valor en los 3 vectores pueden dejar solo una componente
		}
	}
	
	
}
void Animation() {

	if (play)
	{
		if (i_curr_steps >= i_max_steps) //end of animation between frames?
		{
			playIndex++;
			if (playIndex > FrameIndex - 2)	//end of total animation?
			{
				printf("termina anim\n");
				playIndex = 0;
				play = false;
			}
			else //Next frame interpolations
			{
				i_curr_steps = 0; //Reset counter
				//Interpolation
				interpolation();
			}
		}
		else
		{
			//Draw animation
			dogPosX += KeyFrame[playIndex].incX;
			dogPosY += KeyFrame[playIndex].incY;
			dogPosZ += KeyFrame[playIndex].incZ;

			rotDog += KeyFrame[playIndex].rotDogInc;

			head += KeyFrame[playIndex].headInc;

			FLegs += KeyFrame[playIndex].FlegsInc;
			RLegs += KeyFrame[playIndex].RlegsInc;
			tail += KeyFrame[playIndex].tailInc;

			i_curr_steps++;
		}

	}
	
}

void MouseCallback(GLFWwindow *window, double xPos, double yPos)
{
	if (firstMouse)
	{
		lastX = xPos;
		lastY = yPos;
		firstMouse = false;
	}

	GLfloat xOffset = xPos - lastX;
	GLfloat yOffset = lastY - yPos;  // Reversed since y-coordinates go from bottom to left

	lastX = xPos;
	lastY = yPos;

	camera.ProcessMouseMovement(xOffset, yOffset);
}