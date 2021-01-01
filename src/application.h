#ifndef APPLICATION_H
#define APPLICATION_H

#include "opengl/opengl.h"

class Application : public gl::OpenGLWindow {
	public:
		Application();
		~Application();

	protected:
		void update(float dt) override;
		void processEvent(const SDL_Event &event) override;
        void sizeChanged(int w, int h) override;
        
        gl::Mesh *m_grid;
        gl::Shader *m_shader;
        
        void draw();
        void updateCamera();
};

#endif
