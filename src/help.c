#include "help.h"

char* HELP_STR =
    "~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n  "
    "- pbtn         - print user push button state\n  \n    #example:\n    $ "
    "pbtn\n      Pushbutton inactive\n  \n  - led s N S    - set led N to "
    "static state where\n\t\t   N - led number [0-3]\n\t\t   S - led state "
    "[0,1]\n    #example:\n    $ led s 0 1\n      LED: [0] set to static state "
    "[1]\n\n  - led b N O P  - set led N to blink state where\n\t\t   N - led "
    "number [0-3]\n\t\t   O - led ON interval [x1ms]\n\t\t   P - length of "
    "blinking interval [x1ms]\n    #example:\n    $ led b 1 100 1000\n      "
    "LED: [1] set to blink state, on time [100]ms & period [1000]ms\n      \n  "
    "- led p N D \t - set led N to PWM state where\n\t\t   N - led number "
    "[0-3]\n\t\t   D - led PWM duty cycle [0-100]\n    #example:\n    $ led p "
    "2 30\n      LED: [2] set to pwm state, duty cycle [30]\n      \n  - dac1 "
    "s f1 a1 f2 a2 - initialize DAC1 on pin PA4 with DMA, fs = 40 kHz, \n  "
    "\t\t\t  generate linear combination of two sine waves x(t) = a1 sin(2πf1 "
    "t) + a2 sin(2πf2 t) + 0.5\n  \t\t\t  f1 - frequency of first sine wave "
    "[10-10000Hz]\n  \t\t\t  a1 - amplitude of first sine wave [float]\n  "
    "\t\t\t  f2 - frequency of second sine wave [10-10000Hz]\n  \t\t\t  a2 - "
    "amplitude of second sine wave [float]\n  \t\t\t  where |a1| + |a2| <= "
    "0.49\n    #example:\n    $ dac1 s 10 0.2 11 0.15\n      DAC1 generates "
    "linear combination of sine waves, a1 = [2.000e-1], a2=[1.500e-1], "
    "f1=[10]Hz, f2=[11]Hz, on pin PA4\n      \n  - dac1 s    - stop "
    "(deinitialize) DAC1\n  \n    #example:\n    $ dac1 s\n      DAC1 "
    "deinitialized\n\n  - adc1 r    - initialize ADC1 on pin PA1 with DMA, fs "
    "= 40 kHz\n  \n    #example:\n    $ adc1 r\n      ADC initialized on pin "
    "PA1\n  \n  - adc1 s    - stop (deinitialize) ADC1\n  \n    #example:\n    "
    "$ adc1 s\n      ADC1 deinitialized\n\n  - dac1 t f a   - initialize DAC1 "
    "on pin PA4 with DMA, fs = 40 kHz, \n  \t\t\t       generate triangluar "
    "wave  \n  \t\t\t       f - frequency of the wave [10 - 10000Hz]\n  \t\t\t "
    "      a - amplitude of the wave [float]\n    #example:\n    $ dac1 t 10 "
    "0.5\n      DAC1 generates triangle wave of amplitude a = [0.5], and "
    "frequency f = [10]Hz, on pin PA4      \n      \n  - help     - print out "
    "help manual\n    #example:\n    $ "
    "help\n\n~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~"
    "~~~  \n";

void printHelp(void) { printUSART2(HELP_STR); }

