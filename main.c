#include <16F877A.h>
#device ADC = 10
#include <stdio.h>
#use DELAY(clock = 4M)
#fuses XT, NOPROTECT, NOWDT
#use RS232(baud = 9600, parity = N, xmit = pin_c6, rcv = pin_c7, bits = 8)

#define trigger1 pin_b1
#define echo1 pin_b0
#define trigger2 pin_b4
#define echo2 pin_b3

#int_timer0
void timer_0()
{ /*Se realiza una escritura en el registro 0. */
    set_timer0(0);
}

void main()
{ /* Función principal. */
    int16 temperatureSensor;
    int8 pulseSensor1, pulseSensor2;                                    /** Variables para obtener valores de bits del PIC (temperatura, óptico y de nivel, respectivamente).*/
    float voltageTemperatureSensor, distance1, time1, distance2, time2; /** Variables para obtener valores de voltaje del PIC (temperatura, óptico y de nivel, respectivamente).*/
    float soundSpeed = 0.0343;
    char in; /* Variable que permitirá la selección del sensor a utilizar y visualizar datos. */

    while (true) /* Ciclo infinito. */
    {
        printf("\r\nSeleccione el sensor que desea utilizar: \r\n");
        printf("De temperatura | Óptico | De nivel\r\n");
        in = getchar();

        switch (in)
        {
        case 'a':
            setup_adc_ports(all_analog);                                   /* Activa todos los puertos de entrada del convertidor. */
            setup_adc(ADC_CLOCK_INTERNAL);                                 /* Indica que se utiliza el reloj interno del PIC. */
            set_adc_channel(0);                                            /* Permite el uso del canal 0.*/
            delay_us(1000);                                                /* Interrupción de 1 segundo. */
            for(;;) {
               temperatureSensor = read_adc();                                /* Realiza la lectura de datos en bits. */
               voltageTemperatureSensor = temperatureSensor * (5.0 / 1023.0); /* Se calcula el voltaje generado por el sensor a partir de la resolución del PIC. */
            // printf("\r\n%4Ld\r\n", temperatureSensor);
               printf("%2.2f\r\n", voltageTemperatureSensor);
               delay_ms(1000);
            }
        case 'b':
            setup_timer_0(RTCC_EXT_H_TO_L | RTCC_DIV_1); /* Configuración del contador con detección de señales alto-bajo con división de frecuencia igual a 1 para el sensor detector de pulsos. */
            set_timer0(0);
            enable_interrupts(int_timer0); /* Habilita interrupciones en el timer 0. */
            enable_interrupts(global);

            pulseSensor1 = get_timer0(); /* Se obtienen los datos en el timer 0. */
            pulseSensor2 = pulseSensor1;

            for (;;)
            {
                pulseSensor1 = get_timer0();
                 
                if (pulseSensor1 != pulseSensor2)
                {
                    printf("%3u\r\n", pulseSensor1*6);
                    pulseSensor2 = pulseSensor1;
                }
               delay_ms(10000);
            }
         case 'c':
            setup_timer_1(T1_internal | T1_div_by_1); /* Indica que se trabajará con el timer1
                                               //       con modo de configuración de interrupción interna y preescaler de 1. */

            output_high(trigger1); // Se envía la señal de pulso que inicializa el sensor.
            delay_us(10);          // Pulso conduración de 10us.
            output_low(trigger1);  // Se genera una rafaga de pulsos de 10us.

            while (!input(echo1))
                ;
            set_timer1(0);

            while (input(echo1))
                ;
            time1 = get_timer1();                   // Obtiene las interrupciones generadas.
            distance1 = (time1 / 2) * (soundSpeed); // Calcula la distancia recorrida por la salida,
            // desde el sensor hasta el líquido.

            printf("\r\nDistancia recorrida (tinaco): %f centimetros\r\n", distance1);
            printf("Tiempo recorrido (tinaco): %f segundos\r\n", time1);

            setup_timer_2(t2_div_by_1, 255, 1); /* Inicialización del timer 2 del PIC para el sensor detector de nivel en el tambo. */

            output_high(trigger2);
            delay_us(10);
            output_low(trigger2);

            while (!input(echo2))
                ;
            set_timer2(0);

            while (input(echo2))
                ;
            time2 = get_timer2() * 8;
            distance2 = (time2 / 2) * (soundSpeed) * 8;

            printf("\r\nDistancia recorrida (pozo): %f centimetros\r\n", distance2);
            printf("Tiempo recorrido (pozo): %f segundos\r\n", time2);

            delay_ms(500); // Genera una interrupción de 500ms.
            break;
        default:
            break;
        }
        delay_us(1000);
    }
}
