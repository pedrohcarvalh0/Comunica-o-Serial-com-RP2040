#  Comunicação Serial com RP2040 UART, SPI e I2C

Este projeto foi desenvolvido para o Raspberry Pi Pico W e implementa diversas funcionalidades, incluindo o controle de LEDs, exibição de caracteres em um display SSD1306, e interação com botões. O código foi escrito em C e utiliza interrupções, debouncing, e comunicação serial via UART.

---

##  Funcionalidades

1. **Monitor Serial UART** :
   - Cada caractere digitado no Serial Monitor do VS Code é exibido no display SSD1306.
   - Se o caractere for um número entre 0 e 9, um símbolo correspondente é exibido na matriz 5x5 de 
     LEDs WS2812.

2. **Botões e LEDs RGB** :
   - Botão A: Alterna o estado do LED RGB Verde (ligado/desligado).
   - Botão B: Alterna o estado do LED RGB Azul (ligado/desligado).
   - O estado dos LEDs é exibido no display SSD1306 e uma mensagem descritiva é enviada ao Serial Monitor.

3. **Interrupções** :
   - As funcionalidades dos botões são implementadas utilizando rotinas de interrupção (IRQ).
  
4. **Debouncing** :
   - O tratamento do bouncing dos botões é feito via software para garantir que cada pressionamento seja registrado corretamente.
  
5. **Controle de LEDs** :
   - O projeto demonstra o controle de LEDs comuns (RGB) e LEDs WS2812 (matriz 5x5).

6. **Display SSD1306** :
   - O display SSD1306 é utilizado para exibir mensagens e caracteres, demonstrando o uso de fontes maiúsculas e minúsculas e o protocolo I2C.
     
---

##  Requisitos

```plaintext
🔧 Hardware:
- Raspberry Pi Pico.
- Placa BitDogLab (Para Teste Real)

💻 Software:
- SDK do Raspberry Pi Pico configurado.
- Compilador C compatível (como GCC).
- Ferramentas para upload do código para o Pico (como `picotool`).
```

---

## 🚀 Compilação e Upload

```plaintext
1. Clone o repositório do código:
   git clone <URL_DO_REPOSITORIO>

2. Compile o programa usando CMake:
   mkdir build
   cd build
   cmake ..
   make

   Também é possível compilar diretamente na  extensão do Raspberry Pi Pico Project no VS Code clicando em "Compile Project"

3. Envie o arquivo `.uf2` gerado para o Raspberry Pi Pico:
   - Mantenha o botão `BOOTSEL` pressionado enquanto conecta o Pico ao computador.
   - Através da extensão do Raspberry Pi Pico Project no VS Code, é possível enviar o código para a placa clicando em "Run Project (USB)".
   - Caso não consiga enviar diretamente, copie o arquivo `.uf2` gerado para a unidade montada.

```

## 🎥 Demonstração

```plaintext
https://youtu.be/1oxMhdqzrCk?si=ITu73VqAeLJvTzUw
```

---

