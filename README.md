# My Lox Programs
A complete collection of the programs I created in Lox for the course CS4080 - Concepts of Programming Languages. Lox is a language used in the textbook "Crafting Interpreters." The Lox repository can be found [here](https://github.com/munificent/craftinginterpreters). 

## How to run
These instructions are intended for my reference only, and should not be considered a conclusive guide for the general public. For more information about setting up Lox on your machine, please reference the [Lox language repository](https://github.com/munificent/craftinginterpreters). 
### Prerequisites
1. Install Ubuntu
2. Install Chocolatey (in order to install Dart)
    - Open up Windows Powershell
    - Type the command `Start-Process powershell -Verb RunAs` to run as administrator
    - Use the command on the [Chocolatey](https://chocolatey.org/install) website
3. Install [Dart](https://dart.dev/get-dart)
4. Clone the [Lox language repository](https://github.com/munificent/craftinginterpreters)
5. In your Ubuntu terminal, switch to the proper directory for your Lox language repository
    - For me, the command is: `cd /mnt/c/Users/prize/.vscode/Crafting_Interpreters`
6. Run the commands `make get` and `make` in your Ubuntu terminal

### To run the java implementation of Lox (JLox)
Run the following commands in your Ubuntu terminal
1. `javac -d . -sourcepath java java/com/craftinginterpreters/lox/*.java`
2. `java com.craftinginterpreters.lox.Lox ./myprograms/{MYFILE}.lox`

### To run the c implementation of Lox (CLox)
Run the following commands in your Ubuntu terminal
1. `make clox`
2. `./clox myprograms/{MYFILE}.lox`

