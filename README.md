
# Homework #3 (Tiny PL/0 Compiler)

This assignment implements a Recursive Descent Parser and Intermediate
Code Generator for tiny PL/0. It is capable of getting tokens produced by the scanner we developed in HW2 and produces an output file that does one of two things:

- States the given error if one is encountered (stopping the compilation process).

- If no error is encountered, the Symbol Table is fully populated and the output file will contain assembly code for the virtual machine we developed in HW1.

## Uploading to Eustis

***(Note: Be sure to be on the school network or on the university VPN to establish a connection to Eustis!)***

**1.** Open MobaXTerm

**2.** Establish a new SSH session with the following settings:

	Remote Host: eustis3.eecs.ucf.edu
	Specify username: (enter your NID in this field)
	Port: 22

**3.** Download and extract the submission into a "ss_hw3" folder, replicating the current submission 	folder structure.

**4.** Use the "cd" command in terminal to go to directory containing the folder you just created.

Ex: If you extracted the submission to your downloads folder:

    "C:\Users\{CurrentUser}\Downloads\ss_hw3" 

Then in the terminal type: 

    cd Downloads

**5.** Once in that directory, type the following command:

    scp -r ss_hw3 YOUR_NID@eustis3.eecs.ucf.edu:~/

***(Note: Be sure to replace "YOUR_NID" with your actual UCF NID.)***
## How To Compile

***Connect to the university network/VPN.***

1. In MobaXTerm, type the following command:

       gcc ./ss_hw3/parsercodegen.c

2. Now the program has been compiled. To run the program with a given input 	file and output file use the following command:
    
       ./a.out "./ss_hw3/input.txt" "./ss_hw3/output.txt"

(Note: You can replace "input.txt"/"output.txt" with the filename of whatever input/output file you would like to use to run the program. 
It MUST be located in the "ss_hw3" folder if you use this command though.)

3. After running these commands, text should be printed onto the screen showing the assembly code and the populated symbol table or whatever error is generated.

4. A text file containing the actual assembly code will be created in the "ss_hw3" directory and will be called "output.txt".