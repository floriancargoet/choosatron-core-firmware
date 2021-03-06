#include "cdam_keypad.h"
#include "cdam_constants.h"

// The plan: events and state, can always check the state of a given
// button, or the value of the current pressed keys, also get events
// to be informed of button down, up, and all keys up.

namespace cdam
{

#define BTN_ONE   0x01
#define BTN_TWO   0x02
#define BTN_THREE 0x04
#define BTN_FOUR  0x08

/* Public Methods */

Keypad::Keypad() {}

void Keypad::initialize(uint8_t aPinBtnOne, uint8_t aPinBtnTwo,
                        uint8_t aPinBtnThree, uint8_t aPinBtnFour) {

	pinMode(aPinBtnOne, INPUT_PULLUP);
	pinMode(aPinBtnTwo, INPUT_PULLUP);
	pinMode(aPinBtnThree, INPUT_PULLUP);
	pinMode(aPinBtnFour, INPUT_PULLUP);

	ButtonData btnOne   = { aPinBtnOne, 1, 0, BTN_IDLE_STATE, BTN_NO_EVENT, 0, 0, 0 };
	ButtonData btnTwo   = { aPinBtnTwo, 2, 0, BTN_IDLE_STATE, BTN_NO_EVENT, 0, 0, 0 };
	ButtonData btnThree = { aPinBtnThree, 3, 0, BTN_IDLE_STATE, BTN_NO_EVENT, 0, 0, 0 };
	ButtonData btnFour  = { aPinBtnFour, 4, 0, BTN_IDLE_STATE, BTN_NO_EVENT, 0, 0, 0 };

	this->buttonData[0] = btnOne;
	this->buttonData[1] = btnTwo;
	this->buttonData[2] = btnThree;
	this->buttonData[3] = btnFour;

	this->state = KEYPAD_IDLE_STATE;
	this->buttons = 0;
	this->keypadValue = 0;
	this->lastValue = 0;
	this->lastButtons = 0;
	this->multiCount = 0;
	this->multiUp = 0;
	this->multiUpValue = 0;
	this->clearEvents();
	this->active = true;
}

void Keypad::updateState() {
	if (this->active) {
		getFilteredButtons();
	}

    /*for (int i = NUM_BUTTONS - 1; i >= 0; i--) { // notice reverse count order, to accomodate left shift
        ButtonEvent event = buttonData[i].event;
        switch (event) {
            case BTN_DOWN_EVENT:
                Serial.print("Btn Down Event: ");
                Serial.println(buttonData[i].num);
                break;
            case BTN_HELD_EVENT:
                Serial.print("Btn Held Event: ");
                Serial.println(buttonData[i].num);
                break;
            case BTN_UP_EVENT:
                Serial.print("Btn Up Event: ");
                Serial.println(buttonData[i].num);
                break;
            default:
                break;
        }
    }

    switch (this->lastEvent) {
        case KEYPAD_MULTI_UP_EVENT:
            Serial.print("Multi-Up Event: ");
            printBinary(this->multiUpEvent);
            Serial.print("Value: ");
            Serial.println(this->lastValue);
            break;
        case KEYPAD_MULTI_DOWN_EVENT:
            Serial.print("Multi-Down Event: ");
            printBinary(this->multiDownEvent);
            Serial.print("Value: ");
            Serial.println(this->lastValue);
            break;
        default:
            break;
    }*/
}

//
// detect button events
//

bool Keypad::buttonsDown() {
	if (this->lastButtons) {
		return true;
	}
	return false;
}

bool Keypad::buttonDown(uint8_t aBtnNum) {
	//DEBUG("Button %d: %d", aBtnNum, buttonData[aBtnNum - 1].value);
	return buttonData[aBtnNum - 1].value ? true : false;
}

uint8_t Keypad::buttonEvent(ButtonEvent aEvent, uint8_t aRange) {
	uint8_t result = 0;
	for (int i = NUM_BUTTONS - 1; i >= 0; i--) {
		if (this->buttonData[i].event == aEvent) {
			if (aRange == 0) {
				result = this->buttonData[i].num;
			} else if ((0 < buttonData[i].num) && (buttonData[i].num <= aRange)) {
				result = this->buttonData[i].num;
			}
			clearEvents();
		}
	}
	return result;
}

uint8_t Keypad::buttonEventValue(ButtonEvent aEvent, uint8_t aBtnNum) {
	uint8_t result = 0;
	if (buttonData[aBtnNum - 1].event == aEvent) {
		this->buttonData[aBtnNum - 1].event = BTN_NO_EVENT;
		result = buttonData[aBtnNum - 1].num;
	}
	//clearEvents();
	return result;
}

/*char Keypad::buttonEventChar(ButtonEvent aEvent, uint8_t aBtnNum) {
    uint8_t value = buttonEventValue(aEvent, aBtnNum);
    if (value != 0) {
        return value + '0';
    }
    return '\0';
}*/

uint8_t Keypad::buttonValue(ButtonState aState, uint8_t aBtnNum) {
	uint8_t result = 0;
	if (buttonData[aBtnNum - 1].state == aState) {
		result = buttonData[aBtnNum - 1].num;
	}
	return result;
}

/*char Keypad::buttonChar(uint8_t aBtnNum) {
char result = buttonData[aBtnNum].num + '0';
return result;
}*/

uint8_t Keypad::keypadEvent(KeypadEvent aEvent, uint8_t aRange) {
	uint8_t result = 0;
	if (this->event == aEvent) {
		if (aRange == 0) {
			result = this->lastValue;
		} else if ((0 < this->lastValue) && (this->lastValue <= aRange)) {
			result = this->lastValue;
		}
		clearEvents();
	}
	return result;
}

uint8_t Keypad::keypadEventValue(KeypadEvent aEvent) {
	uint8_t result = 0;
	if (this->event == aEvent) {
    //this->event = KEYPAD_NO_EVENT;
		result = this->lastValue;
		clearEvents();
	}
	return result;
}

/*char Keypad::keypadEventChar(KeypadEvent aEvent) {
    uint8_t value = keypadEventValue(aEvent);
    if (value != 0) {
        return value + '0';
    }
    return '\0';
}*/

void Keypad::clearEvents() {
	for (int i = NUM_BUTTONS - 1; i >= 0; i--) {
		this->buttonData[i].event = BTN_NO_EVENT;
	}
	this->lastEvent = this->event;
	this->event = KEYPAD_NO_EVENT;
	this->pressedEvents = 0;
	this->releasedEvents = 0;
	this->heldEvents = 0;
	this->multiDownEvent = 0;
	this->multiUpEvent = 0;

	this->state = KEYPAD_IDLE_STATE;
	this->buttons = 0;
	this->keypadValue = 0;
	this->lastValue = 0;
	this->lastButtons = 0;
	this->multiCount = 0;
	this->multiUp = 0;
	this->multiUpValue = 0;
}

void Keypad::setButtonEvent(ButtonEvent aEvent, uint8_t aBtnNum) {
	this->buttonData[aBtnNum - 1].event = aEvent;
	this->buttonData[aBtnNum - 1].active = 1;
	this->buttons |= (buttonData[aBtnNum - 1].active << (aBtnNum - 1));
	this->lastValue += this->buttonData[aBtnNum - 1].num;
}

void Keypad::setKeypadEvent(KeypadEvent aEvent, uint8_t aValue) {
	this->event = aEvent;
	this->lastValue = aValue;
}

/* Private Methods */

ButtonEvent Keypad::filterButton(ButtonData *aBtnData) {
	uint8_t val = !digitalRead(aBtnData->pin);
	aBtnData->event = BTN_NO_EVENT;
	switch (aBtnData->state)
	{
		case BTN_IDLE_STATE:
			if (!aBtnData->active && (val != 0)) {
				// Filter button down.
				aBtnData->count = BTN_FILTER_DOWN_COUNT;
				aBtnData->state = BTN_FILTER_DOWN_STATE;
			} else if (aBtnData->active && (val == 0)) {
				// Filter button up.
				aBtnData->count = BTN_FILTER_UP_COUNT;
				aBtnData->state = BTN_FILTER_UP_STATE;
			}
			break;
		case BTN_FILTER_DOWN_STATE:
			if (val == 1 && (!aBtnData->active)) {
				if ((aBtnData->count != 0) && (--aBtnData->count == 0)) {
					aBtnData->active = val;
					aBtnData->event = BTN_DOWN_EVENT;
					aBtnData->count = BTN_HOLD_COUNT;
					aBtnData->state = BTN_FILTER_HOLD_STATE; // Button is down.
				}
			} else {
				aBtnData->count = BTN_FILTER_UP_COUNT;
				aBtnData->state = BTN_FILTER_UP_STATE;
			}
			break;
		case BTN_FILTER_HOLD_STATE:
			if (val == 1 && (aBtnData->active)) {
				if ((aBtnData->count != 0) && (--aBtnData->count == 0)) {
					aBtnData->event = BTN_HELD_EVENT;
					aBtnData->state = BTN_IDLE_STATE;
				}
			} else {
				// Filter button up.
				aBtnData->count = BTN_FILTER_UP_COUNT;
				aBtnData->state = BTN_FILTER_UP_STATE;
			}
			break;
		case BTN_FILTER_UP_STATE:
			if (val == 0 && (aBtnData->active)) {
				if ((aBtnData->count != 0) && (--aBtnData->count == 0)) {
					aBtnData->active = val;
					aBtnData->heldFlag = 0;
					aBtnData->event = BTN_UP_EVENT;
					aBtnData->state = BTN_IDLE_STATE; // Button let up.
				}
			} else {
				aBtnData->count = BTN_FILTER_DOWN_COUNT;
				aBtnData->state = BTN_FILTER_DOWN_STATE;
			}
			break;
		default:
			aBtnData->state = BTN_IDLE_STATE;
			break;
	}
	aBtnData->value = val;
	return aBtnData->event;
}

/*void Keypad::printBinary(uint8_t aValue) {
    Serial.print("0x");
    Serial.print(IsBitSet(aValue, 7) ? '1' : '0');
    Serial.print(IsBitSet(aValue, 6) ? '1' : '0');
    Serial.print(IsBitSet(aValue, 5) ? '1' : '0');
    Serial.print(IsBitSet(aValue, 4) ? '1' : '0');
    Serial.print(IsBitSet(aValue, 3) ? '1' : '0');
    Serial.print(IsBitSet(aValue, 2) ? '1' : '0');
    Serial.print(IsBitSet(aValue, 1) ? '1' : '0');
    Serial.println(IsBitSet(aValue, 0) ? '1' : '0');
}*/

uint8_t Keypad::keypadTotal(uint8_t aButtons) {
	uint8_t total = 0;
	for (int i = NUM_BUTTONS - 1; i >= 0; i--) {
		if (IsBitSet(aButtons, i)) {
			total += buttonData[i].num;
		}
	}
	return total;
}

void Keypad::getFilteredButtons(void) {
    //KeypadEvent keyEvent = KEYPAD_NO_EVENT;
    this->keypadValue = 0;

    for (int i = NUM_BUTTONS - 1; i >= 0; i--) { // notice reverse count order, to accomodate left shift
        ButtonEvent event = filterButton(&buttonData[i]);
        if (buttonData[i].active) {
            //this->buttons = (this->buttons << 1) | buttonData[i].active;
            this->buttons |= (buttonData[i].active << i);
            this->keypadValue += buttonData[i].num;
        }
        switch (event) {
            case BTN_DOWN_EVENT:
                this->pressedEvents |= (1 << i);
                break;
            case BTN_HELD_EVENT:
                this->heldEvents |= (1 << i);
                break;
            case BTN_UP_EVENT:
                this->pressedEvents &= ~(1 << i);
                this->heldEvents &= ~(1 << i);
                this->releasedEvents |= (1 << i);
                break;
            default:
                break;
        }
    }

    switch (this->state)
    {
        case KEYPAD_IDLE_STATE:
            if ((this->lastButtons == 0) && (this->buttons > 0)) {
                // Filter multi-down.
                this->multiCount = KEYPAD_FILTER_DOWN_COUNT;
                this->state = KEYPAD_FILTER_MULTI_DOWN_STATE;
            } else if ((this->buttons | this->lastButtons) == this->lastButtons) {
                if (this->lastButtons != 0) {
                    // Filter multi-up.
                    this->multiUp = this->lastButtons;
                    this->multiUpValue = keypadTotal(this->lastButtons);
                    this->multiCount = KEYPAD_FILTER_UP_COUNT;
                    this->state = KEYPAD_FILTER_MULTI_UP_STATE;
                }
            }
            break;
        case KEYPAD_FILTER_MULTI_DOWN_STATE:
            // We want to make sure no buttons have gone up, only down.
            // The AND reveals any down buttons lost to break the filter.
            if ((this->buttons & this->lastButtons) == this->lastButtons) {
                uint8_t newBtns = this->buttons & ~this->lastButtons;
                if (newBtns > 0) {
                    this->multiCount = KEYPAD_FILTER_DOWN_COUNT;
                } else if ((this->multiCount != 0) && (--this->multiCount == 0)) {
                    this->multiDownEvent = this->buttons;
                    this->lastValue = this->keypadValue;
                    this->event = KEYPAD_MULTI_DOWN_EVENT;
                    this->state = KEYPAD_IDLE_STATE; // Button is down.
                }
            } else {
                this->multiUp = this->lastButtons;
                this->multiUpValue = keypadTotal(this->lastButtons);
                this->multiCount = KEYPAD_FILTER_UP_COUNT;
                this->state = KEYPAD_FILTER_MULTI_UP_STATE;
            }
            break;
        case KEYPAD_FILTER_MULTI_UP_STATE:
            // We want to make sure new buttons haven't gone down, only up.
            // The OR reveals new down buttons to break the filter.
            if ((this->buttons | this->lastButtons) == this->lastButtons) {
                uint8_t newUpBtns = this->buttons ^ this->lastButtons;
                if (newUpBtns > 0) {
                    this->multiCount = KEYPAD_FILTER_UP_COUNT;
                } else if ((this->multiCount != 0) && (--this->multiCount == 0)) {
                    if (this->buttons > 0) {
                        this->multiUp = this->buttons;
                        this->multiUpValue = this->keypadValue;
                        this->multiCount = KEYPAD_FILTER_UP_COUNT;
                    } else {
                        this->multiUpEvent = this->multiUp;
                        this->multiUp = 0;
                        this->keypadValue = this->multiUpValue;
                        this->lastValue = this->keypadValue;
                        this->event = KEYPAD_MULTI_UP_EVENT;
                        this->state = KEYPAD_IDLE_STATE;
                    }
                }
            } else {
                this->multiUp = 0;
                this->multiUpValue = 0;
                this->multiCount = KEYPAD_FILTER_DOWN_COUNT;
                this->state = KEYPAD_FILTER_MULTI_DOWN_STATE;
            }
            break;
        default:
            this->state = KEYPAD_IDLE_STATE;
            break;
    }

    this->lastButtons = this->buttons;
    this->buttons = 0;
    this->lastEvent = this->event;
}

}