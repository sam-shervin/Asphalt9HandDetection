# Real-Time Hand Gesture Controller for Asphalt

A Python-based control system that uses MediaPipe and native gesture/key-handling libraries to translate hand gestures into in-game actions for **Asphalt 8/9**. It detects gestures like fist closures and hand orientations to simulate keyboard inputs for steering and actions like drifting or nitro.

## Files

All files are in the same folder:
- `main.py`  
- `gesture_utils.cpp`  
- `keyhandler.cpp`  
- **Compiled libraries** (see Build Instructions):
  - Windows: `keyhandler.dll`, `gesture_utils.dll`
  - macOS/Linux: `libkeyhandler.dylib` / `libkeyhandler.so`, `libgesture_utils.dylib` / `libgesture_utils.so`  
- `requirements.txt`

---

## Prerequisites

1. **Python 3.7+**  
2. **C++ compiler**:
   - **Windows**: MinGW-w64 or Visual Studio (MSVC)  
   - **macOS/Linux**: `g++` (with `-fPIC` support on Linux)

3. **Python packages** (see `requirements.txt`):
   ```txt
   opencv-python
   mediapipe
   pyautogui
    ```

---

## Build Instructions

### Windows (MinGW/Visual Studio)

1. Open a terminal (e.g., PowerShell, Command Prompt with MinGW on PATH, or “Developer Cmd Prompt for VS”).
2. Compile **keyhandler**:

   ```sh
   # Using MinGW:
   g++ -std=c++17 -shared -o keyhandler.dll keyhandler.cpp -Wl,--out-implib,libkeyhandler.a
   ```

   or, with MSVC (Developer Cmd Prompt):

   ```bat
   cl /LD keyhandler.cpp /Fe:keyhandler.dll
   ```
3. Compile **gesture\_utils**:

   ```sh
   # Using MinGW:
   g++ -std=c++17 -shared -o gesture_utils.dll gesture_utils.cpp -Wl,--out-implib,libgesture_utils.a
   ```

   or, with MSVC:

   ```bat
   cl /LD gesture_utils.cpp /Fe:gesture_utils.dll
   ```
4. Ensure `keyhandler.dll` and `gesture_utils.dll` are in the same folder as `main.py`.

---

### macOS / Linux

1. Open a terminal.
2. Install Xcode Command Line Tools (macOS) or ensure `g++` is installed (Linux).
3. Compile **keyhandler**:

   * **macOS**:

     ```sh
     g++ -dynamiclib -o libkeyhandler.dylib keyhandler.cpp \
         -framework CoreFoundation -framework CoreGraphics
     ```
   * **Linux**:

     ```sh
     g++ -std=c++17 -fPIC -shared -o libkeyhandler.so keyhandler.cpp -ldl -lpthread
     ```
4. Compile **gesture\_utils**:

   * **macOS**:

     ```sh
     g++ -std=c++17 -shared -o libgesture_utils.dylib gesture_utils.cpp
     ```
   * **Linux**:

     ```sh
     g++ -std=c++17 -fPIC -shared -o libgesture_utils.so gesture_utils.cpp
     ```
5. Rename or symlink libraries so that `main.py` can find them:

   * **macOS**:

     ```sh
     mv libkeyhandler.dylib keyhandler.dll
     mv libgesture_utils.dylib gesture_utils.dll
     ```
   * **Linux**:

     ```sh
     mv libkeyhandler.so keyhandler.dll
     mv libgesture_utils.so gesture_utils.dll
     ```
6. Place `keyhandler.dll` and `gesture_utils.dll` next to `main.py`.

---

## Python Setup

1. Create a virtual environment (optional but recommended):

   ```sh
   python3 -m venv venv
   source venv/bin/activate   # macOS/Linux
   venv\Scripts\activate      # Windows
   ```
2. Install dependencies:

   ```sh
   pip install -r requirements.txt
   ```

---

## Usage

1. Confirm your webcam is connected.
2. Run the script:

   ```sh
   python main.py
   ```
3. **Controls detected**:

   * **Right hand closed (fist)** → press Space
   * **Left hand closed (fist)** → press Down arrow
   * **Both hands**:

     * Measure slope between wrist points.
     * Positive slope → hold Right arrow; negative slope → hold Left arrow.
4. **Exit**: When the cursor (index fingertip) of any hand moves into the **top-left** corner of the frame (within 10% width and height), the script automatically terminates.

---

## Notes

* Adjust `FRAME_W`, `FRAME_H`, `TURN_COOLDOWN` and `FPS` in `main.py` as needed.
* On Linux, you might need to install `libgl1-mesa-glx` or equivalent for OpenCV video capture.
* Tested on macOS Sequoia 15.4.1


