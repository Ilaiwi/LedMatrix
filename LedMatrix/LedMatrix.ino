#include <SPI.h>// SPI Library used to clock data out to the shift registers
#include "Buffer.h"
#define latch_pin 2// can use any pin you want to latch the shift registers
#define blank_pin 4// same, can use any pin you want for this, just make sure you pull up via a 1k to 5V
#define data_pin 11// used by SPI, must be pin 11
#define clock_pin 13// used by SPI, must be 13
#define LedsInRow 10
#define LedsInColumn 10
#define PAM_resulition 4


union TenBits
{
	struct {
		unsigned bits : 10;
	};
};
//
//union Buffer {
//	struct
//	{
//		unsigned bits : 40;
//	};
//	struct
//	{
//		unsigned byte_0 : 8;
//		unsigned byte_1 : 8;
//		unsigned byte_2 : 8;
//		unsigned byte_3 : 8;
//		unsigned byte_4 : 8;
//	};
//	struct
//	{
//
//		unsigned blue : 10;
//		unsigned red : 10;
//		unsigned green : 10;
//		unsigned level : 10;
//	};
//};

enum State {  // <-- the use of typedef is optional
	notifications,
	snake,
	animation,
	music,
	touch,
};

enum notificationType {  // <-- the use of typedef is optional
	whatsapp,
	sms,
	facebook,
};

typedef struct Color {
	byte blue;
	byte green;
	byte red;
} Color;

// sanke game variabels

typedef struct Link {
	int x;
	int y;
	struct Link * next;
} Link;



Link * pHead = nullptr;
Link * pTail = nullptr;

int curDirection = 4;
int newDirection = 4;
int appleX = 5;
int appleY = 5;

int musicCurrent = 1;
int animationCurrent = 1;
int animationCount = 5;
int musicCount = 3;

unsigned long oldTimer, curTimer;
boolean dead = 1;
TenBits led[10];
unsigned long oldTimerAnimation, curTimerAnimation;
Color snakeColor;

//end snake game varibales
State state = notifications;
int notificationIndex=0;
notificationType notificationsList[25];
Color allLEDs[10][10];
String serialInput = "";
TenBits anode[10] = { 0b1000000000,0b0100000000,0b0010000000,0b0001000000,0b0000100000,0b0000010000 , 0b0000001000,0b0000000100,0b0000000010,0b0000000001 };
TenBits redArray[LedsInRow][PAM_resulition];
TenBits greenArray[LedsInRow][PAM_resulition];
TenBits blueArray[LedsInRow][PAM_resulition];
int level = 0;//this increments through the anode levels
int level_counter = 0;
int BAM_Bit, BAM_Counter = 0; // Bit Angle Modulation variables to keep track of things

void setup() {
	SPI.setBitOrder(MSBFIRST);//Most Significant Bit First
	SPI.setDataMode(SPI_MODE0);// Mode 0 Rising edge of data, keep clock low
	SPI.setClockDivider(SPI_CLOCK_DIV2);//Run the data in at 16MHz/2 - 8MHz
	noInterrupts();// kill interrupts until everybody is set up
				   //We use Timer 1 to refresh the cube
	TCCR1A = B00000000;//Register A all 0's since we're not toggling any pins
	TCCR1B = B00001011;//bit 3 set to place in CTC mode, will call an interrupt on a counter match
					   //bits 0 and 1 are set to divide the clock by 64, so 16MHz/64=250kHz
	TIMSK1 = B00000010;//bit 1 set to call the interrupt on an OCR1A match
	OCR1A = 10; // you can play with this, but I set it to 30, which means:
				//our clock runs at 250kHz, which is 1/250kHz = 4us
				//with OCR1A set to 30, this means the interrupt will be called every (30+1)x4us=124us, 
				// which gives a multiplex frequency of about 8kHz
				//finally set up the Outputs
	pinMode(latch_pin, OUTPUT);//Latch
	pinMode(data_pin, OUTPUT);//MOSI DATA
	pinMode(clock_pin, OUTPUT);//SPI Clock
	pinMode(blank_pin, OUTPUT);//Output Enable  important to do this last, so LEDs do not flash on boot up
	SPI.begin();//start up the SPI library

	//for snake game
	randomSeed(analogRead(4));
	setRandomSnakeColor();

	Serial.begin(9600);

	//  clearArrays();
	//  printColorArrays();


	//LED(0, 0, 0, 15, 0);
	//LED(1, 0, 15, 0, 0);
	//LED(1, 1, 0, 0, 15);
	//  LED(1,1,1,4,6);
	//  LED(2,2,1,4,6);
	//  LED(3,3,1,4,6);
	//  LED(4,4,1,4,6);
	//  LED(5,5,1,4,6);
	//  LED(6,6,1,4,6);
	//  LED(7,7,1,4,6);
	//  LED(8,8,1,4,6);
	//  LED(9,9,1,4,6);

	clrscr();
	printColorArrays();
	interrupts();//let the show begin, this lets the multiplexing start
}

void loop() {
	serialInput = Serial.readStringUntil('\n');
	if (serialInput == "anim") {
		Serial.println("anim");
		state = animation;
	}
	else if(serialInput == "noti")
	{
		clrscr();
		state = notifications;
	}
	else if (serialInput == "snake") {
			clrscr();
			state = snake;
	}
	//basicRowAnimation();
	else if (serialInput == "music") {
		clrscr();
		state = music;
	}
	else if (serialInput == "touch") {
		clrscr();
		state = touch;
	}
	switch (state)
	{

	case notifications:
		if (serialInput == "facebook") {
			notificationType notification = facebook;
			notificationsList[notificationIndex] = notification;
			notificationIndex++;
			Serial.println("facebook re");
			showNotificationsOnMatix();
		}
		else if (serialInput == "whatsapp")
		{
			notificationType notification = whatsapp;
			notificationsList[notificationIndex] = notification;
			notificationIndex++;
			Serial.println("whatsapp re");
			showNotificationsOnMatix();
		}
		else if (serialInput == "sms")
		{
			notificationType notification = sms;
			notificationsList[notificationIndex] = notification;
			notificationIndex++;
			Serial.println("sms re");
			showNotificationsOnMatix();
		}
		else if (serialInput=="call") {
			missedCallAnimation();
			missedCallAnimation();
			missedCallAnimation();
			missedCallAnimation();
			missedCallAnimation();
			missedCallAnimation();
			missedCallAnimation();
			missedCallAnimation();
			clrscr();

			showNotificationsOnMatix();

		}
		
		break;
	case snake :
		snakeInit();
		/*screenUpdate();*/
		oldTimer = millis();
		curTimer = millis();
		dead = 0;
		while (!dead) {
			curTimer = millis();
			int input = Serial.read();
			if (input == 113) {
				state = notifications;
				break;
			}
			setDirection(input);

			if (curTimer - oldTimer >= 320) {
				curDirection = newDirection;
				moveSnake(curDirection);
				screenUpdate();
				oldTimer = millis();
			}
		}
		basicRowAnimation();
		clrscr();
		break;
		//int musicCurrent = 1;
		//int animationCurrent = 1;
		//int animationCount = 3;
		//int musicCount = 3;
	case animation:
		if (serialInput == "next") {
			if (animationCurrent == animationCount) {
				animationCurrent = 1;
			}
			else {
				animationCurrent++;
			}
		}
		else if (serialInput == "prev") {
			if (animationCurrent == 1) {
				animationCurrent = animationCount;
			}
			else {
				animationCurrent--;
			}
			
		}
		play_acimation(animationCurrent);
		break;
	case music:
		if (serialInput == "next") {
			if (musicCurrent == musicCount) {
				musicCurrent = 1;
			}
			else {
				musicCurrent++;
			}
		}
		else if (serialInput == "prev") {
			if (musicCurrent == 1) {
				musicCurrent = musicCount;
			}
			else {
				musicCurrent--;
			}
		}
		mp3_play(musicCurrent);
		break;
	case touch:
		break;
	default:
		break;
	}

	//animation after dying
	

	/*clrscr();

	while (1) {
		curTimer = millis();
		if (curTimer - oldTimer >= 700) {
			for (int i = 0; i<10; i++) {
				led[i].bits = ~led[i].bits;
			}
			oldTimer = millis();
		}
		ledToMatrix();
	}*/

}




void LED(int row, int column, byte red, byte green, byte blue) { //****LED Routine****LED Routine****LED Routine****LED Routine
																 //  Serial.println("LED line");
																 //This is where it all starts
																 //This routine is how LEDs are updated, with the inputs for the LED location and its R G and B brightness levels

																 // First, check and make sure nothing went beyond the limits, just clamp things at either 0 or 7 for location, and 0 or 15 for brightness
																 //level must always be 0
																 //int level = 0;

	if (row<0)
		row = 0;
	if (row>9)
		row = 9;
	if (column<0)
		column = 0;
	if (column>9)
		column = 9;
	if (red<0)
		red = 0;
	if (red>15)
		red = 15;
	if (green<0)
		green = 0;
	if (green>15)
		green = 15;
	if (blue<0)
		blue = 0;
	if (blue>15)
		blue = 15;

	for (int i = 0; i < PAM_resulition; i++)
	{
		bitWrite(redArray[row][i].bits, column, bitRead(red, i));
	}

	for (int i = 0; i < PAM_resulition; i++)
	{
		bitWrite(greenArray[row][i].bits, column, bitRead(green, i));
	}

	for (int i = 0; i < PAM_resulition; i++)
	{
		bitWrite(blueArray[row][i].bits, column, bitRead(blue, i));
	}
	allLEDs[row][column].red = red;
	allLEDs[row][column].blue = blue;
	allLEDs[row][column].green = green;

}//****LED routine end****LED routine end****LED routine end****LED routine end****LED routine end****LED routine end****LED routine end

void printColorArrays() {
	Serial.println("red");
	for (int i = 0; i<LedsInRow; i++) {
		for (int y = 0; y<PAM_resulition; y++)
			Serial.println(redArray[i][y].bits, BIN);
	}
	Serial.println("blue");
	for (int i = 0; i<LedsInRow; i++) {
		for (int y = 0; y<PAM_resulition; y++)
			Serial.println(blueArray[i][y].bits, BIN);
	}
	Serial.println("green");
	for (int i = 0; i<LedsInRow; i++) {
		for (int y = 0; y<PAM_resulition; y++)
			Serial.println(greenArray[i][y].bits, BIN);
	}
	Serial.println("end");
}


void ClearArrays() {
	for (int i = 0; i<LedsInRow; i++) {
		for (int y = 0; y<PAM_resulition; y++)
			redArray[i][y].bits = 0;
	}
	for (int i = 0; i<LedsInRow; i++) {
		for (int y = 0; y<PAM_resulition; y++)
			blueArray[i][y].bits = 0;
	}
	for (int i = 0; i<LedsInRow; i++) {
		for (int y = 0; y<PAM_resulition; y++)
			greenArray[i][y].bits = 0;
	}
}


void mp3_play(int i) {
	
	switch (i)
	{
	case 1:
		play_acimation(i);
		break;
	case 2:
		play_acimation(i);
		break;
	case 3:
		play_acimation(i);
		break;
	default:
		break;
	}

}

void play_acimation(int i) {

	switch (i)
	{
	case 1:
		circleAnimation();
		circleAnimation();
		break;
	case 2:
		basicRowAnimation();
		basicRowAnimation();
		break;
	case 3:
		randomFullPanel();
		randomFullPanel();
		break;
	case 4:
		heartAnimation();
		heartAnimation();
		break;
	case 5:
		samerAnimation();
		samerAnimation();
		break;
	default:
		break;
	}

}

Buffer toBuffer(int level) {
	Buffer buffer;
	buffer.red = redArray[level][BAM_Bit].bits;
	buffer.green = greenArray[level][BAM_Bit].bits;
	buffer.blue = blueArray[level][BAM_Bit].bits;
	buffer.level = anode[level].bits;
	//  Serial.println("new write");
	//  Serial.println(buffer.blue,BIN);
	return buffer;
}

void transferBuffer(Buffer buffer) {
	//  Serial.println("new write");
	//  Serial.println(buffer.byte_0,BIN);
	SPI.transfer(buffer.byte_4);
	SPI.transfer(buffer.byte_3);
	SPI.transfer(buffer.byte_2);
	SPI.transfer(buffer.byte_1);
	SPI.transfer(buffer.byte_0);
}

ISR(TIMER1_COMPA_vect) {//***MultiPlex BAM***MultiPlex BAM***MultiPlex BAM***MultiPlex BAM***MultiPlex BAM***MultiPlex BAM***MultiPlex BAM
						//  Serial.print("int");
	BAM_Counter++;

	//This routine is called in the background automatically at frequency set by OCR1A
	//In this code, I set OCR1A to 30, so this is called every 124us, giving each level in the cube 124us of ON time
	//There are 8 levels, so we have a maximum brightness of 1/8, since the level must turn off before the next level is turned on
	//The frequency of the multiplexing is then 124us*8=992us, or 1/992us= about 1kHz


	PORTD |= 1 << blank_pin;//The first thing we do is turn all of the LEDs OFF, by writing a 1 to the blank pin
							//Note, in my bread-boarded version, I was able to move this way down in the cube, meaning that the OFF time was minimized
							//do to signal integrity and parasitic capcitance, my rise/fall times, required all of the LEDs to first turn off, before updating
							//otherwise you get a ghosting effect on the previous level

							//This is 4 bit 'Bit angle Modulation' or BAM, There are 8 levels, so when a '1' is written to the color brightness, 
							//each level will have a chance to light up for 1 cycle, the BAM bit keeps track of which bit we are modulating out of the 4 bits
							//Bam counter is the cycle count, meaning as we light up each level, we increment the BAM_Counter
	if (BAM_Counter == 1) {
		BAM_Bit++;
	}
	else if (BAM_Counter == 3) {
		BAM_Bit++;
	}
	else if (BAM_Counter == 7) {
		BAM_Bit++;
	}
	Buffer buffer = toBuffer(level);
	//  Serial.println("new write");
	//  Serial.println(buffer.bits,BIN);
	transferBuffer(buffer);
	if (BAM_Counter == 15) {
		BAM_Counter = 0;
		BAM_Bit = 0;
		level_counter++;
	}
	if (level_counter == 1) {
		//    Serial.println("levle");
		//    Serial.println(level);
		level++;
		level_counter = 0;
	}
	//  break;
	//}//switch_case

	//SPI.transfer(~anode[anodelevel]);//finally, send out the anode level byte

	PORTD |= 1 << latch_pin;//Latch pin HIGH
	PORTD &= 0 << latch_pin;//Latch pin LOW
	PORTD &= 0 << blank_pin;//Blank pin LOW to turn on the LEDs with the new data
							//Serial.println(~anode[anodelevel],BIN);
							//level = level+8;//increment the level variable by 8, which is used to shift out data, since the next level woudl be the next 8 bytes in the arrays
							//delay(1000);
	if (level == LedsInRow) {//go back to 0 if max is reached
		level = 0;

	}
	//if(level==8)//if you hit 64 on level, this means you just sent out all 63 bytes, so go back
	//level=0;
}//***MultiPlex BAM END***MultiPlex BAM END***MultiPlex BAM END***MultiPlex BAM END***MultiPlex BAM END***MultiPlex BAM END***MultiPlex BAM END


void addHead(int x, int y)
{
	Link *temp;
	temp = (Link*)malloc(sizeof(Link));

	// create new head
	temp->x = x;
	temp->y = y;
	temp->next = nullptr;

	if (pHead != nullptr)
		pHead->next = temp;

	// point to new head
	pHead = temp;
}

void snakeInit() {
	int x = 2;
	int y = 2;
	for (int i = 0; i<2; i++, x++) {
		addHead(x, y);
		if (i == 0)
			pTail = pHead;
	}
}

void setDirection(int input) {
		if (input == 119) {
			if (curDirection != 2)
				newDirection = 1;
		}
		if (input == 115) {
			if (curDirection != 1)
				newDirection = 2;
		}
		if (input ==100 ) {
			if (curDirection != 4)
				newDirection = 3;
		}
		if (input == 97) {
			if (curDirection != 3)
				newDirection = 4;
		}
		

}

void moveSnake(int direction)
{
	int newX = pHead->x;
	int newY = pHead->y;
	if (direction == 1)
		newY--;
	if (direction == 2)
		newY++;
	if (direction == 3)
		newX--;
	if (direction == 4)
		newX++;

	if (newX > 10)
		newX = 1;
	if (newX < 1)
		newX = 10;
	if (newY > 10)
		newY = 1;
	if (newY < 1)
		newY = 10;

	dead |= check(newX, newY);

	if (!dead) {
		if (newX == appleX && newY == appleY) {
			addHead(newX, newY);
			newApple();
		}
		else {
			Link *temp = pTail;

			// point to new tail
			pTail = pTail->next;

			// new head
			pHead->next = temp;
			pHead = temp;

			pHead->x = newX;
			pHead->y = newY;
			pHead->next = nullptr;
		}
	}
}

void newApple() {
	boolean check = 0;
	Link * ptr = pTail;
	do {
		check = 0;
		appleX = random(7) + 1;
		appleY = random(7) + 1;
		Serial.println(appleX);
		setRandomSnakeColor();
		while (ptr != nullptr) {
			if (appleX == (ptr->x) && appleY == (ptr->y)) {
				check = 1;
				break;
			}
			ptr = ptr->next;
		}
	} while (check == 1);
}

boolean check(int x, int y) {
	Link *ptr;
	ptr = pTail;
	while (ptr != nullptr)
	{
		if (x == ptr->x && y == ptr->y)
			return 1;
		ptr = ptr->next;
	}

	return 0;
}

void clrscr()
{
	ClearArrays();
	for (int i = 0; i<10; i++) {
		led[i].bits = 0b0000000000;
	}
}

void screenUpdate() {
	Link * ptr;
	ptr = pTail;

	clrscr();
	while (ptr != nullptr) {
		led[ptr->y - 1].bits = led[ptr->y - 1].bits | (1 << (10 - ptr->x));
		ptr = ptr->next;
	}
	for (int i = 0; i< 10; i++) {
		for (int y = 0; y < 10; y++)
		{
			if (bitRead(led[i].bits, y)) {
				LED(i, y, snakeColor.red, snakeColor.blue, snakeColor.green);
			}
		}
	}

	led[appleY - 1].bits = led[appleY - 1].bits | (1 << (10 - appleX));
	LED(appleY - 1, 10 - appleX, 15, 15, 15);
	//ledToMatrix();
}

void ledToMatrix() {

	for (int i = 0; i< 10; i++) {
		for (int y = 0; y < 10; y++)
		{
			if (bitRead(led[i].bits, y)) {
				LED(i, y, 0, 0, 7);
			}
		}
	}
}

void setRandomSnakeColor() {
	snakeColor.blue = random(15);
	snakeColor.red = random(15);
	snakeColor.green = random(15);

}

void basicRowAnimation() {
	oldTimerAnimation = millis();
	int count = 0;
	int rand_r, rand_g, rand_b;
	while (count<10) {
		rand_r = random(15);
		rand_g = random(15);
		rand_b = random(15);
		curTimerAnimation = millis();
		if (curTimerAnimation - oldTimerAnimation >= 200) {
			led[count].bits = 0b1111111111;
			oldTimerAnimation = millis();
			for (int y = 0; y < 10; y++)
			{
				LED(count, y, rand_r, rand_b, rand_g);
			}
			count++;
		}
		if (count == 10) {
			FadeLEDs();
		}
	}
}

void randomFullPanel() {
	ClearArrays();
	oldTimerAnimation = millis();
	int count = 0;
	int rand_r, rand_g, rand_b;
	while (count<10) {

		curTimerAnimation = millis();
		if (curTimerAnimation - oldTimerAnimation >= 200) {
			led[count].bits = 0b1111111111;
			oldTimerAnimation = millis();
			
			for (int y = 0; y < 10; y++)
			{
				rand_r = random(15);
				rand_g = random(15);
				rand_b = random(15);
				LED(count, y, rand_r, rand_g, rand_b);
			}
			count++;
		}


	}
	

}

void showNotificationsOnMatix() {
	int index_x = 0,index_y=0;
	for (int i = 0; i < notificationIndex; i++)
	{
		if (notificationsList[i] == facebook) {
			LED(index_y, index_x, 0, 0, 15);
			LED(index_y+1, index_x, 0, 0, 15);
			LED(index_y, index_x+1, 0, 0, 15);
			LED(index_y+1, index_x+1, 0, 0, 15);
			index_x = index_x + 2;
			if (index_x == 10) {
				index_y = index_y + 2;
				index_x = 0;
			}
		}
		if (notificationsList[i] == whatsapp) {
			LED(index_y, index_x, 0, 15, 0);
			LED(index_y + 1, index_x, 0, 15, 0);
			LED(index_y, index_x + 1, 0, 15, 0);
			LED(index_y + 1, index_x + 1, 0, 15, 0);
			index_x = index_x + 2;
			if (index_x == 10) {
				index_y = index_y + 2;
				index_x = 0;
			}
		}
		if (notificationsList[i] == sms) {
			LED(index_y, index_x, 15, 0, 0);
			LED(index_y + 1, index_x, 15, 0, 0);
			LED(index_y, index_x + 1, 15, 0, 0);
			LED(index_y + 1, index_x + 1, 15, 0, 0);
			index_x = index_x + 2;
			if (index_x == 10) {
				index_y = index_y + 2;
				index_x = 0;
			}
			
		}
	}
}

void circleAnimation() {
	clrscr();

	LED(4, 4, 15, 0, 0);
	LED(4, 5, 15, 0, 0);
	LED(5, 4, 15, 0, 0);
	LED(5, 5, 15, 0, 0);

	FadeLEDs();


	LED(2, 3, 15, 15, 0);
	LED(2, 4, 15, 15, 0);
	LED(2, 5, 15, 15, 0);
	LED(2, 6, 15, 15, 0);
	LED(3, 2, 15, 15, 0);
	LED(3, 7, 15, 15, 0);
	LED(4, 2, 15, 15, 0);
	LED(4, 4, 15, 0, 0);
	LED(4, 5, 15, 0, 0);
	LED(4, 7, 15, 15, 0);
	LED(5, 2, 15, 15, 0);
	LED(5, 3, 0, 0, 0);
	LED(5, 4, 15, 0, 0);
	LED(5, 5, 15, 0, 0);
	LED(5, 6, 0, 0, 0);
	LED(5, 7, 15, 15, 0);
	LED(5, 8, 0, 0, 0);
	LED(5, 9, 0, 0, 0);
	LED(6, 0, 0, 0, 0);
	LED(6, 1, 0, 0, 0);
	LED(6, 2, 15, 15, 0);
	LED(6, 7, 15, 15, 0);
	LED(7, 3, 15, 15, 0);
	LED(7, 4, 15, 15, 0);
	LED(7, 5, 15, 15, 0);
	LED(7, 6, 15, 15, 0);

	FadeLEDs();

	LED(0, 1, 0, 0, 0);
	LED(0, 2, 15, 0, 15);
	LED(0, 3, 15, 0, 15);
	LED(0, 4, 15, 0, 15);
	LED(0, 5, 15, 0, 15);
	LED(0, 6, 15, 0, 15);
	LED(0, 7, 15, 0, 15);
	LED(0, 8, 0, 0, 0);
	LED(0, 9, 0, 0, 0);
	LED(1, 0, 0, 0, 0);
	LED(1, 1, 15, 0, 15);
	LED(1, 2, 0, 0, 0);
	LED(1, 3, 0, 0, 0);
	LED(1, 4, 0, 0, 0);
	LED(1, 5, 0, 0, 0);
	LED(1, 6, 0, 0, 0);
	LED(1, 7, 0, 0, 0);
	LED(1, 8, 15, 0, 15);
	LED(1, 9, 0, 0, 0);
	LED(2, 0, 15, 0, 15);
	LED(2, 1, 0, 0, 0);
	LED(2, 2, 0, 0, 0);
	LED(2, 3, 15, 15, 0);
	LED(2, 4, 15, 15, 0);
	LED(2, 5, 15, 15, 0);
	LED(2, 6, 15, 15, 0);
	LED(2, 7, 0, 0, 0);
	LED(2, 8, 0, 0, 0);
	LED(2, 9, 15, 0, 15);
	LED(3, 0, 15, 0, 15);
	LED(3, 1, 0, 0, 0);
	LED(3, 2, 15, 15, 0);
	LED(3, 3, 0, 0, 0);
	LED(3, 4, 0, 0, 0);
	LED(3, 5, 0, 0, 0);
	LED(3, 6, 0, 0, 0);
	LED(3, 7, 15, 15, 0);
	LED(3, 8, 0, 0, 0);
	LED(3, 9, 15, 0, 15);
	LED(4, 0, 15, 0, 15);
	LED(4, 1, 0, 0, 0);
	LED(4, 2, 15, 15, 0);
	LED(4, 3, 0, 0, 0);
	LED(4, 4, 15, 0, 0);
	LED(4, 5, 15, 0, 0);
	LED(4, 6, 0, 0, 0);
	LED(4, 7, 15, 15, 0);
	LED(4, 8, 0, 0, 0);
	LED(4, 9, 15, 0, 15);
	LED(5, 0, 15, 0, 15);
	LED(5, 1, 0, 0, 0);
	LED(5, 2, 15, 15, 0);
	LED(5, 3, 0, 0, 0);
	LED(5, 4, 15, 0, 0);
	LED(5, 5, 15, 0, 0);
	LED(5, 6, 0, 0, 0);
	LED(5, 7, 15, 15, 0);
	LED(5, 8, 0, 0, 0);
	LED(5, 9, 15, 0, 15);
	LED(6, 0, 15, 0, 15);
	LED(6, 1, 0, 0, 0);
	LED(6, 2, 15, 15, 0);
	LED(6, 3, 0, 0, 0);
	LED(6, 4, 0, 0, 0);
	LED(6, 5, 0, 0, 0);
	LED(6, 6, 0, 0, 0);
	LED(6, 7, 15, 15, 0);
	LED(6, 8, 0, 0, 0);
	LED(6, 9, 15, 0, 15);
	LED(7, 0, 15, 0, 15);
	LED(7, 1, 0, 0, 0);
	LED(7, 2, 0, 0, 0);
	LED(7, 3, 15, 15, 0);
	LED(7, 4, 15, 15, 0);
	LED(7, 5, 15, 15, 0);
	LED(7, 6, 15, 15, 0);
	LED(7, 7, 0, 0, 0);
	LED(7, 8, 0, 0, 0);
	LED(7, 9, 15, 0, 15);
	LED(8, 0, 0, 0, 0);
	LED(8, 1, 15, 0, 15);
	LED(8, 2, 0, 0, 0);
	LED(8, 3, 0, 0, 0);
	LED(8, 4, 0, 0, 0);
	LED(8, 5, 0, 0, 0);
	LED(8, 6, 0, 0, 0);
	LED(8, 7, 0, 0, 0);
	LED(8, 8, 15, 0, 15);
	LED(8, 9, 0, 0, 0);
	LED(9, 0, 0, 0, 0);
	LED(9, 1, 0, 0, 0);
	LED(9, 2, 15, 0, 15);
	LED(9, 3, 15, 0, 15);
	LED(9, 4, 15, 0, 15);
	LED(9, 5, 15, 0, 15);
	LED(9, 6, 15, 0, 15);
	LED(9, 7, 15, 0, 15);
	LED(9, 8, 0, 0, 0);
	LED(9, 9, 0, 0, 0);

	FadeLEDs();

}


void allLEDsToMatix() {

	for (int i = 0; i < 10; i++)
	{
		for (int y = 0; y < 10; y++)
		{
			LED(i, y, allLEDs[i][y].red, allLEDs[i][y].green, allLEDs[i][y].blue);
		}
	}
}

void FadeLEDs() {
	for (int x = 0; x < 10; x++)
	{
		for (int i = 0; i < 10; i++)
		{
			for (int y = 0; y < 10; y++)
			{
				
						allLEDs[i][y].red= (int)(allLEDs[i][y].red*((10-x)/10.0));
						allLEDs[i][y].blue= (int)(allLEDs[i][y].blue*((10 - x) / 10.0));
						allLEDs[i][y].green= (int)(allLEDs[i][y].green*((10 - x) / 10.0));
				}
			}	
		allLEDsToMatix();
		delay(20);

	}
		
}



void missedCallAnimation() {

	clrscr();

	LED(1, 4, 2, 6, 0);
	LED(1, 5, 2, 6, 0);
	LED(1, 6, 2, 6, 0);
	LED(2, 3, 2, 6, 0);
	LED(2, 7, 2, 6, 0);
	LED(3, 3, 1, 6, 0);
	LED(3, 7, 2, 6, 0);
	LED(4, 3, 7, 9, 15);
	LED(4, 4, 7, 9, 15);
	LED(4, 5, 7, 9, 15);
	LED(4, 6, 7, 9, 15);
	LED(5, 4, 7, 9, 15);
	LED(5, 7, 7, 9, 15);
	LED(6, 3, 7, 9, 15);
	LED(6, 4, 7, 9, 15);
	LED(6, 5, 7, 9, 15);
	LED(6, 6, 7, 9, 15);
	LED(7, 3, 14, 6, 0);
	LED(7, 4, 14, 6, 0);
	LED(7, 5, 14, 6, 0);
	LED(7, 6, 14, 6, 0);
	LED(7, 7, 14, 6, 0);
	LED(8, 3, 14, 6, 0);
	LED(9, 3, 14, 6, 0);


	FadeLEDs();

	LED(1, 4, 2, 6, 0);
	LED(1, 5, 2, 6, 0);
	LED(1, 6, 2, 6, 0);
	LED(2, 3, 2, 6, 0);
	LED(2, 7, 2, 6, 0);
	LED(3, 3, 1, 6, 0);
	LED(3, 7, 2, 6, 0);
	LED(4, 3, 7, 9, 15);
	LED(4, 4, 7, 9, 15);
	LED(4, 5, 7, 9, 15);
	LED(4, 6, 7, 9, 15);
	LED(5, 4, 7, 9, 15);
	LED(5, 7, 7, 9, 15);
	LED(6, 3, 7, 9, 15);
	LED(6, 4, 7, 9, 15);
	LED(6, 5, 7, 9, 15);
	LED(6, 6, 7, 9, 15);
	LED(7, 3, 14, 6, 0);
	LED(7, 4, 14, 6, 0);
	LED(7, 5, 14, 6, 0);
	LED(7, 6, 14, 6, 0);
	LED(7, 7, 14, 6, 0);
	LED(8, 3, 14, 6, 0);
	LED(9, 3, 14, 6, 0);


	FadeLEDs();

	LED(1, 0, 13, 5, 6);
	LED(1, 2, 13, 5, 6);
	LED(1, 3, 13, 5, 6);
	LED(1, 4, 13, 5, 6);
	LED(1, 6, 13, 15, 13);
	LED(1, 7, 13, 15, 13);
	LED(1, 8, 13, 15, 13);
	LED(1, 9, 13, 15, 13);
	LED(2, 0, 13, 5, 6);
	LED(2, 2, 13, 5, 6);
	LED(2, 4, 13, 5, 6);
	LED(2, 9, 13, 15, 13);
	LED(3, 0, 13, 5, 6);
	LED(3, 1, 13, 5, 6);
	LED(3, 2, 13, 5, 6);
	LED(3, 4, 13, 5, 6);
	LED(3, 6, 13, 15, 13);
	LED(3, 7, 13, 15, 13);
	LED(3, 8, 13, 15, 13);
	LED(4, 9, 13, 15, 13);
	LED(5, 0, 13, 5, 2);
	LED(5, 2, 13, 5, 2);
	LED(5, 3, 13, 5, 2);
	LED(5, 4, 13, 5, 2);
	LED(5, 6, 13, 15, 13);
	LED(5, 7, 13, 15, 13);
	LED(5, 8, 13, 15, 13);
	LED(5, 9, 13, 15, 13);
	LED(6, 0, 13, 5, 2);
	LED(6, 2, 13, 5, 2);
	LED(6, 4, 13, 5, 2);
	LED(7, 0, 13, 5, 2);
	LED(7, 1, 13, 5, 2);
	LED(7, 2, 13, 5, 2);
	LED(7, 4, 13, 5, 2);
	LED(8, 6, 13, 5, 13);
	LED(8, 7, 13, 5, 13);
	LED(8, 8, 13, 5, 13);
	LED(8, 9, 13, 5, 13);
	
	FadeLEDs();

	LED(1, 0, 13, 5, 6);
	LED(1, 2, 13, 5, 6);
	LED(1, 3, 13, 5, 6);
	LED(1, 4, 13, 5, 6);
	LED(1, 6, 13, 15, 13);
	LED(1, 7, 13, 15, 13);
	LED(1, 8, 13, 15, 13);
	LED(1, 9, 13, 15, 13);
	LED(2, 0, 13, 5, 6);
	LED(2, 2, 13, 5, 6);
	LED(2, 4, 13, 5, 6);
	LED(2, 9, 13, 15, 13);
	LED(3, 0, 13, 5, 6);
	LED(3, 1, 13, 5, 6);
	LED(3, 2, 13, 5, 6);
	LED(3, 4, 13, 5, 6);
	LED(3, 6, 13, 15, 13);
	LED(3, 7, 13, 15, 13);
	LED(3, 8, 13, 15, 13);
	LED(4, 9, 13, 15, 13);
	LED(5, 0, 13, 5, 2);
	LED(5, 2, 13, 5, 2);
	LED(5, 3, 13, 5, 2);
	LED(5, 4, 13, 5, 2);
	LED(5, 6, 13, 15, 13);
	LED(5, 7, 13, 15, 13);
	LED(5, 8, 13, 15, 13);
	LED(5, 9, 13, 15, 13);
	LED(6, 0, 13, 5, 2);
	LED(6, 2, 13, 5, 2);
	LED(6, 4, 13, 5, 2);
	LED(7, 0, 13, 5, 2);
	LED(7, 1, 13, 5, 2);
	LED(7, 2, 13, 5, 2);
	LED(7, 4, 13, 5, 2);
	LED(8, 6, 13, 5, 13);
	LED(8, 7, 13, 5, 13);
	LED(8, 8, 13, 5, 13);
	LED(8, 9, 13, 5, 13);

	FadeLEDs();

}


void heartAnimation() {
	
	clrscr();

	LED(0, 5, 15, 0, 0);
	LED(0, 6, 15, 0, 0);
	LED(0, 7, 15, 0, 0);
	LED(0, 8, 15, 0, 0);
	LED(1, 4, 15, 0, 0);
	LED(1, 9, 15, 0, 0);
	LED(2, 3, 15, 0, 0);
	LED(2, 9, 15, 0, 0);
	LED(3, 2, 15, 0, 0);
	LED(3, 8, 15, 0, 0);
	LED(4, 1, 15, 0, 0);
	LED(4, 7, 15, 0, 0);
	LED(5, 1, 15, 0, 0);
	LED(5, 7, 15, 0, 0);
	LED(6, 2, 15, 0, 0);
	LED(6, 8, 15, 0, 0);
	LED(7, 3, 15, 0, 0);
	LED(7, 9, 15, 0, 0);
	LED(8, 4, 15, 0, 0);
	LED(8, 9, 15, 0, 0);
	LED(9, 5, 15, 0, 0);
	LED(9, 6, 15, 0, 0);
	LED(9, 7, 15, 0, 0);
	LED(9, 8, 15, 0, 0);

	FadeLEDs();
	LED(0, 5, 15, 0, 0);
	LED(0, 6, 15, 0, 0);
	LED(0, 7, 15, 0, 0);
	LED(0, 8, 15, 0, 0);
	LED(1, 4, 15, 0, 0);
	LED(1, 5, 15, 6, 6);
	LED(1, 6, 15, 6, 6);
	LED(1, 7, 15, 6, 6);
	LED(1, 8, 15, 6, 6);
	LED(1, 9, 15, 0, 0);
	LED(2, 3, 15, 0, 0);
	LED(2, 4, 15, 6, 6);
	LED(2, 8, 15, 6, 6);
	LED(2, 9, 15, 0, 0);
	LED(3, 2, 15, 0, 0);
	LED(3, 3, 15, 6, 6);
	LED(3, 7, 15, 6, 6);
	LED(3, 8, 15, 0, 0);
	LED(4, 1, 15, 0, 0);
	LED(4, 2, 15, 6, 6);
	LED(4, 6, 15, 6, 6);
	LED(4, 7, 15, 0, 0);
	LED(5, 1, 15, 0, 0);
	LED(5, 2, 15, 6, 6);
	LED(5, 6, 15, 6, 6);
	LED(5, 7, 15, 0, 0);
	LED(6, 2, 15, 0, 0);
	LED(6, 3, 15, 6, 6);
	LED(6, 7, 15, 6, 6);
	LED(6, 8, 15, 0, 0);
	LED(7, 3, 15, 0, 0);
	LED(7, 4, 15, 6, 6);
	LED(7, 8, 15, 6, 6);
	LED(7, 9, 15, 0, 0);
	LED(8, 4, 15, 0, 0);
	LED(8, 5, 15, 6, 6);
	LED(8, 6, 15, 6, 6);
	LED(8, 7, 15, 6, 6);
	LED(8, 8, 15, 6, 6);
	LED(8, 9, 15, 0, 0);
	LED(9, 5, 15, 0, 0);
	LED(9, 6, 15, 0, 0);
	LED(9, 7, 15, 0, 0);
	LED(9, 8, 15, 0, 0);

	FadeLEDs();
	LED(0, 5, 15, 0, 0);
	LED(0, 6, 15, 0, 0);
	LED(0, 7, 15, 0, 0);
	LED(0, 8, 15, 0, 0);
	LED(1, 4, 15, 0, 0);
	LED(1, 5, 15, 6, 6);
	LED(1, 6, 15, 6, 6);
	LED(1, 7, 15, 6, 6);
	LED(1, 8, 15, 6, 6);
	LED(1, 9, 15, 0, 0);
	LED(2, 3, 15, 0, 0);
	LED(2, 4, 15, 6, 6);
	LED(2, 5, 7, 0, 0);
	LED(2, 6, 7, 0, 0);
	LED(2, 7, 7, 0, 0);
	LED(2, 8, 15, 6, 6);
	LED(2, 9, 15, 0, 0);
	LED(3, 2, 15, 0, 0);
	LED(3, 3, 15, 6, 6);
	LED(3, 4, 7, 0, 0);
	LED(3, 6, 7, 0, 0);
	LED(3, 7, 15, 6, 6);
	LED(3, 8, 15, 0, 0);
	LED(4, 1, 15, 0, 0);
	LED(4, 2, 15, 6, 6);
	LED(4, 3, 7, 0, 0);
	LED(4, 5, 7, 0, 0);
	LED(4, 6, 15, 6, 6);
	LED(4, 7, 15, 0, 0);
	LED(5, 1, 15, 0, 0);
	LED(5, 2, 15, 6, 6);
	LED(5, 3, 7, 0, 0);
	LED(5, 5, 7, 0, 0);
	LED(5, 6, 15, 6, 6);
	LED(5, 7, 15, 0, 0);
	LED(6, 2, 15, 0, 0);
	LED(6, 3, 15, 6, 6);
	LED(6, 4, 7, 0, 0);
	LED(6, 6, 7, 0, 0);
	LED(6, 7, 15, 6, 6);
	LED(6, 8, 15, 0, 0);
	LED(7, 3, 15, 0, 0);
	LED(7, 4, 15, 6, 6);
	LED(7, 5, 7, 0, 0);
	LED(7, 6, 7, 0, 0);
	LED(7, 7, 7, 0, 0);
	LED(7, 8, 15, 6, 6);
	LED(7, 9, 15, 0, 0);
	LED(8, 4, 15, 0, 0);
	LED(8, 5, 15, 6, 6);
	LED(8, 6, 15, 6, 6);
	LED(8, 7, 15, 6, 6);
	LED(8, 8, 15, 6, 6);
	LED(8, 9, 15, 0, 0);
	LED(9, 5, 15, 0, 0);
	LED(9, 6, 15, 0, 0);
	LED(9, 7, 15, 0, 0);
	LED(9, 8, 15, 0, 0);

	FadeLEDs();
	LED(0, 5, 15, 0, 0);
	LED(0, 6, 15, 0, 0);
	LED(0, 7, 15, 0, 0);
	LED(0, 8, 15, 0, 0);
	LED(1, 4, 15, 0, 0);
	LED(1, 5, 15, 6, 6);
	LED(1, 6, 15, 6, 6);
	LED(1, 7, 15, 6, 6);
	LED(1, 8, 15, 6, 6);
	LED(1, 9, 15, 0, 0);
	LED(2, 3, 15, 0, 0);
	LED(2, 4, 15, 6, 6);
	LED(2, 5, 7, 0, 0);
	LED(2, 6, 7, 0, 0);
	LED(2, 7, 7, 0, 0);
	LED(2, 8, 15, 6, 6);
	LED(2, 9, 15, 0, 0);
	LED(3, 2, 15, 0, 0);
	LED(3, 3, 15, 6, 6);
	LED(3, 4, 7, 0, 0);
	LED(3, 5, 2, 4, 10);
	LED(3, 6, 7, 0, 0);
	LED(3, 7, 15, 6, 6);
	LED(3, 8, 15, 0, 0);
	LED(4, 1, 15, 0, 0);
	LED(4, 2, 15, 6, 6);
	LED(4, 3, 7, 0, 0);
	LED(4, 4, 2, 4, 10);
	LED(4, 5, 7, 0, 0);
	LED(4, 6, 15, 6, 6);
	LED(4, 7, 15, 0, 0);
	LED(5, 1, 15, 0, 0);
	LED(5, 2, 15, 6, 6);
	LED(5, 3, 7, 0, 0);
	LED(5, 4, 2, 4, 10);
	LED(5, 5, 7, 0, 0);
	LED(5, 6, 15, 6, 6);
	LED(5, 7, 15, 0, 0);
	LED(6, 2, 15, 0, 0);
	LED(6, 3, 15, 6, 6);
	LED(6, 4, 7, 0, 0);
	LED(6, 5, 2, 4, 10);
	LED(6, 6, 7, 0, 0);
	LED(6, 7, 15, 6, 6);
	LED(6, 8, 15, 0, 0);
	LED(7, 3, 15, 0, 0);
	LED(7, 4, 15, 6, 6);
	LED(7, 5, 7, 0, 0);
	LED(7, 6, 7, 0, 0);
	LED(7, 7, 7, 0, 0);
	LED(7, 8, 15, 6, 6);
	LED(7, 9, 15, 0, 0);
	LED(8, 4, 15, 0, 0);
	LED(8, 5, 15, 6, 6);
	LED(8, 6, 15, 6, 6);
	LED(8, 7, 15, 6, 6);
	LED(8, 8, 15, 6, 6);
	LED(8, 9, 15, 0, 0);
	LED(9, 5, 15, 0, 0);
	LED(9, 6, 15, 0, 0);
	LED(9, 7, 15, 0, 0);
	LED(9, 8, 15, 0, 0);

	FadeLEDs();



}

void samerAnimation() {

	clrscr();


	LED(2, 0, 5, 9, 0);
	LED(2, 4, 5, 9, 0);
	LED(2, 5, 5, 9, 0);
	LED(2, 6, 5, 9, 0);
	LED(2, 7, 5, 9, 0);
	LED(3, 0, 5, 9, 0);
	LED(3, 4, 5, 9, 0);
	LED(3, 8, 5, 9, 0);
	LED(4, 0, 5, 9, 0);
	LED(4, 4, 5, 9, 0);
	LED(4, 8, 5, 9, 0);
	LED(5, 0, 5, 9, 0);
	LED(5, 4, 5, 9, 0);
	LED(5, 8, 5, 9, 0);
	LED(6, 0, 5, 9, 0);
	LED(6, 4, 5, 9, 0);
	LED(6, 8, 5, 9, 0);
	LED(7, 0, 5, 9, 0);
	LED(7, 3, 5, 9, 0);
	LED(7, 4, 5, 9, 0);
	LED(7, 8, 5, 9, 0);
	LED(8, 1, 5, 9, 0);
	LED(8, 2, 5, 9, 0);
	LED(8, 3, 5, 9, 0);
	LED(8, 8, 5, 9, 0);



	FadeLEDs();

	LED(0, 0, 12, 8, 5);
	LED(0, 1, 12, 8, 5);
	LED(0, 2, 12, 8, 5);
	LED(0, 3, 12, 8, 5);
	LED(1, 4, 12, 8, 5);
	LED(1, 5, 12, 8, 5);
	LED(2, 4, 12, 3, 5);
	LED(2, 6, 12, 8, 5);
	LED(2, 7, 12, 8, 5);
	LED(3, 4, 12, 3, 5);
	LED(3, 8, 12, 8, 5);
	LED(4, 4, 12, 3, 5);
	LED(4, 9, 12, 8, 5);
	LED(5, 4, 12, 3, 5);
	LED(5, 9, 12, 8, 5);
	LED(6, 4, 12, 3, 5);
	LED(6, 8, 12, 8, 5);
	LED(7, 4, 12, 3, 5);
	LED(7, 6, 12, 8, 5);
	LED(7, 7, 12, 8, 5);
	LED(8, 4, 12, 8, 5);
	LED(8, 5, 12, 8, 5);
	LED(9, 0, 12, 8, 5);
	LED(9, 1, 12, 8, 5);
	LED(9, 2, 12, 8, 5);
	LED(9, 3, 12, 8, 5);


	FadeLEDs();

	LED(0, 0, 9, 3, 5);
	LED(0, 1, 9, 3, 5);
	LED(0, 2, 9, 3, 5);
	LED(0, 3, 9, 3, 5);
	LED(1, 4, 9, 3, 5);
	LED(1, 5, 9, 3, 5);
	LED(1, 6, 9, 3, 5);
	LED(1, 7, 9, 3, 5);
	LED(2, 7, 9, 3, 5);
	LED(2, 8, 9, 3, 5);
	LED(3, 4, 9, 3, 5);
	LED(3, 5, 9, 3, 5);
	LED(3, 6, 9, 3, 5);
	LED(3, 7, 9, 3, 5);
	LED(4, 0, 9, 3, 5);
	LED(4, 1, 9, 3, 5);
	LED(4, 2, 9, 3, 5);
	LED(4, 3, 9, 3, 5);
	LED(4, 4, 9, 3, 5);
	LED(5, 5, 9, 3, 5);
	LED(5, 6, 9, 3, 5);
	LED(5, 7, 9, 3, 5);
	LED(6, 7, 9, 3, 5);
	LED(6, 8, 9, 3, 5);
	LED(7, 5, 9, 3, 5);
	LED(7, 6, 9, 3, 5);
	LED(7, 7, 9, 3, 5);
	LED(8, 0, 9, 3, 5);
	LED(8, 1, 9, 3, 5);
	LED(8, 2, 9, 3, 5);
	LED(8, 3, 9, 3, 5);
	LED(8, 4, 9, 3, 5);
	LED(8, 5, 9, 3, 5);


	FadeLEDs();

	LED(2, 0, 3, 11, 5);
	LED(2, 1, 3, 11, 5);
	LED(2, 2, 3, 11, 5);
	LED(2, 3, 3, 11, 5);
	LED(2, 4, 3, 11, 5);
	LED(2, 5, 3, 11, 5);
	LED(2, 6, 3, 11, 5);
	LED(2, 7, 3, 11, 5);
	LED(2, 8, 3, 11, 5);
	LED(3, 0, 3, 11, 5);
	LED(3, 4, 3, 11, 5);
	LED(3, 8, 3, 11, 5);
	LED(4, 0, 3, 11, 5);
	LED(4, 4, 3, 11, 5);
	LED(4, 8, 3, 11, 5);
	LED(5, 0, 3, 11, 5);
	LED(5, 4, 3, 11, 5);
	LED(5, 8, 3, 11, 5);
	LED(6, 0, 3, 11, 5);
	LED(6, 4, 3, 11, 5);
	LED(6, 8, 3, 11, 5);
	LED(7, 0, 3, 11, 5);
	LED(7, 4, 3, 11, 5);
	LED(7, 8, 3, 11, 5);
	LED(8, 0, 3, 11, 5);
	LED(8, 8, 3, 11, 5);


	FadeLEDs();

	LED(2, 0, 15, 7, 15);
	LED(2, 1, 15, 7, 15);
	LED(2, 2, 15, 7, 15);
	LED(2, 3, 15, 7, 15);
	LED(2, 4, 15, 7, 15);
	LED(2, 5, 15, 7, 15);
	LED(2, 6, 15, 7, 15);
	LED(2, 7, 15, 7, 15);
	LED(2, 8, 15, 7, 15);
	LED(3, 4, 15, 7, 15);
	LED(3, 8, 15, 7, 15);
	LED(4, 4, 15, 7, 15);
	LED(4, 5, 15, 7, 15);
	LED(4, 8, 15, 7, 15);
	LED(5, 2, 15, 7, 15);
	LED(5, 3, 15, 7, 15);
	LED(5, 5, 15, 7, 15);
	LED(5, 8, 15, 7, 15);
	LED(6, 1, 15, 7, 15);
	LED(6, 5, 15, 7, 15);
	LED(6, 8, 15, 7, 15);
	LED(7, 0, 15, 7, 15);
	LED(7, 1, 15, 7, 15);
	LED(7, 5, 15, 7, 15);
	LED(7, 6, 15, 7, 15);
	LED(7, 7, 15, 7, 15);
	LED(7, 8, 15, 7, 15);
	LED(8, 0, 15, 7, 15);


	FadeLEDs();




}