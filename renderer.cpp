#include "renderer.h"

#ifdef FSGD_END_USE_D3D
#include "d3d11_renderer_impl.h"
#endif

namespace end
{
	renderer::renderer(native_handle_type window_handle)
	{
		p_impl = new impl(window_handle,default_view); //create and initialize the implementation
	}

	renderer::~renderer() 
	{
		// Clean up implementation
		delete p_impl;
	}

	void renderer::draw()
	{
		// draw views...
		p_impl->draw_view(default_view);
		// draw views...
		// draw views...
	}
}