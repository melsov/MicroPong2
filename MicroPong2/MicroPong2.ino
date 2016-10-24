/*
 Name:		MicroPong2.ino
 Created:	10/23/2016 12:21:45 PM
 Author:	melsov
*/

// the setup function runs once when you press reset or power the board
#include <space03.h>
#include <space02.h>
#include <space01.h>
#include <MicroView.h>
#include <fontlargenumber.h>
#include <font8x16.h>
#include <font5x7.h>
#include <7segment.h>


const int sensorPin = A1;
const float sensorMaxValue = 1024.0;

const int renderDelay = 16; // About 60hz
const int startDelay = 4000;
const int gameOverDelay = 6000;

const int scoreToWin = 3;
int playerScore = 0;
int enemyScore = 0;

const float paddleWidth = LCDWIDTH/16.0;
const float paddleHeight = LCDHEIGHT/3.0;
const float halfPaddleWidth = paddleWidth/2.0;
const float halfPaddleHeight = paddleHeight/2.0;

float playerPosX = 1.0 + halfPaddleWidth;
float playerPosY = 0.0;
float enemyPosX = LCDWIDTH - 1.0 - halfPaddleWidth;
float enemyPosY = 0.0;
float enemyVelY = 0.5;

float ballRadius = 2.0;
float ballSpeedX = 1.0;
float ballPosX = LCDWIDTH/2.0;
float ballPosY = LCDHEIGHT/2.0;
float ballVelX = -1.0 * ballSpeedX;
float ballVelY = 0;

float paddleSpeed = 1.0;
bool shouldStart = false;

/*
 * game modes
*/
void setGameMode(int mode) {
	if (mode == 0) //easy
	{ 
		enemyVelY = 0.4;
		paddleSpeed = .9;
		ballSpeedX = .8;
	}
	else if (mode == 1) // hard
	{
		enemyVelY = 0.6;
		paddleSpeed = 1.0;
		ballSpeedX = 1.1;
	}
	ballVelX = -1.0 * ballSpeedX;
}

void setup()
{
	initializeGraphics();
 	initializeInput();
	displayGameStart();
}

void resetGame()
{
	enemyScore = 0;
	playerScore = 0;
	enemyPosY = 0.0;
	ballPosX = LCDWIDTH/2.0;
	ballPosY = LCDHEIGHT/2.0;
	ballVelX = -1.0 * ballSpeedX;
	ballVelY = 0.0;

	shouldStart = false;
}

void initializeGraphics()
{
	uView.begin();
	uView.setFontType(0);
}

void initializeInput()
{
	digitalWrite(sensorPin, HIGH);
	pinMode(sensorPin, INPUT);
}

void displayGameStart()
{
	uView.clear(PAGE);
	renderString(20,10, "PONG");
	uView.display();
	delay(startDelay);
	uView.clear(PAGE);
	renderString(10,5, "Choose");
	renderString(10,17, "UP=hard");
	renderString(10,29, "DOWN=easy");
	uView.display();
	//delay(startDelay);
}

/*
 * Get game mode from user
*/
bool theyChoseAGameMode() 
{
	float jValue = digitalJoystickValue();
	if (abs(jValue) < .5) 
	{
		return false;
	}

	uView.clear(PAGE);
	if (jValue > 0.0) 
	{
		renderString(20, 10, "HARD");
		setGameMode(1);
	} 
	else 
	{
		renderString(20, 10, "EASY");
		setGameMode(0);
	}
	uView.display();
	delay(startDelay);
	return true;
}

void loop()
{
	if (!shouldStart) 
	{
		shouldStart = theyChoseAGameMode();
		delay(50);
		return;
	}
	updateGame();
	renderGame();
	
	if (playerScore >= scoreToWin)
	{
		gameOver(true);
	}
	else if (enemyScore >= scoreToWin)
	{
		gameOver(false);
	}
}

void updateGame()
{
	updatePlayer();
	updateEnemy();
	updateBall();
}

float clampPaddlePosY(float paddlePosY)
{
	float newPaddlePosY = paddlePosY;
	
	if (paddlePosY - halfPaddleHeight < 0)
	{
		newPaddlePosY = halfPaddleHeight;
	}
	else if (paddlePosY + halfPaddleHeight > LCDHEIGHT)
	{
		newPaddlePosY = LCDHEIGHT - halfPaddleHeight;
	}
	
	return newPaddlePosY;
}

/* 
Converts analog joystick readings into zero, one or negative one.
Getting accurate analog readings would require knowing the real values at the joystick home
position and y-min, y-max positions and scaling accordingly. Theses values could vary from
one device to the next. Digitizing seems much more headache free. 
*/
float digitalJoystickValue() {
	float knobValue = analogRead(sensorPin)/sensorMaxValue;
	knobValue = knobValue * 2.0f - 1.0f;	

	/*If we get a small knobValue, it's hard to tell if the player intended to move at all. 
	When the joystick used for testing was centered, not pushed either way,
	knobValue was around -.3 (oh no!). Therefore, ignore small knobValues. */

	if (abs(knobValue) < .5) 
	{
		return 0;
	} 
	else if (knobValue > 0.0) 
	{
		return 1.0;
	} 
	else 
	{
		return -1.0;
	}
}

void updatePlayer()
{
	float jValue = digitalJoystickValue();
	playerPosY += jValue * paddleSpeed;
	playerPosY = clampPaddlePosY(playerPosY);

	//playerPosY = clampPaddlePosY(knobValue * LCDHEIGHT); // Original code
}

void updateEnemy()
{
	// Prevent enemy paddle form jittering
	if (abs(enemyPosY - ballPosY) < enemyVelY) 
	{
		return;
	}
	
	// Follow the ball at a set speed
	if (enemyPosY < ballPosY)
	{
		enemyPosY += enemyVelY;
	}
	else if(enemyPosY > ballPosY)
	{
		enemyPosY -= enemyVelY;
	}
	
	enemyPosY = clampPaddlePosY(enemyPosY);
}

void updateBall()
{
	ballPosY += ballVelY;
	ballPosX += ballVelX;
	
	// Top and bottom wall collisions
	if (ballPosY < ballRadius)
	{
		ballPosY = ballRadius;
		ballVelY *= -1.0;
	}
	else if (ballPosY > LCDHEIGHT - ballRadius)
	{
		ballPosY = LCDHEIGHT - ballRadius;
		ballVelY *= -1.0;
	}
	
	// Left and right wall collisions
	if (ballPosX < ballRadius)
	{
		ballPosX = ballRadius;
		ballVelX = ballSpeedX;
		enemyScore++;
	}
	else if (ballPosX > LCDWIDTH - ballRadius)
	{
		ballPosX = LCDWIDTH - ballRadius;
		ballVelX *= -1.0 * ballSpeedX;
		playerScore++;
	}
	
	// Paddle collisions
	if (ballPosX < playerPosX + ballRadius + halfPaddleWidth)
	{
		if (ballPosY > playerPosY - halfPaddleHeight - ballRadius && 
			ballPosY < playerPosY + halfPaddleHeight + ballRadius)
		{
			ballVelX = ballSpeedX;
			ballVelY = 2.0 * (ballPosY - playerPosY) / halfPaddleHeight;
		}
	}
	else if (ballPosX > enemyPosX - ballRadius - halfPaddleWidth)
	{
		if (ballPosY > enemyPosY - halfPaddleHeight - ballRadius && 
			ballPosY < enemyPosY + halfPaddleHeight + ballRadius)
		{
			ballVelX = -1.0 * ballSpeedX;
			ballVelY = 2.0 * (ballPosY - enemyPosY) / halfPaddleHeight;
		}
	}
}

void renderGame()
{
	uView.clear(PAGE);
	
	renderScores(playerScore, enemyScore);
	renderPaddle(playerPosX, playerPosY);
	renderPaddle(enemyPosX, enemyPosY);
	renderBall(ballPosX, ballPosY);

	uView.display();
	delay(renderDelay);
}

void renderString(int x, int y, String string)
{
	uView.setCursor(x,y);
	uView.print(string);
}

void renderPaddle(int x, int y)
{
	uView.rect(
		x - halfPaddleWidth, 
		y - halfPaddleHeight, 
		paddleWidth, 
		paddleHeight);
}

void renderBall(int x, int y)
{
	uView.circle(x, y, 2);	
}

void renderScores(int firstScore, int secondScore)
{
	renderString(10, 0, String(firstScore));
	renderString(LCDWIDTH - 14, 0, String(secondScore));
}

void gameOver(bool didWin)
{
	if (didWin)
	{
		renderString(20,10, "You");
		renderString(20,30, "Win!");
	}
	else
	{
		renderString(20,10, "You");
		renderString(15,30, "Lose!");
	}
	
	uView.display();
	delay(gameOverDelay);
	
	// Get ready to start the game again.
    resetGame();
	displayGameStart();
}