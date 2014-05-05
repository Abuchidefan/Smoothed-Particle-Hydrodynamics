#include "owWorldSimulation.h"
#include "owPhysicsFluidSimulator.h"
#include "VectorMath.h"
#include <stdio.h>
#include <iostream>
#include <fstream>

extern int numOfLiquidP;
extern int numOfElasticP;
extern int numOfBoundaryP;
extern int iterationCount;

int old_x=0, old_y=0;	// Used for mouse event
float camera_trans[] = {0, 0, -8.0};
float camera_rot[]   = {0, 0, 0};
float camera_trans_lag[] = {0, 0, -8.0};
float camera_rot_lag[] = {0, 0, 0};
const float inertia = 1.0f;
float modelView[16];
int buttonState = 0;
float sc = 0.025;		//0.0145;//0.045;//0.07

Vector3D ort1(1,0,0),ort2(0,1,0),ort3(0,0,1);
GLsizei viewHeight, viewWidth;
int winIdMain;
int winIdSub;
int PARTICLE_COUNT = 0;
int PARTICLE_COUNT_RoundedUp = 0;
int MUSCLE_COUNT = 100;//increase this value and modify corresponding code if you plan to add more than 10 muscles
double totalTime = 0;
int frames_counter = 0;
double calculationTime;
double renderTime;
double fps;
char device_full_name [1000];
double prevTime;
unsigned int * p_indexb;
float * d_b;
float * p_b;
float * e_c;
float * v_b;
void calculateFPS();
owPhysicsFluidSimulator * fluid_simulation;
owHelper * helper;
int local_NDRange_size = 256;//256;

float * muscle_activation_signal_cpp;

//==============TESTED INFO=================

extern const int tested_id;

//===============================
int steps;
void zero_vel_buff(){
	float * v_b = fluid_simulation->getVelocityBuffer();
	for(int i = 0; i < PARTICLE_COUNT; i++){
		if(int(v_b[4 * i + 3]) != BOUNDARY_PARTICLE){
			v_b[4 * i + 0] = 0.f;
			v_b[4 * i + 1] = 0.f;
			v_b[4 * i + 2] = 0.f;
			//v_b[4 * i + 3] = v_b[4 * i + 3];
		}
	}
	fluid_simulation->putVelocityBuffer(v_b);
	//delete v_b;
}
bool need = true;
int const num_of_slice = 11;
float result[] = {0.0f,0.0f,0.0f,0.0f,0.0f,0.0f,0.0f,0.0f,0.0f,0.0f,};
void display(void)
{
	helper->refreshTime();
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);	
	Vector3D vcenter(0,0,0);
	Vector3D vbox[8];

	//       [7]----[6]
	//      / |     /| 
	//    [3]----[2] | 
	//     | [4]--|-[5]   
	//     | /    | /
	//    [0]----[1]  

	vbox[0] = Vector3D(XMIN,YMIN,ZMIN);
	vbox[1] = Vector3D(XMAX,YMIN,ZMIN);
	vbox[2] = Vector3D(XMAX,YMAX,ZMIN);
	vbox[3] = Vector3D(XMIN,YMAX,ZMIN);
	vbox[4] = Vector3D(XMIN,YMIN,ZMAX);
	vbox[5] = Vector3D(XMAX,YMIN,ZMAX);
	vbox[6] = Vector3D(XMAX,YMAX,ZMAX);
	vbox[7] = Vector3D(XMIN,YMAX,ZMAX);
	glBegin(GL_LINES);
	sc *=10;
	glColor3ub(255, 0, 0);
	glVertex3d(vcenter.x,vcenter.y,vcenter.z);
	glVertex3d(vcenter.x+sc,vcenter.y,vcenter.z);
	glColor3ub(0,255, 0);
	glVertex3d(vcenter.x,vcenter.y,vcenter.z);
	glVertex3d(vcenter.x,vcenter.y+sc,vcenter.z);
	glColor3ub(0, 0, 255);
	glVertex3d(vcenter.x,vcenter.y,vcenter.z);
	glVertex3d(vcenter.x,vcenter.y,vcenter.z+sc);
	sc /=10;
	vcenter = Vector3D(-(XMIN+XMAX)/2,-(YMIN+YMAX)/2,-(ZMIN+ZMAX)/2);
	vcenter *= sc;
	Vector3D v1,v2,v3,v4,v5,v6,v7,v8;
	v1 = Vector3D( -XMAX/2, -YMAX/2, -ZMAX/2)*sc;
	v2 = Vector3D(  XMAX/2, -YMAX/2, -ZMAX/2)*sc;
	v3 = Vector3D(  XMAX/2,  YMAX/2, -ZMAX/2)*sc;
	v4 = Vector3D( -XMAX/2,  YMAX/2, -ZMAX/2)*sc;
	v5 = Vector3D( -XMAX/2, -YMAX/2,  ZMAX/2)*sc;
	v6 = Vector3D(  XMAX/2, -YMAX/2,  ZMAX/2)*sc;
	v7 = Vector3D(  XMAX/2,  YMAX/2,  ZMAX/2)*sc;
	v8 = Vector3D( -XMAX/2,  YMAX/2,  ZMAX/2)*sc;
	glColor3ub(255,255,255);//yellow
	glVertex3d(v1.x,v1.y,v1.z); 
	glVertex3d(v2.x,v2.y,v2.z);

	glVertex3d(v2.x,v2.y,v2.z);
	glVertex3d(v3.x,v3.y,v3.z);

	glVertex3d(v3.x,v3.y,v3.z);
	glVertex3d(v4.x,v4.y,v4.z);

	glVertex3d(v4.x,v4.y,v4.z); //glColor3ub(0,255,0);//green
	glVertex3d(v1.x,v1.y,v1.z);

	//glColor3ub(0,0,255);//blue
	glVertex3d(v1.x,v1.y,v1.z); //glColor3ub(255,255,0);//yellow
	glVertex3d(v5.x,v5.y,v5.z);

	glVertex3d(v2.x,v2.y,v2.z);
	glVertex3d(v6.x,v6.y,v6.z);

	glVertex3d(v3.x,v3.y,v3.z);
	glVertex3d(v7.x,v7.y,v7.z);

	glVertex3d(v4.x,v4.y,v4.z);
	glVertex3d(v8.x,v8.y,v8.z);

	glVertex3d(v5.x,v5.y,v5.z);
	glVertex3d(v6.x,v6.y,v6.z);

	glVertex3d(v6.x,v6.y,v6.z);
	glVertex3d(v7.x,v7.y,v7.z);

	glVertex3d(v7.x,v7.y,v7.z);
	glVertex3d(v8.x,v8.y,v8.z);

	glVertex3d(v8.x,v8.y,v8.z);
	glVertex3d(v5.x,v5.y,v5.z);
	
	glEnd();

	//glColor3ub(255,255,255);//yellow
	/*p_indexb = fluid_simulation->getParticleIndexBuffer();
	int pib;
	for(int i=0;i<PARTICLE_COUNT;i++)
	{
		pib = p_indexb[2*i + 1];
		p_indexb[2*pib + 0] = i;
	}*/
	glPointSize(3.f);
	//glBegin(GL_POINTS);
	p_b = fluid_simulation->getPositionBuffer();
	d_b = fluid_simulation->getDensityBuffer();
	v_b = fluid_simulation->getVelocityBuffer();
	float dc, rho;
	int id = 0;
//	if(need && iterationCount < 480){
//		float * zero_position = new float[4];
//		float * zero_velocity = new float[4];
//		
//		zero_position[0] = p_b[ id * 4 + 0 ];
//		zero_position[1] = p_b[ id * 4 + 1 ];
//		zero_position[2] = p_b[ id * 4 + 2 ];
//		zero_position[3] = p_b[ id * 4 + 3 ];
//		zero_velocity[0] = v_b[ id * 4 + 0 ];
//		zero_velocity[1] = v_b[ id * 4 + 1 ];
//		zero_velocity[2] = v_b[ id * 4 + 2 ];
//		zero_velocity[3] = v_b[ id * 4 + 3 ];
//		if(zero_velocity[1] > 0.f){
////			std::cout <<"velocity:" << zero_velocity[0] << "\t" << zero_velocity[1] << "\t" << zero_velocity[2] << "\t" << std::endl;
//			zero_vel_buff();
//			//need = false;
//		}
//	}
	
	if( iterationCount  > 500 && steps < 20000){
		std::vector<Vector3D> histogramm;
		//histogramm.resize(num_of_slice);
		for(int i = 0; i<PARTICLE_COUNT; i++)
		{
			if(int(p_b[i*4+3]) != BOUNDARY_PARTICLE){
				if(p_b[i*4 + 1] <= YMAX * 1/2 && p_b[i*4 + 1] >= YMAX * 1/2 - r0 / 5.0){
					Vector3D v(v_b[i*4+0],v_b[i*4+1],v_b[i*4+2]);
					v.count = i;
					histogramm.push_back(v);
				}
			}
			
		}
		if(histogramm.size()!=0){
			steps++;
			std::ofstream out_f ("v_log.txt",std::ios_base::app);
			std::cout<< "===================HISTOGRAM====================" <<std::endl;
			/*for(int i=0;i<num_of_slice;i++){
				out_f << i << "\t" << result[i] << "\n";
			}*/
			for(int i=0;i<histogramm.size();i++){
				double l = histogramm[i].length();
				out_f << iterationCount << "\t" << i << "\t" << histogramm[i].count << "\t" << p_b[histogramm[i].count*4 + 1]-YMAX/2 << "\t"<< p_b[histogramm[i].count*4]-XMAX/2 << "\t" << p_b[histogramm[i].count*4 + 2]-ZMAX/2 << "\t"  << l << "\n";
			}
			out_f <<"=======================\n";
			std::cout<< "======================END=======================" <<std::endl;
			out_f.close();
		}
		//exit(1);
	}
	if(steps >= 20000){
		//owHelper::loadConfigToFiles(p_b,v_b);
		exit(0);
	}
	for(int i = 0; i<PARTICLE_COUNT; i++)
	{
		//rho = d_b[ p_indexb[ i * 2 + 0 ] ];
		//if( rho < 0 ) rho = 0;
		//if( rho > 2 * rho0) rho = 2 * rho0;
		//dc = 100.0 * ( rho - rho0 ) / rho0 ;
		//if(dc>1.f) dc = 1.f;
		////  R   G   B
		//glColor4f(  0,  0,  1, 1.0f);//blue
		//if( (dc=100*(rho-rho0*1.00f)/rho0) >0 )	glColor4f(   0,  dc,   1,1.0f);//cyan
		//if( (dc=100*(rho-rho0*1.01f)/rho0) >0 )	glColor4f(   0,   1,1-dc,1.0f);//green
		//if( (dc=100*(rho-rho0*1.02f)/rho0) >0 )	glColor4f(  dc,   1,   0,1.0f);//yellow
		//if( (dc=100*(rho-rho0*1.03f)/rho0) >0 )	glColor4f(   1,1-dc,   0,1.0f);//red
		//if( (dc=100*(rho-rho0*1.04f)/rho0) >0 )	glColor4f(   1,   0,   0,1.0f);
		if((int)p_b[i*4 + 3] != BOUNDARY_PARTICLE /*&& (int)p_b[i*4 + 3] != ELASTIC_PARTICLE*/){
			glBegin(GL_POINTS);
			glColor4f(   0,0,   1,1.0f);
			if((int)p_b[i*4+3]==2) glColor4f(   1,   1,   0,  1.0f);
			/*if( i == id){
				glColor4f(   1,   0,   0,  1.0f);*/
				glVertex3f( (p_b[i*4]-XMAX/2)*sc , (p_b[i*4+1]-YMAX/2)*sc, (p_b[i*4+2]-ZMAX/2)*sc );
			//}
			//glVertex3f( (p_b[i*4])*sc , (p_b[i*4+1])*sc, (p_b[i*4+2])*sc );
			glEnd();
			/**/if( 1){
				/*glBegin(GL_LINES);
					glVertex3f( (p_b[i*4]-XMAX/2)*sc , (p_b[i*4+1]-YMAX/2)*sc, (p_b[i*4+2]-ZMAX/2)*sc );
					float c1 = 1.0f;
					glVertex3f( ((p_b[i*4] + c1 * v_b[i*4]-XMAX/2)*sc) , (c1*(p_b[i*4 + 1] + c1 * v_b[i*4 + 1]-YMAX/2)*sc), ((p_b[i*4 + 2] + c1 * v_b[i*4 + 2]-ZMAX/2)*sc) );
				glEnd();*/
			}/**/
		}
		else{
			glBegin(GL_LINES);
			//glVertex3f( (p_b[i*4])*sc , (p_b[i*4+1])*sc, (p_b[i*4+2])*sc );
			//glVertex3f( (p_b[i*4] + v_b[i*4])*sc , (p_b[i*4 + 1] + v_b[i*4 + 1])*sc, (p_b[i*4 + 2] + v_b[i*4 + 2])*sc );
			glVertex3f( (p_b[i*4]-XMAX/2)*sc , (p_b[i*4+1]-YMAX/2)*sc, (p_b[i*4+2]-ZMAX/2)*sc );
			glVertex3f( (p_b[i*4] + v_b[i*4] * 1-XMAX/2)*sc , (p_b[i*4 + 1] + v_b[i*4 + 1] * 1-YMAX/2)*sc, (p_b[i*4 + 2] + v_b[i*4 + 2] * 1-ZMAX/2)*sc );
			glEnd();
		}
	}
	e_c = fluid_simulation->getElasticConnections();
	
	//if(generateInitialConfiguration)
	/*for(int i, i_ec=0; i_ec < numOfElasticP * NEIGHBOR_COUNT; i_ec++)
	{
		//offset = 0
		if((j=e_c[ 4 * i_ec + 0 ])>=0)
		{
			i = (i_ec / NEIGHBOR_COUNT); //+ (generateInitialConfiguration!=1)*numOfBoundaryP;

			glColor4b(255/2, 125/2, 0, 100/2);
			if(e_c[ 4 * i_ec + 2 ]>1.f) glColor4b(255/2, 0, 0, 255/2);
			
			glBegin(GL_LINES);
			glVertex3f( (p_b[i*4]-XMAX/2)*sc , (p_b[i*4+1]-YMAX/2)*sc, (p_b[i*4+2]-ZMAX/2)*sc );
			glVertex3f( (p_b[j*4]-XMAX/2)*sc , (p_b[j*4+1]-YMAX/2)*sc, (p_b[j*4+2]-ZMAX/2)*sc );
			glEnd();
		}
	}*/
	//glEnd();
	glutSwapBuffers();
	helper->watch_report("graphics: \t\t%9.3f ms\n====================================\n");
	renderTime = helper->get_elapsedTime();
	totalTime += calculationTime + renderTime;
	calculateFPS();
}
void calculateFPS()
{
    //  Increase frame count
	frames_counter++;
    int timeInterval = totalTime - prevTime;
    if(timeInterval >= 1000)
    {
		fps = frames_counter / (timeInterval / 1000.0f);
        prevTime = totalTime;
        frames_counter = 0;
		printf("FPS: \t\t%9.3f fps\n====================================\n",	fps );
    }
}/**/
void respond_mouse(int button, int state, int x, int y)
{
	if (button == GLUT_LEFT_BUTTON)
		buttonState = 1;
	if (button == GLUT_RIGHT_BUTTON)
		buttonState = 3;
	int mods;
	mods = glutGetModifiers();
    if (mods & GLUT_ACTIVE_CTRL) 
    {
        buttonState = 2;
    } 
	if(state == GLUT_UP)
		buttonState = 0;
	old_x=x;
	old_y=y;
	if (button == 3)// mouse wheel up
    {
        sc *= 1.1f;// Zoom in
    }
    else
	if (button == 4)// mouse wheel down
    {
        sc /= 1.1f;// Zoom out
    }
}

// GLUT callback
// called on mouse movement

void mouse_motion (int x, int y) 
{
	float dx,dy;
	dy = (float)(y - old_y);	
	dx = (float)(x - old_x);
	
	if(buttonState == 1)
	{
		camera_rot[0] += dy / 5.0f;
		camera_rot[1] += dx / 5.0f;
	}
	if(buttonState == 3){
		// middle = translate
		camera_trans[0] += dx / 100.0f;
		camera_trans[1] -= dy / 100.0f;
		//camera_trans[2] += (dy / 100.0f) * 0.5f * fabs(camera_trans[2]);
	}
	if(buttonState == 2){
		// middle = translate
		//camera_trans[0] += dx / 100.0f;
		//camera_trans[1] -= dy / 100.0f;
		camera_trans[2] += (dy / 100.0f) * 0.5f * fabs(camera_trans[2]);
	}
	old_x=x;
	old_y=y;
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	for (int c = 0; c < 3; ++c)
	{
	    camera_trans_lag[c] += (camera_trans[c] - camera_trans_lag[c]) * inertia;
		camera_rot_lag[c] += (camera_rot[c] - camera_rot_lag[c]) * inertia;
	}
    glTranslatef(camera_trans_lag[0], camera_trans_lag[1], camera_trans_lag[2]);
	glRotatef(camera_rot_lag[0], 1.0, 0.0, 0.0);
	glRotatef(camera_rot_lag[1], 0.0, 1.0, 0.0);
	glGetFloatv(GL_MODELVIEW_MATRIX, modelView);
}

void respond_key_pressed(unsigned char key, int x, int y)
{
	/*if(key=='a')
	{
		muscle_activation_signal += 0.1f;
		if(muscle_activation_signal>1.f) muscle_activation_signal = 1.f;
	}*/

	return;
}

//Auxiliary function
/* There can be only one idle() callback function. In an 
   animation, this idle() function must update not only the 
   main window but also all derived subwindows */ 
void idle (void) 
{ 
  glutSetWindow (winIdMain); 
  glutPostRedisplay (); 
  glutSetWindow (winIdSub); 
  glutPostRedisplay (); 
} 
void drawString (char *s) 
{ 
  unsigned int i; 
  for (i = 0; i < strlen (s); i++) 
    glutBitmapCharacter (GLUT_BITMAP_HELVETICA_10, s[i]); 
} 
void drawStringBig (char *s) 
{ 
  unsigned int i; 
  for (i = 0; i < strlen (s); i++) 
	  glutBitmapCharacter (GLUT_BITMAP_HELVETICA_18, s[i]); 
}
static char label[1000];                            /* Storage for current string   */

void subMenuDisplay() 
{ 
	/* Clear subwindow */ 
	glutSetWindow (winIdSub); 
	glClearColor(0.7f, 0.7f, 0.7f, 1.0f);
	glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); 
	/* Write State Variables */ 
	glColor3f (1.0F, 1.0F, 1.0F);
	sprintf(label,"Liquid particles: %d, elastic matter particles: %d, boundary particles: %d; total count: %d", numOfLiquidP,
																												 numOfElasticP,
																												 numOfBoundaryP,PARTICLE_COUNT); 
	glRasterPos2f (0.01F, 0.75F); 
	drawStringBig (label); 
	glColor3f (1.0F, 1.0F, 1.0F); 
	sprintf(label,"Selected device: %s     FPS = %.2f, time left: %d", device_full_name, fps, iterationCount * timeStep); 
	glRasterPos2f (0.01F, 0.40F); 
	drawStringBig (label); 

	//sprintf(label,"Muscle activation signal: %.3f (press 'a' to activate)", muscle_activation_signal);
	//glRasterPos2f (0.01F, 0.05F);
	//drawStringBig (label);

	glutSwapBuffers (); 
} 
void subMenuReshape (int w, int h) 
{ 
  glViewport (0, 0, w, h); 
  glMatrixMode (GL_PROJECTION); 
  glLoadIdentity (); 
  gluOrtho2D (0.0F, 1.0F, 0.0F, 1.0F); 
}
void Timer(int value)
{
	calculationTime = fluid_simulation->simulationStep();
	// Re-register for next callback
    glutTimerFunc(TIMER_INTERVAL*0, Timer, 0);
	glutPostRedisplay();
}
void SetProjectionMatrix(void){
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();									// Current projection matrix is dropped to identity matrix 
	glFrustum(-1, 1, -1, 1, 3, 15*3);						// Set up perspective projection
}
void SetModelviewMatrix(void){
     glMatrixMode(GL_MODELVIEW);                                   
     glLoadIdentity();                                             
     glTranslatef(0.0, 0.0, -8.0);                              
     glRotatef(0*10.0, 1.0, 0.0, 0.0);
     glRotatef(0.0, 0.0, 1.0, 0.0);                              
}
GLvoid resize(GLsizei width, GLsizei height){
	if(height == 0)
	{
		height = 1;										
	}

	glViewport(0, 0, width, height);					// Sev view area

	viewHeight = height;
	viewWidth = width;

	SetProjectionMatrix();
	SetModelviewMatrix();

//================= fixes a small bug -- scene's point of view is set to initial after window rezise ('forgotten' to update)
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	for (int c = 0; c < 3; ++c)
	{
	    camera_trans_lag[c] += (camera_trans[c] - camera_trans_lag[c]) * inertia;
		camera_rot_lag[c] += (camera_rot[c] - camera_rot_lag[c]) * inertia;
	}
    glTranslatef(camera_trans_lag[0], camera_trans_lag[1], camera_trans_lag[2]);
	glRotatef(camera_rot_lag[0], 1.0, 0.0, 0.0);
	glRotatef(camera_rot_lag[1], 0.0, 1.0, 0.0);
	glGetFloatv(GL_MODELVIEW_MATRIX, modelView);
//==================
}
void init(void){
	glClearColor(0.7f, 0.7f, 0.7f, 1.0f);
	glClearDepth(1.0f);

	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);
	glShadeModel(GL_SMOOTH);
	glEnable(GL_LINE_SMOOTH);
	glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

}
void draw(void)
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glPushMatrix();
	glColor3f(1.0f, 1.0f, 1.0f);
	glPopMatrix();
}

void run(int argc, char** argv, const bool with_graphics)
{
	helper = new owHelper();
	fluid_simulation = new owPhysicsFluidSimulator(helper);
	if(with_graphics){
		glutInit(&argc, argv);
		glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);
		glutInitWindowSize(800, 600);
		glutInitWindowPosition(100, 100);
		winIdMain = glutCreateWindow("Palyanov Andrey for OpenWorm: OpenCL PCISPH fluid + elastic matter demo [2013]");
		glutIdleFunc (idle); 
		glEnable(GL_LIGHTING);
		glEnable(GL_LIGHT0);
		glEnable(GL_COLOR_MATERIAL);
		glEnable(GL_NORMALIZE);
		glEnable(GL_AUTO_NORMAL);
		float ambient[4] = {1.0, 1.0, 1.0, 1};
		glLightModelfv(GL_LIGHT_MODEL_AMBIENT, ambient);
		//Init physic Simulation
		init();
		glutDisplayFunc(display);
		glutReshapeFunc(resize);
		glutMouseFunc(respond_mouse);
		glutMotionFunc(mouse_motion);	// The former handles movement while the mouse is clicked, 
		glutKeyboardFunc(respond_key_pressed);
		//Create sub window which contains information about simulation: FPS, and particles count
		winIdSub = glutCreateSubWindow (winIdMain, 5, 5, 1000 - 10, 600 / 10); 
		glutDisplayFunc (subMenuDisplay); 
		glutReshapeFunc (subMenuReshape); 
		glutTimerFunc(TIMER_INTERVAL * 0, Timer, 0);
		glutMainLoop();
		fluid_simulation->~owPhysicsFluidSimulator();
	}else{
		while(1){
			fluid_simulation->simulationStep();
			helper->refreshTime();
		}
	}
/*	{
		double step_time = 0, total_work_time = 0;
		int steps_cnt = 0;
		while(steps_cnt<100){
			step_time = fluid_simulation->simulationStep();
			total_work_time += step_time;
			helper->refreshTime();
			steps_cnt++;
		}

		printf("\ntotal calculation time (1000 steps) = %f ms\n",total_work_time);
	}*/	
	
}
