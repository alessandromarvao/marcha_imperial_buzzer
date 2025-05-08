#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/pwm.h"
#include "pico/multicore.h"

#define BUZZER_A_PIN 21
#define BUZZER_B_PIN 8

// Frequências da escala cromática de C4 a C5 (valores arredondados)
const float notes[] = {
    261.63f, // C4
    277.18f, // C#4
    293.66f, // D4
    311.13f, // D#4
    329.63f, // E4
    349.23f, // F4
    369.99f, // F#4
    392.00f, // G4
    415.30f, // G#4
    440.00f, // A4
    466.16f, // A#4
    493.88f, // B4
    523.25f,  // C5
    554.37f,  // C#5
    587.33f,  // D5
    622.25f,  // D#5
    659.26f,  // E5
    698.46f,  // F5
    739.99f,  // F#5
    784.00f,  // G5
    830.60f,  // G#5
    880.00f,  // A5
    932.32f,  // A#5
    987.76f,  // B5
    1046.5f, // C6
    1108.73f,  // C#6 (554.37 * 2)
    1174.66f,  // D6 (587.33 * 2)
    1244.50f,  // D#6 (622.25 * 2)
    1318.52f,  // E6 (659.26 * 2)
    1396.92f,  // F6 (698.46 * 2)
    1479.98f,  // F#6 (739.99 * 2)
    1567.98f,  // G6 (783.99 * 2)
    1661.22f,  // G#6 (830.61 * 2)
    1760.00f,  // A6 (880.00 * 2)
    1864.66f,  // A#6 (932.33 * 2)
    1975.54f,  // B6 (987.77 * 2)
    2093.00f   // C7 (1046.50 * 2)
};

const float chorus[] = {
    392.00f, 392.00f, 392.00f, 369.99f, 392.00f, 369.99f, 392.00f, 
    392.00f, 392.00f, 392.00f, 369.99f, 311.13f, 392.00f, 392.00f,
    392.00f, 392.00f, 392.00f, 392.00f, 277.18f, 277.18f, 277.18f,
    277.18f, 311.13f, 311.13f, 261.63f, 311.13f, 392.00f, 311.13f, 392.00f,
};

const int chorus_duration[] = {
    500, 500, 500, 500, 500, 500, 1000, 
    500, 500, 500, 500, 500, 500, 1000, 
    500, 500, 500, 500, 500, 500, 500, 
    500, 500, 500, 500, 500, 500, 500, 1000
};

const float solo[] = {
    784.00f, 784.00f, 784.00f, 784.00f, 622.25f, 932.32f, 784.00f, 622.25f, 932.32f, 784.00f,
    1174.66f, 1174.66f, 1174.66f, 1174.66f, 1244.50f, 932.32f, 784.00f, 622.25f, 932.32f, 784.00f,
};

const int solo_duration[] = {
    500, 500, 300, 550, 500, 300, 550, 500, 400, 1300,
    500, 500, 300, 550, 500, 300, 550, 500, 400, 1300,
};


void set_pwm_freq(uint gpio_pin, float freq) {
    gpio_set_function(gpio_pin, GPIO_FUNC_PWM);
    uint slice_num = pwm_gpio_to_slice_num(gpio_pin);
    uint channel = pwm_gpio_to_channel(gpio_pin);

    // Divisor de clock fixo para manter wrap dentro de 16 bits
    const float clock_divider = 8.0f;
    pwm_set_clkdiv(slice_num, clock_divider);

    // Cálculo do valor de wrap
    uint32_t wrap = (uint32_t)(125000000.0f / (clock_divider * freq)) - 1;
    pwm_set_wrap(slice_num, wrap);
    
    // Duty cycle de 50% para melhor qualidade de som
    // Para ajustar o volume: mude o duty cycle (ex: wrap / 4 para 25%)
    pwm_set_chan_level(slice_num, channel, wrap / 2);
    
    pwm_set_enabled(slice_num, true);
}

void play_note(int buzzer, float note, int note_duration, int interval) {
    set_pwm_freq(buzzer, note);
    // Nota toca por 500ms
    sleep_ms(note_duration);
    // Pequena pausa entre notas
    pwm_set_enabled(pwm_gpio_to_slice_num(buzzer), false);
    sleep_ms(50);
}

void core1_entry() {
    printf("Tocando no core 1.\n");
    while(true) {
        for(int i = 0; i < (sizeof(solo)/sizeof(solo[0])); i++) {
            play_note(BUZZER_A_PIN, solo[i], solo_duration[i], 50);
        }
    }
}

int main() {
    stdio_init_all();

    sleep_ms(5000);
    multicore_launch_core1(core1_entry);

    printf("Tocando no core 0\n");
    // Toca cada nota da escala cromática
    for(int i = 0; i < (sizeof(chorus)/sizeof(chorus[0])); i++) {
        play_note(BUZZER_B_PIN, chorus[i], chorus_duration[i], 50);
    }
    
    // Longa pausa após a escala completa
    sleep_ms(2000);

    while(true) {
        tight_loop_contents();
    }
}