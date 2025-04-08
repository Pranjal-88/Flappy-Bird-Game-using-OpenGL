#include <GL/glew.h>
#include <GL/glut.h>
#include <iostream>
#include <vector>
#include <string>
#include <cmath>

#define WINDOW_WIDTH 800
#define WINDOW_HEIGHT 600
#define OBSTACLE_WIDTH 60
#define OBSTACLE_GAP 200
#define GRAVITY 0.4f
#define JUMP_STRENGTH 7.0f

// Different types of obstacles
enum ObstacleType {
    TEACHER, 
    PUDDLE,
    STUDENT_GROUP,
    RANDOM_DOG
};

struct Obstacle {
    float x, height;
    bool passed;
    ObstacleType type;
};

std::vector<Obstacle> obstacles;
float adityaX = 150, adityaY = 300, velocity = 0;
int score = 0, highScore = 0;
int timeLeft = 90; // 90 seconds to reach class
bool gameOver = false, gameStarted = false;
bool successful = false; // Did Aditya reach class in time?
float runningPhase = 0.0f; // For running animation
int background_scroll = 0;
float lastTimerUpdate = 0;

// Function to display text on screen
void drawText(const char* text, int x, int y) {
    glColor3f(1.0f, 1.0f, 1.0f); // White text
    glRasterPos2i(x, y);
    while (*text) {
        glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, *text++);
    }
}

// Function to draw Aditya Rana (tall boy with glasses)
void drawAditya() {
    float legOffset = sin(runningPhase) * 15.0f; // For running animation
    
    // Body (tall rectangle)
    glColor3f(0.2f, 0.4f, 0.8f); // Blue shirt
    glBegin(GL_QUADS);
    glVertex2f(adityaX - 10, adityaY - 25);
    glVertex2f(adityaX + 10, adityaY - 25);
    glVertex2f(adityaX + 10, adityaY + 25);
    glVertex2f(adityaX - 10, adityaY + 25);
    glEnd();
    
    // Head (circle approximation)
    glColor3f(0.95f, 0.85f, 0.6f); // Skin color
    glBegin(GL_POLYGON);
    float radius = 15.0f;
    for (int i = 0; i < 20; i++) {
        float angle = 2.0f * 3.1415926f * i / 20;
        glVertex2f(adityaX + sin(angle) * radius, adityaY + 40 + cos(angle) * radius);
    }
    glEnd();
    
    // Glasses (spectacles)
    glColor3f(0.0f, 0.0f, 0.0f); // Black
    // Left lens frame
    glBegin(GL_LINE_LOOP);
    radius = 6.0f;
    for (int i = 0; i < 20; i++) {
        float angle = 2.0f * 3.1415926f * i / 20;
        glVertex2f(adityaX - 7 + sin(angle) * radius, adityaY + 40 + cos(angle) * radius);
    }
    glEnd();
    
    // Right lens frame
    glBegin(GL_LINE_LOOP);
    for (int i = 0; i < 20; i++) {
        float angle = 2.0f * 3.1415926f * i / 20;
        glVertex2f(adityaX + 7 + sin(angle) * radius, adityaY + 40 + cos(angle) * radius);
    }
    glEnd();
    
    // Bridge of glasses
    glBegin(GL_LINES);
    glVertex2f(adityaX - 1, adityaY + 40);
    glVertex2f(adityaX + 1, adityaY + 40);
    glEnd();
    
    // Hair
    glColor3f(0.1f, 0.1f, 0.1f); // Black hair
    glBegin(GL_QUADS);
    glVertex2f(adityaX - 15, adityaY + 45);
    glVertex2f(adityaX + 15, adityaY + 45);
    glVertex2f(adityaX + 15, adityaY + 55);
    glVertex2f(adityaX - 15, adityaY + 55);
    glEnd();
    
    // Legs (with running animation)
    glColor3f(0.1f, 0.1f, 0.3f); // Dark blue pants
    // Left leg
    glBegin(GL_QUADS);
    glVertex2f(adityaX - 8, adityaY - 25);
    glVertex2f(adityaX - 2, adityaY - 25);
    glVertex2f(adityaX - 2 + legOffset, adityaY - 60);
    glVertex2f(adityaX - 8 + legOffset, adityaY - 60);
    glEnd();
    
    // Right leg (opposite phase)
    glBegin(GL_QUADS);
    glVertex2f(adityaX + 2, adityaY - 25);
    glVertex2f(adityaX + 8, adityaY - 25);
    glVertex2f(adityaX + 8 - legOffset, adityaY - 60);
    glVertex2f(adityaX + 2 - legOffset, adityaY - 60);
    glEnd();
    
    // Backpack
    glColor3f(0.5f, 0.2f, 0.2f); // Brown backpack
    glBegin(GL_QUADS);
    glVertex2f(adityaX - 15, adityaY - 15);
    glVertex2f(adityaX - 5, adityaY - 15);
    glVertex2f(adityaX - 5, adityaY + 15);
    glVertex2f(adityaX - 15, adityaY + 15);
    glEnd();
    
    // Update running animation
    runningPhase += 0.2f;
}

// Function to draw obstacles based on type
void drawObstacle(float x, float height, ObstacleType type) {
    // Declare all variables at the beginning of the function (before the switch)
    float radius;
    
    switch(type) {
        case TEACHER:
            // Angry teacher
            glColor3f(0.8f, 0.2f, 0.2f); // Red clothes
            
            // Body
            glBegin(GL_QUADS);
            glVertex2f(x, height + 30);
            glVertex2f(x + OBSTACLE_WIDTH, height + 30);
            glVertex2f(x + OBSTACLE_WIDTH, height + 100);
            glVertex2f(x, height + 100);
            glEnd();
            
            // Head
            glColor3f(0.95f, 0.85f, 0.6f); // Skin color
            glBegin(GL_POLYGON);
            radius = 20.0f;
            for (int i = 0; i < 20; i++) {
                float angle = 2.0f * 3.1415926f * i / 20;
                glVertex2f(x + OBSTACLE_WIDTH/2 + sin(angle) * radius, 
                          height + 130 + cos(angle) * radius);
            }
            glEnd();
            
            // Angry expression
            glColor3f(0.0f, 0.0f, 0.0f); // Black
            // Eyes
            glBegin(GL_LINES);
            glVertex2f(x + OBSTACLE_WIDTH/2 - 10, height + 135);
            glVertex2f(x + OBSTACLE_WIDTH/2 - 2, height + 130);
            
            glVertex2f(x + OBSTACLE_WIDTH/2 + 10, height + 135);
            glVertex2f(x + OBSTACLE_WIDTH/2 + 2, height + 130);
            glEnd();
            
            // Mouth
            glBegin(GL_LINES);
            glVertex2f(x + OBSTACLE_WIDTH/2 - 10, height + 115);
            glVertex2f(x + OBSTACLE_WIDTH/2 + 10, height + 115);
            glEnd();
            break;
            
        case PUDDLE:
            // Water puddle
            glColor3f(0.0f, 0.4f, 0.8f); // Blue water
            
            // Puddle shape (ellipse approximation)
            glBegin(GL_POLYGON);
            for (int i = 0; i < 20; i++) {
                float angle = 2.0f * 3.1415926f * i / 20;
                glVertex2f(x + OBSTACLE_WIDTH/2 + sin(angle) * OBSTACLE_WIDTH/2, 
                          height + 20 + cos(angle) * 10);
            }
            glEnd();
            
            // Water reflection
            glColor3f(0.2f, 0.6f, 1.0f); // Lighter blue
            glBegin(GL_LINES);
            glVertex2f(x + 10, height + 20);
            glVertex2f(x + 20, height + 20);
            
            glVertex2f(x + 30, height + 22);
            glVertex2f(x + 45, height + 22);
            
            glVertex2f(x + 15, height + 18);
            glVertex2f(x + 25, height + 18);
            glEnd();
            break;
            
        case STUDENT_GROUP:
            // Group of students blocking the way
            for (int i = 0; i < 3; i++) {
                // Bodies
                glColor3f(0.2f + 0.2f * i, 0.3f, 0.7f - 0.2f * i); // Different colored clothes
                glBegin(GL_QUADS);
                glVertex2f(x + i*20, height + 30);
                glVertex2f(x + i*20 + 15, height + 30);
                glVertex2f(x + i*20 + 15, height + 80);
                glVertex2f(x + i*20, height + 80);
                glEnd();
                
                // Heads
                glColor3f(0.95f, 0.85f, 0.6f); // Skin color
                glBegin(GL_POLYGON);
                radius = 12.0f;
                for (int j = 0; j < 20; j++) {
                    float angle = 2.0f * 3.1415926f * j / 20;
                    glVertex2f(x + i*20 + 7.5f + sin(angle) * radius, 
                              height + 95 + cos(angle) * radius);
                }
                glEnd();
            }
            break;
            
        case RANDOM_DOG:
            // Dog running across
            glColor3f(0.6f, 0.4f, 0.2f); // Brown dog
            
            // Body
            glBegin(GL_QUADS);
            glVertex2f(x, height + 20);
            glVertex2f(x + 40, height + 20);
            glVertex2f(x + 40, height + 40);
            glVertex2f(x, height + 40);
            glEnd();
            
            // Head
            glBegin(GL_QUADS);
            glVertex2f(x + 40, height + 25);
            glVertex2f(x + 55, height + 25);
            glVertex2f(x + 55, height + 45);
            glVertex2f(x + 40, height + 45);
            glEnd();
            
            // Tail
            glBegin(GL_TRIANGLES);
            glVertex2f(x, height + 30);
            glVertex2f(x - 15, height + 45);
            glVertex2f(x - 5, height + 30);
            glEnd();
            
            // Legs
            glBegin(GL_QUADS);
            glVertex2f(x + 10, height + 10);
            glVertex2f(x + 15, height + 10);
            glVertex2f(x + 15, height + 20);
            glVertex2f(x + 10, height + 20);
            
            glVertex2f(x + 30, height + 10);
            glVertex2f(x + 35, height + 10);
            glVertex2f(x + 35, height + 20);
            glVertex2f(x + 30, height + 20);
            glEnd();
            break;
    }
}

// Function to initialize/reset game state
void initGame() {
    adityaY = 300.0f;
    velocity = 0.0f;
    obstacles.clear();
    // Create a mix of obstacles
    for (int i = 0; i < 15; i++) {
        ObstacleType type = static_cast<ObstacleType>(rand() % 4);
        float height = rand() % 200 + 50;
        if (type == PUDDLE) height = 0; // Puddles are on the ground
        obstacles.push_back({static_cast<float>(WINDOW_WIDTH + i * 300), height, false, type});
    }
    score = 0;
    timeLeft = 90;
    gameOver = false;
    gameStarted = false;
    successful = false;
    lastTimerUpdate = 0;
    background_scroll = 0;
}

// Function to check for collisions
void checkCollision() {
    if (adityaY - 60 <= 0) { // Hit ground
        adityaY = 60; // Prevent falling through ground
        velocity = 0;
    }

    if (adityaY + 55 >= WINDOW_HEIGHT) { // Hit ceiling
        adityaY = WINDOW_HEIGHT - 55;
        velocity = -2; // Bounce off ceiling slightly
    }

    // Check collision with obstacles
    for (auto &obstacle : obstacles) {
        float collisionY = 0;
        float obstacleHeight = 0;
        
        // Different collision boxes based on obstacle type
        switch(obstacle.type) {
            case TEACHER:
                collisionY = obstacle.height + 30; // Bottom of teacher
                obstacleHeight = 100; // Height of teacher
                break;
            case PUDDLE:
                collisionY = obstacle.height + 10; // Bottom of puddle
                obstacleHeight = 20; // Height of puddle
                break;
            case STUDENT_GROUP:
                collisionY = obstacle.height + 30; // Bottom of students
                obstacleHeight = 70; // Height of student group
                break;
            case RANDOM_DOG:
                collisionY = obstacle.height + 10; // Bottom of dog
                obstacleHeight = 40; // Height of dog
                break;
        }
        
        // Check if Aditya's bounding box intersects with obstacle
        if (adityaX + 15 > obstacle.x && adityaX - 15 < obstacle.x + OBSTACLE_WIDTH) {
            if ((adityaY - 60 < collisionY + obstacleHeight && adityaY - 25 > collisionY) ||
                (adityaY + 55 > collisionY && adityaY - 60 < collisionY + obstacleHeight)) {
                // Collision detected!
                gameOver = true;
                successful = false;
                return;
            }
        }
    }
    
    // Check if reached the end (completed 1500 points)
    if (score >= 1500) {
        gameOver = true;
        successful = true;
    }
    
    // Check if time ran out
    if (timeLeft <= 0) {
        gameOver = true;
        successful = false;
    }
}

// Function to draw the school background
void drawBackground() {
    int scrollOffset = background_scroll % WINDOW_WIDTH;
    
    // Sky
    glBegin(GL_QUADS);
    glColor3f(0.4f, 0.7f, 1.0f); // Blue sky
    glVertex2f(0, 0);
    glVertex2f(WINDOW_WIDTH, 0);
    glVertex2f(WINDOW_WIDTH, WINDOW_HEIGHT);
    glVertex2f(0, WINDOW_HEIGHT);
    glEnd();
    
    // Ground
    glBegin(GL_QUADS);
    glColor3f(0.7f, 0.7f, 0.7f); // Gray sidewalk
    glVertex2f(0, 0);
    glVertex2f(WINDOW_WIDTH, 0);
    glVertex2f(WINDOW_WIDTH, 60);
    glVertex2f(0, 60);
    glEnd();
    
    // Sidewalk lines
    glColor3f(0.8f, 0.8f, 0.8f); // Lighter gray
    for (int i = -scrollOffset; i < WINDOW_WIDTH; i += 100) {
        glBegin(GL_QUADS);
        glVertex2f(i, 30);
        glVertex2f(i + 50, 30);
        glVertex2f(i + 50, 40);
        glVertex2f(i, 40);
        glEnd();
    }
    
    // Buildings in background (scrolling)
    for (int i = -scrollOffset; i < WINDOW_WIDTH + 300; i += 300) {
        // School building (destination)
        if (i > WINDOW_WIDTH - 300 && score >= 1400) {
            // School is approaching
            glColor3f(0.8f, 0.6f, 0.3f); // Brown building
            glBegin(GL_QUADS);
            glVertex2f(i, 60);
            glVertex2f(i + 250, 60);
            glVertex2f(i + 250, 350);
            glVertex2f(i, 350);
            glEnd();
            
            // School door
            glColor3f(0.4f, 0.3f, 0.2f); // Dark brown door
            glBegin(GL_QUADS);
            glVertex2f(i + 100, 60);
            glVertex2f(i + 150, 60);
            glVertex2f(i + 150, 120);
            glVertex2f(i + 100, 120);
            glEnd();
            
            // School sign
            glColor3f(1.0f, 1.0f, 1.0f); // White sign
            glBegin(GL_QUADS);
            glVertex2f(i + 70, 270);
            glVertex2f(i + 180, 270);
            glVertex2f(i + 180, 320);
            glVertex2f(i + 70, 320);
            glEnd();
            
            // Draw "SCHOOL" text
            glColor3f(0.0f, 0.0f, 0.0f); // Black text
            glRasterPos2i(i + 95, 290);
            const char* text = "SCHOOL";
            for (int j = 0; text[j] != '\0'; j++) {
                glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, text[j]);
            }
        } else {
            // Regular background buildings
            float height = 150 + (i * 7541) % 150; // Pseudorandom height
            float colorVar = (i * 6151) % 10 / 30.0f; // Pseudorandom color variation
            
            glColor3f(0.5f + colorVar, 0.5f, 0.5f - colorVar); // Building color
            glBegin(GL_QUADS);
            glVertex2f(i, 60);
            glVertex2f(i + 200, 60);
            glVertex2f(i + 200, 60 + height);
            glVertex2f(i, 60 + height);
            glEnd();
            
            // Windows
            glColor3f(0.8f, 0.9f, 1.0f); // Light blue windows
            for (int w = 0; w < 5; w++) {
                for (int h = 0; h < height/40; h++) {
                    glBegin(GL_QUADS);
                    glVertex2f(i + 10 + w*40, 80 + h*40);
                    glVertex2f(i + 30 + w*40, 80 + h*40);
                    glVertex2f(i + 30 + w*40, 100 + h*40);
                    glVertex2f(i + 10 + w*40, 100 + h*40);
                    glEnd();
                }
            }
        }
    }
}

// Function to update game state
void update(int value) {
    if (gameOver) return;
    
    if (gameStarted) {
        // Scroll the background
        background_scroll += 5;
        
        // Move obstacles
        for (auto &obstacle : obstacles) {
            obstacle.x -= 5; // Move obstacles left
            
            // Reset obstacle when it moves out of screen
            if (obstacle.x + OBSTACLE_WIDTH < 0) {
                obstacle.x = WINDOW_WIDTH + rand() % 100;
                
                // Change the obstacle type for variety
                obstacle.type = static_cast<ObstacleType>(rand() % 4);
                
                float height = rand() % 200 + 50;
                if (obstacle.type == PUDDLE) height = 0; // Puddles are on the ground
                obstacle.height = height;
                
                obstacle.passed = false;
            }
            
            // Scoring
            if (!obstacle.passed && obstacle.x + OBSTACLE_WIDTH < adityaX) {
                obstacle.passed = true;
                score += 100;
            }
        }
        
        // Apply gravity
        velocity -= GRAVITY;
        adityaY += velocity;
        
        // Update timer (approximately once per second)
        float currentTime = glutGet(GLUT_ELAPSED_TIME) / 1000.0f;
        if (currentTime - lastTimerUpdate >= 1.0f) {
            timeLeft--;
            lastTimerUpdate = currentTime;
        }
        
        checkCollision();
    }
    
    glutPostRedisplay();
    glutTimerFunc(16, update, 0);
}

// Function to handle keypresses
void handleKeypress(unsigned char key, int x, int y) {
    if (key == ' ' && !gameOver) {
        if (!gameStarted) {
            gameStarted = true;
            glutTimerFunc(16, update, 0); // Start updating when the game starts
            lastTimerUpdate = glutGet(GLUT_ELAPSED_TIME) / 1000.0f;
        }
        velocity = JUMP_STRENGTH; // Make Aditya jump
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
        // Title screen
        drawText("Aditya Rana - Can he reach class?", WINDOW_WIDTH / 2 - 150, WINDOW_HEIGHT / 2 + 50);
        drawText("Press SPACE to Start Running", WINDOW_WIDTH / 2 - 120, WINDOW_HEIGHT / 2);
        drawText("Press SPACE to Jump over obstacles", WINDOW_WIDTH / 2 - 150, WINDOW_HEIGHT / 2 - 30);
        drawText("You have 90 seconds to reach class!", WINDOW_WIDTH / 2 - 150, WINDOW_HEIGHT / 2 - 60);
        
        // Show Aditya even before starting
        adityaX = WINDOW_WIDTH / 2 - 100;
        adityaY = 150;
        drawAditya();
    } else {
        // Draw obstacles
        for (auto &obstacle : obstacles) {
            drawObstacle(obstacle.x, obstacle.height, obstacle.type);
        }
        
        // Draw Aditya
        drawAditya();
        
        // Display score and timer
        char scoreText[50];
        sprintf(scoreText, "Distance: %d/1500m", score);
        drawText(scoreText, 10, WINDOW_HEIGHT - 30);
        
        char timeText[50];
        sprintf(timeText, "Time Left: %d seconds", timeLeft);
        drawText(timeText, 10, WINDOW_HEIGHT - 60);
        
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
            
            if (successful) {
                // Success message
                drawText("YOU MADE IT TO CLASS IN TIME!", WINDOW_WIDTH / 2 - 150, WINDOW_HEIGHT / 2 + 50);
                drawText("The teacher looks surprised to see you on time.", WINDOW_WIDTH / 2 - 170, WINDOW_HEIGHT / 2 + 20);
            } else {
                // Game over text
                if (timeLeft <= 0) {
                    drawText("OUT OF TIME! YOU'RE LATE AGAIN!", WINDOW_WIDTH / 2 - 150, WINDOW_HEIGHT / 2 + 50);
                } else {
                    drawText("OUCH! YOU DIDN'T MAKE IT!", WINDOW_WIDTH / 2 - 120, WINDOW_HEIGHT / 2 + 50);
                }
            }
            
            // Show final score
            char finalText[100];
            sprintf(finalText, "Distance covered: %d/1500m", score);
            drawText(finalText, WINDOW_WIDTH / 2 - 100, WINDOW_HEIGHT / 2);
            
            // Time remaining/used
            if (successful) {
                sprintf(finalText, "Time remaining: %d seconds", timeLeft);
            } else {
                sprintf(finalText, "Try to manage your time better next time!");
            }
            drawText(finalText, WINDOW_WIDTH / 2 - 120, WINDOW_HEIGHT / 2 - 30);
            
            drawText("Press R to Try Again", WINDOW_WIDTH / 2 - 80, WINDOW_HEIGHT / 2 - 80);
        }
    }
    
    glutSwapBuffers();
}

// Function to set up OpenGL
void setup() {
    glClearColor(0.0f, 0.0f, 0.3f, 1.0f);
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
    glutCreateWindow("Aditya Rana - Can he reach class?");
    
    setup();
    initGame();
    
    glutDisplayFunc(display);
    glutKeyboardFunc(handleKeypress);
    
    glutMainLoop();
    return 0;
}   