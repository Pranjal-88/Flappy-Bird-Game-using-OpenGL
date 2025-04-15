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
#define DAY_NIGHT_TRANSITION 50 // Score threshold for day/night transition
#define TRANSITION_ZONE 30      // Score range for smooth transition (15 points before and after transition)

struct Pipe {
    float x, height;
    bool passed;
};

struct Color {
    float r, g, b;
    
    Color(float red, float green, float blue) : r(red), g(green), b(blue) {}
    
    // Linear interpolation between two colors
    static Color lerp(const Color& a, const Color& b, float t) {
        // Ensure t is between 0 and 1
        t = t < 0 ? 0 : (t > 1 ? 1 : t);
        return Color(
            a.r + (b.r - a.r) * t,
            a.g + (b.g - a.g) * t,
            a.b + (b.b - a.b) * t
        );
    }
};

// Define day, twilight and night colors for smooth transition
const Color DAY_SKY(0.4f, 0.7f, 1.0f);           // Light blue sky
const Color TWILIGHT_SKY(0.6f, 0.4f, 0.7f);      // Purple twilight sky
const Color NIGHT_SKY(0.1f, 0.1f, 0.3f);         // Dark blue night sky

const Color DAY_GROUND(0.3f, 0.6f, 0.1f);        // Green ground
const Color TWILIGHT_GROUND(0.25f, 0.5f, 0.1f);  // Slightly darker green ground
const Color NIGHT_GROUND(0.2f, 0.4f, 0.1f);      // Darker green ground

const Color DAY_PIPE(0.0f, 0.8f, 0.0f);          // Green pipes
const Color TWILIGHT_PIPE(0.0f, 0.7f, 0.0f);     // Medium green pipes
const Color NIGHT_PIPE(0.0f, 0.6f, 0.0f);        // Darker green pipes

const Color DAY_PIPE_CAP(0.0f, 0.7f, 0.0f);      // Pipe cap
const Color TWILIGHT_PIPE_CAP(0.0f, 0.6f, 0.0f); // Medium pipe cap
const Color NIGHT_PIPE_CAP(0.0f, 0.5f, 0.0f);    // Darker pipe cap

std::vector<Pipe> pipes;
float birdX = 200, birdY = 300, velocity = 0;
int score = 0, highScore = 0;
bool gameOver = false, gameStarted = false;
float wingAngle = 0.0f;  // For wing animation
float starAlpha = 0.0f;  // For star opacity

// Function to get transition progress (0.0 = full day, 0.5 = twilight, 1.0 = full night)
float getTransitionProgress() {
    // Calculate the position in the current day/night cycle
    int cyclePosition = score % (DAY_NIGHT_TRANSITION * 2);
    
    // For smooth transition throughout the cycle
    if (cyclePosition < DAY_NIGHT_TRANSITION) {
        // Transitioning from day (0.0) to night (1.0)
        float progress = static_cast<float>(cyclePosition) / DAY_NIGHT_TRANSITION;
        
        // Apply easing function for smoother transition
        return 0.5f * (1.0f - cos(M_PI * progress));
    } else {
        // Transitioning from night (1.0) back to day (0.0)
        float progress = static_cast<float>(cyclePosition - DAY_NIGHT_TRANSITION) / DAY_NIGHT_TRANSITION;
        
        // Apply easing function for smoother transition
        return 1.0f - 0.5f * (1.0f - cos(M_PI * progress));
    }
}

// Function to get the current sky color based on transition
Color getCurrentSkyColor() {
    float t = getTransitionProgress();
    
    // First half: day to twilight
    if (t <= 0.5f) {
        // Normalize t to 0-1 range for this segment
        float normalizedT = t * 2.0f;
        return Color::lerp(DAY_SKY, TWILIGHT_SKY, normalizedT);
    } 
    // Second half: twilight to night
    else {
        // Normalize t to 0-1 range for this segment
        float normalizedT = (t - 0.5f) * 2.0f;
        return Color::lerp(TWILIGHT_SKY, NIGHT_SKY, normalizedT);
    }
}

// Function to get the current ground color based on transition
Color getCurrentGroundColor() {
    float t = getTransitionProgress();
    
    // First half: day to twilight
    if (t <= 0.5f) {
        // Normalize t to 0-1 range for this segment
        float normalizedT = t * 2.0f;
        return Color::lerp(DAY_GROUND, TWILIGHT_GROUND, normalizedT);
    } 
    // Second half: twilight to night
    else {
        // Normalize t to 0-1 range for this segment
        float normalizedT = (t - 0.5f) * 2.0f;
        return Color::lerp(TWILIGHT_GROUND, NIGHT_GROUND, normalizedT);
    }
}

// Function to get the current pipe color based on transition
Color getCurrentPipeColor() {
    float t = getTransitionProgress();
    
    // First half: day to twilight
    if (t <= 0.5f) {
        // Normalize t to 0-1 range for this segment
        float normalizedT = t * 2.0f;
        return Color::lerp(DAY_PIPE, TWILIGHT_PIPE, normalizedT);
    } 
    // Second half: twilight to night
    else {
        // Normalize t to 0-1 range for this segment
        float normalizedT = (t - 0.5f) * 2.0f;
        return Color::lerp(TWILIGHT_PIPE, NIGHT_PIPE, normalizedT);
    }
}

// Function to get the current pipe cap color based on transition
Color getCurrentPipeCapColor() {
    float t = getTransitionProgress();
    
    // First half: day to twilight
    if (t <= 0.5f) {
        // Normalize t to 0-1 range for this segment
        float normalizedT = t * 2.0f;
        return Color::lerp(DAY_PIPE_CAP, TWILIGHT_PIPE_CAP, normalizedT);
    } 
    // Second half: twilight to night
    else {
        // Normalize t to 0-1 range for this segment
        float normalizedT = (t - 0.5f) * 2.0f;
        return Color::lerp(TWILIGHT_PIPE_CAP, NIGHT_PIPE_CAP, normalizedT);
    }
}

// Function to determine time of day status
std::string getTimeOfDayStatus() {
    float t = getTransitionProgress();
    
    if (t < 0.3f) {
        return "Day";
    } else if (t < 0.45f) {
        return "Sunset";
    } else if (t < 0.55f) {
        return "Twilight";
    } else if (t < 0.7f) {
        return "Dusk";
    } else if (t < 0.9f) {
        return "Night";
    } else {
        return "Dawn";
    }
}

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
    Color pipeColor = getCurrentPipeColor();
    Color pipeCapColor = getCurrentPipeCapColor();
    
    // Top pipe
    glColor3f(pipeColor.r, pipeColor.g, pipeColor.b);
    glBegin(GL_QUADS);
    glVertex2f(x, 0);
    glVertex2f(x + PIPE_WIDTH, 0);
    glVertex2f(x + PIPE_WIDTH, height);
    glVertex2f(x, height);
    glEnd();
    
    // Pipe top cap
    glColor3f(pipeCapColor.r, pipeCapColor.g, pipeCapColor.b);
    glBegin(GL_QUADS);
    glVertex2f(x - 5, height);
    glVertex2f(x + PIPE_WIDTH + 5, height);
    glVertex2f(x + PIPE_WIDTH + 5, height + 10);
    glVertex2f(x - 5, height + 10);
    glEnd();

    // Bottom pipe
    glColor3f(pipeColor.r, pipeColor.g, pipeColor.b);
    glBegin(GL_QUADS);
    glVertex2f(x, height + PIPE_GAP);
    glVertex2f(x + PIPE_WIDTH, height + PIPE_GAP);
    glVertex2f(x + PIPE_WIDTH, WINDOW_HEIGHT);
    glVertex2f(x, WINDOW_HEIGHT);
    glEnd();
    
    // Bottom pipe cap
    glColor3f(pipeCapColor.r, pipeCapColor.g, pipeCapColor.b);
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
    starAlpha = 0.0f;
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

// Function to draw the moon with phases
void drawMoon() {
    float transitionProgress = getTransitionProgress();
    float moonOpacity = 0.0f;
    
    // Moon starts appearing at transition 0.4 (just before twilight) and fully appears at 0.6
    if (transitionProgress > 0.4f) {
        moonOpacity = (transitionProgress - 0.4f) / 0.2f;
        moonOpacity = moonOpacity > 1.0f ? 1.0f : moonOpacity;
    }
    
    glColor4f(0.9f, 0.9f, 0.8f, moonOpacity); // Slightly off-white for the moon with transparency
    
    // Enable blending for transparency
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    
    // Draw the moon (simple circle approximation using a polygon)
    glBegin(GL_POLYGON);
    float radius = 30.0f;
    float centerX = WINDOW_WIDTH - 80.0f;
    float centerY = WINDOW_HEIGHT - 80.0f;
    
    for (int i = 0; i < 20; i++) {
        float angle = 2.0f * M_PI * i / 20;
        glVertex2f(centerX + radius * cos(angle), centerY + radius * sin(angle));
    }
    glEnd();
    
    glDisable(GL_BLEND);
}

// Function to draw stars
void drawStars() {
    float transitionProgress = getTransitionProgress();
    
    // Stars start appearing at transition 0.45 (during twilight) and fully appear at 0.7
    if (transitionProgress > 0.45f) {
        starAlpha = (transitionProgress - 0.45f) / 0.25f;
        starAlpha = starAlpha > 1.0f ? 1.0f : starAlpha;
    } else {
        starAlpha = 0.0f;
    }
    
    // Only draw stars if there's some visibility
    if (starAlpha > 0.01f) {
        // Enable blending for transparency
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        
        glColor4f(1.0f, 1.0f, 1.0f, starAlpha);
        glPointSize(2.0f);
        
        // Draw background stars (more numerous, smaller)
        glBegin(GL_POINTS);
        srand(12345); // Fixed seed for consistent star pattern
        for (int i = 0; i < 100; i++) {
            float x = (rand() % WINDOW_WIDTH);
            float y = ((rand() % (WINDOW_HEIGHT - 100)) + 100); // Keep stars in upper part of sky
            glVertex2f(x, y);
        }
        glEnd();
        
        // Draw a few brighter stars
        glPointSize(3.0f);
        glBegin(GL_POINTS);
        srand(67890); // Different seed for variation
        for (int i = 0; i < 15; i++) {
            float x = (rand() % WINDOW_WIDTH);
            float y = ((rand() % (WINDOW_HEIGHT - 150)) + 150);
            glVertex2f(x, y);
        }
        glEnd();
        
        glDisable(GL_BLEND);
    }
}

// Function to draw the sun
void drawSun() {
    float transitionProgress = getTransitionProgress();
    float sunOpacity = 1.0f;
    
    // Sun starts fading at transition 0.3 and is gone by 0.6
    if (transitionProgress > 0.3f) {
        sunOpacity = 1.0f - (transitionProgress - 0.3f) / 0.3f;
        sunOpacity = sunOpacity < 0.0f ? 0.0f : sunOpacity;
    }
    
    // Sun color changes as it sets - from yellow to orange to red
    float r = 1.0f;
    float g = sunOpacity > 0.5f ? 0.9f : (0.6f + sunOpacity * 0.6f);
    float b = sunOpacity > 0.7f ? 0.0f : 0.0f;
    
    glColor4f(r, g, b, sunOpacity); // Sun with transparency and color variation
    
    // Don't draw the sun if it's completely faded out
    if (sunOpacity > 0.01f) {
        // Enable blending for transparency
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        
        // Draw the sun (simple circle approximation using a polygon)
        glBegin(GL_POLYGON);
        float radius = 40.0f;
        float centerX = WINDOW_WIDTH - 80.0f;
        float centerY = WINDOW_HEIGHT - 80.0f;
        
        for (int i = 0; i < 20; i++) {
            float angle = 2.0f * M_PI * i / 20;
            glVertex2f(centerX + radius * cos(angle), centerY + radius * sin(angle));
        }
        glEnd();
        
        glDisable(GL_BLEND);
    }
}

// Function to draw the background
void drawBackground() {
    // Get current sky and ground colors based on transition
    Color skyColor = getCurrentSkyColor();
    Color groundColor = getCurrentGroundColor();
    
    // Sky background with gradient
    glBegin(GL_QUADS);
    glColor3f(skyColor.r, skyColor.g, skyColor.b);
    glVertex2f(0, 0);
    glVertex2f(WINDOW_WIDTH, 0);
    
    // Slightly different color at the top for a gradient effect
    float t = getTransitionProgress();
    if (t < 0.5f) {
        // Day to twilight - make top slightly darker
        glColor3f(skyColor.r * 0.8f, skyColor.g * 0.8f, skyColor.b);
    } else {
        // Night - make top even darker
        glColor3f(skyColor.r * 0.7f, skyColor.g * 0.7f, skyColor.b * 0.9f);
    }
    
    glVertex2f(WINDOW_WIDTH, WINDOW_HEIGHT);
    glVertex2f(0, WINDOW_HEIGHT);
    glEnd();
    
    // Draw day/night elements in order
    drawSun();
    drawStars();
    drawMoon();
    
    // Draw ground
    glBegin(GL_QUADS);
    glColor3f(groundColor.r, groundColor.g, groundColor.b);
    glVertex2f(0, 0);
    glVertex2f(WINDOW_WIDTH, 0);
    glVertex2f(WINDOW_WIDTH, 30);
    glVertex2f(0, 30);
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
        
        // Display day/night status with time of day
        std::string timeStatus = getTimeOfDayStatus();
        drawText(timeStatus.c_str(), 10, WINDOW_HEIGHT - 70);
        
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
    glutCreateWindow("Flappy Bird - Smooth Day & Night Cycle");

    setup();
    initGame();

    glutDisplayFunc(display);
    glutKeyboardFunc(handleKeypress);

    glutMainLoop();
    return 0;
}