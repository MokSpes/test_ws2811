
#include "serial.h"


#define MIPIN 2


strip_WS2811 strip = strip_WS2811(10, 1);


void setup()
{
	// put your setup code here, to run once:
	pinMode(LED_BUILTIN, OUTPUT);
	digitalWrite(LED_BUILTIN, LOW);
	digitalWrite(MIPIN, LOW);
	pinMode(MIPIN, OUTPUT);

	Serial.begin(9600);
}


void loop(void)
{
//	todoon(0xFF, 0xFF, 0xFF, 1000);
//	todoon(0xFF, 0, 0, 1000);
//	todoon(0, 0xFF, 0, 1000);
//	todoon(0, 0, 0xFF, 1000);

//	theaterChase(0xFF,0xFF,0xFF,50);
//	theaterChase(0xFF,0x00,0x00,50);
//	theaterChase(0x00,0xFF,0x00,50);
//	theaterChase(0x00,0x00,0xFF,50);

	debug_hsv();
}

void debug_rgb(void)
{
	static int H = 0;
	static int S = 255;
	static int V = 255;

	while (Serial.available() > 0)
	{
		if ((Serial.peek() >= '0') && (Serial.peek() <= '9'))
			H = Serial.parseInt();
		else
		{
			uint8_t ch = Serial.read();
			if (ch == 'H' || ch == 'h') H = Serial.parseInt();
			else if (ch == 'S' || ch == 's') S = Serial.parseInt();
			else if (ch == 'V' || ch == 'v') V = Serial.parseInt();
		}
	
		if (Serial.read() == '\n')
		{
			Serial.print("H:"); Serial.print(H, DEC); Serial.print(" S:"); Serial.print(S, DEC);Serial.print(" V:"); Serial.print(V, DEC); Serial.print(" "); 
			strip.setPixelHSV(0, H, S, V);
			strip.show();
		}
	}
}

void debug_hsv(void)
{
	static int H = 0;
	static int S = 255;
	static int V = 255;

	while (Serial.available() > 0)
	{
		if (isDigit(Serial.peek()))
			H = Serial.parseInt();
		else
		{
			uint8_t ch = Serial.read();
			if (ch == 'H' || ch == 'h') H = Serial.parseInt();
			else if (ch == 'S' || ch == 's') S = Serial.parseInt();
			else if (ch == 'V' || ch == 'v') V = Serial.parseInt();
		}
	
		if (Serial.read() == '\n') // cuando llega el final de linea, ENTONCES se actualiza
		{
			uint16_t longitud = strip.getNumPixels(); uint16_t i;
			Serial.print("H:"); Serial.print(H, DEC); Serial.print(" S:"); Serial.print(S, DEC);Serial.print(" V:"); Serial.print(V, DEC); Serial.print(" "); 

			for(i = 0; i < longitud; i++) strip.setPixelHSV(i, H, S, V);

			strip.show();
		}
	}
}

void test_color(uint8_t hmin, uint8_t hmax, uint16_t wait)
{
	uint16_t longitud=strip.getNumPixels();
	uint8_t H = 0;

	for (H = hmin; H <= hmax; H++)
	{
		for(uint16_t i = 0; i < 1; i++)
			strip.setPixelHSV(i, H, 255, 255);
		strip.show();
		delay(wait);
	}

	for (H = hmax; H >= hmin; H--)
	{
		for(uint16_t i = 0; i < 1; i++)
			strip.setPixelHSV(i, H, 255, 255);
		strip.show();
		delay(wait);
	}
}



void todoon(uint8_t R, uint8_t G, uint8_t B, uint16_t wait)
{
	uint16_t longitud=strip.getNumPixels();

	for(uint16_t i = 0; i < longitud; i++) strip.setPixelRGB(i, R, G, B);

	strip.show();
	delay(wait);
}


//Theatre-style crawling lights.
void theaterChase(uint8_t R, uint8_t G, uint8_t B, uint8_t wait)
{
	uint16_t longitud = strip.getNumPixels();

	for (uint8_t j = 0; j < 10; j++)   //do 10 cycles of chasing
	{
		for (uint8_t q = 0; q < 4; q++)
		{
			for (uint8_t i = 0; i < longitud; i = i + 4) strip.setPixelRGB(i + q, R, G, B);  //turn every fourth pixel on

			strip.show();
			delay(wait);

			for (uint8_t i = 0; i < longitud; i = i + 4) strip.setPixelRGB(i + q, 0, 0, 0);      //turn every third pixel off
		}
	}
}
