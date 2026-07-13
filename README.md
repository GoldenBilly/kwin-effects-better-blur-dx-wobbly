# Better Blur DX — Wobbly API companion

This fork of Better Blur DX adds the Better Wobbly Windows inter-effect mesh API v1.
Better Blur DX still owns blur generation and the final composite; Better Wobbly
Windows supplies only the deformed window geometry.

See [Better Wobbly Windows](https://github.com/GoldenBilly/BetterWobblyWindows)

The versioned inter-effect API is documented in
[`docs/inter-effect-api.md`](docs/inter-effect-api.md). It lets another KWin
effect request a deformed Better Blur DX composite without accessing or
reimplementing the plugin's private textures, framebuffers or shaders.

Better Blur DX is a loose continuation of [Better Blur](https://github.com/taj-ny/kwin-effects-forceblur/) - a fork of the Plasma 6 blur effect with additional features and bug fixes.

![image](https://github.com/user-attachments/assets/f8a7c618-89b4-485a-b0f8-29dd5f77e3ca)

### Features
- Force blur
- Adjustable blur brightness, contrast and saturation
- Adjustable corner radius
- Refraction

You may notice these are *less* features than the original Better Blur.
Notably ["Static Blur" is currently missing](https://github.com/xarblu/kwin-effects-better-blur-dx/issues/16)
because it wasn't trivially portable to the Plasma 6.5 blur and needs to be reimplemented.

### Bug fixes
Fixes for blur-related Plasma bugs that haven't been patched yet.

- Blur may sometimes disappear during animations
- [Transparent color schemes don't work properly with the Breeze application style](https://github.com/taj-ny/kwin-effects-forceblur/pull/38)

### Supported Plasma releases
Better Blur DX should always work on the current stable version of Plasma.
Older versions aren't tested much besides a basic "it compiles" and may or may not work.

Currently supported versions: **6.5**, **6.6**, **6.7**

### X11 Support
While building the effect is supported for both X11 and Wayland, X11 is more or less deprecated and not tested much.
Thus the X11 version of the effect needs to be explicitly built using the `-DBBDX_X11=ON` `cmake` option.

We'll see how feasible maintaining the X11 version is for future Plasma releases as it seems like
KWin X11 doesn't see any API changes since version 6.5 meaning the Wayland and X11 will keep drifting apart.

# Installation
> [!IMPORTANT]
> If the effect stops working after a system upgrade, rebuild and reinstall it.
> The effect only works for the *exact KWin* version it was built for.
> Additionally KWin needs to be restarted (e.g. via logout+login) to load an *updated* version of the effect
> (the initial install doesn't need this).

## Distribution packages
No distribution packages are currently available for this fork. Install it manually from source.

## Manual
> [!NOTE]
> On Fedora Kinoite and other distributions based on it, the effect must be built in a container.

### Dependencies
- CMake
- Extra CMake Modules
- Plasma 6
- Qt 6
- KF6
- KWin development packages

<details>
  <summary>Arch Linux</summary>
  <br>

  Wayland:
  ```
  sudo pacman -S base-devel git extra-cmake-modules qt6-tools kwin
  ```

  X11:
  ```
  sudo pacman -S base-devel git extra-cmake-modules qt6-tools kwin-x11
  ```
</details>

<details>
  <summary>Debian-based (KDE Neon, Kubuntu, Ubuntu)</summary>
  <br>

  Wayland:
  ```
  sudo apt install -y git cmake g++ extra-cmake-modules qt6-tools-dev kwin-dev libkf6configwidgets-dev gettext libkf6crash-dev libkf6globalaccel-dev libkf6kio-dev libkf6service-dev libkf6notifications-dev libkf6kcmutils-dev libkdecorations3-dev libxcb-composite0-dev libxcb-randr0-dev libxcb-shm0-dev libxcb-res0-dev libxcb-sync-dev
  ```

  X11:
  ```
  sudo apt install -y git cmake g++ extra-cmake-modules qt6-tools-dev kwin-x11-dev libkf6configwidgets-dev gettext libkf6crash-dev libkf6globalaccel-dev libkf6kio-dev libkf6service-dev libkf6notifications-dev libkf6kcmutils-dev libkdecorations3-dev libxcb-composite0-dev libxcb-randr0-dev libxcb-shm0-dev libxcb-res0-dev libxcb-sync-dev
  ```
</details>

<details>
  <summary>Fedora 41, 42</summary>
  <br>

  Wayland:
  ```
  sudo dnf -y install git cmake extra-cmake-modules gcc-g++ kf6-kwindowsystem-devel plasma-workspace-devel libplasma-devel qt6-qtbase-private-devel qt6-qtbase-devel cmake kwin-devel extra-cmake-modules kwin-devel kf6-knotifications-devel kf6-kio-devel kf6-kcrash-devel kf6-ki18n-devel kf6-kguiaddons-devel libepoxy-devel kf6-kglobalaccel-devel kf6-kcmutils-devel kf6-kconfigwidgets-devel kf6-kdeclarative-devel kdecoration-devel kf6-kglobalaccel kf6-kdeclarative libplasma kf6-kio qt6-qtbase kf6-kguiaddons kf6-ki18n wayland-devel libdrm-devel
  ```

  X11:
  ```
  sudo dnf -y install git cmake extra-cmake-modules gcc-g++ kf6-kwindowsystem-devel plasma-workspace-devel libplasma-devel qt6-qtbase-private-devel qt6-qtbase-devel cmake extra-cmake-modules kf6-knotifications-devel kf6-kio-devel kf6-kcrash-devel kf6-ki18n-devel kf6-kguiaddons-devel libepoxy-devel kf6-kglobalaccel-devel kf6-kcmutils-devel kf6-kconfigwidgets-devel kf6-kdeclarative-devel kdecoration-devel kf6-kglobalaccel kf6-kdeclarative libplasma kf6-kio qt6-qtbase kf6-kguiaddons kf6-ki18n wayland-devel libdrm-devel kwin-x11-devel
  ```
</details>

<details>
  <summary>openSUSE</summary>
  <br>

  Wayland:
  ```
  sudo zypper in -y git cmake-full gcc-c++ kf6-extra-cmake-modules kcoreaddons-devel kguiaddons-devel kconfigwidgets-devel kwindowsystem-devel ki18n-devel kiconthemes-devel kpackage-devel frameworkintegration-devel kcmutils-devel kirigami2-devel "cmake(KF6Config)" "cmake(KF6CoreAddons)" "cmake(KF6FrameworkIntegration)" "cmake(KF6GuiAddons)" "cmake(KF6I18n)" "cmake(KF6KCMUtils)" "cmake(KF6KirigamiPlatform)" "cmake(KF6WindowSystem)" "cmake(Qt6Core)" "cmake(Qt6DBus)" "cmake(Qt6Quick)" "cmake(Qt6Svg)" "cmake(Qt6Widgets)" "cmake(Qt6Xml)" "cmake(Qt6UiTools)" "cmake(KF6Crash)" "cmake(KF6GlobalAccel)" "cmake(KF6KIO)" "cmake(KF6Service)" "cmake(KF6Notifications)" libepoxy-devel kwin6-devel
  ```

  X11:
  ```
  sudo zypper in -y git cmake-full gcc-c++ kf6-extra-cmake-modules kcoreaddons-devel kguiaddons-devel kconfigwidgets-devel kwindowsystem-devel ki18n-devel kiconthemes-devel kpackage-devel frameworkintegration-devel kcmutils-devel kirigami2-devel "cmake(KF6Config)" "cmake(KF6CoreAddons)" "cmake(KF6FrameworkIntegration)" "cmake(KF6GuiAddons)" "cmake(KF6I18n)" "cmake(KF6KCMUtils)" "cmake(KF6KirigamiPlatform)" "cmake(KF6WindowSystem)" "cmake(Qt6Core)" "cmake(Qt6DBus)" "cmake(Qt6Quick)" "cmake(Qt6Svg)" "cmake(Qt6Widgets)" "cmake(Qt6Xml)" "cmake(Qt6UiTools)" "cmake(KF6Crash)" "cmake(KF6GlobalAccel)" "cmake(KF6KIO)" "cmake(KF6Service)" "cmake(KF6Notifications)" libepoxy-devel kwin6-x11-devel
  ```
</details>

### Building
```sh
cd kwin-effects-better-blur-dx-wobbly
./build.sh
```

For X11, configure manually with `-DBBDX_X11=ON`. The installed plugin and KCM
filenames are unique and do not overwrite the upstream effect.

**Remove the *build* directory when rebuilding the effect.**

# Usage

1. Build and install this fork. It appears as **Better Blur DX — Wobbly API**.
2. Disable the original effect if it is installed.
3. Enable **Better Blur DX — Wobbly API** and **Better Wobbly Windows**.

There is no need to uninstall the original effect. Keep it disabled in KWin's
Desktop Effects while this fork is enabled. Only one Better Blur DX implementation
should be enabled at a time, because enabling both may cause visual or rendering
conflicts.

This fork provides normal stationary desktop blur and also handles API v1 requests
from Better Wobbly Windows. During an active request, it generates or reuses its own
cache and composites it directly through the Wobbly mesh. Better Wobbly's standalone
renderer runs only when the provider is missing or fails technically. A
`DeclinedByPolicy` result is final and prevents fallback from bypassing
`Blur only matching` or `Blur all except matching`.

The fork intentionally reads `[Effect-better-blur-dx]` from `kwinrc`, so existing
Better Blur DX visual settings continue to apply.

# Known Issues
## Effect pairing

Use this fork with **Better Wobbly Windows**. Keep the original Better Blur DX
disabled when this fork is enabled; running both blur effects may cause conflicts.
Do not enable KWin's stock Wobbly Windows effect at the same time as Better Wobbly
Windows, because they both deform the same window geometry.

## High cursor latency or stuttering on Wayland
This effect can be very resource-intensive if you have a lot of windows open. On Wayland, high GPU load may result in higher cursor latency or even stuttering. If that bothers you, set the following environment variable: ``KWIN_DRM_NO_AMS=1``. If that's not enough, try enabling or disabling the software cursor by also setting ``KWIN_FORCE_SW_CURSOR=0`` or ``KWIN_FORCE_SW_CURSOR=1``.

Intel GPUs use software cursor by default due to [this bug](https://gitlab.freedesktop.org/drm/intel/-/issues/9571), however it doesn't seem to affect all GPUs.

# Credits
- [a-parhom/LightlyShaders](https://github.com/a-parhom/LightlyShaders) - CMakeLists.txt files
- [taj-ny/kwin-effects-forceblur](https://github.com/taj-ny/kwin-effects-forceblur) - The original Better Blur
