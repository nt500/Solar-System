#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>

#define _USE_MATH_DEFINES
#include <math.h>

#ifdef WIN32
#include <windows.h>
#pragma warning(disable:4996)
#include "glew.h"
#endif

#include <GL/gl.h>
#include <GL/glu.h>
#include "glut.h"

//	This is a sample OpenGL / GLUT program
//
//	The objective is to draw a 3d object and change the color of the axes
//		with a glut menu
//
//	The left mouse button does rotation
//	The middle mouse button does scaling
//	The user interface allows:
//		1. The axes to be turned on and off
//		2. The color of the axes to be changed
//		3. Debugging to be turned on and off
//		4. Depth cueing to be turned on and off
//		5. The projection to be changed
//		6. The transformations to be reset
//		7. The program to quit
//
//	Author:			Joe Graphics

// NOTE: There are a lot of good reasons to use const variables instead
// of #define's.  However, Visual C++ does not allow a const variable
// to be used as an array size or as the case in a switch( ) statement.  So in
// the following, all constants are const variables except those which need to
// be array sizes or cases in switch( ) statements.  Those are #defines.


// title of these windows:

const char *WINDOWTITLE = { "Final Project -- Neil Thayamballi" };
const char *GLUITITLE   = { "User Interface Window" };


// what the glui package defines as true and false:

const int GLUITRUE  = { true  };
const int GLUIFALSE = { false };


// the escape key:

#define ESCAPE		0x1b


// initial window size:

const int INIT_WINDOW_SIZE = { 600 };


// size of the box:

const float BOXSIZE = { 2.f };



// multiplication factors for input interaction:
//  (these are known from previous experience)

const float ANGFACT = { 1. };
const float SCLFACT = { 0.005f };


// minimum allowable scale factor:

const float MINSCALE = { 0.025f };

// scroll wheel button values:

const int SCROLL_WHEEL_UP   = 3;
const int SCROLL_WHEEL_DOWN = 4;

// equivalent mouse movement when we click the scroll wheel:

const float SCROLL_WHEEL_CLICK_FACTOR = 5.f;


// active mouse buttons (or them together):

const int LEFT   = { 4 };
const int MIDDLE = { 2 };
const int RIGHT  = { 1 };


// which projection:

enum Projections
{
	ORTHO,
	PERSP
};


// which button:

enum ButtonVals
{
	reset,
	QUIT
};


// window background color (rgba):

const GLfloat BACKCOLOR[ ] = { 0., 0., 0., 1. };


// line width for the axes:

const GLfloat AXES_WIDTH   = { 3. };


// the color numbers:
// this order must match the radio button order

enum Colors
{
	RED,
	YELLOW,
	GREEN,
	CYAN,
	BLUE,
	MAGENTA,
	WHITE,
	BLACK
};

char * ColorNames[ ] =
{
	"Red",
	"Yellow",
	"Green",
	"Cyan",
	"Blue",
	"Magenta",
	"White",
	"Black"
};


// the color definitions:
// this order must match the menu order

const GLfloat Colors[ ][3] = 
{
	{ 1., 0., 0. },		// red
	{ 1., 1., 0. },		// yellow
	{ 0., 1., 0. },		// green
	{ 0., 1., 1. },		// cyan
	{ 0., 0., 1. },		// blue
	{ 1., 0., 1. },		// magenta
	{ 1., 1., 1. },		// white
	{ 0., 0., 0. },		// black
};


// fog parameters:

const GLfloat FOGCOLOR[4] = { .0, .0, .0, 1. };
const GLenum  FOGMODE     = { GL_LINEAR };
const GLfloat FOGDENSITY  = { 0.30f };
const GLfloat FOGSTART    = { 1.5 };
const GLfloat FOGEND      = { 4. };


// non-constant global variables:

int		ActiveButton;			// current button that is down
GLuint	AxesList;				// list to hold the axes
int		AxesOn;					// != 0 means to draw the axes
int		DebugOn;				// != 0 means to print debugging info
int		DepthCueOn;				// != 0 means to use intensity depth cueing
int		MainWindow;				// window id for main graphics window
float	Scale;					// scaling factor
int		WhichColor;				// index into Colors[ ]
int		WhichProjection;		// ORTHO or PERSP
int 	WhichView;				// sets the viewing position
int		Xmouse, Ymouse;			// mouse values
float	Xrot, Yrot;				// rotation angles in degrees

GLuint SunList;
GLuint MercuryList;
GLuint VenusList;
GLuint EarthList;
GLuint MarsList;
GLuint JupiterList;
GLuint SaturnList;
GLuint UranusList;
GLuint NeptuneList;
GLuint StarsList;

GLuint SunTexture;
GLuint MercuryTexture;
GLuint VenusTexture;
GLuint EarthTexture;
GLuint MarsTexture;
GLuint JupiterTexture;
GLuint SaturnTexture;
GLuint SaturnRingTexture;
GLuint UranusTexture;
GLuint UranusRingTexture;
GLuint NeptuneTexture;
GLuint StarsTexture;

int Mercury;
int Venus;
int Earth;
int Mars;
int Jupiter;
int Saturn;
int Uranus;
int Neptune;

float White[] = { 1., 1., 1., 1. };
bool Frozen = 0;
float angle = 0;
float Time;

#define MS_PER_CYCLE 100000

struct point {
	float x, y, z;		// coordinates
	float nx, ny, nz;	// surface normal
	float s, t;		// texture coords
};

int	SphNumLngs, SphNumLats;
struct point * SphPts;

struct point * SphPtsPointer(int lat, int lng)
{
	if (lat < 0)	lat += (SphNumLats - 1);
	if (lng < 0)	lng += (SphNumLngs - 1);
	if (lat > SphNumLats - 1)	lat -= (SphNumLats - 1);
	if (lng > SphNumLngs - 1)	lng -= (SphNumLngs - 1);
	return &SphPts[SphNumLngs*lat + lng];
}


struct bmfh
{
	short bfType;
	int bfSize;
	short bfrotationerved1;
	short bfrotationerved2;
	int bfOffBits;
} FileHeader;

struct bmih
{
	int biSize;
	int biWidth;
	int biHeight;
	short biPlanes;
	short biBitCount;
	int biComprotationsion;
	int biSizeImage;
	int biXPelsPerMeter;
	int biYPelsPerMeter;
	int biClrUsed;
	int biClrImportant;
} InfoHeader;

const int birgb = { 0 };

// function prototypes:

void	Animate( );
unsigned char *BmpToTexture( char *, int *, int *);
void	Display( );
void	DoAxesMenu( int );
void	DoColorMenu( int );
void	DoDepthMenu( int );
void	DoDebugMenu( int );
void	DoMainMenu( int );
void	DoProjectMenu( int );
void	DoRasterString( float, float, float, char * );
void	DoStrokeString( float, float, float, float, char * );
void	DoViewMenu(int);
float	ElapsedSeconds( );
void	InitGraphics( );
void	InitLists( );
void	InitMenus( );
void	Keyboard( unsigned char, int, int );
void	MouseButton( int, int, int, int );
void	MouseMotion( int, int );
void 	OsuSphere(float radius, int slices, int stacks);
int		ReadInt( FILE * );
short	ReadShort( FILE * );
void	Reset( );
void	rotationize( int, int );
void	Visibility( int );

void	Axes( float );
void	HsvRgb( float[3], float [3] );

void DrawPoint(struct point *p)
{
	glNormal3f(p->nx, p->ny, p->nz);
	glTexCoord2f(p->s, p->t);
	glVertex3f(p->x, p->y, p->z);
}

void OsuSphere(float radius, int slices, int stacks) {
	// set the globals:

	SphNumLngs = slices;
	SphNumLats = stacks;
	if( SphNumLngs < 3 )
		SphNumLngs = 3;
	if( SphNumLats < 3 )
		SphNumLats = 3;

	// allocate the point data structure:

	SphPts = new struct point[ SphNumLngs * SphNumLats ];

	// fill the SphPts structure:

	for( int ilat = 0; ilat < SphNumLats; ilat++ )
	{
		float lat = -M_PI/2.  +  M_PI * (float)ilat / (float)(SphNumLats-1);	// ilat=0/lat=0. is the south pole
											// ilat=SphNumLats-1, lat=+M_PI/2. is the north pole
		float xz = cosf( lat );
		float  y = sinf( lat );
		for( int ilng = 0; ilng < SphNumLngs; ilng++ )				// ilng=0, lng=-M_PI and
											// ilng=SphNumLngs-1, lng=+M_PI are the same meridian
		{
			float lng = -M_PI  +  2. * M_PI * (float)ilng / (float)(SphNumLngs-1);
			float x =  xz * cosf( lng );
			float z = -xz * sinf( lng );
			struct point* p = SphPtsPointer( ilat, ilng );
			p->x  = radius * x;
			p->y  = radius * y;
			p->z  = radius * z;
			p->nx = x;
			p->ny = y;
			p->nz = z;
			p->s = ( lng + M_PI    ) / ( 2.*M_PI );
			p->t = ( lat + M_PI/2. ) / M_PI;
            p->s = ( lng + M_PI    ) / ( 2.*M_PI );
            p->t = ( lat + M_PI/2. ) / M_PI;										
		}
	}

	struct point top, bot;		// top, bottom points

	top.x =  0.;		top.y  = radius;	top.z = 0.;
	top.nx = 0.;		top.ny = 1.;		top.nz = 0.;
	top.s  = 0.;		top.t  = 1.;

	bot.x =  0.;		bot.y  = -radius;	bot.z = 0.;
	bot.nx = 0.;		bot.ny = -1.;		bot.nz = 0.;
	bot.s  = 0.;		bot.t  =  0.;

	// connect the north pole to the latitude SphNumLats-2:

	glBegin(GL_TRIANGLE_STRIP);
	for (int ilng = 0; ilng < SphNumLngs; ilng++)
	{
		float lng = -M_PI + 2. * M_PI * (float)ilng / (float)(SphNumLngs - 1);
		top.s = (lng + M_PI) / (2. * M_PI);
		DrawPoint(&top);
		struct point* p = SphPtsPointer(SphNumLats - 2, ilng);	// ilat=SphNumLats-1 is the north pole
		DrawPoint(p);
	}
	glEnd();

	// connect the south pole to the latitude 1:

	glBegin( GL_TRIANGLE_STRIP );
	for (int ilng = SphNumLngs - 1; ilng >= 0; ilng--)
	{
		float lng = -M_PI + 2. * M_PI * (float)ilng / (float)(SphNumLngs - 1);
		bot.s = (lng + M_PI) / (2. * M_PI);
		DrawPoint(&bot);
		struct point* p = SphPtsPointer(1, ilng);					// ilat=0 is the south pole
		DrawPoint(p);
	}
	glEnd();

	// connect the horizontal strips:

	for( int ilat = 2; ilat < SphNumLats-1; ilat++ )
	{
		struct point* p;
		glBegin(GL_TRIANGLE_STRIP);
		for( int ilng = 0; ilng < SphNumLngs; ilng++ )
		{
			p = SphPtsPointer( ilat, ilng );
			DrawPoint( p );
			p = SphPtsPointer( ilat-1, ilng );
			DrawPoint( p );
		}
		glEnd();
	}

	// clean-up:

	delete [ ] SphPts;
	SphPts = NULL;
}    

void DrawCircle(float r)
{
	glColor3f(1., 1., 1.);
	glBegin(GL_LINE_STRIP);
	for (int i = 0; i <= 100; i++)
	{
		glVertex3f(r*cos((2 * M_PI*i) / 100), 0., r*sin((2 * M_PI*i) / 100));
	}
	glEnd();
}

void DrawRing(float r1, float r2, int n)
{
	float angle = 0.;
	float changeAngle = 2 * M_PI / n;
	glBegin(GL_QUADS);
	for (int i = 0; i < n; i++)
	{
		glNormal3f(0, 1, 0);
		glTexCoord2f(1, 0);
		glVertex3f(r1*cos(angle), 0., r1*sin(angle));
		glNormal3f(0, 1, 0);
		glTexCoord2f(0, 0);
		glVertex3f(r2*cos(angle), 0., r2*sin(angle));
		glNormal3f(0, 1, 0);
		glTexCoord2f(0, 1);
		glVertex3f(r2*cos(angle+changeAngle), 0., r2*sin(angle+changeAngle));
		glNormal3f(0, 1, 0);
		glTexCoord2f(1, 1);
		glVertex3f(r1*cos(angle + changeAngle), 0., r1*sin(angle + changeAngle));
		angle += changeAngle;
	}
	glEnd();
}

int generateStartingPoint(int range_bottom, int range_top)
{
	int randomNumber;
	int temp;
	temp = range_top - range_bottom;
	randomNumber = rand() % temp + range_bottom;
	return randomNumber;
}

float calculateRotation(float num)
{
	float rotation = num * num * num;
	return rotation;
}

// utility to create an array from 3 separate values:
float *Array3(float a, float b, float c)
{
	static float array[4];
	array[0] = a;
	array[1] = b;
	array[2] = c;
	array[3] = 1.;
	return array;
}

// utility to create an array from 4 separate values:
float * Array4(float r, float g, float b, float a)
{
	static float array[4];
	array[0] = r;
	array[1] = g;
	array[2] = b;
	array[3] = a;
	return array;
}

// utility to create an array from a multiplier and an array:
float *
MulArray3(float factor, float array0[3])
{
	static float array[4];
	array[0] = factor * array0[0];
	array[1] = factor * array0[1];
	array[2] = factor * array0[2];
	array[3] = 1.;
	return array;
}

void
SetMaterial(float r, float g, float b, float shininess)
{
	glMaterialfv(GL_BACK, GL_EMISSION, Array3(0., 0., 0.));
	glMaterialfv(GL_BACK, GL_AMBIENT, MulArray3(.4f, White));
	glMaterialfv(GL_BACK, GL_DIFFUSE, MulArray3(1., White));
	glMaterialfv(GL_BACK, GL_SPECULAR, Array3(0., 0., 0.));
	glMaterialf(GL_BACK, GL_SHININESS, 2.f);
	glMaterialfv(GL_FRONT, GL_EMISSION, Array3(0., 0., 0.));
	glMaterialfv(GL_FRONT, GL_AMBIENT, Array3(r, g, b));
	glMaterialfv(GL_FRONT, GL_DIFFUSE, Array3(r, g, b));
	glMaterialfv(GL_FRONT, GL_SPECULAR, MulArray3(.8f, White));
	glMaterialf(GL_FRONT, GL_SHININESS, shininess);
}

void
SetPointLight(int ilight, float x, float y, float z, float r, float g, float b)
{
	glLightfv(ilight, GL_POSITION, Array3(x, y, z));
	glLightfv(ilight, GL_AMBIENT, Array3(0., 0., 0.));
	glLightfv(ilight, GL_DIFFUSE, Array3(r, g, b));
	glLightfv(ilight, GL_SPECULAR, Array3(r, g, b));
	glLightf(ilight, GL_CONSTANT_ATTENUATION, 1.);
	glLightf(ilight, GL_LINEAR_ATTENUATION, 0.);
	glLightf(ilight, GL_QUADRATIC_ATTENUATION, 0.);
	glEnable(ilight);
}

void
SetSpotLight(int ilight, float x, float y, float z, float xdir, float ydir, float zdir, float r, float g, float b)
{
	glLightfv(ilight, GL_POSITION, Array3(x, y, z));
	glLightfv(ilight, GL_SPOT_DIRECTION, Array3(xdir, ydir, zdir));
	glLightf(ilight, GL_SPOT_EXPONENT, 1.);
	glLightf(ilight, GL_SPOT_CUTOFF, 30.);
	glLightfv(ilight, GL_AMBIENT, Array3(0., 0., 0.));
	glLightfv(ilight, GL_DIFFUSE, Array3(r, g, b));
	glLightfv(ilight, GL_SPECULAR, Array3(r, g, b));
	glLightf(ilight, GL_CONSTANT_ATTENUATION, 1.);
	glLightf(ilight, GL_LINEAR_ATTENUATION, 0.);
	glLightf(ilight, GL_QUADRATIC_ATTENUATION, 0.);
	glEnable(ilight);
}

void
SetParallelLight(int ilight, float x, float y, float z, float r, float g, float b)
{
	glLightfv(ilight, GL_POSITION, Array4(x, y, z, 0.));
	glLightfv(ilight, GL_AMBIENT, Array3(0., 0., 0.));
	glLightfv(ilight, GL_DIFFUSE, Array3(r, g, b));
	glLightfv(ilight, GL_SPECULAR, Array3(r, g, b));
	glLightf(ilight, GL_CONSTANT_ATTENUATION, 1.);
	glLightf(ilight, GL_LINEAR_ATTENUATION, 0.);
	glLightf(ilight, GL_QUADRATIC_ATTENUATION, 0.);
	glEnable(ilight);
}

// main program:

int
main( int argc, char *argv[ ] )
{
	// turn on the glut package:
	// (do this before checking argc and argv since it might
	// pull some command line arguments out)

	glutInit( &argc, argv );


	// setup all the graphics stuff:

	InitGraphics( );


	// create the display structurotation that will not change:

	InitLists( );


	// init all the global variables used by Display( ):
	// this will also post a redisplay

	Reset( );


	// setup all the user interface stuff:

	InitMenus( );


	// draw the scene once and wait for some interaction:
	// (this will never return)

	glutSetWindow( MainWindow );
	glutMainLoop( );


	// this is here to make the compiler happy:

	return 0;
}


// this is where one would put code that is to be called
// everytime the glut main loop has nothing to do
//
// this is typically where animation parameters are set
//
// do not call Display( ) from here -- let glutMainLoop( ) do it

void
Animate( )
{
	// put animation stuff in here -- change some global variables
	// for Display( ) to find:

	// force a call to Display( ) next time it is convenient:
	int ms = glutGet(GLUT_ELAPSED_TIME);
	int msp = ms;
	ms %= MS_PER_CYCLE;
	Time = (float)ms / (float)(MS_PER_CYCLE - 1);

	glutSetWindow( MainWindow );
	glutPostRedisplay( );
}


// draw the complete scene:

void
Display( )
{
	if( DebugOn != 0 )
	{
		fprintf( stderr, "Display\n" );
	}


	// set which window we want to do the graphics into:

	glutSetWindow( MainWindow );


	// erase the background:

	glDrawBuffer( GL_BACK );
	glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
	glEnable( GL_DEPTH_TEST );


	// specify shading to be smooth:

	glShadeModel(GL_SMOOTH);

	// set the viewport to a square centered in the window:

	GLsizei vx = glutGet( GLUT_WINDOW_WIDTH );
	GLsizei vy = glutGet( GLUT_WINDOW_HEIGHT );
	GLsizei v = vx < vy ? vx : vy;			// minimum dimension
	GLint xl = ( vx - v ) / 2;
	GLint yb = ( vy - v ) / 2;
	glViewport( xl, yb,  v, v );


	// set the viewing volume:
	// remember that the Z clipping  values are actually
	// given as DISTANCES IN FRONT OF THE EYE
	// USE gluOrtho2D( ) IF YOU ARE DOING 2D !

	glMatrixMode( GL_PROJECTION );
	glLoadIdentity( );
	if( WhichProjection == ORTHO )
		glOrtho( -3., 3.,     -3., 3.,     0.1, 2000. );
	else
		gluPerspective( 90., 1.,	0.1, 2000. );


	// place the objects into the scene:

	glMatrixMode( GL_MODELVIEW );
	glLoadIdentity( );


	// set the eye position, look-at position, and up-vector:
	if (WhichView == 0)
	{
		gluLookAt(0., 0., 50., 0., 0., 0., 0., 1., 0.);
	}
	else if (WhichView == 1)
	{
		gluLookAt(20., 4., 20., 20., 5., 10., 0., 1., 0.);	
	}

	// set parallel light:

	SetParallelLight(GL_LIGHT1, 1., 1., 0.75, 1., 1., 1.);

	// rotate the scene:

	glRotatef( (GLfloat)Yrot, 0., 1., 0. );
	glRotatef( (GLfloat)Xrot, 1., 0., 0. );


	// uniformly scale the scene:

	if( Scale < MINSCALE )
		Scale = MINSCALE;
	glScalef( (GLfloat)Scale, (GLfloat)Scale, (GLfloat)Scale );

	// set the fog parameters:

	if( DepthCueOn != 0 )
	{
		glFogi( GL_FOG_MODE, FOGMODE );
		glFogfv( GL_FOG_COLOR, FOGCOLOR );
		glFogf( GL_FOG_DENSITY, FOGDENSITY );
		glFogf( GL_FOG_START, FOGSTART );
		glFogf( GL_FOG_END, FOGEND );
		glEnable( GL_FOG );
	}
	else
	{
		glDisable( GL_FOG );
	}


	// possibly draw the axes:

	if( AxesOn != 0 )
	{
		glColor3fv( &Colors[WhichColor][0] );
		glCallList( AxesList );
	}


	// since we are using glScalef( ), be sure normals get unitized:

	glEnable( GL_NORMALIZE );

	glLightModelfv(GL_LIGHT_MODEL_AMBIENT, MulArray3(.2, White));
	glLightModeli(GL_LIGHT_MODEL_TWO_SIDE, GL_FALSE);

	// Point lighting from the sun
	SetPointLight(GL_LIGHT0, 0., 0., 0., 1., 1., 1.);
	// draw the current object:

	glEnable(GL_LIGHTING);
	glDisable(GL_LIGHT1);
	glEnable(GL_LIGHT0);
	glCallList(StarsList);

	glPushMatrix();
	glRotatef(45., 0., 1., 0.);
	glRotatef(30., 0., 0., 1.);
	// Sun
	glPushMatrix();
	glRotatef(360 * Time, 0., 1., 0.);
	glCallList(SunList);
	glPopMatrix();
	
	// Mercury
	glDisable(GL_LIGHTING);
	DrawCircle(6.25);
	glEnable(GL_LIGHTING);
	glPushMatrix();
	glRotatef(360 * Time * sqrt(calculateRotation(38.)) / sqrt(calculateRotation(6.25)), 0., 1., 0.);
	glRotated(Mercury, 0, 1, 0);
	glTranslatef(6.25, 0., 0.);
	glRotatef(360 * Mercury/10 * Time, 0., 1., 0.);
	SetMaterial(1.0, 1.0, 1.0, 2.);
	glCallList( MercuryList );
	glPopMatrix();
	
	// Venus
	glDisable(GL_LIGHTING);
	DrawCircle(8.25);
	glEnable(GL_LIGHTING);
	glPushMatrix();
	glRotatef(360 * Time * sqrt(calculateRotation(38.)) / sqrt(calculateRotation(8.25)), 0., 1., 0.);
	glRotated(Venus, 0, 1, 0);
	glTranslatef(8.25, 0., 0.);
	glRotatef(360 * Venus/10 * Time, 0., 1., 0.);
	SetMaterial(1.0, 1.0, 1.0, 2.);
	glCallList( VenusList );
	glPopMatrix();
	
	// Earth
	glDisable(GL_LIGHTING);
	DrawCircle(11.5);
	glEnable(GL_LIGHTING);
	glPushMatrix();
	glRotatef(360 * Time * sqrt(calculateRotation(38.)) / sqrt(calculateRotation(11.5)), 0., 1., 0.);
	glRotated(Earth, 0, 1, 0);
	glTranslatef(11.5, 0., 0.);
	glRotatef(360 * Earth/10 * Time, 0., 1., 0.);
	SetMaterial(1.0, 1.0, 1.0, 2.);
	glCallList( EarthList );
	glDisable(GL_LIGHTING);
	glPopMatrix();
	
	// Mars
	glDisable(GL_LIGHTING);
	DrawCircle(14.5);
	glEnable(GL_LIGHTING);
	glPushMatrix();
	glRotatef(360 * Time * sqrt(calculateRotation(38.)) / sqrt(calculateRotation(14.5)), 0., 1., 0.);
	glRotated(Mars, 0, 1, 0);
	glTranslatef(14.5, 0., 0.);
	glRotatef(360 * Mars/10 * Time, 0., 1., 0.);
	SetMaterial(1.0, 1.0, 1.0, 2.);
	glCallList( MarsList );
	glPopMatrix();
	
	// Jupiter
	glDisable(GL_LIGHTING);
	DrawCircle(18.25);
	glEnable(GL_LIGHTING);
	glPushMatrix();
	glRotatef(360 * Time * sqrt(calculateRotation(38.)) / sqrt(calculateRotation(18.25)), 0., 1., 0.);
	glRotated(Jupiter, 0, 1, 0);
	glTranslatef(18.25, 0., 0.);
	glRotatef(360 * Jupiter/10 * Time, 0., 1., 0.);
	SetMaterial(1.0, 1.0, 1.0, 2.);
	glCallList( JupiterList );
	glPopMatrix();
	
	// Saturn
	glDisable(GL_LIGHTING);
	DrawCircle(23.75);
	glEnable(GL_LIGHTING);
	glPushMatrix();
	glRotatef(360 * Time * sqrt(calculateRotation(38.)) / sqrt(calculateRotation(23.75)), 0., 1., 0.);
	glRotated(Saturn, 0, 1, 0);
	glTranslatef(23.75, 0., 0.);
	glRotatef(40., -1., 0., 1.);
	glRotatef(360 * Saturn / 10 * Time, 0., 1., 0.);
	SetMaterial(1.0, 1.0, 1.0, 2.);
	glCallList( SaturnList );
	glPopMatrix();
	
	// Uranus
	glDisable(GL_LIGHTING);
	DrawCircle(29.75);
	glEnable(GL_LIGHTING);
	glPushMatrix();
	glRotatef(360 * Time * sqrt(calculateRotation(38.)) / sqrt(calculateRotation(29.75)), 0., 1., 0.);
	glRotated(Uranus, 0, 1, 0);
	glTranslatef(29.75, 0., 0.);
	glRotatef(70., 1., 0.35, 0.25);
	glRotatef(360 * Uranus / 10 * Time, 0., 1., 0.);
	SetMaterial(1.0, 1.0, 1.0, 2.);
	glCallList( UranusList );
	glPopMatrix();
	
	// Neptune
	glDisable(GL_LIGHTING);
	DrawCircle(35.);
	glEnable(GL_LIGHTING);
	glPushMatrix();
	glRotatef(360 * Time * sqrt(calculateRotation(38.)) / sqrt(calculateRotation(35.)), 0., 1., 0.);
	glRotated(Neptune, 0, 1, 0);
	glTranslatef(35., 0., 0.);
	glRotatef(360 * Neptune / 10 * Time, 0., 1., 0.);
	SetMaterial(1.0, 1.0, 1.0, 2.);
	glCallList( NeptuneList );
	glPopMatrix();
	
	glPopMatrix();

	glDisable(GL_LIGHTING);

	// draw some gratuitous text that is fixed on the screen:
	//
	// the projection matrix is reset to define a scene whose
	// world coordinate system goes from 0-100 in each axis
	//
	// this is called "percent units", and is just a convenience
	//
	// the modelview matrix is reset to identity as we don't
	// want to transform these coordinates

	glDisable( GL_DEPTH_TEST );
	glMatrixMode( GL_PROJECTION );
	glLoadIdentity( );
	gluOrtho2D( 0., 100.,     0., 100. );
	glMatrixMode( GL_MODELVIEW );
	glLoadIdentity( );
	glColor3f( 1., 1., 1. );
	DoRasterString( 5., 5., 0., "Solar System" );


	// swap the double-buffered framebuffers:

	glutSwapBuffers( );


	// be sure the graphics buffer has been sent:
	// note: be sure to use glFlush( ) here, not glFinish( ) !

	glFlush( );
}


void
DoAxesMenu( int id )
{
	AxesOn = id;

	glutSetWindow( MainWindow );
	glutPostRedisplay( );
}


void
DoColorMenu( int id )
{
	WhichColor = id - RED;

	glutSetWindow( MainWindow );
	glutPostRedisplay( );
}


void
DoDebugMenu( int id )
{
	DebugOn = id;

	glutSetWindow( MainWindow );
	glutPostRedisplay( );
}


void
DoDepthMenu( int id )
{
	DepthCueOn = id;

	glutSetWindow( MainWindow );
	glutPostRedisplay( );
}


// main menu callback:

void
DoMainMenu( int id )
{
	switch( id )
	{
		case reset:
			Reset( );
			break;

		case QUIT:
			// gracefully close out the graphics:
			// gracefully close the graphics window:
			// gracefully exit the program:
			glutSetWindow( MainWindow );
			glFinish( );
			glutDestroyWindow( MainWindow );
			exit( 0 );
			break;

		default:
			fprintf( stderr, "Don't know what to do with Main Menu ID %d\n", id );
	}

	glutSetWindow( MainWindow );
	glutPostRedisplay( );
}


void
DoProjectMenu( int id )
{
	WhichProjection = id;

	glutSetWindow( MainWindow );
	glutPostRedisplay( );
}

// use glut to display a string of characters using a raster font:

void
DoRasterString( float x, float y, float z, char *s )
{
	glRasterPos3f( (GLfloat)x, (GLfloat)y, (GLfloat)z );

	char c;			// one character to print
	for( ; ( c = *s ) != '\0'; s++ )
	{
		glutBitmapCharacter( GLUT_BITMAP_TIMES_ROMAN_24, c );
	}
}


// use glut to display a string of characters using a stroke font:

void
DoStrokeString( float x, float y, float z, float ht, char *s )
{
	glPushMatrix( );
		glTranslatef( (GLfloat)x, (GLfloat)y, (GLfloat)z );
		float sf = ht / ( 119.05f + 33.33f );
		glScalef( (GLfloat)sf, (GLfloat)sf, (GLfloat)sf );
		char c;			// one character to print
		for( ; ( c = *s ) != '\0'; s++ )
		{
			glutStrokeCharacter( GLUT_STROKE_ROMAN, c );
		}
	glPopMatrix( );
}


void DoViewMenu(int id)
{
	WhichView = id;

	glutSetWindow(MainWindow);
	glutPostRedisplay();
}


// return the number of seconds since the start of the program:

float
ElapsedSeconds( )
{
	// get # of milliseconds since the start of the program:

	int ms = glutGet( GLUT_ELAPSED_TIME );

	// convert it to seconds:

	return (float)ms / 1000.f;
}


// initialize the glui window:

void
InitMenus( )
{
	glutSetWindow( MainWindow );

	int numColors = sizeof( Colors ) / ( 3*sizeof(int) );
	int colormenu = glutCreateMenu( DoColorMenu );
	for( int i = 0; i < numColors; i++ )
	{
		glutAddMenuEntry( ColorNames[i], i );
	}

	int axesmenu = glutCreateMenu( DoAxesMenu );
	glutAddMenuEntry( "Off",  0 );
	glutAddMenuEntry( "On",   1 );

	int depthcuemenu = glutCreateMenu( DoDepthMenu );
	glutAddMenuEntry( "Off",  0 );
	glutAddMenuEntry( "On",   1 );

	int debugmenu = glutCreateMenu( DoDebugMenu );
	glutAddMenuEntry( "Off",  0 );
	glutAddMenuEntry( "On",   1 );

	int projmenu = glutCreateMenu( DoProjectMenu );
	glutAddMenuEntry( "Orthographic",  ORTHO );
	glutAddMenuEntry( "Perspective",   PERSP );

	int viewmenu = glutCreateMenu( DoViewMenu );
	glutAddMenuEntry("Outside", 0);
	glutAddMenuEntry("Inside", 1);

	int mainmenu = glutCreateMenu( DoMainMenu );
	glutAddSubMenu(   "Axes",          axesmenu);
	glutAddSubMenu(   "Colors",        colormenu);
	glutAddSubMenu(   "Depth Cue",     depthcuemenu);
	glutAddSubMenu(   "Projection",    projmenu );
	glutAddMenuEntry( "Reset",         reset );
	glutAddSubMenu(   "Debug",         debugmenu);
	glutAddSubMenu(   "View",   	   viewmenu);
	glutAddMenuEntry( "Quit",          QUIT );

// attach the pop-up menu to the right mouse button:

	glutAttachMenu( GLUT_RIGHT_BUTTON );
}



// initialize the glut and OpenGL libraries:
//	also setup display lists and callback functions

void
InitGraphics( )
{
	unsigned char *Texture;
	int Width, Height;
	// request the display modes:
	// ask for red-green-blue-alpha color, double-buffering, and z-buffering:

	glutInitDisplayMode( GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH );

	// set the initial window configuration:

	glutInitWindowPosition( 0, 0 );
	glutInitWindowSize( INIT_WINDOW_SIZE, INIT_WINDOW_SIZE );

	// open the window and set its title:

	MainWindow = glutCreateWindow( WINDOWTITLE );
	glutSetWindowTitle( WINDOWTITLE );

	// set the framebuffer clear values:

	glClearColor( BACKCOLOR[0], BACKCOLOR[1], BACKCOLOR[2], BACKCOLOR[3] );

	// setup the callback functions:
	// DisplayFunc -- redraw the window
	// ReshapeFunc -- handle the user rotationizing the window
	// KeyboardFunc -- handle a keyboard input
	// MouseFunc -- handle the mouse button going down or up
	// MotionFunc -- handle the mouse moving with a button down
	// PassiveMotionFunc -- handle the mouse moving with a button up
	// VisibilityFunc -- handle a change in window visibility
	// EntryFunc	-- handle the cursor entering or leaving the window
	// SpecialFunc -- handle special keys on the keyboard
	// SpaceballMotionFunc -- handle spaceball translation
	// SpaceballRotateFunc -- handle spaceball rotation
	// SpaceballButtonFunc -- handle spaceball button hits
	// ButtonBoxFunc -- handle button box hits
	// DialsFunc -- handle dial rotations
	// TabletMotionFunc -- handle digitizing tablet motion
	// TabletButtonFunc -- handle digitizing tablet button hits
	// MenuStateFunc -- declare when a pop-up menu is in use
	// TimerFunc -- trigger something to happen a certain time from now
	// IdleFunc -- what to do when nothing else is going on

	glutSetWindow( MainWindow );
	glutDisplayFunc( Display );
	glutReshapeFunc( rotationize );
	glutKeyboardFunc( Keyboard );
	glutMouseFunc( MouseButton );
	glutMotionFunc( MouseMotion );
	glutPassiveMotionFunc( NULL );
	glutVisibilityFunc( Visibility );
	glutEntryFunc( NULL );
	glutSpecialFunc( NULL );
	glutSpaceballMotionFunc( NULL );
	glutSpaceballRotateFunc( NULL );
	glutSpaceballButtonFunc( NULL );
	glutButtonBoxFunc( NULL );
	glutDialsFunc( NULL );
	glutTabletMotionFunc( NULL );
	glutTabletButtonFunc( NULL );
	glutMenuStateFunc( NULL );
	glutTimerFunc( -1, NULL, 0 );
	glutIdleFunc( Animate );

	// Sun Texture Mapping
	Texture = BmpToTexture("sun.bmp", &Width, &Height);
	if (Texture == NULL || Width != 2048 || Height != 1024)
	{
		printf("Wrong BMP File!");
		exit(1);
	}
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	glGenTextures(1, &SunTexture);
	glBindTexture(GL_TEXTURE_2D, SunTexture);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
	glTexImage2D(GL_TEXTURE_2D, 0, 3, Width, Height, 0, GL_RGB, GL_UNSIGNED_BYTE, Texture);
	// Mercury Texture Mapping
	Texture = BmpToTexture("mercury.bmp", &Width, &Height);
	if (Texture == NULL || Width != 2048 || Height != 1024)
	{
		printf("Wrong BMP File!");
		exit(1);
	}
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	glGenTextures(1, &MercuryTexture);
	glBindTexture(GL_TEXTURE_2D, MercuryTexture);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
	glTexImage2D(GL_TEXTURE_2D, 0, 3, Width, Height, 0, GL_RGB, GL_UNSIGNED_BYTE, Texture);
	// Venus Texture Mapping
	Texture = BmpToTexture("venus.bmp", &Width, &Height);
	if (Texture == NULL || Width != 2048 || Height != 1024)
	{
		printf("Wrong BMP File!");
		exit(1);
	}
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	glGenTextures(1, &VenusTexture);
	glBindTexture(GL_TEXTURE_2D, VenusTexture);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
	glTexImage2D(GL_TEXTURE_2D, 0, 3, Width, Height, 0, GL_RGB, GL_UNSIGNED_BYTE, Texture);
	// Earth Texture Mapping
	Texture = BmpToTexture("worldtex.bmp", &Width, &Height);
	if (Texture == NULL || Width != 1024 || Height != 512)
	{
		printf("Wrong BMP File!");
		exit(1);
	}
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	glGenTextures(1, &EarthTexture);
	glBindTexture(GL_TEXTURE_2D, EarthTexture);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
	glTexImage2D(GL_TEXTURE_2D, 0, 3, Width, Height, 0, GL_RGB, GL_UNSIGNED_BYTE, Texture);
	// Mars Texture Mapping
	Texture = BmpToTexture("mars.bmp", &Width, &Height);
	if (Texture == NULL || Width != 2048 || Height != 1024)
	{
		printf("Wrong BMP File!");
		exit(1);
	}
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	glGenTextures(1, &MarsTexture);
	glBindTexture(GL_TEXTURE_2D, MarsTexture);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
	glTexImage2D(GL_TEXTURE_2D, 0, 3, Width, Height, 0, GL_RGB, GL_UNSIGNED_BYTE, Texture);
	// Jupiter Texture Mapping
	Texture = BmpToTexture("jupiter.bmp", &Width, &Height);
	if (Texture == NULL || Width != 2048 || Height != 1024)
	{
		printf("Wrong BMP File!");
		exit(1);
	}
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	glGenTextures(1, &JupiterTexture);
	glBindTexture(GL_TEXTURE_2D, JupiterTexture);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
	glTexImage2D(GL_TEXTURE_2D, 0, 3, Width, Height, 0, GL_RGB, GL_UNSIGNED_BYTE, Texture);
	// Saturn Texture Mapping
	Texture = BmpToTexture("saturn.bmp", &Width, &Height);
	if (Texture == NULL || Width != 2048 || Height != 1024)
	{
		printf("Wrong BMP File!");
		exit(1);
	}
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	glGenTextures(1, &SaturnTexture);
	glBindTexture(GL_TEXTURE_2D, SaturnTexture);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
	glTexImage2D(GL_TEXTURE_2D, 0, 3, Width, Height, 0, GL_RGB, GL_UNSIGNED_BYTE, Texture);
	// Saturn Ring Texture Mapping
	Texture = BmpToTexture("saturnring.bmp", &Width, &Height);
	if (Texture == NULL || Width != 2048 || Height != 125)
	{
		printf("Wrong BMP File!");
		exit(1);
	}
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	glGenTextures(1, &SaturnRingTexture);
	glBindTexture(GL_TEXTURE_2D, SaturnRingTexture);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
	glTexImage2D(GL_TEXTURE_2D, 0, 3, Width, Height, 0, GL_RGB, GL_UNSIGNED_BYTE, Texture);
	// Uranus Texture Mapping
	Texture = BmpToTexture("uranus.bmp", &Width, &Height);
	if (Texture == NULL || Width != 2048 || Height != 1024)
	{
		printf("Wrong BMP File!");
		exit(1);
	}
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	glGenTextures(1, &UranusTexture);
	glBindTexture(GL_TEXTURE_2D, UranusTexture);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
	glTexImage2D(GL_TEXTURE_2D, 0, 3, Width, Height, 0, GL_RGB, GL_UNSIGNED_BYTE, Texture);
	// Uranus Ring Texture Mapping
	Texture = BmpToTexture("uranusring.bmp", &Width, &Height);
	if (Texture == NULL || Width != 1024 || Height != 72)
	{
		printf("Wrong BMP File!");
		exit(1);
	}
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	glGenTextures(1, &UranusRingTexture);
	glBindTexture(GL_TEXTURE_2D, UranusRingTexture);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
	glTexImage2D(GL_TEXTURE_2D, 0, 3, Width, Height, 0, GL_RGB, GL_UNSIGNED_BYTE, Texture);
	// Neptune Texture Mapping
	Texture = BmpToTexture("neptune.bmp", &Width, &Height);
	if (Texture == NULL || Width != 2048 || Height != 1024)
	{
		printf("Wrong BMP File!");
		exit(1);
	}
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	glGenTextures(1, &NeptuneTexture);
	glBindTexture(GL_TEXTURE_2D, NeptuneTexture);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
	glTexImage2D(GL_TEXTURE_2D, 0, 3, Width, Height, 0, GL_RGB, GL_UNSIGNED_BYTE, Texture);

	// Generate Starting Points
	Mercury = generateStartingPoint(0, 360);
	Venus = generateStartingPoint(0, 360);
	Earth = generateStartingPoint(0, 360);
	Mars = generateStartingPoint(0, 360);
	Jupiter = generateStartingPoint(0, 360);
	Saturn = generateStartingPoint(0, 360);
	Uranus = generateStartingPoint(0, 360);
	Neptune = generateStartingPoint(0, 360);

	// Stars Texture
	Texture = BmpToTexture("stars.bmp", &Width, &Height);
	if (Texture == NULL || Width != 1470 || Height != 980)
	{
		printf("Wrong BMP File!");
		exit(1);
	}
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	glGenTextures(1, &StarsTexture);
	glBindTexture(GL_TEXTURE_2D, StarsTexture);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
	glTexImage2D(GL_TEXTURE_2D, 0, 3, Width, Height, 0, GL_RGB, GL_UNSIGNED_BYTE, Texture);

	// init glew (a window must be open to do this):

#ifdef WIN32
	GLenum err = glewInit( );
	if( err != GLEW_OK )
	{
		fprintf( stderr, "glewInit Error\n" );
	}
	else
		fprintf( stderr, "GLEW initialized OK\n" );
	fprintf( stderr, "Status: Using GLEW %s\n", glewGetString(GLEW_VERSION));
#endif

}


// initialize the display lists that will not change:
// (a display list is a way to store opengl commands in
//  memory so that they can be played back efficiently at a later time
//  with a call to glCallList( )

void
InitLists( )
{
	float dx = BOXSIZE / 2.f;
	float dy = BOXSIZE / 2.f;
	float dz = BOXSIZE / 2.f;
	glutSetWindow( MainWindow );

	// Sun Display List:
	SunList = glGenLists( 1 );
	glNewList( SunList, GL_COMPILE );
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, SunTexture);
	glColor3f(1., 1., 1.);
	OsuSphere(5., 100, 100);
	glDisable(GL_TEXTURE_2D);
	glEndList( );
	// Mercury Display List:
	MercuryList = glGenLists(1);
	glNewList(MercuryList, GL_COMPILE);
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, MercuryTexture);
	glColor3f(1., 1., 1.);
	OsuSphere(0.5, 100, 100);
	glDisable(GL_TEXTURE_2D);
	glEndList();
	// Venus Display List:
	VenusList = glGenLists(1);
	glNewList(VenusList, GL_COMPILE);
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, VenusTexture);
	glColor3f(1., 1., 1.);
	OsuSphere(1., 100, 100);
	glDisable(GL_TEXTURE_2D);
	glEndList();
	// Earth Display List:
	EarthList = glGenLists(1);
	glNewList(EarthList, GL_COMPILE);
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, EarthTexture);
	glColor3f(1., 1., 1.);
	OsuSphere(1., 100, 100);
	glDisable(GL_TEXTURE_2D);
	glEndList();
	// Mars Display List:
	MarsList = glGenLists(1);
	glNewList(MarsList, GL_COMPILE);
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, MarsTexture);
	glColor3f(1., 1., 1.);
	OsuSphere(0.75, 100, 100);
	glDisable(GL_TEXTURE_2D);
	glEndList();
	// Jupiter Display List:
	JupiterList = glGenLists(1);
	glNewList(JupiterList, GL_COMPILE);
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, JupiterTexture);
	glColor3f(1., 1., 1.);
	OsuSphere(2.125, 100, 100);
	glDisable(GL_TEXTURE_2D);
	glEndList();
	// Saturn Display List:
	SaturnList = glGenLists(1);
	glNewList(SaturnList, GL_COMPILE);
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, SaturnTexture);
	glColor3f(1., 1., 1.);
	OsuSphere(2., 100, 100);
	glBindTexture(GL_TEXTURE_2D, SaturnRingTexture);
	glColor3f(1., 1., 1.);
	DrawRing(2.25, 3.25, 50);
	glDisable(GL_TEXTURE_2D);
	glEndList();
	// Uranus Display List:
	UranusList = glGenLists(1);
	glNewList(UranusList, GL_COMPILE);
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, UranusTexture);
	glColor3f(1., 1., 1.);
	OsuSphere(1.625, 100, 100);
	glBindTexture(GL_TEXTURE_2D, UranusRingTexture);
	glColor3f(1., 1., 1.);
	DrawRing(1.75, 2.375, 50);
	glDisable(GL_TEXTURE_2D);
	glEndList();
	// Neptune Display List:
	NeptuneList = glGenLists(1);
	glNewList(NeptuneList, GL_COMPILE);
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, NeptuneTexture);
	glColor3f(1., 1., 1.);
	OsuSphere(1.625, 100, 100);
	glDisable(GL_TEXTURE_2D);
	glEndList();

	glBegin(GL_POLYGON);
	glNormal3f(0., 1., 0.);
	for (int i = 0; i <= 100; i++)
	{
		glTexCoord2f(1 - sin(0.01 * M_PI * i), 0.5 + 0.5 * sin(0.02 * M_PI * i));
		glVertex3f(40. * cos(0.02 * M_PI * i), -7.5, 40. * sin(0.02 * M_PI * i));
	}
	glEnd();

	glDisable(GL_TEXTURE_2D);
	glEndList();
	// Stars Display List:
	StarsList = glGenLists(1);
	glNewList(StarsList, GL_COMPILE);
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, StarsTexture);
	glColor3f(1., 1., 1.);

	glBegin(GL_QUADS);
	glTexCoord2f(0., 0.);
	glVertex3f(100., 100., -100);
	glTexCoord2f(1., 0.);
	glVertex3f(100., -100., -100);
	glTexCoord2f(1., 1.);
	glVertex3f(-100., -100., -100);
	glTexCoord2f(0., 1.);
	glVertex3f(-100., 100., -100);
	glEnd();
	
	glBegin(GL_QUADS);
	glTexCoord2f(0., 0.);
	glVertex3f(100., 100., 100);
	glTexCoord2f(1., 0.);
	glVertex3f(100., -100., 100);
	glTexCoord2f(1., 1.);
	glVertex3f(-100., -100., 100);
	glTexCoord2f(0., 1.);
	glVertex3f(-100., 100., 100);
	glEnd();

	glBegin(GL_QUADS);
	glTexCoord2f(0., 0.);
	glVertex3f(100., -100., 100);
	glTexCoord2f(1., 0.);
	glVertex3f(100., -100., -100);
	glTexCoord2f(1., 1.);
	glVertex3f(-100., -100., -100);
	glTexCoord2f(0., 1.);
	glVertex3f(-100., -100., 100);
	glEnd();

	glBegin(GL_QUADS);
	glTexCoord2f(0., 0.);
	glVertex3f(100., 100., 100);
	glTexCoord2f(1., 0.);
	glVertex3f(100., 100., -100);
	glTexCoord2f(1., 1.);
	glVertex3f(-100., 100., -100);
	glTexCoord2f(0., 1.);
	glVertex3f(-100., 100., 100);
	glEnd();

	glBegin(GL_QUADS);
	glTexCoord2f(0., 0.);
	glVertex3f(100., 100., 100);
	glTexCoord2f(1., 0.);
	glVertex3f(100., 100., -100);
	glTexCoord2f(1., 1.);
	glVertex3f(100., -100., -100);
	glTexCoord2f(0., 1.);
	glVertex3f(100., -100., 100);
	glEnd();

	glBegin(GL_QUADS);
	glTexCoord2f(0., 0.);
	glVertex3f(-100., 100., 100);
	glTexCoord2f(1., 0.);
	glVertex3f(-100., 100., -100);
	glTexCoord2f(1., 1.);
	glVertex3f(-100., -100., -100);
	glTexCoord2f(0., 1.);
	glVertex3f(-100., -100., 100);
	glEnd();
	glDisable(GL_TEXTURE_2D);
	glEndList();
	// create the axes:

	AxesList = glGenLists( 1 );
	glNewList( AxesList, GL_COMPILE );
		glLineWidth( AXES_WIDTH );
			Axes( 1.5 );
		glLineWidth( 1. );
	glEndList( );
}


// the keyboard callback:

void
Keyboard( unsigned char c, int x, int y )
{
	if( DebugOn != 0 )
		fprintf( stderr, "Keyboard: '%c' (0x%0x)\n", c, c );

	switch( c )
	{
		case 'o':
		case 'O':
			WhichProjection = ORTHO;
			break;

		case 'p':
		case 'P':
			WhichProjection = PERSP;
			break;

		case 'q':
		case 'Q':
		case ESCAPE:
			DoMainMenu( QUIT );	// will not return here
			break;				// happy compiler

		case 'f': case 'F':
            Frozen = !Frozen;
            if (Frozen) glutIdleFunc(NULL);
            else glutIdleFunc(Animate);
            break;

		case '1':
			WhichView = 1;
			break;

		case '0':
			WhichView = 0;
			break;		

		default:
			fprintf( stderr, "Don't know what to do with keyboard hit: '%c' (0x%0x)\n", c, c );
	}

	// force a call to Display( ):

	glutSetWindow( MainWindow );
	glutPostRedisplay( );
}


// called when the mouse button transitions down or up:

void
MouseButton( int button, int state, int x, int y )
{
	int b = 0;			// LEFT, MIDDLE, or RIGHT

	if( DebugOn != 0 )
		fprintf( stderr, "MouseButton: %d, %d, %d, %d\n", button, state, x, y );

	
	// get the proper button bit mask:

	switch( button )
	{
		case GLUT_LEFT_BUTTON:
			b = LEFT;		break;

		case GLUT_MIDDLE_BUTTON:
			b = MIDDLE;		break;

		case GLUT_RIGHT_BUTTON:
			b = RIGHT;		break;

		case SCROLL_WHEEL_UP:
			Scale += SCLFACT * SCROLL_WHEEL_CLICK_FACTOR;
			// keep object from turning inside-out or disappearing:
			if (Scale < MINSCALE)
				Scale = MINSCALE;
			break;

		case SCROLL_WHEEL_DOWN:
			Scale -= SCLFACT * SCROLL_WHEEL_CLICK_FACTOR;
			// keep object from turning inside-out or disappearing:
			if (Scale < MINSCALE)
				Scale = MINSCALE;
			break;

		default:
			b = 0;
			fprintf( stderr, "Unknown mouse button: %d\n", button );
	}


	// button down sets the bit, up clears the bit:

	if( state == GLUT_DOWN )
	{
		Xmouse = x;
		Ymouse = y;
		ActiveButton |= b;		// set the proper bit
	}
	else
	{
		ActiveButton &= ~b;		// clear the proper bit
	}
}


// called when the mouse moves while a button is down:

void
MouseMotion( int x, int y )
{
	if( DebugOn != 0 )
		fprintf( stderr, "MouseMotion: %d, %d\n", x, y );


	int dx = x - Xmouse;		// change in mouse coords
	int dy = y - Ymouse;

	if( ( ActiveButton & LEFT ) != 0 )
	{
		Xrot += ( ANGFACT*dy );
		Yrot += ( ANGFACT*dx );
	}


	if( ( ActiveButton & MIDDLE ) != 0 )
	{
		Scale += SCLFACT * (float) ( dx - dy );

		// keep object from turning inside-out or disappearing:

		if( Scale < MINSCALE )
			Scale = MINSCALE;
	}

	Xmouse = x;			// new current position
	Ymouse = y;

	glutSetWindow( MainWindow );
	glutPostRedisplay( );
}


// reset the transformations and the colors:
// this only sets the global variables --
// the glut main loop is rotationponsible for redrawing the scene

void
Reset( )
{
	ActiveButton = 0;
	AxesOn = 1;
	DebugOn = 0;
	DepthCueOn = 0;
	Frozen = 0;
	Scale  = 1.0;
	WhichColor = WHITE;
	WhichProjection = PERSP;
	WhichView = 0;
	Xrot = Yrot = 0.;
}


// called when user rotationizes the window:

void
rotationize( int width, int height )
{
	if( DebugOn != 0 )
		fprintf( stderr, "rotationize: %d, %d\n", width, height );

	// don't really need to do anything since window size is
	// checked each time in Display( ):

	glutSetWindow( MainWindow );
	glutPostRedisplay( );
}


// handle a change to the window's visibility:

void
Visibility ( int state )
{
	if( DebugOn != 0 )
		fprintf( stderr, "Visibility: %d\n", state );

	if( state == GLUT_VISIBLE )
	{
		glutSetWindow( MainWindow );
		glutPostRedisplay( );
	}
	else
	{
		// could optimize by keeping track of the fact
		// that the window is not visible and avoid
		// animating or redrawing it ...
	}
}



///////////////////////////////////////   HANDY UTILITIES:  //////////////////////////


// the stroke characters 'X' 'Y' 'Z' :

static float xx[ ] = {
		0.f, 1.f, 0.f, 1.f
	      };

static float xy[ ] = {
		-.5f, .5f, .5f, -.5f
	      };

static int xorder[ ] = {
		1, 2, -3, 4
		};

static float yx[ ] = {
		0.f, 0.f, -.5f, .5f
	      };

static float yy[ ] = {
		0.f, .6f, 1.f, 1.f
	      };

static int yorder[ ] = {
		1, 2, 3, -2, 4
		};

static float zx[ ] = {
		1.f, 0.f, 1.f, 0.f, .25f, .75f
	      };

static float zy[ ] = {
		.5f, .5f, -.5f, -.5f, 0.f, 0.f
	      };

static int zorder[ ] = {
		1, 2, 3, 4, -5, 6
		};

// fraction of the length to use as height of the characters:
const float LENFRAC = 0.10f;

// fraction of length to use as start location of the characters:
const float BASEFRAC = 1.10f;

//	Draw a set of 3D axes:
//	(length is the axis length in world coordinates)

void
Axes( float length )
{
	glBegin( GL_LINE_STRIP );
		glVertex3f( length, 0., 0. );
		glVertex3f( 0., 0., 0. );
		glVertex3f( 0., length, 0. );
	glEnd( );
	glBegin( GL_LINE_STRIP );
		glVertex3f( 0., 0., 0. );
		glVertex3f( 0., 0., length );
	glEnd( );

	float fact = LENFRAC * length;
	float base = BASEFRAC * length;

	glBegin( GL_LINE_STRIP );
		for( int i = 0; i < 4; i++ )
		{
			int j = xorder[i];
			if( j < 0 )
			{
				
				glEnd( );
				glBegin( GL_LINE_STRIP );
				j = -j;
			}
			j--;
			glVertex3f( base + fact*xx[j], fact*xy[j], 0.0 );
		}
	glEnd( );

	glBegin( GL_LINE_STRIP );
		for( int i = 0; i < 5; i++ )
		{
			int j = yorder[i];
			if( j < 0 )
			{
				
				glEnd( );
				glBegin( GL_LINE_STRIP );
				j = -j;
			}
			j--;
			glVertex3f( fact*yx[j], base + fact*yy[j], 0.0 );
		}
	glEnd( );

	glBegin( GL_LINE_STRIP );
		for( int i = 0; i < 6; i++ )
		{
			int j = zorder[i];
			if( j < 0 )
			{
				
				glEnd( );
				glBegin( GL_LINE_STRIP );
				j = -j;
			}
			j--;
			glVertex3f( 0.0, fact*zy[j], base + fact*zx[j] );
		}
	glEnd( );

}


// function to convert HSV to RGB
// 0.  <=  s, v, r, g, b  <=  1.
// 0.  <= h  <=  360.
// when this returns, call:
//		glColor3fv( rgb );

void
HsvRgb( float hsv[3], float rgb[3] )
{
	// guarantee valid input:

	float h = hsv[0] / 60.f;
	while( h >= 6. )	h -= 6.;
	while( h <  0. ) 	h += 6.;

	float s = hsv[1];
	if( s < 0. )
		s = 0.;
	if( s > 1. )
		s = 1.;

	float v = hsv[2];
	if( v < 0. )
		v = 0.;
	if( v > 1. )
		v = 1.;

	// if sat==0, then is a gray:

	if( s == 0.0 )
	{
		rgb[0] = rgb[1] = rgb[2] = v;
		return;
	}

	// get an rgb from the hue itself:
	
	float i = floor( h );
	float f = h - i;
	float p = v * ( 1.f - s );
	float q = v * ( 1.f - s*f );
	float t = v * ( 1.f - ( s * (1.f-f) ) );

	float r, g, b;			// red, green, blue
	switch( (int) i )
	{
		case 0:
			r = v;	g = t;	b = p;
			break;
	
		case 1:
			r = q;	g = v;	b = p;
			break;
	
		case 2:
			r = p;	g = v;	b = t;
			break;
	
		case 3:
			r = p;	g = q;	b = v;
			break;
	
		case 4:
			r = t;	g = p;	b = v;
			break;
	
		case 5:
			r = v;	g = p;	b = q;
			break;
	}


	rgb[0] = r;
	rgb[1] = g;
	rgb[2] = b;
}

unsigned char *
BmpToTexture( char *filename, int *width, int *height )
{

	int s, t, e;		// counters
	int numextra;		// # extra bytes each line in the file is padded with
	FILE *fp;
	unsigned char *texture;
	int nums, numt;
	unsigned char *tp;


	fp = fopen( filename, "rb" );
	if( fp == NULL )
	{
		fprintf( stderr, "Cannot open Bmp file '%s'\n", filename );
		return NULL;
	}

	FileHeader.bfType = ReadShort( fp );


	// if bfType is not 0x4d42, the file is not a bmp:

	if( FileHeader.bfType != 0x4d42 )
	{
		fprintf( stderr, "Wrong type of file: 0x%0x\n", FileHeader.bfType );
		fclose( fp );
		return NULL;
	}


	FileHeader.bfSize = ReadInt( fp );
	FileHeader.bfrotationerved1 = ReadShort( fp );
	FileHeader.bfrotationerved2 = ReadShort( fp );
	FileHeader.bfOffBits = ReadInt( fp );


	InfoHeader.biSize = ReadInt( fp );
	InfoHeader.biWidth = ReadInt( fp );
	InfoHeader.biHeight = ReadInt( fp );

	nums = InfoHeader.biWidth;
	numt = InfoHeader.biHeight;

	InfoHeader.biPlanes = ReadShort( fp );
	InfoHeader.biBitCount = ReadShort( fp );
	InfoHeader.biComprotationsion = ReadInt( fp );
	InfoHeader.biSizeImage = ReadInt( fp );
	InfoHeader.biXPelsPerMeter = ReadInt( fp );
	InfoHeader.biYPelsPerMeter = ReadInt( fp );
	InfoHeader.biClrUsed = ReadInt( fp );
	InfoHeader.biClrImportant = ReadInt( fp );


	// fprintf( stderr, "Image size found: %d x %d\n", ImageWidth, ImageHeight );


	texture = new unsigned char[ 3 * nums * numt ];
	if( texture == NULL )
	{
		fprintf( stderr, "Cannot allocate the texture array!\b" );
		return NULL;
	}


	// extra padding bytes:

	numextra =  4*(( (3*InfoHeader.biWidth)+3)/4) - 3*InfoHeader.biWidth;


	// we do not support comprotationsion:

	if( InfoHeader.biComprotationsion != birgb )
	{
		fprintf( stderr, "Wrong type of image comprotationsion: %d\n", InfoHeader.biComprotationsion );
		fclose( fp );
		return NULL;
	}
	


	rewind( fp );
	fseek( fp, 14+40, SEEK_SET );

	if( InfoHeader.biBitCount == 24 )
	{
		for( t = 0, tp = texture; t < numt; t++ )
		{
			for( s = 0; s < nums; s++, tp += 3 )
			{
				*(tp+2) = fgetc( fp );		// b
				*(tp+1) = fgetc( fp );		// g
				*(tp+0) = fgetc( fp );		// r
			}

			for( e = 0; e < numextra; e++ )
			{
				fgetc( fp );
			}
		}
	}

	fclose( fp );

	*width = nums;
	*height = numt;
	return texture;
}



int
ReadInt( FILE *fp )
{
	unsigned char b3, b2, b1, b0;
	b0 = fgetc( fp );
	b1 = fgetc( fp );
	b2 = fgetc( fp );
	b3 = fgetc( fp );
	return ( b3 << 24 )  |  ( b2 << 16 )  |  ( b1 << 8 )  |  b0;
}


short
ReadShort( FILE *fp )
{
	unsigned char b1, b0;
	b0 = fgetc( fp );
	b1 = fgetc( fp );
	return ( b1 << 8 )  |  b0;
}