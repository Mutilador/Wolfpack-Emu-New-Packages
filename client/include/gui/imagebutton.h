#if !defined(__IMAGEBUTTON_H__)
#define __IMAGEBUTTON_H__

#include "control.h"
#include "gui.h"
#include "enums.h"

typedef void (*fnButtonClickedCallback)(cControl *button);

class cImageButton : public cControl {
private:
	// There are a lot of different possibilities for the states
	cTexture *gumps[3]; // For every state one
	unsigned short gumpids[3]; // For every state one
	unsigned short gumphues[3]; // One hue per state
	bool partialhue[3]; // One partial hue value per state
	//unsigned char hittest[4]; // One hit-test mask for every state - Faster
	bool mouseOver_; // False by default (is the mouse over this control)
	bool mouseHolding_; // False by default (is the space key being pressed)
	bool spaceHolding_; // False by default (is the space key being pressed)
	unsigned int pressRepeatRate_; // Number of miliseconds a press will be regarded as an onClick event.
	SDL_TimerID pressRepeatTimer_; // Timer id of the current press repeat timer

	// We need to store some sort of callback here.
	fnButtonClickedCallback callback;

	inline enButtonStates getState() {
		if (!mouseHolding_ && !spaceHolding_) {
			if ((mouseOver_ || Gui->inputFocus() == this) && gumps[BS_HOVER]) {
				return BS_HOVER;
			} else {
				return BS_UNPRESSED;
			}
		} else {
			if (mouseOver_ || spaceHolding_) {
				return BS_PRESSED;
			} else {
				if (gumps[BS_HOVER]) {
					return BS_HOVER;
				} else {
					return BS_UNPRESSED;
				}
			}
		}
	}
public:
	cImageButton(int x, int y, unsigned short up, unsigned short down);
	virtual ~cImageButton();

	inline void setCallback(fnButtonClickedCallback callback) {
		this->callback = callback;
	}

	void setStateGump(enButtonStates state, unsigned short id, unsigned short hue = 0, bool partialHue = false);

	inline bool mouseOver() const { return mouseOver_; } // Returns true if the mouse is over this button
	inline bool mouseHolding() const { return mouseHolding_; } // Returns true if the mouse button is hold
	inline bool spaceHolding() const { return spaceHolding_; } // Returns true if the space key is being held
	
	// Press Repeat Rate
	inline unsigned int pressRepeatRate() const { return pressRepeatRate_; }
	inline SDL_TimerID pressRepeatTimer() const { return pressRepeatTimer_; }
	inline void setPressRepeatTimer(SDL_TimerID data) { pressRepeatTimer_ = data; }

	inline void setPressRepeatRate(unsigned int data) {
		pressRepeatRate_ = data;
	}

	void update();
	void draw(int xoffset, int yoffset);

	void onMouseLeave(); // Toggle the mouseOver state
	void onMouseEnter(); // Toggle the mouseOver state
	void onMouseDown(int x, int y, unsigned char button, bool pressed);
	void onMouseUp(int x, int y, unsigned char button, bool pressed);
	void onKeyDown(const SDL_keysym &key);
	void onKeyUp(const SDL_keysym &key);
	void onBlur(cControl *newFocus);

	virtual void onClick(); // The button has been clicked	

	cControl *getControl(int x, int y);
};

#endif
