#include "Platform.h"

namespace vuk {

IPlatformEventListener::IPlatformEventListener(){}
IPlatformEventListener::~IPlatformEventListener(){}

IPlatformFrameListener::IPlatformFrameListener(){}
IPlatformFrameListener::~IPlatformFrameListener(){}

Platform* Platform::m_instance = nullptr;

Platform::Platform() : m_window(nullptr), m_renderer(nullptr), m_updateStepMs(16) {
	SDL_Init(SDL_INIT_VIDEO);
	TTF_Init();
}
Platform::~Platform(){}

Platform* Platform::instance() {
	if(m_instance == nullptr) m_instance = new Platform();
	return m_instance;
}
	
int Platform::createAndShowWindow(const std::string& caption, int width, int height) {
	m_window = SDL_CreateWindow(caption.c_str(), width, height, 0);
	if(m_window == nullptr) return 1;
	
	m_renderer = SDL_CreateRenderer(m_window, 0);
	if(m_renderer == nullptr) return 2;
	
	return 0;
}

void Platform::quit() {
	if(m_renderer != nullptr) SDL_DestroyRenderer(m_renderer);
	if(m_window != nullptr) SDL_DestroyWindow(m_window);
}

int Platform::resizeWindow(int width, int height) {
	if(m_window == nullptr) return 1;
	
	if(SDL_SetWindowSize(m_window, width, height)) {
		return 0;
	}
	
	return 2;
}

int Platform::goFullscreen(int* width, int* height) {
	if(m_window == nullptr) return 1;
	
	if(SDL_SetWindowFullscreenMode(m_window, nullptr)) {
		SDL_GetWindowSize(m_window, width, height);
		return 0;
	}
	
	return 2;
}

void Platform::pollEvents(IPlatformEventListener* listener) {
	SDL_Event evt;
	while(SDL_PollEvent(&evt)) {
		listener->OnEvent(&evt);
	}
}

int Platform::loadTexture(const std::string& filename, SDL_Texture** dst, int* width, int* height) {
	SDL_Surface* s = IMG_Load(filename.c_str());
	if(s == 0) return 1;
	
	*dst = SDL_CreateTextureFromSurface(m_renderer, s);
	if(*dst == 0) {
		SDL_DestroySurface(s);
		return 2;
	}
	*width = s->w;
	*height = s->h;
	SDL_DestroySurface(s);
	
	return 0;
}

int Platform::createTexture(SDL_Texture** dst, int width, int height) {
	*dst = SDL_CreateTexture(m_renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET, 
		width, height);
	if(*dst == nullptr) return 1;
	return 0;
}

void Platform::destroyTexture(SDL_Texture** texture) {
	SDL_DestroyTexture(*texture);
	*texture = nullptr;
}

int Platform::setRenderTarget(SDL_Texture* target) {
	if(SDL_SetRenderTarget(m_renderer, target)) 
		return 0;
	return 1;
}

void Platform::renderTexture(SDL_Texture* texture, SDL_FRect* src, SDL_FRect* dst) {
	SDL_RenderTexture(m_renderer, texture, src, dst);
}

int Platform::loadFont(const std::string& filename, int dimension, int fontId) {
	if(m_fonts.find(fontId) != m_fonts.end()) return 1;
	
	TTF_Font* font = TTF_OpenFont(filename.c_str(), dimension);
	
	if(font == nullptr) return 2;
	
	m_fonts[fontId] = font;
	
	return 0;
}

int Platform::createTextTexture(int fontId, const std::string& text, const SDL_Color& color, int maxWidth, SDL_Texture** dst, int* width, int* height) {
	if(m_fonts.find(fontId) == m_fonts.end()) return 1;
	
	TTF_Font* font = m_fonts[fontId];
	SDL_Surface* s = TTF_RenderText_Blended_Wrapped(font, text.c_str(), text.length(), color, maxWidth);
	
	if(s == nullptr) return 2;
	
	*dst = SDL_CreateTextureFromSurface(m_renderer, s);
	if(*dst == nullptr) return 3;
			
	*width = s->w;
	*height = s->h;
	
	SDL_DestroySurface(s);
	
	return 0;
}

void Platform::initFrameTimer(int updateStepMs) {
	m_frameTime = SDL_GetTicks();
	m_frameTimeAccumulator = 0;
	m_updateStepMs = updateStepMs;
}

void Platform::executeFrame(IPlatformFrameListener* listener) {
	Uint32 now = SDL_GetTicks();
	Uint32 elapsed = now - m_frameTime;
	m_frameTime = now;
	m_frameTimeAccumulator += elapsed;
	
	while(m_frameTimeAccumulator >= m_updateStepMs) {
		m_frameTimeAccumulator -= m_updateStepMs;
		listener->OnFrameUpdateStep();
	}
	
	SDL_RenderClear(m_renderer);
	listener->OnFrameRender(elapsed);
	SDL_RenderPresent(m_renderer);
}

};