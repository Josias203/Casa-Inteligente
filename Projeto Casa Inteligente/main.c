#include <atmel_start.h>
#ifndef F_CPU
#define F_CPU 16000000UL // Frequência de trabalho da CPU
#endif

#define BAUD 9600
#define MYUBRR F_CPU/16/BAUD-1
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <avr/eeprom.h>

#include "nokia5110.h"

#define tam_vetor 4

// Variaveis globais
unsigned char temp_string[tam_vetor];
uint16_t temp_ADC = 0;
uint16_t temp_maior = 0;

char *lampSala = "OFF";
char *lampCozinha = "OFF";
char lampQuartos[2] = {'0','0'};
char *garagem = "Fechada";

// Conversão de inteiro para string
void int_to_str(uint16_t s, unsigned char *d)
{
	uint8_t n = tam_vetor - 2;

	for (int8_t i = n; i >= 0; i--)
	{
		d[i]=s%10+48;
		s/=10;
	}
}

// Mostra todos os valores dos componentes da Casa
void atualiza_lcd()
{
	nokia_lcd_clear(); // Limpa o LCD
	nokia_lcd_set_cursor(0, 0); // Muda o cursos para a posição 0, 0
	nokia_lcd_write_string("Casa  ",1);
	int_to_str(temp_ADC, temp_string); //converte a temperatura em string
	nokia_lcd_write_string((char*)temp_string, 1); //Escreve o valor no buffer do LCD
	int_to_str(temp_maior, temp_string);
	nokia_lcd_write_char('/', 1);
	nokia_lcd_write_string((char*)temp_string, 1);
	nokia_lcd_write_char('C', 1);
	nokia_lcd_set_cursor(0, 10);
	nokia_lcd_write_string("Sala:      ",1);
	nokia_lcd_write_string(lampSala,1);
	nokia_lcd_set_cursor(0, 20);
	nokia_lcd_write_string("Cozinha:   ",1);
	nokia_lcd_write_string(lampCozinha,1);
	nokia_lcd_set_cursor(0, 30);
	nokia_lcd_write_string("Quartos: ",1);
	nokia_lcd_write_char(lampQuartos[0],1);
	nokia_lcd_write_string(" | ",1);
	nokia_lcd_write_char(lampQuartos[1],1);
	nokia_lcd_set_cursor(0, 40);
	nokia_lcd_write_string("Gara.: ",1);
	nokia_lcd_write_string(garagem,1);
	nokia_lcd_render(); // Atualiza a tela do display com o conteúdo do buffer
}

// ||Função para inicialização da USART||
void USART_Init(unsigned int ubrr)
{
	UBRR0H = (unsigned char)(ubrr>>8); //Ajusta a taxa de transmissão
	UBRR0L = (unsigned char)ubrr;
	UCSR0B = (1<<RXCIE0)|(1<<RXEN0)|(1<<TXEN0); //Habilita o transmissor e o receptor
	UCSR0C = (1<<USBS0)|(3<<UCSZ00); //Ajusta o formato do frame: 8 bits de dados e 2 de parada
}

ISR(USART_RX_vect)
{
	char recebido;
	recebido = UDR0; // UDR0 contém o dado recebido via USART
	switch (recebido)
	{
		case 'q':
		PORTC |= (1<<PC1);
		lampSala = "ON";
		nokia_lcd_set_cursor(0, 10);
		nokia_lcd_write_string("              ",1);
		nokia_lcd_set_cursor(0, 10);
		nokia_lcd_write_string("Sala:      ",1);
		nokia_lcd_write_string(lampSala,1);
		nokia_lcd_render();
		break;

		case 'w':
		PORTC &= ~(1<<PC1);
		lampSala = "OFF";
		nokia_lcd_set_cursor(0, 10);
		nokia_lcd_write_string("              ",1);
		nokia_lcd_set_cursor(0, 10);
		nokia_lcd_write_string("Sala:      ",1);
		nokia_lcd_write_string(lampSala,1);
		nokia_lcd_render();
		break;

		case 'e':
		PORTC |= (1<<PC2);
		lampCozinha = "ON";
		nokia_lcd_set_cursor(0, 20);
		nokia_lcd_write_string("              ",1);
		nokia_lcd_set_cursor(0, 20);
		nokia_lcd_write_string("Cozinha:   ",1);
		nokia_lcd_write_string(lampCozinha,1);
		nokia_lcd_render();
		break;

		case 'r':
		PORTC &= ~(1<<PC2);
		lampCozinha = "OFF";
		nokia_lcd_set_cursor(0, 20);
		nokia_lcd_write_string("              ",1);
		nokia_lcd_set_cursor(0, 20);
		nokia_lcd_write_string("Cozinha:   ",1);
		nokia_lcd_write_string(lampCozinha,1);
		nokia_lcd_render();
		break;

		case '0': // '0' em int é igual a 48, OCR0A = (48-48)*63.7 = 0
		case '1': // '1' em int é igual a 49, OCR0A = (49-48)*63.7 = 63
		case '2': // '2' em int é igual a 50, OCR0A = (50-48)*63.7 = 127
		case '3': // '3' em int é igual a 51, OCR0A = (51-48)*63.7 = 191
		case '4': // '4' em int é igual a 52, OCR0A = (49-48)*63.7 = 254
		lampQuartos[0] = recebido;
		OCR0A = (recebido-48)*63.7;
		nokia_lcd_set_cursor(0, 30);
		nokia_lcd_write_string("              ",1);
		nokia_lcd_set_cursor(0, 30);
		nokia_lcd_write_string("Quartos: ",1);
		nokia_lcd_write_char(lampQuartos[0],1);
		nokia_lcd_write_string(" | ",1);
		nokia_lcd_write_char(lampQuartos[1],1);
		nokia_lcd_render();
		break;

		case '5': // '5' em int é igual a 53, OCR0B = (53-53)*63.7 = 0
		case '6': // '6' em int é igual a 54, OCR0B = (54-53)*63.7 = 63
		case '7': // '7' em int é igual a 55, OCR0B = (55-53)*63.7 = 127
		case '8': // '8' em int é igual a 56, OCR0B = (56-53)*63.7 = 191
		case '9': // '0' em int é igual a 56, OCR0B = (57-53)*63.7 = 254
		lampQuartos[1] = recebido-5;
		OCR0B = (recebido-53)*63.7;
		nokia_lcd_set_cursor(0, 30);
		nokia_lcd_write_string("              ",1);
		nokia_lcd_set_cursor(0, 30);
		nokia_lcd_write_string("Quartos: ",1);
		nokia_lcd_write_char(lampQuartos[0],1);
		nokia_lcd_write_string(" | ",1);
		nokia_lcd_write_char(lampQuartos[1],1);
		nokia_lcd_render();
		break;

		case 'a':
		garagem = "Abrindo";
		nokia_lcd_set_cursor(0, 40);
		nokia_lcd_write_string("              ",1);
		nokia_lcd_set_cursor(0, 40);
		nokia_lcd_write_string("Gara.: ",1);
		nokia_lcd_write_string(garagem,1);
		nokia_lcd_render();
		// Gira o motor no sentido horario, abrindo o portão
		for (uint8_t j = 0, i = 3; j < 64; j++)
		{
			PORTC = (1<<i)|(PORTC&0b00000110);
			i++;
			_delay_ms(50);
			if(i == 7) i = 3;
		}

		garagem = "Aberta";
		nokia_lcd_set_cursor(0, 40);
		nokia_lcd_write_string("              ",1);
		nokia_lcd_set_cursor(0, 40);
		nokia_lcd_write_string("Gara.: ",1);
		nokia_lcd_write_string(garagem,1);
		nokia_lcd_render();
		break;

		case 's':
		garagem = "Fechan.";
		nokia_lcd_set_cursor(0, 40);
		nokia_lcd_write_string("              ",1);
		nokia_lcd_set_cursor(0, 40);
		nokia_lcd_write_string("Gara.: ",1);
		nokia_lcd_write_string(garagem,1);
		nokia_lcd_render();
		// Gira o motor no sentido anti-horario, fechando o portão
		for (uint8_t j = 0, i = 6; j < 64; j++)
		{
			PORTC = (1<<i)|(PORTC&0b00000110);
			i--;
			_delay_ms(50);
			if(i == 2) i = 6;
		}

		garagem = "Fechada";
		nokia_lcd_set_cursor(0, 40);
		nokia_lcd_write_string("              ",1);
		nokia_lcd_set_cursor(0, 40);
		nokia_lcd_write_string("Gara.: ",1);
		nokia_lcd_write_string(garagem,1);
		nokia_lcd_render();
		break;
	}
}
// Função que gera um onda quadrada de T = 1 ms (f = 1 khz) no pino PB0 por 3000ms = 3s
void alarme_sonoro()
{
	for(int16_t i = 0; i < 3000; i++)
	{
		PORTB |= (1 << PB0);
		_delay_us(500);
		PORTB &= ~(1 << PB0);
		_delay_us(500);
	}
}

ISR(INT0_vect) // Alarme sensor de presença Terraço
{
	nokia_lcd_clear();
	nokia_lcd_set_cursor(10, 10);
	nokia_lcd_set_cursor(0, 10);
	nokia_lcd_write_string("ALARME!", 2);
	nokia_lcd_set_cursor(20, 30);
	nokia_lcd_write_string("Terraco", 1);
	nokia_lcd_render();

	alarme_sonoro();

	atualiza_lcd();
}

ISR(INT1_vect) // Alarme sensor de presença Casa
{
	nokia_lcd_clear();
	nokia_lcd_set_cursor(0, 10);
	nokia_lcd_write_string("ALARME!", 2);
	nokia_lcd_set_cursor(30, 30);
	nokia_lcd_write_string("Casa", 1);
	nokia_lcd_render();

	alarme_sonoro(); // Chama função que gera a onda quadrada

	atualiza_lcd(); // Chama função que atualiza os valores do LCD
}

ISR(ADC_vect)
{
	temp_ADC = ((ADC/1023.0)*500); // 10mV/°C

	if (temp_ADC > temp_maior)
	{
		temp_maior = temp_ADC;
		eeprom_update_byte((uint8_t *)0, temp_maior);  // Salva a maior temperatura lida no endereço 0 da EEPROM
	}
}

int main()
{
	// GPIO
	DDRC = 0b1111110; // Pinos da porta C de PC1-PC6 como saída
	DDRD = 0b01110000; // Pinos da porta D de PD4-PD6 como saída
	DDRB = 0b00111111; // Pinos da porta B de PB0-PB5 como saída
	PORTD = 0b00001100; // habilita pull-ups nos pinos PD2 e PD3

	USART_Init(MYUBRR);

	// Fast PWM, TOP = 0xFF, OC0A e OC0B habilitados
	TCCR0A = 0b10100011; // PWM não invertido nos pinos OC0A e OC0B
	TCCR0B = 0b00000101; // liga TC0, prescaler = 1024, fpwm = f0sc/(256*prescaler) = 16MHz/(256*1024) = 61Hz

	EICRA = 0b00001010; // interrupção externa INT0 e INT1 na borda de descida
	EIMSK = 0b00000011; // habilita a interrupção externa INT0 e INT1

	ADMUX = 0b01000000; // AVCC (5V), canal 0
	ADCSRA = 0b11101111; // habilita o AD, habilita interrupção, modo de conversão contínua, prescaler = 128
	ADCSRB = 0x00; // modo de conversão contínua
	DIDR0 = 0b00111110; // habilita pino PC0 como entrada do ADC0

	nokia_lcd_init(); // Inicia o LCD

	sei(); // habilita interrupções globais, ativando o bit I do SREG

	atualiza_lcd(); // Chama função que atualiza os valores do LCD

	temp_maior = eeprom_read_byte ((const uint8_t*)0); // Ler o valor do endereço 0 da EEPROM
	
	while(1)
	{
		if(temp_ADC >= 70) // Se a temperutura da casa for maior que 70°C, liga o alarme sonoro e mostra um aviso
		{
			nokia_lcd_clear();
			nokia_lcd_set_cursor(0, 0);
			nokia_lcd_write_string("Perigo!", 2);
			nokia_lcd_set_cursor(0, 20);
			nokia_lcd_write_string("Temperatura   interna alta", 1);
			nokia_lcd_render();

			alarme_sonoro(); // Chama função que gera a onda quadrada

			atualiza_lcd(); // Chama função que atualiza os valores do LCD
		}

		else
		{
			nokia_lcd_set_cursor(0, 0);
			nokia_lcd_write_string("              ",1);
			nokia_lcd_set_cursor(0, 0);
			nokia_lcd_write_string("Casa  ",1);
			int_to_str(temp_ADC, temp_string); //converte a temperatura em string
			nokia_lcd_write_string((char*)temp_string, 1); //Escreve o valor no buffer do LCD
			int_to_str(temp_maior, temp_string); //converte a temperatura em string
			nokia_lcd_write_char('/', 1);
			nokia_lcd_write_string((char*)temp_string, 1); //Escreve o valor no buffer do LCD
			nokia_lcd_write_char('C', 1);
			nokia_lcd_render();
			_delay_ms(1000);
		}
	}
}