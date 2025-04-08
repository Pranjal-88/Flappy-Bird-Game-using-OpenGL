#include <GL/glew.h>
#include <GL/glut.h>
#include <iostream>
#include <vector>
#include <string>

#define WINDOW_WIDTH 800
#define WINDOW_HEIGHT 600
#define PIPE_WIDTH 50
#define PIPE_GAP 150
#define GRAVITY 0.5f
#define JUMP_STRENGTH 8.0f

struct Pipe {
    float x, height;
    bool passed;
};

std::vector<Pipe> pipes;
float birdX = 200, birdY = 300, velocity = 0;
int score = 0, highScore = 0;
bool gameOver = false, gameStarted = false;

// Function to display text on screen
void drawText(const char* text, int x, int y) {
    glColor3f(1.0f, 1.0f, 1.0f); // White text
    glRasterPos2i(x, y);
    while (*text) {
        glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, *text++);
    }
}

// Function to draw the bird
void drawBird() {
    glColor3f(1.0f, 1.0f, 0.0f); // Yellow bird
    glBegin(GL_QUADS);
    glVertex2f(birdX - 15, birdY - 15);
    glVertex2f(birdX + 15, birdY - 15);
    glVertex2f(birdX + 15, birdY + 15);
    glVertex2f(birdX - 15, birdY + 15);
    glEnd();
}

// Function to draw a pipe
void drawPipe(float x, float height) {
    glColor3f(0.0f, 0.8f, 0.0f); // Green pipes

    // Top pipe
    glBegin(GL_QUADS);
    glVertex2f(x, 0);
    glVertex2f(x + PIPE_WIDTH, 0);
    glVertex2f(x + PIPE_WIDTH, height);
    glVertex2f(x, height);
    glEnd();

    // Bottom pipe
    glBegin(GL_QUADS);
    glVertex2f(x, height + PIPE_GAP);
    glVertex2f(x + PIPE_WIDTH, height + PIPE_GAP);
    glVertex2f(x + PIPE_WIDTH, WINDOW_HEIGHT);
    glVertex2f(x, WINDOW_HEIGHT);
    glEnd();
}

// Function to initialize/reset game state
void initGame() {
    birdY = 300.0f;
    velocity = 0.0f;
    pipes.clear();
    for (int i = 0; i < 5; i++) {
        pipes.push_back({static_cast<float>(WINDOW_WIDTH + i * 200), static_cast<float>(rand() % 200 + 100), false});
    }
    score = 0;
    gameOver = false;
    gameStarted = false;
}

// Function to check for collisions
void checkCollision() {
    if (birdY <= 0 || birdY >= WINDOW_HEIGHT) {
        gameOver = true;
    }

    for (auto &pipe : pipes) {
        if (birdX + 15 > pipe.x && birdX - 15 < pipe.x + PIPE_WIDTH) {
            if (birdY - 15 < pipe.height || birdY + 15 > pipe.height + PIPE_GAP) {
                gameOver = true;
            }
        }
    }
}

// Function to update game state
void update(int value) {
    if (gameOver) return;

    float farthestX = 0;
    for (const auto &p : pipes) {
        if (p.x > farthestX) farthestX = p.x;
    }

    for (auto &pipe : pipes) {
        pipe.x -= 5; // Move pipes left

        if (pipe.x + PIPE_WIDTH < 0) {
            pipe.x = farthestX + 200; // Proper spacing from last pipe
            pipe.height = rand() % 200 + 100;
            pipe.passed = false;
        }

        if (!pipe.passed && pipe.x + PIPE_WIDTH < birdX) {
            pipe.passed = true;
            score += 10;
            if (score > highScore) {
                highScore = score;
            }
        }
    }

    velocity -= GRAVITY;
    birdY += velocity;

    checkCollision();
    glutPostRedisplay();
    glutTimerFunc(16, update, 0);
}


// Function to handle keypresses
void handleKeypress(unsigned char key, int x, int y) {
    if (key == ' ' && !gameOver) {
        if (!gameStarted) {
            gameStarted = true;
            glutTimerFunc(16, update, 0); // Start updating when the game starts
        }
        velocity = JUMP_STRENGTH; // Make the bird jump
    }
    if (key == 'r' && gameOver) {
        initGame();
        glutPostRedisplay();
    }
}

// Function to render the game
void display() {
    glClear(GL_COLOR_BUFFER_BIT);

    if (!gameStarted) {
        drawText("Press SPACE to Start", WINDOW_WIDTH / 2 - 80, WINDOW_HEIGHT / 2);
    } else if (gameOver) {
        drawText("Game Over! Press R to Restart", WINDOW_WIDTH / 2 - 100, WINDOW_HEIGHT / 2);
    } else {
        drawBird();
        for (auto &pipe : pipes) {
            drawPipe(pipe.x, pipe.height);
        }

        // Display Score and High Score
        drawText(("Score: " + std::to_string(score)).c_str(), 10, WINDOW_HEIGHT - 30);
        drawText(("High Score: " + std::to_string(highScore)).c_str(), 10, WINDOW_HEIGHT - 50);
    }

    glutSwapBuffers();
}

// Function to set up OpenGL
void setup() {
    glClearColor(0.4f, 0.7f, 1.0f, 1.0f); // Blue background
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluOrtho2D(0, WINDOW_WIDTH, 0, WINDOW_HEIGHT);
}

// Main function
int main(int argc, char** argv) {
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);
    glutInitWindowSize(WINDOW_WIDTH, WINDOW_HEIGHT);
    glutCreateWindow("Flappy Bird");

    setup();
    initGame();

    glutDisplayFunc(display);
    glutKeyboardFunc(handleKeypress);

    glutMainLoop();
    return 0;
}
