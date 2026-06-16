#include <stdio.h>
#include "mpu6050.h"
#include "servo.h"
#include "driver/gpio.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

// ===================== PINAGEM =====================
#define SDA_MPU     22
#define SCL_MPU     21
#define SERVO_PITCH 18     // Servo que controla Pitch (frente/trás)
#define SERVO_ROLL  19     // Servo que controla Roll (lateral)

// ===================== LIMITES =====================
#define PITCH_MIN   -45.0f
#define PITCH_MAX    45.0f
#define ROLL_MIN    -45.0f
#define ROLL_MAX     45.0f

// Função auxiliar para mapear valores
int map_value(float value, float in_min, float in_max, int out_min, int out_max) {
    float proportion = (value - in_min) / (in_max - in_min);
    int result = out_min + (int)(proportion * (out_max - out_min));
    
    if (result < out_min) result = out_min;
    if (result > out_max) result = out_max;
    return result;
}

void app_main(void)
{
    printf("=== Iniciando Sistema MPU6050 + Servos ===\n");

    // ==================== INICIALIZAÇÃO ====================
    mpu_main();                    // Inicializa o MPU6050
    
    // Inicializa os dois servos
    configuracao(SERVO_PITCH);
    configuracao(SERVO_ROLL);
    
    vTaskDelay(pdMS_TO_TICKS(1500));   // Aguarda estabilização
    
    printf("Sistema pronto! MPU6050 + 2 Servos\n\n");

    // ==================== LOOP PRINCIPAL ====================
    while(1)
    {
        mpu_snapshot_t snapshot;
        
        if(mpu_get_snapshot(&snapshot))
        {
            // ==================== CONTROLE DOS SERVOS ====================
            
            // Mapeia Pitch e Roll para ângulo dos servos (0° a 180°)
            int angulo_pitch = map_value(snapshot.pitch, PITCH_MIN, PITCH_MAX, 0, 180);
            int angulo_roll  = map_value(snapshot.roll,  ROLL_MIN,  ROLL_MAX,  0, 180);
            
            // Move os servos
            servo_mover(angulo_pitch);     // Atenção: sua biblioteca atual só controla 1 canal!
            
            // Debug
            printf("Pitch: %6.2f° → Servo1: %3d° | Roll: %6.2f° → Servo2: %3d°\n",
                   snapshot.pitch, angulo_pitch, snapshot.roll, angulo_roll);
        }
        else
        {
            printf("Erro: Timeout ao ler MPU6050\n");
        }
        
        vTaskDelay(pdMS_TO_TICKS(80));   // ~12.5 atualizações por segundo
    }
}