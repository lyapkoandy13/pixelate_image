#include <errno.h>
#define STB_IMAGE_IMPLEMENTATION
#include "stb/stb_image.h"
#define STB_IMAGE_RESIZE_IMPLEMENTATION
#include "stb/stb_image_resize2.h"
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define ANSI_RESET "\x1b[0m"
#define ANSI_TEXT "\e[38;2;"
#define ANSI_BACKGROUND "\e[48;2;"

typedef enum { PM_DEFAULT = '1', PM_ROW = '2' } PrintType;

typedef enum { CT_TEXT = 0, CT_BACKGROUND = 1 } ColorType;

typedef struct {
  unsigned char r;
  unsigned char g;
  unsigned char b;
} Color;

void *mallocOrDie(size_t size) {
  void *allocated = malloc(size);
  if (!allocated && size) {
    printf("Could not allocate memory!");
    exit(EXIT_FAILURE);
  }
  return allocated;
}

void *reallocOrDie(void *ptr, size_t size) {
  void *allocated = realloc(ptr, size);
  if (!allocated && size) {
    printf("Could not reallocate memory!");
    exit(EXIT_FAILURE);
  }
  return allocated;
}

char *numberToString(unsigned char number) {
  int length = snprintf(NULL, 0, "%u", number);
  char *str = mallocOrDie(length + 1);
  snprintf(str, length + 1, "%u", number);
  str[length + 1] = 0;

  return str;
}

char *getTerminalColor(ColorType type, Color color) {
  char *r = numberToString(color.r);
  char *g = numberToString(color.g);
  char *b = numberToString(color.b);

  char *prefix = type == CT_TEXT ? ANSI_TEXT : ANSI_BACKGROUND;

  // int size = sizeof(r), len = strlen(r);

  int result_size = strlen(prefix) + strlen(r) + strlen(g) + strlen(b) +
                    4; // 4 = 2 x ';' + 'm' + '\0'
  char *result = mallocOrDie(result_size);
  // "\e[38;2;200;0;0m"
  // where rrr;ggg;bbb in 38;2;rrr;ggg;bbbm can go from 0 to 255 respectively
  strcpy(result, prefix); // prefix
  strcat(result, r);      // r
  strcat(result, ";");    // ;
  strcat(result, g);      // g
  strcat(result, ";");    // ;
  strcat(result, b);      // b
  strcat(result, "m");    // m

  free(r);
  free(g);
  free(b);

  return result;
}

char *getColoredString(const char *str, const ColorType type,
                       const Color color) {
  char *ansi_prefix = getTerminalColor(type, color);

  char *result = reallocOrDie(ansi_prefix, strlen(ansi_prefix) + strlen(str) +
                                               strlen(ANSI_RESET) + 1);

  strcat(result, str);
  strcat(result, ANSI_RESET);

  return result;
}

char *getColoredBackground(const int len, const Color color) {
  char *spaces = mallocOrDie(sizeof(char) * len + 1);

  for (int i = 0; i < len; i++) {
    spaces[i] = ' ';
    // printf("Iter %d: %d %c \n", i, spaces[i], spaces[i]);
  }
  spaces[len] = 0;

  char *result = getColoredString(spaces, CT_BACKGROUND, color);
  free(spaces);
  return result;
}

void printImageToTerminalPerRow(const int cols, const int rows,
                                const int channels, const unsigned char *data) {
  for (int column = 0; column < cols; column++) {
    int sizeStep = 1024;
    int rowSize = sizeStep;
    int rowLen = 0;
    char *rowStr = mallocOrDie(rowSize);
    for (int row = rows - 1; row >= 0; row--) {
      const int offset = column * cols * channels + row * channels;
      const Color color = {data[offset], data[offset + 1], data[offset + 2]};
      char *str = getColoredBackground(1, color);
      rowLen = strlen(rowStr);
      if (rowLen + strlen(str) > (unsigned long)rowSize) {
        rowStr = reallocOrDie(rowStr, rowSize += sizeStep);
      }
      strcat(rowStr, str);
      free(str);
    }
    puts(rowStr);
    free(rowStr);
  }
}

void printImageToTerminal(const int cols, const int rows, const int channels,
                          const unsigned char *data) {
  for (int column = 0; column < cols; column++) {
    for (int row = rows - 1; row >= 0; row--) {
      const int offset = column * cols * channels + row * channels;
      const Color color = {data[offset], data[offset + 1], data[offset + 2]};
      char *str = getColoredBackground(1, color);
      printf("%s", str);
      free(str);
    }
    printf("\n");
  }
}

int stringToNumber(char *strNum) {
  char *p;
  errno = 0;
  long conv = strtol(strNum, &p, 10);

  // Check for errors: e.g., the string does not represent an integer
  // or the integer is larger than int
  if (errno != 0 || *p != '\0' || conv > INT_MAX || conv < INT_MIN) {
    perror("Failed to convert string to number");
    exit(EXIT_FAILURE);
  }

  return conv;
}

int main(int argc, char **argv) {
  if (argc < 2) {
    perror("No image provided\n");
    exit(EXIT_FAILURE);
  }

  const char *imgPath = argv[1];

  PrintType printMode = PM_DEFAULT;
  if (argc > 2 && argc < 4) {
    perror("If you pass > 2 args, you need to provide with and height");
  }

  int r_cols = 0, r_rows = 0, resize = 0;
  if (argc > 2) {
    //   printMode = *argv[2];
    resize = 1;
    r_cols = stringToNumber(argv[2]);
    r_rows = stringToNumber(argv[3]);
    printf("Got args: %d %d\n", r_cols, r_rows);
  }

  int cols, rows, channels;
  unsigned char *data = stbi_load(imgPath, &cols, &rows, &channels, 0);
  if (data == NULL) {
    perror("Failed to load the image\n");
    exit(EXIT_FAILURE);
  }

  unsigned char *resized_data = NULL;
  if (resize == 0 || r_cols == 0 || r_rows == 0) {
    resized_data = data;
    r_cols = cols;
    r_rows = rows;
  } else {
    resized_data = stbir_resize_uint8_srgb(data, cols, rows, 0, NULL, r_cols,
                                           r_rows, 0, STBIR_RGBA);
  }

  if (printMode == PM_DEFAULT) {
    printImageToTerminal(r_cols, r_rows, channels, resized_data);
  } else {
    printImageToTerminalPerRow(cols, rows, channels, data);
  }

  printf("width: %d; height: %d; channels: %d; print mode: %s\n", cols, rows,
         channels, printMode == PM_DEFAULT ? "Default" : "Per Row");

  if (resize == 1) {
    printf("Resized to: %dx%d\n", r_cols, r_rows);
  }

  stbi_image_free(data);
  free(resized_data);

  return EXIT_SUCCESS;
}
