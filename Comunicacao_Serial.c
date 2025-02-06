#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "pico/stdlib.h"
#include "hardware/pio.h"
#include "hardware/irq.h"
#include "hardware/clocks.h"
#include "ws2812.pio.h"      // Biblioteca para controle dos LEDs WS2812
#include "hardware/i2c.h"
#include "inc/ssd1306.h"     // Biblioteca para o display SSD1306
#include "inc/font.h"        // Biblioteca de fontes (modificada para incluir minúsculas)

#define WS2812_PIN      7      // Matriz de WS2812 na GPIO 7
#define NUM_PIXELS      25     // 5x5 LEDs
#define LED_RED_PIN    13
#define LED_BLUE_PIN   12
#define LED_GREEN_PIN  11
#define BTN_A_PIN       5
#define BTN_B_PIN       6

// Pinos do I2C para o display SSD1306
#define I2C_PORT        i2c1
#define I2C_SDA         14
#define I2C_SCL         15
#define SSD1306_ADDR    0x3C

// Tempo de _debounce_ (em microsegundos)
#define DEBOUNCE_DELAY_US 300000

// Flags para indicar que um botão foi pressionado (serão tratadas no loop principal)
volatile bool btn_a_flag = false;
volatile bool btn_b_flag = false;
volatile uint64_t last_interrupt_time_a = 0;
volatile uint64_t last_interrupt_time_b = 0;

// Estados dos LEDs (botão A altera LED verde; botão B altera LED azul)
volatile bool led_green_state = false;
volatile bool led_blue_state  = false;

// Objeto do display SSD1306
ssd1306_t ssd;

bool numbers[10][NUM_PIXELS] = {
    { // 0
      0, 1, 1, 1, 0,
      0, 1, 0, 1, 0,
      0, 1, 0, 1, 0,
      0, 1, 0, 1, 0,
      0, 1, 1, 1, 0
    },
    { // 1
      0, 1, 1, 1, 0,
      0, 0, 1, 0, 0,
      0, 0, 1, 0, 0,
      0, 1, 1, 0, 0,
      0, 0, 1, 0, 0
    },
    { // 2
      0, 1, 1, 1, 0,
      0, 1, 0, 0, 0,
      0, 1, 1, 1, 0,
      0, 0, 0, 1, 0,
      0, 1, 1, 1, 0
    },
    { // 3
      0, 1, 1, 1, 0,
      0, 0, 0, 1, 0,
      0, 1, 1, 1, 0,
      0, 0, 0, 1, 0,
      0, 1, 1, 1, 0
    },
    { // 4
      0, 1, 0, 0, 0,
      0, 0, 0, 1, 0,
      0, 1, 1, 1, 0,
      0, 1, 0, 1, 0,
      0, 1, 0, 1, 0
    },
    { // 5
      0, 1, 1, 1, 0,
      0, 0, 0, 1, 0,
      0, 1, 1, 1, 0,
      0, 1, 0, 0, 0,
      0, 1, 1, 1, 0
    },
    { // 6
      0, 1, 1, 1, 0,
      0, 1, 0, 1, 0,
      0, 1, 1, 1, 0,
      0, 1, 0, 0, 0,
      0, 1, 1, 1, 0
    },
    { // 7
      0, 1, 0, 0, 0,
      0, 0, 0, 1, 0,
      0, 1, 0, 0, 0,
      0, 0, 0, 1, 0,
      0, 1, 1, 1, 0
    },
    { // 8
      0, 1, 1, 1, 0,
      0, 1, 0, 1, 0,
      0, 1, 1, 1, 0,
      0, 1, 0, 1, 0,
      0, 1, 1, 1, 0
    },
    { // 9
      0, 1, 1, 1, 0,
      0, 0, 0, 1, 0,
      0, 1, 1, 1, 0,
      0, 1, 0, 1, 0,
      0, 1, 1, 1, 0
    }
};

// Função para enviar um pixel para o WS2812 (utilizando PIO)
static inline void put_pixel(uint32_t pixel_grb) {
    pio_sm_put_blocking(pio0, 0, pixel_grb << 8u);
}

// Função para montar o valor da cor no formato GRB
static inline uint32_t urgb_u32(uint8_t r, uint8_t g, uint8_t b) {
    return ((uint32_t)(r) << 8) | ((uint32_t)(g) << 16) | (uint32_t)(b);
}

// Função para desenhar um número (de 0 a 9) na matriz WS2812
void display_number(bool *buffer, uint8_t r, uint8_t g, uint8_t b) {
    uint32_t color = urgb_u32(r, g, b);
    for (int i = 0; i < NUM_PIXELS; i++) {
        put_pixel(buffer[i] ? color : 0);
    }
}

// Função para atualizar o conteúdo do display SSD1306
void update_display_message(const char* msg) {
    ssd1306_fill(&ssd, false);                   // Limpa o buffer
    ssd1306_draw_string(&ssd, msg, 0, 0);        // Desenha a string (posição x=0, y=0)
    ssd1306_send_data(&ssd);                     // Envia o buffer para o display
}

//  ROTINA DE INTERRUPÇÃO PARA OS BOTÕES

void gpio_callback(uint gpio, uint32_t events) {

    uint64_t current_time = time_us_64();

    if(gpio == BTN_A_PIN) {
        if(current_time - last_interrupt_time_a > DEBOUNCE_DELAY_US) {
            // Toggle no LED verde (GPIO LED_GREEN_PIN -> 11)
            led_green_state = !led_green_state;
            gpio_put(LED_GREEN_PIN, led_green_state);
            last_interrupt_time_a = current_time;
            btn_a_flag = true;  // Sinaliza para o loop principal que o botão A foi pressionado
        }
    }
    else if(gpio == BTN_B_PIN) {
        if(current_time - last_interrupt_time_b > DEBOUNCE_DELAY_US) {
            // Toggle no LED azul (GPIO LED_BLUE_PIN -> 12)
            led_blue_state = !led_blue_state;
            gpio_put(LED_BLUE_PIN, led_blue_state);
            last_interrupt_time_b = current_time;
            btn_b_flag = true;  // Sinaliza para o loop principal que o botão B foi pressionado
        }
    }
}

void setup() {
  // Configuração dos LEDs RGB
    gpio_init(LED_RED_PIN);
    gpio_set_dir(LED_RED_PIN, GPIO_OUT);
    gpio_put(LED_RED_PIN, false);
    gpio_init(LED_GREEN_PIN);
    gpio_set_dir(LED_GREEN_PIN, GPIO_OUT);
    gpio_put(LED_GREEN_PIN, false);
    gpio_init(LED_BLUE_PIN);
    gpio_set_dir(LED_BLUE_PIN, GPIO_OUT);
    gpio_put(LED_BLUE_PIN, false);

    // Configuração dos Botões (A e B)
    gpio_init(BTN_A_PIN);
    gpio_set_dir(BTN_A_PIN, GPIO_IN);
    gpio_pull_up(BTN_A_PIN);
    gpio_init(BTN_B_PIN);
    gpio_set_dir(BTN_B_PIN, GPIO_IN);
    gpio_pull_up(BTN_B_PIN);
    
    // Registra a função de callback para os dois botões
    gpio_set_irq_enabled_with_callback(BTN_A_PIN, GPIO_IRQ_EDGE_FALL, true, &gpio_callback);
    gpio_set_irq_enabled(BTN_B_PIN, GPIO_IRQ_EDGE_FALL, true);

    // Configuração do Display SSD1306 via I2C
    i2c_init(I2C_PORT, 400 * 1000);  // Inicializa I2C a 400 kHz
    gpio_set_function(I2C_SDA, GPIO_FUNC_I2C);
    gpio_set_function(I2C_SCL, GPIO_FUNC_I2C);
    gpio_pull_up(I2C_SDA);
    gpio_pull_up(I2C_SCL);

    ssd1306_init(&ssd, 128, 64, false, SSD1306_ADDR, I2C_PORT);
    ssd1306_config(&ssd);
    ssd1306_fill(&ssd, false);
    ssd1306_send_data(&ssd);
    update_display_message("Sistema Iniciado");
}

int main() {
    stdio_init_all();
    setup();

    // Configuração dos LEDs WS2812 via PIO
    PIO pio = pio0;
    int sm = 0;
    uint offset = pio_add_program(pio, &ws2812_program);
    ws2812_program_init(pio, sm, offset, WS2812_PIN, 800000, false);

    char serial_char = 0;    // Variável para armazenar o caractere recebido via UART
    char msg[50];            // Buffer para mensagens que serão enviadas para o display e para a UART

        while (true) {
        // ----- Tratamento da entrada serial (cada caractere individual) -----
        int ch = getchar_timeout_us(0);
        if (ch != PICO_ERROR_TIMEOUT) {
            serial_char = (char) ch;
            sprintf(msg, "Recebido: %c", serial_char);
            update_display_message(msg);
            printf("Caractere recebido: %c\n", serial_char);

            // Se o caractere for um dígito, exibe o símbolo correspondente na matriz WS2812
            if (serial_char >= '0' && serial_char <= '9') {
                int digit = serial_char - '0';
                // Exibe o símbolo com intensidade: neste exemplo, a cor azul (0, 0, 10)
                display_number(numbers[digit], 20, 0, 20);
            } else {
                // Se não for dígito, limpa a matriz de LEDs (desliga todos os LEDs)
                for (int i = 0; i < NUM_PIXELS; i++) {
                    put_pixel(0);
                }
            }
        }

        // ----- Tratamento das interrupções dos botões -----
        if (btn_a_flag) {
            btn_a_flag = false;
            sprintf(msg, "LED Verde %s", led_green_state ? "ligado" : "desligado");
            update_display_message(msg);
            printf("Botao A pressionado: LED Verde %s\n", led_green_state ? "ligado" : "desligado");
        }
        if (btn_b_flag) {
            btn_b_flag = false;
            sprintf(msg, "LED Azul %s", led_blue_state ? "ligado" : "desligado");
            update_display_message(msg);
            printf("Botao B pressionado: LED Azul %s\n", led_blue_state ? "ligado" : "desligado");
        }
        sleep_ms(50);
    }

    return 0;
}
