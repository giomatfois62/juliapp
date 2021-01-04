#include "application.h"
#include "imgui/myimgui.h"
#include "opengl/geometry.h"

using namespace gl;
using namespace glm;

static float xmin = -2, xmax = 2, ymin = -2, ymax = 2;
double scale = 1;
const double scaleFactor = 1.15;
glm::vec2 trans;
double trans1 = 0, trans2 = 0;

gl::Shader *m_single;
gl::Shader *m_double;
gl::Shader *m_screen;

unsigned int framebuffer;
unsigned int texColorBuffer;
unsigned int rbo;
unsigned int quadVAO, quadVBO;

void cleanupFrameBuffer()
{
    glDeleteFramebuffers(1, &framebuffer);
    glDeleteRenderbuffers(1, &rbo);
    glDeleteTextures(1, &texColorBuffer);
}

void setupFrameBuffer(int w, int h)
{
    glGenFramebuffers(1, &framebuffer);
    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
    
    glGenTextures(1, &texColorBuffer);
    glBindTexture(GL_TEXTURE_2D, texColorBuffer);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, w, h, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texColorBuffer, 0);
    
    glGenRenderbuffers(1, &rbo);
    glBindRenderbuffer(GL_RENDERBUFFER, rbo); 
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, w, h);  
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, rbo);

    if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        std::cout << "ERROR::FRAMEBUFFER:: Framebuffer is not complete!" << std::endl;
    
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void setupQuad()
{
    float quadVertices[] = { 
        // positions   // texCoords
        -1.0f,  1.0f,  0.0f, 1.0f,
        -1.0f, -1.0f,  0.0f, 0.0f,
         1.0f, -1.0f,  1.0f, 0.0f,

        -1.0f,  1.0f,  0.0f, 1.0f,
         1.0f, -1.0f,  1.0f, 0.0f,
         1.0f,  1.0f,  1.0f, 1.0f
    };
    
    // screen quad VAO
    glGenVertexArrays(1, &quadVAO);
    glGenBuffers(1, &quadVBO);
    glBindVertexArray(quadVAO);
    glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
}

void firstPass()
{
    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
    glEnable(GL_DEPTH_TEST);
        
    glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); // we're not using the stencil buffer now
}

void secondPass()
{
    glBindFramebuffer(GL_FRAMEBUFFER, 0); // back to default
    glDisable(GL_DEPTH_TEST); // disable depth test so screen-space quad isn't discarded due to depth test.
    
    glClearColor(1.0f, 1.0f, 1.0f, 1.0f); 
    glClear(GL_COLOR_BUFFER_BIT);
    
    glUseProgram(m_screen->program());
    glBindVertexArray(quadVAO);
    glBindTexture(GL_TEXTURE_2D, texColorBuffer);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glBindVertexArray(0);

}

Application::Application()
{
	setTitle("Application");

    ImGui::Init(sdlWindow(), context());
    
    m_single = new gl::Shader("./shaders/shader.vert", 
                              "./shaders/shader.frag");
    
    m_double = new gl::Shader("./shaders/shader.vert", 
                              "./shaders/double.frag");
    
    m_screen = new gl::Shader("./shaders/screen.vert", 
                              "./shaders/screen.frag");
    
    m_screen->setInt("screenTexture", 0);
    
    float aspect = width / (float)height;
    m_grid = gl::grid(-aspect, aspect, -1, 1, 2, 2);
    xmin = -2*aspect;
    xmax = 2*aspect;
    
    scene()->camera.setPosition(glm::vec3(0,0,-1));
    scene()->camera.setRotation(glm::vec3(0,0,0));
    scene()->camera.setPerspective(90, width / (float)height, 0.01, 10);
    //scene()->camera.setOrtho(-1,1,1,-1,-1,1);
    
    setupFrameBuffer(width, height);
    setupQuad();
    
    glEnable(GL_DEPTH_TEST);
}

Application::~Application()
{
    delete m_grid;
    delete m_shader;
}

void Application::draw()
{
    glUseProgram(m_shader->program());

	glBindVertexArray(m_grid->VAO);

	glDrawElements(GL_TRIANGLES, m_grid->indices.size(), GL_UNSIGNED_INT, 0);

	glBindVertexArray(0);
}
    
void Application::update(float dt)
{
    static int iters = 100; 
    static glm::vec2 constant(-0.306,0.625);
    static float threshold = 4;
    static int julia = 1;
    static ImVec4 color = ImVec4(114.0f/255.0f, 144.0f/255.0f, 154.0f/255.0f, 255.0f/255.0f);
    glm::vec4 gcolor = {color.x, color.y, color.z, color.w};
    const static float minC1 = -2, maxC1 = 2, minC2 = -2, maxC2 = 2;
    static bool fill = true;
    static float animSpeed = 1;
    static int precision = 1; //double;
    
    auto animate = [&](float &val, int dir, float min, float max){
        if (dir == 1)
            val += 0.001 * animSpeed;
        else
            val -= 0.001 * animSpeed;
        
        if (val > max)
            val = min;
        if (val < min)
            val = max;
        
        return val;
    };
    
    static int animateC1 = 0;
    if (animateC1)
        animate(constant.x, animateC1, minC1, maxC1);
        
    static int animateC2 = 0;
    if (animateC2)
        animate(constant.y, animateC2, minC2, maxC2);
    
    if (precision == 1) {
        m_shader = m_double;
        
        m_shader->setDouble("threshold", (double)threshold);
        m_shader->setDouble("xmin", xmin * scale + trans1);
        m_shader->setDouble("xmax", xmax * scale + trans1);
        m_shader->setDouble("ymin", ymin * scale + trans2);
        m_shader->setDouble("ymax", ymax * scale + trans2);
    } else {
        m_shader = m_single;
        
        m_shader->setFloat("threshold", threshold);
        m_shader->setFloat("xmin", xmin * scale + trans1);
        m_shader->setFloat("xmax", xmax * scale + trans1);
        m_shader->setFloat("ymin", ymin * scale + trans2);
        m_shader->setFloat("ymax", ymax * scale + trans2);
    }
    
    m_shader->setVec2("constant", constant);
    m_shader->setInt("iters", iters);
    m_shader->setInt("jul", julia);
    m_shader->setVec4("color", gcolor);
    
    m_shader->setInt("fill", fill);
    m_shader->setMat4("view", scene()->camera.view());
	m_shader->setMat4("projection", scene()->camera.projection());
    m_shader->setMat4("model", mat4(1.0f));

    //firstPass();
    
    // draw grid with shader
    draw();
    
    //secondPass();

	ImGui::RenderFrame(sdlWindow(), [&](){
        //ImGui::ShowDemoWindow();
        
        if (!ImGui::Begin("Options")) {
			ImGui::End();
			return;
		}
		
		double xm = xmin * scale + trans1;
        double xma = xmin * scale + trans1;
        double ym = ymin * scale + trans2;
        double yma = ymax * scale + trans2;
		
        ImGui::InputDouble("xmin", &xm,0,0,"%.16f");
		ImGui::InputDouble("xmax", &xma,0,0,"%.16f");
        ImGui::InputDouble("ymin", &ym,0,0,"%.16f");
        ImGui::InputDouble("ymax", &yma,0,0,"%.16f");
        
        ImGui::SliderInt("Iterations", &iters, 1, 1000);
        ImGui::SliderFloat("Threshold", &threshold, 1, 100);
        
        ImGui::SliderFloat("C1", &constant.x, minC1, maxC1); ImGui::SameLine();
        if (ImGui::Button("Animate##C1")) {
            animateC1++;
            animateC1 = animateC1 %3;
        } 
        
        ImGui::SliderFloat("C2", &constant.y, minC2, maxC2); ImGui::SameLine();
        if (ImGui::Button("Animate##C2")) {
            animateC2++;
            animateC2 = animateC2 % 3;
        }
        ImGui::SliderFloat("Speed", &animSpeed, 0, 10);
        
        ImGui::ColorEdit3("Color", (float*)&color);
        
        ImGui::RadioButton("Float", &precision, 0); ImGui::SameLine();
        ImGui::RadioButton("Double", &precision, 1);
        
        ImGui::RadioButton("Mandelbrot", &julia, 0); ImGui::SameLine();
        ImGui::RadioButton("Julia", &julia, 1);
        ImGui::Checkbox("Fill Sets", &fill);

		ImGui::End();
	});
}

void Application::processEvent(const SDL_Event &event)
{
	ImGui::ProcessEvent(event);
    
    if (event.type == SDL_MOUSEWHEEL) {        
        scale *= (event.wheel.y < 0) ? scaleFactor : 1.0 / scaleFactor;
    }
    
    if (event.type == SDL_MOUSEMOTION) {
        int state = SDL_GetMouseState(nullptr, nullptr);
        if (state & SDL_BUTTON(SDL_BUTTON_RIGHT)) {
            int x = -event.motion.xrel;
            int y = event.motion.yrel;
            
            glm::vec2 step;
            step.x = (xmax - xmin) * scale * x / (double)width;
            step.y = (ymax - ymin) * scale * y / (double)height;
            
            trans += step;
            
            trans1 += (xmax - xmin) * scale * x / (double)width;
            trans2 += (ymax - ymin) * scale * y / (double)height;
            
        }
        
    }
}

void Application::sizeChanged(int w, int h)
{
    glViewport(0, 0, w, h);
    
    float aspect = w / (float)height;
    scene()->camera.setPerspective(90, aspect, 0.01, 10);
    //scene()->camera.setOrtho(-1,1,1,-1,-1,1);
    
    delete m_grid;
    m_grid = gl::grid(-aspect, aspect, -1, 1, 2, 2);
    
    xmin = -2*aspect;
    xmax = 2*aspect;
    
    cleanupFrameBuffer();
    setupFrameBuffer(w, h);
}
