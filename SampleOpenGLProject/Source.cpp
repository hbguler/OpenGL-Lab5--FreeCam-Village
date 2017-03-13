#include "Angel.h"
#include "libobj.h"
#include "texture.h"
#include <time.h>
#include <fstream>
#include "glui.h"
#include <vector>


typedef Angel::vec4 color4;
typedef Angel::vec4 point4;

clock_t start = clock();

//----------------------------------------------------------------------------

//init floor

GLuint floor_vao;

GLfloat vertices_floor_buffer[] = {
	-20 ,0 , -20 , 
	-20 ,0 , 60, 
	60,0 , 60, 
	60,0 , -20, 
};

GLfloat texture_floor_buffer[] = {
	-20,-20 ,
	-20,60 ,
	60,60,
	60,-20
};

GLfloat normals_floor_buffer[] = {
	0,1,0
};

GLuint floor_vertices;
GLuint floor_textures;
GLuint floor_normals;

//----------------------------------------------------------------------------

//init houses

GLuint house_vao;

vector<vec3> house_vertices_buffer;	//v
vector<vec3> house_normals_buffer;	//vn
vector<vec2> house_textures_buffer; //vt
vector<vec3> faces; //f

GLuint house_vertices;
GLuint house_textures;
GLuint house_normals;

//----------------------------------------------------------------------------

//init light

//point4 light_position(eyeX, eyeY, eyeZ, 0.0);

int vert0frag1 = 1;
int vert0frag1handler = 0;

int off0on1 = 1;
int off0on1handler = 0;

point4 light_ambient = vec4(0.2, 0.2, 0.2, 0.0);
point4 light_diffuse = vec4(0.5, 0.0, 0.0, 1.0);
point4 light_specular = vec4(0.7, 0.6, 0.6, 1.0);

float material_shininess = 1;

point4 material_ambient = vec4(1.0, 0.0, 1.0, 1.0);
point4 material_diffuse = vec4(1.0, 0.8, 0.0, 1.0);
point4 material_specular = vec4(1.0, 0.0, 1.0, 1.0);

color4 ambient_product = light_ambient * material_ambient;
color4 diffuse_product = light_diffuse * material_diffuse;
color4 specular_product = light_specular * material_specular;

//----------------------------------------------------------------------------

GLuint projection;

int window_width = 800;
int window_height = 600;

//----------------------------------------------------------------------------

GLMmodel* village;
mat4 villageAt(1.0);

bool keys[1024];
GLfloat phi = 0.0;
//GLfloat eyeX = 20.0, eyeY = 10.0, eyeZ = 50.0;
GLfloat eyeX = 5.0, eyeY = 2.0, eyeZ = 5.0;
vec3  eye(eyeX, eyeY, eyeZ);
vec3  at(0.643, -0.404, 0.653);
point4  up(0.0, 1.0, 0.0, 0.0);

GLfloat radius = 1.0;
GLfloat mouseSens = 0.1;
GLfloat mouseSpeed = 0.005;
GLfloat horizontalAngle = 3.14;
GLfloat verticalAngle = 0.0;

GLfloat speed; 
GLfloat yaw = -315.0f;
GLfloat pitch = -28.0f;

GLuint view;  
GLuint model;
GLuint program;
GLuint color;
GLuint thetaLoc;
GLuint pos;

GLfloat deltaTime = 0.0;
GLfloat lastFrame = 0.0;

int instance_count = 20;

GLuint TextureID;
GLuint TextureGreen;
GLuint TextureBrick;
GLuint TextureFloor;

int   main_window;

//----------------------------------------------------------------------------

// Reading object .obj file
void readOBJ(const char* filename) {

	std::vector<unsigned int> vertexIndices, uvIndices, normalIndices;
	std::vector<vec3> temp_vertices;
	std::vector<vec2> temp_uvs;
	std::vector<vec3> temp_normals;


	FILE * file = fopen(filename, "r");
	if (file == NULL) {
		printf("Impossible to open the file \n");
		getchar();
	}

	while (1) {

		char lineHeader[128];
		int res = fscanf(file, "%s", lineHeader);
		if (res == EOF)
			break;

		if (strcmp(lineHeader, "v") == 0) {
			vec3 vertex;
			fscanf(file, "%f %f %f\n", &vertex.x, &vertex.y, &vertex.z);
			temp_vertices.push_back(vertex);
		}
		else if (strcmp(lineHeader, "vt") == 0) {
			vec2 uv;
			fscanf(file, "%f %f\n", &uv.x, &uv.y);
			temp_uvs.push_back(uv);
		}
		else if (strcmp(lineHeader, "vn") == 0) {
			vec3 normal;
			fscanf(file, "%f %f %f\n", &normal.x, &normal.y, &normal.z);
			temp_normals.push_back(normal);
		}
		else if (strcmp(lineHeader, "f") == 0) {
			std::string vertex1, vertex2, vertex3;
			unsigned int vertexIndex[3], uvIndex[3], normalIndex[3];
			int matches = fscanf(file, "%d/%d/%d %d/%d/%d %d/%d/%d\n", &vertexIndex[0], &uvIndex[0], &normalIndex[0], &vertexIndex[1], &uvIndex[1], &normalIndex[1], &vertexIndex[2], &uvIndex[2], &normalIndex[2]);

			vertexIndices.push_back(vertexIndex[0]);
			vertexIndices.push_back(vertexIndex[1]);
			vertexIndices.push_back(vertexIndex[2]);
			uvIndices.push_back(uvIndex[0]);
			uvIndices.push_back(uvIndex[1]);
			uvIndices.push_back(uvIndex[2]);
			normalIndices.push_back(normalIndex[0]);
			normalIndices.push_back(normalIndex[1]);
			normalIndices.push_back(normalIndex[2]);
		}

		else {
			char stupidBuffer[1000];
			fgets(stupidBuffer, 1000, file);
		}

	}

	// For each vertex of each triangle
	for (unsigned int i = 0; i<vertexIndices.size(); i++) {

		// Get the indices of its attributes
		unsigned int vertexIndex = vertexIndices[i];
		unsigned int uvIndex = uvIndices[i];
		unsigned int normalIndex = normalIndices[i];

		// Get the attributes thanks to the index
		vec3 vertex = temp_vertices[vertexIndex - 1];
		vec2 uv = temp_uvs[uvIndex - 1];
		vec3 normal = temp_normals[normalIndex - 1];

		// Put the attributes in buffers
		house_vertices_buffer.push_back(vertex);
		house_textures_buffer.push_back(uv);
		house_normals_buffer.push_back(normal);

	}
	fclose(file);
}

//----------------------------------------------------------------------------

// Reading material .mtl file
void readMTL(const char* filename) {


	ifstream inMTL;
	inMTL.open(filename);

	if (!inMTL.good())
	{
		cout << "ERROR OPENING MTL FILE" << endl;
		exit(1);
	}

	while (!inMTL.eof())
	{
		// 5
		string line;
		getline(inMTL, line);
		string type = line.substr(0, 2);

		if (type.compare("ne") == 0) {
			//new material
			string l = "newmtl ";
			string m = line.substr(l.size());
			//materialNames.push_back(m);

		}
		else if (type.compare("Kd") == 0) {
			// Copy line for parsing
			char* l = new char[line.size() + 1];
			memcpy(l, line.c_str(), line.size() + 1);

			vec3 diffuseTemp;
			// Extract tokens
			strtok(l, " ");

			diffuseTemp.x = atof(strtok(NULL, " "));
			diffuseTemp.y = atof(strtok(NULL, " "));
			diffuseTemp.z = atof(strtok(NULL, " "));
			//diffusionVector.push_back(diffuseTemp);
			// Wrap up
			delete[] l;

		}
		else if (type.compare("Ks") == 0) {
			char* l = new char[line.size() + 1];
			memcpy(l, line.c_str(), line.size() + 1);

			vec3 specularTemp;
			// Extract tokens
			strtok(l, " ");

			specularTemp.x = atof(strtok(NULL, " "));
			specularTemp.y = atof(strtok(NULL, " "));
			specularTemp.z = atof(strtok(NULL, " "));
			//specularVector.push_back(specularTemp);
			delete[] l;

		}

	}

	//fclose(file);
	printf("Loading Complete!\n");
}

//----------------------------------------------------------------------------

void init()
{

	readOBJ("singleHouse.obj");
	program = InitShader("vshader56.glsl", "fshader56.glsl");
	glUseProgram(program);

	//----------------------------------------------------------------------------

	glGenVertexArrays(1, &house_vao);
	glBindVertexArray(house_vao);

	glGenBuffers(1, &house_vertices);
	glBindBuffer(GL_ARRAY_BUFFER, house_vertices);
	glBufferData(GL_ARRAY_BUFFER, house_vertices_buffer.size() * sizeof(vec3), &house_vertices_buffer[0], GL_STATIC_DRAW);

	glGenBuffers(1, &house_textures);
	glBindBuffer(GL_ARRAY_BUFFER, house_textures);
	glBufferData(GL_ARRAY_BUFFER, house_textures_buffer.size() * sizeof(vec2), &house_textures_buffer[0], GL_STATIC_DRAW);

	glGenBuffers(1, &house_normals);
	glBindBuffer(GL_ARRAY_BUFFER, house_normals);
	glBufferData(GL_ARRAY_BUFFER, house_normals_buffer.size() * sizeof(vec3) , &house_normals_buffer[0], GL_STATIC_DRAW);

	//----------------------------------------------------------------------------

	glGenVertexArrays(1, &floor_vao);
	glBindVertexArray(floor_vao);

	glGenBuffers(1, &floor_vertices);
	glBindBuffer(GL_ARRAY_BUFFER, floor_vertices);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices_floor_buffer), vertices_floor_buffer, GL_STATIC_DRAW);

	glGenBuffers(1, &floor_textures);
	glBindBuffer(GL_ARRAY_BUFFER, floor_textures);
	glBufferData(GL_ARRAY_BUFFER, sizeof(texture_floor_buffer), texture_floor_buffer, GL_STATIC_DRAW);

	glGenBuffers(1, &floor_normals);
	glBindBuffer(GL_ARRAY_BUFFER, floor_normals);
	glBufferData(GL_ARRAY_BUFFER, sizeof(normals_floor_buffer), normals_floor_buffer, GL_STATIC_DRAW);

	//----------------------------------------------------------------------------

	TextureGreen = loadBMP_custom("green_tile_512.bmp");
	TextureBrick = loadBMP_custom("red_brick_512.bmp");
	TextureFloor = loadBMP_custom("ground.bmp");

	TextureID = glGetUniformLocation(program, "myTextureSampler");


	//----------------------------------------------------------------------------

	glUniform4fv(glGetUniformLocation(program, "AmbientProduct"), 1, ambient_product);
	glUniform4fv(glGetUniformLocation(program, "DiffuseProduct"), 1, diffuse_product);
	glUniform4fv(glGetUniformLocation(program, "SpecularProduct"), 1, specular_product);
	glUniform1f(glGetUniformLocation(program, "Shininess"), material_shininess);
	
	//----------------------------------------------------------------------------

	pos = glGetUniformLocation(program, "pos");
	model = glGetUniformLocation(program, "model");
	view = glGetUniformLocation(program, "view");
	projection = glGetUniformLocation(program, "projection");
	color = glGetUniformLocation(program, "modelColor");
	thetaLoc = glGetUniformLocation(program, "theta");
	vert0frag1handler = glGetUniformLocation(program, "vert0frag1");
	off0on1handler = glGetUniformLocation(program, "off0on1");

	//----------------------------------------------------------------------------

	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);
	glEnable(GL_CULL_FACE);
	glClearColor(0.0, 0.0, 0.0, 1.0); /* white background */
}

//----------------------------------------------------------------------------

void display(void)
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	//vec4(eyeX, eyeY, eyeZ, 0.0)
	//vec4(at.x, at.y, at.z, 0.0)
	glUniform4fv(glGetUniformLocation(program, "LightPosition"), 1, vec4(at.x, at.y, at.z, 0.0));
	glUniform1i(vert0frag1handler, vert0frag1);
	glUniform1i(off0on1handler, off0on1);

	glUniform4fv(color, 1, vec4(0.0, 1.0, 0.0, 1.0));
	glUniformMatrix4fv(model, 1, GL_TRUE, villageAt);

	mat4  v = LookAt(eye, eye + at, up);
	glUniformMatrix4fv(view, 1, GL_TRUE, v);

	// 1rst attribute buffer : vertices
	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, house_vertices);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0 );

	// 2nd attribute buffer : UVs
	glEnableVertexAttribArray(1);
	glBindBuffer(GL_ARRAY_BUFFER, house_textures);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, (void*)0 );

	// 3rd attribute buffer : normals
	glEnableVertexAttribArray(2);
	glBindBuffer(GL_ARRAY_BUFFER, house_normals);
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 0, (void*)0 );

	for (int x = 0; x < instance_count; ++x)
	{
		for (int y = 0; y < instance_count; ++y)
		{
			int space = 4;

			glUniform4fv(pos, 1, vec4(space * x, 0.0, space * y, 1.0));
			
			// Bind our texture in Texture Unit 0
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, TextureGreen);
			// Set our "myTextureSampler" sampler to user Texture Unit 0
			glUniform1i(TextureID, 0);
			// Draw the roof
			glDrawArrays(GL_TRIANGLES, 0, 12);

			// Bind our texture in Texture Unit 0
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, TextureBrick);
			// Set our "myTextureSampler" sampler to user Texture Unit 0
			glUniform1i(TextureID, 0);
			// Draw the roof
			glDrawArrays(GL_TRIANGLES, 12, house_vertices_buffer.size());
		}
	}

	glDisableVertexAttribArray(0);
	glDisableVertexAttribArray(1);
	glDisableVertexAttribArray(2);

	glUniform4fv(pos,1, vec4(0,0,0,0));

	//4th attribute buffer : floor
	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, floor_vertices);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(0));

	glEnableVertexAttribArray(1);
	glBindBuffer(GL_ARRAY_BUFFER, floor_textures);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, (void*)0);

	glEnableVertexAttribArray(2);
	glBindBuffer(GL_ARRAY_BUFFER, floor_normals);
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);

	// Bind our texture in Texture Unit 0
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, TextureFloor);
	glUniform1i(TextureID, 0);

	glDrawArrays(GL_POLYGON, 0, 4);

	glDisableVertexAttribArray(0);
	glDisableVertexAttribArray(1);
	glDisableVertexAttribArray(2);

	glutSwapBuffers();
}

//----------------------------------------------------------------------------

void
keyboard(unsigned char key, int x, int y)
{
	switch (key) {
	case 033: // Escape Key
	case 'q': case 'Q':
		exit(EXIT_SUCCESS);
		break;

	case ' ': //Space bar
		eyeX = 0; eyeY = 0; eyeZ = 0;
		break;
	
	case 'l':
		vert0frag1 = -1 * vert0frag1;
		break;
	case 'f':
		off0on1 = -1 * off0on1;
		break;
	}

	if (key >= 0 && key < 1024)
	{
		keys[key] = true;
	}
}

void keyboardUp(unsigned char key, int x, int y) {
	if (key >= 0 && key < 1024)
	{
		keys[key] = false;
	}
}

//----------------------------------------------------------------------------

void special(int key, int x, int y) {
	if (key >= 0 && key < 1024)
	{
		keys[key] = true;
	}
}
void specialUp(int key, int x, int y) {
	if (key >= 0 && key < 1024)
	{
		keys[key] = false;
	}
}

//----------------------------------------------------------------------------

void movement() {
	speed = 5.0 * deltaTime;
	if (keys[GLUT_KEY_UP]) {
		eye += at * speed;
	}
	if (keys[GLUT_KEY_DOWN]) {
		eye -= at * speed;
	}
	if (keys[GLUT_KEY_LEFT]) {
		eye -= normalize(cross(at, up)) * speed;
	}
	if (keys[GLUT_KEY_RIGHT]) {
		eye += normalize(cross(at, up)) * speed;
	}
}

//----------------------------------------------------------------------------

float mouseX, mouseY;
float springiness = 50; // tweak to taste.
GLfloat lastX, lastY, xoffset, yoffset;
bool firstMouse = true, mouseWarp = false;

void updateCameraVectors() {
	vec3 temp;
	temp.x = cos(yaw*DegreesToRadians) * cos(pitch * DegreesToRadians);
	temp.y = sin(pitch*DegreesToRadians);
	temp.z = sin(yaw*DegreesToRadians)*cos(pitch*DegreesToRadians);
	at = normalize(temp);
}
void processMouseMovement() {
	xoffset *= mouseSens;
	yoffset *= mouseSens;
	yaw += xoffset;
	pitch += yoffset;
	if (pitch > 89.0f)
		pitch = 89.0f;
	if (pitch < -89.0f)
		pitch = -89.0f;
	updateCameraVectors();
}


void mousePassive(int x, int y) {

	int windowWidth = glutGet(GLUT_WINDOW_WIDTH);
	int windowHeight = glutGet(GLUT_WINDOW_HEIGHT);

	if (lastX == x && lastY == y) return;
	printf("%d %d\n", x, y);
	if (x <= 100 || y <= 100 || x >= windowWidth - 100 || y >= windowHeight - 100) {
		printf("Working mouse.\n");
		lastX = glutGet(GLUT_WINDOW_WIDTH) / 2;
		lastY = glutGet(GLUT_WINDOW_HEIGHT) / 2;
		glutWarpPointer(glutGet(GLUT_WINDOW_WIDTH) / 2, glutGet(GLUT_WINDOW_HEIGHT) / 2);

	}
	else {
		if (firstMouse)
		{
			lastX = x;
			lastY = y;
			firstMouse = false;
		}
		xoffset = x - lastX;
		yoffset = lastY - y;  // Reversed since y-coordinates go from bottom to left
		lastX = x;
		lastY = y;
		processMouseMovement();
	}



}
//----------------------------------------------------------------------------

void idle() {

	GLfloat currentFrame = (double)(clock() - start) / CLOCKS_PER_SEC;


	deltaTime = currentFrame - lastFrame;
	lastFrame = currentFrame;

	movement();

	
	GLUI_Master.auto_set_viewport();
	if (glutGetWindow() != main_window)
		glutSetWindow(main_window);

	glutPostRedisplay();

}

//----------------------------------------------------------------------------
void
reshape(int width, int height)
{
	glViewport(0, 0, width, height);
	window_width = width;
	window_height = height;

	GLfloat fovy = 90.0;
	GLfloat aspect = GLfloat(width) / height;
	GLfloat zNear = 0.1, zFar = 100.0;

	mat4  p = Perspective(fovy, aspect, zNear, zFar);
	glUniformMatrix4fv(projection, 1, GL_TRUE, p);

}



//----------------------------------------------------------------------------

int
main(int argc, char **argv)
{
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH);

	glutInitWindowSize(window_width, window_height);
	main_window = glutCreateWindow("VILLAGE");
	glutWarpPointer(window_width / 2, window_height / 2);
	
	glutSetCursor(GLUT_CURSOR_NONE);

	glewInit();
	init();

	glutDisplayFunc(display);

	glutPassiveMotionFunc(mousePassive);
	glutIdleFunc(idle);
	glutSpecialFunc(special);
	glutSpecialUpFunc(specialUp);
	glutKeyboardFunc(keyboard);
	glutKeyboardUpFunc(keyboardUp);
	glutReshapeFunc(reshape);

	glutMainLoop();
	return 0;
}