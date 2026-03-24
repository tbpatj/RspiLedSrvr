# RspiLedSrvr — Project Summary

This document describes the classes, functions, data models, and HTTP API endpoints that make up **RspiLedSrvr**. It is intended to give enough detail for someone to recreate the project from scratch in a different programming language.

---

## Table of Contents

1. [Project Overview](#1-project-overview)
2. [Repository Layout](#2-repository-layout)
3. [Global State](#3-global-state)
4. [Main Loop](#4-main-loop)
5. [Core Classes](#5-core-classes)
   - [Animation](#51-animation)
   - [AnimationObject](#52-animationobject)
   - [TransitionObject](#53-transitionobject)
   - [LedDeviceMapping](#54-leddevicemapping)
   - [LedDeviceSettings](#55-leddevicesettings)
   - [Device (abstract)](#56-device-abstract-base-class)
   - [AddressableLedDevice](#57-addressableleddevice)
   - [NonAddressableLedDevice](#58-nonaddressableleddevice)
   - [LedController (abstract)](#59-ledcontroller-abstract-base-class)
   - [WS2811xController](#510-ws2811xcontroller)
   - [NonAddressableController](#511-nonaddressablecontroller)
   - [FrameProcessor](#512-frameprocessor)
   - [CaptureDevice](#513-capturedevice)
   - [Preset](#514-preset)
   - [Sleeper](#515-sleeper)
   - [SleepController](#516-sleepcontroller)
   - [FileManage](#517-filemanage)
6. [Utility Functions](#6-utility-functions)
7. [Persistence Functions](#7-persistence-functions)
8. [HTTP Server & REST API Endpoints](#8-http-server--rest-api-endpoints)
   - [Response Format](#81-standard-response-format)
   - [Device Endpoints](#82-device-endpoints)
   - [Preset Endpoints](#83-preset-endpoints)
   - [Animation / Mode Endpoints](#84-animation--mode-endpoints)
   - [TV / Capture-Device Endpoints](#85-tv--capture-device-endpoints)
   - [Debug Endpoints](#86-debug-endpoints)
   - [Static Client Files](#87-static-client-files)
9. [JSON Data Schemas](#9-json-data-schemas)
10. [Dependencies & Libraries](#10-dependencies--libraries)

---

## 1. Project Overview

RspiLedSrvr is a Raspberry Pi C++ application that:

* Drives **addressable** (WS281x) and **non-addressable** (analog RGB) LED strips via GPIO.
* Captures a live webcam / capture-card feed and computes per-edge average colours so LEDs around a TV screen can mirror what is on the TV ("ambilight" effect).
* Serves a React front-end and exposes a REST API (port **3001**) so the front-end (or any HTTP client) can manage devices, presets, animations and TV settings at run-time.
* Persists device and preset configuration as JSON files on disk.
* Runs at ~60 FPS in its main loop.

---

## 2. Repository Layout

```
main.cpp                        – entry point, global declarations, main loop
src/
  animations/
    animation.cpp               – Animation class
    init-animations.cpp         – scan ./resources/animations/ at startup
  device/
    device.cpp                  – Device abstract base class
    animationObject.cpp         – AnimationObject class
    transitionObject.cpp        – TransitionObject class
    captureDevice/
      capture-device.cpp        – CaptureDevice class
      frame-processor.cpp       – FrameProcessor class
    ledDevice/
      ledDevices.cpp            – includes everything below
      addressableLedDevice.cpp  – AddressableLedDevice class
      ledMapping.cpp            – LedDeviceMapping struct
      settings.cpp              – LedDeviceSettings class
      ledControllers/
        ledController.cpp       – LedController abstract base class
        ledControllers.cpp      – includes all controllers
        ws2811xController.cpp   – WS2811xController class
      nonAddressableLed/
        nonAddressableController.cpp    – NonAddressableController class
        nonAddressableLedDevice.cpp     – NonAddressableLedDevice class
  presets/
    preset.cpp                  – Preset class
    ledPresets/ledPreset.cpp    – (reserved, currently empty)
  sleeping/
    sleeper.cpp                 – Sleeper class
    sleepController.cpp         – SleepController class
  files/
    file-manage.cpp             – FileManage class
    load-save.cpp               – load/save helper functions
  server/
    server.cpp                  – RunLedServer(), wires up all endpoint groups
    serve-client.cpp            – serves React build from ./build/
    responses/server-responses.cpp  – CreateResponse() helpers
    device-endpoints/           – CRUD endpoints for devices
    preset-endpoints/           – CRUD endpoints for presets
    animation-endpoints/        – list / upload / delete animations
    capture-device-endpoints/   – capture-mapping query
    tv-endpoints/               – TV settings get/save/aspect-ratio
    debug-endpoints/            – debug image / info
    util-endpoints/             – (placeholder)
  utils/
    math-util.cpp               – min/max helpers
    basic-presets.cpp           – getSleepPreset()
    base-64.cpp                 – base64 encoder, Mat→base64
    cv-tools.cpp                – getDifferenceImage()
resources/
  animations/                   – animation image files (PNG, etc.)
  devices.json                  – persisted device list
  presets.json                  – persisted preset list
  tv_settings.json              – persisted TV / capture settings
  json.hpp                      – nlohmann/json single-header
  httplib.h                     – cpp-httplib single-header
build/                          – compiled React front-end (external repo)
```

---

## 3. Global State

The following variables are declared at the top of `main.cpp` and are shared across all source files via `#include` chaining.

| Variable | Type | Purpose |
|---|---|---|
| `show_processed_image` | `bool` | Debug: show frame-processor output in an OpenCV window |
| `show_webcam_feed` | `bool` | Debug: show raw webcam frame |
| `show_LEDS` | `bool` | Debug: show LED colours in a 1-pixel-high OpenCV window |
| `show_animation` | `bool` | Debug: show current animation frame |
| `write_frame_proccessor_data` | `bool` | Debug: log frame-processor internals to stdout (note: variable name contains a typo in source) |
| `using_webcam` | `bool` | Set to `true` each frame a device requests TV input; reset to `false` after all devices update |
| `tv_sleeping` | `bool` | `true` when the capture input has had no change for `sleepTime` ms |
| `tv_no_signal` | `bool` | `true` when the capture card shows its "no signal" test pattern |
| `gpio_initialised` | `bool` | `true` when pigpio initialised successfully |
| `captureDevice` | `CaptureDevice` | Single global capture device (webcam / capture card) |
| `animations` | `vector<Animation>` | All loaded animation images |
| `presets` | `vector<Preset>` | All saved presets |
| `devices` | `vector<unique_ptr<Device>>` | All active LED devices |
| `running` | `int` | Main loop runs while `== 1` |
| `sleepController` | `SleepController` | Watches TV state and sleeps/wakes devices automatically |
| `svr` | `httplib::Server` | The HTTP server instance |

---

## 4. Main Loop

The application starts two threads:

1. **Server thread** – calls `RunLedServer()`, which registers all HTTP endpoints and then blocks on `svr.listen("0.0.0.0", 3001)`.
2. **Main thread** – runs a `while(running == 1)` loop targeting 60 FPS:

```
loop every ~16 ms:
    if captureDevice.isCapturing and using_webcam:
        captureDevice.updateFrame()   // grab new frame from webcam
        captureDevice.processFrame()  // run FrameProcessor
    sleepController.update()          // check TV signal, sleep/wake devices
    using_webcam = false              // reset flag
    for each device:
        device.update()               // compute and send LED colours
```

On program exit, devices and presets are saved to disk.

---

## 5. Core Classes

### 5.1 Animation

**File:** `src/animations/animation.cpp`

Stores metadata for a single animation file and lazily loads the image on first use.

| Member | Type | Description |
|---|---|---|
| `name` | `string` | Filename stem (e.g. `"rainbow"`) |
| `extension` | `string` | File extension (e.g. `".png"`) |
| `animation` | `cv::Mat` | Loaded image (rows = frames, cols = pixels/LEDs) |
| `animationLoaded` | `bool` | Whether the image has been read from disk |

**Key methods:**

| Method | Signature | Description |
|---|---|---|
| Constructor | `Animation(string name, string extension)` | Stores metadata; does **not** load image yet |
| `loadAnimation` | `void loadAnimation()` | Reads image from `./resources/animations/<name><ext>` |
| `getAnimation` | `cv::Mat getAnimation()` | Returns the image, calling `loadAnimation` if needed |
| `getName` | `string getName()` | Returns the animation name |
| `getExtension` | `string getExtension()` | Returns the file extension |

**Animation image format:** Each **row** is one frame; each **column** is one pixel/LED colour at that frame. Animations loop when the last row is reached.

---

### 5.2 AnimationObject

**File:** `src/device/animationObject.cpp`

Manages the playback state of a single animation for one device. Handles timing, frame advancement, and pixel interpolation between frames.

| Member | Type | Description |
|---|---|---|
| `animIndx` | `int` | Index of the current row (frame) in the animation image |
| `aStart` | `chrono::milliseconds` | Timestamp when the current frame started |
| `animT` | `float` | 0–1 progress through the current frame duration |
| `animImage` | `cv::Mat` | The active animation image |
| `cCols` | `int` | Column count of the current animation image |

**Key methods:**

| Method | Description |
|---|---|
| `setAnimImage(cv::Mat)` | Replaces the animation image (used for TV mode) |
| `setAnimIndx(int)` | Forces the current frame index |
| `getCCols()` | Returns column count |
| `getCurFrame()` | Returns an interpolated 1-row `cv::Mat` representing the current frame blend |
| `updateTiming(long long animSpeed)` | Advances frame timing; increments `animIndx` when a frame duration expires |
| `resetTiming(int mode)` | Loads a new animation from `animations[mode]` and resets timing |
| `interpolateFrames(curFrame, nextFrame, float fraction)` | Linearly blends two frames using `cv::addWeighted` |
| `colorShift(cv::Mat&, int shift)` | Shifts the hue of the animation image in HSV space |

---

### 5.3 TransitionObject

**File:** `src/device/transitionObject.cpp`

Provides a 0→1 progress value over a configurable time period. Used to smoothly interpolate LED colours when a preset or mode changes.

| Member | Type | Description |
|---|---|---|
| `tStart` | `chrono::milliseconds` | Timestamp when the transition started |
| `t` | `float` | Current progress (0.0 = start, 1.0 = done) |
| `transitionSpeed` | `long long` | Duration of the transition in milliseconds (default 1000) |

**Key methods:**

| Method | Description |
|---|---|
| `resetTiming()` | Restarts the transition from 0 |
| `updateTiming()` | Recomputes `t` based on elapsed time; clamps to 1.0 |
| `getPercT()` | Returns current progress (0–1, or > 1 when held) |
| `setPercT(float)` | Manually overrides the progress value |
| `setData(json)` | Reads `transition_speed` from JSON |
| `getTransitionSpeed()` | Returns the configured duration in ms |

---

### 5.4 LedDeviceMapping

**File:** `src/device/ledDevice/ledMapping.cpp`

Plain data structure describing how a range of LED indices maps to a range of pixel columns in an animation image.

| Field | Type | Description |
|---|---|---|
| `ledSIndx` | `int` | Start index in the LED strip |
| `ledEIndx` | `int` | End index in the LED strip |
| `mapSIndx` | `int` | Start column in the animation/capture image |
| `mapEIndx` | `int` | End column in the animation/capture image |

Direction: if `ledSIndx > ledEIndx` the strip segment is traversed in reverse.

---

### 5.5 LedDeviceSettings

**File:** `src/device/ledDevice/settings.cpp`

Holds all run-time settings for a single LED device.

| Field | Type | Description |
|---|---|---|
| `name` | `string` | Settings/preset name |
| `deviceName` | `string` | Name of the owning device |
| `mode` | `int` | `-1` = TV mode, `0`+ = index into global `animations`, `-1000` = uninitialised |
| `power` | `bool` | `true` = LEDs on |
| `deviceType` | `int` | `0` = non-addressable, `1` = addressable |
| `animSpeed` | `long long` | Milliseconds per animation frame |
| `mappings` | `vector<LedDeviceMapping>` | Ordered list of LED-to-image mappings |

**Key methods:**

| Method | Description |
|---|---|
| `setData(json)` | Parses and applies all settings fields from JSON |
| `setMappings(json)` | Parses the `mapping` array from JSON |
| `getMappings()` | Serialises `mappings` to a JSON array |
| `getJson()` | Returns full JSON representation |
| `getModeFromStr(string)` | Converts `"tv"` → `-1`, animation name → index |
| `getStrFromMode()` | Converts mode index back to string |

---

### 5.6 Device (abstract base class)

**File:** `src/device/device.cpp`

Base class for all LED devices. All devices share a common interface.

| Protected field | Type | Description |
|---|---|---|
| `name` | `string` | Unique device identifier |
| `preset` | `string` | Name of the currently applied preset |
| `type` | `int` | `0` = non-addressable, `1` = addressable |

**Pure virtual / key methods every subclass must implement:**

| Method | Description |
|---|---|
| `update()` | Called once per frame; computes and sends colours to hardware |
| `getJson()` | Serialises the full device state to JSON |
| `setData(json)` | Applies new configuration from JSON |

**Non-pure virtual methods (overridable):**

| Method | Description |
|---|---|
| `setPreset(string)` / `getPreset()` | Get/set the current preset name |
| `setName(string)` / `getName()` | Get/set the device name |
| `showMapping(int index)` | Briefly illuminates LEDs belonging to mapping `index` |
| `showLength()` | Briefly illuminates all LEDs to visualise total length |
| `startDebugOutput()` / `stopDebugOutput()` | Capture debug frames to a `cv::Mat` |
| `usingTV()` | Returns `1` if this device is currently in TV mode |
| `getIntType()` / `setType(int)` / `setType(string)` / `getStrType()` | Type accessors |

---

### 5.7 AddressableLedDevice

**File:** `src/device/ledDevice/addressableLedDevice.cpp`

Inherits `Device`. Controls a WS281x (or compatible) addressable LED strip. Each LED is individually addressable.

**Additional private fields:**

| Field | Type | Description |
|---|---|---|
| `settings` | `LedDeviceSettings` | Current settings/mode/mappings |
| `pinOut` | `int` | GPIO pin number |
| `ledCount` | `int` | Number of LEDs in the strip |
| `ledController` | `LedController*` | Pointer to hardware abstraction layer |
| `leds` | `vector<int>` | Current colour for each LED (packed 0x00RRGGBB) |
| `leds2` | `vector<int>` | Previous colour snapshot used as transition start |
| `t` | `TransitionObject` | Main transition for preset/mode changes |
| `a` | `AnimationObject` | Animation playback state |
| `editorT` | `TransitionObject` | Short-lived transition used during mapping/length preview |
| `editIndex` | `int` | `-2` = show-length mode, `-1` = normal/TV, `0`+ = show specific mapping |
| `refresh_rate` | `int` | ms between forced hardware refreshes when power is off (default 10 000) |

**Key methods:**

| Method | Description |
|---|---|
| `update()` | Main per-frame logic: handles edit preview, TV mode, animation, transitions, then calls `ledController->render()` |
| `updateTiming()` | Advances `t` and `a` timing objects |
| `resetTransition()` | Snapshots current LED colours into `leds2` and resets `t` |
| `setLEDCount(int)` | Resizes `leds` and `leds2` vectors |
| `updateSettingsToPreset(string)` | Finds the matching `Preset` and applies its settings |
| `setData(json)` | Full update from JSON; handles preset lookup and transition reset |
| `updateFromImageAnimation()` | Reads the current animation frame, maps pixel columns to LED ranges (with interpolation if needed), and calls `updateLED` for each LED |
| `updateLED(int index, int color)` | Sets one LED colour with transition interpolation applied; calls `ledController->setLEDColor` |
| `interpolate(...)` | Overloaded: linearly interpolates between two packed-int colours or separate R/G/B values |
| `startDebugOutput()` / `stopDebugOutput()` | Capture 120 frames of LED output as a `cv::Mat` image |

---

### 5.8 NonAddressableLedDevice

**File:** `src/device/ledDevice/nonAddressableLed/nonAddressableLedDevice.cpp`

Inherits `Device`. Controls an analog RGB LED strip driven by PWM on three GPIO pins. The entire strip is one colour at a time.

**Additional private fields:**

| Field | Type | Description |
|---|---|---|
| `settings` | `LedDeviceSettings` | Current settings/mode/mappings |
| `nac` | `NonAddressableController*` | Hardware abstraction for PWM output |
| `rgb` | `cv::Vec3i` | Current R/G/B colour |
| `rgb2` | `cv::Vec3i` | Previous colour snapshot for transitions |
| `t` | `TransitionObject` | Transition object |
| `a` | `AnimationObject` | Animation playback state |

**Key methods:**

| Method | Description |
|---|---|
| `update()` | Per-frame logic: TV mode or animation mode, calls `nac->render()` |
| `updateRGB(r, g, b)` | Sets the strip colour with transition interpolation |
| `updateFromImageAnimation()` | Averages pixel colours across all mapped columns to get a single R/G/B value for the strip |
| `resetTransition()` | Saves `rgb` → `rgb2`, resets `t` |
| `setData(json)` | Applies new configuration including pin assignments |
| `getJson()` | Serialises device state |

---

### 5.9 LedController (abstract base class)

**File:** `src/device/ledDevice/ledControllers/ledController.cpp`

Interface that all hardware LED drivers must implement.

| Method | Description |
|---|---|
| `initLed(int pinOut, int ledCount)` | Initialise the hardware (returns success) |
| `setLEDColor(int index, int color)` | Set one LED to packed-int colour |
| `setLEDColor(int index, int r, int g, int b)` | Set one LED to R/G/B values |
| `render()` | Push buffered colours to hardware |

---

### 5.10 WS2811xController

**File:** `src/device/ledDevice/ledControllers/ws2811xController.cpp`

Concrete `LedController` for WS2811/WS2812B strips using the `rpi_ws281x` library. In the `main` branch the actual library calls are commented out (stub). The `functioning-leds` branch contains the real implementation.

---

### 5.11 NonAddressableController

**File:** `src/device/ledDevice/nonAddressableLed/nonAddressableController.cpp`

Controls an analog RGB strip via three PWM GPIO pins (R, G, B). Uses `pigpio` (`gpioPWM`) when `gpio_initialised` is true; otherwise the calls are commented-out stubs.

| Member | Type | Description |
|---|---|---|
| `pins` | `vector<int>` | GPIO pin numbers [R, G, B] |
| `color` | `vector<int>` | Current PWM duty cycle [R, G, B] |

**Key methods:**

| Method | Description |
|---|---|
| `setPins(vector<int>)` | Sets GPIO pin numbers |
| `setPins(json)` | Reads `r_pin`, `g_pin`, `b_pin` from JSON |
| `setColor(int r, int g, int b)` | Buffers colour; drives GPIO PWM if initialised |
| `render()` | (no-op in base; can be overridden) |
| `getPinsJson()` | Serialises pin assignments to JSON |

---

### 5.12 FrameProcessor

**File:** `src/device/captureDevice/frame-processor.cpp`

Processes a single video frame into a 1-pixel-high image where each pixel is the average colour of a rectangular region on one edge of the frame. This "edge strip" image is what LED mappings reference.

**Configuration fields:**

| Field | Type | Description |
|---|---|---|
| `iterationsX` | `int` | Number of sample points along top and bottom edges |
| `iterationsY` | `int` | Number of sample points along left and right edges |
| `paddingX/Y` | `int` | Half-width/height of each sampling rectangle |
| `boundOffsetX/Y` | `int` | Crops this many pixels from the frame edges (for letterboxing) |
| `bezelScaledX/Y` | `double` | Fractional inset to account for physical TV bezel |
| `deadzoneDecrease` | `float` | Maximum brightness reduction for dark pixels |
| `deadzoneThreshold` | `int` | V-channel value below which brightness reduction is applied |
| `deadzonePower` | `float` | Curve exponent for the deadzone brightness reduction |

**Key methods:**

| Method | Description |
|---|---|
| `process(cv::Mat frame)` | Samples the four edges; runs post-processing; returns `true` on success |
| `processWidth(frame, type)` | Samples one edge (`"top"`, `"bottom"`, `"left"`, `"right"`) |
| `postProcessing()` | Applies LUT-based brightness deadzone in HSV space |
| `initLut()` | Pre-computes the brightness-reduction lookup table |
| `initStep(cv::Mat frame)` | Recomputes `stepX`/`stepY` based on frame dimensions and bezel |
| `getImage()` | Returns the processed 1-row `cv::Mat` |
| `getMappingsImage()` | Returns a colour-coded 1-row image showing edge segment ranges |
| `getCaptureMappings()` | Returns JSON with `{topS, topE, bottomS, bottomE, leftS, leftE, rightS, rightE}` pixel indices |
| `setData(json)` | Updates padding, iterations, deadzone parameters |
| `setBoundOffset(int x, int y)` | Sets letterbox crop offsets |
| `setBezelScaled(double x, double y)` | Sets bezel inset and triggers `initStep` recalc |
| `startDebugImage()` / `stopDebugImage()` | Capture 120 frames of processed output |

**Output image layout** (columns left→right):
```
[  top pixels (iterationsX)  |  bottom pixels (iterationsX)  |  left pixels (iterationsY)  |  right pixels (iterationsY)  ]
```

---

### 5.13 CaptureDevice

**File:** `src/device/captureDevice/capture-device.cpp`

Wraps an `OpenCV VideoCapture` and a `FrameProcessor`. Manages webcam initialisation, aspect-ratio correction, bezel compensation, sleep detection, and no-signal detection.

**Key fields:**

| Field | Type | Description |
|---|---|---|
| `cap` | `cv::VideoCapture` | OpenCV capture object |
| `frame` | `cv::Mat` | Most recent raw frame |
| `frameProcessor` | `FrameProcessor` | Edge sampling processor |
| `isCapturing` | `bool` | Whether the webcam is open and producing frames |
| `aspectW / aspectH` | `int` | Detected or configured content aspect ratio |
| `screenTruthW/H` | `int` | Physical screen aspect ratio (default 16:9) |
| `bezelX / bezelY` | `double` | Bezel percentage of screen width/height |
| `sleepTime` | `long long` | ms without frame change before `tv_sleeping` is set (default 3000) |
| `no_capture_pixels` | `vector<cv::Vec3b>` | Expected pixel colours for the "no signal" test pattern |

**Key methods:**

| Method | Description |
|---|---|
| `initCapture()` | Opens webcam at index 0 and sets FPS |
| `updateFrame()` | Grabs the next frame; calls `checkLastFrames()` and `checkSleepTime()` |
| `processFrame()` | Runs `frameProcessor.process(frame)`; reinitialises on persistent failures |
| `getImage()` | Returns the processed edge-strip image |
| `getRawFrame()` | Returns the raw camera frame |
| `getMappingsImage()` | Returns the colour-coded mapping image |
| `getCaptureMappings()` | Returns the JSON capture-mapping index ranges |
| `setAspectRatio(w, h)` | Calculates and applies letterbox offsets for a given content aspect ratio |
| `setScreenTruth(w, h)` | Updates the physical-screen aspect ratio and recalculates bezel/offsets |
| `calcBezel()` | Recalculates bezel-scaled values based on current aspect ratios |
| `checkLastFrames()` | Probes random pixels to detect frame motion; updates `timeSLFC`; checks for no-signal pattern |
| `checkSleepTime()` | Sets `tv_sleeping = true` when no frame change for `sleepTime` ms |
| `checkIfSignal(int index)` | Tests specific pixel against the no-signal test pattern |
| `getJson()` | Serialises all capture settings (iterations, padding, deadzone, bezel, aspect ratio) |
| `setData(json)` | Applies settings from JSON |
| `startDebugImage()` / `stopDebugImage()` | Delegate to `FrameProcessor` debug capture |

---

### 5.14 Preset

**File:** `src/presets/preset.cpp`

A named snapshot of `LedDeviceSettings` tied to a specific device.

| Field | Type | Description |
|---|---|---|
| `name` | `string` | Preset name (e.g. `"default"`, `"sleep"`) |
| `deviceName` | `string` | Name of the device this preset belongs to |
| `deviceType` | `string` | `"addressable"` or `"non-addressable"` |
| `data` | `json` | Full raw JSON of the settings |

**Key methods:**

| Method | Description |
|---|---|
| `getName()` / `setName()` | Get/set name |
| `getDeviceName()` / `setDeviceName()` | Get/set device name |
| `getDeviceType()` / `setDeviceType()` | Get/set type string |
| `getNumType()` | Returns `1` for addressable, `0` for non-addressable |
| `getJson()` / `setJson()` | Raw JSON access |

---

### 5.15 Sleeper

**File:** `src/sleeping/sleeper.cpp`

Records the pre-sleep state of a device so it can be restored when the TV signal returns.

| Field | Type | Description |
|---|---|---|
| `index` | `int` | Index of the device in the global `devices` vector |
| `name` | `string` | Device name |
| `prev_preset` | `string` | Preset name the device was using before sleep |

---

### 5.16 SleepController

**File:** `src/sleeping/sleepController.cpp`

Called once per frame. Monitors `tv_no_signal` and automatically applies/removes a `"sleep"` preset on all devices that are in TV mode.

**Logic:**

* If `tv_no_signal` becomes true and `state == 0`: saves each active TV-mode device's preset into a `Sleeper`, then switches all of them to `"sleep"` preset. Sets `state = 1`.
* If `tv_no_signal` becomes false and `state == 1`: restores every device that is still on `"sleep"` to its saved preset. Sets `state = 0`.
* Polls the capture device every 2 seconds when not actively using the webcam to detect signal changes without blocking the main loop.

---

### 5.17 FileManage

**File:** `src/files/file-manage.cpp`

Simple file read/write utility.

| Method | Description |
|---|---|
| `write(string content)` | Overwrites the file with `content` |
| `read()` | Reads the entire file as a single string and returns it |

Three instances are created at startup:
* `devicesManager` → `resources/devices.json`
* `presetsManager` → `resources/presets.json`
* `tvSettingsManager` → `resources/tv_settings.json`

---

## 6. Utility Functions

### `src/utils/math-util.cpp`

| Function | Description |
|---|---|
| `int max(int a, int b)` | Returns the larger of two integers |
| `int min(int a, int b)` | Returns the smaller of two integers |
| `double maxd(double a, double b)` | Returns the larger of two doubles |
| `double mind(double a, double b)` | Returns the smaller of two doubles |

### `src/utils/basic-presets.cpp`

| Function | Description |
|---|---|
| `json getSleepPreset(json settings)` | Copies a settings JSON object and sets `name = "sleep"` and `power = "off"` |

### `src/utils/base-64.cpp`

| Function | Description |
|---|---|
| `string base64_encode(bytes, len)` | Standard Base64 encoder |
| `string convertMatToBase64(cv::Mat)` | JPEG-encodes an `cv::Mat` and Base64-encodes the result |

### `src/utils/cv-tools.cpp`

| Function | Description |
|---|---|
| `cv::Vec3b processPixel(pixAbove, pixBelow)` | Returns a pixel whose channels are `abs(above - below) * 10`, clamped to 0–255 |
| `cv::Mat getDifferenceImage(cv::Mat)` | Computes a row-by-row difference image (useful for detecting animation movement) |

---

## 7. Persistence Functions

**File:** `src/files/load-save.cpp`

| Function | Description |
|---|---|
| `loadDevices()` | Reads `resources/devices.json`; instantiates `AddressableLedDevice` or `NonAddressableLedDevice` and pushes into `devices` |
| `saveDevices()` | Serialises all devices to JSON and writes `resources/devices.json` |
| `loadPresets()` | Reads `resources/presets.json`; pushes `Preset` objects into `presets` |
| `savePresets()` | Serialises all presets and writes `resources/presets.json` |
| `loadTVSettings()` | Reads `resources/tv_settings.json` and calls `captureDevice.setData()` |
| `saveTVSettings()` | Calls `captureDevice.getJson()` and writes `resources/tv_settings.json` |
| `loadDeviceAndPresets()` | Calls `loadDevices()`, `loadPresets()`, `loadTVSettings()` in order |

---

## 8. HTTP Server & REST API Endpoints

The server starts on **port 3001** and listens on all interfaces (`0.0.0.0`). CORS is permitted for `http://localhost:3000`.

### 8.1 Standard Response Format

All API endpoints return JSON in the following envelope:

```json
{
  "status": "success" | "error",
  "message": "Human-readable description",
  "code": "200" | "400" | "404" | "500",
  "data": <payload object or array>
}
```

### 8.2 Device Endpoints

#### `GET /devices`

Returns a list of all configured devices.

**Response `data`:** Array of device JSON objects (see [Device JSON Schema](#91-device-json)).

---

#### `POST /addDevice`

Creates a new device.

**Request body:** Device JSON object (see [Device JSON Schema](#91-device-json)).

**Behaviour:**
* Rejects if a device with the same `name` already exists.
* Creates an `AddressableLedDevice` or `NonAddressableLedDevice` depending on `type`.
* Automatically creates `"default"` and `"sleep"` presets for addressable devices.
* Saves devices and presets to disk.

---

#### `POST /updateDevice/:name`

Updates an existing device by name.

**URL parameter:** `name` – device name.  
**Request body:** Partial or full device JSON; any provided fields are applied.

**Behaviour:** Calls `device.setData(body)`; saves devices to disk.

---

#### `DELETE /devices/delete/:name`

Deletes a device and all its associated presets.

**URL parameter:** `name` – device name.  
**Request body:** `{ "type": "addressable" | "non-addressable" }` (required to disambiguate).

---

#### `POST /show-mapping/:name`

Temporarily lights up the LEDs belonging to a specific mapping segment (useful for visual configuration).

**URL parameter:** `name` – device name.  
**Request body:** `{ "index": <int> }` – the mapping index to highlight.

---

#### `POST /show-length/:name`

Temporarily lights up all LEDs on a device at full white to visualise the total strip length.

**URL parameter:** `name` – device name.  
**Request body:** `{}` (index field present but ignored).

---

### 8.3 Preset Endpoints

#### `GET /presets`

Returns all saved presets.

**Response `data`:** Array of preset JSON objects (see [Preset JSON Schema](#92-preset-json)).

---

#### `POST /presets`

Creates or updates a preset.

**Request body:** `{ "settings": <LedDeviceSettings JSON> }`.

**Behaviour:**
* Looks for an existing preset with matching `name`, `device_name`, and `device_type`.
* If found: updates it. If not found: appends it.
* Saves presets to disk.
* Updates the corresponding device's active preset name.

---

### 8.4 Animation / Mode Endpoints

#### `GET /modes`

Returns a list of all available mode names (animation names + `"tv"`).

**Response `data`:** Array of strings, e.g. `["rainbow", "pulse", "tv"]`.

---

#### `POST /animations/upload`

Uploads a new animation image file (multipart/form-data).

**Form field:** `file` – the image file.

**Behaviour:**
* Rejects if an animation with the same filename stem already exists.
* Saves the file to `./resources/animations/`.
* Adds an `Animation` object to the global `animations` vector.

---

#### `DELETE /animations/:name`

Deletes an animation image and removes it from the runtime list.

**URL parameter:** `name` – animation name (filename stem, no extension).

---

### 8.5 TV / Capture-Device Endpoints

#### `GET /tv`

Returns the current capture device and frame-processor settings.

**Response `data`:** Capture device JSON (see [Capture Device JSON Schema](#93-capture-device-json)).

---

#### `POST /tv`

Updates capture device / frame-processor settings and saves them.

**Request body:** Partial or full capture device JSON.

---

#### `POST /tv/aspect-ratio`

Convenience endpoint to set just the content aspect ratio.

**Request body:** `{ "aspect_ratio": "16:9" | "4:3" | "21:9" | "1:1" | "W:H" }`.

**Behaviour:** Calls `captureDevice.setAspectRatio(w, h)`.

---

#### `GET /capture-mappings`

Returns the pixel-index ranges for each edge segment in the processed image.

**Response `data`:**
```json
{
  "topS": 0, "topE": <iterationsX-1>,
  "bottomS": <iterationsX>, "bottomE": <iterationsX*2-1>,
  "leftS": <iterationsX*2>, "leftE": <iterationsX*2+iterationsY-1>,
  "rightS": <iterationsX*2+iterationsY>, "rightE": <iterationsX*2+iterationsY*2-1>
}
```

---

### 8.6 Debug Endpoints

#### `GET /debug-info`

Returns the raw capture-card frame dimensions.

**Response `data`:** `{ "input": { "width": <int>, "height": <int> } }`.

---

#### `GET /debug-image`

Captures 120 frames (~5 seconds) of both the frame-processor output and device LED output. Returns Base64-encoded JPEG images.

**Response `data`:**
```json
{
  "image":          "<base64 JPEG of frame-processor output over 120 frames>",
  "led_image":      "<base64 JPEG of device[0] LED colours over 120 frames>",
  "diff_image":     "<base64 JPEG of frame-by-frame difference of image>",
  "diff_led_image": "<base64 JPEG of frame-by-frame difference of led_image>"
}
```

---

### 8.7 Static Client Files

| Pattern | Description |
|---|---|
| `GET /static/**` | Serves files from `./build/static/` |
| `GET /**` | Serves `./build/index.html` (React SPA catch-all) |

---

## 9. JSON Data Schemas

### 9.1 Device JSON

#### Addressable LED Device

```json
{
  "name":            "string – unique device identifier",
  "type":            "addressable",
  "led_count":       42,
  "pin_out":         18,
  "preset":          "string – name of active preset",
  "transition_speed": 1000,
  "settings": {
    "name":           "string",
    "device_type":    "addressable",
    "device_name":    "string",
    "mode":           "tv | <animation-name>",
    "power":          "on | off",
    "animation_speed": 1000,
    "mapping": [
      {
        "ledSIndx": 0,
        "ledEIndx": 10,
        "mapSIndx": 0,
        "mapEIndx": 10
      }
    ]
  }
}
```

#### Non-Addressable LED Device

```json
{
  "name":            "string – unique device identifier",
  "type":            "non-addressable",
  "preset":          "string",
  "transition_speed": 1000,
  "pin_out": {
    "r_pin": 0,
    "g_pin": 0,
    "b_pin": 0
  },
  "settings": {
    "name":           "string",
    "device_type":    "non-addressable",
    "device_name":    "string",
    "mode":           "tv | <animation-name>",
    "power":          "on | off",
    "animation_speed": 1000,
    "mapping": [
      {
        "ledSIndx": 0,
        "ledEIndx": 0,
        "mapSIndx": 0,
        "mapEIndx": 5
      }
    ]
  }
}
```

---

### 9.2 Preset JSON

```json
{
  "name":        "string – preset name",
  "device_name": "string – owning device name",
  "device_type": "addressable | non-addressable",
  "mode":        "tv | <animation-name>",
  "power":       "on | off",
  "animation_speed": 1000,
  "mapping": [ ... ]
}
```

---

### 9.3 Capture Device JSON

```json
{
  "padding":    { "x": 80, "y": 80 },
  "iterations": { "x": 16, "y": 9 },
  "step":       { "x": 40.0, "y": 40.0 },
  "deadzone":   { "decrease": 38.0, "threshold": 70, "power": 3.0 },
  "process1Pxl": false,
  "input":      { "width": 1920, "height": 1080 },
  "aspect_ratio": "16:9",
  "truth_aspect_ratio": { "x": 16, "y": 9 },
  "bezel":      { "x": 0.0, "y": 0.0 }
}
```

---

## 10. Dependencies & Libraries

| Library | Purpose | Distribution |
|---|---|---|
| [nlohmann/json](https://github.com/nlohmann/json) | JSON parsing and serialisation | Single-header `resources/json.hpp` |
| [cpp-httplib](https://github.com/yhirose/cpp-httplib) | Embedded HTTP/HTTPS server | Single-header `resources/httplib.h` |
| [OpenCV 4.x](https://github.com/opencv/opencv/tree/4.x) | Frame capture, image processing, colour space conversions | System install (`pkg-config opencv4`) |
| [rpi_ws281x](https://github.com/jgarff/rpi_ws281x) | WS2811/WS2812B LED strip driver over DMA | System install (`-lws2811`) – Raspberry Pi only; stubbed on other platforms |
| [pigpio](https://abyz.me.uk/rpi/pigpio/) | GPIO PWM for non-addressable LED strips | System install (`-lpigpio`) – Raspberry Pi only; stubbed on other platforms |
| [websocketpp](https://github.com/zaphoyd/websocketpp) | WebSocket server (work in progress) | Referenced in compile flags |

**Compile command (development / macOS):**
```
g++ main.cpp -o app -I/usr/local/include/opencv4 -std=c++14 `pkg-config --libs opencv4`
```

**Compile command (Raspberry Pi with LEDs):**
```
g++ main.cpp -o app -std=c++14 `pkg-config --cflags --libs opencv4` -I/usr/local/include/ws2811 -lws2811 -lstdc++fs
```

**Required runtime files (same directory as the executable):**
```
./build/            – compiled React front-end
./resources/
    animations/     – animation image files
    devices.json    – persisted device configuration (created automatically)
    presets.json    – persisted preset configuration (created automatically)
    tv_settings.json – persisted TV/capture settings (created automatically)
```
