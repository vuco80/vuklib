#include "Platform.h"
#include <fstream>

namespace vuk {

IGuiListener::IGuiListener(){}
IGuiListener::~IGuiListener(){}

// GuiObject

GuiObject::GuiObject(int id, const SDL_Rect& rect, IGuiListener* listener) :
	m_id(id), m_rect(rect), m_listener(listener), m_highlighted(false) {}

GuiObject::~GuiObject(){}

int GuiObject::getId() const {
	return m_id;
}

const SDL_Rect& GuiObject::getRect() const {
	return m_rect;
}

void GuiObject::onEvent(const SDL_Event* evt) {
	switch(evt->type){
		case SDL_EVENT_MOUSE_BUTTON_DOWN:
		{
			SDL_Point mouse = {(int)evt->button.x, (int)evt->button.y};
			if(SDL_PointInRect(&mouse, &getRect())) {
				//m_listener->onGuiClick(getId(), IGuiListener::GuiEventArgs(IGuiListener::GuiTypes::GuiType_Button, 1));
				m_listener->onGuiClick(getId());
			}
		}
		break;
		
		case SDL_EVENT_MOUSE_MOTION:
		{
			SDL_Point mouse = {(int)evt->motion.x, (int)evt->motion.y};
			if(SDL_PointInRect(&mouse, &getRect())) {
				setHighlighted(true);
			} else setHighlighted(false);
		}
		break;
		
		default:
		break;
	}
}

bool GuiObject::isHighlighted() const {
	return m_highlighted;
}

void GuiObject::setHighlighted(bool value) {
	m_highlighted = value;
}

// GuiButton

GuiButton::GuiButton(int id, const SDL_Rect& rect, const std::string& text, IGuiListener* listener) :
	GuiObject(id, rect, listener), m_text(text), m_texCaption(nullptr) {}
GuiButton::~GuiButton() {
	if(m_texCaption != nullptr) vuk::Platform::instance()->destroyTexture(&m_texCaption);
}

void GuiButton::render(const SDL_Color& primaryColor, const SDL_Color& secondaryColor, const SDL_Color& textColor, int fontId) {
	const SDL_Rect& rect = getRect();
	SDL_FRect externalRect = {(float)rect.x, (float)rect.y, (float)rect.w, (float)rect.h};
	SDL_FRect innerRect = {(float)(rect.x+2), (float)(rect.y+2), (float)(rect.w-4), (float)(rect.h-4)};
		
	if(isHighlighted()) {
		vuk::Platform::instance()->renderRect(true, primaryColor, externalRect);
		vuk::Platform::instance()->renderRect(true, secondaryColor, innerRect);
		if(m_texCaption == nullptr) {
			vuk::Platform::instance()->createTextTexture(fontId, m_text, textColor, rect.w, &m_texCaption, &m_captionWidth, &m_captionHeight);
		}
		SDL_FRect captionDst = {innerRect.x + (innerRect.w / 2) - (m_captionWidth / 2), innerRect.y + (innerRect.h / 2) - (m_captionHeight / 2), (float)m_captionWidth, (float)m_captionHeight};
		vuk::Platform::instance()->renderTexture(m_texCaption, nullptr, &captionDst);
	}
	else {
		vuk::Platform::instance()->renderRect(true, secondaryColor, externalRect);
		vuk::Platform::instance()->renderRect(true, primaryColor, innerRect);
		if(m_texCaption == nullptr) {
			vuk::Platform::instance()->createTextTexture(fontId, m_text, textColor, rect.w, &m_texCaption, &m_captionWidth, &m_captionHeight);
		}
		SDL_FRect captionDst = {innerRect.x + (innerRect.w / 2) - (m_captionWidth / 2), innerRect.y + (innerRect.h / 2) - (m_captionHeight / 2), (float)m_captionWidth, (float)m_captionHeight};
		vuk::Platform::instance()->renderTexture(m_texCaption, nullptr, &captionDst);
	}
}

// GuiLayout

GuiLayout::GuiLayout(int id, IGuiListener* listener, int fontId, 
	const SDL_Color& primaryColor, const SDL_Color& secondaryColor, const SDL_Color& textColor) :
	m_id(id), m_listener(listener), m_fontId(fontId), m_primaryColor(primaryColor), m_secondaryColor(secondaryColor),
	m_textColor(textColor) {}
GuiLayout::~GuiLayout() {}

int GuiLayout::getId() const {
	return m_id;
}

int GuiLayout::loadGuiObjects(const std::string& scriptFile) {
	std::ifstream file;
	file.open(scriptFile.c_str());
	char buffer[256], skip[30], text[256];
	std::string str;
	while(!file.eof()) {
		file >> buffer;
		str.assign(buffer);
		if(str.compare("type:") == 0) {
			int id, x, y, w, h;
			file >> buffer >> skip >> id >> skip >> x >> y >> w >> h >> skip;
			file.getline(text, 256);
			//SDL_Log("trovato nuovo %s con id %d, rect %d,%d,%d,%d text %s", buffer, id,
			//	x, y, w, h, text);
			str.assign(buffer);
			if(str.compare("button") == 0) {
				str.assign(text);
				m_objects.push_back(new GuiButton(id, {x, y, w, h}, str, m_listener));
			}
		}
	}
	file.close();
	SDL_Log("[OK] Loaded %lld gui objects from file %s", m_objects.size(), scriptFile.c_str());
	
	return 0;
}

void GuiLayout::onEvent(const SDL_Event* evt) {
	for(auto i=m_objects.begin(); i!=m_objects.end(); ++i) {
		(*i)->onEvent(evt);
	}
}

void GuiLayout::onUpdate() {
	
}

void GuiLayout::render() {
	for(auto i=m_objects.begin(); i!=m_objects.end(); ++i) {
		(*i)->render(m_primaryColor, m_secondaryColor, m_textColor, m_fontId);
	}
}

// GuiManager

GuiManager* GuiManager::m_instance = nullptr;

GuiManager::GuiManager() : m_currentLayout(-1) {}
GuiManager::~GuiManager() {}

GuiManager* GuiManager::instance() {
	if(m_instance == nullptr) m_instance = new GuiManager();
	return m_instance;
}

void GuiManager::addLayout(GuiLayout* layout) {
	m_layouts.push_back(layout);
}

void GuiManager::setCurrentLayout(int id) {
	int index = 0;
	for(auto i=m_layouts.begin(); i!=m_layouts.end(); ++i) {
		if((*i)->getId() == id) {
			m_currentLayout = index;
		}
		index++;
	}
}

void GuiManager::removeLayout(int id) {
	for(auto i=m_layouts.begin(); i!=m_layouts.end(); ++i) {
		if((*i)->getId() == id) {
			delete (*i);
			m_layouts.erase(i);
			break;
		}
	}
}

void GuiManager::onEvent(const SDL_Event* evt) {
	if(m_currentLayout < 0) return;
	m_layouts[m_currentLayout]->onEvent(evt);
}

void GuiManager::onUpdate() {
	if(m_currentLayout < 0) return;
	m_layouts[m_currentLayout]->onUpdate();
}

void GuiManager::render() {
	if(m_currentLayout < 0) return;
	m_layouts[m_currentLayout]->render();
}

int GuiManager::getActionForObject(GuiManager::GuiEvents evt, int sender) {
	for(auto i=m_actionsTable.begin(); i!=m_actionsTable.end(); ++i) {
		if((*i).event == evt && (*i).id == sender) return (*i).action;
	}
	
	return -1;
}

void GuiManager::loadActionsTable(const std::string& actionScript) {
	std::ifstream file;
	file.open(actionScript.c_str());
	
	char buffer[256], skip[30];
	std::string str;
	while(!file.eof()) {
		int id, action;
		
		file >> skip >> id >> skip >> buffer >> skip >> action;
		SDL_Log("action %s object %d, arg %d", buffer, id, action);
		str.assign(buffer);
		GuiEvents event = GuiEvents::GuiEvent_Undefined;
		if(str.compare("onclick") == 0) event = GuiEvents::GuiEvent_OnClick;
		m_actionsTable.push_back({id, event, action});
	}
	
	file.close();
}

};
