
/* Derived from scene.c in the The OpenGL Programming Guide */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include <GL/gl.h>
#include <GL/glu.h>
#include <GL/glx.h>
#include <GL/glut.h>

// #include <OpenGL/gl.h>
// #include <OpenGL/glu.h>
// #include <GLUT/glut.h>

struct Vertexes {
	float x, y, z;

};
typedef struct Vertexes Vertex;

struct Normals {
	float Nx, Ny, Nz;

};
typedef struct Normals Normal;

	/* flags used to control the appearance of the image */
int lineDrawing = 0;	// draw polygons as solid or lines
int lighting = 1;	// use diffuse and specular lighting
int smoothShading = 1;  // smooth or flat shading
int textures = 0;
float spin = 0, lightHeight = 5;

float **heightMap;
int width, height, depth, maxDepth = 0, lButtonPressed = 0, rButtonPressed = 0;
float camX, camY, camZ;

// the key states. These variables will be zero
//when no key is being presses
float deltaAngle = 0.0f;
float deltaMove = 0;

GLubyte  ***Image;
GLuint   textureID[7];
static int directionalLight = 1;
static GLfloat lightPosition[4];
static GLfloat floorPlane[4];
static GLfloat floorShadow[4][4];
static GLfloat floorVertices[4][3] = {
	{0,-0.001,10},
	{10,-0.001,10},
	{10,-0.001, 0},
	{0,-0.001, 0},
};

enum {
  X, Y, Z, W
};
enum {
  A, B, C, D
};

void lightRotation(void) {
	//spin = spin + 1; /*MAC LINE */
	spin = spin + 0.1; //Ubuntu line

	if(spin >= 360) {
		spin = spin - 360;
	}
	glutPostRedisplay();
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

	return calculatedNormal;
}

/* Find the plane equation given 3 points. */
void findPlane(GLfloat plane[4], GLfloat v0[3], GLfloat v1[3], GLfloat v2[3]){
	GLfloat vec0[3], vec1[3];

	/* Need 2 vectors to find cross product. */
	vec0[X] = v1[X] - v0[X];
	vec0[Y] = v1[Y] - v0[Y];
	vec0[Z] = v1[Z] - v0[Z];

	vec1[X] = v2[X] - v0[X];
	vec1[Y] = v2[Y] - v0[Y];
	vec1[Z] = v2[Z] - v0[Z];

	/* find cross product to get A, B, and C of plane equation */
	plane[A] = vec0[Y] * vec1[Z] - vec0[Z] * vec1[Y];
	plane[B] = -(vec0[X] * vec1[Z] - vec0[Z] * vec1[X]);
	plane[C] = vec0[X] * vec1[Y] - vec0[Y] * vec1[X];

	plane[D] = -(plane[A] * v0[X] + plane[B] * v0[Y] + plane[C] * v0[Z]);
}

void shadowMatrix(GLfloat shadowMat[4][4], GLfloat groundplane[4], GLfloat lightpos[4]) {
	GLfloat dot;

	/* Find dot product between light position vector and ground plane normal. */
	dot = groundplane[X] * lightpos[X] + groundplane[Y] * lightpos[Y] + groundplane[Z] * lightpos[Z] + groundplane[W] * lightpos[W];

	shadowMat[0][0] = dot - lightpos[X] * groundplane[X];
	shadowMat[1][0] = 0.f - lightpos[X] * groundplane[Y];
	shadowMat[2][0] = 0.f - lightpos[X] * groundplane[Z];
	shadowMat[3][0] = 0.f - lightpos[X] * groundplane[W];

	shadowMat[X][1] = 0.f - lightpos[Y] * groundplane[X];
	shadowMat[1][1] = dot - lightpos[Y] * groundplane[Y];
	shadowMat[2][1] = 0.f - lightpos[Y] * groundplane[Z];
	shadowMat[3][1] = 0.f - lightpos[Y] * groundplane[W];

	shadowMat[X][2] = 0.f - lightpos[Z] * groundplane[X];
	shadowMat[1][2] = 0.f - lightpos[Z] * groundplane[Y];
	shadowMat[2][2] = dot - lightpos[Z] * groundplane[Z];
	shadowMat[3][2] = 0.f - lightpos[Z] * groundplane[W];

	shadowMat[X][3] = 0.f - lightpos[W] * groundplane[X];
	shadowMat[1][3] = 0.f - lightpos[W] * groundplane[Y];
	shadowMat[2][3] = 0.f - lightpos[W] * groundplane[Z];
	shadowMat[3][3] = dot - lightpos[W] * groundplane[W];

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
GLfloat light_gray[] = {0.3, 0.3, 0.3, 0.3};
GLfloat brown[] = {0.5, 0.35, 0.05, 0.35};
GLfloat random[] = {1.0, 1.0, 1.0, 1.0};
GLfloat position[] = { 0.0, 0.0, 1.5, 1.0 };

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
	glMaterialf(GL_FRONT, GL_SHININESS, 30);
	//draw plane
	glBegin(GL_QUADS);
		glVertex3f( 0,-0.001, 0);
		glVertex3f( 0,-0.001,10);
		glVertex3f(10,-0.001,10);
		glVertex3f(10,-0.001, 0);
	glEnd();

	/* turn texturing on */
	if (textures == 1) {
		glEnable(GL_TEXTURE_2D);
		glBindTexture(GL_TEXTURE_2D, textureID[0]); //change texture
	/* if textured, then use white as base colour */
		glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, light_gray);
	}

	glPushMatrix();

	glTranslatef(5,0.5,5);
	//normals = (Normal *)malloc(sizeof(Normal));

	//glutSolidCube(1);

	glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, white);
	glMaterialfv(GL_FRONT, GL_SPECULAR, white);
	glMaterialf(GL_FRONT, GL_SHININESS, 30);

	glBegin(GL_TRIANGLES);
		//Bottom
		glNormal3f(0.0, -0.5, 0.0);
		glVertex3f(0.5, -0.5, -0.5);
		glVertex3f(0.5, -0.5, 0.5);
		glVertex3f(-0.5, -0.5, 0.5);

		glNormal3f(0.0, -0.5, 0.0);
		glVertex3f(0.5, -0.5, -0.5);
		glVertex3f(-0.5, -0.5, 0.5);
		glVertex3f(-0.5, -0.5, -0.5);

		//Top
		glNormal3f(-0.5, -0.5, -0.5);
		glVertex3f(0.5, 0.5, -0.5);
		glVertex3f(-0.5, 0.5, -0.5);
		glVertex3f(-0.5, 0.5, 0.5);

		glNormal3f(0.5, 0.5, 0.5);
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

	if (textures == 1) 
		glDisable(GL_TEXTURE_2D);

	glRotated ((GLdouble) spin, 0.0, 1.0, 0.0);
	glLightfv (GL_LIGHT0, GL_POSITION, position);

	glTranslated (1.0, 3.0, 3.0);
	glutIdleFunc(lightRotation);
	glutWireCube (0.1);
	glEnable (GL_LIGHTING);

	  /* Reposition the light source. */
	lightPosition[0] = 12*cos(spin);
	lightPosition[1] = lightHeight;
	lightPosition[2] = 12*sin(spin);
	if (directionalLight) {
		lightPosition[3] = 0.0;
	} else {
		lightPosition[3] = 1.0;
	}

	shadowMatrix(floorShadow, floorPlane, lightPosition);
	glMultMatrixf((GLfloat *) floorShadow);

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
			0, 1, 0);   // Up vector
}

void keyboard(unsigned char key, int x, int y)
{
	switch (key) {
		case 27:
		case 'q':
			exit(0);
			break;
		case '1':		// draw polygons as outlines
			lineDrawing = 1;
			lighting = 0;
			smoothShading = 0;
			textures = 0;
			init();
			display();
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
		case '5':		// texture with  smooth shading
			lineDrawing = 0;
			lighting = 1;
			smoothShading = 1;
			textures = 1;
			init();
			display();
			break;
		case 'w':		// draw polygons as outlines
			glTranslatef(0.0, 0.0, 0.5);
			init();
			display();
			break;
		case 'd':		// draw polygons as outlines
			glTranslatef(-1.0, 0.0, 0.0);
			init();
			display();
			break;
		case 'a':		// draw polygons as outlines
			glTranslatef(1.0, 0.0, 0.0);
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

		camX = -(x-500) * -sinf(10*(M_PI/180)) * cosf((45)*(M_PI/180));
		camY = -(x-500) * -sinf((45)*(M_PI/180));
		camZ = (x-500) * cosf((10)*(M_PI/180)) * cosf((45)*(M_PI/180));
		

		glMatrixMode (GL_MODELVIEW);
		glLoadIdentity ();

		gluLookAt(camX,camZ,camY,   // Camera position
		  0, -20, 0,
			0, 1, 0);   // Up vector
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

	for (x = 0; x < 7; x++){
		switch(x) {
			case 0:
				fileName = "brick.ppm";
				break;
			case 1:
				fileName = "horrible.ppm";
				break;
			case 2:
				fileName = "moon.ppm";
				break;
			case 3:
				fileName = "mud.ppm";
				break;
			case 4:
				fileName = "psych.ppm";
				break;
			case 5:
				fileName = "spots.ppm";
				break;
			case 6:
				fileName = "wood.ppm";
		}

		if ((fp = fopen("image.txt", "r")) == 0) {
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

						header++;
						x = 0;
					}

					else if( header == 1) {
						depth = atof(instr);
						header++;
					}
					else {

					}
				}
			}
		}

	}

	for(i=0; i<64; i++) {
		for(j=0; j<64; j++) {
			fscanf(fp, "%d %d %d", &red, &green, &blue);
			Image[i][j][0] = red;
			Image[i][j][1] = green;
			Image[i][j][2] = blue;
			Image[i][j][3] = 255;
		}
	}

	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	glGenTextures(7,textureID);//
	glBindTexture(GL_TEXTURE_2D, textureID[0]); //loop to include 7 files
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 64, 64, 0, GL_RGBA,
		GL_UNSIGNED_BYTE, Image); //64 is from file
	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
	glEnable(GL_TEXTURE_GEN_S);
	glEnable(GL_TEXTURE_GEN_T);

	fclose(fp);
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

	/* Setup floor plane for projected shadow calculations. */
	findPlane(floorPlane, floorVertices[1], floorVertices[2], floorVertices[3]);

	glutMainLoop();
	return 0; 
}

