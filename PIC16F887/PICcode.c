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
