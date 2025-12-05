#ifndef VUK_PLATFORM_H
#define VUK_PLATFORM_H

#include <SDL3/SDL.h>
#include <SDL3_image/SDL_image.h>
#include <SDL3_ttf/SDL_ttf.h>
#include <map>
#include <string>
#include <vector>

namespace vuk {
	
class IGuiListener {
public:
	IGuiListener();
	~IGuiListener();
	
	virtual void onGuiClick(int sender) = 0;
};

class GuiObject {
public:
	GuiObject(int id, const SDL_Rect& rect, IGuiListener* listener);
	virtual ~GuiObject();
	
	int getId() const;
	const SDL_Rect& getRect() const;
	void onEvent(const SDL_Event* evt);
	virtual void render(const SDL_Color& primaryColor, const SDL_Color& secondaryColor, const SDL_Color& textColor, int fontId) = 0;
	bool isHighlighted() const;
	
private:
	void setHighlighted(bool value);
	
private:
	int m_id;
	SDL_Rect m_rect;
	IGuiListener* m_listener;
	bool m_highlighted;
};

class GuiButton : public GuiObject {
public:
	GuiButton(int id, const SDL_Rect& rect, const std::string& text, IGuiListener* listener);
	~GuiButton();
	void render(const SDL_Color& primaryColor, const SDL_Color& secondaryColor, const SDL_Color& textColor, int fontId) override;
private:
	std::string m_text;
	SDL_Texture* m_texCaption;
	int m_captionWidth, m_captionHeight;
};

class GuiLayout {
public:
	GuiLayout(int id, IGuiListener* listener, int fontId, 
		const SDL_Color& primaryColor, const SDL_Color& secondaryColor, const SDL_Color& textColor);
	~GuiLayout();
	
	int getId() const;
	int loadGuiObjects(const std::string& scriptFile);
	
	void onEvent(const SDL_Event* evt);
	void onUpdate();
	void render();
	
private:
	int m_id;
	IGuiListener* m_listener;
	int m_fontId;
	SDL_Color m_primaryColor, m_secondaryColor, m_textColor;
	std::vector<GuiObject*> m_objects;
};

class GuiManager {
public:
	~GuiManager();
	static GuiManager* instance();
	
	void loadActionsTable(const std::string& actionScript);
	void addLayout(GuiLayout* layout);
	void setCurrentLayout(int id);
	void removeLayout(int id);
	
	void onEvent(const SDL_Event* evt);
	void onUpdate();
	void render();
	
	enum GuiEvents {
		GuiEvent_Undefined,
		GuiEvent_OnClick
	};
	
	int getActionForObject(GuiEvents evt, int sender);
	
private:
	GuiManager();
	
	struct ActionTableRow {
		int id;
		GuiEvents event;
		int action;
	};
	
	static GuiManager* m_instance;
	std::vector<GuiLayout*> m_layouts;
	int m_currentLayout;
	std::vector<ActionTableRow> m_actionsTable;
};
	
class IFrameTrackerListener {
public:
	IFrameTrackerListener();
	~IFrameTrackerListener();
	
	virtual void onFrameTrackerTick(int frameTrackerId, int tickCount) = 0;
};

class FrameTracker {
public:
	FrameTracker(int id, int frequence, IFrameTrackerListener* listener);
	~FrameTracker();
	
	void update();
	int getId() const;
private:
	int m_id;
	IFrameTrackerListener* m_listener;
	int m_tickFrequence;
	int m_frameCount;
	int m_tickCount;
};

//---
	
class IPlatformEventListener {
public:
	IPlatformEventListener();
	virtual ~IPlatformEventListener();
	
	virtual void OnEvent(const SDL_Event* evt) = 0;
};

class IPlatformFrameListener {
public:
	IPlatformFrameListener();
	virtual ~IPlatformFrameListener();
	
	virtual void OnFrameUpdateStep() = 0;
	virtual void OnFrameRender(unsigned int elapsedMs) = 0;
};

class Platform {
public:
	~Platform();
	
	static Platform* instance();
	
	int createAndShowWindow(const std::string& caption, int width, int height, int logicWidth, int logicHeight);
	void quit();
	int resizeWindow(int width, int height);
	int goFullscreen(int* width, int* height);
	
	void pollEvents(IPlatformEventListener* listener);
	
	int loadTexture(const std::string& filename, SDL_Texture** dst, int* width, int* height);
	int createTexture(SDL_Texture** dst, int width, int height);
	void destroyTexture(SDL_Texture** texture);
	
	int setRenderTarget(SDL_Texture* target);
	void renderClear();
	void renderTexture(SDL_Texture* texture, SDL_FRect* src, SDL_FRect* dst);
	void renderRect(bool fill, const SDL_Color& color, const SDL_FRect& rect);
	void enableAlphaBlending();
	
	int loadFont(const std::string& filename, int dimension, int fontId);
	int createTextTexture(int fontId, const std::string& text, const SDL_Color& color, int maxWidth, SDL_Texture** dst, int* width, int* height);
	
	void initFrameTimer(int updateStepMs);
	void executeFrame(IPlatformFrameListener* listener);
	
	void addFrameTracker(int id, int frequence, IFrameTrackerListener* listener);
	void clearFrameTrackers();
	void removeFrameTracker(int id);
	
private:
	Platform();
	
	static Platform* m_instance;
	SDL_Window* m_window;
	SDL_Renderer* m_renderer;
	std::map<int, TTF_Font*> m_fonts;
	Uint32 m_updateStepMs;
	Uint32 m_frameTime;
	Uint32 m_frameTimeAccumulator;
	Uint32 m_frameTimeRenderAccumulator;
	std::vector<FrameTracker> m_frameTrackers;
};

};

#endif
