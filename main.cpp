#include <GL/glut.h>
#include <math.h>
#include <iostream>

#define STB_IMAGE_IMPLEMENTATION
#include "include/stb_image.h"
using namespace std;

// Konstanta frame per detik untuk timer
#define FPS 120
// Konversi derajat ke radian
#define TO_RADIANS 3.14/180.0

GLuint groundTextureID;
GLuint treeTextureID;
GLuint waterTextureID;
float starField[500][3];
bool isDayMode = true;
bool isBulbOn = false;
float patungRotationAngle = 0.0f;

// Ukuran window
const int width = 1280;
const int height = 720;

// Variabel sudut untuk animasi (jika diperlukan)
int i;
float sudut;
// Variabel untuk geseran (transformasi posisi)
double x_geser = 0.0, y_geser = 0.0, z_geser = 0.0;

// Variabel sudut kamera (pitch: rotasi atas-bawah, yaw: rotasi kiri-kanan)
float pitch = 0.0, yaw = 0.0;
// Posisi kamera pada sumbu X dan Z, serta posisi vertikal terbang (height)
float camX = 0.0, camZ = 1500, terbang = -300;

// Deklarasi fungsi utama OpenGL
void display();
void reshape(int w, int h);
void timer(int);
void passive_motion(int, int);
void camera();
void keyboard(unsigned char key, int x, int y);
void keyboard_up(unsigned char key, int x, int y);
void initStars();

// Struktur untuk status gerakan kamera
struct Motion {
    bool Forward, Backward, Left, Right, Naik, Turun;
};
Motion motion = { false,false,false,false,false,false };

// Gerakan Tank
// Variabel untuk melacak status tombol IJKL
float tankPosX = 0.0f; // Posisi X tank
float tankPosZ = 0.0f; // Posisi Z tank
struct TankMotion {
    bool ForwardI, BackwardK, ForwardL, BackwardJ;
};
TankMotion tankMotion = { false, false, false, false };

void initStars() {
    for (int i = 0; i < 500; i++) {
        float theta = (float)(rand() % 360) * TO_RADIANS;
        float phi = (float)(rand() % 180 - 90) * TO_RADIANS;
        starField[i][0] = 4000.0f * cos(phi) * cos(theta);
        starField[i][1] = 4000.0f * sin(phi);
        starField[i][2] = 4000.0f * cos(phi) * sin(theta);
    }
}

void init() {

    glClearColor(1, 1, 1, 0.0); // Background putih
    glutSetCursor(GLUT_CURSOR_NONE); // Hilangkan cursor mouse
    glEnable(GL_DEPTH_TEST); // Aktifkan depth test untuk 3D
    glEnable(GL_BLEND); // Aktifkan blending untuk transparansi
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA); // Atur fungsi blending
    glDepthFunc(GL_LEQUAL); // Fungsi depth test
    glutWarpPointer(width / 2, height / 2); // Letakkan pointer mouse di tengah

    glEnable(GL_TEXTURE_2D); // Aktifkan kemampuan tekstur 2D

    // --- PENGATURAN PENCAHAYAAN (WORLD LIGHT / MATAHARI) ---
    glEnable(GL_LIGHTING); // Aktifkan sistem pencahayaan OpenGL
    glEnable(GL_LIGHT0);   // Aktifkan sumber cahaya pertama (GL_LIGHT0)a
    glEnable(GL_LIGHT1);
    glEnable(GL_LIGHT2);
    glEnable(GL_LIGHT3);
    glEnable(GL_LIGHT4);
    glEnable(GL_COLOR_MATERIAL);
    glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);

    initStars();
    stbi_set_flip_vertically_on_load(1);

    // --- Muat tekstur untuk ground menggunakan stb_image ---
    int img_width, img_height, img_channels;
    unsigned char* img_data = stbi_load("D:\\Kuliah bang\\Semester 6\\Grafika Komputer\\Tugas_Rancang\\src\\textures\\grass.jpg", &img_width, &img_height, &img_channels, 0);

    if (img_data) {
        glGenTextures(1, &groundTextureID);
        glBindTexture(GL_TEXTURE_2D, groundTextureID);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

        GLenum format;
        if (img_channels == 3) { // 3 channels = RGB
            format = GL_RGB;
        } else if (img_channels == 4) { // 4 channels = RGBA
            format = GL_RGBA;
        } else {
            printf("Error: Unsupported image format channels: %d\n", img_channels);
            stbi_image_free(img_data);
            return;
        }

        gluBuild2DMipmaps(GL_TEXTURE_2D, GL_RGB, img_width, img_height, format, GL_UNSIGNED_BYTE, img_data);

        stbi_image_free(img_data); // Penting: bebaskan memori setelah tekstur diunggah ke GPU
        printf("Ground texture loaded successfully! ID: %d, W: %d, H: %d, Channels: %d\n", groundTextureID, img_width, img_height, img_channels);
    } else {
        printf("Error: Failed to load ground texture: %s\n", stbi_failure_reason());
    }

    // --- Muat tekstur untuk pohon ---
    unsigned char* tree_data = stbi_load("D:\\Kuliah bang\\Semester 6\\Grafika Komputer\\Tugas_Rancang\\src\\textures\\trees.jpg", &img_width, &img_height, &img_channels, 0); // Ganti dengan path ke tekstur pohon Anda
    if (tree_data) {
        glGenTextures(1, &treeTextureID);
        glBindTexture(GL_TEXTURE_2D, treeTextureID);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

        GLenum format;
        if (img_channels == 3) { // 3 channels = RGB
            format = GL_RGB;
        } else if (img_channels == 4) { // 4 channels = RGBA
            format = GL_RGBA;
        } else {
            printf("Error: Unsupported image format channels: %d\n", img_channels);
            stbi_image_free(tree_data);
            return;
        }

        gluBuild2DMipmaps(GL_TEXTURE_2D, GL_RGB, img_width, img_height, format, GL_UNSIGNED_BYTE, tree_data);

        stbi_image_free(tree_data); // Penting: bebaskan memori setelah tekstur diunggah ke GPU
        printf("Tree texture loaded successfully! ID: %d, W: %d, H: %d, Channels: %d\n", treeTextureID, img_width, img_height, img_channels);
    } else {
        printf("Error: Failed to load ground texture: %s\n", stbi_failure_reason());
    }

    // --- Muat tekstur untuk air ---
    unsigned char* water_data = stbi_load("D:\\Kuliah bang\\Semester 6\\Grafika Komputer\\Tugas_Rancang\\src\\textures\\water.jpg", &img_width, &img_height, &img_channels, 0); // Ganti dengan path ke tekstur pohon Anda
    if (water_data) {
        glGenTextures(1, &waterTextureID);
        glBindTexture(GL_TEXTURE_2D, waterTextureID);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

        GLenum format;
        if (img_channels == 3) { // 3 channels = RGB
            format = GL_RGB;
        } else if (img_channels == 4) { // 4 channels = RGBA
            format = GL_RGBA;
        } else {
            printf("Error: Unsupported image format channels: %d\n", img_channels);
            stbi_image_free(water_data);
            return;
        }

        gluBuild2DMipmaps(GL_TEXTURE_2D, GL_RGB, img_width, img_height, format, GL_UNSIGNED_BYTE, water_data);

        stbi_image_free(water_data); // Penting: bebaskan memori setelah tekstur diunggah ke GPU
        printf("Water texture loaded successfully! ID: %d, W: %d, H: %d, Channels: %d\n", waterTextureID, img_width, img_height, img_channels);
    } else {
        printf("Error: Failed to load ground texture: %s\n", stbi_failure_reason());
    }
}

void ground() {
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, groundTextureID);

    glColor3f(0.0, 1.0, 0.0);

    float ground_y = -500.1;

    glBegin(GL_QUADS);

    float texture_repeat_x = 10.0;
    float texture_repeat_z = 10.0; // Menggunakan Z sebagai sumbu 'kedalaman' tekstur
    glTexCoord2f(0.0f, 0.0f);
    glVertex3f(-1000.0, ground_y, -1000.0);
    glTexCoord2f(texture_repeat_x, 0.0f);
    glVertex3f(1200.0, ground_y, -1000.0);
    glTexCoord2f(texture_repeat_x, texture_repeat_z);
    glVertex3f(1200.0, ground_y, 2000.0);
    glTexCoord2f(0.0f, texture_repeat_z);
    glVertex3f(-1000.0, ground_y, 2000.0);

    glEnd();
    glDisable(GL_TEXTURE_2D);
}

void treeBackgroundX() {
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, treeTextureID);

    glColor3f(1.0f, 1.0f, 1.0f);

    glBegin(GL_QUADS);


    float x_start_world = -1000.0; // Titik awal X dari pagar pohon
    float x_end_world = 1200.0;    // Titik akhir X dari pagar pohon
    float y_base_world = -500.1;   // Ketinggian dasar pohon (sama dengan ground)
    float y_top_world = y_base_world + 150.0; // Tinggi pohon adalah 100 unit di atas y_base_world
    float z_position_world = -1000.0; // Posisi Z dari pagar pohon (ini akan menjadi garis lurus)

    float length_world_x = x_end_world - x_start_world; // Panjang dunia di sumbu X (2000 unit)
    float texture_repeat_s = length_world_x / 150.0; // Contoh: setiap 50 unit dunia, tekstur diulang 1 kali.

    float texture_repeat_t = 1.0; // Biasanya tinggi tekstur diulang 1 kali (dari 0.0 ke 1.0) untuk billboard.

    // Sudut Kiri Bawah
    glTexCoord2f(0.0f, 0.0f); // Koordinat tekstur (s, t)
    glVertex3f(x_start_world, y_base_world, z_position_world);

    // Sudut Kanan Bawah
    glTexCoord2f(texture_repeat_s, 0.0f); // s akan berulang sesuai panjang
    glVertex3f(x_end_world, y_base_world, z_position_world);

    // Sudut Kanan Atas
    glTexCoord2f(texture_repeat_s, texture_repeat_t); // t biasanya 1.0 untuk tinggi
    glVertex3f(x_end_world, y_top_world, z_position_world);

    // Sudut Kiri Atas
    glTexCoord2f(0.0f, texture_repeat_t); // t biasanya 1.0 untuk tinggi
    glVertex3f(x_start_world, y_top_world, z_position_world);

    glEnd();
    glDisable(GL_TEXTURE_2D);
}

void treeBackgroundZ() {
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, treeTextureID); // Gunakan tekstur pohon Anda
    glColor3f(1.0f, 1.0f, 1.0f); // Pastikan warna dasar putih

    glBegin(GL_QUADS);

    // --- Koordinat Dunia untuk Tembok Pohon Vertikal ---
    // Tembok ini akan membentang sepanjang sumbu Z dan memiliki tinggi di sumbu Y.
    // Posisi X dari tembok ini akan tetap.

    float z_start_wall = -1000.0; // Titik awal Z dari pagar pohon
    float z_end_wall = 2000.0;    // Titik akhir Z dari pagar pohon
    float y_base_wall = -500.1;   // Dasar tembok berada di ketinggian ground
    float y_top_wall = y_base_wall + 150.0; // Tinggi tembok 100 unit
    float x_position_wall = 1200.0; // Posisi X dari tembok ini (sesuaikan)

    // --- Pengulangan Tekstur ---
    // Hitung pengulangan tekstur berdasarkan panjang tembok di sumbu Z.
    float wall_length_z = z_end_wall - z_start_wall; // Panjang tembok di Z
    float texture_repeat_s = wall_length_z / 150.0; // Contoh: setiap 50 unit dunia, tekstur diulang 1 kali.
                                                   // Sesuaikan 50.0 ini.
    float texture_repeat_t = 1.0; // Tinggi tekstur normal (dari 0.0 ke 1.0)

    // --- Definisi Vertex untuk Quad Tembok Pohon ---
    // Quad akan vertikal (YZ plane), menghadap +X atau -X tergantung bagaimana Anda menyusunnya.
    // Di sini kita akan membuat pagar yang menghadap ke arah X negatif (ke dalam) jika x_position_wall positif,
    // atau ke arah X positif (ke dalam) jika x_position_wall negatif.

    // Sudut Belakang Bawah (di Z_start_wall)
    glTexCoord2f(0.0f, 0.0f);
    glVertex3f(x_position_wall, y_base_wall, z_start_wall);

    // Sudut Depan Bawah (di Z_end_wall)
    glTexCoord2f(texture_repeat_s, 0.0f);
    glVertex3f(x_position_wall, y_base_wall, z_end_wall);

    // Sudut Depan Atas (di Z_end_wall)
    glTexCoord2f(texture_repeat_s, texture_repeat_t);
    glVertex3f(x_position_wall, y_top_wall, z_end_wall);

    // Sudut Belakang Atas (di Z_start_wall)
    glTexCoord2f(0.0f, texture_repeat_t);
    glVertex3f(x_position_wall, y_top_wall, z_start_wall);

    glEnd();
    glDisable(GL_TEXTURE_2D);
}

void street() {
    glColor3f(0.3, 0.3, 0.3); // Warna abu-abu gelap untuk jalan

    // Sisi Kiri
    glBegin(GL_QUADS);
        glVertex3f(-600.0, -500, 0.0); // Titik kiri belakang
        glVertex3f(-450.0, -500, 0.0); // Titik kanan belakang
        glVertex3f(-450.0, -500, 1200.0); // Titik kanan depan
        glVertex3f(-600.0, -500, 1200.0); // Titik kiri depan
    glEnd();

    // Sisi Kiri Bawah
    glBegin(GL_QUADS);
        glVertex3f(-600.0, -500, 1200.0); // Titik kiri belakang
        glVertex3f(-450.0, -500, 1200.0); // Titik kanan belakang
        glVertex3f(-450.0, -500, 1350.0); // Titik kanan depan
        glVertex3f(-450.0, -500, 1350.0); // Titik kiri depan
    glEnd();

    // Sisi Bawah
    glBegin(GL_QUADS);
        glVertex3f(-450.0, -500, 1200.0);
        glVertex3f(200.0, -500, 1200.0);
        glVertex3f(200.0, -500, 1350.0);
        glVertex3f(-450.0, -500, 1350.0);
    glEnd();

    // Sisi Kanan Bawah
    glBegin(GL_QUADS);
        glVertex3f(200.0, -500, 1350.0); // Titik kiri belakang
        glVertex3f(350.0, -500, 1200.0); // Titik kanan belakang
        glVertex3f(350.0, -500, 1200.0); // Titik kanan depan
        glVertex3f(200.0, -500, 1200.0); // Titik kiri depan
    glEnd();

    // Sisi Kanan
    glBegin(GL_QUADS);
        glVertex3f(200.0, -500, 1200.0);
        glVertex3f(350.0, -500, 1200.0);
        glVertex3f(350.0, -500, 0.0);
        glVertex3f(200.0, -500, 0.0);
    glEnd();

    // Sisi Kanan Atas
    glBegin(GL_QUADS);
        glVertex3f(200.0, -500, 0.0);
        glVertex3f(350.0, -500, 0.0);
        glVertex3f(200.0, -500, -150.0);
        glVertex3f(200.0, -500, -150.0);
    glEnd();

    // Sisi Atas
    glBegin(GL_QUADS);
        glVertex3f(-450.0, -500, 0.0);
        glVertex3f(200.0, -500, 0.0);
        glVertex3f(200.0, -500, -150.0);
        glVertex3f(-450.0, -500, -150.0);
    glEnd();

    // Sisi Kiri Atas
    glBegin(GL_QUADS);
        glVertex3f(-600.0, -500, 0.0);
        glVertex3f(-450.0, -500, 0.0);
        glVertex3f(-450.0, -500, -150.0);
        glVertex3f(-450.0, -500, -150.0);
    glEnd();

    // Garis Tepi
    glColor3f(1.0, 1.0, 1.0); // Warna putih

    // Garis Tepi Dalam
    // Sisi Tepi Kiri
    glBegin(GL_QUADS);
        glVertex3f(-450.0, -500.0, 0.0);
        glVertex3f(-440.0, -500.0, 0.0);
        glVertex3f(-440.0, -500.0, 1200.0);
        glVertex3f(-450.0, -500.0, 1200.0);
    glEnd();

    // Sisi Tepi Bawah
    glBegin(GL_QUADS);
        glVertex3f(-450.0, -500.0, 1200.0);
        glVertex3f(200.0, -500.0, 1200.0);
        glVertex3f(200.0, -500.0, 1190.0);
        glVertex3f(-450.0, -500.0, 1190.0);
    glEnd();

    // Sisi Tepi Kanan
    glBegin(GL_QUADS);
        glVertex3f(190.0, -500.0, 1200.0);
        glVertex3f(200.0, -500.0, 1200.0);
        glVertex3f(200.0, -500.0, 0.0);
        glVertex3f(190.0, -500.0, 0.0);
    glEnd();

    // Sisi Tepi Atas
    glBegin(GL_QUADS);
        glVertex3f(-450.0, -500.0, 0.0);
        glVertex3f(200.0, -500.0, 0.0);
        glVertex3f(200.0, -500.0, 10.0);
        glVertex3f(-450.0, -500.0, 10.0);
    glEnd();

    // Garis Tepi Luar
    // Sisi Kiri
    glBegin(GL_QUADS);
        glVertex3f(-610.0, -500, 0.0); // Titik kiri belakang
        glVertex3f(-600.0, -500, 0.0); // Titik kanan belakang
        glVertex3f(-600.0, -500, 1200.0); // Titik kanan depan
        glVertex3f(-610.0, -500, 1200.0); // Titik kiri depan
    glEnd();

    // Sisi Kiri Bawah
    glBegin(GL_QUADS);
        glVertex3f(-610.0, -500, 1200.0); // Titik kiri belakang
        glVertex3f(-600.0, -500, 1200.0); // Titik kanan belakang
        glVertex3f(-440.0, -500, 1360.0); // Titik kanan depan
        glVertex3f(-450.0, -500, 1360.0); // Titik kiri depan
    glEnd();

    // Sisi Bawah
    glBegin(GL_QUADS);
        glVertex3f(-450.0, -500, 1350.0);
        glVertex3f(200.0, -500, 1350.0);
        glVertex3f(200.0, -500, 1360.0);
        glVertex3f(-450.0, -500, 1360.0);
    glEnd();

    // Sisi Kanan Bawah
    glBegin(GL_QUADS);
        glVertex3f(200.0, -500, 1350.0); // Titik kiri belakang
        glVertex3f(200.0, -500, 1360.0); // Titik kanan belakang
        glVertex3f(360.0, -500, 1200.0); // Titik kanan depan
        glVertex3f(350.0, -500, 1200.0); // Titik kiri depan
    glEnd();

    // Sisi Kanan
    glBegin(GL_QUADS);
        glVertex3f(350.0, -500, 1200.0);
        glVertex3f(360.0, -500, 1200.0);
        glVertex3f(360.0, -500, 0.0);
        glVertex3f(350.0, -500, 0.0);
    glEnd();

    // Sisi Kanan Atas
    glBegin(GL_QUADS);
        glVertex3f(360.0, -500, 0.0);
        glVertex3f(350.0, -500, 0.0);
        glVertex3f(200.0, -500, -150.0);
        glVertex3f(200.0, -500, -160.0);
    glEnd();

    // Sisi Atas
    glBegin(GL_QUADS);
        glVertex3f(-450.0, -500, -150.0);
        glVertex3f(200.0, -500, -150.0);
        glVertex3f(200.0, -500, -160.0);
        glVertex3f(-450.0, -500, -160.0);
    glEnd();

    // Sisi Kiri Atas
    glBegin(GL_QUADS);
        glVertex3f(-450.0, -500, -150.0);
        glVertex3f(-450.0, -500, -160.0);
        glVertex3f(-610.0, -500, 0.0);
        glVertex3f(-600.0, -500, 0.0);
    glEnd();

}

void basic_cube(float x1, float x2, float y1, float y2, float z1, float z2, float color1, float color2, float color3) {
    glColor3f(color1, color2, color3);

    glBegin(GL_QUADS);
        // Bottom face
        glVertex3f(x1, y1, z1);
        glVertex3f(x2, y1, z1);
        glVertex3f(x2, y1, z2);
        glVertex3f(x1, y1, z2);

        // Top face
        glVertex3f(x1, y2, z1);
        glVertex3f(x2, y2, z1);
        glVertex3f(x2, y2, z2);
        glVertex3f(x1, y2, z2);

        // Front face
        glVertex3f(x1, y1, z2);
        glVertex3f(x2, y1, z2);
        glVertex3f(x2, y2, z2);
        glVertex3f(x1, y2, z2);

        // Back face
        glVertex3f(x1, y1, z1);
        glVertex3f(x2, y1, z1);
        glVertex3f(x2, y2, z1);
        glVertex3f(x1, y2, z1);

        // Left face
        glVertex3f(x1, y1, z1);
        glVertex3f(x1, y1, z2);
        glVertex3f(x1, y2, z2);
        glVertex3f(x1, y2, z1);

        // Right face
        glVertex3f(x2, y1, z1);
        glVertex3f(x2, y1, z2);
        glVertex3f(x2, y2, z2);
        glVertex3f(x2, y2, z1);
    glEnd();
}

void buat_pagar_besi_mengelilingi(float x_min, float x_max, float z_min, float z_max, float y) {
    float tinggi_pagar = 20.0f;
    float diameter_tiang = 2.0f;
    float jarak_tiang = 5.0f;

    // Sisi depan (sepanjang sumbu X pada Z_min)
    for (float x = x_min; x <= x_max; x += jarak_tiang) {
        basic_cube(x - diameter_tiang / 2, x + diameter_tiang / 2,
                   y, y + tinggi_pagar,
                   z_min - diameter_tiang, z_min + diameter_tiang,
                   0.2f, 0.2f, 0.2f); // Abu gelap
    }

    // Sisi belakang (sepanjang sumbu X pada Z_max)
    for (float x = x_min; x <= x_max; x += jarak_tiang) {
        basic_cube(x - diameter_tiang / 2, x + diameter_tiang / 2,
                   y, y + tinggi_pagar,
                   z_max - diameter_tiang, z_max + diameter_tiang,
                   0.2f, 0.2f, 0.2f);
    }

    // Sisi kiri (sepanjang sumbu Z pada X_min)
    for (float z = z_min + jarak_tiang; z < z_max; z += jarak_tiang) {
        basic_cube(x_min - diameter_tiang, x_min + diameter_tiang,
                   y, y + tinggi_pagar,
                   z - diameter_tiang / 2, z + diameter_tiang / 2,
                   0.2f, 0.2f, 0.2f);
    }

    // Sisi kanan (sepanjang sumbu Z pada X_max)
    for (float z = z_min + jarak_tiang; z < z_max; z += jarak_tiang) {
        basic_cube(x_max - diameter_tiang, x_max + diameter_tiang,
                   y, y + tinggi_pagar,
                   z - diameter_tiang / 2, z + diameter_tiang / 2,
                   0.2f, 0.2f, 0.2f);
    }
}

void drawTank() {
    //Tank
    glPushMatrix();
    glTranslatef(tankPosX, 0.0f, tankPosZ); // Y=0.0f karena pergerakan horizontal
    basic_cube(-850.0, -700.0, -500.0, -470.0, 400.0, 300.0, 0.16, 0.24, 0.08); // Bawah Tank
    basic_cube(-800.0, -720.0, -470.0, -450.0, 380.0, 320.0, 0.2, 0.3, 0.1);// Atas Tank
    basic_cube(-720.0, -640.0, -465.0, -455.0, 355.0, 345.0, 0.0, 0.3, 0.1);//Moncong Tank
    glPopMatrix();
}

void drawArtilery() {
    //AA1
    basic_cube(-890.0, -660.0, -500.0, -499.0, 760.0, 940.0, 0.6, 0.6, 0.6);//Alas
    basic_cube(-850.0, -800.0, -500.0, -470.0, 790.0, 840.0, 0.9, 0.9, 0.9);//Bawah
    basic_cube(-845.0, -805.0, -470.0, -455.0, 795.0, 835.0, 0.7, 0.7, 0.7);//Middle
    basic_cube(-850.0, -800.0, -455.0, -445.0, 790.0, 840.0, 0.9, 0.9, 0.9);//Atas
    basic_cube(-820.0, -790.0, -445.0, -435.0, 795.0, 835.0, 0.8, 0.8, 0.8);//Atas Pucuk
    basic_cube(-805.0, -755.0, -465.0, -455.0, 810.0, 820.0, 0.4, 0.4, 0.4);//Moncong Kecil
    basic_cube(-755.0, -695.0, -465.0, -455.0, 805.0, 825.0, 0.4, 0.4, 0.4);//Moncong Besar
    //AA2
    basic_cube(-850.0, -800.0, -500.0, -470.0, 860.0, 910.0, 0.9, 0.9, 0.9);//Bawah
    basic_cube(-845.0, -805.0, -470.0, -455.0, 865.0, 905.0, 0.7, 0.7, 0.7);//Middle
    basic_cube(-850.0, -800.0, -455.0, -445.0, 860.0, 910.0, 0.9, 0.9, 0.9);//Atas
    basic_cube(-820.0, -790.0, -445.0, -435.0, 865.0, 905.0, 0.8, 0.8, 0.8);//Atas Pucuk
    basic_cube(-805.0, -755.0, -465.0, -455.0, 880.0, 890.0, 0.4, 0.4, 0.4);//Moncong Kecil
    basic_cube(-755.0, -695.0, -465.0, -455.0, 875.0, 895.0, 0.4, 0.4, 0.4);//Moncong Besar
    buat_pagar_besi_mengelilingi(-890.0, -660.0, 760.0, 940.0, -499.0f);//Pagar
    basic_cube(-658.0, -657.0, -490.0, -480.0, 830.0, 870.0, 1.0, 0.96, 0.82);//Display AA
}

void drawPesawat() {
    // Definisi Alas (dari kode Anda)
    float alas_x1 = 30.0;
    float alas_x2 = 340.0;
    float alas_y_base = -500.0;
    float alas_y_top = -499.0;
    float alas_z1 = 1400.0;
    float alas_z2 = 1580.0;

    basic_cube(alas_x1, alas_x2, alas_y_base, alas_y_top, alas_z1, alas_z2, 0.6, 0.6, 0.6);//Alas

    // --- Posisi Pusat Pesawat Relatif terhadap Alas ---
    // Mari kita hitung titik tengah alas sebagai referensi
    float center_x = (alas_x1 + alas_x2) / 2.0f; // (50 + 320) / 2 = 185.0
    float center_y = alas_y_top + 20.0f;          // Sedikit di atas alas
    float center_z = (alas_z1 + alas_z2) / 2.0f; // (1400 + 1580) / 2 = 1490.0

    // --- Dimensi Pesawat (Sesuaikan untuk proporsi Cureng) ---
    float body_length = 150.0f; // Panjang badan dari depan ke belakang
    float body_width = 30.0f;   // Lebar badan
    float body_height = 40.0f;  // Tinggi badan di bagian tengah

    float wing_span = 200.0f;   // Rentang sayap total
    float wing_chord = 60.0f;   // Lebar sayap (dari depan ke belakang)
    float wing_thickness = 5.0f;// Ketebalan sayap

    float tail_horizontal_span = 80.0f; // Lebar sayap ekor horizontal
    float tail_horizontal_chord = 30.0f; // Lebar sayap ekor horizontal
    float tail_vertical_height = 60.0f; // Tinggi sirip ekor vertikal
    float tail_thickness = 5.0f;        // Ketebalan ekor

    float engine_radius = 25.0f; // Radius mesin (untuk kubus, ini akan menjadi sisi kubus)
    float engine_length = 30.0f; // Panjang mesin
    float propeller_width = 5.0f; // Ketebalan propeler
    float propeller_height = 80.0f; // Tinggi propeler

    // --- Warna Pesawat ---
    float pesawat_color_r = 0.5f, pesawat_color_g = 0.5f, pesawat_color_b = 0.4f; // Abu-abu militer/hijau pudar
    float prop_color_r = 0.3f, prop_color_g = 0.3f, prop_color_b = 0.3f; // Abu-abu gelap

    // --- Menggambar Pesawat ---
    // Gunakan glPushMatrix dan glPopMatrix untuk mengelompokkan objek pesawat
    glPushMatrix();
        // Geser ke posisi pusat alas
        glTranslatef(center_x, center_y, center_z);

        // Optional: Rotasi pesawat jika ingin menghadap arah lain (misal, 90 derajat di Y)
        glRotatef(10.0f, 1.0f, 0.0f, 0.0f); // Jika ingin pesawat menghadap X positif

        // 1. Badan Pesawat (Fuselage) - disederhanakan
        basic_cube(-body_width / 2, body_width / 2,
                   0.0f, body_height,
                   -body_length / 2, body_length / 2,
                   pesawat_color_r, pesawat_color_g, pesawat_color_b);

        // 2. Sayap (Wings) - Sayap tunggal di bagian tengah
        basic_cube(-(wing_span / 2), wing_span / 2,
                   body_height * 0.4f, body_height * 0.4f + wing_thickness, // Posisi Y di bawah badan
                   -wing_chord / 2, wing_chord / 2,
                   pesawat_color_r* 0.8f, pesawat_color_g* 0.8f, pesawat_color_b* 0.8f);

        // 3. Ekor Horizontal (Stabilizer)
        basic_cube(-(tail_horizontal_span / 2), tail_horizontal_span / 2,
                   body_height * 0.7f, body_height * 0.7f + tail_thickness, // Posisi Y di belakang
                   body_length / 2 - tail_horizontal_chord, body_length / 2, // Di ujung belakang badan
                   pesawat_color_r, pesawat_color_g, pesawat_color_b);

        // 4. Ekor Vertikal (Fin)
        basic_cube(-tail_thickness / 2, tail_thickness / 2,
                   body_height * 0.7f, body_height * 0.7f + tail_vertical_height, // Dari atas badan hingga ketinggian ekor
                   body_length / 2 - tail_horizontal_chord / 2, body_length / 2 + tail_horizontal_chord / 2, // Di belakang
                   pesawat_color_r, pesawat_color_g, pesawat_color_b);

        // 5. Mesin (Engine)
        basic_cube(-engine_radius / 2, engine_radius / 2,
                   body_height * 0.5f, body_height * 0.5f + engine_radius, // Di tengah depan badan
                   -body_length / 2 - engine_length, -body_length / 2, // Menonjol ke depan
                   pesawat_color_r * 0.8f, pesawat_color_g * 0.8f, pesawat_color_b * 0.8f); // Warna lebih gelap

        // 6. Propeler (Balok Vertikal)
        basic_cube(-propeller_width / 2, propeller_width / 2,
                   body_height * 0.5f + engine_radius / 2 - propeller_height / 2, body_height * 0.5f + engine_radius / 2 + propeller_height / 2,
                   -body_length / 2 - engine_length - propeller_width, -body_length / 2 - engine_length, // Depan mesin
                   prop_color_r, prop_color_g, prop_color_b);


    glPopMatrix(); // Kembalikan matriks transformasi

}

void drawKapal(){
    float ground_y = -500;
    float texture_repeat_x = 1.0;
    float texture_repeat_z = 1.0; // Menggunakan Z sebagai sumbu 'kedalaman' tekstur

    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, waterTextureID); // Gunakan tekstur pohon Anda

    glBegin(GL_QUADS);
    glTexCoord2f(0.0f, 0.0f);
    glVertex3f(400.0, ground_y, 240.0);
    glTexCoord2f(texture_repeat_x, 0.0f);
    glVertex3f(600.0, ground_y, 240.0);
    glTexCoord2f(texture_repeat_x, texture_repeat_z);
    glVertex3f(600.0, ground_y, -240.0);
    glTexCoord2f(0.0f, texture_repeat_z);
    glVertex3f(400.0, ground_y, -240.0);
    glEnd();
    glDisable(GL_TEXTURE_2D);

    basic_cube(460.0, 540.0, -500.0, -460.0, -190.0, 190.0, 0.3, 0.3, 0.8); // Badan kapal 1
    basic_cube(480.0, 520.0, -500.0, -460.0, -210.0, 210.0, 0.3, 0.3, 0.8); // Badan kapal 2
    // Dek atas kapal
    basic_cube(470.0, 530.0, -460.0, -445.0, -150.0, 150.0, 0.4, 0.4, 0.9);
    // Kabin tengah kapal
    basic_cube(480.0, 520.0, -445.0, -420.0, -50.0, 50.0, 0.8, 0.8, 0.9);
    // Cerobong asap
    basic_cube(495.0, 505.0, -420.0, -390.0, -10.0, 10.0, 0.2, 0.2, 0.2);
    // Tiang bendera depan
    basic_cube(500.0, 502.0, -465.0, -435.0, -200.0, -198.0, 0.8, 0.0, 0.0); // Tiang
    basic_cube(500.0, 502.0, -435.0, -430.0, -200.0, -178.0, 1.0, 0.0, 0.0); // Bendera Merah
    basic_cube(500.0, 502.0, -440.0, -435.0, -200.0, -178.0, 1.0, 1.0, 1.0); // Bendera Putih
    // Tiang bendera belakang
    basic_cube(500.0, 502.0, -465.0, -435.0, 178.0, 180.0, 0.8, 0.0, 0.0);// Tiang
    basic_cube(500.0, 502.0, -435.0, -430.0, 178.0, 200.0, 1.0, 0.0, 0.0);// Bendera Merah
    basic_cube(500.0, 502.0, -440.0, -435.0, 178.0, 200.0, 1.0, 1.0, 1.0);// Bendera Putih
    //Jendela
    basic_cube(479.0, 521.0, -440.0, -425.0, -10.0, 10.0, 0.2, 0.2, 0.2);//Jendela 1
    basic_cube(479.0, 521.0, -440.0, -425.0, -35.0, -15.0, 0.2, 0.2, 0.2);//Jendela 2
    basic_cube(479.0, 521.0, -440.0, -425.0, 15.0, 35.0, 0.2, 0.2, 0.2);//Jendela 3
    buat_pagar_besi_mengelilingi(463.0, 537.0, -187.0, 187.0, -476.0f);//Pagar
}

void custom_2d(float x1, float y1, float z1, float x2, float y2, float z2, float x3, float y3, float z3, float x4, float y4, float z4, float color1, float color2, float color3) {
    glColor3f(color1, color2, color3);
    glBegin(GL_QUADS);
    glVertex3f(x1, y1, z1);
    glVertex3f(x2, y2, z2);
    glVertex3f(x3, y3, z3);
    glVertex3f(x4, y4, z4);
    glEnd();
}

void rooftopZ(float x_min, float x_max, float y_base, float y_peak,
              float z_min, float z_max, float color1, float color2, float color3) {

    glColor3f(color1, color2, color3);

    // Punggungan akan berada di tengah rentang Z
    float ridge_z_coord = (z_min + z_max) / 2.0;

    // --- Menggambar Sisi Miring Atap ---
    glBegin(GL_QUADS);
        // Sisi Miring "Depan" (dekat z_min)
        glVertex3f(x_max, y_base, z_min);
        glVertex3f(x_min, y_base, z_min);
        glVertex3f(x_min, y_peak, ridge_z_coord);
        glVertex3f(x_max, y_peak, ridge_z_coord);

        // Sisi Miring "Belakang" (dekat z_max)
        glVertex3f(x_min, y_base, z_max);
        glVertex3f(x_max, y_base, z_max);
        glVertex3f(x_max, y_peak, ridge_z_coord);
        glVertex3f(x_min, y_peak, ridge_z_coord);
    glEnd();

    // --- Menggambar Muka Segitiga (Gable Faces) ---
    glColor3f(1.0, 1.0, 1.0);
    glBegin(GL_TRIANGLES);
        // Muka Segitiga Kiri (di x_min)
        glVertex3f(x_min, y_base, z_min);
        glVertex3f(x_min, y_base, z_max);
        glVertex3f(x_min, y_peak, ridge_z_coord);

        // Muka Segitiga Kanan (di x_max)
        glVertex3f(x_max, y_base, z_max);
        glVertex3f(x_max, y_base, z_min);
        glVertex3f(x_max, y_peak, ridge_z_coord);
    glEnd();
}

void rooftopX(float x_min, float x_max, float y_base, float y_peak,
              float z_min, float z_max, float color1, float color2, float color3) {

    glColor3f(color1, color2, color3);

    // Punggungan akan berada di tengah rentang X
    float ridge_x_coord = (x_min + x_max) / 2.0;

    // --- Menggambar Sisi Miring Atap ---
    glBegin(GL_QUADS);
        // Sisi Miring "Kiri" (dekat x_min) - secara visual menghadap kiri
        glVertex3f(x_min, y_base, z_max); // Belakang-bawah-kiri
        glVertex3f(x_min, y_base, z_min); // Depan-bawah-kiri
        glVertex3f(ridge_x_coord, y_peak, z_min); // Puncak-depan
        glVertex3f(ridge_x_coord, y_peak, z_max); // Puncak-belakang

        // Sisi Miring "Kanan" (dekat x_max) - secara visual menghadap kanan
        glVertex3f(x_max, y_base, z_min); // Depan-bawah-kanan
        glVertex3f(x_max, y_base, z_max); // Belakang-bawah-kanan
        glVertex3f(ridge_x_coord, y_peak, z_max); // Puncak-belakang
        glVertex3f(ridge_x_coord, y_peak, z_min); // Puncak-depan
    glEnd();

    // --- Menggambar Muka Segitiga (Gable Faces) ---
    glBegin(GL_TRIANGLES);
        // Muka Segitiga Depan (di z_min) - menghadap "depan" dalam orientasi X
        glColor3f(1.0, 1.0, 1.0);
        glVertex3f(x_min, y_base, z_min); // Kiri-bawah-depan
        glVertex3f(x_max, y_base, z_min); // Kanan-bawah-depan
        glVertex3f(ridge_x_coord, y_peak, z_min); // Puncak-depan

        // Muka Segitiga Belakang (di z_max) - menghadap "belakang" dalam orientasi X

        glColor3f(1.0, 1.0, 1.0);
        glVertex3f(x_max, y_base, z_max); // Kanan-bawah-belakang
        glVertex3f(x_min, y_base, z_max); // Kiri-bawah-belakang
        glVertex3f(ridge_x_coord, y_peak, z_max); // Puncak-belakang
    glEnd();
}

void bangunan_belakang() {
    // Bangunan Belakang
    basic_cube(-350.0, 100.0, -500.0, -495.0, -300.0, -450.0, 0.4, 0.28, 0.15); // Alas Belakang
    basic_cube(-200.0, -50.0, -500.0, -495.0, -200.0, -300.0, 0.4, 0.28, 0.15); // Alas Depan
    basic_cube(-345.0, 95.0, -495.0, -490.0, -305.0, -445.0, 0.5, 0.28, 0.15); // Alas Belakang 2
    basic_cube(-195.0, -55.0, -495.0, -490.0, -205.0, -305.0, 0.5, 0.28, 0.15); // Alas Depan 2
    basic_cube(-340.0, 90.0, -490.0, -485.0, -310.0, -440.0, 0.4, 0.30, 0.15); // Alas Belakang 3
    basic_cube(-190.0, -60.0, -490.0, -485.0, -210.0, -310.0, 0.4, 0.30, 0.15); // Alas Depan 3

    basic_cube(-350.0, -330.0, -495.0, -475.0, -300.0, -320.0, 0.1, 0.1, 0.1); // Bawah Tiang 1
    basic_cube(-347.5, -332.5, -475.0, -400.0, -302.5, -317.5, 0.9, 0.9, 0.9); // Atas Tiang 1

    basic_cube(-350.0, -330.0, -495.0, -475.0, -450.0, -430.0, 0.1, 0.1, 0.1); // Bawah Tiang 2
    basic_cube(-347.5, -332.5, -475.0, -400.0, -447.5, -432.5, 0.9, 0.9, 0.9); // Atas Tiang 2

    basic_cube(80.0, 100.0, -495.0, -475.0, -300.0, -320.0, 0.1, 0.1, 0.1); // Bawah Tiang 3
    basic_cube(82.5, 97.5, -475.0, -400.0, -302.5, -317.5, 0.9, 0.9, 0.9); // Atas Tiang 3

    basic_cube(80.0, 100.0, -495.0, -475.0, -450.0, -430.0, 0.1, 0.1, 0.1); // Bawah Tiang 4
    basic_cube(82.5, 97.5, -475.0, -400.0, -447.5, -432.5, 0.9, 0.9, 0.9); // Atas Tiang 4

    basic_cube(-350.0, 100.0, -400.0, -380.0, -300.0, -450.0, 0.6, 0.28, 0.15); // Atap Belakang
    basic_cube(-200.0, -50.0, -400.0, -380.0, -200.0, -300.0, 0.6, 0.28, 0.15); // Atap Depan

    // Tiang Depan Kiri (menggunakan alas depan)
    basic_cube(-200.0, -180.0, -495.0, -475.0, -200.0, -220.0, 0.1, 0.1, 0.1); // Bagian bawah
    basic_cube(-197.5, -182.5, -475.0, -400.0, -202.5, -217.5, 0.9, 0.9, 0.9); // Bagian atas

    // Tiang Depan Kanan (menggunakan alas depan)
    basic_cube(-70.0, -50.0, -495.0, -475.0, -200.0, -220.0, 0.1, 0.1, 0.1); // Bagian bawah
    basic_cube(-67.5, -52.5, -475.0, -400.0, -202.5, -217.5, 0.9, 0.9, 0.9); // Bagian atas

    // Tiang Tengah Kiri (menggunakan alas depan, sisi yang lebih dalam)
    basic_cube(-200.0, -180.0, -495.0, -475.0, -300.0, -320.0, 0.1, 0.1, 0.1); // Bagian bawah
    basic_cube(-197.5, -182.5, -475.0, -400.0, -302.5, -317.5, 0.9, 0.9, 0.9); // Bagian atas

    // Tiang Tengah Kanan (menggunakan alas depan, sisi yang lebih dalam)
    basic_cube(-70.0, -50.0, -495.0, -475.0, -300.0, -320.0, 0.1, 0.1, 0.1); // Bagian bawah
    basic_cube(-67.5, -52.5, -475.0, -400.0, -302.5, -317.5, 0.9, 0.9, 0.9); // Bagian atas

    // Tiang Belakang
    basic_cube(-200.0, -180.0, -495.0, -475.0, -430.0, -450.0, 0.1, 0.1, 0.1); // Bagian bawah
    basic_cube(-197.5, -182.5, -475.0, -400.0, -432.5, -447.5, 0.9, 0.9, 0.9); // Bagian atas

    basic_cube(-70.0, -50.0, -495.0, -475.0, -430.0, -450.0, 0.1, 0.1, 0.1); // Bagian bawah
    basic_cube(-67.5, -52.5, -475.0, -400.0, -432.5, -447.5, 0.9, 0.9, 0.9); // Bagian atas

    basic_cube(-350.0, 100.0, -400.0, -380.0, -300.0, -450.0, 0.6, 0.28, 0.15); // Atap Belakang
    basic_cube(-200.0, -50.0, -400.0, -380.0, -200.0, -300.0, 0.6, 0.28, 0.15); // Atap Depan

    rooftopZ(-350.0, 100.0, -380.0, -350.0, -450.0, -300.0, 0.6, 0.28, 0.15);
    rooftopX(-200.0, -50.0, -380.0, -350.0, -375.0, -200.0, 0.6, 0.28, 0.15);
}

void museum() {
    // Bangunan Kanan (Main)
    basic_cube(500.0, 800.0, -500.0, -495.0, 820.0, 1000.0, 0.7, 0.7, 0.7); // lt1
    basic_cube(501.0, 799.0, -500.0, -490.0, 825.0, 999.0, 0.6, 0.6, 0.6); // lt2
    basic_cube(500.0, 800.0, -500.0, -485.0, 830.0, 1000.0, 0.7, 0.7, 0.7); // lt3
    basic_cube(520.0, 780.0, -500.0, -400.0, 860.0, 865.0, 1.0, 0.96, 0.82); // Tembok kiri
    basic_cube(520.0, 525.0, -500.0, -400.0, 860.0, 980.0, 1.0, 0.96, 0.82); // Tembok Depan
    basic_cube(775.0, 780.0, -500.0, -400.0, 860.0, 980.0, 1.0, 0.96, 0.82); // Tembok Belakang
    //basic_cube(520.0, 780.0, -500.0, -400.0, 975.0, 980.0, 1.0, 0.96, 0.82); // Tembok Kanan
    basic_cube(500.0, 800.0, -400.0, -390.0, 820.0, 1000.0, 0.55, 0.0, 0.2); // Landasan Atap
    basic_cube(520.0, 780.0, -400.0, -375.0, 840.0, 980.0, 0.55, 0.0, 0.2); // Atap 1
    basic_cube(520.0, 780.0, -400.0, -360.0, 860.0, 960.0, 0.55, 0.0, 0.2); // Atap 2
    basic_cube(520.0, 780.0, -400.0, -345.0, 880.0, 940.0, 0.55, 0.0, 0.2); // Atap 3
    basic_cube(520.0, 780.0, -400.0, -330.0, 900.0, 920.0, 0.55, 0.0, 0.2); // Atap 4
    basic_cube(505.0, 510.0, -490.0, -400.0, 825.0, 832.0, 0.9, 0.9, 0.9); // Pilar 1
    basic_cube(555.0, 560.0, -490.0, -400.0, 825.0, 832.0, 0.9, 0.9, 0.9); // Pilar 2
    basic_cube(605.0, 610.0, -490.0, -400.0, 825.0, 832.0, 0.9, 0.9, 0.9); // Pilar 3
    basic_cube(690.0, 695.0, -490.0, -400.0, 825.0, 832.0, 0.9, 0.9, 0.9); // Pilar 5
    basic_cube(740.0, 745.0, -490.0, -400.0, 825.0, 832.0, 0.9, 0.9, 0.9); // Pilar 5
    basic_cube(790.0, 795.0, -490.0, -400.0, 825.0, 832.0, 0.9, 0.9, 0.9); // Pilar 6
    basic_cube(630.0, 670.0, -500.0, -430.0, 859.0, 866.0, 0.5, 0.5, 0.5); // Frame Pintu
    basic_cube(652.0, 668.0, -500.0, -432.0, 858.0, 867.0, 0.9, 0.9, 0.9); // Pintu Kaca kanan
    basic_cube(632.0, 648.0, -500.0, -432.0, 858.0, 867.0, 0.9, 0.9, 0.9); // Pintu Kaca kiri
    basic_cube(680.0, 675.0, -500.0, -400.0, 859.0, 866.0, 0.9, 0.9, 0.9); // Pilar pintu kanan
    basic_cube(620.0, 625.0, -500.0, -400.0, 859.0, 866.0, 0.9, 0.9, 0.9); // Pilar pintu kiri
    basic_cube(775.0, 781.0, -500.0, -400.0, 859.0, 865.0, 0.9, 0.9, 0.9); // Pilar Pojok kiri Belakang
    basic_cube(518.0, 525.0, -500.0, -400.0, 858.0, 865.0, 0.9, 0.9, 0.9); // Pilar Pojok kiri Depan 1
    basic_cube(518.0, 527.0, -500.0, -400.0, 907.0, 912.0, 0.9, 0.9, 0.9); // Pilar Pojok kiri Depan 2
    basic_cube(518.0, 525.0, -500.0, -400.0, 975.0, 982.0, 0.9, 0.9, 0.9); // Pilar Pojok kanan Depan
    basic_cube(775.0, 781.0, -500.0, -400.0, 975.0, 981.0, 0.9, 0.9, 0.9); // Pilar Pojok kanan Belakang

    //Jendela Kanan Pintu
    basic_cube(730.0, 750.0, -470.0, -430.0, 859.0, 866.0, 0.5, 0.5, 0.5); // Frame Jedela kanan pintu 1
    basic_cube(700.0, 720.0, -470.0, -430.0, 859.0, 866.0, 0.5, 0.5, 0.5); // Frame Jedela kanan pintu 2
    basic_cube(732.0, 748.0, -438.0, -432.0, 858.0, 867.0, 0.0, 1.0, 1.0); // Jedela kanan pintu 1 Bawah
    basic_cube(732.0, 748.0, -468.0, -440.0, 858.0, 867.0, 0.0, 1.0, 1.0); // Jedela kanan pintu 1 Atas
    basic_cube(702.0, 718.0, -438.0, -432.0, 858.0, 867.0, 0.0, 1.0, 1.0); // Jedela kanan pintu 2 Bawah
    basic_cube(702.0, 718.0, -468.0, -440.0, 858.0, 867.0, 0.0, 1.0, 1.0); // Jedela kanan pintu 2 Atas

    //Jendela Tembok Kiri
    basic_cube(520.0, 620.0, -470.0, -430.0, 859.0, 866.0, 0.5, 0.5, 0.5); // Frame Jedela kiri pintu
    basic_cube(602.0, 618.0, -438.0, -432.0, 858.0, 867.0, 0.0, 1.0, 1.0); // Jedela kiri pintu Bawah 1
    basic_cube(602.0, 618.0, -468.0, -440.0, 858.0, 867.0, 0.0, 1.0, 1.0); // Jedela kiri pintu Atas 1
    basic_cube(582.0, 600.0, -438.0, -432.0, 858.0, 867.0, 0.0, 1.0, 1.0); // Jedela kiri pintu Bawah 2
    basic_cube(582.0, 600.0, -468.0, -440.0, 858.0, 867.0, 0.0, 1.0, 1.0); // Jedela kiri pintu Atas 2
    basic_cube(562.0, 580.0, -438.0, -432.0, 858.0, 867.0, 0.0, 1.0, 1.0); // Jedela kiri pintu Bawah 3
    basic_cube(562.0, 580.0, -468.0, -440.0, 858.0, 867.0, 0.0, 1.0, 1.0); // Jedela kiri pintu Atas 3
    basic_cube(542.0, 560.0, -438.0, -432.0, 858.0, 867.0, 0.0, 1.0, 1.0); // Jedela kiri pintu Bawah 4
    basic_cube(542.0, 560.0, -468.0, -440.0, 858.0, 867.0, 0.0, 1.0, 1.0); // Jedela kiri pintu Atas 4
    basic_cube(527.0, 540.0, -438.0, -432.0, 858.0, 867.0, 0.0, 1.0, 1.0); // Jedela kiri pintu Bawah 5
    basic_cube(527.0, 540.0, -468.0, -440.0, 858.0, 867.0, 0.0, 1.0, 1.0); // Jedela kiri pintu Atas 5

    //Jendela Tembok Kanan
//    basic_cube(520.0, 620.0, -470.0, -430.0, 974.0, 981.0, 0.5, 0.5, 0.5); // Frame Jedela Tembok Kanan 1
//    basic_cube(640.0, 660.0, -470.0, -430.0, 974.0, 981.0, 0.5, 0.5, 0.5); // Frame Jedela Tembok Kanan 2
//    basic_cube(602.0, 618.0, -438.0, -432.0, 973.0, 982.0, 0.0, 1.0, 1.0); // Jendela Tembok Kanan Bawah 1
//    basic_cube(602.0, 618.0, -468.0, -440.0, 973.0, 982.0, 0.0, 1.0, 1.0); // Jendela Tembok Kanan Atas 1
//    basic_cube(582.0, 600.0, -438.0, -432.0, 973.0, 982.0, 0.0, 1.0, 1.0); // Jendela Tembok Kanan Bawah 2
//    basic_cube(582.0, 600.0, -468.0, -440.0, 973.0, 982.0, 0.0, 1.0, 1.0); // Jendela Tembok Kanan Atas 2
//    basic_cube(562.0, 580.0, -438.0, -432.0, 973.0, 982.0, 0.0, 1.0, 1.0); // Jendela Tembok Kanan Bawah 3
//    basic_cube(562.0, 580.0, -468.0, -440.0, 973.0, 982.0, 0.0, 1.0, 1.0); // Jendela Tembok Kanan Atas 3
//    basic_cube(542.0, 560.0, -438.0, -432.0, 973.0, 982.0, 0.0, 1.0, 1.0); // Jendela Tembok Kanan Bawah 4
//    basic_cube(542.0, 560.0, -468.0, -440.0, 973.0, 982.0, 0.0, 1.0, 1.0); // Jendela Tembok Kanan Atas 4
//    basic_cube(527.0, 540.0, -438.0, -432.0, 973.0, 982.0, 0.0, 1.0, 1.0); // Jendela Tembok Kanan Bawah 5
//    basic_cube(527.0, 540.0, -468.0, -440.0, 973.0, 982.0, 0.0, 1.0, 1.0); // Jendela Tembok Kanan Atas 5
//    basic_cube(642.0, 658.0, -438.0, -432.0, 973.0, 982.0, 0.0, 1.0, 1.0); // Jendela Tembok Kanan Sendiri Bawah
//    basic_cube(642.0, 658.0, -468.0, -440.0, 973.0, 982.0, 0.0, 1.0, 1.0); // Jendela Tembok Kanan Sendiri Atas

    //Jendela Tembok Depan
    basic_cube(519.0, 526.0, -470.0, -430.0, 860.0, 980.0, 0.5, 0.5, 0.5); // Frame Jedela Tembok Depan
    basic_cube(518.0, 527.0, -438.0, -432.0, 867.0, 885.0, 0.0, 1.0, 1.0); // Jendela Tembok Depan Bawah 1
    basic_cube(518.0, 527.0, -468.0, -440.0, 867.0, 885.0, 0.0, 1.0, 1.0); // Jendela Tembok Depan Atas 1
    basic_cube(518.0, 527.0, -438.0, -432.0, 887.0, 905.0, 0.0, 1.0, 1.0); // Jendela Tembok Depan Bawah 2
    basic_cube(518.0, 527.0, -468.0, -440.0, 887.0, 905.0, 0.0, 1.0, 1.0); // Jendela Tembok Depan Atas 2
    basic_cube(518.0, 527.0, -438.0, -432.0, 914.0, 932.0, 0.0, 1.0, 1.0); // Jendela Tembok Depan Bawah 3
    basic_cube(518.0, 527.0, -468.0, -440.0, 914.0, 932.0, 0.0, 1.0, 1.0); // Jendela Tembok Depan Atas 3
    basic_cube(518.0, 527.0, -438.0, -432.0, 934.0, 952.0, 0.0, 1.0, 1.0); // Jendela Tembok Depan Bawah 4
    basic_cube(518.0, 527.0, -468.0, -440.0, 934.0, 952.0, 0.0, 1.0, 1.0); // Jendela Tembok Depan Atas 4
    basic_cube(518.0, 527.0, -438.0, -432.0, 954.0, 973.0, 0.0, 1.0, 1.0); // Jendela Tembok Depan Bawah 5
    basic_cube(518.0, 527.0, -468.0, -440.0, 954.0, 973.0, 0.0, 1.0, 1.0); // Jendela Tembok Depan Atas 5


    //Interior Bangunan Kanan
    basic_cube(501.0, 799.0, -401.0, -400.0, 821.0, 999.0, 0.95, 0.95, 0.95); // Landasan Atap Dalam
    basic_cube(675.0, 680.0, -500.0, -400.0, 920.0, 981.0, 0.8, 0.8, 0.8); // Tembok Recep Depan
    basic_cube(679.0, 781.0, -500.0, -400.0, 920.0, 925.0, 0.8, 0.8, 0.8); // Tembok Recep Kanan
    basic_cube(730.0, 760.0, -500.0, -430.0, 919.0, 926.0, 0.6, 0.3, 0.1); // Pintu Dalam
    basic_cube(732.0, 734.0, -461.0, -459.0, 917.0, 928.0, 0.5, 0.2, 0.1); // Gagang

    //Alas
    basic_cube(755.0, 775.0, -500.0, -484.0, 880.0, 900.0, 0.9, 0.9, 0.9); // Alas Vas
    //basic_cube(528.0, 548.0, -500.0, -484.0, 880.0, 960.0, 0.9, 0.9, 0.9); // Alas Meja panjang
    basic_cube(565.0, 585.0, -500.0, -470.0, 870.0, 890.0, 0.55, 0.0, 0.2); // Alas Temprament 1
    basic_cube(595.0, 615.0, -500.0, -470.0, 870.0, 890.0, 0.55, 0.0, 0.2); // Alas Temprament 2
}

void museum_2() {

    const float z_axis = 1400.0;

    // Lantai/Alas
    basic_cube(500.0, 800.0, -500.0, -495.0, -1000.0 + z_axis, -820.0 + z_axis, 0.7, 0.7, 0.7); // lt1
    basic_cube(501.0, 799.0, -500.0, -490.0, -999.0 + z_axis, -825.0 + z_axis, 0.6, 0.6, 0.6); // lt2
    basic_cube(500.0, 800.0, -500.0, -485.0, -1000.0 + z_axis, -830.0 + z_axis, 0.7, 0.7, 0.7); // lt3

    // Tembok
    basic_cube(520.0, 780.0, -500.0, -400.0, -865.0 + z_axis, -860.0 + z_axis, 1.0, 0.96, 0.82); // Tembok kiri
    basic_cube(520.0, 525.0, -500.0, -400.0, -980.0 + z_axis, -860.0 + z_axis, 1.0, 0.96, 0.82); // Tembok Depan
    basic_cube(775.0, 780.0, -500.0, -400.0, -980.0 + z_axis, -860.0 + z_axis, 1.0, 0.96, 0.82); // Tembok Belakang
    basic_cube(520.0, 780.0, -500.0, -400.0, -980.0 + z_axis, -975.0 + z_axis, 1.0, 0.96, 0.82); // Tembok Kanan

    // Atap Bertingkat
    basic_cube(500.0, 800.0, -400.0, -390.0, -1000.0 + z_axis, -820.0 + z_axis, 0.55, 0.0, 0.2); // Landasan Atap
    basic_cube(520.0, 780.0, -400.0, -375.0, -980.0 + z_axis, -840.0 + z_axis, 0.55, 0.0, 0.2); // Atap 1
    basic_cube(520.0, 780.0, -400.0, -360.0, -960.0 + z_axis, -860.0 + z_axis, 0.55, 0.0, 0.2); // Atap 2
    basic_cube(520.0, 780.0, -400.0, -345.0, -940.0 + z_axis, -880.0 + z_axis, 0.55, 0.0, 0.2); // Atap 3
    basic_cube(520.0, 780.0, -400.0, -330.0, -920.0 + z_axis, -900.0 + z_axis, 0.55, 0.0, 0.2); // Atap 4

    // Pilar
    basic_cube(505.0, 510.0, -490.0, -400.0, -832.0 + z_axis, -825.0 + z_axis, 0.9, 0.9, 0.9); // Pilar 1
    basic_cube(555.0, 560.0, -490.0, -400.0, -832.0 + z_axis, -825.0 + z_axis, 0.9, 0.9, 0.9); // Pilar 2
    basic_cube(605.0, 610.0, -490.0, -400.0, -832.0 + z_axis, -825.0 + z_axis, 0.9, 0.9, 0.9); // Pilar 3
    basic_cube(690.0, 695.0, -490.0, -400.0, -832.0 + z_axis, -825.0 + z_axis, 0.9, 0.9, 0.9); // Pilar 5
    basic_cube(740.0, 745.0, -490.0, -400.0, -832.0 + z_axis, -825.0 + z_axis, 0.9, 0.9, 0.9); // Pilar 5
    basic_cube(790.0, 795.0, -490.0, -400.0, -832.0 + z_axis, -825.0 + z_axis, 0.9, 0.9, 0.9); // Pilar 6

    // Pintu Utama (Frame dan Kaca)
    basic_cube(630.0, 670.0, -500.0, -430.0, -866.0 + z_axis, -859.0 + z_axis, 0.5, 0.5, 0.5); // Frame Pintu
    basic_cube(652.0, 668.0, -500.0, -432.0, -867.0 + z_axis, -858.0 + z_axis, 0.9, 0.9, 0.9); // Pintu Kaca kanan
    basic_cube(632.0, 648.0, -500.0, -432.0, -867.0 + z_axis, -858.0 + z_axis, 0.9, 0.9, 0.9); // Pintu Kaca kiri

    // Pilar Pintu dan Pojok
    basic_cube(680.0, 675.0, -500.0, -400.0, -866.0 + z_axis, -859.0 + z_axis, 0.9, 0.9, 0.9); // Pilar pintu kanan
    basic_cube(620.0, 625.0, -500.0, -400.0, -866.0 + z_axis, -859.0 + z_axis, 0.9, 0.9, 0.9); // Pilar pintu kiri
    basic_cube(775.0, 781.0, -500.0, -400.0, -865.0 + z_axis, -859.0 + z_axis, 0.9, 0.9, 0.9); // Pilar Pojok kiri Belakang
    basic_cube(518.0, 525.0, -500.0, -400.0, -865.0 + z_axis, -858.0 + z_axis, 0.9, 0.9, 0.9); // Pilar Pojok kiri Depan 1
    basic_cube(518.0, 527.0, -500.0, -400.0, -912.0 + z_axis, -907.0 + z_axis, 0.9, 0.9, 0.9); // Pilar Pojok kiri Depan 2
    basic_cube(518.0, 525.0, -500.0, -400.0, -982.0 + z_axis, -975.0 + z_axis, 0.9, 0.9, 0.9); // Pilar Pojok kanan Depan
    basic_cube(775.0, 781.0, -500.0, -400.0, -981.0 + z_axis, -975.0 + z_axis, 0.9, 0.9, 0.9); // Pilar Pojok kanan Belakang

    // Jendela Kanan Pintu
    basic_cube(730.0, 750.0, -470.0, -430.0, -866.0 + z_axis, -859.0 + z_axis, 0.5, 0.5, 0.5); // Frame Jedela kanan pintu 1
    basic_cube(700.0, 720.0, -470.0, -430.0, -866.0 + z_axis, -859.0 + z_axis, 0.5, 0.5, 0.5); // Frame Jedela kanan pintu 2
    basic_cube(732.0, 748.0, -438.0, -432.0, -867.0 + z_axis, -858.0 + z_axis, 0.0, 1.0, 1.0); // Jedela kanan pintu 1 Bawah
    basic_cube(732.0, 748.0, -468.0, -440.0, -867.0 + z_axis, -858.0 + z_axis, 0.0, 1.0, 1.0); // Jedela kanan pintu 1 Atas
    basic_cube(702.0, 718.0, -438.0, -432.0, -867.0 + z_axis, -858.0 + z_axis, 0.0, 1.0, 1.0); // Jedela kanan pintu 2 Bawah
    basic_cube(702.0, 718.0, -468.0, -440.0, -867.0 + z_axis, -858.0 + z_axis, 0.0, 1.0, 1.0); // Jedela kanan pintu 2 Atas

    // Jendela Tembok Kiri
    basic_cube(520.0, 620.0, -470.0, -430.0, -866.0 + z_axis, -859.0 + z_axis, 0.5, 0.5, 0.5); // Frame Jedela kiri pintu
    basic_cube(602.0, 618.0, -438.0, -432.0, -867.0 + z_axis, -858.0 + z_axis, 0.0, 1.0, 1.0); // Jedela kiri pintu Bawah 1
    basic_cube(602.0, 618.0, -468.0, -440.0, -867.0 + z_axis, -858.0 + z_axis, 0.0, 1.0, 1.0); // Jedela kiri pintu Atas 1
    basic_cube(582.0, 600.0, -438.0, -432.0, -867.0 + z_axis, -858.0 + z_axis, 0.0, 1.0, 1.0); // Jedela kiri pintu Bawah 2
    basic_cube(582.0, 600.0, -468.0, -440.0, -867.0 + z_axis, -858.0 + z_axis, 0.0, 1.0, 1.0); // Jedela kiri pintu Atas 2
    basic_cube(562.0, 580.0, -438.0, -432.0, -867.0 + z_axis, -858.0 + z_axis, 0.0, 1.0, 1.0); // Jedela kiri pintu Bawah 3
    basic_cube(562.0, 580.0, -468.0, -440.0, -867.0 + z_axis, -858.0 + z_axis, 0.0, 1.0, 1.0); // Jedela kiri pintu Atas 3
    basic_cube(542.0, 560.0, -438.0, -432.0, -867.0 + z_axis, -858.0 + z_axis, 0.0, 1.0, 1.0); // Jedela kiri pintu Bawah 4
    basic_cube(542.0, 560.0, -468.0, -440.0, -867.0 + z_axis, -858.0 + z_axis, 0.0, 1.0, 1.0); // Jedela kiri pintu Atas 4
    basic_cube(527.0, 540.0, -438.0, -432.0, -867.0 + z_axis, -858.0 + z_axis, 0.0, 1.0, 1.0); // Jedela kiri pintu Bawah 5
    basic_cube(527.0, 540.0, -468.0, -440.0, -867.0 + z_axis, -858.0 + z_axis, 0.0, 1.0, 1.0); // Jedela kiri pintu Atas 5

    // Jendela Tembok Depan
    basic_cube(519.0, 526.0, -470.0, -430.0, -980.0 + z_axis, -860.0 + z_axis, 0.5, 0.5, 0.5); // Frame Jedela Tembok Depan
    basic_cube(518.0, 527.0, -438.0, -432.0, -885.0 + z_axis, -867.0 + z_axis, 0.0, 1.0, 1.0); // Jendela Tembok Depan Bawah 1
    basic_cube(518.0, 527.0, -468.0, -440.0, -885.0 + z_axis, -867.0 + z_axis, 0.0, 1.0, 1.0); // Jendela Tembok Depan Atas 1
    basic_cube(518.0, 527.0, -438.0, -432.0, -905.0 + z_axis, -887.0 + z_axis, 0.0, 1.0, 1.0); // Jendela Tembok Depan Bawah 2
    basic_cube(518.0, 527.0, -468.0, -440.0, -905.0 + z_axis, -887.0 + z_axis, 0.0, 1.0, 1.0); // Jendela Tembok Depan Atas 2
    basic_cube(518.0, 527.0, -438.0, -432.0, -932.0 + z_axis, -914.0 + z_axis, 0.0, 1.0, 1.0); // Jendela Tembok Depan Bawah 3
    basic_cube(518.0, 527.0, -468.0, -440.0, -932.0 + z_axis, -914.0 + z_axis, 0.0, 1.0, 1.0); // Jendela Tembok Depan Atas 3
    basic_cube(518.0, 527.0, -438.0, -432.0, -952.0 + z_axis, -934.0 + z_axis, 0.0, 1.0, 1.0); // Jendela Tembok Depan Bawah 4
    basic_cube(518.0, 527.0, -468.0, -440.0, -952.0 + z_axis, -934.0 + z_axis, 0.0, 1.0, 1.0); // Jendela Tembok Depan Atas 4
    basic_cube(518.0, 527.0, -438.0, -432.0, -973.0 + z_axis, -954.0 + z_axis, 0.0, 1.0, 1.0); // Jendela Tembok Depan Bawah 5
    basic_cube(518.0, 527.0, -468.0, -440.0, -973.0 + z_axis, -954.0 + z_axis, 0.0, 1.0, 1.0); // Jendela Tembok Depan Atas 5

    // Jendela Tembok Kanan
    basic_cube(520.0, 620.0, -470.0, -430.0, -981.0 + z_axis, -974.0 + z_axis, 0.5, 0.5, 0.5); // Frame Jedela Tembok Kanan 1
    basic_cube(640.0, 660.0, -470.0, -430.0, -981.0 + z_axis, -974.0 + z_axis, 0.5, 0.5, 0.5); // Frame Jedela Tembok Kanan 2
    basic_cube(602.0, 618.0, -438.0, -432.0, -982.0 + z_axis, -973.0 + z_axis, 0.0, 1.0, 1.0); // Jendela Tembok Kanan Bawah 1
    basic_cube(602.0, 618.0, -468.0, -440.0, -982.0 + z_axis, -973.0 + z_axis, 0.0, 1.0, 1.0); // Jendela Tembok Kanan Atas 1
    basic_cube(582.0, 600.0, -438.0, -432.0, -982.0 + z_axis, -973.0 + z_axis, 0.0, 1.0, 1.0); // Jendela Tembok Kanan Bawah 2
    basic_cube(582.0, 600.0, -468.0, -440.0, -982.0 + z_axis, -973.0 + z_axis, 0.0, 1.0, 1.0); // Jendela Tembok Kanan Atas 2
    basic_cube(562.0, 580.0, -438.0, -432.0, -982.0 + z_axis, -973.0 + z_axis, 0.0, 1.0, 1.0); // Jendela Tembok Kanan Bawah 3
    basic_cube(562.0, 580.0, -468.0, -440.0, -982.0 + z_axis, -973.0 + z_axis, 0.0, 1.0, 1.0); // Jendela Tembok Kanan Atas 3
    basic_cube(542.0, 560.0, -438.0, -432.0, -982.0 + z_axis, -973.0 + z_axis, 0.0, 1.0, 1.0); // Jendela Tembok Kanan Bawah 4
    basic_cube(542.0, 560.0, -468.0, -440.0, -982.0 + z_axis, -973.0 + z_axis, 0.0, 1.0, 1.0); // Jendela Tembok Kanan Atas 4
    basic_cube(527.0, 540.0, -438.0, -432.0, -982.0 + z_axis, -973.0 + z_axis, 0.0, 1.0, 1.0); // Jendela Tembok Kanan Bawah 5
    basic_cube(527.0, 540.0, -468.0, -440.0, -982.0 + z_axis, -973.0 + z_axis, 0.0, 1.0, 1.0); // Jendela Tembok Kanan Atas 5
    basic_cube(642.0, 658.0, -438.0, -432.0, -982.0 + z_axis, -973.0 + z_axis, 0.0, 1.0, 1.0); // Jendela Tembok Kanan Sendiri Bawah
    basic_cube(642.0, 658.0, -468.0, -440.0, -982.0 + z_axis, -973.0 + z_axis, 0.0, 1.0, 1.0); // Jendela Tembok Kanan Sendiri Atas
}

void buat_kursi(float x, float y, float z) {
    float alas_tinggi = 2.0f;
    float sandaran_tinggi = 16.0f;
    float kaki_tinggi = 10.0f;
    float lebar = 10.0f;

    // Alas kursi
    basic_cube(x - lebar/2, x + lebar/2, y, y + alas_tinggi, z - lebar/2, z + lebar/2, 0.7f, 0.4f, 0.2f);

    // Sandaran belakang
    basic_cube(x - lebar/2, x + lebar/2, y + alas_tinggi, y + alas_tinggi + sandaran_tinggi,
               z + lebar/2 - 2.0f, z + lebar/2, 0.6f, 0.3f, 0.1f);

    // Kaki-kaki (empat sudut)
    float kaki_size = 1.5f;
    basic_cube(x - lebar/2, x - lebar/2 + kaki_size, y - kaki_tinggi, y, z - lebar/2, z - lebar/2 + kaki_size, 0.3f, 0.3f, 0.3f); // kiri depan
    basic_cube(x + lebar/2 - kaki_size, x + lebar/2, y - kaki_tinggi, y, z - lebar/2, z - lebar/2 + kaki_size, 0.3f, 0.3f, 0.3f); // kanan depan
    basic_cube(x - lebar/2, x - lebar/2 + kaki_size, y - kaki_tinggi, y, z + lebar/2 - kaki_size, z + lebar/2, 0.3f, 0.3f, 0.3f); // kiri belakang
    basic_cube(x + lebar/2 - kaki_size, x + lebar/2, y - kaki_tinggi, y, z + lebar/2 - kaki_size, z + lebar/2, 0.3f, 0.3f, 0.3f); // kanan belakang
}

void buat_rak_buku(float x, float y, float z) {
    float lebar = 40.0f;
    float tinggi = 80.0f;
    float kedalaman = 10.0f;
    float ketebalan_papan = 2.0f;
    int jumlah_rak = 5;

    // Bingkai Samping Kiri
    basic_cube(x - lebar / 2, x - lebar / 2 + ketebalan_papan,
               y, y + tinggi,
               z - kedalaman / 2, z + kedalaman / 2,
               0.5f, 0.25f, 0.1f);

    // Bingkai Samping Kanan
    basic_cube(x + lebar / 2 - ketebalan_papan, x + lebar / 2,
               y, y + tinggi,
               z - kedalaman / 2, z + kedalaman / 2,
               0.5f, 0.25f, 0.1f);

    // Papan Belakang
    basic_cube(x - lebar / 2 + ketebalan_papan, x + lebar / 2 - ketebalan_papan,
               y, y + tinggi,
               z - kedalaman / 2, z - kedalaman / 2 + ketebalan_papan,
               0.4f, 0.2f, 0.1f);

    // Tiap Rak Buku (horizontal) dan isi bukunya
    for (int i = 0; i <= jumlah_rak; ++i) {
        float tinggi_rak = y + i * ((tinggi - ketebalan_papan) / jumlah_rak);

        // Papan rak
        basic_cube(x - lebar / 2 + ketebalan_papan, x + lebar / 2 - ketebalan_papan,
                   tinggi_rak, tinggi_rak + ketebalan_papan,
                   z - kedalaman / 2, z + kedalaman / 2,
                   0.6f, 0.3f, 0.1f);

        // Buku acak - posisi dan warna bervariasi
        float buku_tinggi = 10.0f + (i % 3) * 3.0f;
        float buku_lebar = 2.0f;
        float buku_kedalaman = kedalaman - 4.0f;
        float x_buku = x - lebar / 2 + 4.0f;
        for (int j = 0; j < 6; ++j) {
            float r = 0.3f + 0.1f * (j % 3);
            float g = 0.3f + 0.2f * (i % 2);
            float b = 0.6f - 0.1f * (j % 2);
            basic_cube(x_buku, x_buku + buku_lebar,
                       tinggi_rak + ketebalan_papan, tinggi_rak + ketebalan_papan + buku_tinggi,
                       z - buku_kedalaman / 2, z + buku_kedalaman / 2,
                       r, g, b);
            x_buku += buku_lebar + 1.0f; // Jarak antar buku
        }
    }
}

void buat_meja(float x, float y, float z) {
    float panjang = 20.0f;
    float lebar = 30.0f;
    float tinggi = 10.0f;
    float tinggi_kaki = 15.0f;
    float ketebalan_meja = 2.0f;
    float ukuran_kaki = 2.0f;

    // Permukaan atas meja
    basic_cube(x - panjang / 2, x + panjang / 2,
               y, y + ketebalan_meja,
               z - lebar / 2, z + lebar / 2,
               0.6f, 0.3f, 0.1f); // Warna kayu coklat

    // Kaki kiri depan
    basic_cube(x - panjang / 2 + 1.0f, x - panjang / 2 + 1.0f + ukuran_kaki,
               y - tinggi_kaki, y,
               z - lebar / 2 + 1.0f, z - lebar / 2 + 1.0f + ukuran_kaki,
               0.3f, 0.3f, 0.3f);

    // Kaki kanan depan
    basic_cube(x + panjang / 2 - 1.0f - ukuran_kaki, x + panjang / 2 - 1.0f,
               y - tinggi_kaki, y,
               z - lebar / 2 + 1.0f, z - lebar / 2 + 1.0f + ukuran_kaki,
               0.3f, 0.3f, 0.3f);

    // Kaki kiri belakang
    basic_cube(x - panjang / 2 + 1.0f, x - panjang / 2 + 1.0f + ukuran_kaki,
               y - tinggi_kaki, y,
               z + lebar / 2 - 1.0f - ukuran_kaki, z + lebar / 2 - 1.0f,
               0.3f, 0.3f, 0.3f);

    // Kaki kanan belakang
    basic_cube(x + panjang / 2 - 1.0f - ukuran_kaki, x + panjang / 2 - 1.0f,
               y - tinggi_kaki, y,
               z + lebar / 2 - 1.0f - ukuran_kaki, z + lebar / 2 - 1.0f,
               0.3f, 0.3f, 0.3f);
}

void buat_jam_dinding(float x, float y, float z) {
    int segmen = 40;
    float radius = 5.0f;

    // Wajah jam menghadap ke +X (bidang YZ)
    glColor3f(0.9f, 0.9f, 0.9f);
    glBegin(GL_TRIANGLE_FAN);
        glVertex3f(x, y, z); // Titik pusat jam
        for (int i = 0; i <= segmen; ++i) {
            float angle = 2.0f * 3.1415926f * i / segmen;
            float dy = radius * cosf(angle);
            float dz = radius * sinf(angle);
            glVertex3f(x, y + dy, z + dz);
        }
    glEnd();

    // Bingkai jam
    glColor3f(0.0f, 0.0f, 0.0f);
    glLineWidth(2.0f);
    glBegin(GL_LINE_LOOP);
        for (int i = 0; i <= segmen; ++i) {
            float angle = 2.0f * 3.1415926f * i / segmen;
            float dy = radius * cosf(angle);
            float dz = radius * sinf(angle);
            glVertex3f(x - 0.01f, y + dy, z + dz);
        }
    glEnd();

    // Jarum jam
    glColor3f(0.0f, 0.0f, 0.0f);
    glLineWidth(1.5f);
    glBegin(GL_LINES);
        // Jarum pendek (jam)
        glVertex3f(x - 0.02f, y, z);
        glVertex3f(x - 0.02f, y + 2.5f, z);
        // Jarum panjang (menit)
        glVertex3f(x - 0.02f, y, z);
        glVertex3f(x - 0.02f, y, z + 3.5f);
    glEnd();
}

void buat_laptop(float x, float y, float z) {
    float lebar = 12.0f;
    float ketebalan = 0.5f;
    float kedalaman_keyboard = 8.0f;
    float tinggi_layar = 7.0f;

    // Keyboard (horizontal bagian bawah)
    basic_cube(x - lebar / 2, x + lebar / 2,
               y, y + ketebalan,
               z - kedalaman_keyboard / 2, z + kedalaman_keyboard / 2,
               0.1f, 0.1f, 0.1f); // Warna dasar keyboard

    // Layar (tegak di sisi belakang keyboard, menghadap -Z)
    basic_cube(x - lebar / 2, x + lebar / 2,
               y + ketebalan, y + ketebalan + tinggi_layar,
               z - kedalaman_keyboard / 2 - 0.5f, z - kedalaman_keyboard / 2,
               0.0f, 0.0f, 0.0f); // Frame layar

    // Tampilan layar (CYAN)
    basic_cube(x - lebar / 2 + 0.5f, x + lebar / 2 - 0.5f,
               y + ketebalan + 0.5f, y + ketebalan + tinggi_layar - 0.5f,
               z - kedalaman_keyboard / 2 - 0.45f, z - kedalaman_keyboard / 2.5 - 0.15f,
               0.0f, 1.0f, 1.0f); // Cyan terang
}

void buat_tv_dinding(float x, float y, float z) {
    float lebar = 24.0f;
    float tinggi = 14.0f;
    float ketebalan = 0.8f;

    // Frame TV
    basic_cube(x - ketebalan / 2, x + ketebalan / 2,
               y, y + tinggi,
               z - lebar / 2, z + lebar / 2,
               0.05f, 0.05f, 0.05f); // Warna frame hitam

    // Layar TV
    basic_cube(x - ketebalan / 2 + 0.1f, x + ketebalan / 2 - 0.1f,
               y + 0.5f, y + tinggi - 0.5f,
               z - lebar / 2 + 0.5f, z + lebar / 2 - 0.5f,
               0.0f, 0.0f, 0.0f); // Warna layar mati (gelap)
}

void buat_kota_mini(float x, float y, float z) {
    float permukaan_y = y + 2.0f; // permukaan meja
    float alas_tinggi = 0.2f;

    // ALAS miniatur hijau (menutupi seluruh area meja: 20 × 30)
    basic_cube(x - 10.0f, x + 10.0f,
               permukaan_y, permukaan_y + alas_tinggi,
               z - 15.0f, z + 15.0f,
               0.1f, 0.5f, 0.1f); // Rumput hijau

    // BLOK GEDUNG KECIL (12+ bangunan acak)
    basic_cube(x - 8.5f, x - 7.0f, permukaan_y + alas_tinggi, permukaan_y + 5.5f,
               z - 13.0f, z - 11.5f, 0.6f, 0.4f, 0.4f);
    basic_cube(x - 6.0f, x - 4.0f, permukaan_y + alas_tinggi, permukaan_y + 6.0f,
               z - 14.0f, z - 12.5f, 0.4f, 0.6f, 0.7f);
    basic_cube(x - 2.5f, x - 1.0f, permukaan_y + alas_tinggi, permukaan_y + 4.5f,
               z - 13.5f, z - 11.0f, 0.7f, 0.7f, 0.7f);
    basic_cube(x + 1.0f, x + 3.5f, permukaan_y + alas_tinggi, permukaan_y + 7.5f,
               z - 14.0f, z - 12.0f, 0.4f, 0.7f, 1.0f);
    basic_cube(x + 5.5f, x + 7.5f, permukaan_y + alas_tinggi, permukaan_y + 6.5f,
               z - 13.0f, z - 11.0f, 0.9f, 0.6f, 0.4f);

    // BLOK TENGAH (apartemen, taman, dan jalan)
    basic_cube(x - 0.5f, x + 0.5f, permukaan_y + alas_tinggi, permukaan_y + 5.0f,
               z - 0.5f, z + 0.5f, 0.9f, 0.9f, 0.9f); // apartemen tengah

    // Taman kota (kiri)
    basic_cube(x - 9.0f, x - 6.0f, permukaan_y + alas_tinggi, permukaan_y + 0.5f,
               z - 2.0f, z + 3.0f, 0.0f, 0.6f, 0.2f);

    // BLOK DEPAN (arah pengunjung)
    basic_cube(x - 4.0f, x - 1.0f, permukaan_y + alas_tinggi, permukaan_y + 5.5f,
               z + 10.0f, z + 13.5f, 0.6f, 0.7f, 0.8f);
    basic_cube(x + 2.0f, x + 5.0f, permukaan_y + alas_tinggi, permukaan_y + 4.0f,
               z + 9.5f, z + 12.5f, 1.0f, 0.7f, 0.2f);

    // Monumen depan tengah
    basic_cube(x - 0.8f, x + 0.8f, permukaan_y + alas_tinggi, permukaan_y + 6.0f,
               z + 13.0f, z + 14.0f, 1.0f, 1.0f, 0.0f);

    // Jalan horizontal
    basic_cube(x - 9.0f, x + 9.0f, permukaan_y + alas_tinggi, permukaan_y + alas_tinggi + 0.1f,
               z, z + 1.0f, 0.2f, 0.2f, 0.2f);
    // Jalan vertikal
    basic_cube(x - 0.5f, x + 0.5f, permukaan_y + alas_tinggi, permukaan_y + alas_tinggi + 0.1f,
               z - 12.0f, z + 12.0f, 0.2f, 0.2f, 0.2f);
}

void buat_vas_besar(float x, float y, float z) {
    float tinggi = 20.0f;
    float diameter = 10.0f;
    float ketebalan_dinding = 1.0f;

    // Alas vas (padat)
    basic_cube(x - diameter / 2, x + diameter / 2,
               y, y + 2.0f,
               z - diameter / 2, z + diameter / 2,
               0.3f, 0.3f, 0.3f); // Abu tua

    // Badan utama (silinder persegi padat)
    basic_cube(x - diameter / 2 + ketebalan_dinding, x + diameter / 2 - ketebalan_dinding,
               y + 2.0f, y + tinggi - 3.0f,
               z - diameter / 2 + ketebalan_dinding, z + diameter / 2 - ketebalan_dinding,
               0.8f, 0.85f, 0.9f); // Abu kebiruan terang

    // Leher vas
    basic_cube(x - diameter / 4, x + diameter / 4,
               y + tinggi - 3.0f, y + tinggi - 1.0f,
               z - diameter / 4, z + diameter / 4,
               0.7f, 0.7f, 0.75f);

    // Bibir vas melebar
    basic_cube(x - diameter / 2, x + diameter / 2,
               y + tinggi - 1.0f, y + tinggi,
               z - diameter / 2, z + diameter / 2,
               0.5f, 0.5f, 0.55f);
}

void buat_frame_gambar(float x, float y, float z) {
    float lebar = 24.0f;
    float tinggi = 12.0f;
    float ketebalan = 0.5f;

    // Frame luar (kayu gelap)
    basic_cube(x - ketebalan / 2, x + ketebalan / 2,
               y, y + tinggi,
               z - lebar / 2, z + lebar / 2,
               0.3f, 0.2f, 0.1f);

    // Lapisan 1 - Langit biru (paling belakang)
    basic_cube(x - 0.8f, x - 0.8f,
               y + 0.5f, y + tinggi - 0.5f,
               z - lebar / 2 + 0.5f, z + lebar / 2 - 0.5f,
               0.4f, 0.7f, 1.0f);

    // Lapisan 2 - Gunung belakang kiri
    basic_cube(x - 1.0f, x - 0.6f,
               y + 3.0f, y + 8.5f,
               z - 11.0f, z - 5.0f,
               0.5f, 0.3f, 0.6f);

    // Lapisan 3 - Gunung belakang kanan
    basic_cube(x - 1.0f, x - 0.4f,
               y + 2.0f, y + 7.5f,
               z + 6.0f, z + 11.0f,
               0.6f, 0.5f, 0.6f);

    // Lapisan 4 - Gunung tengah abu
    basic_cube(x - 1.1f, x - 0.2f,
               y + 0.5f, y + 6.0f,
               z - 7.0f, z + 7.0f,
               0.4f, 0.4f, 0.4f);

    // Lapisan 5 - Salju puncak
    basic_cube(x - 1.2f, x,
               y + 5.5f, y + 6.2f,
               z - 3.0f, z + 2.0f,
               1.0f, 1.0f, 1.0f);
}

void buat_ac_dinding(float x, float y, float z) {
    float lebar = 16.0f;
    float tinggi = 6.0f;
    float kedalaman = 4.0f;

    // Body utama AC
    basic_cube(x - lebar / 2, x + lebar / 2,
               y, y + tinggi,
               z - kedalaman, z,
               0.9f, 0.9f, 0.95f); // Warna putih keabu-abuan

    // Grill depan (garis-garis hitam horizontal)
    for (int i = 0; i < 4; ++i) {
        float y_start = y + 1.0f + i * 1.0f;
        basic_cube(x - lebar / 2 + 0.5f, x + lebar / 2 - 0.5f,
                   y_start, y_start + 0.3f,
                   z - 0.5f, z - 0.2f,
                   0.2f, 0.2f, 0.2f); // Grill hitam
    }

    // Tampilan kecil digital AC
    basic_cube(x + lebar / 2 - 2.0f, x + lebar / 2 - 1.0f,
               y + 4.0f, y + 5.0f,
               z - 0.5f, z - 0.1f,
               0.1f, 0.3f, 1.0f); // Biru digital
}

void buat_tumpukan_buku(float x, float y, float z) {
    float tebal_buku = 1.2f;
    float panjang = 9.0f;
    float lebar = 6.0f;

    // Buku 1 (paling bawah)
    basic_cube(x - panjang / 2, x + panjang / 2,
               y, y + tebal_buku,
               z - lebar / 2, z + lebar / 2,
               0.8f, 0.2f, 0.2f); // Merah

    // Buku 2 (biru, geser sedikit)
    basic_cube(x - panjang / 2 + 0.3f, x + panjang / 2 + 0.3f,
               y + tebal_buku, y + 2 * tebal_buku,
               z - lebar / 2 - 0.2f, z + lebar / 2 - 0.2f,
               0.2f, 0.4f, 0.8f);

    // Buku 3 (kuning, miring ke kanan)
    basic_cube(x - panjang / 2 - 0.2f, x + panjang / 2 - 0.2f,
               y + 2 * tebal_buku, y + 3 * tebal_buku,
               z - lebar / 2 + 0.3f, z + lebar / 2 + 0.3f,
               1.0f, 0.9f, 0.2f);

    // Buku 4 (hijau, paling atas)
    basic_cube(x - panjang / 2 + 0.5f, x + panjang / 2 + 0.5f,
               y + 3 * tebal_buku, y + 4 * tebal_buku,
               z - lebar / 2 - 0.4f, z + lebar / 2 - 0.4f,
               0.1f, 0.6f, 0.3f);
}

void buat_lampu_plafon(float x, float y, float z) {
    float radius = 5.0f;
    float tinggi = 2.0f;
    float kedalaman_bohlam = 1.0f;

    // Casing luar (bodi pelindung menonjol dari plafon)
    basic_cube(x - radius, x + radius,
               y - tinggi, y,
               z - radius, z + radius,
               0.7f, 0.7f, 0.7f); // Abu terang

    // Reflektor bohlam (cekung sedikit di tengah)
    basic_cube(x - radius + 1.0f, x + radius - 1.0f,
               y - tinggi - kedalaman_bohlam, y - tinggi,
               z - radius + 1.0f, z + radius - 1.0f,
               0.2f, 0.2f, 0.2f); // Abu gelap/hitam

    // Bohlam (bola dalam reflektor)
    basic_cube(x - 1.0f, x + 1.0f,
               y - tinggi - kedalaman_bohlam - 0.5f, y - tinggi - kedalaman_bohlam + 0.5f,
               z - 1.0f, z + 1.0f,
               1.0f, 1.0f, 0.6f); // Kuning terang, kesan menyala
}

void buat_frame_kecil(float x, float y, float z, int isi) {
    float lebar = 8.0f;
    float tinggi = 6.0f;
    float tebal = 0.3f;

    // Frame luar (kayu gelap kecil)
    basic_cube(x - tebal / 2, x + tebal / 2,
               y, y + tinggi,
               z - lebar / 2, z + lebar / 2,
               0.3f, 0.2f, 0.1f);

    // Isi gambar (bervariasi sesuai 'isi')
    if (isi == 1) {
        // Isi: Gradasi oranye
        basic_cube(x - 0.2f, x + 0.2f,
                   y + 0.5f, y + tinggi - 0.5f,
                   z - lebar / 2 + 0.5f, z + lebar / 2 - 0.5f,
                   1.0f, 0.6f, 0.2f);
    } else if (isi == 2) {
        // Isi: Lingkaran abstrak (representasi)
        basic_cube(x - 0.2f, x + 0.2f,
                   y + 2.0f, y + 4.0f,
                   z - 2.0f, z + 2.0f,
                   0.2f, 0.7f, 1.0f);
    } else if (isi == 3) {
        // Isi: Garis miring modern
        basic_cube(x - 0.2f, x + 0.2f,
                   y + 1.0f, y + 5.0f,
                   z - 3.0f, z - 1.0f,
                   0.4f, 1.0f, 0.6f);
    }
}

void buat_meja_panjang(float x, float y, float z) {
    float panjang = 20.0f;
    float lebar = 80.0f;
    float tinggi_kaki = 16.0f;
    float ketebalan_atas = 2.0f;
    float ukuran_kaki = 2.0f;

    // Alas meja atas (kayu atau permukaan putih)
    basic_cube(x - panjang / 2, x + panjang / 2,
               y, y + ketebalan_atas,
               z - lebar / 2, z + lebar / 2,
               0.6, 0.3, 0.1); // putih terang

    // Kaki kiri depan
    basic_cube(x - panjang / 2 + 1.0f, x - panjang / 2 + 1.0f + ukuran_kaki,
               y - tinggi_kaki, y,
               z - lebar / 2 + 1.0f, z - lebar / 2 + 1.0f + ukuran_kaki,
               0.3f, 0.3f, 0.3f);

    // Kaki kanan depan
    basic_cube(x + panjang / 2 - 1.0f - ukuran_kaki, x + panjang / 2 - 1.0f,
               y - tinggi_kaki, y,
               z - lebar / 2 + 1.0f, z - lebar / 2 + 1.0f + ukuran_kaki,
               0.3f, 0.3f, 0.3f);

    // Kaki kiri belakang
    basic_cube(x - panjang / 2 + 1.0f, x - panjang / 2 + 1.0f + ukuran_kaki,
               y - tinggi_kaki, y,
               z + lebar / 2 - 1.0f - ukuran_kaki, z + lebar / 2 - 1.0f,
               0.3f, 0.3f, 0.3f);

    // Kaki kanan belakang
    basic_cube(x + panjang / 2 - 1.0f - ukuran_kaki, x + panjang / 2 - 1.0f,
               y - tinggi_kaki, y,
               z + lebar / 2 - 1.0f - ukuran_kaki, z + lebar / 2 - 1.0f,
               0.3f, 0.3f, 0.3f);
}

void buat_piring_makanan(float x, float y, float z) {
    // Piring datar
    basic_cube(x - 4.0f, x + 4.0f,
               y, y + 0.4f,
               z - 4.0f, z + 4.0f,
               1.0f, 1.0f, 0.95f); // Putih keramik

    // Makanan bulat di tengah piring (nasi/gudeg misalnya)
    basic_cube(x - 2.0f, x + 2.0f,
               y + 0.4f, y + 1.8f,
               z - 2.0f, z + 2.0f,
               0.9f, 0.6f, 0.3f); // Coklat hangat
}

void buat_mangkok_makanan(float x, float y, float z) {
    // Mangkok
    basic_cube(x - 2.5f, x + 2.5f,
               y, y + 2.5f,
               z - 2.5f, z + 2.5f,
               0.9f, 0.9f, 0.9f); // Abu terang

    // Isi makanan (sup/kari)
    basic_cube(x - 2.0f, x + 2.0f,
               y + 2.0f, y + 2.8f,
               z - 2.0f, z + 2.0f,
               1.0f, 0.8f, 0.4f); // Kuning keemasan
}

void buat_pot_tanaman(float x, float y, float z) {
    // Pot
    basic_cube(x - 2.0f, x + 2.0f,
               y, y + 2.0f,
               z - 2.0f, z + 2.0f,
               0.5f, 0.25f, 0.15f); // Tanah liat

    // Tanaman hijau tinggi
    basic_cube(x - 0.5f, x + 0.5f,
               y + 2.0f, y + 6.0f,
               z - 0.5f, z + 0.5f,
               0.1f, 0.8f, 0.1f);
}

void buat_kapal_perang_miniatur(float x, float y, float z) {
    float panjang = 20.0f; // sesuai dengan alas (595–615)
    float lebar = 6.0f;
    float tinggi_dasar = 1.5f;
    float tinggi_total = 5.0f;

    // Badan kapal (dek utama)
    basic_cube(x - panjang / 2, x + panjang / 2,
               y, y + tinggi_dasar,
               z - lebar / 2, z + lebar / 2,
               0.4f, 0.4f, 0.5f); // Abu besi

    // Menara tengah (command tower)
    basic_cube(x - 1.5f, x + 1.5f,
               y + tinggi_dasar, y + tinggi_dasar + 2.5f,
               z - 1.5f, z + 1.5f,
               0.3f, 0.3f, 0.3f);

    // Radar atau tiang di atas menara
    basic_cube(x - 0.2f, x + 0.2f,
               y + tinggi_dasar + 2.5f, y + tinggi_dasar + 4.5f,
               z - 0.2f, z + 0.2f,
               0.2f, 0.2f, 0.2f);

    // Meriam depan
    basic_cube(x - 0.5f, x + 0.5f,
               y + tinggi_dasar, y + tinggi_dasar + 1.0f,
               z - lebar / 2 - 1.0f, z - lebar / 2,
               0.2f, 0.2f, 0.2f);

    // Meriam belakang
    basic_cube(x - 0.5f, x + 0.5f,
               y + tinggi_dasar, y + tinggi_dasar + 1.0f,
               z + lebar / 2, z + lebar / 2 + 1.0f,
               0.2f, 0.2f, 0.2f);

    // Haluan depan (runcing sedikit)
    basic_cube(x - 1.0f, x + 1.0f,
               y, y + 1.0f,
               z - lebar / 2 - 2.0f, z - lebar / 2,
               0.4f, 0.4f, 0.5f);
}

void buat_tank_perang_miniatur(float x, float y, float z) {
    float panjang = 14.0f;
    float lebar = 8.0f;
    float tinggi_badan = 2.5f;

    // Badan utama tank
    basic_cube(x - panjang / 2, x + panjang / 2,
               y, y + tinggi_badan,
               z - lebar / 2, z + lebar / 2,
               0.25f, 0.35f, 0.25f); // Hijau militer

    // Menara atas (turret)
    basic_cube(x - 2.0f, x + 2.0f,
               y + tinggi_badan, y + tinggi_badan + 2.0f,
               z - 2.0f, z + 2.0f,
               0.2f, 0.3f, 0.2f); // Lebih gelap

    // Meriam utama
    basic_cube(x - 0.3f, x + 0.3f,
               y + tinggi_badan + 1.0f, y + tinggi_badan + 1.4f,
               z + 2.0f, z + 7.0f,
               0.1f, 0.1f, 0.1f); // Abu gelap

    // Roda kiri
    basic_cube(x - panjang / 2 + 0.7f, x + panjang / 2 - 0.7f,
               y - 0.7f, y,
               z - lebar / 2 - 0.3f, z - lebar / 2 + 0.3f,
               0.1f, 0.1f, 0.1f);

    // Roda kanan
    basic_cube(x - panjang / 2 + 0.7f, x + panjang / 2 - 0.7f,
               y - 0.7f, y,
               z + lebar / 2 - 0.3f, z + lebar / 2 + 0.3f,
               0.1f, 0.1f, 0.1f);
}

void buat_cangkir_kopi_biasa(float x, float y, float z) {
    float radius_cup = 2.5f;
    float tinggi_cup = 3.0f;

    // Badan cangkir (silinder putih)
    basic_cube(x - radius_cup, x + radius_cup,
               y, y + tinggi_cup,
               z - radius_cup, z + radius_cup,
               1.0f, 1.0f, 1.0f);

    // Bibir atas cangkir (melingkar sedikit lebih besar)
    basic_cube(x - radius_cup - 0.1f, x + radius_cup + 0.1f,
               y + tinggi_cup, y + tinggi_cup + 0.1f,
               z - radius_cup - 0.1f, z + radius_cup + 0.1f,
               0.3f, 0.15f, 0.05f);

    // Kopi di dalam (coklat gelap, sedikit ke dalam)
    basic_cube(x - radius_cup + 0.3f, x + radius_cup - 0.3f,
               y + tinggi_cup - 0.4f, y + tinggi_cup - 0.1f,
               z - radius_cup + 0.3f, z + radius_cup - 0.3f,
               0.3f, 0.15f, 0.05f); // Warna kopi

    // Gagang cangkir (kanan luar)
    basic_cube(x + radius_cup + 0.2f, x + radius_cup + 0.8f,
               y + 1.0f, y + 2.5f,
               z - 0.4f, z + 0.4f,
               1.0f, 1.0f, 1.0f);

    // Sambungan gagang atas
    basic_cube(x + radius_cup, x + radius_cup + 0.2f,
               y + 2.2f, y + 2.5f,
               z - 0.4f, z + 0.4f,
               1.0f, 1.0f, 1.0f);

    // Sambungan gagang bawah
    basic_cube(x + radius_cup, x + radius_cup + 0.2f,
               y + 1.0f, y + 1.3f,
               z - 0.4f, z + 0.4f,
               1.0f, 1.0f, 1.0f);
}

void masjid() {
    glPushMatrix();

    glTranslatef(900.0f, 0.0f, 1300.0f);
    glRotatef(-90.0f, 0.0f, 1.0f, 0.0f);

    // Dinding utama
    glColor3f(0.4, 0.9, 0.7);
    glBegin(GL_QUADS);
        // Depan
        glVertex3f(-150, -500, 150);
        glVertex3f(150, -500, 150);
        glVertex3f(150, -300, 150);
        glVertex3f(-150, -300, 150);
        // Kanan
        glVertex3f(150, -500, 150);
        glVertex3f(150, -500, -150);
        glVertex3f(150, -300, -150);
        glVertex3f(150, -300, 150);
        // Belakang
        glVertex3f(150, -500, -150);
        glVertex3f(-150, -500, -150);
        glVertex3f(-150, -300, -150);
        glVertex3f(150, -300, -150);
        // Kiri
        glVertex3f(-150, -500, -150);
        glVertex3f(-150, -500, 150);
        glVertex3f(-150, -300, 150);
        glVertex3f(-150, -300, -150);
    glEnd();

    // Atap datar
    glColor3f(0.4, 0.2, 0.1);
    glBegin(GL_QUADS);
        glVertex3f(-160, -300, 160);
        glVertex3f(160, -300, 160);
        glVertex3f(160, -280, 160);
        glVertex3f(-160, -280, 160);

        glVertex3f(-160, -300, -160);
        glVertex3f(160, -300, -160);
        glVertex3f(160, -280, -160);
        glVertex3f(-160, -280, -160);

        glVertex3f(-160, -280, -160);
        glVertex3f(-160, -280, 160);
        glVertex3f(160, -280, 160);
        glVertex3f(160, -280, -160);
    glEnd();

    // Pintu depan
    glColor3f(0.2, 0.1, 0.05);
    glBegin(GL_QUADS);
        glVertex3f(-30, -500, 151.1);
        glVertex3f(30, -500, 151.1);
        glVertex3f(30, -350, 151.1);
        glVertex3f(-30, -350, 151.1);
    glEnd();

    // Jendela lancip (3 buah)
    for (int i = -90; i <= 90; i += 90) {
        // Segitiga atas jendela
        glColor3f(0.2, 0.4, 0.6);
        glBegin(GL_TRIANGLES);
            glVertex3f(i, -350, 151.2);
            glVertex3f(i + 30, -320, 151.2);
            glVertex3f(i + 60, -350, 151.2);
        glEnd();

        // Bawah jendela
        glColor3f(0.6, 0.8, 0.9); // warna kaca
        glBegin(GL_QUADS);
            glVertex3f(i, -500, 151.2);
            glVertex3f(i + 60, -500, 151.2);
            glVertex3f(i + 60, -350, 151.2);
            glVertex3f(i, -350, 151.2);
        glEnd();
    }

    glPopMatrix();
}

void kios() {
    glPushMatrix();
    glTranslated(700.0, 0.0, 1800.0); // Tambah jarak antar bangunan
    glRotatef(180.0f, 0.0f, 1.0f, 0.0f);

    float lebar = 600;
    float tinggi = 150;
    float kedalaman = 100;
    float atapTinggi = 20;
    float teras = 100;

    // Dinding
    glColor3f(1.0, 0.95, 0.75);
    glBegin(GL_QUADS);
        // Depan
        glVertex3f(-lebar / 2, -500, kedalaman);
        glVertex3f(lebar / 2, -500, kedalaman);
        glVertex3f(lebar / 2, -500 + tinggi, kedalaman);
        glVertex3f(-lebar / 2, -500 + tinggi, kedalaman);
        // Belakang
        glVertex3f(-lebar / 2, -500, 0);
        glVertex3f(lebar / 2, -500, 0);
        glVertex3f(lebar / 2, -500 + tinggi, 0);
        glVertex3f(-lebar / 2, -500 + tinggi, 0);
        // Samping kiri
        glVertex3f(-lebar / 2, -500, 0);
        glVertex3f(-lebar / 2, -500, kedalaman);
        glVertex3f(-lebar / 2, -500 + tinggi, kedalaman);
        glVertex3f(-lebar / 2, -500 + tinggi, 0);
        // Samping kanan
        glVertex3f(lebar / 2, -500, 0);
        glVertex3f(lebar / 2, -500, kedalaman);
        glVertex3f(lebar / 2, -500 + tinggi, kedalaman);
        glVertex3f(lebar / 2, -500 + tinggi, 0);
    glEnd();

    // Atap
    glColor3f(0.9, 0.4, 0.5);
    glBegin(GL_QUADS);
        glVertex3f(-lebar / 2 - 10, -500 + tinggi, -10);
        glVertex3f(lebar / 2 + 10, -500 + tinggi, -10);
        glVertex3f(lebar / 2 + 10, -500 + tinggi + atapTinggi, -10);
        glVertex3f(-lebar / 2 - 10, -500 + tinggi + atapTinggi, -10);
        // Belakang atap
        glVertex3f(-lebar / 2 - 10, -500 + tinggi, kedalaman + teras);
        glVertex3f(lebar / 2 + 10, -500 + tinggi, kedalaman + teras);
        glVertex3f(lebar / 2 + 10, -500 + tinggi + atapTinggi, kedalaman + teras);
        glVertex3f(-lebar / 2 - 10, -500 + tinggi + atapTinggi, kedalaman + teras);
        // Atap atas
        glVertex3f(-lebar / 2 - 10, -500 + tinggi + atapTinggi, -10);
        glVertex3f(-lebar / 2 - 10, -500 + tinggi + atapTinggi, kedalaman + teras);
        glVertex3f(lebar / 2 + 10, -500 + tinggi + atapTinggi, kedalaman + teras);
        glVertex3f(lebar / 2 + 10, -500 + tinggi + atapTinggi, -10);
    glEnd();

    // Teras
    glColor3f(0.7, 0.7, 0.7);
    glBegin(GL_QUADS);
        glVertex3f(-lebar / 2, -500, kedalaman + teras);
        glVertex3f(lebar / 2, -500, kedalaman + teras);
        glVertex3f(lebar / 2, -500, kedalaman);
        glVertex3f(-lebar / 2, -500, kedalaman);
    glEnd();

    // Pilar
    glColor3f(0.4, 0.4, 0.4);
    for (int i = -250; i <= 250; i += 160) {
        glBegin(GL_QUADS);
            glVertex3f(i - 10, -500, kedalaman + teras - 5);
            glVertex3f(i + 10, -500, kedalaman + teras - 5);
            glVertex3f(i + 10, -500 + tinggi + atapTinggi, kedalaman + teras - 5);
            glVertex3f(i - 10, -500 + tinggi + atapTinggi, kedalaman + teras - 5);
        glEnd();
    }

    // Jendela
    glColor3f(0.6, 0.8, 0.9);
    for (int i = -250; i <= 250; i += 125) {
        glBegin(GL_QUADS);
            glVertex3f(i, -450, kedalaman + 0.1);
            glVertex3f(i + 60, -450, kedalaman + 0.1);
            glVertex3f(i + 60, -370, kedalaman + 0.1);
            glVertex3f(i, -370, kedalaman + 0.1);
        glEnd();
    }

    // Pintu kios tengah
    glColor3f(0.3, 0.2, 0.1);
    glBegin(GL_QUADS);
        glVertex3f(-40, -500, kedalaman + 0.2);
        glVertex3f(40, -500, kedalaman + 0.2);
        glVertex3f(40, -350, kedalaman + 0.2);
        glVertex3f(-40, -350, kedalaman + 0.2);
    glEnd();

    glPopMatrix();
}

void jalanMuseum() {
    //Alas Jalan Ke Museum
    basic_cube(360.0, 480.0, -500.0, -499.5, 400.0, 1000.0, 0.9, 0.9, 0.9); //Jalan 1
    basic_cube(480.0, 800.0, -500.0, -499.5, 730.0, 770.0, 0.9, 0.9, 0.9); //Jalan 2
    basic_cube(635.0, 675.0, -500.0, -499.5, 770.0, 820.0, 0.9, 0.9, 0.9); //Jalan 2 kanan
    basic_cube(480.0, 800.0, -500.0, -499.5, 630.0, 670.0, 0.9, 0.9, 0.9); //Jalan 3
    basic_cube(635.0, 675.0, -500.0, -499.5, 575.0, 635.0, 0.9, 0.9, 0.9); //Jalan 3 kiri
    basic_cube(760.0, 800.0, -500.0, -499.5, 670.0, 730.0, 0.9, 0.9, 0.9); //Jalan Penutup
    basic_cube(460.0, 480.0, -500.0, -499.0, 450.0, 510.0, 0.5, 0.5, 0.5); //Alas Bangku Taman 1
    basic_cube(460.0, 480.0, -500.0, -499.0, 890.0, 950.0, 0.5, 0.5, 0.5); //Alas Bangku Taman 2
}

void patung_pahlawan() {
    // Warna kebauan
    glPushMatrix();
    float abuR = 0.5f, abuG = 0.5f, abuB = 0.5f;

    float patung_center_x = 725.0f; // Rata-rata dari 700 dan 750
    float patung_center_y = -400.0f; // Kira-kira di bagian tengah-atas patung
    float patung_center_z = 700.0f; // Rata-rata dari 675 dan 725

    basic_cube(700.0, 750.0, -500.0, -490.5, 675.0, 725.0, 1.0, 1.0, 1.0); //Alas Patung 1
    basic_cube(710.0, 740.0, -490.0, -475.5, 685.0, 715.0, 0.95, 0.95, 0.95); //Alas Patung 2
    basic_cube(700.0, 750.0, -475.0, -465.5, 675.0, 725.0, 1.0, 1.0, 1.0); //Alas Patung 3

    glTranslatef(patung_center_x, patung_center_y, patung_center_z);
    glRotatef(patungRotationAngle, 0.0f, 1.0f, 0.0f); // Rotasi di sekitar sumbu Y
    glTranslatef(-patung_center_x, -patung_center_y, -patung_center_z);

    // Kaki
    basic_cube(720.0, 727.0, -465.5, -455.5, 690.0, 695.0, abuR, abuG, abuB); // Kaki kiri
    basic_cube(730.0, 737.0, -465.5, -455.5, 690.0, 695.0, abuR, abuG, abuB); // Kaki kanan
    // Badan
    basic_cube(718.0, 739.0, -455.5, -440.5, 688.0, 697.0, abuR, abuG, abuB); // Badan
    // Tangan
    basic_cube(713.0, 718.0, -452.0, -440.5, 688.0, 697.0, abuR, abuG, abuB); // Tangan kiri
    basic_cube(739.0, 744.0, -452.0, -440.5, 688.0, 697.0, abuR, abuG, abuB); // Tangan kanan
    // Leher
    basic_cube(725.0, 732.0, -440.5, -437.5, 691.0, 694.0, abuR, abuG, abuB); // Leher
    // Kepala
    basic_cube(722.0, 735.0, -437.5, -430.5, 689.0, 696.0, abuR, abuG, abuB); // Kepala
    // Topi
    basic_cube(720.0, 737.0, -430.5, -428.0, 688.0, 697.0, abuR * 0.8, abuG * 0.8, abuB * 0.8); // Topi datar
    // Pose tangan
    basic_cube(739.0, 742.0, -445.0, -435.0, 697.0, 702.0, abuR, abuG, abuB); // Tangan kanan diagonal ke atas
    glPopMatrix();
}

void drawBench(float x, float y, float z) {
    float panjang = 20.0f;
    float lebar = 60.0f;
    float tinggi_duduk = 1.5f;
    float tebal_atas = 1.2f;
    float tinggi_kaki = 9.0f;
    float lebar_kaki = 2.0f;

    // Dudukan bangku
    basic_cube(x - panjang / 2, x + panjang / 2,
               y, y + tebal_atas,
               z - lebar / 2, z + lebar / 2,
               0.6f, 0.4f, 0.2f); // Coklat kayu

    // Kaki kiri depan
    basic_cube(x - panjang / 2 + 1.0f, x - panjang / 2 + 1.0f + lebar_kaki,
               y - tinggi_kaki, y,
               z - lebar / 2 + 1.0f, z - lebar / 2 + 1.0f + lebar_kaki,
               0.3f, 0.3f, 0.3f); // Abu gelap

    // Kaki kanan depan
    basic_cube(x + panjang / 2 - 1.0f - lebar_kaki, x + panjang / 2 - 1.0f,
               y - tinggi_kaki, y,
               z - lebar / 2 + 1.0f, z - lebar / 2 + 1.0f + lebar_kaki,
               0.3f, 0.3f, 0.3f);

    // Kaki kiri belakang
    basic_cube(x - panjang / 2 + 1.0f, x - panjang / 2 + 1.0f + lebar_kaki,
               y - tinggi_kaki, y,
               z + lebar / 2 - 1.0f - lebar_kaki, z + lebar / 2 - 1.0f,
               0.3f, 0.3f, 0.3f);

    // Kaki kanan belakang
    basic_cube(x + panjang / 2 - 1.0f - lebar_kaki, x + panjang / 2 - 1.0f,
               y - tinggi_kaki, y,
               z + lebar / 2 - 1.0f - lebar_kaki, z + lebar / 2 - 1.0f,
               0.3f, 0.3f, 0.3f);
}

void drawSimpleTree(float x, float z) {
    // --- Dimensi Pohon ---
    float trunk_height = 50.0f; // Tinggi batang pohon
    float trunk_width = 15.0f;  // Lebar/kedalaman batang pohon

    float leaves_base_size = 60.0f; // Ukuran dasar daun (terluas)
    float leaves_segment_height = 25.0f; // Tinggi setiap segmen daun
    int num_leaves_segments = 4;        // Jumlah segmen daun (untuk efek piramida)

    // --- Warna Pohon ---
    float trunk_color_r = 0.4f, trunk_color_g = 0.2f, trunk_color_b = 0.05f; // Coklat
    float leaves_color_r = 0.1f, leaves_color_g = 0.6f, leaves_color_b = 0.1f; // Hijau

    // --- Batang Pohon ---
    // Posisi X dan Z akan berada di pusat batang.
    // Y akan dimulai dari -500.0.
    basic_cube(x - trunk_width / 2, x + trunk_width / 2,
               -500.0f, -500.0f + trunk_height,
               z - trunk_width / 2, z + trunk_width / 2,
               trunk_color_r, trunk_color_g, trunk_color_b);

    // --- Daun (Berbentuk Piramida dengan 4 Tumpuk basic_cube) ---
    // Daun akan dimulai dari atas batang pohon.
    float current_y = -500.0f + trunk_height; // Y awal untuk segmen daun pertama

    for (int i = 0; i < num_leaves_segments; ++i) {
        // Ukuran daun akan mengecil di setiap segmen ke atas
        float current_leaves_size = leaves_base_size - (i * (leaves_base_size / num_leaves_segments));
        if (i == num_leaves_segments - 1) { // Paling atas lebih kecil
            current_leaves_size = leaves_base_size / (num_leaves_segments * 2);
        }

        // Gambar segmen daun
        basic_cube(x - current_leaves_size / 2, x + current_leaves_size / 2,
                   current_y, current_y + leaves_segment_height,
                   z - current_leaves_size / 2, z + current_leaves_size / 2,
                   leaves_color_r, leaves_color_g, leaves_color_b);

        current_y += leaves_segment_height; // Pindah ke Y selanjutnya untuk segmen berikutnya
    }
}

void drawStars() {
    glDisable(GL_LIGHTING); // Bintang tidak boleh dipengaruhi oleh pencahayaan
    glPointSize(2.0f);      // Ukuran bintang (bisa disesuaikan)
    glColor3f(1.0f, 1.0f, 1.0f); // Bintang berwarna putih

    glBegin(GL_POINTS); // Menggambar bintang sebagai titik
    for (int i = 0; i < 500; i++) {
        // Gambar bintang. Karena bintang seharusnya tidak bergerak relatif terhadap kamera
        // dalam hal rotasi (tetapi berpusat pada kamera), mereka digambar seperti skybox.
        // Posisi bintang harus sangat jauh agar terlihat tak terbatas.
        glVertex3f(starField[i][0], starField[i][1], starField[i][2]);
    }
    glEnd();

    glEnable(GL_LIGHTING); // Aktifkan kembali pencahayaan untuk objek scene lainnya
}

void drawMonument() {
    basic_cube(-150.0, -100.0, -500.0, -495.0, 350.0, 400.0, 1.0, 1.0, 1.0);
    basic_cube(-145.0, -105.0, -495.0, -480.0, 355.0, 395.0, 0.95, 0.95, 0.95);
}

void drawFlag() {
    custom_2d(50.0, -500.0, 1000.0, 190.0, -500.0, 1000.0, 190.0, -500.0, 400.0, 50.0, -500.0, 400.0, 0.85, 0.85, 0.85); // Alas
    basic_cube(70.0, 100.0, -500.0, -495.0, 685.0, 715.0, 0.9, 0.9, 0.9); // Landasan
    basic_cube(82.5, 87.5, -495.0, -350.0, 698.5, 702.5, 0.95, 0.95, 0.95); // Tiang
    basic_cube(85.0, 85.5, -360.0, -350.0, 660.0, 698.5, 1.0, 0.5, 0.75); // Bendera Merah
    basic_cube(85.0, 85.5, -370.0, -360.0, 660.0, 698.5, 1.0, 1.0, 1.0); // Bendera Merah
}

// Fungsi utama menggambar objek dan scene
void draw() {
    glPushMatrix();
    ground();
    street();
    bangunan_belakang();

    drawKapal();
    drawArtilery();
    drawPesawat();
    drawTank();

    treeBackgroundX();
    treeBackgroundZ();

    masjid();
    kios();
    drawMonument();
    drawBench(470.0f, -490.0f, 480.0f);//Bangku 1
    drawBench(470.0f, -490.0f, 920.0f);//Bangku 2
    drawFlag();
    patung_pahlawan();
    jalanMuseum();

    // Bikin Pohon
    drawSimpleTree(480.0, 1200.0);
    drawSimpleTree(580.0, 1100.0);
    drawSimpleTree(590.0, 1280.0);

    drawSimpleTree(700.0, -880.0);
    drawSimpleTree(800.0, -700.0);
    drawSimpleTree(500.0, -900.0);
    drawSimpleTree(850.0, -600.0);
    drawSimpleTree(632.0, -678.0);
    drawSimpleTree(653.0, -478.0);

    drawSimpleTree(-800.0, -880.0);
    drawSimpleTree(-750.0, -700.0);
    drawSimpleTree(-700.0, -900.0);
    drawSimpleTree(-650.0, -600.0);
    drawSimpleTree(-800.0, -678.0);
    drawSimpleTree(-633.0, -478.0);

    drawSimpleTree(-800.0, 1300.0);
    drawSimpleTree(-633.0, 1400.0);

    // Pagar Tank
    basic_cube(-890.0, -660.0, -500.0, -499.0, 260.0, 440.0, 0.6, 0.6, 0.6);//Alas
    buat_pagar_besi_mengelilingi(-890.0f, -660.0f, 260.0f, 440.0f, -499.0f);//Pagar
    basic_cube(-658.0, -657.0, -490.0, -480.0, 330.0, 370.0, 1.0, 0.96, 0.82);//Display Tank


    // Hiasan
    basic_cube(-350.0, 100.0, -500.0, -495.0, 150.0, 200.0, 1.0, 1.0, 1.0); // Alas
    basic_cube(-340.0, -300.0, -495.0, -475.0, 155.0, 195.0, 0.95, 0.95, 0.95); // Fondasi 1
    basic_cube(-220.0, -180.0, -495.0, -475.0, 155.0, 195.0, 0.95, 0.95, 0.95); // Fondasi 2
    basic_cube(-80.0, -40.0, -495.0, -475.0, 155.0, 195.0, 0.95, 0.95, 0.95); // Fondasi 3
    basic_cube(50.0, 90.0, -495.0, -475.0, 155.0, 195.0, 0.95, 0.95, 0.95); // Fondasi 4
    basic_cube(-335.0, -305.0, -475.0, -350.0, 160.0, 190.0, 0.90, 0.90, 0.90); // Tiang 1
    basic_cube(-215.0, -185.0, -475.0, -350.0, 160.0, 190.0, 0.90, 0.90, 0.90); // Tiang 2
    basic_cube(-75.0, -45.0, -475.0, -350.0, 160.0, 190.0, 0.90, 0.90, 0.90); // Tiang 3
    basic_cube(55.0, 85.0, -475.0, -350.0, 160.0, 190.0, 0.90, 0.90, 0.90); // Tiang 4
    basic_cube(-350.0, 100.0, -350.0, -345.0, 150.0, 200.0, 1.0, 1.0, 1.0); // Atap

    // Hanya gambar bintang jika mode malam
    if (!isDayMode) {
        drawStars(); // Gambar bintang-bintang
    }

    museum();
    museum_2();

    //===Ruangan Interior===
    //Meja Receptionist
    basic_cube(620.0, 675.0, -500.0, -470.0, 920.0, 922.0, 0.6, 0.3, 0.1); // Meja tinggi
    basic_cube(620.0, 675.0, -472.0, -470.0, 920.0, 935.0, 0.6, 0.3, 0.1); // Meja atas
    basic_cube(620.0, 622.0, -500.0, -470.0, 920.0, 935.0, 0.6, 0.3, 0.1); // Meja kiri
    basic_cube(625.0, 633.0, -470.0, -469.5, 923.0, 933.0, 1.0, 1.0, 1.0); // Kertas 1
    basic_cube(533.0, 545.0, -470.0, -467.5, 930.0, 943.0, 1.0, 0.96, 0.82); // Kertas 2
    basic_cube(533.0, 545.0, -470.0, -467.5, 897.0, 910.0, 1.0, 0.96, 0.82); // Kertas 3

    //Kursi
    buat_kursi(645.0, -475.0, 945.0); //Kursi 1
    buat_kursi(745.0, -475.0, 960.0); //Kursi 2

    //Rak Buku
    buat_rak_buku(700.0, -500.0, 930.0); // Rak Buku

    //Meja
    buat_meja(580.0, -470.0, 925.0); // Meja 1
    buat_meja(765.0, -470.0, 960.0); // Meja 2

    //Tumpukan Buku
    buat_tumpukan_buku(768.0, -468.0, 968.0);//Buku

    //Jam
    buat_jam_dinding(674.0, -435.0, 955.0);// Jam 1
    buat_jam_dinding(774.0, -420.0, 955.0);// Jam 2

    //Hiasan Meja Recept
    buat_laptop(647.5, -470.0, 927.5);// Laptop
    buat_cangkir_kopi_biasa(665.5f, -470.0f, 927.5f);//Kopi

    //Televisi
    buat_tv_dinding(674.0, -460.0, 955.0); //TV 1
    buat_tv_dinding(681.0, -460.0, 955.0); //TV 2

    //Kota
    buat_kota_mini(580.0, -470.0, 925.0);//Miniatur

    //Vas
    buat_vas_besar(765.0, -484.0, 890.0);// Vases

    //Frame Gambar
    buat_frame_gambar(774.0, -445.0, 955.0);//Frame Gambar

    //AC
    buat_ac_dinding(610.0, -410.0, 975.0);// AC 1
    buat_ac_dinding(700.0, -410.0, 975.0);// AC 2

    //Lampu
    buat_lampu_plafon(590.0, -400.0, 940.0);//Lampu 1
    buat_lampu_plafon(730.0, -400.0, 960.0);//Lampu 2
    buat_lampu_plafon(650.0, -400.0, 480.0);//Lampu 3
    buat_lampu_plafon(-125.0, -400.0, -375.0);//Lampu 4

    // Frame Foto Kecil
    buat_frame_kecil(774.0, -445.0, 890.0, 1);//1
    buat_frame_kecil(774.0, -455.0, 895.0, 2);//2
    buat_frame_kecil(774.0, -455.0, 885.0, 3);//3

    //Meja Panjang
    buat_meja_panjang(538.0, -470.0, 920.0); // Meja Panjang
    buat_piring_makanan(538.0, -468.0, 890.0);  // Kiri
    buat_mangkok_makanan(538.0, -468.0, 920.0); // Tengah
    buat_pot_tanaman(538.0, -468.0, 950.0);     // Kanan

    //Miniatur
    buat_kapal_perang_miniatur(605.0, -470.0, 880.0); //Kapal
    buat_tank_perang_miniatur(575.0, -470.0, 880.0); // Tank

    glPopMatrix(); // Kembalikan matriks asli

    // Cetak posisi geser untuk debug di konsol
    cout << "Geser X: " << x_geser << ", Y: " << y_geser << ", Z: " << z_geser << endl;
    glFlush(); // Pastikan perintah OpenGL dijalankan
}

// Fungsi display untuk rendering frame
void display() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); // Bersihkan buffer warna dan depth
    glLoadIdentity(); // Reset matriks model view

     // --- PENGATURAN CAHAYA SIANG/MALAM ---
    GLfloat light_position[4];
    GLfloat light_ambient[4];
    GLfloat light_diffuse[4];
    GLfloat light_specular[4];
    GLfloat background_color[4]; // Untuk mengubah warna latar belakang

    // Properti cahaya bohlam (ini akan tetap, hanya posisinya yang diatur nanti)
    GLfloat light1_color[] = { 0.2f, 0.2f, 0.2f, 1.0f };
    GLfloat light1_specular[] = { 0.5f, 0.5f, 0.4f, 1.0f };
    GLfloat light2_color[] = { 0.2f, 0.2f, 0.2f, 1.0f };
    GLfloat light2_specular[] = { 0.5f, 0.5f, 0.4f, 1.0f };
    GLfloat light3_color[] = { 0.2f, 0.2f, 0.2f, 1.0f };
    GLfloat light3_specular[] = { 0.5f, 0.5f, 0.4f, 1.0f };
    GLfloat light4_color[] = { 0.2f, 0.2f, 0.2f, 1.0f };
    GLfloat light4_specular[] = { 0.5f, 0.5f, 0.4f, 1.0f };

    if (isDayMode) {
        // SIANG HARI: Terang, ambient cerah, diffuse putih/kuning muda
        light_position[0] = 0.0f; light_position[1] = 0.0f; light_position[2] = 0.0f; light_position[3] = 0.0f;
        light_ambient[0]  = 0.8f; light_ambient[1]  = 0.8f; light_ambient[2]  = 0.8f; light_ambient[3]  = 1.0f; // Ambient sangat terang
        light_diffuse[0]  = 0.6f; light_diffuse[1]  = 0.6f; light_diffuse[2]  = 0.6f; light_diffuse[3]  = 1.0f;
        light_specular[0] = 0.5f; light_specular[1] = 0.5f; light_specular[2] = 0.5f; light_specular[3] = 1.0f;
        background_color[0] = 0.7f; background_color[1] = 0.8f; background_color[2] = 1.0f; background_color[3] = 1.0f; // Langit biru cerah

        glDisable(GL_LIGHT1);
        glDisable(GL_LIGHT2);
        glDisable(GL_LIGHT3);
        glDisable(GL_LIGHT4);
    } else {
        // MALAM HARI: Gelap, ambient sangat redup/biru, diffuse sangat rendah
        light_position[0] = 0.0f; light_position[1] = 0.5f; light_position[2] = 0.0f; light_position[3] = 0.0f; // Cahaya sangat redup dari atas
        light_ambient[0]  = 0.05f; light_ambient[1]  = 0.05f; light_ambient[2]  = 0.15f; light_ambient[3]  = 1.0f; // Ambient sangat redup, kebiruan
        light_diffuse[0]  = 0.1f; light_diffuse[1]  = 0.1f; light_diffuse[2]  = 0.1f; light_diffuse[3]  = 1.0f;
        light_specular[0] = 0.0f; light_specular[1] = 0.0f; light_specular[2] = 0.0f; light_specular[3] = 1.0f; // Tidak ada kilau di malam hari
        background_color[0] = 0.05f; background_color[1] = 0.05f; background_color[2] = 0.1f; background_color[3] = 1.0f; // Langit gelap kebiruan

        if (isBulbOn) {
            glEnable(GL_LIGHT1);
            // Atur properti warna bohlam (ambient = diffuse = specular agar cahaya terlihat dari dekat)
            glLightfv(GL_LIGHT1, GL_AMBIENT, light1_color);
            glLightfv(GL_LIGHT1, GL_DIFFUSE, light1_color);
            glLightfv(GL_LIGHT1, GL_SPECULAR, light1_specular);
            // Atur redaman cahaya (opsional, membuat cahaya memudar seiring jarak)
            glLightf(GL_LIGHT1, GL_CONSTANT_ATTENUATION, 0.1f);  // Redaman konstan
            glLightf(GL_LIGHT1, GL_LINEAR_ATTENUATION, 0.0001f); // Redaman linier (memudar lebih cepat)
            glLightf(GL_LIGHT1, GL_QUADRATIC_ATTENUATION, 0.00005f); // Redaman kuadratik (memudar lebih cepat lagi)

            glEnable(GL_LIGHT2);
            // Atur properti warna bohlam (ambient = diffuse = specular agar cahaya terlihat dari dekat)
            glLightfv(GL_LIGHT2, GL_AMBIENT, light2_color);
            glLightfv(GL_LIGHT2, GL_DIFFUSE, light2_color);
            glLightfv(GL_LIGHT2, GL_SPECULAR, light2_specular);
            // Atur redaman cahaya (opsional, membuat cahaya memudar seiring jarak)
            glLightf(GL_LIGHT2, GL_CONSTANT_ATTENUATION, 0.1f);  // Redaman konstan
            glLightf(GL_LIGHT2, GL_LINEAR_ATTENUATION, 0.0001f); // Redaman linier (memudar lebih cepat)
            glLightf(GL_LIGHT2, GL_QUADRATIC_ATTENUATION, 0.00005f); // Redaman kuadratik (memudar lebih cepat lagi)

            glEnable(GL_LIGHT3);
            // Atur properti warna bohlam (ambient = diffuse = specular agar cahaya terlihat dari dekat)
            glLightfv(GL_LIGHT3, GL_AMBIENT, light3_color);
            glLightfv(GL_LIGHT3, GL_DIFFUSE, light3_color);
            glLightfv(GL_LIGHT3, GL_SPECULAR, light3_specular);
            // Atur redaman cahaya (opsional, membuat cahaya memudar seiring jarak)
            glLightf(GL_LIGHT3, GL_CONSTANT_ATTENUATION, 0.1f);  // Redaman konstan
            glLightf(GL_LIGHT3, GL_LINEAR_ATTENUATION, 0.0001f); // Redaman linier (memudar lebih cepat)
            glLightf(GL_LIGHT3, GL_QUADRATIC_ATTENUATION, 0.00005f); // Redaman kuadratik (memudar lebih cepat lagi)

            glEnable(GL_LIGHT4);
            // Atur properti warna bohlam (ambient = diffuse = specular agar cahaya terlihat dari dekat)
            glLightfv(GL_LIGHT4, GL_AMBIENT, light4_color);
            glLightfv(GL_LIGHT4, GL_DIFFUSE, light4_color);
            glLightfv(GL_LIGHT4, GL_SPECULAR, light4_specular);
            // Atur redaman cahaya (opsional, membuat cahaya memudar seiring jarak)
            glLightf(GL_LIGHT4, GL_CONSTANT_ATTENUATION, 0.1f);  // Redaman konstan
            glLightf(GL_LIGHT4, GL_LINEAR_ATTENUATION, 0.0001f); // Redaman linier (memudar lebih cepat)
            glLightf(GL_LIGHT4, GL_QUADRATIC_ATTENUATION, 0.00005f); // Redaman kuadratik (memudar lebih cepat lagi)
        } else {
            glDisable(GL_LIGHT1);
            glDisable(GL_LIGHT2);
            glDisable(GL_LIGHT3);
            glDisable(GL_LIGHT4);
        }

    }

    glLightfv(GL_LIGHT0, GL_POSITION, light_position);
    glLightfv(GL_LIGHT0, GL_AMBIENT, light_ambient);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, light_diffuse);
    glLightfv(GL_LIGHT0, GL_SPECULAR, light_specular);

    // Set warna latar belakang
    glClearColor(background_color[0], background_color[1], background_color[2], background_color[3]);

    camera(); // Set posisi dan arah kamera

    GLfloat bulb_pos_1[] = { 590.0f, -403.5f, 940.0f, 1.0f };
    glLightfv(GL_LIGHT1, GL_POSITION, bulb_pos_1);
    GLfloat bulb_pos_2[] = { 730.0f, -403.5f, 960.0f, 1.0f };
    glLightfv(GL_LIGHT2, GL_POSITION, bulb_pos_2);
    GLfloat bulb_pos_3[] = { 650.0, -400.0, 480.0, 1.0f };
    glLightfv(GL_LIGHT3, GL_POSITION, bulb_pos_3);
    GLfloat bulb_pos_4[] = { -125.0, -400.0, -375.0, 1.0f };
    glLightfv(GL_LIGHT4, GL_POSITION, bulb_pos_4);


    draw(); // Gambar objek di scene
    glutSwapBuffers(); // Tukar buffer untuk double buffering (menghilangkan flicker)
}

// Fungsi reshape untuk handle perubahan ukuran window
void reshape(int w, int h) {
    glViewport(0, 0, w, h); // Tentukan viewport untuk seluruh window
    glMatrixMode(GL_PROJECTION); // Pindah ke matriks proyeksi
    glLoadIdentity(); // Reset matriks proyeksi
    gluPerspective(50, 16.0 / 9.0, 2, 10000); // Atur perspektif kamera
    glMatrixMode(GL_MODELVIEW); // Kembali ke matriks model view
}

// Fungsi timer untuk refresh layar secara berkala berdasarkan FPS
void timer(int) {
    glutPostRedisplay(); // Minta tampilan di-refresh
    glutWarpPointer(width / 2, height / 2); // Reset posisi pointer ke tengah window

    patungRotationAngle += 0.75f; // Tambahkan 0.5 derajat setiap frame (sesuaikan kecepatan)
    if (patungRotationAngle > 360.0f) {
        patungRotationAngle -= 360.0f; // Agar sudut tidak terus bertambah tak terbatas
    }

    // Perbarui posisi tank
    float tankSpeed = 5.0f; // Kecepatan gerakan tank, sesuaikan

    if (tankMotion.ForwardI) { // Maju pada positif X
        tankPosX += tankSpeed;
    }
    if (tankMotion.BackwardK) { // Mundur pada negatif X
        tankPosX -= tankSpeed;
    }
    if (tankMotion.ForwardL) { // Maju pada positif Z
        tankPosZ += tankSpeed;
    }
    if (tankMotion.BackwardJ) { // Mundur pada negatif Z
        tankPosZ -= tankSpeed;
    }

    glutTimerFunc(1000 / FPS, timer, 0); // Set timer ulang agar terus berjalan
}

// Fungsi untuk menangani pergerakan mouse tanpa klik
void passive_motion(int x, int y) {
    int dev_x, dev_y;
    dev_x = (width / 2) - x; // Deviasi posisi mouse horisontal dari tengah
    dev_y = (height / 2) - y; // Deviasi posisi mouse vertikal dari tengah
    yaw += (float)dev_x / 20.0; // Update rotasi yaw berdasarkan deviasi X
    pitch += (float)dev_y / 20.0; // Update rotasi pitch berdasarkan deviasi Y
}

// Fungsi pengaturan kamera berdasarkan input keyboard dan mouse (rotasi dan posisi)
void camera() {
    // Jika tombol W ditekan, kamera bergerak maju berdasarkan sudut yaw
    if (motion.Forward) {
        camX += cos((yaw + 90) * TO_RADIANS) * 2;
        camZ -= sin((yaw + 90) * TO_RADIANS) * 2;
    }
    // Jika tombol S ditekan, kamera bergerak mundur
    if (motion.Backward) {
        camX += cos((yaw + 90 + 180) * TO_RADIANS) * 2;
        camZ -= sin((yaw + 90 + 180) * TO_RADIANS) * 2;
    }
    // Jika tombol A ditekan, kamera bergerak ke kiri
    if (motion.Left) {
        camX += cos((yaw + 90 + 90) * TO_RADIANS) * 2;
        camZ -= sin((yaw + 90 + 90) * TO_RADIANS) * 2;
    }
    // Jika tombol D ditekan, kamera bergerak ke kanan
    if (motion.Right) {
        camX += cos((yaw + 90 - 90) * TO_RADIANS) * 2;
        camZ -= sin((yaw + 90 - 90) * TO_RADIANS) * 2;
    }
    // Jika tombol E ditekan, naikkan posisi vertikal kamera (terbang ke atas)
    if (motion.Naik) {
        terbang -= 2.0;
    }
    // Jika tombol Q ditekan, turunkan posisi vertikal kamera (terbang ke bawah)
    if (motion.Turun) {
        terbang += 2.0;
    }

    // Batasi sudut pitch agar tidak berputar berlebihan
    if (pitch >= 70)
        pitch = 70;
    if (pitch <= -90)
        pitch = -90;

    // Rotasi scene sesuai pitch dan yaw (rotasi kamera)
    glRotatef(-pitch, 1.0, 0.0, 0.0);
    glRotatef(-yaw, 0.0, 1.0, 0.0);

    // Translasi kamera ke posisi sesuai perhitungan (geser posisi)
    glTranslatef(-camX, -terbang, -350 - camZ);

    // Batasi posisi vertical agar tidak turun terlalu rendah
    if (terbang < 25 - 500)
        terbang = 24 - 500;
}

// Fungsi keyboard untuk menangani input tombol keyboard saat ditekan
void keyboard(unsigned char key, int x, int y) {
    switch (key) {
    case 'W':
    case 'w':
        motion.Forward = true; // Set tombol maju aktif
        break;
    case 'A':
    case 'a':
        motion.Left = true; // Set tombol kiri aktif
        break;
    case 'S':
    case 's':
        motion.Backward = true; // Set tombol mundur aktif
        break;
    case 'D':
    case 'd':
        motion.Right = true; // Set tombol kanan aktif
        break;
    case 'E':
    case 'e':
        motion.Naik = true; // Set tombol naik aktif
        break;
    case 'Q':
    case 'q':
        motion.Turun = true; // Set tombol turun aktif
        break;
    case '6': // Geser objek ke kanan (X bertambah)
        x_geser += 1.5;
        break;
    case '4': // Geser objek ke kiri (X berkurang)
        x_geser -= 1.5;
        break;
    case '9': // Geser objek ke atas (Y bertambah)
        y_geser += 1.5;
        break;
    case '3': // Geser objek ke bawah (Y berkurang)
        y_geser -= 1.5;
        break;
    case '8': // Geser objek ke depan (Z berkurang)
        z_geser -= 1.5;
        break;
    case '5': // Geser objek ke belakang (Z bertambah)
        z_geser += 0.5;
        break;
    case '`': // Keluar program
        exit(1);
    case 'N': // Toggle Day/Night mode
    case 'n':
        isDayMode = !isDayMode;
        glutPostRedisplay();
        break;
    case 'O':
    case 'o':
        isBulbOn = !isBulbOn;
        glutPostRedisplay();
        break;

    case 'I':
    case 'i':
        tankMotion.ForwardI = true;
        break;
    case 'K':
    case 'k':
        tankMotion.BackwardK = true;
        break;
    case 'L':
    case 'l':
        tankMotion.ForwardL = true;
        break;
    case 'J':
    case 'j':
        tankMotion.BackwardJ = true;
        break;
    }
}

// Fungsi keyboard_up untuk menangani input tombol dilepas
void keyboard_up(unsigned char key, int x, int y) {
    switch (key) {
    case 'W':
    case 'w':
        motion.Forward = false;
        break;
    case 'A':
    case 'a':
        motion.Left = false;
        break;
    case 'S':
    case 's':
        motion.Backward = false;
        break;
    case 'D':
    case 'd':
        motion.Right = false;
        break;
    case 'E':
    case 'e':
        motion.Naik = false;
        break;
    case 'Q':
    case 'q':
        motion.Turun = false;
        break;

    case 'I':
    case 'i':
        tankMotion.ForwardI = false;
        break;
    case 'K':
    case 'k':
        tankMotion.BackwardK = false;
        break;
    case 'L':
    case 'l':
        tankMotion.ForwardL = false;
        break;
    case 'J':
    case 'j':
        tankMotion.BackwardJ = false;
        break;
    }
}

// Fungsi utama program
int main(int argc, char** argv) {
    glutInit(&argc, argv); // Inisialisasi GLUT
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH); // Mode tampilan dengan double buffer dan depth buffer
    glutInitWindowSize(width, height); // Ukuran window
    glutInitWindowPosition(100, 50); // Posisi window di layar
    glutCreateWindow("TR GRAFKOM KELOMPOK X"); // Buat window dengan judul

    init(); // Panggil fungsi inisialisasi

    glutDisplayFunc(display); // Fungsi display terpanggil saat perlu render
    glutReshapeFunc(reshape); // Fungsi reshape terpanggil saat window diubah ukuran
    glutPassiveMotionFunc(passive_motion); // Fungsi menangani pergerakan mouse tanpa klik
    glutTimerFunc(0, timer, 0); // Timer untuk refresh frame
    glutKeyboardFunc(keyboard); // Fungsi keyboard saat tombol ditekan
    glutKeyboardUpFunc(keyboard_up); // Fungsi keyboard saat tombol dilepas

    glutMainLoop(); // Masuk ke loop utama GLUT

    return 0;
}
