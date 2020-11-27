
// se ha de definir el PIN (arduino) donde se ha conectado el string. Ahora de momento es pin2 -> PORT

#define STRIP_WS2811_PIN 2
#define STRIP_WS2811_PORT PORTD


// protocolo serie


class strip_WS2811
{
public:
	strip_WS2811(uint16_t lon, uint8_t formato);
	~strip_WS2811();

	clean();
	show();
	setPixelRGB(uint16_t num, uint8_t R, uint8_t G, uint8_t B);
	setPixelHSV(uint16_t num, uint8_t H, uint8_t S, uint8_t V);
	uint16_t getNumPixels();

protected:
	uint16_t numpixels = 0;
	uint16_t numbytes = 0; // numero de bytes del array, para no tener de calcularlo cada vez
	uint8_t *pixels = NULL;

	// diferentes fabricantes de leds conectan de diferentes maneras los colores al chip de control
	uint8_t offset_R = 0;
	uint8_t offset_G = 1;
	uint8_t offset_B = 2;
};
