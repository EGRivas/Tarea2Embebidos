// Emmanuel Rivas | Dorian Borja | Genesis Vargas
// P2 Sistemas Embebidos
// Tarea 2 - Comunicación microcontroladores
// ===== ATmega328P - Matriz 8x8 por PORTB (columnas) y PORTD (filas) =====
// Botones (pull-up): PC0 LEFT, PC1 RIGHT, PC2 ATTACK
// PIC (señales): PC3 muerte, PC5 victoria, PC4 comunicación (INT del PIC)

#define F_CPU 8000000UL
#include <avr/io.h>
#include <util/delay.h>
#include <stdint.h>
#include <string.h>

// ---------------- I/O ----------------
const uint8_t rowSel[8] = {1,2,4,8,16,32,64,128};
uint8_t fb[8]; // framebuffer: fb[y] = bitmask columnas (1=LED ON)

void io_init(void){
    DDRB = 0xFF; PORTB = 0x00;     // columnas
    DDRD = 0xFF; PORTD = 0x00;     // filas
    DDRC &= ~(_BV(PC0)|_BV(PC1)|_BV(PC2));   // botones
    PORTC |=  (_BV(PC0)|_BV(PC1)|_BV(PC2));  // pull-ups
    DDRC |= _BV(PC3)|_BV(PC4)|_BV(PC5);      // líneas al PIC
    PORTC &= ~(_BV(PC3)|_BV(PC4)|_BV(PC5));
}

uint8_t btn_left(void){  return !(PINC & _BV(PC0)); }
uint8_t btn_right(void){ return !(PINC & _BV(PC1)); }
uint8_t btn_fire(void){  return !(PINC & _BV(PC2)); }

// ---------------- Señal al PIC ----------------
void pic_pulse_status(void){ PORTC |= _BV(PC4); _delay_us(12); PORTC &= ~_BV(PC4); }
void pic_death(void){   PORTC |= _BV(PC3); PORTC &= ~_BV(PC5); _delay_us(8); pic_pulse_status(); PORTC &= ~_BV(PC3); }
void pic_victory(void){ PORTC |= _BV(PC5); PORTC &= ~_BV(PC3); _delay_us(8); pic_pulse_status(); PORTC &= ~_BV(PC5); }

// ---------------- Framebuffer y refresco ----------------
void fb_clear(void){ memset(fb, 0, sizeof(fb)); }
void px_set(uint8_t x, uint8_t y){ if(x<8 && y<8) fb[y] |=  (1u<<x); }
void px_clr(uint8_t x, uint8_t y){ if(x<8 && y<8) fb[y] &= ~(1u<<x); }

void refresh_once(void){
    for (uint8_t y=0; y<8; y++){
        PORTB = 0xFF;  PORTD = 0x00; _delay_us(2);  // blank/dead-time
        PORTD = (1u<<y); _delay_us(2);
        PORTB = (uint8_t)~fb[y];
        _delay_us(60);
    }
}
void show_ms(uint16_t ms){ while(ms--) refresh_once(); }

// ---------------- Bitmaps ----------------
const uint8_t DIG1[8] = {0x00,0x08,0x18,0x08,0x08,0x08,0x3E,0x00};
const uint8_t DIG2[8] = {0x00,0x3C,0x42,0x02,0x1C,0x20,0x7E,0x00};
const uint8_t DIG3[8] = {0x00,0x7E,0x04,0x18,0x04,0x42,0x3C,0x00};
const uint8_t IMG_X[8]    = {0x81,0x42,0x24,0x18,0x18,0x24,0x42,0x81};
const uint8_t IMG_SMILE[8]= {0x00,0x24,0x00,0x00,0x00,0x42,0x3C,0x00};

void blit(const uint8_t img[8]){ memcpy(fb, img, 8); }

// ---------------- RNG ----------------
// logica de aleatoriedad
uint8_t lfsr=0xA7;
uint8_t rnd8(void){ 
  lfsr^=lfsr<<3; 
  lfsr^=lfsr>>5; 
  lfsr^=lfsr<<1; 
  return lfsr; 
}

// ---------------- Juego ----------------
typedef struct { int8_t x, y; uint8_t act; } Bullet;
typedef struct { int8_t x, y; uint8_t act; } Enemy;

#define MAX_ENEMIES 10

Bullet pshot;                 // bala del jugador
Enemy  enem[MAX_ENEMIES];     // enemigos (2 píxeles horizontales)

void game_clear_entities(void){
    pshot.act = 0;
    for(uint8_t i=0;i<MAX_ENEMIES;i++) enem[i].act=0;
}

// carga de imagen en la matriz
void draw_scene(uint8_t px){
    fb_clear();
    px_set(px, 7);                  // jugador
    if(pshot.act) px_set(pshot.x, pshot.y); // bala jugador
    for(uint8_t i=0;i<MAX_ENEMIES;i++)       // enemigos
        if(enem[i].act){
            px_set(enem[i].x, enem[i].y);
            if(enem[i].x+1<8) px_set(enem[i].x+1, enem[i].y);
        }
}

// spawn aleatorio de enemigos
uint8_t spawn_enemy(void){
    for(uint8_t i=0;i<MAX_ENEMIES;i++){
        if(!enem[i].act){
            uint8_t x = rnd8() % 7; // 0..6 (2 px de ancho)
            enem[i].x = x; enem[i].y = 0; enem[i].act=1;
            return 1;
        }
    }
    return 0;
}
