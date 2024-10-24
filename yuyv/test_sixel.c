#include <stdio.h>

// Function to print Sixel data
void print_sixel(int width, int height, unsigned char pixels[height][width]) {
    // Start Sixel graphics
    printf("\033Pq"); // Start Sixel mode

    // Loop over each pixel in the 2D array
    for (int row = 0; row < height; row++) {
        // Start a new Sixel row
        printf("M");

        for (int col = 0; col < width; col++) {
            // Get the pixel value (0-255)
            unsigned char value = pixels[row][col];

            // Sixel encoding: scale pixel value (0-255) to Sixel value (0-63)
            int sixel_value = value * 63 / 255;

            // Print the Sixel character for this pixel
            printf("%c", sixel_value + 0x20); // Sixel characters start at ASCII 0x20
        }

        // End the current row of pixels
        printf("!"); // Signal end of a row
    }

    // End Sixel graphics
    printf("\033\\"); // End Sixel mode
}

int main() {
    // Example 5x5 pixel grayscale image
    unsigned char pixels[5][5] = {
        {255, 200, 150, 100, 50},
        {200, 150, 100, 50, 0},
        {150, 100, 50, 0, 0},
        {100, 50, 0, 0, 0},
        {50, 0, 0, 0, 0}
    };

    // Call the function to print the Sixel image
    print_sixel(5, 5, pixels);

    // Flush the output to ensure everything is displayed
    fflush(stdout);

    return 0;
}
