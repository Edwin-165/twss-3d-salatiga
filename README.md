# 🏛️ Taman Wisata Sejarah Salatiga - 3D OpenGL Project

Taman Wisata Sejarah Salatiga located in Salatiga, Central Java that also known as "Taman Wisesa". This place is a cultural heritage park that commemorates Indonesia’s national heroes. Completed in 2023, the park features authentic military relics, monuments, and statues honoring figures such as Brigjen Sudiarto, Yos Sudarso, and Adisucipto. Set amidst scenic rice fields, TWSS offers visitors an immersive journey into the past through historical exhibits, educational installations, and tranquil natural surroundings. It serves as both a place of reflection and recreation, blending patriotism with public engagement.<br><br>
This project is a 3D visualization of the "Taman Wisata Sejarah Salatiga" (Salatiga Historical Tourist Park), developed using OpenGL (GLUT) in C++. It was created as a college project to demonstrate 3D rendering techniques, object modeling, camera control, lighting, and texture mapping.

[Project Overview](https://github.com/user-attachments/assets/ef82b486-17ed-41f7-afea-d48a44f180ed)

## 🚀 Project Overview

Our team consisted of 3 people has designed and implemented a virtual representation of the Taman Wisata Sejarah Salatiga, aiming to capture its essence and key features in a 3D environment. The scene includes:

* **Diverse Terrain:** A detailed ground with grass textures, and water bodies.
* **Architectural Models:**
    * A grand **Museum** building with intricate details, windows, and an interior.
    * Another **Museum** building (museum_2) with similar architectural elements.
    * A traditional **Mosque** with distinct features.
    * Quaint **Kiosks** lining the streets.
    * A historical **Monument** and a **Hero Statue**.
    * A dedicated **Rear Building** area.
* **Vehicles & Structures:**
    * A **Tank** with independent movement controls (for a bit of fun!).
    * An **Artillery** piece.
    * A **Plane** model.
    * A **Boat** model on the water.
* **Environmental Elements:**
    * **Trees** (textured and simple models) for a lush landscape.
    * **Roads** to define paths.
    * **Benches** for a park-like feel.
    * A **Flagpole** with a flag.
* **Interior Details (Museum):**
    * A **Receptionist Desk** complete with a **Laptop** and **Coffee Mug**.
    * **Chairs** and **Bookshelves** with various books.
    * A **Wall Clock** and **Wall-mounted TV**.
    * A fascinating **Miniature City** on a table.
    * Decorative **Vases** and **Picture Frames**.
    * Functional **Ceiling Lights** and **Wall-mounted AC units**.
    * A **Long Table** adorned with **Plates of Food**, **Bowls of Soup**, and **Potted Plants**.
    * Miniature **Warships** and **Tanks** for display.
* **Dynamic Features:**
    * **Day/Night Cycle:** Toggle between day and night modes, affecting ambient lighting and the appearance of stars.
    * **Interior Lighting:** Turn on/off lights within the museum during night mode.
    * **Animated Statue:** The hero statue continuously rotates.
    * **Tank Movement:** Control the tank's position independently.

## 🛠️ Technologies Used

* **OpenGL (GLUT):** For rendering 3D graphics and managing the windowing system.
* **C++:** The primary programming language used for development.
* **stb_image:** A single-file public domain library used for loading image textures (e.g., `.jpg`) into OpenGL.
* **Code::Blocks IDE:** The integrated development environment used for building and running the project.

## 🚀 Getting Started

To compile and run this project on your local machine, follow these steps:

### Prerequisites

* **A C++ compiler:** GCC (MinGW for Windows, or Clang/g++ for Linux/macOS) is recommended.
* **OpenGL, GLUT, and GLU libraries:** These are usually part of your system's development tools or can be installed via package managers.
* **`stb_image.h`:** This header file is included directly in the project. Ensure it's in your `include` directory (as shown in `main.cpp`).

### 📝 Side Note
_This code is a bit chaotic and far from optimized—think of it as a creative sandbox rather than production-grade elegance. Also, don’t forget to update the texture file paths when changing themes, or you'll get that mysterious “invisible object” vibe._ 😅🙏

### ©  Credits
* Edwin Hartono
* Rangga Prawiro U.
* Aaron Fafa Dyland S.

### 📥 Cloning the Repository

```bash
git clone https://github.com/Edwin-165/twss-3d-salatiga.git
cd twss-3d-salatiga
