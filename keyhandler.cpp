// keyhandler_optimized.cpp
// Optimized macOS/Windows key handler with constexpr mapping, real-time event queue, and high-priority threads.

#include <thread>
#include <atomic>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <vector>
#include <cstdint>

#ifdef _WIN32
  #include <windows.h>
  #include <winuser.h>
#else
  #include <ApplicationServices/ApplicationServices.h>
  #include <dispatch/dispatch.h>
  #include <dispatch/queue.h>
#endif

// Ring buffer for events
struct KeyEvent { bool down; uint16_t keycode; };

class EventQueue {
public:
    EventQueue(size_t capacity = 128)
      : _buffer(capacity), _head(0), _tail(0), _count(0) {}

    bool push(const KeyEvent &evt) {
        std::unique_lock<std::mutex> lk(_mtx);
        if (_count == _buffer.size()) return false;
        _buffer[_tail] = evt;
        _tail = (_tail + 1) % _buffer.size();
        ++_count;
        _cv.notify_one();
        return true;
    }

    KeyEvent pop() {
        std::unique_lock<std::mutex> lk(_mtx);
        _cv.wait(lk, [this]{ return _count > 0; });
        auto evt = _buffer[_head];
        _head = (_head + 1) % _buffer.size();
        --_count;
        return evt;
    }

private:
    std::vector<KeyEvent> _buffer;
    size_t _head, _tail, _count;
    std::mutex _mtx;
    std::condition_variable _cv;
};

static EventQueue g_queue;

#ifdef _WIN32
constexpr WORD mapKeyWin(const char k0) {
    switch(k0) {
        case 's': return VK_SPACE;
        case 'l': return VK_LEFT;
        case 'r': return VK_RIGHT;
        case 'u': return VK_UP;
        case 'd': return VK_DOWN;
        default:  return 0;
    }
}
#else
constexpr CGKeyCode mapKeyMac(const char k0) {
    switch(k0) {
        case 's': return 49;   // space
        case 'l': return 123;  // left
        case 'r': return 124;  // right
        case 'u': return 126;  // up
        case 'd': return 125;  // down
        default:  return UINT16_MAX;
    }
}

// GCD queue for posting CGEvents
static dispatch_queue_t cgQueue;

struct PostData {
    CGKeyCode keycode;
    bool down;
};

void postEvent(void *context) {
    PostData *data = (PostData*)context;
    CGEventRef e = CGEventCreateKeyboardEvent(NULL, data->keycode, data->down);
    CGEventPost(kCGHIDEventTap, e);
    CFRelease(e);
    delete data;
}
#endif

// Worker thread: send native events
static void eventWorker() {
#ifdef _WIN32
    SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_HIGHEST);
#endif
    while (true) {
        KeyEvent evt = g_queue.pop();
#ifdef _WIN32
        INPUT in = {0};
        in.type = INPUT_KEYBOARD;
        in.ki.wVk    = 0;
        in.ki.wScan  = MapVirtualKey(evt.keycode, MAPVK_VK_TO_VSC);
        in.ki.dwFlags = evt.down ? KEYEVENTF_SCANCODE : (KEYEVENTF_SCANCODE | KEYEVENTF_KEYUP);
        SendInput(1, &in, sizeof(in));
#else
        PostData *data = new PostData{ evt.keycode, evt.down };
        dispatch_async_f(cgQueue, data, postEvent);
#endif
    }
}

extern "C" void initKeyHandler() {
#ifndef _WIN32
    cgQueue = dispatch_queue_create("com.example.keyhandler", 
        dispatch_queue_attr_make_with_qos_class(DISPATCH_QUEUE_SERIAL, QOS_CLASS_USER_INTERACTIVE, 0));
#endif
    std::thread(eventWorker).detach();
}

extern "C" void pressKeyDown(const char* key) {
    if (!key || !key[0]) return;
    uint16_t code;
#ifdef _WIN32
    code = mapKeyWin(key[0]);
#else
    code = mapKeyMac(key[0]);
#endif
    if (!code || code == UINT16_MAX) return;
    g_queue.push({true, code});
}

extern "C" void pressKeyUp(const char* key) {
    if (!key || !key[0]) return;
    uint16_t code;
#ifdef _WIN32
    code = mapKeyWin(key[0]);
#else
    code = mapKeyMac(key[0]);
#endif
    if (!code || code == UINT16_MAX) return;
    g_queue.push({false, code});
}

// Automatically initialize on load
struct _Initializer { _Initializer(){ initKeyHandler(); } } _init;
