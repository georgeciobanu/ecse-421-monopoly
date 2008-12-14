
//
// This code based off of NeHe Lesson #7 created by Jeff Molofee '99 
// (ported to Linux/GLUT by Richard Campbell '99)
//
// If you've found this code useful, please let me know.
//
// Visit me at www.demonews.com/hosted/nehe 
// (email Richard Campbell at ulmont@bellsouth.net)
//

#include <fstream>
#include <stdexcept>
#include <string>
#include <vector>

#include <cmath>
using std::sin;

#include <GL/glut.h>
#include <GL/gl.h>
#include <GL/glu.h>

#include "property_coordinates.hpp"

/* ascii codes for various special keys */
#define ESCAPE 27
#define PAGE_UP 73
#define PAGE_DOWN 81
#define UP_ARROW 72
#define DOWN_ARROW 80
#define LEFT_ARROW 75
#define RIGHT_ARROW 77

/* The number of our GLUT window */
int window; 

/* use lighting */
bool light;

/* L pressed */
bool lp;

/* F pressed */
bool fp;


GLfloat xrot;   // x rotation 
GLfloat yrot;   // y rotation 
GLfloat xspeed; // x rotation speed
GLfloat yspeed; // y rotation speed

/* white ambient light at half intensity (rgba) */
GLfloat LightAmbient[] = { 0.5f, 0.5f, 0.5f, 1.0f };

/* super bright, full intensity diffuse light. */
GLfloat LightDiffuse[] = { 1.0f, 1.0f, 1.0f, 1.0f };

/* position of light (x, y, z, (position of light)) */
GLfloat LightPosition[] = { 0.0f, 0.0f, 2.0f, 1.0f };

GLuint	filter;			/* Which Filter To Use (nearest/linear/mipmapped) */
GLuint	texture[3];		/* Storage for 3 textures. */

/* Image type - contains height, width, and data */
struct Image {
    unsigned long width;
    unsigned long height;
    unsigned long bytes_per_pixel;
    std::vector<char> data;
    
    bool load_raw(std::string const &filename) {
        std::ifstream infile(filename.c_str(), std::ios::binary);
        data.resize( width * height * bytes_per_pixel );
        infile.read( &data[0], data.size() );
        return !infile.fail();
    }
    
    Image() {}
    Image(unsigned long w,
          unsigned long h,
          unsigned long bytes_pp,
          std::string const &filename) 
     : width(w), height(h), bytes_per_pixel(bytes_pp) {
        load_raw(filename);
    }
};

// Load Bitmaps And Convert To Textures
GLvoid LoadGLTextures(GLvoid) {	
    // Load Texture
    Image image1;
    
    image1.width = 1024;
    image1.height = 1024;
    image1.bytes_per_pixel = 4;
    if (!image1.load_raw("../data/board_1024x1024_RGBA.raw")) {
	    throw std::runtime_error( "Couldn't load image" );
    }        

    // Create Textures	
    glGenTextures(3, &texture[0]);

    // texture 1 (poor quality scaling)
    glBindTexture(GL_TEXTURE_2D, texture[0]);   // 2d texture (x and y size)

    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_NEAREST); // cheap scaling when image bigger than texture
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_NEAREST); // cheap scaling when image smalled than texture

    // 2d texture, level of detail 0 (normal), 3 components (red, green, blue), x size from image, y size from image, 
    // border 0 (normal), rgb color data, unsigned byte data, and finally the data itself.
    glTexImage2D(GL_TEXTURE_2D, 0, 3, image1.width, image1.height, 0, GL_RGBA, GL_UNSIGNED_BYTE, &(image1.data[0]));

    // texture 2 (linear scaling)
    glBindTexture(GL_TEXTURE_2D, texture[1]);   // 2d texture (x and y size)
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR); // scale linearly when image bigger than texture
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR); // scale linearly when image smalled than texture
    glTexImage2D(GL_TEXTURE_2D, 0, 3, image1.width, image1.height, 0, GL_RGBA, GL_UNSIGNED_BYTE, &(image1.data[0]));

    // texture 3 (mipmapped scaling)
    glBindTexture(GL_TEXTURE_2D, texture[2]);   // 2d texture (x and y size)
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR); // scale linearly when image bigger than texture
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR_MIPMAP_NEAREST); // scale linearly + mipmap when image smalled than texture
    glTexImage2D(GL_TEXTURE_2D, 0, 3, image1.width, image1.height, 0, GL_RGBA, GL_UNSIGNED_BYTE, &(image1.data[0]));

    // 2d texture, 3 colors, width, height, RGB in that order, byte data, and the data.
    gluBuild2DMipmaps(GL_TEXTURE_2D, 3, image1.width, image1.height, GL_RGBA, GL_UNSIGNED_BYTE, &(image1.data[0])); 
};

/* A general OpenGL initialization function.  Sets all of the initial parameters. */
GLvoid InitGL(GLsizei Width, GLsizei Height)	// We call this right after our OpenGL window is created.
{
    LoadGLTextures();                           // load the textures.

    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);	// This Will Clear The Background Color To Black
    glClearDepth(1.0);				// Enables Clearing Of The Depth Buffer
    glDepthFunc(GL_LESS);			// The Type Of Depth Test To Do
    glEnable(GL_DEPTH_TEST);			// Enables Depth Testing
    glShadeModel(GL_SMOOTH);			// Enables Smooth Color Shading
    
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();				// Reset The Projection Matrix
    
    gluPerspective(60.0f,(GLfloat)Width/(GLfloat)Height,0.1f,100.0f);	// Calculate The Aspect Ratio Of The Window
    
    glMatrixMode(GL_MODELVIEW);

    // set up light number 1.
    glLightfv(GL_LIGHT1, GL_AMBIENT, LightAmbient);  // add lighting. (ambient)
    glLightfv(GL_LIGHT1, GL_DIFFUSE, LightDiffuse);  // add lighting. (diffuse).
    glLightfv(GL_LIGHT1, GL_POSITION,LightPosition); // set light position.
    glEnable(GL_LIGHT1);                             // turn light 1 on.
}

/* The function called when our window is resized (which shouldn't happen, because we're fullscreen) */
GLvoid ReSizeGLScene(GLsizei Width, GLsizei Height)
{
    if (Height==0)				// Prevent A Divide By Zero If The Window Is Too Small
	Height=1;

    glViewport(0, 0, Width, Height);		// Reset The Current Viewport And Perspective Transformation

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();

    gluPerspective(60.0f,(GLfloat)Width/(GLfloat)Height,0.1f,100.0f);
    glMatrixMode(GL_MODELVIEW);
}

/* The main drawing function. */
GLvoid DrawGLScene(GLvoid)
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);		// Clear The Screen And The Depth Buffer
    glLoadIdentity();				// Reset The View

    glTranslatef( 0.0f, 0.0f, -0.7f);

    glTranslatef(0.0f,0.0f, cos(yrot*4 * M_PI/180 )/8 );
    
    glRotatef(xrot,1.0f,0.0f,0.0f);		// Rotate On The X Axis

    glEnable(GL_TEXTURE_2D);                    // Enable texture mapping.
    glBindTexture(GL_TEXTURE_2D, texture[filter]);   // choose the texture to use.
    glColor3f( 1, 1, 1 );

    glRotatef( -45, 1, 0, 0 );

    glTranslatef(0, 0.25f, 0); // focus on our side
    glTranslatef(0, 0, 0.1f); // lift it a bit

    glRotatef(yrot,0.0f,0.0f,1.0f);		// Rotate On The Y Axis

    glTranslatef(-0.5f, -0.5f, 0); // center the board

    glBegin(GL_QUADS);		                // begin drawing a cube
    
    // Front Face (note that the texture's corners have to match the quad's corners)
    glNormal3f( 0.0f, 0.0f, 1.0f);                              // front face points out of the screen on z.
    glTexCoord2f(1.0f, 1.0f); glVertex3f( 1.0f,  0.0f,  0.0f);	// Bottom Left Of The Texture and Quad
    glTexCoord2f(0.0f, 1.0f); glVertex3f( 0.0f,  0.0f,  0.0f);	// Bottom Right Of The Texture and Quad
    glTexCoord2f(0.0f, 0.0f); glVertex3f( 0.0f,  1.0f,  0.0f);	// Top Right Of The Texture and Quad
    glTexCoord2f(1.0f, 0.0f); glVertex3f( 1.0f,  1.0f,  0.0f);	// Top Left Of The Texture and Quad
    
    glEnd();                                    // done with the polygon.

    glDisable(GL_TEXTURE_2D);                    // Enable texture mapping.

    glColor3f( 1, 0, 0 );
    for (unsigned i = 0; i < 41; ++i) {
        vector2f p = property_coordinates(i);
        if ( i == 40 ) {
            p = jail_coordinates();
        }
        glPushMatrix();
            glTranslatef( p.x(), p.y(), 0 );
//            glutSolidTeapot( 10./1024 );
            glutSolidSphere( 10./1024, 8, 8 );
        glPopMatrix();
    }

    xrot+=xspeed;		                // X Axis Rotation	
    yrot+=yspeed;		                // Y Axis Rotation

    // since this is double buffered, swap the buffers to display what just got drawn.
    glutSwapBuffers();
}


/* The function called whenever a normal key is pressed. */
void keyPressed(unsigned char key, int x, int y) 
{
    /* avoid thrashing this procedure */
    usleep(100);

    switch (key) {    
    case ESCAPE: // kill everything.
	/* shut down our window */
	glutDestroyWindow(window); 
	
	/* exit the program...normal termination. */
	exit(1);                   	
	break; // redundant.

    case 76: 
    case 108: // switch the lighting.
	printf("L/l pressed; light is: %d\n", light);
	light = light ? 0 : 1;              // switch the current value of light, between 0 and 1.
	printf("Light is now: %d\n", light);
	if (!light) {
	    glDisable(GL_LIGHTING);
	} else {
	    glEnable(GL_LIGHTING);
	}
	break;

    case 70:
    case 102: // switch the filter.
	printf("F/f pressed; filter is: %d\n", filter);
	filter+=1;
	if (filter>2) {
	    filter=0;	
	}	
	printf("Filter is now: %d\n", filter);
	break;

    default:
	break;
    }	
}

/* The function called whenever a normal key is pressed. */
void specialKeyPressed(int key, int x, int y) 
{
    /* avoid thrashing this procedure */
    usleep(100);

    switch (key) {    
    case GLUT_KEY_PAGE_UP: // move the cube into the distance.
//	z-=0.02f;
	break;
    
    case GLUT_KEY_PAGE_DOWN: // move the cube closer.
//	z+=0.02f;
	break;

    case GLUT_KEY_UP: // decrease x rotation speed;
	xspeed-=0.01f;
	break;

    case GLUT_KEY_DOWN: // increase x rotation speed;
	xspeed+=0.01f;
	break;

    case GLUT_KEY_LEFT: // decrease y rotation speed;
	yspeed-=0.01f;
	break;
    
    case GLUT_KEY_RIGHT: // increase y rotation speed;
	yspeed+=0.01f;
	break;

    default:
	break;
    }	
}

int main(int argc, char **argv) 
{  
    /* Initialize GLUT state - glut will take any command line arguments that pertain to it or 
       X Windows - look at its documentation at http://reality.sgi.com/mjk/spec3/spec3.html */  
    glutInit(&argc, argv);  

    /* Select type of Display mode:   
     Double buffer 
     RGBA color
     Alpha components supported 
     Depth buffer */  
    glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE | GLUT_ALPHA | GLUT_DEPTH);  

    /* get a 640 x 480 window */
    glutInitWindowSize(640, 480);  

    /* the window starts at the upper left corner of the screen */
    glutInitWindowPosition(0, 0);  

    /* Open a window */  
    window = glutCreateWindow("Jeff Molofee's GL Code Tutorial ... NeHe '99");  

    /* Register the function to do all our OpenGL drawing. */
    glutDisplayFunc(&DrawGLScene);  

    /* Go fullscreen.  This is as soon as possible. */
//    glutFullScreen();

    /* Even if there are no events, redraw our gl scene. */
    glutIdleFunc(&DrawGLScene);

    /* Register the function called when our window is resized. */
    glutReshapeFunc(&ReSizeGLScene);

    /* Register the function called when the keyboard is pressed. */
    glutKeyboardFunc(&keyPressed);

    /* Register the function called when special keys (arrows, page down, etc) are pressed. */
    glutSpecialFunc(&specialKeyPressed);

    /* Initialize our window. */
    InitGL(640, 480);
  
    /* Start Event Processing Engine */  
    glutMainLoop();  

    return 1;
}

