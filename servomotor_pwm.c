#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/pwm.h"

#define SERVO_PIN 22 // definição do pino gpio do servo

void wrap_handler() {
    static float current_pulse = 500.0f; // braço do servomotor inicia o movimento continuo na posição 0 graus
    static int direction = 10; // constante que representa a vwelocidade com que o braço do servomotor ira se mover quando em movimento continuo
    // ecolhido um pulso de 10microssegundos para movimento suave do braço do servo motor
    
    current_pulse += direction; // valor do pulso vai incrementado ou decrementando conforme o valor de direction no if..else 
    // o que significa que o braço ira fazer um movimento suave 

    pwm_clear_irq(pwm_gpio_to_slice_num(SERVO_PIN));//limpa flag de interrupção
        
    if (current_pulse >= 2400.0f) { // if que limita o braço em 180 graus 
        direction = -10; // começa a mover o braço na direção contraria 
        current_pulse = 2400.0f; // pulso fica limitado a 2400microssegundos
    } else if (current_pulse <= 500.0f) { // else if que limita o braço em 0 graus
        direction = 10; // move o braço na direção contraria
        current_pulse = 500.0f; // pulso fica limitado a 500 microssegundos
    }
        
    pwm_set_gpio_level(SERVO_PIN, current_pulse * 2.5); // multiplica-se por 2.5 para converter de microssegundos para contagem pwm
    // 2.5 é o numero de contagens por microssegundos (valor de wrap / 20ms = X contagens/microssegundo)
    // necessário para manter o braço na posição desejada
}

void servo_positions(){
    // define um duty cycle de 0,12%
    pwm_set_gpio_level(SERVO_PIN, 2400 * 2.5); // braço do servomotor vai para a posição 180 graus
    sleep_ms(5000); // agarda 5s na posição definida
    // define um duty cycle de 0,0735%
    pwm_set_gpio_level(SERVO_PIN, 1470 * 2.5); // braço do servomotor vai para posição de 90 graus
    sleep_ms(5000); // agarda 5s na posição definida
    // define um duty cycle de 0,025%
    pwm_set_gpio_level(SERVO_PIN, 500 * 2.5); // braço do servomotor vai para posição 0 graus
    sleep_ms(5000); // agarda 5s na posição definida
}

uint pwm_setup_irq(){
    // Configuração inicial do PWM
    gpio_set_function(SERVO_PIN, GPIO_FUNC_PWM); // gpio -> pwm
    uint slice = pwm_gpio_to_slice_num(SERVO_PIN); // obtem canal (slice) pwm da gpio

    pwm_config config = pwm_get_default_config(); // pega config inicial do pwm
    // wrap e divisor de clock sao definidos antes da inicialização do slice
    pwm_config_set_clkdiv(&config, 50.0f); // escolhido de um clock de 50Hz (pwm_set_clkdiv recebe um float como segundo parametro)
    pwm_config_set_wrap(&config, 49999);   // wrap calculado pela formula apresentada na aula sincrona | wrap + 1 = Fclock / (Di + Df/16) * Fpwm
    pwm_init(slice, &config, true); // inicia o slice

    pwm_set_irq_enabled(slice, true); // habilita interrupção pwm para o slice 
    irq_set_exclusive_handler(PWM_IRQ_WRAP, wrap_handler); //define qual o tipo de interrupção 
    // interrupção é ativada quando counter de um slice atinge o valor wrap 

    return slice;
}

int main() {
    uint slice = pwm_setup_irq(); // função que inicializa wrap com interrupção 

    servo_positions(); // rotina de posicionamento inicial do braço do servo 

    irq_set_enabled(PWM_IRQ_WRAP, true); // habilita a interrupção após a rotina de posicionamento inicial

    while (true) {
    
        sleep_ms(10);
    }

    return 0;
}