
#include "build/build_config.h"

#if defined(OS_LINUX)

#include "extension_wpe/uinput_manager.h"
#include "extension_wpe/wpe_key_converter.h"

#include <linux/input.h>
#include <linux/uinput.h>
#include <fcntl.h>
#include <stdio.h>
#include <errno.h>

static int lookup_code(int keysym, bool *isShiftRequired);

UInputManager* UInputManager::_instance = NULL;

UInputManager* UInputManager::getInstance() {
    if (NULL == _instance)
        _instance = new UInputManager();

    return _instance;
}

UInputManager::UInputManager()
    : _deviceDescriptor(0),
      _isReady(false) {
    _logger = new Logger();
}

UInputManager::~UInputManager() {
    delete _logger;
    ioctl(_deviceDescriptor, UI_DEV_DESTROY);   // try destroy device
}

bool UInputManager::registerUinputDevice() {
    struct uinput_user_dev uidev;

    printf("This is %d from %s in %s\n",__LINE__,__func__,__FILE__);
    _deviceDescriptor = open("/dev/uinput", O_WRONLY | O_NONBLOCK | O_CREAT | O_NDELAY, S_IREAD | S_IWRITE);

    if (0 > _deviceDescriptor) {
        printf("Can't open uinput device\n");
        _logger->Log(kWarningLogLevel, "Can't open uinput device");
        return false;
    }

    // enable Key and Synchronization events
    int ret = ioctl(_deviceDescriptor, UI_SET_EVBIT, EV_KEY);
    if (0 > ret) {
        _logger->Log(kWarningLogLevel, "Can't register uinput key events");
        return false;
    }
    printf("This is %d from %s in %s\n",__LINE__,__func__,__FILE__);
    ret = ioctl(_deviceDescriptor, UI_SET_EVBIT, EV_SYN);
    if (0 > ret) {
        _logger->Log(kWarningLogLevel, "Can't register uinput synchronization events");
        return false;
    }

    // initialize device info
    memset(&uidev, 0, sizeof(uidev));
    uidev.id.bustype = BUS_USB;
    uidev.id.version = 0x01;
    snprintf(uidev.name, UINPUT_MAX_NAME_SIZE, "wd_key_input");
    ret = write(_deviceDescriptor, &uidev, sizeof(uidev));

    if (0 > ret) {
        _logger->Log(kWarningLogLevel, "Can not initialize user input device");
        return false;
    }
    registerHandledKeys();

    ret = ioctl(_deviceDescriptor, UI_DEV_CREATE); // create device
    if (0 > ret) {
        _logger->Log(kWarningLogLevel, "Can not create user input device");
        return false;
    }

    _isReady = true;
    return true;
}

int UInputManager::injectKeyEvent(void* event) {
    int res = -1;
    int keyCode = 0;
    bool isShiftRequired = false;
    struct input_event ev;

    memset(&ev, 0, sizeof(ev));

    gettimeofday(&(ev.time), NULL);

    KeyEvent *keyEvent = (KeyEvent *) event;
    printf("#### Key text: %s, modifiers: %d\n", keyEvent->text().c_str(), (int)keyEvent->modifiers());
    int keyText = keyEvent->text().c_str()[0];

     printf("This is %d from %s in %s\n",__LINE__,__func__,__FILE__);
    // Check keyCode for capital letters
    if ((keyText>='>' && keyText<='Z') ||       // '>','?','@'  included
            (keyText>='!' && keyText<='&') ||   // '!' - '&'
            (keyText>='(' && keyText<='+') ||    // '(' - '+'
            (keyText>='^' && keyText<='_') ||    // '^','_'
            (keyText>='{' && keyText<='}') ||    // '{' - '}'
            '<' == keyText ) {
        ev.type = EV_KEY;
        ev.code = KEY_RIGHTSHIFT;
        res = write(_deviceDescriptor, &ev, sizeof(ev));
    }

    printf("%s:%s:%d #### Key code: %x\n", __FILE__, __func__, __LINE__, keyEvent->key());

    ev.type = EV_KEY;
    ev.value = keyEvent->type();
    keyCode = lookup_code(keyEvent->key(), &isShiftRequired);
    if (isShiftRequired) {
        ev.code = KEY_LEFTSHIFT;
        res = write(_deviceDescriptor, &ev, sizeof(ev));
        printf("%s:%s:%d #### ev.code: %x type = %x\n", __FILE__, __func__, __LINE__, ev.code, ev.value);
    }
    ev.code = keyCode;
    res = write(_deviceDescriptor, &ev, sizeof(ev));

    printf("%s:%s:%d #### ev.code: %x type = %x\n", __FILE__, __func__, __LINE__, ev.code, ev.value);
    if (ev.value == KeyEvent::KeyRelease) {
        ev.type = EV_SYN;
        ev.code = SYN_REPORT;
        ev.value = 0;
        res = write(_deviceDescriptor, &ev, sizeof(ev));
    }

    return res;
}

void UInputManager::registerHandledKeys() {
    // set range of keys
    for (int i=0; i<256; i++) {
        ioctl(_deviceDescriptor, UI_SET_KEYBIT, i);
    }
    ioctl(_deviceDescriptor, UI_SET_KEYBIT, 0xE000U);   // POWER
    ioctl(_deviceDescriptor, UI_SET_KEYBIT, 0xEF00U);   // MENU
    ioctl(_deviceDescriptor, UI_SET_KEYBIT, 0xE002U);   // BACK
    ioctl(_deviceDescriptor, UI_SET_KEYBIT, 0xE100U);   // UP
    ioctl(_deviceDescriptor, UI_SET_KEYBIT, 0xE101U);   // DOWN
    ioctl(_deviceDescriptor, UI_SET_KEYBIT, 0xE102U);   // LEFT
    ioctl(_deviceDescriptor, UI_SET_KEYBIT, 0xE103U);   // RIGHT
    ioctl(_deviceDescriptor, UI_SET_KEYBIT, 0xE001U);   // OK
    ioctl(_deviceDescriptor, UI_SET_KEYBIT, 0xE00EU);   // INFO
    ioctl(_deviceDescriptor, UI_SET_KEYBIT, 0xE00FU);   // TEXT
    ioctl(_deviceDescriptor, UI_SET_KEYBIT, 0xE403U);   // RECOERD
    ioctl(_deviceDescriptor, UI_SET_KEYBIT, 0xE402U);   // STOP
    ioctl(_deviceDescriptor, UI_SET_KEYBIT, 0xE301U);   // ONE
    ioctl(_deviceDescriptor, UI_SET_KEYBIT, 0xE302U);   // TWO
    ioctl(_deviceDescriptor, UI_SET_KEYBIT, 0xE303U);   // THREE
    ioctl(_deviceDescriptor, UI_SET_KEYBIT, 0xE304U);   // FOUR
    ioctl(_deviceDescriptor, UI_SET_KEYBIT, 0xE305U);   // FIVE
    ioctl(_deviceDescriptor, UI_SET_KEYBIT, 0xE306U);   // SIX
    ioctl(_deviceDescriptor, UI_SET_KEYBIT, 0xE307U);   // SEVEN
    ioctl(_deviceDescriptor, UI_SET_KEYBIT, 0xE308U);   // EIGHT
    ioctl(_deviceDescriptor, UI_SET_KEYBIT, 0xE309U);   // NINE
    ioctl(_deviceDescriptor, UI_SET_KEYBIT, 0xE300U);   // ZERO
    ioctl(_deviceDescriptor, UI_SET_KEYBIT, 0xEE01U);   // COMPANION_DEVICE_KEY_LIVE_SWIPE
    ioctl(_deviceDescriptor, UI_SET_KEYBIT, 0xEE02U);   // COMPANION_DEVICE_KEY_VOD_SWIPE
    ioctl(_deviceDescriptor, UI_SET_KEYBIT, 0xEE03U);   // COMPANION_DEVICE_KEY_PAD_UP
    ioctl(_deviceDescriptor, UI_SET_KEYBIT, 0xEE04U);   // COMPANION_DEVICE_KEY_PAD_DOWN
    ioctl(_deviceDescriptor, UI_SET_KEYBIT, 0xEE05U);   // COMPANION_DEVICE_KEY_PAD_LEFT
    ioctl(_deviceDescriptor, UI_SET_KEYBIT, 0xEE06U);   // COMPANION_DEVICE_KEY_PAD_RIGHT
}

bool UInputManager::isReady() {
    return _isReady;
}

int UInputManager::injectSynEvent() {
    struct input_event ev;

    memset(&ev, 0, sizeof(ev));

    ev.type = EV_SYN;
    ev.code = SYN_REPORT;
    ev.value = 0;
    gettimeofday(&(ev.time), NULL);

    int res = write(_deviceDescriptor, &ev, sizeof(ev));

    return res;
}

static int lookup_code(int keysym, bool *isShiftRequired) {

    if (keysym & WD_NORMAL_KEY_IDENTIFIER) {
        keysym = (keysym & WD_NORMAL_KEY_MASK);
    }
    printf("%s:%s:%d keysym = %x \n",__FILE__, __func__, __LINE__, keysym);

    if (((0x41 <= keysym) && (0x5A >= keysym)) ||  /* Identify Uppercase Letters */
        ((0x21 <= keysym) && (0x29 >= keysym)) ||  /* Identify Specical Characters */
        ((0x7B <= keysym) && (0x7E >= keysym)) ||
         (0x2A == keysym) || (0x2B == keysym) ||
         (0x3A == keysym) || (0x3C == keysym) ||
         (0x3E == keysym) || (0x3F == keysym) ||
         (0x5E == keysym) || (0x5F == keysym) ||
         (0x40 == keysym)) {
        *isShiftRequired = true;
    }

    switch (keysym) {
        /* Special Characters with shift*/
        case 0x21: return KEY_1; // ! = 1 + shiftkey
        case 0x22: return KEY_APOSTROPHE; // " = 2 + shiftkey
        case 0x23: return KEY_3; // # = 3 + shiftkey
        case 0x24: return KEY_4; // $ = 4 + shiftkey
        case 0x25: return KEY_5; // % = 5 + shiftkey
        case 0x26: return KEY_7; // & = 7 + shiftkey
        case 0x27: return KEY_GRAVE; // ~ = ` + shiftkey
        case 0x28: return KEY_9; // ( = 9 + shiftkey
        case 0x29: return KEY_0; // ) = 0 + shiftkey
        case 0x2A: return KEY_8; //* = 8 + shiftkey
        case 0x2B: return KEY_EQUAL; //+ = = + shiftkey

        case 0x3A: return KEY_SEMICOLON; //: = ; + shiftkey
        case 0x3C: return KEY_COMMA; //< = , + shiftkey
        case 0x3E: return KEY_DOT; //> = . + shiftkey
        case 0x3F: return KEY_SLASH; //? = / + shiftkey

        case 0x5E: return KEY_6; // ^ = 6 + shiftkey
        case 0x5F: return KEY_MINUS; // _ = - + shiftkey

        case 0x7B: return KEY_LEFTBRACE; // { = [ +shiftkey
        case 0x7C: return KEY_BACKSLASH; // | = \ + shiftkey
        case 0x7D: return KEY_RIGHTBRACE; // } = ] + shiftkey
        case 0x7E: return KEY_GRAVE; // ~ = ` + shiiftkey

        case 0x40: return KEY_2; //@ = 2 + shiftkey
        /*Special characters without shift*/
        case 0x2C: return KEY_COMMA; //,
        case 0x2D: return KEY_MINUS; //-
        case 0x2E: return KEY_DOT; //.
        case 0x2F: return KEY_SLASH; /* / */
        case 0x3B: return KEY_SEMICOLON; // ;
        case 0x3D: return KEY_EQUAL; // =
        case 0x5B: return KEY_LEFTBRACE; // [
        case 0x5C: return KEY_BACKSLASH; /* \ */
        case 0x5D: return KEY_RIGHTBRACE; // ]

        /*Key code mapping for space key*/
        case 0x20:  return KEY_SPACE;
        /* KeyCode mapping for numerals */
        case 0x30:  return KEY_0;
        case 0x31:  return KEY_1;
        case 0x32:  return KEY_2;
        case 0x33:  return KEY_3;
        case 0x34:  return KEY_4;
        case 0x35:  return KEY_5;
        case 0x36:  return KEY_6;
        case 0x37:  return KEY_7;
        case 0x38:  return KEY_8;
        case 0x39:  return KEY_9;

        /* KeyCode mapping for alphabets */
        case 0x41:
        case 0x61:  return KEY_A;
        case 0x42:
        case 0x62:  return KEY_B;
        case 0x43:
        case 0x63:  return KEY_C;
        case 0x44:
        case 0x64:  return KEY_D;
        case 0x45:
        case 0x65:  return KEY_E;
        case 0x46:
        case 0x66:  return KEY_F;
        case 0x47:
        case 0x67:  return KEY_G;
        case 0x48:
        case 0x68:  return KEY_H;
        case 0x49:
        case 0x69:  return KEY_I;
        case 0x4A:
        case 0x6A:  return KEY_J;
        case 0x4B:
        case 0x6B:  return KEY_K;
        case 0x4C:
        case 0x6C:  return KEY_L;
        case 0x4D:
        case 0x6D:  return KEY_M;
        case 0x4E:
        case 0x6E:  return KEY_N;
        case 0x4F:
        case 0x6F:  return KEY_O;
        case 0x50:
        case 0x70:  return KEY_P;
        case 0x51:
        case 0x71:  return KEY_Q;
        case 0x52:
        case 0x72:  return KEY_R;
        case 0x53:
        case 0x73:  return KEY_S;
        case 0x54:
        case 0x74:  return KEY_T;
        case 0x55:
        case 0x75:  return KEY_U;
        case 0x56:
        case 0x76:  return KEY_V;
        case 0x57:
        case 0x77:  return KEY_W;
        case 0x58:
        case 0x78:  return KEY_X;
        case 0x59:
        case 0x79:  return KEY_Y;
        case 0x5A:
        case 0x7A:  return KEY_Z;
        default: return keysym;
     }
}

#endif // OS_LINUX
