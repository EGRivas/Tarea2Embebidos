// Emmanuel Rivas | Dorian Borja | Genesis Vargas
// P2 Sistemas Embebidos
// Tarea 2 - Comunicación microcontroladores
// === PIC16F887 - AUDIO SLAVE (MikroC PRO for PIC) ===
// Pines:
//   RB0/INT  : Comunicacion desde ATmega (PC4)
//   RD0      : Bit "DEATH"  (1 = tocar derrota)
//   RD1      : Bit "VICTORY"(1 = tocar victoria)
//   RC2      : BUZZER (Sound_Out)

#define NOTE_C4  262u
#define NOTE_D4  294u
#define NOTE_E4  330u
#define NOTE_F4  349u
#define NOTE_G4  392u
#define NOTE_A4  440u
#define NOTE_B4  494u
#define NOTE_C5  523u
#define NOTE_D5  587u
#define NOTE_E5  659u
#define NOTE_F5  698u
#define NOTE_G5  784u
#define NOTE_A5  880u

// Comandos de comunicacion
#define CMD_NONE     0x00
#define CMD_DEATH    0x01
#define CMD_VICTORY  0x02

volatile unsigned char current_cmd = CMD_NONE;
volatile unsigned char pending_cmd = CMD_NONE;
volatile unsigned char new_cmd     = 0;
volatile unsigned char sound_busy  = 0;

// Para evitar declarar dentro del ISR
volatile unsigned char d_bus = 0;

// Reproducir nota con pequeño espaciado
void play(unsigned int freq, unsigned int ms) {
    Sound_Play(freq, ms);
    Delay_ms(15); // espaciado
}

// ---- MELODÍA VICTORIA: Lost Woods (Zelda: OOT) ----
void Melody_Victory_LostWoods() {
    unsigned int s = 170;  // duración estándar para cada nota
    play(NOTE_F4, s); play(NOTE_A4, s); play(NOTE_B4, s+150);

    play(NOTE_F4, s); play(NOTE_A4, s); play(NOTE_B4, s+150);

    play(NOTE_F4, s); play(NOTE_A4, s); play(NOTE_B4, s);
    play(NOTE_E5, s+70); play(NOTE_D5, s+160);

    play(NOTE_B4, s); play(NOTE_C5, s); play(NOTE_B4, s+60);
    play(NOTE_G4, s); play(NOTE_E4, s+560);

    play(NOTE_D4, s); play(NOTE_E4, s); play(NOTE_G4, s+100);
    play(NOTE_E4, s+160);
}

// ---- MELODÍA DERROTA: Secuencia La menor ----
void Melody_Defeat_Sad() {
    play(NOTE_A4, 220); play(NOTE_G4, 220); play(NOTE_F4, 240);
    play(NOTE_E4, 260); play(NOTE_D4, 280); play(NOTE_C4, 360);
    Delay_ms(80);
    play(NOTE_E4, 220); play(NOTE_C4, 360);
    play(NOTE_E4, 360); play(NOTE_C4, 220);
}

// ---- ISR INT0: flanco en RB0 para leer RD1..RD0 ----
void interrupt(void) {
    if (INTF_bit) {
        Delay_us(8);                // settle del bus
        d_bus = PORTD & 0x03;       // RD1..RD0

        // Decodifica: prioridad a la señal alta que llegue
        // RD1=1 -> Victoria, RD0=1 -> Derrota (si ambas 0 o 1, se ignora)
        if((d_bus & 0x02) && !(d_bus & 0x01)){
            if (!sound_busy) { current_cmd = CMD_VICTORY; new_cmd = 1; }
            else             { pending_cmd = CMD_VICTORY; }
        }
        else if((d_bus & 0x01) && !(d_bus & 0x02)){
            if (!sound_busy) { current_cmd = CMD_DEATH; new_cmd = 1; }
            else             { pending_cmd = CMD_DEATH; }
        }
        INTF_bit = 0;               // se limpia flag
    }
}

void main() {
    // Digital I/O
    ANSEL  = 0; ANSELH = 0;
    C1ON_bit = 0; C2ON_bit = 0;

    TRISB = 0xFF;           // RB0 como entrada (INT0)
    TRISD = 0xFF;           // RD1..RD0 entradas (bus 2 bits)
    TRISC = 0b11111011;     // RC2 salida (0=out)
    PORTB = 0; PORTC = 0; PORTD = 0;

    Sound_Init(&PORTC, 2);  // RC2 (C.2)

    // INT0 en flanco ascendente
    INTEDG_bit = 1;   // flanco
    INTF_bit  = 0;
    INTE_bit  = 1;    // enable INT0
    GIE_bit   = 1;    // enable global

    while(1) {
        if (new_cmd) {
            unsigned char cmd = current_cmd;
            new_cmd = 0;
            sound_busy = 1;

            switch (cmd) {
                case CMD_VICTORY: Melody_Victory_LostWoods(); break;
                case CMD_DEATH:   Melody_Defeat_Sad();        break;
                default: break;
            }

            sound_busy = 0;

            // se encadena si quedo algo pendiente
            if (pending_cmd != CMD_NONE) {
                current_cmd = pending_cmd;
                pending_cmd = CMD_NONE;
                new_cmd = 1;
            }
        }
    }
}




