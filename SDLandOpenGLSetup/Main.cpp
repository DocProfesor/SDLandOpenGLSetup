// SDLTutorial_Setup.cpp : Defines the entry point for the console application.
//

#include <iostream>
#include "SDL.h"
#include <vector>
#include "GL\glew.h"
#include "GL\glut.h"
#include <GL\GLU.h>
#include "SDL_image.h"
#include <string>
#include <SDL_opengl.h>
#include "SDL_thread.h"

using namespace std;

#undef main

#define WINDOW_WIDTH 1200
#define WINDOW_HEIGHT 800
#define FPS 60

//Starts up SDL, creates window, and initializes OpenGL
bool init();

//Initializes rendering program and clear color
bool initGL();

//Input handler
void handleKeys(unsigned char key, int x, int y);

//Per frame update
void update();

//Renders quad to the screen
void render();

//Frees media and shuts down SDL
void close();

//Shader loading utility programs
void printProgramLog(GLuint program);
void printShaderLog(GLuint shader);

//The window we'll be rendering to
SDL_Window* gWindow = NULL;

//OpenGL context
SDL_GLContext gContext;

//Render flag
bool gRenderQuad = true;

//Graphics program
GLuint gProgramID = 0;
GLuint gVertexPos2DLocation = -1;
GLuint gVBO = 0;
GLuint gIBO = 0;

//Test commit

enum LAYER_TYPE {
BACKGROUND, FOREGROUND, TILE_MAP, COLLISION_MAP
};

void cap_framerate(Uint32 starting_tick) {
	Uint32 delta;

	delta = SDL_GetTicks() - starting_tick;
	if ((1000 / FPS) > delta) {
		SDL_Delay(1000 / FPS - delta);
	}
}

int threadFunction(void* data)
{
	//Print incoming data
	printf("Running thread with value = %d\n", (int)data);
	SDL_Delay(10000);
	printf("Closing thread with value = %d\n", (int)data);

	return 0;
}

void update() {

}

class Sprite {
	protected:
		SDL_Surface *image;
		SDL_Rect rect;

		int origin_x=0, origin_y=0;

	public:
		int inc = 0;

		Sprite(Uint32 color, int x, int y, int w = 48, int h = 64) {
			image = SDL_CreateRGBSurface(0, w, h, 32, 0, 0, 0, 0);
			
			SDL_FillRect(image, NULL, color);

			rect = image->clip_rect;
			rect.x = x - origin_x;
			rect.y = y - origin_y;
		}
		
		void update() {
			//Insert update code below vvvv
			rect.x += inc;

		}

		void draw(SDL_Surface *destination) {
			SDL_BlitSurface(image, NULL, destination, &rect);
		}

		SDL_Surface* get_image() const {
			return image;
		}

		bool operator==(const Sprite &other) const {
			return (image == other.get_image());
		}
};

class SpriteGroup {
private:
	vector<Sprite*> sprites;
	int sprites_size=0;

public:
	LAYER_TYPE type = BACKGROUND;

	SpriteGroup copy() {
		SpriteGroup new_group;

		for (int i = 0; i < sprites_size; i++) {
			new_group.add(sprites[i]);
		}

		return new_group;
	}

	void add(Sprite *sprite) {
		sprites.push_back(sprite);

		sprites_size = sprites.size();
	}

	void remove(Sprite sprite_object) {
		for (int i = 0; i < sprites_size; i++) {
			if (*sprites[i] == sprite_object) {
				sprites.erase(sprites.begin() + i);

				sprites_size = sprites.size();
			}
		}
	}

	bool has(Sprite sprite_object) {
		for (int i = 0; i < sprites_size; i++) {
			if (*sprites[i] == sprite_object) {
				return true;
			}
		}

		return false;
	}

	void update() {
		if (!sprites.empty()) {
			for (int i = 0; i < sprites_size; i++) {
				sprites[i]->update();
			}
		}
	}

	void draw(SDL_Surface *destination) {
		if (!sprites.empty()) {
			for (int i = 0; i < sprites_size; i++) {
				sprites[i]->draw(destination);
			}
		}
	}

	void empty() {
		sprites.clear();
		sprites_size = sprites.size();
	}

	int size() {
		return sprites_size;
	}

	vector <Sprite*> GetSprites() {
		return sprites;
	}

	friend bool operator== (SpriteGroup & a, SpriteGroup & b) {
		return (&a == &b);
	};

};

//bool operator==(SpriteGroup &a, SpriteGroup &b) {
//	return (&a == &b);
//}

class Block : public Sprite {
public:
	Block(Uint32 color, int x, int y, int w = 48, int h = 64) : Sprite(color, x, y, w, h) {
		update_properties();
	}

	void update_properties() {
		origin_x = 0;
		origin_y = 0;

		set_position(rect.x, rect.y);
	}

	void set_position(int x, int y) {
		//rect.x = x - origin_x;
		//rect.y = y - origin_y;
		rect.x = x;
		rect.y = y;
	}

	void set_image(const char filename[] = NULL) {
		if (filename != NULL) {
			SDL_Surface *loaded_image = NULL;

			//Se usa IMG_Load en vez de BMP_Load para poder utilizar archivos .png
			loaded_image = IMG_Load(filename);

			if (loaded_image != NULL) {
				image = loaded_image;

				int old_x = rect.x;
				int old_y = rect.y;

				rect = image->clip_rect;

				rect.x = old_x;
				rect.y = old_y;

				update_properties();
			}
			else {
				const char* msg = SDL_GetError();
			}
		}
	}
};

//class Layer {
//public:
//	vector<Sprite*> layerImages;
//	int count = 0;
//	int origin_x =0, origin_y=0, layer_index;
//	LAYER_TYPE type = BACKGROUND;
//	SDL_Surface *drawing_area;
//
//	Layer(SDL_Surface *destination, SpriteGroup *group) {
//		layerImages = group->GetSprites();
//		count = layerImages.size;
//		drawing_area = destination;
//	}
//
//	void update(){
//		if (!layerImages.empty) {
//			for (int i = 0; i < count; i++) {
//				layerImages[i]->update();
//			}
//		}
//	}
//
//	void draw() {
//		if (!layerImages.empty()) {
//			for (int i = 0; i < count; i++) {
//				layerImages[i]->draw(drawing_area);
//			}
//		}
//	}
//};

//class LayerManager {
//public:
//	vector<SpriteGroup*> layers;
//	vector<Animation*> animations;
//
//	int count = 0;
//	int origin_x = 0, origin_y = 0;
//
//	//Reference to the drawing surface
//	SDL_Surface *drawing_area;
//
//	//Time keeping
//	time_t
//
//	LayerManager(SDL_Surface *destination) {
//		drawing_area = destination;
//	}
//
//	void add(SpriteGroup *group) {
//		layers.push_back(group);
//
//		count = layers.size();
//	}
//
//	void remove(SpriteGroup layer) {
//		for (int i = 0; i < count; i++) {
//			if (*layers[i] == layer) {
//				layers.erase(layers.begin() + i);
//
//				count = layers.size();
//			}
//		}
//	}
//
//	void draw() {
//		if (!layers.empty()) {
//			for (int i = 0; i < count; i++) {
//				layers[i]->draw(drawing_area);
//			}
//		}
//	}
//
//	void animate(long elapsedTime) {
//		if (!animations.empty()) {
//			for (int i = 0; i < animations.size; i++) {
//				animations[i]->update(elapsedTime);
//			}
//		}
//	}
//};

class Animation {
public:
	vector<Sprite*> frames;
	int currentFrame;
	long animTime;
	long totalDuration;

	void update(long elapsedTime) {

	}

	class AniFrame {

	};
};

bool init()
{
	//Initialization flag
	bool success = true;

	//Initialize SDL
	if (SDL_Init(SDL_INIT_VIDEO) < 0)
	{
		printf("SDL could not initialize! SDL Error: %s\n", SDL_GetError());
		success = false;
	}
	else
	{
		//Use OpenGL 3.1 core
		SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
		SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 1);
		SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);

		//Create window
		gWindow = SDL_CreateWindow("SDL Tutorial", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, WINDOW_WIDTH, WINDOW_HEIGHT, SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN);
		if (gWindow == NULL)
		{
			printf("Window could not be created! SDL Error: %s\n", SDL_GetError());
			success = false;
		}
		else
		{
			//Create context
			gContext = SDL_GL_CreateContext(gWindow);
			if (gContext == NULL)
			{
				printf("OpenGL context could not be created! SDL Error: %s\n", SDL_GetError());
				success = false;
			}
			else
			{
				//Initialize GLEW
				glewExperimental = GL_TRUE;
				GLenum glewError = glewInit();
				if (glewError != GLEW_OK)
				{
					printf("Error initializing GLEW! %s\n", glewGetErrorString(glewError));
				}

				//Use Vsync
				if (SDL_GL_SetSwapInterval(1) < 0)
				{
					printf("Warning: Unable to set VSync! SDL Error: %s\n", SDL_GetError());
				}

				//Initialize OpenGL
				if (!initGL())
				{
					printf("Unable to initialize OpenGL!\n");
					success = false;
				}
			}
		}
	}

	return success;
}

bool initGL()
{
	//Success flag
	bool success = true;

	//Generate program
	gProgramID = glCreateProgram();

	//Create vertex shader
	GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);

	//Get vertex source
	const GLchar* vertexShaderSource[] =
	{
		"#version 140\nin vec2 LVertexPos2D; void main() { gl_Position = vec4( LVertexPos2D.x, LVertexPos2D.y, 0, 1 ); }"
	};

	//Set vertex source
	glShaderSource(vertexShader, 1, vertexShaderSource, NULL);

	//Compile vertex source
	glCompileShader(vertexShader);

	//Check vertex shader for errors
	GLint vShaderCompiled = GL_FALSE;
	glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &vShaderCompiled);
	if (vShaderCompiled != GL_TRUE)
	{
		printf("Unable to compile vertex shader %d!\n", vertexShader);
		printShaderLog(vertexShader);
		success = false;
	}
	else
	{
		//Attach vertex shader to program
		glAttachShader(gProgramID, vertexShader);


		//Create fragment shader
		GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);

		//Get fragment source
		const GLchar* fragmentShaderSource[] =
		{
			"#version 140\nout vec4 LFragment; void main() { LFragment = vec4( 1.0, 1.0, 1.0, 1.0 ); }"
		};

		//Set fragment source
		glShaderSource(fragmentShader, 1, fragmentShaderSource, NULL);

		//Compile fragment source
		glCompileShader(fragmentShader);

		//Check fragment shader for errors
		GLint fShaderCompiled = GL_FALSE;
		glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &fShaderCompiled);
		if (fShaderCompiled != GL_TRUE)
		{
			printf("Unable to compile fragment shader %d!\n", fragmentShader);
			printShaderLog(fragmentShader);
			success = false;
		}
		else
		{
			//Attach fragment shader to program
			glAttachShader(gProgramID, fragmentShader);


			//Link program
			glLinkProgram(gProgramID);

			//Check for errors
			GLint programSuccess = GL_TRUE;
			glGetProgramiv(gProgramID, GL_LINK_STATUS, &programSuccess);
			if (programSuccess != GL_TRUE)
			{
				printf("Error linking program %d!\n", gProgramID);
				printProgramLog(gProgramID);
				success = false;
			}
			else
			{
				//Get vertex attribute location
				gVertexPos2DLocation = glGetAttribLocation(gProgramID, "LVertexPos2D");
				if (gVertexPos2DLocation == -1)
				{
					printf("LVertexPos2D is not a valid glsl program variable!\n");
					success = false;
				}
				else
				{
					//Initialize clear color
					glClearColor(0.f, 0.f, 0.f, 1.f);

					//VBO data
					GLfloat vertexData[] =
					{
						-0.5f, -0.5f,
						0.5f, -0.5f,
						0.5f,  0.5f,
						-0.5f,  0.5f
					};

					//IBO data
					GLuint indexData[] = { 0, 1, 2, 3 };

					//Create VBO
					glGenBuffers(1, &gVBO);
					glBindBuffer(GL_ARRAY_BUFFER, gVBO);
					glBufferData(GL_ARRAY_BUFFER, 2 * 4 * sizeof(GLfloat), vertexData, GL_STATIC_DRAW);

					//Create IBO
					glGenBuffers(1, &gIBO);
					glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, gIBO);
					glBufferData(GL_ELEMENT_ARRAY_BUFFER, 4 * sizeof(GLuint), indexData, GL_STATIC_DRAW);
				}
			}
		}
	}

	return success;
}

void handleKeys(unsigned char key, int x, int y)
{
	//Toggle quad
	if (key == 'q')
	{
		gRenderQuad = !gRenderQuad;
	}
}

void printProgramLog(GLuint program)
{
	//Make sure name is shader
	if (glIsProgram(program))
	{
		//Program log length
		int infoLogLength = 0;
		int maxLength = infoLogLength;

		//Get info string length
		glGetProgramiv(program, GL_INFO_LOG_LENGTH, &maxLength);

		//Allocate string
		char* infoLog = new char[maxLength];

		//Get info log
		glGetProgramInfoLog(program, maxLength, &infoLogLength, infoLog);
		if (infoLogLength > 0)
		{
			//Print Log
			printf("%s\n", infoLog);
		}

		//Deallocate string
		delete[] infoLog;
	}
	else
	{
		printf("Name %d is not a program\n", program);
	}
}

void printShaderLog(GLuint shader)
{
	//Make sure name is shader
	if (glIsShader(shader))
	{
		//Shader log length
		int infoLogLength = 0;
		int maxLength = infoLogLength;

		//Get info string length
		glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &maxLength);

		//Allocate string
		char* infoLog = new char[maxLength];

		//Get info log
		glGetShaderInfoLog(shader, maxLength, &infoLogLength, infoLog);
		if (infoLogLength > 0)
		{
			//Print Log
			printf("%s\n", infoLog);
		}

		//Deallocate string
		delete[] infoLog;
	}
	else
	{
		printf("Name %d is not a shader\n", shader);
	}
}

void render()
{
	//Clear color buffer
	glClear(GL_COLOR_BUFFER_BIT);

	//Render quad
	if (gRenderQuad)
	{
		//Bind program
		glUseProgram(gProgramID);

		//Enable vertex position
		glEnableVertexAttribArray(gVertexPos2DLocation);

		//Set vertex data
		glBindBuffer(GL_ARRAY_BUFFER, gVBO);
		glVertexAttribPointer(gVertexPos2DLocation, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(GLfloat), NULL);

		//Set index data and render
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, gIBO);
		glDrawElements(GL_TRIANGLE_FAN, 4, GL_UNSIGNED_INT, NULL);

		//Disable vertex position
		glDisableVertexAttribArray(gVertexPos2DLocation);

		//Unbind program
		glUseProgram(NULL);
	}
}

void close()
{
	//Deallocate program
	glDeleteProgram(gProgramID);

	//Destroy window	
	SDL_DestroyWindow(gWindow);
	gWindow = NULL;

	//Quit SDL subsystems
	SDL_Quit();
}


int main(int argc, char *argv[])
{
	SDL_Window *window;

	SDL_Init(SDL_INIT_EVERYTHING);
	IMG_Init(IMG_INIT_PNG);

	int threadFunction(void* data);
	int data = 101;

	//window = SDL_CreateWindow("Hi DEVs",
	//	SDL_WINDOWPOS_UNDEFINED,
	//	SDL_WINDOWPOS_UNDEFINED,
	//	WINDOW_WIDTH,	//WIDTH
	//	WINDOW_HEIGHT,	//HEIGHT
	//	SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN);

	//Start up SDL and create window
	if (!init())
	{
		printf("Failed to initialize!\n");
	}

	//SDL_Surface *screen = SDL_GetWindowSurface(gWindow);
	//Uint32 color1 = SDL_MapRGB(screen->format, 255, 255, 255);
	//Uint32 color2 = SDL_MapRGB(screen->format, 0, 0, 255);
	//Uint32 color3 = SDL_MapRGB(screen->format, 0, 255, 255);

	//SDL_FillRect(screen, NULL, color2);

	//Block block1(color3, 0, 0);
	//block1.set_image("background.png");
	//Block block2(color3, 0, 0);
	//block2.set_image("platforms.png");
	//Block block3(color3, 0, 0);
	//block3.set_image("foreground.png");


	//SpriteGroup active_sprites;
	//active_sprites.add(&block1);
	//active_sprites.add(&block2);
	//active_sprites.add(&block3);

	//active_sprites.draw(screen);

	//int a = 1, b = 2, c = 3;

	//vector<Sprite*> images = active_sprites.GetSprites();

	//images[0]->inc = a;
	//images[1]->inc = b;
	//images[2]->inc = c;

	////update
	//SDL_UpdateWindowSurface(gWindow);

	Uint32 starting_tick;

	SDL_Event event;
	//SDL_Renderer* gRenderer = SDL_GetRenderer(window);

	//SDL_Thread *thread = SDL_CreateThread(threadFunction, "LazyThread", (void*)data);
	bool running = true;

	//Enable text input
	SDL_StartTextInput();

	while (running) {
		starting_tick = SDL_GetTicks();

		while(SDL_PollEvent(&event)) {
			if (event.type == SDL_QUIT) {
				running = false;
				break;
			}
			//Handle keypress with current mouse position
			else if (event.type == SDL_TEXTINPUT)
			{
				int x = 0, y = 0;
				SDL_GetMouseState(&x, &y);
				handleKeys(event.text.text[0], x, y);
			}
		}
		data++;
		//SDL_CreateThread(threadFunction, "LazyThread", (void*)data);

		//active_sprites.update();
		//active_sprites.draw(screen);

		////Clear screen
		//SDL_SetRenderDrawColor(gRenderer, 0xFF, 0xFF, 0xFF, 0xFF);
		//SDL_RenderClear(gRenderer);

		//Render prompt
		//gSplashTexture.render(0, 0);
		//SDL_UpdateWindowSurface(window);

		//Update screen
		//SDL_RenderPresent(gRenderer);

		//Render quad
		render();

		//Update screen
		SDL_GL_SwapWindow(gWindow);

		cap_framerate(starting_tick);
	}

	//SDL_WaitThread(thread, NULL);
	//IMG_Quit();
	//SDL_DestroyWindow(window);
	//SDL_Delay(5000);

	//SDL_Quit();
	//Free resources and close SDL
	close();

    return 0;
}

