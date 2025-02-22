# Exploit and Patch Project
  
In this project, you will by identifying and exploiting vulnerabilities in seven programs. These vulnerabilities will cover the types of vulnerabilities discussed in class:

* TOCTOU race conditions
* Integer-based vulnerabilities
* Buffer overflows

These programs are named `problem{1-7}` and can be found in the same directory as this readme. For this project, I have turned off various protections to allow exploits to work. For example, the binaries will not use a stack canary or ASLR.

For each program, I have also provided a `problem#.c` file. This is the file is very similar to the file from which the binary was compiled. However, this file has had any secret information removed.

If you wish to compile the code files, use `make binaries`. This will build binaries in the `./out/` directory to avoid erasing the binaries in the workspace directory. Do not compile any other way as this may result in deleting the binaries in the workspace directory, which are necessary to complete the project.

## Running

* To build the binaries, run `make binaries`
  * Binaries are placed in `./out`
* To generate a diff-based patch, run `make diff`
  * Patches are placed in `./diff`

## Hints

* Problem 2: Neither `void backdoor()` or `void flag()` are ever called. What vulnerability allows you to execute code at arbitrary places within the binary? 
   * If you need to find the memory address of a function, consider using `objdump`.
* Problem 3: You are not going to be able to steal the value of secret. How else might you cause the password check to pass?
* Problem 5: For this problem, you will need to use a script to execute and exploit the binary.
   * Make sure to include your script in the writeup.
   * When patching this binary, the functionality you are trying to achieve is to ensure that the file doesn't exist before you open it. If you do so, we will count this as preserving necessary functionality.
* Problem 6: If you create a script, make sure to include it in your writeup.
* Problem 7: Do not fix by removing the threading or modifying how subtraction is done one value at a time. Instead, what is the proper way to protect state when shared between threads?

## Writeup

As you solve each problem, you will write up details about your solutions in `submission.md`. This is the file that you will submit on Canvas.

Below is an example of how such a writeup would look. This writeup is for the `example` binary found in this handout. This program will print out the flag if given two numbers that when multiplied together produce 42. This can be determined by using the `strings` program on the binary to identify a string that lists the secret value. Note, the below example has a **much** larger patch than will be needed in your programs.

### Flag
flag{CS366-EXAMPLEFLAG}

### Exploit Steps
1. Run `strings` on the `./example`.
2. Look for the secret: `strings example | grep secret`.
3. Notice the string that says the answers is 42.
4. Run `echo "6 7" | ./example`

### Patch
```diff
--- out/handout/example.c       2023-02-28 05:21:36.602922800 +0000
+++ src/example_patched.c       2023-02-28 05:20:43.636893600 +0000
@@ -26,7 +26,26 @@
   }
 
   int secret;
-  // Set the secret value. Hidden from prying eyes...
+  FILE *fp;
+  int secret;
+
+  // Make sure we can open the file
+  fp = fopen("secret.txt", "r");
+  if (fp == NULL) {
+    printf("Error opening file\n");
+    return -1;
+  }
+
+  // Read in the secret to a variable called secret
+  if (fscanf(fp, "%d", &secret) != 1) {
+    printf("Error reading integer from file\n");
+    return -1;
+  }
+
+  // Close the file
+  fclose(fp);
 
   // See if the user knows the secret
   if (num1 * num2 == secret) {
```

### Explanation
The code is vulnerable because a secret value is stored in plaintext in the file. This allows an attacker to find it using `strings`. My solution is to store the secret in a file `secrets.txt` that will have appropriate permissions to protect it. The program will then read in the value in this file, removing the ability for an attacker to find it by scanning the binary.
