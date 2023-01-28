#include<GL/glew.h>
#include<GL/gl.h>
#include<AL/al.h>
#include<AL/alut.h>
#include<X11/keysym.h>
#include<SOIL/SOIL.h>
#include"include/render.h"
#include"include/fpslock.h"
#include"include/shaders.h"
#include"include/vmathmod.h"
#include"include/sphere.h"
#include"include/texload.h"
#include"include/helperfuncs.h"
#include"include/arcballcamera.h"
#include"include/fontrender.h"
#include"include/noise.h"
#include"include/planetinfo.h"
#include"include/triangulationTable.h"

#define NUM_STARS 300 * 6			//Keep Multiple of 6 to keep equal stars per cube map face
#define INFO_RESOLUTION 512
#define TITLE_RESOLUTION 2048
#define STARFIELD_RESOLUTION 1024
#define SOLARSYSTEM_RESOLUTION 2048
#define ZOOMEDPLANET_RESOLUTION 1024

#define BUFFER_OFFSET(x) (void*)(x)

enum {
	vPosition = 0,
	vColor,
	vNormal,
	vTexCoord
};

enum {
	starVao = 0,
	sphereVao,
	orbitVao,
	blankVao,
	sideMenuVao,
	saturnRingVao,
	zoomedPlanetVao,
	lenVao
};

enum {
	starVbo = 0,
	sphereVbo,
	sphereEabo,
	orbitVbo,
	sideMenuVbo,
	saturnRingVbo,
	zoomedPlanetVbo,
	sampleSsbo,
	celltypeSsbo,
	tritableSsbo,
	lenBo
};

enum {
	sunTex = 0,
	mercuryTex,
	venusTex,
	earthTex,
	marsTex,
	jupiterTex,
	saturnTex,
	uranusTex,
	neptuneTex,
	lenPlanetTex
};

enum {
	starTex = 0,
	starfieldColorTex,
	solarsystemColorTex,
	solarsystemPickTex,
	saturnRingTex,
	sataliteTex,
	titleTex,
	filterTex,
	filterSizeRegulationTex,
	satDisplayTex,
	zoomedPlanetTex,
	borderColorTex,
	noiseTex,
	lenTex
};

enum {
	solarsystemDepthRbo = 0,
	zoomedPlanetRbo,
	lenRbo
};

enum {
	starfieldFbo = 0,
	solarsystemFbo,
	filterFbo,
	satDisplayFbo,
	zoomedPlanetFbo,
	lenFbo
};

GLuint vao[lenVao];
GLuint bo[lenBo];
GLuint tex[lenTex];
GLuint texInfo[lenPlanetTex];
GLuint rbo[lenRbo];
GLuint fbo[lenFbo];

struct {
	GLuint skybox;
	GLuint starfield;
	GLuint planet;
	GLuint orbit;
	GLuint solarsystem;
	GLuint blur;
	GLuint sidemenu;
	GLuint ring;
	GLuint fade;
	GLuint doffilter;
	GLuint satdisplay;
	GLuint noise[9];
	GLuint zoomedsatalite;
	GLuint triangulation;
	GLuint marchingcube;
	GLuint scalarfield;
} program;

struct {
	GLint pMat = 0;
	GLint vMat = 1;
	GLint mMat = 2;
	struct {
		GLint lightPos = 3;
		GLint zoomedLayer = 4;
	} zoomplanet;
	struct {
		GLint lightZ = 11;
	} planet;
	struct {
		GLint focalDist = 0;
		GLint focalDepth = 1;
	} dof;
	struct {
		GLint fadeAlpha = 0;
	} fade;
	struct {
		GLint isBorder = 0;
	} sidemenu;
	struct {
		GLint cellsPerAxis = 3;
		GLint numVertices = 4;
		GLint isolevel = 5;
		GLint cellType = 0;
		GLint scalarField = 1;
		GLint triTable = 2;
	} tri;
	struct {
		GLint cellsPerAxis = 0;
		GLint isolevel = 1;
		GLint scalarField = 0;
	} mc;
	struct {
		GLint samplesPerAxis = 0;
		GLint radius = 1;
	} sf;
} uniforms;

ArcballCamera *camera;
sphere *sp;
float currentTimeInHours;
FontRender* titleFont;
FontRender* contentFont;
ALuint source;
const int samplesPerAxis = 300;
const int samplesTotal = samplesPerAxis * samplesPerAxis * samplesPerAxis;
const int cellsPerAxis = samplesPerAxis - 1;
const int cellsTotal = cellsPerAxis * cellsPerAxis * cellsPerAxis;
const float isolevel = 1.0f;
const int verticesPerCell = 5 * 3;
const vmath::ivec2 HorizontalFilter = vmath::ivec2(1, 0);
const vmath::ivec2 VerticalFilter = vmath::ivec2(0, 1);
const vmath::vec3 FinalZoomedPlanetCoords = vmath::vec3(-1.5f, 0.0f, -5.0f);
struct {
	GLfloat sigma = 0.0f;
	GLfloat sideMenuX = 0.6f;
	GLfloat cameraRot = 0.0f;
	GLfloat fade = 1.0f;
	// GLfloat fade = 0.0f;
	struct {
		vmath::vec3 position = vmath::vec3(0.0f, 0.0f, 0.0f);
		float scale = 0.0f;
		float initialTransScaleMercury = 0.0f;
		float initialTransScaleVenus = 0.0f;
		float initialTransScaleEarth = 0.0f;
		float initialTransScaleMars = 0.0f;
		float initialTransScaleJupiter = 0.0f;
		float initialTransScaleSaturn = 0.0f;
		float initialTransScaleUranus = 0.0f;
		float initialTransScaleNeptune = 0.0f;
		// float initialTransScaleMercury = 1.0f;
		// float initialTransScaleVenus = 1.0f;
		// float initialTransScaleEarth = 1.0f;
		// float initialTransScaleMars = 1.0f;
		// float initialTransScaleJupiter = 1.0f;
		// float initialTransScaleSaturn = 1.0f;
		// float initialTransScaleUranus = 1.0f;
		// float initialTransScaleNeptune = 1.0f;
	} planet;
} updatevals;
enum Animation {
	Nothing = -1,
	ZoomIn = 0,
	ZoomOut,
	Title,
	Initial,
	NothingFadeIn
};
Animation currentAnimating = Title;
// Animation currentAnimating = Nothing;
int currentZoomedPlanet = -1;
bool isDOFEnable = false;
GLfloat distToCamera = 15.0f;

void GLAPIENTRY MessageCallback(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* message, const void* userParam) {
  fprintf(stderr, "GL CALLBACK: %s type = 0x%x, severity = 0x%x, message = %s\n", ( type == GL_DEBUG_TYPE_ERROR ? "** GL ERROR **" : "" ), type, severity, message);
}

void loadNoiseShaders() {
	ShaderInfo vertexShader = { GL_VERTEX_SHADER, "shaders/zoomedplanetnoise.vert" };
	ShaderInfo sun[] = {
		vertexShader,
		{ GL_FRAGMENT_SHADER, "shaders/noise/sun.frag" }
	};
	ShaderInfo mercury[] = {
		vertexShader,
		{ GL_FRAGMENT_SHADER, "shaders/noise/mercury.frag" }
	};
	ShaderInfo venus[] = {
		vertexShader,
		{ GL_FRAGMENT_SHADER, "shaders/noise/venus.frag" }
	};
	ShaderInfo earth[] = {
		vertexShader,
		{GL_FRAGMENT_SHADER, "shaders/noise/earth.frag"}
	};
	ShaderInfo mars[] = {
		vertexShader,
		{ GL_FRAGMENT_SHADER, "shaders/noise/mars.frag" }
	};
	ShaderInfo jupiter[] = {
		vertexShader,
		{ GL_FRAGMENT_SHADER, "shaders/noise/jupiter.frag" }
	};
	ShaderInfo saturn[] = {
		vertexShader,
		{ GL_FRAGMENT_SHADER, "shaders/noise/saturn.frag" }
	};
	ShaderInfo uranus[] = {
		vertexShader,
		{ GL_FRAGMENT_SHADER, "shaders/noise/uranus.frag" }
	};
	ShaderInfo neptune[] = {
		vertexShader,
		{ GL_FRAGMENT_SHADER, "shaders/noise/neptune.frag" }
	};

	if((program.noise[0] = gldlCreateShaderProgram(sun, len(sun), "SunNoise")) == -1) {
		gldlPrintErrorMessage();
	}
	if((program.noise[1] = gldlCreateShaderProgram(mercury, len(mercury), "MercuryNoise")) == -1) {
		gldlPrintErrorMessage();
	}
	if((program.noise[2] = gldlCreateShaderProgram(venus, len(venus), "VenusNoise")) == -1) {
		gldlPrintErrorMessage();
	}
	if((program.noise[3] = gldlCreateShaderProgram(earth, len(earth), "EarthNoise")) == -1) {
		gldlPrintErrorMessage();
	}
	if((program.noise[4] = gldlCreateShaderProgram(mars, len(mars), "MarsNoise")) == -1) {
		gldlPrintErrorMessage();
	}
	if((program.noise[5] = gldlCreateShaderProgram(jupiter, len(jupiter), "JupiterNoise")) == -1) {
		gldlPrintErrorMessage();
	}
	if((program.noise[6] = gldlCreateShaderProgram(saturn, len(saturn), "SaturnNoise")) == -1) {
		gldlPrintErrorMessage();
	}
	if((program.noise[7] = gldlCreateShaderProgram(uranus, len(uranus), "UranusNoise")) == -1) {
		gldlPrintErrorMessage();
	}
	if((program.noise[8] = gldlCreateShaderProgram(neptune, len(neptune), "NeptuneNoise")) == -1) {
		gldlPrintErrorMessage();
	}
}

void loadShaders(void) {
	gldlError err;
	
	ShaderInfo skybox[] = {
		{GL_VERTEX_SHADER, "shaders/skybox.vert"},
		{GL_FRAGMENT_SHADER, "shaders/skybox.frag"}
	};

	ShaderInfo starfield[] = {
		{GL_VERTEX_SHADER, "shaders/starfield.vert"},
		{GL_FRAGMENT_SHADER, "shaders/starfield.frag"}
	};

	ShaderInfo planet[] = {
		{GL_VERTEX_SHADER, "shaders/planet.vert"},
		{GL_FRAGMENT_SHADER, "shaders/planet.frag"}
	};

	ShaderInfo orbit[] = {
		{GL_VERTEX_SHADER, "shaders/orbit.vert"},
		{GL_FRAGMENT_SHADER, "shaders/orbit.frag"}
	};
	
	ShaderInfo solarsystem[] = {
		{GL_VERTEX_SHADER, "shaders/solarsystem.vert"},
		{GL_FRAGMENT_SHADER, "shaders/solarsystem.frag"}
	};

	ShaderInfo blur[] = {
		{GL_VERTEX_SHADER, "shaders/blur.vert"},
		{GL_FRAGMENT_SHADER, "shaders/blur.frag"}
	};

	ShaderInfo sidemenu[] = {
		{GL_VERTEX_SHADER, "shaders/sidemenu.vert"},
		{GL_FRAGMENT_SHADER, "shaders/sidemenu.frag"}
	};

	ShaderInfo ring[] = {
		{GL_VERTEX_SHADER, "shaders/ring.vert"},
		{GL_FRAGMENT_SHADER, "shaders/ring.frag"}
	};

	ShaderInfo fade[] = {
		{GL_VERTEX_SHADER, "shaders/fade.vert"},
		{GL_FRAGMENT_SHADER, "shaders/fade.frag"}
	};

	ShaderInfo doffilter[] = {
		{GL_VERTEX_SHADER, "shaders/doffilterpro.vert"},
		{GL_FRAGMENT_SHADER, "shaders/doffilterpro.frag"}
	};

	ShaderInfo satdisplay[] = {
		{GL_VERTEX_SHADER, "shaders/satdisplay.vert"},
		{GL_FRAGMENT_SHADER, "shaders/satdisplay.frag"}
	};

	ShaderInfo zoomedsatalite[] = {
		{GL_VERTEX_SHADER, "shaders/zoomedsatalite.vert"},
		{GL_FRAGMENT_SHADER, "shaders/zoomedsatalite.frag"}
	};

	ShaderInfo triangulation[] = {
		{ GL_VERTEX_SHADER, "shaders/tri.vert" },
		{ GL_FRAGMENT_SHADER, "shaders/tri.frag" }
	};

	ShaderInfo marchingcube[] = {
		{ GL_VERTEX_SHADER, "shaders/marchingcube.vert" }
	};

	ShaderInfo scalarfield[] = {
		{ GL_VERTEX_SHADER, "shaders/scalarfield.vert" }
	};

	if((program.skybox = gldlCreateShaderProgram(skybox, len(skybox), "Skybox")) == -1) {
		gldlPrintErrorMessage();
	}
	if((program.starfield = gldlCreateShaderProgram(starfield, len(starfield), "Starfield")) == -1) {
		gldlPrintErrorMessage();
	}
	if((program.planet = gldlCreateShaderProgram(planet, len(planet), "Planet")) == -1) {
		gldlPrintErrorMessage();
	}
	if((program.orbit = gldlCreateShaderProgram(orbit, len(orbit), "Orbit")) == -1) {
		gldlPrintErrorMessage();
	}
	if((program.solarsystem = gldlCreateShaderProgram(solarsystem, len(solarsystem), "SolarSystem")) == -1) {
		gldlPrintErrorMessage();
	}
	if((program.blur = gldlCreateShaderProgram(blur, len(blur), "Blur")) == -1) {
		gldlPrintErrorMessage();
	}
	if((program.sidemenu = gldlCreateShaderProgram(sidemenu, len(sidemenu), "Sidemenu")) == -1) {
		gldlPrintErrorMessage();
	}
	if((program.ring = gldlCreateShaderProgram(ring, len(ring), "Ring")) == -1) {
		gldlPrintErrorMessage();
	}
	if((program.fade = gldlCreateShaderProgram(fade, len(fade), "Fade")) == -1) {
		gldlPrintErrorMessage();
	}
	if((program.doffilter = gldlCreateShaderProgram(doffilter, len(doffilter), "DofFilter")) == -1) {
		gldlPrintErrorMessage();
	}
	if((program.satdisplay = gldlCreateShaderProgram(satdisplay, len(satdisplay), "SATDisplay")) == -1) {
		gldlPrintErrorMessage();
	}
	if((program.zoomedsatalite = gldlCreateShaderProgram(zoomedsatalite, len(zoomedsatalite), "ZoomedSatalite")) == -1) {
		gldlPrintErrorMessage();
	}
	if((program.triangulation = gldlCreateShaderProgram(triangulation, len(triangulation), "Triangulation")) == -1) {
		gldlPrintErrorMessage();
	}
	if((program.marchingcube = gldlCreateShaderProgram(marchingcube, len(marchingcube), "MarchingCube")) == -1) {
		gldlPrintErrorMessage();
	}
	const GLchar* celltypetf = "celltype";
	glTransformFeedbackVaryings(program.marchingcube, 1, &celltypetf, GL_SEPARATE_ATTRIBS);
	glLinkProgram(program.marchingcube);
	if((program.scalarfield = gldlCreateShaderProgram(scalarfield, len(scalarfield), "ScalarField")) == -1) {
		gldlPrintErrorMessage();
	}
	const GLchar* scalarfieldtf = "scalarfield";
	glTransformFeedbackVaryings(program.scalarfield, 1, &scalarfieldtf, GL_SEPARATE_ATTRIBS);
	glLinkProgram(program.scalarfield);

	loadNoiseShaders();
}

void initGL(void) {
	glEnable(GL_DEBUG_OUTPUT);
	glDebugMessageCallback(MessageCallback, 0);

	loadShaders();

	camera = new ArcballCamera();
	camera->rotateOnAxis(vmath::radians(45.0f), vmath::vec3(1.0f, 0.0f, 0.0f));
	camera->saveToHomeMatrix();

	if((titleFont = FontRender::createFontRenderer("font/Cybersky.otf", " ACEHIJLMNOPRSTUVY", 256)) == NULL) {
		printf("Cybersky.otf load failed\n");
	}
	if((contentFont = FontRender::createFontRenderer("font/FreeMono.otf", " :1234567890.,ADEGIMNORSTV-/^abcdeghiklmnoprstuvxy", 128)) == NULL) {
		printf("FreeMono.otf load failed\n");
	}

	ALuint buffer;
	ALuint source;
	ALenum error;

	ALsizei size, freq;
	ALenum format;
	ALvoid *data;
	ALboolean loop = AL_FALSE;

	if(!alutInit(NULL, NULL)) {
		ALenum error = alutGetError();
		printf("%s\n", alutGetErrorString(error));
		exit(1);
	}

	buffer = alutCreateBufferFromFile("music/main.wav");
	if(buffer == AL_NONE) {
		error = alutGetError ();
		printf("Error loading file: %x '%s'\n", error, alutGetErrorString (error));
		alutExit();
		exit(EXIT_FAILURE);
	}

	alGenSources(1, &source);
	if((error = alGetError()) != AL_NO_ERROR) {
		printf("alGenSources: %d", error);
		alDeleteBuffers(1, &buffer);
		exit(1);
	}

	alSourcei(source, AL_BUFFER, buffer);
	if((error = alGetError()) != AL_NO_ERROR) {
		printf("alSource: %d", error);
		alDeleteBuffers(1, &buffer);
		alDeleteSources(1, &source);
		exit(1);
	}
	alSourcePlay(source);

	glGenVertexArrays(lenVao, vao);
	glGenBuffers(lenBo, bo);
	glGenFramebuffers(lenFbo, fbo);
	glGenRenderbuffers(lenRbo, rbo);

//*****************************************************************************************************************
//******************************************************Title******************************************************
//*****************************************************************************************************************
{
	GLuint fbo;
	glGenTextures(1, &tex[titleTex]);
	glGenFramebuffers(1, &fbo);
	glBindFramebuffer(GL_FRAMEBUFFER, fbo);
	GLenum DrawBuffers[] = { GL_COLOR_ATTACHMENT0 };
	glBindTexture(GL_TEXTURE_2D, tex[titleTex]);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGBA8, TITLE_RESOLUTION, TITLE_RESOLUTION);
	glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, tex[titleTex], 0);
	glDrawBuffers(len(DrawBuffers), DrawBuffers);
	if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
		printf("Incomplete Text");
	}
	glClearBufferfv(GL_COLOR, 0, vmath::vec4(0.0f, 0.0f, 0.0f, 1.0f));
	glViewport(0, 0, TITLE_RESOLUTION, TITLE_RESOLUTION);
	titleFont->renderFont("SOLAR", vmath::vec2(100.0f, 275.0f), vmath::vec3(1.0f, 1.0f, 1.0f), 0.25f);
	titleFont->renderFont("SYSTEM", vmath::vec2(200.0f, 225.0f), vmath::vec3(1.0f, 1.0f, 1.0f), 0.25f);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

//*****************************************************************************************************************
//****************************************************Starfield****************************************************
//*****************************************************************************************************************
{
	glBindVertexArray(vao[starVao]);
	glBindBuffer(GL_ARRAY_BUFFER, bo[starVbo]);
	glBufferStorage(GL_ARRAY_BUFFER, sizeof(float) * 6 * NUM_STARS, NULL, GL_MAP_WRITE_BIT);
	float* star = (float*)glMapBufferRange(GL_ARRAY_BUFFER, 0, sizeof(float) * 6 * NUM_STARS, GL_MAP_WRITE_BIT);
	for(int i = 0; i < NUM_STARS; i++) {
		star[i * 6 + 0] = random_float() * 2.0f - 1.0f;
		star[i * 6 + 1] = random_float() * 2.0f - 1.0f;
		star[i * 6 + 2] = random_float() * 4.0f;
		star[i * 6 + 3] = 0.9f + random_float() * 0.1f;
		star[i * 6 + 4] = 0.7f + random_float() * 0.3f;
		star[i * 6 + 5] = 0.7f +random_float() * 0.3f;
	}
	glUnmapBuffer(GL_ARRAY_BUFFER);
	glVertexAttribPointer(vPosition, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 6, BUFFER_OFFSET(0));
	glVertexAttribPointer(vColor, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 6, BUFFER_OFFSET(sizeof(float) * 3));
	glEnableVertexAttribArray(vPosition);
	glEnableVertexAttribArray(vColor);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	tex[starTex] = loadTex("textures/star.ktx");
	glGenTextures(1, &tex[starfieldColorTex]);

	glBindFramebuffer(GL_FRAMEBUFFER, fbo[starfieldFbo]);
	glBindTexture(GL_TEXTURE_CUBE_MAP, tex[starfieldColorTex]);
	glTexStorage2D(GL_TEXTURE_CUBE_MAP, 1, GL_RGB8, STARFIELD_RESOLUTION, STARFIELD_RESOLUTION);
	
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_REPEAT);

	glBindTexture(GL_TEXTURE_CUBE_MAP, 0);

	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X, tex[starfieldColorTex], 0);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_CUBE_MAP_POSITIVE_Y, tex[starfieldColorTex], 0);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, GL_TEXTURE_CUBE_MAP_POSITIVE_Z, tex[starfieldColorTex], 0);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT3, GL_TEXTURE_CUBE_MAP_NEGATIVE_X, tex[starfieldColorTex], 0);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT4, GL_TEXTURE_CUBE_MAP_NEGATIVE_Y, tex[starfieldColorTex], 0);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT5, GL_TEXTURE_CUBE_MAP_NEGATIVE_Z, tex[starfieldColorTex], 0);
	GLenum colorAttachmentList[] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2, GL_COLOR_ATTACHMENT3, GL_COLOR_ATTACHMENT4, GL_COLOR_ATTACHMENT5 };
	glDrawBuffers(len(colorAttachmentList), colorAttachmentList);

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}
//*****************************************************************************************************************
//****************************************************Planet****************************************************
//*****************************************************************************************************************
{
	sp = new sphere(500, 500, 1.0f, true);
	glBindVertexArray(vao[sphereVao]);
	glBindBuffer(GL_ARRAY_BUFFER, bo[sphereVbo]);
	glBufferData(GL_ARRAY_BUFFER, sp->getSizeOfVertices(), sp->getVertices(), GL_STATIC_DRAW);
	glVertexAttribPointer(vPosition, 3, GL_FLOAT, GL_FALSE, vertex_stride, BUFFER_OFFSET(position_offset));
	glVertexAttribPointer(vNormal, 3, GL_FLOAT, GL_FALSE, vertex_stride, BUFFER_OFFSET(normal_offset));
	glVertexAttribPointer(vTexCoord, 2, GL_FLOAT, GL_FALSE, vertex_stride, BUFFER_OFFSET(texcoord_offset));
	glEnableVertexAttribArray(vPosition);
	glEnableVertexAttribArray(vNormal);
	glEnableVertexAttribArray(vTexCoord);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, bo[sphereEabo]);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sp->getSizeOfElements(), sp->getElements(), GL_STATIC_DRAW);

	int w, h;
	unsigned char* data;
	glGenTextures(1, &tex[sataliteTex]);
	glBindTexture(GL_TEXTURE_2D_ARRAY, tex[sataliteTex]);
	glTexImage3D(GL_TEXTURE_2D_ARRAY, 0, GL_RGBA, 640, 320, 9, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
	string texNamesSatalites[] = {
		"textures/moon.jpg",
		"textures/deimos.jpg",
		"textures/ganymede.jpg",
		"textures/titan.jpg",
		"textures/titania.jpg",
		"textures/triton.jpg"
	};
	for(int i = 0; i < 6; i++) {
		data = SOIL_load_image(texNamesSatalites[i].c_str(), &w, &h, NULL, SOIL_LOAD_RGBA);
		glTexSubImage3D(GL_TEXTURE_2D_ARRAY, 0, 0, 0, i, w, h, 1, GL_RGBA, GL_UNSIGNED_BYTE, data);
		SOIL_free_image_data(data);
	}
	glGenerateMipmap(GL_TEXTURE_2D_ARRAY);

	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, GL_REPEAT);

	glBindTexture(GL_TEXTURE_2D_ARRAY, 0);
}
//*****************************************************************************************************************
//******************************************************Orbit******************************************************
//*****************************************************************************************************************
{
	glBindVertexArray(vao[orbitVao]);
	glBindBuffer(GL_ARRAY_BUFFER, bo[orbitVbo]);

	vmath::vec2 arr[100];
	for(int i = 0; i < 100; i++) {
		float theta = 2.0f * M_PI * ((float)i / 100);
		arr[i] = vmath::vec2(sin(theta), cos(theta) * 0.8);
	}
	glBufferData(GL_ARRAY_BUFFER, sizeof(arr), arr, GL_STATIC_DRAW);
	glVertexAttribPointer(vPosition, 2, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(0));
	glEnableVertexAttribArray(vPosition);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
}
//*****************************************************************************************************************
//***************************************************Solarsystem***************************************************
//*****************************************************************************************************************
{
	glBindFramebuffer(GL_FRAMEBUFFER, fbo[solarsystemFbo]);
	
	glGenTextures(1, &tex[solarsystemColorTex]);
	glBindTexture(GL_TEXTURE_2D, tex[solarsystemColorTex]);
	glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGBA32F, SOLARSYSTEM_RESOLUTION, SOLARSYSTEM_RESOLUTION);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, tex[solarsystemColorTex], 0);
	glBindTexture(GL_TEXTURE_2D, 0);
	
	glGenTextures(1, &tex[solarsystemPickTex]);
	glBindTexture(GL_TEXTURE_2D, tex[solarsystemPickTex]);
	glTexStorage2D(GL_TEXTURE_2D, 1, GL_R8I, SOLARSYSTEM_RESOLUTION, SOLARSYSTEM_RESOLUTION);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, tex[solarsystemPickTex], 0);
	glBindTexture(GL_TEXTURE_2D, 0);
	
	glBindRenderbuffer(GL_RENDERBUFFER, rbo[solarsystemDepthRbo]);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT32F, SOLARSYSTEM_RESOLUTION, SOLARSYSTEM_RESOLUTION);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, rbo[solarsystemDepthRbo]);
	glBindRenderbuffer(GL_RENDERBUFFER, 0);

	GLenum drawBuffers[] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1 };
	glDrawBuffers(len(drawBuffers), drawBuffers);

	if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
		printf("SolarSystem Framebuffer Incomplete Error");
	}
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}
//*****************************************************************************************************************
//*******************************************************Dof*******************************************************
//*****************************************************************************************************************
{
	glGenTextures(1, &tex[filterTex]);
	glBindTexture(GL_TEXTURE_2D, tex[filterTex]);
	glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGBA32F, SOLARSYSTEM_RESOLUTION, SOLARSYSTEM_RESOLUTION);
	glGenTextures(1, &tex[filterSizeRegulationTex]);
	glBindTexture(GL_TEXTURE_2D, tex[filterSizeRegulationTex]);
	glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGBA8, SOLARSYSTEM_RESOLUTION, SOLARSYSTEM_RESOLUTION);
	
	glBindFramebuffer(GL_FRAMEBUFFER, fbo[filterFbo]);
	glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, tex[filterSizeRegulationTex], 0);
	glDrawBuffer(GL_COLOR_ATTACHMENT0);

	glGenTextures(1, &tex[satDisplayTex]);
	glBindTexture(GL_TEXTURE_2D, tex[satDisplayTex]);
	glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGBA8, SOLARSYSTEM_RESOLUTION, SOLARSYSTEM_RESOLUTION);

	glBindFramebuffer(GL_FRAMEBUFFER, fbo[satDisplayFbo]);
	glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, tex[satDisplayTex], 0);
	glDrawBuffer(GL_COLOR_ATTACHMENT0);
}
//*****************************************************************************************************************
//*****************************************************SideMenu****************************************************
//*****************************************************************************************************************
{
	#define COUNT 84
	glBindVertexArray(vao[sideMenuVao]);
	glBindBuffer(GL_ARRAY_BUFFER, bo[sideMenuVbo]);
	vmath::vec4 vertices[COUNT];
	int n = 0;
	//MainBlock
	vertices[n++] = vmath::vec4(1.0f, -0.7f, 0.8f, 0.0f);
	vertices[n++] = vmath::vec4(1.0f, 0.7f, 0.8f, 1.0f);
	vertices[n++] = vmath::vec4(0.4f, 0.7f, 0.0f, 1.0f);
	vertices[n++] = vmath::vec4(0.4f, 0.7f, 0.0f, 1.0f);
	vertices[n++] = vmath::vec4(0.4f, -0.7f, 0.0f, 0.0f);
	vertices[n++] = vmath::vec4(1.0f, -0.7f, 0.8f, 0.0f);
	
	vertices[n++] = vmath::vec4(1.0f, -0.9f, 0.9f, 0.5f);
	vertices[n++] = vmath::vec4(1.0f, -0.7f, 0.9f, 0.5f);
	vertices[n++] = vmath::vec4(0.525f, -0.7f, 0.9f, 0.5f);
	vertices[n++] = vmath::vec4(0.525f, -0.7f, 0.9f, 0.5f);
	vertices[n++] = vmath::vec4(0.525f, -0.9f, 0.9f, 0.5f);
	vertices[n++] = vmath::vec4(1.0f, -0.9f, 0.9f, 0.5f);

	vertices[n++] = vmath::vec4(1.0f, 0.7f, 0.9f, 0.5f);
	vertices[n++] = vmath::vec4(0.525f, 0.7f, 0.9f, 0.5f);
	vertices[n++] = vmath::vec4(0.525f, 0.9f, 0.9f, 0.5f);
	vertices[n++] = vmath::vec4(0.525f, 0.9f, 0.9f, 0.5f);
	vertices[n++] = vmath::vec4(1.0f, 0.9f, 0.9f, 0.5f);
	vertices[n++] = vmath::vec4(1.0f, 0.7f, 0.9f, 0.5f);

	for(int i = 0; i < 10; i++) {
		GLfloat theta = 2.0f * M_PI * ((float)(i + 10) / 40);
		GLfloat theta_1 = 2.0f * M_PI * ((float)(i + 11) / 40);
		vertices[n++] = vmath::vec4(0.525, 0.7, 0.9f, 0.5f);
		vertices[n++] = vmath::vec4(0.525 + 0.125 * cos(theta), 0.7 + 0.2 * sin(theta), 0.9f, 0.5f);
		vertices[n++] = vmath::vec4(0.525 + 0.125 * cos(theta_1), 0.7 + 0.2 * sin(theta_1), 0.9f, 0.5f);
	}
	for(int i = 0; i < 10; i++) {
		GLfloat theta = 2.0f * M_PI * ((float)(i + 20) / 40);
		GLfloat theta_1 = 2.0f * M_PI * ((float)(i + 21) / 40);
		vertices[n++] = vmath::vec4(0.525, -0.7, 0.9f, 0.5f);
		vertices[n++] = vmath::vec4(0.525 + 0.125 * cos(theta), -0.7 + 0.2 * sin(theta), 0.9f, 0.5f);
		vertices[n++] = vmath::vec4(0.525 + 0.125 * cos(theta_1), -0.7 + 0.2 * sin(theta_1), 0.9f, 0.5f);
	}
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
	glVertexAttribPointer(vPosition, 2, GL_FLOAT, GL_FALSE, sizeof(vmath::vec4), BUFFER_OFFSET(0));
	glVertexAttribPointer(vTexCoord, 2, GL_FLOAT, GL_FALSE, sizeof(vmath::vec4), BUFFER_OFFSET(sizeof(vmath::vec2)));
	glEnableVertexAttribArray(vPosition);
	glEnableVertexAttribArray(vTexCoord);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
}
//*****************************************************************************************************************
//****************************************************SaturnRing***************************************************
//*****************************************************************************************************************
{
	glBindVertexArray(vao[saturnRingVao]);
	glBindBuffer(GL_ARRAY_BUFFER, bo[saturnRingVbo]);
	vmath::vec4 vertices[100];
	for(int i = 0; i < 50; i++) {
		GLfloat theta = 2.0f * M_PI * ((float)i / 49);
		vertices[i * 2 + 0] = vmath::vec4(cos(theta) * 1.8f, sin(theta) * 1.8f, 1.0f, (float)i);
		vertices[i * 2 + 1] = vmath::vec4(cos(theta) * 1.3f, sin(theta) * 1.3f, 0.0f, (float)i);
	}
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
	glVertexAttribPointer(vPosition, 2, GL_FLOAT, GL_FALSE, sizeof(vmath::vec4), BUFFER_OFFSET(0));
	glVertexAttribPointer(vTexCoord, 2, GL_FLOAT, GL_FALSE, sizeof(vmath::vec4), BUFFER_OFFSET(sizeof(vmath::vec2)));
	glEnableVertexAttribArray(vPosition);
	glEnableVertexAttribArray(vTexCoord);
	
	glGenTextures(1, &tex[saturnRingTex]);
	glBindTexture(GL_TEXTURE_2D, tex[saturnRingTex]);
	int w, h;
	unsigned char *data = SOIL_load_image("textures/saturnring.png", &w, &h, NULL, SOIL_LOAD_RGBA);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
	SOIL_free_image_data(data);
	glGenerateMipmap(GL_TEXTURE_2D);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
}
//*****************************************************************************************************************
//***************************************************ZoomedPlanet**************************************************
//*****************************************************************************************************************
{
	glBindFramebuffer(GL_FRAMEBUFFER, fbo[zoomedPlanetFbo]);
	
	glGenTextures(1, &tex[zoomedPlanetTex]);
	glBindTexture(GL_TEXTURE_2D, tex[zoomedPlanetTex]);
	glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGBA8, ZOOMEDPLANET_RESOLUTION, ZOOMEDPLANET_RESOLUTION);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, tex[zoomedPlanetTex], 0);
	glBindTexture(GL_TEXTURE_2D, 0);

	glBindRenderbuffer(GL_RENDERBUFFER, rbo[zoomedPlanetRbo]);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT32F, ZOOMEDPLANET_RESOLUTION, ZOOMEDPLANET_RESOLUTION);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, rbo[zoomedPlanetRbo]);
	glBindRenderbuffer(GL_RENDERBUFFER, 0);

	GLenum drawBuffers[] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1 };
	glDrawBuffers(len(drawBuffers), drawBuffers);

	if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
		printf("ZoomedPlanet Framebuffer Incomplete Error");
	}
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	vmath::vec4 vertices[] = {
		vmath::vec4(1.5f, 1.5f, 1.0f, 1.0f),
		vmath::vec4(-1.5f, 1.5f, 0.0f, 1.0f),
		vmath::vec4(-1.5f, -1.5f, 0.0f, 0.0f),
		vmath::vec4(1.5f, -1.5f, 1.0f, 0.0f)
	};

	glBindVertexArray(vao[zoomedPlanetVao]);
	glBindBuffer(GL_ARRAY_BUFFER, bo[zoomedPlanetVbo]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
	glVertexAttribPointer(vPosition, 2, GL_FLOAT, GL_FALSE, sizeof(vmath::vec4), BUFFER_OFFSET(0));
	glVertexAttribPointer(vTexCoord, 2, GL_FLOAT, GL_FALSE, sizeof(vmath::vec4), BUFFER_OFFSET(sizeof(vmath::vec2)));
	glEnableVertexAttribArray(vPosition);
	glEnableVertexAttribArray(vTexCoord);

	glGenTextures(1, &tex[borderColorTex]);
	glBindTexture(GL_TEXTURE_2D, tex[borderColorTex]);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, vmath::vec4(0.4f, 0.3f, 0.9f, 1.0f));
}
//*****************************************************************************************************************
//***************************************************NoiseTexture**************************************************
//*****************************************************************************************************************
{
	tex[noiseTex] = generate3DNoiseTexture(64);
}
//*****************************************************************************************************************
//*****************************************************Metaball****************************************************
//*****************************************************************************************************************
{
	glBindBuffer(GL_ARRAY_BUFFER, bo[sampleSsbo]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(float) * samplesTotal, NULL, GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	
	glBindBuffer(GL_ARRAY_BUFFER, bo[celltypeSsbo]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(int) * cellsTotal, NULL, GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	glBindBuffer(GL_ARRAY_BUFFER, bo[tritableSsbo]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(tri_table), tri_table, GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}
//*****************************************************************************************************************
//*************************************************SideMenuTextures************************************************
//*****************************************************************************************************************
{
	initPlanetInfo();
	char distFromSun[9][10];
	char revolution[9][10];
	char orbitalSpeed[9][10];
	char mass[9][10];
	char density[9][10];
	char escapeVelocity[9][10];
	char satallite[9][10];
	
	string names[] = {
		"SUN", "MERCURY", "VENUS", "EARTH", "MARS", "JUPITER", "SATURN", "URANUS", "NEPTUNE"
	};
	GLfloat xPositions[] = {
		60.0f, 35.0f, 63.0f, 67.0f, 74.0f, 47.0f, 53.0f, 51.0f, 41.0f
	};
	getDistanceFromSun(distFromSun);
	getRevolution(revolution);
	getOrbitalSpeed(orbitalSpeed);
	getMass(mass);
	getDensity(density);
	getEscapeVelocity(escapeVelocity);
	getSatallite(satallite);

	vmath::vec3 bgColor[] = {
		vmath::vec3(1.0f, 0.75f, 0.0f),
		vmath::vec3(0.4f, 0.4f, 0.4f),
		vmath::vec3(0.8f, 0.5f, 0.2f),
		vmath::vec3(0.1f, 0.25f, 0.5f),
		vmath::vec3(0.95f, 0.4f, 0.25f),
		vmath::vec3(0.7f, 0.6f, 0.5f),
		vmath::vec3(0.95f, 0.8f, 0.6f),
		vmath::vec3(0.6f, 0.75f, 0.8f),
		vmath::vec3(0.25f, 0.4f, 0.8f)
	};
	
	GLuint fbo;
	glGenTextures(lenPlanetTex, texInfo);
	glGenFramebuffers(1, &fbo);
	glBindFramebuffer(GL_FRAMEBUFFER, fbo);
	GLenum DrawBuffers[] = { GL_COLOR_ATTACHMENT0 };
	for(int i = 0; i < lenPlanetTex; i++) {
		glBindTexture(GL_TEXTURE_2D, texInfo[i]);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGBA8, INFO_RESOLUTION, INFO_RESOLUTION);
		glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, texInfo[i], 0);
		glDrawBuffers(len(DrawBuffers), DrawBuffers);
		if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
			printf("Incomplete Text");
		}
		glClearBufferfv(GL_COLOR, 0, vmath::vec4(bgColor[i], 0.75f));
		glViewport(0, 0, INFO_RESOLUTION, INFO_RESOLUTION);
		titleFont->renderFont(names[i].c_str(), vmath::vec2(xPositions[i], 450.0f), vmath::vec3(0.0f, 0.0f, 0.0f), 0.3f);
		char buffer[1024];
		sprintf(buffer, "Distance Sun:%s", distFromSun[i]);
		contentFont->renderFont(buffer, vmath::vec2(15.0f, 385.0f), vmath::vec3(0.0f, 0.0f, 0.0f), 0.23f);
		sprintf(buffer, "Revolution:%s", revolution[i]);
		contentFont->renderFont(buffer, vmath::vec2(15.0f, 330.0f), vmath::vec3(0.0f, 0.0f, 0.0f), 0.23f);
		sprintf(buffer, "Orbital Speed:%s", orbitalSpeed[i]);
		contentFont->renderFont(buffer, vmath::vec2(15.0f, 275.0f), vmath::vec3(0.0f, 0.0f, 0.0f), 0.23f);
		sprintf(buffer, "Mass:%s", mass[i]);
		contentFont->renderFont(buffer, vmath::vec2(15.0f, 220.0f), vmath::vec3(0.0f, 0.0f, 0.0f), 0.23f);
		sprintf(buffer, "Density:%s", density[i]);
		contentFont->renderFont(buffer, vmath::vec2(15.0f, 165.0f), vmath::vec3(0.0f, 0.0f, 0.0f), 0.23f);
		sprintf(buffer, "Escape Velocity:%s", escapeVelocity[i]);
		contentFont->renderFont(buffer, vmath::vec2(15.0f, 110.0f), vmath::vec3(0.0f, 0.0f, 0.0f), 0.23f);
		sprintf(buffer, "Satallite:%s", satallite[i]);
		contentFont->renderFont(buffer, vmath::vec2(15.0f, 55.0f), vmath::vec3(0.0f, 0.0f, 0.0f), 0.23f);
	}
}
	glEnable(GL_PROGRAM_POINT_SIZE);
	glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glDepthFunc(GL_LEQUAL);
}


#define SPEED_UP 1
const static int focusCountLimit = 25;
const static int initPlanetCountLimitMercury = 100 / SPEED_UP;
const static int initPlanetCountLimitVenus = 100 / SPEED_UP;
const static int initPlanetCountLimitEarth = 100 / SPEED_UP;
const static int initPlanetCountLimitMars = 100 / SPEED_UP;
const static int initPlanetCountLimitJupiter = 300 / SPEED_UP;
const static int initPlanetCountLimitSaturn = 300 / SPEED_UP;
const static int initPlanetCountLimitUranus = 300 / SPEED_UP;
const static int initPlanetCountLimitNeptune = 300 / SPEED_UP;
const static int initCameraCountLimit = 1600; // Must be found by printing counter after initial
const static int fadeCountLimit = 100;
vmath::vec3 planetTransIncrement;

void update() {
	static int focusCount = 0;
	static int initPlanetCount = 0;
	static int fadeCount = 0;
	const static float sigmaIncrement = 2.0f / focusCountLimit;
	const static float sideMenuXIncrement = -0.6f / focusCountLimit;
	const static float planetScaleIncrement = 1.0f / focusCountLimit;
	const static float initPlanetIncrementMercury = 1.0f / initPlanetCountLimitMercury;
	const static float initPlanetIncrementVenus = 1.0f / initPlanetCountLimitVenus;
	const static float initPlanetIncrementEarth = 1.0f / initPlanetCountLimitEarth;
	const static float initPlanetIncrementMars = 1.0f / initPlanetCountLimitMars;
	const static float initPlanetIncrementJupiter = 1.0f / initPlanetCountLimitJupiter;
	const static float initPlanetIncrementSaturn = 1.0f / initPlanetCountLimitSaturn;
	const static float initPlanetIncrementUranus = 1.0f / initPlanetCountLimitUranus;
	const static float initPlanetIncrementNeptune = 1.0f / initPlanetCountLimitNeptune;
	const static float initCameraRotIncrement = 3240.0f / initCameraCountLimit;
	const static float fadeCountIncrement = 1.0f / fadeCountLimit;
	const static float initZoomOutIncrement = 25.0f / initCameraCountLimit;
	const static float focusDepthIncrement = 200.0f / focusCountLimit;
	static bool isFadingTitle = true;
	static bool isFadingPlanet = true;
	static int currentPlanet = 1;

	if(currentAnimating == Title) {
		if(isFadingTitle) {
			if(fadeCount < fadeCountLimit) {
				updatevals.fade -= fadeCountIncrement;
				fadeCount++;
			} else {
				updatevals.fade = 0.0f;
				fadeCount = 0;
				isFadingTitle = false;
			}
		} else {
			if(fadeCount < fadeCountLimit) {
				fadeCount++;
				updatevals.fade += fadeCountIncrement;
			} else {
				updatevals.fade = 1.0f;
				fadeCount = 0;
				currentAnimating = Initial;
			}
		}
	} else {
		currentTimeInHours += 2 * SPEED_UP;
		if(currentAnimating == Initial) {
			if(isFadingPlanet) {
				if(fadeCount < fadeCountLimit) {
					updatevals.fade -= fadeCountIncrement;
					fadeCount++;
				} else {
					updatevals.fade = 0.0f;
					fadeCount = 0;
					isFadingPlanet = false;
				}	
			} else if(currentPlanet < 9) {
				if(currentPlanet == 1) {
					if(initPlanetCount < initPlanetCountLimitMercury) {
						updatevals.planet.initialTransScaleMercury += initPlanetIncrementMercury;
					} else {
						currentPlanet++;
						initPlanetCount = 0;
					}
				} else if(currentPlanet == 2) {
					if(initPlanetCount < initPlanetCountLimitVenus) {
						updatevals.planet.initialTransScaleVenus += initPlanetIncrementVenus;
					} else {
						currentPlanet++;
						initPlanetCount = 0;
					}
				} else if(currentPlanet == 3) {
					if(initPlanetCount < initPlanetCountLimitEarth) {
						updatevals.planet.initialTransScaleEarth += initPlanetIncrementEarth;
					} else {
						currentPlanet++;
						initPlanetCount = 0;
					}
				} else if(currentPlanet == 4) {
					if(initPlanetCount < initPlanetCountLimitMars) {
						updatevals.planet.initialTransScaleMars += initPlanetIncrementMars;
					} else {
						currentPlanet++;
						initPlanetCount = 0;
					}
				} else if(currentPlanet == 5) {
					if(initPlanetCount < initPlanetCountLimitJupiter) {
						updatevals.planet.initialTransScaleJupiter += initPlanetIncrementJupiter;
					} else {
						currentPlanet++;
						initPlanetCount = 0;
					}
				} else if(currentPlanet == 6) {
					if(initPlanetCount < initPlanetCountLimitSaturn) {
						updatevals.planet.initialTransScaleSaturn+= initPlanetIncrementSaturn;
					} else {
						currentPlanet++;
						initPlanetCount = 0;
					}
				} else if(currentPlanet == 7) {
					if(initPlanetCount < initPlanetCountLimitUranus) {
						updatevals.planet.initialTransScaleUranus += initPlanetIncrementUranus;
					} else {
						currentPlanet++;
						initPlanetCount = 0;
					}
				} else if(currentPlanet == 8) {
					if(initPlanetCount < initPlanetCountLimitNeptune) {
						updatevals.planet.initialTransScaleNeptune += initPlanetIncrementNeptune;
					} else {
						currentPlanet++;
						initPlanetCount = 0;
					}
				}
				initPlanetCount++;
				distToCamera += initZoomOutIncrement;
			} else {
				if(fadeCount < fadeCountLimit) {
					updatevals.fade += fadeCountIncrement;
					fadeCount++;
				} else {
					updatevals.fade = 1.0f;
					fadeCount = 0;
					currentAnimating = NothingFadeIn;
				}	
			}
		} else if(currentAnimating == NothingFadeIn) {
			if(fadeCount < fadeCountLimit) {
				updatevals.fade -= fadeCountIncrement;
				fadeCount++;
			} else {
				updatevals.fade = 0.0f;
				fadeCount = 0;
				currentAnimating = Nothing;
			}
		}else if(currentAnimating == ZoomIn) {
			if(focusCount < focusCountLimit) {
				updatevals.sideMenuX += sideMenuXIncrement;
				updatevals.planet.scale += planetScaleIncrement;
				updatevals.planet.position += planetTransIncrement;
				isDOFEnable = true;
			} else {
				updatevals.planet.scale = 1.0f;
				updatevals.planet.position = FinalZoomedPlanetCoords;
				currentAnimating = Nothing;
				focusCount = 0;
			}
			focusCount++;
		} else if(currentAnimating == ZoomOut) {
			if(focusCount <= focusCountLimit) {
				updatevals.sideMenuX -= sideMenuXIncrement;
				updatevals.planet.scale -= planetScaleIncrement;
				updatevals.planet.position -= planetTransIncrement;
				isDOFEnable = false;
			} else {
				currentAnimating = Nothing;
				focusCount = 0;
				currentZoomedPlanet = -1;
				updatevals.sigma = 0.0f;
				updatevals.sideMenuX = 0.6f;
				updatevals.planet.scale = 0.0f;
			}
			focusCount++;
		}
	}

	ALenum status;
	alGetSourcei(source, AL_SOURCE_STATE, &status);
	if(status != AL_PLAYING) {
		alSourcePlay(source);
	}
}

vmath::mat4 cameraMat;
vmath::mat4 mMats[9];
	
void renderGL(void) {
	update();

	vmath::mat4 spinRot;
	if(currentAnimating == Initial) {
		spinRot = vmath::rotate(updatevals.cameraRot, vmath::normalize(vmath::vec3(0.0f, 1.0f, 0.0f)));
	} else {
		spinRot = vmath::mat4::identity();
	}
	cameraMat = vmath::translate(0.0f, 0.0f, -distToCamera) * camera->getRotationMatrix() * spinRot ;
	const vmath::mat4 perspective = vmath::perspective(60.0f, winSize.w / winSize.h, 0.1f, 1000.0f);

	if(currentAnimating == Title) {
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glClearBufferfv(GL_COLOR, 0, vmath::vec4(1.0f));
		glViewport(0, 0, (GLsizei)winSize.w, (GLsizei)winSize.h);
		glUseProgram(program.solarsystem);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, tex[titleTex]);
		glBindVertexArray(blankVao);
		glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
	} else if(currentAnimating == Initial) {
		glBindFramebuffer(GL_FRAMEBUFFER, fbo[starfieldFbo]);

		glClearBufferfv(GL_COLOR, 0, vmath::vec4(0.0f, 0.0f, 0.0f, 1.0f));
		glClearBufferfv(GL_COLOR, 1, vmath::vec4(0.0f, 0.0f, 0.0f, 1.0f));
		glClearBufferfv(GL_COLOR, 2, vmath::vec4(0.0f, 0.0f, 0.0f, 1.0f));
		glClearBufferfv(GL_COLOR, 3, vmath::vec4(0.0f, 0.0f, 0.0f, 1.0f));
		glClearBufferfv(GL_COLOR, 4, vmath::vec4(0.0f, 0.0f, 0.0f, 1.0f));
		glClearBufferfv(GL_COLOR, 5, vmath::vec4(0.0f, 0.0f, 0.0f, 1.0f));
		
		glViewport(0, 0, (GLsizei)STARFIELD_RESOLUTION, (GLsizei)STARFIELD_RESOLUTION);

		glUseProgram(program.starfield);
		glBindVertexArray(vao[starVao]);
		
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, tex[starTex]);

		glEnable(GL_BLEND);
		glDrawArrays(GL_POINTS, 0, NUM_STARS);
		glDisable(GL_BLEND);

		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glClearBufferfv(GL_COLOR, 0, vmath::vec4(1.0f, 0.0f, 0.5f, 1.0f));
		glViewport(0, 0, (GLsizei)winSize.w, (GLsizei)winSize.h);
	
		glUseProgram(program.skybox);

		glUniformMatrix4fv(uniforms.vMat, 1, GL_FALSE, cameraMat);
		
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_CUBE_MAP, tex[starfieldColorTex]);

		glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

		//Actual Metaball Code
		glBindVertexArray(blankVao);
		glEnable(GL_RASTERIZER_DISCARD);

		glUseProgram(program.scalarfield);
		glUniform1i(uniforms.sf.samplesPerAxis, samplesPerAxis);
		GLfloat phiInRad =  2.0f * M_PI;
		float sphereData[] = {
			0.0f, 0.0f, 0.0f, 3.5f,
			5.5f * updatevals.planet.initialTransScaleMercury * sin(phiInRad * ((currentTimeInHours / 24.0f) / 88.0f)), 0.0f, 5.5f * updatevals.planet.initialTransScaleMercury * cos(phiInRad * ((currentTimeInHours / 24.0f) / 88.0f)), 0.2f,
			6.5f * updatevals.planet.initialTransScaleVenus * sin(phiInRad * ((currentTimeInHours / 24.0f) / 140.0f)), 0.0f, 6.5f * updatevals.planet.initialTransScaleVenus * cos(phiInRad * ((currentTimeInHours / 24.0f) / 140.0f)), 0.25f,
			7.7f * updatevals.planet.initialTransScaleEarth * sin(phiInRad * ((currentTimeInHours / 24.0f) / 200.0f)), 0.0f, 7.7f * updatevals.planet.initialTransScaleEarth * cos(phiInRad * ((currentTimeInHours / 24.0f) / 200.0f)), 0.3f,
			8.8f * updatevals.planet.initialTransScaleMars * sin(phiInRad * ((currentTimeInHours / 24.0f) / 250.0f)), 0.0f, 8.8f * updatevals.planet.initialTransScaleMars * cos(phiInRad * ((currentTimeInHours / 24.0f) / 250.0f)), 0.31f,
			11.5f * updatevals.planet.initialTransScaleJupiter * sin(phiInRad * ((currentTimeInHours / 24.0f) / 400.0f)), 0.0f, 11.5f * updatevals.planet.initialTransScaleJupiter * cos(phiInRad * ((currentTimeInHours / 24.0f) / 400.0f)), 1.1f,
			14.5f * updatevals.planet.initialTransScaleSaturn * sin(phiInRad * ((currentTimeInHours / 24.0f) / 450.0f)), 0.0f, 14.5f * updatevals.planet.initialTransScaleSaturn * cos(phiInRad * ((currentTimeInHours / 24.0f) / 450.0f)), 1.0f,
			17.3f * updatevals.planet.initialTransScaleUranus * sin(phiInRad * ((currentTimeInHours / 24.0f) / 580.0f)), 0.0f, 17.3f * updatevals.planet.initialTransScaleUranus * cos(phiInRad * ((currentTimeInHours / 24.0f) / 580.0f)), 0.75f,
			19.4f * updatevals.planet.initialTransScaleNeptune * sin(phiInRad * ((currentTimeInHours / 24.0f) / 650.0f)), 0.0f, 19.4f * updatevals.planet.initialTransScaleNeptune * cos(phiInRad * ((currentTimeInHours / 24.0f) / 650.0f)), 0.55f
		};
		// float sphereData[] = {
		// 	0.0f, 0.0f, 0.0f, 3.5f,
		// 	5.5f, 0.0f, 0.0f, 0.2f,
		// 	6.5f, 0.0f, 0.0f, 0.25f,
		// 	7.7f, 0.0f, 0.0f, 0.3f,
		// 	8.8f, 0.0f, 0.0f, 0.31f,
		// 	11.5f, 0.0f, 0.0f, 1.1f,
		// 	14.5f, 0.0f, 0.0f, 1.0f,
		// 	17.3f, 0.0f, 0.0f, 0.75f,
		// 	19.4f, 0.0f, 0.0f, 0.55f
		// };
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_3D, tex[noiseTex]);
		glUniform4fv(uniforms.sf.radius, 9, sphereData);
		glBindBufferBase(GL_TRANSFORM_FEEDBACK_BUFFER, 0, bo[sampleSsbo]);
		glBeginTransformFeedback(GL_POINTS);
		glDrawArrays(GL_POINTS, 0, samplesTotal);
		glEndTransformFeedback();
		
		glUseProgram(program.marchingcube);
		glUniform1i(uniforms.mc.cellsPerAxis, cellsPerAxis);
		glUniform1f(uniforms.mc.isolevel, isolevel);
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, uniforms.mc.scalarField, bo[sampleSsbo]);
		glBindBufferBase(GL_TRANSFORM_FEEDBACK_BUFFER, 0, bo[celltypeSsbo]);
		glBeginTransformFeedback(GL_POINTS);
		glDrawArrays(GL_POINTS, 0, cellsTotal);
		glEndTransformFeedback();

		glDisable(GL_RASTERIZER_DISCARD);

		glUseProgram(program.triangulation);
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, uniforms.tri.cellType, bo[celltypeSsbo]);
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, uniforms.tri.scalarField, bo[sampleSsbo]);
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, uniforms.tri.triTable, bo[tritableSsbo]);
		glUniformMatrix4fv(uniforms.mMat, 1, GL_FALSE, vmath::rotate(90.0f, 0.0f, 1.0f, 0.0f));
		glUniformMatrix4fv(uniforms.vMat, 1, GL_FALSE, vmath::translate(0.0f, 0.0f, -distToCamera) * vmath::rotate(30.0f, 1.0f, 0.0f, 0.0f));
		glUniformMatrix4fv(uniforms.pMat, 1, GL_FALSE, perspective);
		glUniform4fv(7, 1, sphereData);
		glUniform1i(uniforms.tri.cellsPerAxis, cellsPerAxis);
		glUniform1i(uniforms.tri.numVertices, verticesPerCell);
		glUniform1f(uniforms.tri.isolevel, isolevel);
		glDrawArrays(GL_TRIANGLES, 0, verticesPerCell * cellsTotal);

	} else {
		glBindFramebuffer(GL_FRAMEBUFFER, fbo[starfieldFbo]);

		glClearBufferfv(GL_COLOR, 0, vmath::vec4(0.0f, 0.0f, 0.0f, 1.0f));
		glClearBufferfv(GL_COLOR, 1, vmath::vec4(0.0f, 0.0f, 0.0f, 1.0f));
		glClearBufferfv(GL_COLOR, 2, vmath::vec4(0.0f, 0.0f, 0.0f, 1.0f));
		glClearBufferfv(GL_COLOR, 3, vmath::vec4(0.0f, 0.0f, 0.0f, 1.0f));
		glClearBufferfv(GL_COLOR, 4, vmath::vec4(0.0f, 0.0f, 0.0f, 1.0f));
		glClearBufferfv(GL_COLOR, 5, vmath::vec4(0.0f, 0.0f, 0.0f, 1.0f));
		
		glViewport(0, 0, (GLsizei)STARFIELD_RESOLUTION, (GLsizei)STARFIELD_RESOLUTION);

		glUseProgram(program.starfield);
		glBindVertexArray(vao[starVao]);
		
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, tex[starTex]);

		glEnable(GL_BLEND);
		glDrawArrays(GL_POINTS, 0, NUM_STARS);
		glDisable(GL_BLEND);

		glBindFramebuffer(GL_FRAMEBUFFER, fbo[solarsystemFbo]);

		glClearBufferfv(GL_COLOR, 0, vmath::vec4(0.3f, 0.3f, 0.3f, 1.0f));
		glClearBufferfv(GL_DEPTH, 0, vmath::vec1(1.0f));
		glClearBufferiv(GL_COLOR, 1, vmath::ivec1(-1));
		
		glViewport(0, 0, SOLARSYSTEM_RESOLUTION, SOLARSYSTEM_RESOLUTION);

		glUseProgram(program.skybox);

		glUniformMatrix4fv(uniforms.vMat, 1, GL_FALSE, cameraMat);
		
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_CUBE_MAP, tex[starfieldColorTex]);

		glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

		glEnable(GL_DEPTH_TEST);

		GLfloat rotArr[] = {0.0f, 0.0f, -177.0f, 23.4f, 25.2f, 3.1f, 26.7f, -97.8, 28.3f};
		GLfloat scaleArr[] = { 3.5f, 0.2f, 0.25f, 0.3f, 0.31f, 1.1f, 1.0f, 0.75f, 0.55f };
		GLfloat translateArr[] = { 0.0f, 5.5f, 6.5f, 7.7f, 9.1f, 12.5f, 16.5f, 20.3, 23.5 };
		GLfloat revDays[] = { 0.0f, 88.0f, 140.708f, 200.25f, 250.958f, 400.0f, 450.0f, 580.0f, 650.0f };
		GLfloat rotHours[] = { 50.0f, 1416.0f, 5832.0f, 23.933f, 24.61f, 16.933f, 18.65f, 20.233f, 21.116f };
		for(int i = 0; i < 9; i++) {
			if(revDays[i] != 0.0f) {
				GLfloat thetaInDeg = 360.0f * (currentTimeInHours / rotHours[i]);
				GLfloat phiInRad =  2.0f * M_PI * ((currentTimeInHours / 24.0f) / revDays[i]);
				mMats[i] = vmath::translate(translateArr[i] * cos(-phiInRad), 0.0f, translateArr[i] * 0.8f * sin(-phiInRad)) * vmath::rotate(rotArr[i], 0.0f, 0.0f, 1.0f) * vmath::rotate(thetaInDeg * 0.05f, 0.0f, 1.0f, 0.0f) * vmath::scale(scaleArr[i]);
			} else {
				GLfloat thetaInDeg = 360.0f * (currentTimeInHours / rotHours[i]);
				mMats[i] = vmath::rotate(thetaInDeg * 0.05f, 0.0f, 1.0f, 0.0f) * vmath::scale(scaleArr[i]);
			}
			glUseProgram(program.noise[i]);
			glUniformMatrix4fv(uniforms.pMat, 1, GL_FALSE, perspective);
			glUniformMatrix4fv(uniforms.mMat, 1, GL_FALSE, cameraMat * mMats[i]);
			glUniform3fv(uniforms.zoomplanet.lightPos, 1, vmath::vec3(0.0f, 0.0f, -distToCamera));
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_3D, tex[noiseTex]);
			glBindVertexArray(vao[sphereVao]);
			glDrawElements(GL_TRIANGLES, sp->getNumOfTriangle(), GL_UNSIGNED_INT, NULL);
		}
		glUseProgram(program.ring);
		glUniformMatrix4fv(uniforms.pMat, 1, GL_FALSE, perspective);
		glUniformMatrix4fv(uniforms.mMat, 1, GL_FALSE, cameraMat * mMats[6]);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, tex[saturnRingTex]);
		glBindVertexArray(vao[saturnRingVao]);
		glDrawArrays(GL_TRIANGLE_STRIP, 0, 100);
		
		glUseProgram(program.orbit);
		glBindVertexArray(vao[orbitVao]);
		glUniformMatrix4fv(uniforms.pMat, 1, GL_FALSE, perspective);
		glUniformMatrix4fv(uniforms.vMat, 1, GL_FALSE, cameraMat);
		glUniform1fv(uniforms.mMat, 8, &translateArr[1]);
		glDrawArraysInstanced(GL_LINE_LOOP, 0, 100, 8);

		glDisable(GL_DEPTH_TEST);

		GLuint finalTex = tex[solarsystemColorTex];
		if(isDOFEnable) {
			glBindFramebuffer(GL_FRAMEBUFFER, fbo[filterFbo]);
			glUseProgram(program.doffilter);
			
			const int step = log2(SOLARSYSTEM_RESOLUTION);
			for(int i = 0, off = 1; i < step; i++) {
				glBindImageTexture(i % 2, tex[solarsystemColorTex], 0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA32F);
				glBindImageTexture((i + 1) % 2, tex[filterTex], 0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA32F);
				glUniform2i(0, off, 0);
				glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

				glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
				off *= 2;
			}
			
			for(int i = 0, off = 1; i < step; i++) {
				glBindImageTexture((i + step) % 2, tex[solarsystemColorTex], 0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA32F);
				glBindImageTexture((i + 1 + step) % 2, tex[filterTex], 0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA32F);
				glUniform2i(0, 0, off);
				glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

				glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
				off *= 2;
			}

			glBindFramebuffer(GL_FRAMEBUFFER, fbo[satDisplayFbo]);
			glClearBufferfv(GL_COLOR, 0, vmath::vec4(1.0f, 0.0f, 1.0f, 1.0f));
			glUseProgram(program.satdisplay);
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, tex[solarsystemColorTex]);
			glUniform1f(uniforms.dof.focalDist, 37.0f);
			glUniform1f(uniforms.dof.focalDepth, 10.0f);
			glBindVertexArray(vao[blankVao]);
			glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

			finalTex = tex[satDisplayTex];
		}

		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glClearBufferfv(GL_COLOR, 0, vmath::vec4(1.0f, 1.0f, 0.0f, 1.0f));
		glClearBufferfv(GL_DEPTH, 0, vmath::vec1(1.0f));
		glViewport(0, 0, (GLsizei)winSize.w, (GLsizei)winSize.h);

		glUseProgram(program.solarsystem);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, finalTex);
		glBindVertexArray(blankVao);
		glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

		if(currentZoomedPlanet != -1) {
			glBindFramebuffer(GL_FRAMEBUFFER, fbo[zoomedPlanetFbo]);
			glClearBufferfv(GL_COLOR, 0, vmath::vec4(0.0f, 0.0f, 0.0f, 0.6f));
			glClearBufferfv(GL_DEPTH, 0, vmath::vec1(1.0f));
			glViewport(0, 0, ZOOMEDPLANET_RESOLUTION, ZOOMEDPLANET_RESOLUTION);
		
			glEnable(GL_DEPTH_TEST);
			
			glUseProgram(program.noise[currentZoomedPlanet]);
			glUniformMatrix4fv(uniforms.pMat, 1, GL_FALSE, vmath::perspective(45.0f, 1.0f, 0.1f, 1000.0f));
			glUniformMatrix4fv(uniforms.mMat, 1, GL_FALSE, vmath::translate(0.0f, 0.0f, -5.2f) * vmath::rotate(rotArr[currentZoomedPlanet], 0.0f, 0.0f, 1.0f) * vmath::rotate(currentTimeInHours / 2, 0.0f, 1.0f, 0.0f));
			glUniform1i(uniforms.zoomplanet.zoomedLayer, currentZoomedPlanet);
			glUniform3f(uniforms.zoomplanet.lightPos, cos(vmath::radians(currentTimeInHours) / 8) * 5.0f, 0.0f, sin(vmath::radians(currentTimeInHours) / 8) * 5.0f);
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_3D, tex[noiseTex]);
			glBindVertexArray(vao[sphereVao]);
			glDrawElements(GL_TRIANGLES, sp->getNumOfTriangle(), GL_UNSIGNED_INT, NULL);
			if(currentZoomedPlanet >= 3) {
				glUseProgram(program.zoomedsatalite);
				glUniformMatrix4fv(uniforms.pMat, 1, GL_FALSE, vmath::perspective(45.0f, 1.0f, 0.1f, 1000.0f));
				glUniformMatrix4fv(uniforms.mMat, 1, GL_FALSE, vmath::translate(0.0f, 0.0f, -5.2f) * vmath::rotate(rotArr[currentZoomedPlanet], 0.0f, 0.0f, 1.0f) * vmath::rotate(currentTimeInHours / 2, 0.0f, 1.0f, 0.0f));
				glUniform3f(uniforms.zoomplanet.lightPos, cos(vmath::radians(currentTimeInHours) / 8) * 5.0f, 0.0f, sin(vmath::radians(currentTimeInHours) / 8) * 5.0f);
				glUniform1i(uniforms.zoomplanet.zoomedLayer, currentZoomedPlanet);
				glActiveTexture(GL_TEXTURE0);
				glBindTexture(GL_TEXTURE_2D_ARRAY, tex[sataliteTex]);
				glUniform1i(uniforms.zoomplanet.zoomedLayer, currentZoomedPlanet - 3);
				glUniformMatrix4fv(uniforms.mMat, 1, GL_FALSE, vmath::translate(0.0f, 0.0f, -5.2f) * vmath::rotate(currentTimeInHours / 2, 0.0f, 1.0f, 0.0f) * vmath::translate(2.0f * sin(vmath::radians(currentTimeInHours / 4)), 0.0f, 2.0f * cos(vmath::radians(currentTimeInHours / 4))) * vmath::rotate(currentTimeInHours/ 2, 0.0f, 1.0f, 0.0f) * vmath::scale(0.14f));
				glBindVertexArray(vao[sphereVao]);
				glDrawElements(GL_TRIANGLES, sp->getNumOfTriangle(), GL_UNSIGNED_INT, NULL);
			}
			if(currentZoomedPlanet == 6) {
				glUseProgram(program.ring);
				glUniformMatrix4fv(uniforms.pMat, 1, GL_FALSE, vmath::perspective(45.0f, 1.0f, 0.1f, 1000.0f));
				glUniformMatrix4fv(uniforms.mMat, 1, GL_FALSE, vmath::translate(0.0f, 0.0f, -5.2f) * vmath::rotate(30.0f, 0.707f, 0.0f, 0.707f) * vmath::rotate(currentTimeInHours / 2, 0.0f, 1.0f, 0.0f));
				glActiveTexture(GL_TEXTURE0);
				glBindTexture(GL_TEXTURE_2D, tex[saturnRingTex]);
				glBindVertexArray(vao[saturnRingVao]);
				glDrawArrays(GL_TRIANGLE_STRIP, 0, 100);
			}

			glDisable(GL_DEPTH_TEST);
			
			glBindFramebuffer(GL_FRAMEBUFFER, 0);
			glViewport(0, 0, (GLsizei)winSize.w, (GLsizei)winSize.h);

			glUseProgram(program.sidemenu);
			
			glUniformMatrix4fv(uniforms.mMat, 1, GL_FALSE, perspective * vmath::translate(updatevals.planet.position) * vmath::scale(updatevals.planet.scale));
			glUniform1i(uniforms.sidemenu.isBorder, 1);
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, tex[zoomedPlanetTex]);
			glEnable(GL_BLEND);
			glBindVertexArray(vao[zoomedPlanetVao]);
			glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
			glDisable(GL_BLEND);
			
			glUniformMatrix4fv(uniforms.mMat, 1, GL_FALSE, vmath::translate(updatevals.sideMenuX, 0.0f, 0.0f));
			glUniform1i(uniforms.sidemenu.isBorder, 0);
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, texInfo[currentZoomedPlanet]);
			glEnable(GL_BLEND);
			glBindVertexArray(vao[sideMenuVao]);
			glDrawArrays(GL_TRIANGLES, 0, COUNT);
			glDisable(GL_BLEND);
		}
	}

	glEnable(GL_BLEND);
	glUseProgram(program.fade);
	glUniform1f(uniforms.fade.fadeAlpha, updatevals.fade);
	glBindVertexArray(vao[blankVao]);
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
	glDisable(GL_BLEND);
}

void uninitGL(void) {

}

void mousemoveGL(int cx, int cy, bool isDragging) {
	if(currentAnimating == Nothing && currentZoomedPlanet == -1) {
		if(!isDragging) {
			if((currentZoomedPlanet = mousePicker(cx, cy, SOLARSYSTEM_RESOLUTION, fbo[solarsystemFbo])) == -1) {
				camera->click(vmath::ivec2(cx, cy));
				camera->enableDrag();
			} else {
				vmath::mat4 wvp = cameraMat * mMats[currentZoomedPlanet];
				updatevals.planet.position = vmath::vec3(wvp[3][0], wvp[3][1], wvp[3][2]);
				planetTransIncrement = (FinalZoomedPlanetCoords - updatevals.planet.position) / focusCountLimit;
				camera->disableDrag();
				currentAnimating = ZoomIn;
			}
		} else {
			camera->drag(vmath::ivec2(cx, cy));
		}
	}
}

void keyboardGL(int scancode) {
	if(currentAnimating == Nothing) {
		vmath::mat4 wvp;
		switch(scancode) {
		case XK_W: case XK_w:
			if(currentZoomedPlanet == -1) {
				if(distToCamera > 5.0f) {
					distToCamera -= 0.5f;
				}
			}
			break;
		case XK_S: case XK_s:
			if(currentZoomedPlanet == -1) {
				if(distToCamera < 35.0f) {
					distToCamera += 0.5f;
				}
			}
			break;
		case XK_Left:
			camera->click(vmath::ivec2(winSize.w / 2 + 10, 0));
			camera->drag(vmath::ivec2(winSize.w / 2 - 10, 0));
			break;
		case XK_Right:
			camera->click(vmath::ivec2(winSize.w / 2 - 10, 0));
			camera->drag(vmath::ivec2(winSize.w / 2 + 10, 0));
			break;
		case XK_Up:
			camera->click(vmath::ivec2(winSize.w / 2, winSize.w / 2 - 10));
			camera->drag(vmath::ivec2(winSize.w / 2, winSize.w / 2 + 10));
			break;
		case XK_Down:
			camera->click(vmath::ivec2(winSize.w / 2, winSize.w / 2 + 10));
			camera->drag(vmath::ivec2(winSize.w / 2, winSize.w / 2 - 10));
			break;
		case XK_B: case XK_b:
			wvp = cameraMat * mMats[currentZoomedPlanet];
			planetTransIncrement = (FinalZoomedPlanetCoords - vmath::vec3(wvp[3][0], wvp[3][1], wvp[3][2])) / focusCountLimit;
			currentAnimating = ZoomOut;
			break;
		case XK_H: case XK_h:
			camera->restoreFromHomeMatrix();
			break;
		}
	}
}