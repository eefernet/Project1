
## Please read everything below before calling ```git clone```

# Project1 — Sound App 

A desktop audio application built with the [JUCE](https://juce.com/) framework in C++. The app features role-based access control with Owner and Guest user types, each with different levels of access to audio features like recording, playback, filtering, and sound management.

> **Status:** Sprint 1 — Authentication & account management are functional. Audio features and dashboards are planned for Sprints 2–3.

---

## TODO: Add link to Microsoft planner thingy

## Features

### Sprint 1 (Current)
- **Login screen** with Owner authentication (username + password) and Guest quick-access
- **Account creation** with form validation (empty fields, password confirmation, role selection)
- **Role-based user system** using an inheritance hierarchy (`User` → `Owner` / `Guest`)
- **View management** — seamless switching between Login and Account Setup screens
- **First-user enforcement** — the first account created is automatically assigned the `Owner` role (very seecure trust me....)

### Sprint 2–3 (Planned)
- Owner dashboard: record sound, view waveform, play, filter, save, list saved sounds, cluster map
- Guest dashboard: browse sound list, play & filter, download, view cluster map (read-only)
- Persistent user storage
- Audio recording and playback engine

## Architecture

```
Project1/
├── Project1.jucer              # Projucer project file
├── README.md
└── Source/
    ├── Main.cpp                        # App entry point & window setup
    ├── MainComponent.h/cpp             # Root component — screen routing & user management
    │
    ├── Views/
    │   ├── LoginComponent.h/cpp        # Login screen UI & input handling
    │   └── AccountSetupComponent.h/cpp # Account creation form & validation
    │
    ├── Models/
    │   ├── User.h/cpp                  # Abstract base class for all users
    │   ├── Owner.h/cpp                 # Owner subclass (full access)
    │   └── Guest.h/cpp                 # Guest subclass (limited access)
    │
    └── Utils/
        └── UserRole.h                  # Enum for Owner / Guest roles
```

Communication between UI components uses `std::function` callbacks — child components expose public callback members that the parent (`MainComponent`) wires up to its own handler methods. This
should make it easier for us to add multiple views that will be a child of the main view controller.

## Prerequisites

- **JUCE Framework** — download from [juce.com](https://juce.com/get-juce/)
- **Projucer** — included with JUCE, used to generate IDE project files from `Project1.jucer`
- **C++17** compatible compiler
- **Windows:** Visual Studio 2019+ | **macOS:** Xcode 12+ | **Linux:** GCC 9+ with standard JUCE dependencies

## Building

1. Clone the repository:
   ```bash
   git clone https://github.com/eefernet/Project1.git
   ```
2. Open `Project1.jucer` in **Projucer**.
3. Set the JUCE module path in Projucer. Defualt sometimes works, but you might need to point your **Projucer** application to the right foilders that contain your modules and Jucer main folder.
4. Click **Save and Open in IDE**.
5. Build and run from your IDE (tested with Visual Studio — Debug x64).

## Adding Files (IMPORTANT PLEASE READ)

Make sure when you are adding new .h and .cpp files, you DO NOT create files within your IDE. Make sure you create those files in Projucer then open project in your IDE.

## Usage

1. **First launch** — you'll be prompted to create an Owner account (role is locked to Owner for the first user).
2. **Subsequent launches** — log in as Owner with your credentials, continue as Guest, or create additional accounts.
3. Owner and Guest dashboards are placeholders for now and will be implemented in future sprints.
