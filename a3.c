
/* Derived from scene.c in the The OpenGL Programming Guide */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

// #include <GL/gl.h>
// #include <GL/glu.h>
// #include <GL/glx.h>
// #include <GL/glut.h>

#include <OpenGL/gl.h>
#include <OpenGL/glu.h>
#include <GLUT/glut.h>

struct Normals {
	float Nx, Ny, Nz;

};
typedef struct Normals Normal;

	/* flags used to control the appearance of the image */
int lineDrawing = 0;	// draw polygons as solid or lines
int lighting = 1;	// use diffuse and specular lighting
int smoothShading = 1;  // smooth or flat shading
int textures = 1;
float spin = 0, lightHeight = 5;
int textureFile = 0;

int width, height, depth, maxDepth = 0, lButtonPressed = 0, rButtonPressed = 0;
float camX, camY, camZ;
float lightX, lightY, lightZ;

// the key states. These variables will be zero
//when no key is being presses
float deltaAngle = 0.0f;
float deltaMove = 0;

GLubyte  *Image[7];
GLuint   textureID[7];
static GLfloat floorVertices[4][3] = {
	{0,-0.001,10},
	{10,-0.001,10},
	{10,-0.001, 0},
	{0,-0.001, 0},
};

static GLfloat cube[30][3] = {
	//Top
	{0.5, 1, -0.5},
	{-0.5, 1, -0.5},
	{-0.5, 1, 0.5},

	{0.5, 1, -0.5},
	{-0.5, 1, 0.5},
	{0.5, 1, 0.5},

	//Right
	{0.5, 0, -0.5},
	{0.5, 1, -0.5},
	{0.5, 1, 0.5},

	{0.5, 0, -0.5},
	{0.5, 1, 0.5},
	{0.5, 0, 0.5},

	//Front
	{0.5, 0, 0.5},
	{0.5, 1, 0.5},
	{-0.5, 1, 0.5},

	{0.5, 0, 0.5},
	{-0.5, 1, 0.5},
	{-0.5, 0, 0.5},

	//Left
	{-0.5, 0, 0.5},
	{-0.5, 1, 0.5},
	{-0.5, 1, -0.5},

	{-0.5, 0, 0.5},
	{-0.5, 1, -0.5},
	{-0.5, 0, -0.5},

	//Back
	{0.5, 1, -0.5},
	{0.5, 0, -0.5},
	{-0.5, 0, -0.5},

	{0.5, 1, -0.5},
	{-0.5, 0, -0.5},
	{-0.5, 1, -0.5}
};

enum {
  X, Y, Z, W
};
enum {
  A, B, C, D
};

void lightRotation(void) {
	spin = spin + 1; /*MAC LINE */
	//spin = spin + 0.1; //Ubuntu line

	if(spin >= 360) {
		spin = spin - 360;
	}

	lightX = cos((spin * M_PI)/180) * 3;
	lightY = 3;
	lightZ = sin((spin * M_PI)/180) * 3;
	//printf("%f, %f, %f\n",lightX, lightY, lightZ);
	glutPostRedisplay();
}

void makeShadows(void){
	float shadow[4];
	GLfloat * point;
	GLfloat black[]   = {0.0, 0.0, 0.0, 0.0};
	int i;

	glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, black);

	for(i = 0; i < 30; i++){
		point = cube[i];

		shadow[0] = (lightY * point[0]) + ( -lightX * point[1]);
		shadow[1] = 0;
		shadow[2] = ( -lightZ * point[1]) + (lightY * point[2]);
		shadow[3] = lightY;

		shadow[0] = shadow[0] / shadow[3];
		shadow[1] = shadow[1] / shadow[3];
		shadow[2] = shadow[2] / shadow[3];
		shadow[3] = shadow[3] / shadow[3];

		//printf("%f, %f, %f, %f\n",shadow[0], shadow[1], shadow[2], shadow[3] );
		//glVertex3f(point[0], point[1], point[2]);
		glVertex3f(shadow[0], shadow[1], shadow[2]);
	}

}

Normal * calculateNormal(int x1, int y1, int z1, int x2, int y2, int z2, int x3, int y3, int z3){
	float vector1x, vector1y, vector1z, vector2x, vector2y, vector2z;
	Normal * calculatedNormal;

	//calculatedNormal = (Normal *)malloc(sizeof(Normal));

	vector1x = x2 - x1;
	vector1y = y2 - y1;
	vector1z = z2 - z1;

	vector2x = x3 - x1;
	vector2y = y3 - y1;
	vector2z = z3 - z1;

	calculatedNormal -> Nx = vector1y * vector2z - vector2y * vector1z;
	calculatedNormal -> Ny = vector1z * vector2x - vector2z * vector1x;
	calculatedNormal -> Nz = vector1x * vector2y - vector2x * vector1y;

	printf("%f, %f, %f\n", calculatedNormal -> Nx, calculatedNormal -> Ny, calculatedNormal -> Nz);

	return calculatedNormal;
}

/*  Initialize material property and light source.
 */
void init (void)
{
	GLfloat light_ambient[] = { 0.0, 0.0, 0.0, 1.0 };
	GLfloat light_diffuse[] = { 1.0, 1.0, 1.0, 1.0 };
	GLfloat light_specular[] = { 1.0, 1.0, 1.0, 1.0 };
	GLfloat light_full_off[] = {0.0, 0.0, 0.0, 1.0};
	GLfloat light_full_on[] = {1.0, 1.0, 1.0, 1.0};

	GLfloat light_position[] = { 0.5, 1.0, 1.0, 0.0 };

	/* if lighting is turned on then use ambient, diffuse and specular
		lights, otherwise use ambient lighting only */
	if (lighting == 1) {
		glLightfv (GL_LIGHT0, GL_AMBIENT, light_ambient);
		glLightfv (GL_LIGHT0, GL_DIFFUSE, light_diffuse);
		glLightfv (GL_LIGHT0, GL_SPECULAR, light_specular);
	} else {
		glLightfv (GL_LIGHT0, GL_AMBIENT, light_full_on);
		glLightfv (GL_LIGHT0, GL_DIFFUSE, light_full_off);
		glLightfv (GL_LIGHT0, GL_SPECULAR, light_full_off);
	}
	glLightfv (GL_LIGHT0, GL_POSITION, light_position);
	
	glEnable (GL_LIGHTING);
	glEnable (GL_LIGHT0);
	glEnable(GL_DEPTH_TEST);
}

void display (void)
{
GLfloat blue[]  = {0.0, 0.0, 1.0, 1.0};
GLfloat red[]   = {1.0, 0.0, 0.0, 1.0};
GLfloat green[] = {0.0, 1.0, 0.0, 1.0};
GLfloat white[] = {1.0, 1.0, 1.0, 1.0};
GLfloat light_gray[] = {0.8, 0.8, 0.8, 0.8};
GLfloat brown[] = {0.5, 0.35, 0.05, 0.35};
GLfloat random[] = {1.0, 1.0, 1.0, 1.0};
GLfloat position[] = { lightX, lightY, lightZ, 1.0 };
GLfloat light_full_off[] = {0.0, 0.0, 0.0, 1.0};
GLfloat light_diffuse[] = { 1.0, 1.0, 1.0, 1.0 };

float bottomThirdLimit, topThirdLimit;
float randomR, randomG, randomB;
Normal * normals;

	glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	/* draw surfaces as either smooth or flat shaded */
	if (smoothShading == 1)
		glShadeModel(GL_SMOOTH);
	else
		glShadeModel(GL_FLAT);

	/* draw polygons as either solid or outlines */
	if (lineDrawing == 1)
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	else 
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

	glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, light_gray);
	glMaterialfv(GL_FRONT, GL_SPECULAR, light_gray);
	//draw plane

	glLightfv (GL_LIGHT0, GL_DIFFUSE, light_full_off);

	glBegin(GL_TRIANGLES);
		glVertex3f( 0,-0.5, 0);
		glVertex3f( 0,-0.5,10);
		glVertex3f(10,-0.5,10);

		glVertex3f( 0,-0.5, 0);
		glVertex3f(10,-0.5,10);
		glVertex3f(10,-0.5, 0);
	glEnd();

	glLightfv (GL_LIGHT0, GL_DIFFUSE, light_diffuse);

	// glBegin(GL_QUADS);
	// 	//glNormal3f( 0,-0.001, 0);
	// 	glVertex3f( 0,-0.001, 0);

	// 	//glNormal3f( 0,-0.001,10);
	// 	glVertex3f( 0,-0.001,10);

	// 	//glNormal3f(10,-0.001,10);
	// 	glVertex3f(10,-0.001,10);

	// 	//glNormal3f(10,-0.001, 0);
	// 	glVertex3f(10,-0.001, 0);
	// glEnd();

	/* turn texturing on */
	if (textures == 1) {
		glEnable(GL_TEXTURE_2D);
		glBindTexture(GL_TEXTURE_2D, textureID[textureFile]); //change texture
		if(textureFile == 7){
			textureFile = 0;
		}
	/* if textured, then use white as base colour */
		glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, light_gray);
	}

	glPushMatrix();

	glTranslatef(5,0.5,5);
	//normals = (Normal *)malloc(sizeof(Normal));

	//glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, white);
	//glMaterialfv(GL_FRONT, GL_SPECULAR, white);
	glMaterialf(GL_FRONT, GL_SHININESS, 30);

	glBegin(GL_TRIANGLES);
		//Top
		glVertex3f(0.5, 0.5, -0.5);
		glVertex3f(-0.5, 0.5, -0.5);
		glVertex3f(-0.5, 0.5, 0.5);

		glVertex3f(0.5, 0.5, -0.5);
		glVertex3f(-0.5, 0.5, 0.5);
		glVertex3f(0.5, 0.5, 0.5);

		//Right
		glNormal3f(0.5, 0.0, 0.0);
		glVertex3f(0.5, -0.5, -0.5);
		glVertex3f(0.5, 0.5, -0.5);
		glVertex3f(0.5, 0.5, 0.5);

		glNormal3f(0.5, 0.0, 0.0);
		glVertex3f(0.5, -0.5, -0.5);
		glVertex3f(0.5, 0.5, 0.5);
		glVertex3f(0.5, -0.5, 0.5);

		//Front
		glNormal3f(0.0, 0.0, 0.5);
		glVertex3f(0.5, -0.5, 0.5);
		glVertex3f(0.5, 0.5, 0.5);
		glVertex3f(-0.5, 0.5, 0.5);

		glNormal3f(0.0, 0.0, 0.5);
		glVertex3f(0.5, -0.5, 0.5);
		glVertex3f(-0.5, 0.5, 0.5);
		glVertex3f(-0.5, -0.5, 0.5);

		//Left
		glNormal3f(-0.5, 0.0, 0.0);
		glVertex3f(-0.5, -0.5, 0.5);
		glVertex3f(-0.5, 0.5, 0.5);
		glVertex3f(-0.5, 0.5, -0.5);

		glNormal3f(-0.5, 0.0, 0.0);
		glVertex3f(-0.5, -0.5, 0.5);
		glVertex3f(-0.5, 0.5, -0.5);
		glVertex3f(-0.5, -0.5, -0.5);

		//Back
		glNormal3f(0.0, 0.0, -0.5);
		glVertex3f(0.5, 0.5, -0.5);
		glVertex3f(0.5, -0.5, -0.5);
		glVertex3f(-0.5, -0.5, -0.5);

		glNormal3f(0.0, 0.0, -0.5);
		glVertex3f(0.5, 0.5, -0.5);
		glVertex3f(-0.5, -0.5, -0.5);
		glVertex3f(-0.5, 0.5, -0.5);
	glEnd();

	if (textures == 1){
		glDisable(GL_TEXTURE_2D);
	}

	glTranslatef(0,-0.5,0);
	glBegin(GL_TRIANGLES);
		makeShadows();
	glEnd();

	glTranslatef(0,0.5,0);
	
	//glRotated ((GLdouble) spin, 0.0, 1.0, 0.0);

	glLightfv (GL_LIGHT0, GL_POSITION, position);

	glTranslated (lightX, lightY, lightZ);
	glutIdleFunc(lightRotation);
	glutWireCube (0.1);
	glEnable (GL_LIGHTING);

	glPopMatrix ();

	glPopMatrix ();
	glFlush ();
	glutSwapBuffers();
}

void reshape(int w, int h)
{
	glViewport (0, 0, (GLsizei) w, (GLsizei) h);
	glMatrixMode (GL_PROJECTION);
	glLoadIdentity ();
	gluPerspective(45.0, (GLfloat)w/(GLfloat)h, 1.0, 1000.0);
	glMatrixMode (GL_MODELVIEW);
	glLoadIdentity ();
	// Set the camera
	gluLookAt(10, 15, 10,    // Look at point
			0, -20, 0,
			0, 1, 0); 
}

void keyboard(unsigned char key, int x, int y)
{
	switch (key) {
		case 27:
		case 'q':
			exit(0);
			break;
		case '1':		// texture with  smooth shading
			lineDrawing = 0;
			lighting = 1;
			smoothShading = 1;
			textures = 1;
			init();
			display();
			textureFile++;
			break;
		case '2':		// draw polygons as outlines
			lineDrawing = 1;
			lighting = 0;
			smoothShading = 0;
			textures = 0;
			init();
			display();
			break;
		case '3':		// draw polygons as filled
			lineDrawing = 0;
			lighting = 0;
			smoothShading = 0;
			textures = 0;
			init();
			display();
			break;
		case '4':		// diffuse and specular lighting, smooth shading
			lineDrawing = 0;
			lighting = 1;
			smoothShading = 1;
			textures = 0;
			init();
			display();
			break;
		
		case 'w':		// draw polygons as outlines
			glRotatef(5, 1.0, 0.5, 0.0);
			init();
			display();
			break;
		case 'd':		// draw polygons as outlines
			glRotatef(5, 0.0, 0.5, 0.0);
			init();
			display();
			break;
		case 'a':		// draw polygons as outlines
			glRotatef(5, 1.0, -1.0, -1.0);
			init();
			display();
			break;
		case 's':		// draw polygons as outlines
			glTranslatef(0.0, 0.0,-0.5);
			init();
			display();
			break;
		case 'f':		// draw polygons as outlines
			glTranslatef(0.0, -1.0, 0.0);
			init();
			display();
			break;
		case 'v':		// draw polygons as outlines
			glTranslatef(0.0, 1.0,0.0);
			init();
			display();
			break;
	}
}

void mouse(int button, int state, int x, int y) {
	if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN) { //left button and pressed
		lButtonPressed = x;
	}
	else if(button == GLUT_LEFT_BUTTON && state == GLUT_UP){
		lButtonPressed = -1;
	}

	else if (button == GLUT_RIGHT_BUTTON && state == GLUT_DOWN) { //right button and pressed
		rButtonPressed = y;
	}
	else {
		rButtonPressed = -1;
	}
}

void motion(int x, int y) {
	if(lButtonPressed >= 0){

		camX = 5 * -sinf(20*(M_PI/180)) * cosf((5)*(M_PI/180));
		camY = 5 * -sinf((5)*(M_PI/180));
		camZ = -5 * cosf((5)*(M_PI/180)) * cosf((5)*(M_PI/180));
		

		glMatrixMode (GL_MODELVIEW);
		glLoadIdentity ();
		x = x%180;
		y = y%180;

		gluLookAt(x, 15, y,    // Look at point
			0, -20, 0,
			0, 1, 0); 
		display ();
	}

	else if(rButtonPressed >= 1){
		glMatrixMode (GL_MODELVIEW);
		glLoadIdentity ();

		gluLookAt(x, y, 0,    // Look at point
			width/2, 0, height/2,
			0.0, 1.0, 0.0);   // Up vector
		display ();

	}
}

void loadTexture() {
	FILE *fp;
	int  i, j, x;
	int  red, green, blue;
	char * fileName, * buffer;
	char instr[1024];
	int header = 0;

	glGenTextures(7,textureID);

	for (x = 0; x < 7; x++){
		switch(x) {
			case 0:
				fileName = "images/brick.ppm";
				break;
			case 1:
				fileName = "images/horrible.ppm";
				break;
			case 2:
				fileName = "images/moon.ppm";
				break;
			case 3:
				fileName = "images/mud.ppm";
				break;
			case 4:
				fileName = "images/psych.ppm";
				break;
			case 5:
				fileName = "images/spots.ppm";
				break;
			case 6:
				fileName = "images/wood.ppm";
		}

		header = 0;
		i = 0;

		if ((fp = fopen(fileName, "r")) == 0) {
			printf("Error, failed to find the file named %s.\n", fileName);
			exit(0);
		} 

		else {
			while (fgets(instr, sizeof(instr), fp)) {
				if (instr[0] == '#' || instr[0] == 'P') {
					//skip reading in line
				}

				else {
					if(header == 0) {
						//run header info
						buffer = strtok(instr, " ");
						width = atoi(buffer);

						buffer = strtok(NULL, " ");
						height = atoi(buffer);

						Image[x] = malloc (sizeof(GLubyte) * width * height * 4);
						header++;
					}

					else if( header == 1) {
						depth = atof(instr);
						header++;
					}

					else {
						buffer = strtok(instr, "  ");

						while (buffer != NULL){
							//printf("%d\n", atoi(buffer));
							Image[x][i] = atoi(buffer);
							i++;

							if(i % 4 == 0){
								Image[x][i] = 255;
								i++;
							}

							buffer = strtok(NULL, "  ");
						}//end buffer

					}
				}
			}//end file loop
		}//end located file

	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	glBindTexture(GL_TEXTURE_2D, textureID[x]); //loop to include 7 files
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA,
		GL_UNSIGNED_BYTE, Image[x]); 
	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
	glEnable(GL_TEXTURE_GEN_S);
	glEnable(GL_TEXTURE_GEN_T);

	}//end for loop
}


/*  Main Loop
 *  Open window with initial window size, title bar, 
 *  RGBA display mode, and handle input events.
 */
int main(int argc, char** argv)
{
	glutInit(&argc, argv);
	glutInitDisplayMode (GLUT_SINGLE | GLUT_RGBA | GLUT_DEPTH);
	glutInitWindowPosition(80, 80);
	glutInitWindowSize (1024, 768);
	glutCreateWindow (argv[0]);
	init();
	loadTexture();
	glutReshapeFunc (reshape);
	glutDisplayFunc(display);
	glutKeyboardFunc (keyboard);
	glutMouseFunc(mouse);
	glutMotionFunc(motion);


	glutMainLoop();
	return 0; 
}

