
#include "Arduino.h"
#include "serial.h"



/**
 * lon: numpixels de la tira de pixels
 * pin: pin del micro donde esta conectado la tira de leds
 */

strip_WS2811::strip_WS2811(uint16_t lon, uint8_t formato)
{
	// TODO: comprobar que LON no sea demasiado grande
	numpixels = lon;
	numbytes = 3 * lon;
	pixels = (uint8_t *)malloc(numbytes);
	// TODO: comprobar que malloc() no falla
	clean();

	switch (formato)
	{
	case 0: // RGB
		offset_R = 0;
		offset_G = 1;
		offset_B = 2;
		break;

	case 1: // GRB
		offset_R = 1;
		offset_G = 0;
		offset_B = 2;
		break;
	}
}

strip_WS2811::~strip_WS2811(void)
{
	free(pixels);
}

uint16_t strip_WS2811::getNumPixels(void)
{
	return (pixels) ? numpixels : 0; // minimo control de si está inicializada o no
}

strip_WS2811::clean(void)
{
	if (pixels) memset(pixels, 0x00, numbytes);
}

strip_WS2811::show(void)
{
	// si la numpixels es cero, ya ni entra
	if (pixels)
	{
		register volatile uint8_t hi, lo, cb, dd;

		// se asume que no hay interrupciones, por lo que nadie modificar� el contenido del PORTx
		// trama a 800 KHz

		asm volatile (
"		in %[hi],%[port]		\n" //
"		ori %[hi],%[pin]		\n" //
"		in %[lo],%[port]		\n" //
"		andi %[lo],~%[pin]		\n" //
"		ldi %[cb],8 			\n" //
"		ld %[dd],%a[ptr]+		\n" //			carga el primero byte
"		cli 					\n" //			no se permiten interrupciones durante el envio
"Lx00:							\n" //
"		rjmp .+0				\n" // 2
"		rjmp .+0				\n" // 2
"		rjmp .+0				\n" // 2
"Lx01:							\n"
"		nop 					\n" // 1
"		out %[port],%[hi]		\n" // 1		empezar a enviar un bit
"		rjmp .+0				\n" // 2
"		nop 					\n" // 1
"		sbrs %[dd],7			\n" // 2 / 1	el bit a enviar esta b7
"		out %[port],%[lo]		\n" // 0 / 1	si era "0" se ha hecho el pulso corto (5 ciclos)
"		rjmp .+0				\n" // 2
"		rol %[dd]				\n" // 1		b6 -> b7
"		dec %[cb]				\n" // 1		contar un bit enviado
"		out %[port],%[lo]		\n" // 1		si era "1" se ha hecho el pulso largo (10 ciclos)
"		brne Lx00				\n" // 2 / 1	si no es cero:enviar el siguiente bit, si es cero:cargar el siguiente byte
"		ldi %[cb],8 			\n" // 1		8 bits por byte
"		ld %[dd],%a[ptr]+		\n" // 2		carga el siguiente byte
"		sbiw %[cnt],1			\n" // 2		contar 1 byte enviado
"		brne Lx01				\n" // 2 / 1	si no es cero envia el siguiente byte
"		sei 					\n" //			se vuelven a permitir interrupciones
		: [cb] "+r"(cb), [dd] "+r"(dd)
		: [ptr] "e"(pixels), [cnt] "w"(numbytes), [port] "I"(_SFR_IO_ADDR(STRIP_WS2811_PORT)), [pin] "I"(1<<STRIP_WS2811_PIN), [hi] "r"(hi), [lo] "r"(lo)
		);
		delayMicroseconds(300);
	}
}


strip_WS2811::setPixelRGB(uint16_t num, uint8_t R, uint8_t G, uint8_t B)
{
	if (pixels && (num < numpixels))
	{
		uint8_t *p = pixels + 3*num;

		*(p + offset_R) = R;
		*(p + offset_G) = G;
		*(p + offset_B) = B;
	}
}


/**
 * H: 0..255 -> 0�..359�
 * S: 0..255 -> 0..100%
 * V: 0..255 -> 0..100%
 */
strip_WS2811::setPixelHSV(uint16_t num, uint8_t H, uint8_t S, uint8_t V)
{
	if (pixels && (num < numpixels))
	{
		uint8_t *p = pixels + 3*num;

		// V es el valor máximo de la conversion, S se recalcula como el valor mínimo de la conversión
		// max = V
		// min = V * (255 - S) / 255;

		uint8_t mx = V;
		uint8_t mn = (uint16_t)(V * (255 - S)) / 255;

		if (H < 43)			// H [0..42] < 60dg
		{
			*(p + offset_R) = mx;
			*(p + offset_G) = (uint16_t)((mx - mn) * (H - 0)) / 43 + mn;
			*(p + offset_B) = mn;
		}
		else if (H < 85)	// H [43..84] < 120dg
		{
			*(p + offset_R) = mx - (uint16_t)((mx - mn) * (H - 43)) / 42;
			*(p + offset_G) = mx;
			*(p + offset_B) = mn;
		}
		else if (H < 128)	// H [85..127] < 180dg
		{
			*(p + offset_R) = mn;
			*(p + offset_G) = mx;
			*(p + offset_B) = (uint16_t)((mx - mn) * (H - 85)) / 43 + mn;
		}
		else if (H < 171)	// H [128..170] < 240dg
		{
			*(p + offset_R) = mn;
			*(p + offset_G) = mx - (uint16_t)((mx - mn) * (H - 128)) / 43;
			*(p + offset_B) = mx;
		}
		else if (H < 213)	// H [171..212] < 300dg
		{
			*(p + offset_R) = (uint16_t)((mx - mn) * (H - 171)) / 42 + mn;
			*(p + offset_G) = mn;
			*(p + offset_B) = mx;
		}
		else				// H [213..255] < 360dg
		{
			*(p + offset_R) = mx;
			*(p + offset_G) = mn;
			*(p + offset_B) = mx - ((uint16_t)(mx - mn) * (H - 213)) / 43;
		}

		Serial.print(*(p + offset_R), HEX); Serial.print(" "); Serial.print(*(p + offset_G), HEX); Serial.print(" "); Serial.print(*(p + offset_B), HEX); Serial.print("\n");
	}
}
