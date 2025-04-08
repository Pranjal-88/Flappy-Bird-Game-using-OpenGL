#include <GL/glew.h>
#include <GL/glut.h>
#include <iostream>
#include <vector>
#include <string>
#include <cmath>

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
float wingAngle = 0.0f;  // For wing animation

// Function to display text on screen
void drawText(const char* text, int x, int y) {
    glColor3f(1.0f, 1.0f, 1.0f); // White text
    glRasterPos2i(x, y);
    while (*text) {
        glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, *text++);
    }
}

// Function to draw the traditional square flappy bird
void drawBird() {
    // Main body (square)
    glColor3f(1.0f, 1.0f, 0.0f); // Yellow body
    glBegin(GL_QUADS);
    glVertex2f(birdX - 15, birdY - 15);
    glVertex2f(birdX + 15, birdY - 15);
    glVertex2f(birdX + 15, birdY + 15);
    glVertex2f(birdX - 15, birdY + 15);
    glEnd();
    
    // White rectangular eye
    glColor3f(1.0f, 1.0f, 1.0f); // White
    glBegin(GL_QUADS);
    glVertex2f(birdX, birdY + 3);
    glVertex2f(birdX + 10, birdY + 3);
    glVertex2f(birdX + 10, birdY + 10);
    glVertex2f(birdX, birdY + 10);
    glEnd();
    
    // Black pupil
    glColor3f(0.0f, 0.0f, 0.0f); // Black
    glBegin(GL_QUADS);
    glVertex2f(birdX + 5, birdY + 5);
    glVertex2f(birdX + 9, birdY + 5);
    glVertex2f(birdX + 9, birdY + 9);
    glVertex2f(birdX + 5, birdY + 9);
    glEnd();
    
    // Orange rectangular beak
    glColor3f(1.0f, 0.5f, 0.0f); // Orange
    glBegin(GL_QUADS);
    glVertex2f(birdX + 15, birdY - 5);
    glVertex2f(birdX + 25, birdY - 5);
    glVertex2f(birdX + 25, birdY + 5);
    glVertex2f(birdX + 15, birdY + 5);
    glEnd();
    
    // Small wing (animated slightly)
    glColor3f(0.9f, 0.9f, 0.0f); // Slightly darker yellow
    glPushMatrix();
    glTranslatef(birdX - 15, birdY, 0);
    float wingOffset = sin(wingAngle) * 3.0f; // Smaller wing movement
    
    glBegin(GL_QUADS);
    glVertex2f(0, -5 + wingOffset);
    glVertex2f(-8, -8 + wingOffset);
    glVertex2f(-8, 2 + wingOffset);
    glVertex2f(0, 5 + wingOffset);
    glEnd();
    
    glPopMatrix();
    
    // Update wing animation
    wingAngle += 0.2f;
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
    
    // Pipe top cap
    glColor3f(0.0f, 0.7f, 0.0f); // Darker green for the cap
    glBegin(GL_QUADS);
    glVertex2f(x - 5, height);
    glVertex2f(x + PIPE_WIDTH + 5, height);
    glVertex2f(x + PIPE_WIDTH + 5, height + 10);
    glVertex2f(x - 5, height + 10);
    glEnd();

    // Bottom pipe
    glColor3f(0.0f, 0.8f, 0.0f); // Green pipes
    glBegin(GL_QUADS);
    glVertex2f(x, height + PIPE_GAP);
    glVertex2f(x + PIPE_WIDTH, height + PIPE_GAP);
    glVertex2f(x + PIPE_WIDTH, WINDOW_HEIGHT);
    glVertex2f(x, WINDOW_HEIGHT);
    glEnd();
    
    // Bottom pipe cap
    glColor3f(0.0f, 0.7f, 0.0f); // Darker green for the cap
    glBegin(GL_QUADS);
    glVertex2f(x - 5, height + PIPE_GAP - 10);
    glVertex2f(x + PIPE_WIDTH + 5, height + PIPE_GAP - 10);
    glVertex2f(x + PIPE_WIDTH + 5, height + PIPE_GAP);
    glVertex2f(x - 5, height + PIPE_GAP);
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

// Function to draw the background
void drawBackground() {
    // Sky background
    glBegin(GL_QUADS);
    glColor3f(0.4f, 0.7f, 1.0f); // Light blue sky
    glVertex2f(0, 0);
    glVertex2f(WINDOW_WIDTH, 0);
    glVertex2f(WINDOW_WIDTH, WINDOW_HEIGHT);
    glVertex2f(0, WINDOW_HEIGHT);
    glEnd();
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
    
    // Draw the background
    drawBackground();

    if (!gameStarted) {
        drawText("Flappy Bird", WINDOW_WIDTH / 2 - 50, WINDOW_HEIGHT / 2 + 50);
        drawText("Press SPACE to Start", WINDOW_WIDTH / 2 - 80, WINDOW_HEIGHT / 2);
        drawBird(); // Show the bird even before starting
    } else {
        // Draw game elements
        for (auto &pipe : pipes) {
            drawPipe(pipe.x, pipe.height);
        }
        drawBird();
        
        // Always display Score and High Score (whether alive or game over)
        drawText(("Score: " + std::to_string(score)).c_str(), 10, WINDOW_HEIGHT - 30);
        drawText(("High Score: " + std::to_string(highScore)).c_str(), 10, WINDOW_HEIGHT - 50);
        
        if (gameOver) {
            // Semi-transparent overlay
            glColor4f(0.0f, 0.0f, 0.0f, 0.5f);
            glEnable(GL_BLEND);
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
            glBegin(GL_QUADS);
            glVertex2f(0, 0);
            glVertex2f(WINDOW_WIDTH, 0);
            glVertex2f(WINDOW_WIDTH, WINDOW_HEIGHT);
            glVertex2f(0, WINDOW_HEIGHT);
            glEnd();
            glDisable(GL_BLEND);
            
            // Game over text
            drawText("Game Over!", WINDOW_WIDTH / 2 - 50, WINDOW_HEIGHT / 2 + 30);
            
            // Show final score in the center as well
            drawText(("Your Score: " + std::to_string(score)).c_str(), WINDOW_WIDTH / 2 - 60, WINDOW_HEIGHT / 2);
            drawText(("High Score: " + std::to_string(highScore)).c_str(), WINDOW_WIDTH / 2 - 60, WINDOW_HEIGHT / 2 - 30);
            drawText("Press R to Restart", WINDOW_WIDTH / 2 - 80, WINDOW_HEIGHT / 2 - 60);
        }
    }

    glutSwapBuffers();
}

// Function to set up OpenGL
void setup() {
    glClearColor(0.0f, 0.0f, 0.3f, 1.0f); // Dark blue background
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluOrtho2D(0, WINDOW_WIDTH, 0, WINDOW_HEIGHT);
    
    // Enable blending for transparency
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
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