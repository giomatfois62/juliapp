#include "application.h"
#include "imgui/myimgui.h"
#include "opengl/geometry.h"

using namespace gl;
using namespace glm;

static float xmin = -2, xmax = 2, ymin = -2, ymax = 2;
float scale = 1;
const float scaleFactor = 1.15;
glm::vec2 trans;

Application::Application()
{
	setTitle("Application");

    ImGui::Init(sdlWindow(), context());

    m_shader = new gl::Shader("./shaders/shader.vert", 
                              "./shaders/shader.frag");
    
    float aspect = width / (float)height;
    m_grid = gl::grid(-aspect, aspect, -1, 1, 2, 2);
    xmin = -2*aspect;
    xmax = 2*aspect;
    
    scene()->camera.setPosition(glm::vec3(0,0,-1));
    scene()->camera.setRotation(glm::vec3(0,0,0));
    
    scene()->camera.setPerspective(90, width / (float)height, 0.01, 10);
    //scene()->camera.setOrtho(-1,1,1,-1,-1,1);
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
    
    static int animateC1 = 0;
    if (animateC1) {
        if (animateC1 == 1)
            constant.x += 0.001 * animSpeed;
        else
            constant.x -= 0.001 * animSpeed;
        
        if (constant.x > maxC1)
            constant.x = minC1;
        if (constant.x < minC1)
            constant.x = maxC1;
    }
        
    static int animateC2 = 0;
    if (animateC2) {
        if (animateC2 == 1)
            constant.y += 0.001 * animSpeed;
        else
            constant.y -= 0.001 * animSpeed;
        
        if (constant.y > maxC2)
            constant.y = minC2;
        if (constant.y < minC2)
            constant.y = maxC2;
    }
    
    m_shader->setVec2("constant", constant);
    m_shader->setInt("iters", iters);
    m_shader->setFloat("threshold", threshold);
    m_shader->setInt("jul", julia);
    m_shader->setVec4("color", gcolor);
    m_shader->setFloat("xmin", xmin * scale + trans.x);
    m_shader->setFloat("xmax", xmax * scale + trans.x);
    m_shader->setFloat("ymin", ymin * scale + trans.y);
    m_shader->setFloat("ymax", ymax * scale + trans.y);
    m_shader->setInt("fill", fill);
    m_shader->setMat4("view", scene()->camera.view());
	m_shader->setMat4("projection", scene()->camera.projection());
    m_shader->setMat4("model", mat4(1.0f));
    
    // draw grid with shader
    draw();
    
	ImGui::RenderFrame(sdlWindow(), [&](){
        if (!ImGui::Begin("Options")) {
			ImGui::End();
			return;
		}
		
        ImGui::InputFloat("xmin", &xmin);
		ImGui::InputFloat("xmax", &xmax);
        ImGui::InputFloat("ymin", &ymin);
        ImGui::InputFloat("ymax", &ymax);
        
        ImGui::SliderInt("Iterations", &iters, 1, 500);
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
        scale *= (event.wheel.y < 0) ? scaleFactor : 1.0f / scaleFactor;
    }
    
    if (event.type == SDL_MOUSEMOTION) {
        int state = SDL_GetMouseState(nullptr, nullptr);
        if (state & SDL_BUTTON(SDL_BUTTON_RIGHT)) {
            int x = -event.motion.xrel;
            int y = event.motion.yrel;
            
            glm::vec2 step;
            step.x = (xmax - xmin) * scale * x / (float)width;
            step.y = (ymax - ymin) * scale * y / (float)height;
            
            trans += step;
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
}
