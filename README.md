<p align="center">
  <img src="https://raw.githubusercontent.com/gaming-gaming/Central-Processing-Unit-Crock-Pot-Utility/124753ab3375716c8dd6d9670b0769d608b46e36/brand/logo_text.svg" alt="Logo" width="75%"/>
</p>

# Central Processing Unit Crock-Pot Utility
**Central Processing Unit Crock-Pot Utility (CPU^2)** is a script for controlling the usage percentage of a processor, and therefore the temperature of the IHS as well.

Here is a list of things you can cook on a 100°C CPU (assuming the IHS is ~90°C):

- Scrambled eggs
- Water for hot chocolate and tea
- Any Crock-Pot meal not on high setting

> [!CAUTION]
> For your own health and safety, please do not consume anything directly off your processor. Using a food-safe heat conducting plane is strongly recommended.

# Using CPU^2
To control the load of your processor, use the left and right arrow keys. You may exit the program by pressing `Q`.

> [!NOTE]
> This script is currently in beta. Temperature control and controller support are not yet implemented.

# Compiling

Windows:
```cmd
g++ main.cpp -o "cpu^2.exe" -std=c++11 -pthread -O0 -m32
```

Linux:
```bash
g++ main.cpp -o "cpu^2" -std=c++11 -pthread -O0 -m32
```

> [!IMPORTANT]
> Compiler optimization must remain off. Math is done to heat your processor, but the compiler may see it as arbitrary.
