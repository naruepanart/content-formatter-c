#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/**
 * Removes asterisks (*) and double quotes (") from the content string
 * Modifies the string in-place using a read/write pointer approach
 */
void remove_special_chars(char* content) {
  char* write_ptr = content;  // Pointer for writing processed characters
  char* read_ptr = content;   // Pointer for reading original characters

  // Process each character until null terminator
  while (*read_ptr) {
    // Keep character if it's not '*' or '"'
    if (*read_ptr != '*' && *read_ptr != '"') {
      *write_ptr++ = *read_ptr;  // Copy character and advance write pointer
    }
    read_ptr++;  // Always advance read pointer
  }
  *write_ptr = '\0';  // Add null terminator to end of processed string
}

/**
 * Parses content into individual lines, trimming whitespace from each line
 * Returns array of dynamically allocated strings containing the processed lines
 */
char** parse_lines(char* content, int* line_count, int max_lines) {
  // Allocate memory for array of line pointers
  char** lines = malloc(max_lines * sizeof(char*));
  if (!lines)
    return NULL;  // Memory allocation failed

  *line_count = 0;                         // Initialize output parameter
  const char* current_position = content;  // Start at beginning of content

  // Process until end of content or maximum lines reached
  while (*current_position && *line_count < max_lines) {
    // Find next newline or end of string
    const char* line_end = strchr(current_position, '\n');
    if (!line_end)
      line_end = current_position + strlen(current_position);

    // Calculate line length (excluding newline)
    size_t line_length = line_end - current_position;

    if (line_length > 0) {
      // Allocate memory for line copy (+1 for null terminator)
      char* line = malloc(line_length + 1);
      if (!line)
        continue;  // Skip if allocation fails

      // Copy line content and add null terminator
      memcpy(line, current_position, line_length);
      line[line_length] = '\0';

      // Trim leading whitespace
      char* line_start = line;
      while (*line_start && isspace((unsigned char)*line_start)) {
        line_start++;
      }

      // Only process non-empty lines after trimming
      if (*line_start) {
        // Trim trailing whitespace
        char* line_end_ptr = line + strlen(line) - 1;
        while (line_end_ptr > line_start && isspace((unsigned char)*line_end_ptr)) {
          line_end_ptr--;
        }
        *(line_end_ptr + 1) = '\0';  // Terminate after last non-whitespace

        // Move trimmed content to beginning if needed
        if (line_start != line) {
          memmove(line, line_start, strlen(line_start) + 1);
        }

        // Store processed line in array
        lines[(*line_count)++] = line;
      } else {
        free(line);  // Free memory for empty lines
      }
    }

    // Move to next line (skip newline character if present)
    current_position = (*line_end == '\n') ? line_end + 1 : line_end;
  }

  return lines;
}

/**
 * Processes a single line by removing existing numbering, colons, and formatting with new numbering
 * Returns dynamically allocated formatted string
 */
char* process_line(char* line, int line_number, size_t max_line_length) {
  char* current_pos = line;

  // Skip any leading whitespace
  while (*current_pos && isspace((unsigned char)*current_pos)) {
    current_pos++;
  }

  // Skip existing numbering (digits followed by dot)
  while (*current_pos && isdigit((unsigned char)*current_pos)) {
    current_pos++;
  }
  if (*current_pos == '.')
    current_pos++;  // Skip the dot
  while (*current_pos && isspace((unsigned char)*current_pos)) {
    current_pos++;  // Skip whitespace after numbering
  }

  // Remove colon and any text before it (title portion)
  char* colon_position = strchr(current_pos, ':');
  if (colon_position) {
    // Move text after colon to current position
    memmove(current_pos, colon_position + 1, strlen(colon_position + 1) + 1);
  }

  // Skip any new leading whitespace after processing
  while (*current_pos && isspace((unsigned char)*current_pos)) {
    current_pos++;
  }

  // Calculate required buffer size for formatted line
  size_t number_prefix_length = snprintf(NULL, 0, "%d. ", line_number);
  size_t text_length = strlen(current_pos);
  size_t total_length = number_prefix_length + text_length + 3;  // +3 for "\n\n\0"

  // Allocate memory for formatted result
  char* formatted_line = malloc(total_length);
  if (!formatted_line)
    return NULL;

  // Create formatted line with numbering and double newline
  snprintf(formatted_line, total_length, "%d. %s\n\n", line_number, current_pos);
  return formatted_line;
}

/**
 * Main function: reads file, processes content, and writes formatted result
 */
int main() {
  const char* filename = "content-formatter.txt";
  char* file_content = NULL;
  char* formatted_result = NULL;
  size_t content_buffer_size = 0;
  size_t result_buffer_size = 0;

  // Open input file for reading
  FILE* input_file = fopen(filename, "r");
  if (!input_file) {
    fprintf(stderr, "Error: Cannot open file %s\n", filename);
    return 1;
  }

  // Determine file size
  fseek(input_file, 0, SEEK_END);
  long file_size = ftell(input_file);
  fseek(input_file, 0, SEEK_SET);

  if (file_size <= 0) {
    fprintf(stderr, "Error: Empty file or file read error\n");
    fclose(input_file);
    return 1;
  }

  // Allocate buffer for file content (+1 for null terminator)
  content_buffer_size = file_size + 1;
  file_content = malloc(content_buffer_size);
  if (!file_content) {
    fprintf(stderr, "Error: Memory allocation failed\n");
    fclose(input_file);
    return 1;
  }

  // Read entire file into buffer and null-terminate
  size_t bytes_read = fread(file_content, 1, file_size, input_file);
  file_content[bytes_read] = '\0';
  fclose(input_file);

  // Process content: remove special characters
  remove_special_chars(file_content);

  // Estimate maximum number of lines by counting newlines
  int max_lines_estimate = 0;
  char* temp_ptr = file_content;
  while (*temp_ptr) {
    if (*temp_ptr == '\n')
      max_lines_estimate++;
    temp_ptr++;
  }
  max_lines_estimate += 2;  // Add buffer for safety

  // Parse content into individual lines
  int actual_line_count = 0;
  char** lines_array = parse_lines(file_content, &actual_line_count, max_lines_estimate);
  if (!lines_array || actual_line_count == 0) {
    fprintf(stderr, "No lines to process\n");
    free(file_content);
    return 1;
  }

  // Allocate buffer for formatted result (conservative estimate)
  result_buffer_size = content_buffer_size * 2;
  formatted_result = malloc(result_buffer_size);
  if (!formatted_result) {
    fprintf(stderr, "Error: Memory allocation failed\n");
    free(file_content);
    for (int i = 0; i < actual_line_count; i++)
      free(lines_array[i]);
    free(lines_array);
    return 1;
  }
  formatted_result[0] = '\0';  // Initialize as empty string

  // Process each line and build formatted result
  size_t current_output_position = 0;
  for (int i = 0, current_number = 1; i < actual_line_count; i++) {
    char* formatted_line = process_line(lines_array[i], current_number++, result_buffer_size - current_output_position);

    if (formatted_line) {
      size_t line_length = strlen(formatted_line);

      // Check if buffer has enough space
      if (current_output_position + line_length < result_buffer_size) {
        memcpy(formatted_result + current_output_position, formatted_line, line_length);
        current_output_position += line_length;
        formatted_result[current_output_position] = '\0';
      } else {
        // Expand buffer if needed
        result_buffer_size = current_output_position + line_length + 1024;
        char* new_buffer = realloc(formatted_result, result_buffer_size);
        if (new_buffer) {
          formatted_result = new_buffer;
          memcpy(formatted_result + current_output_position, formatted_line, line_length);
          current_output_position += line_length;
          formatted_result[current_output_position] = '\0';
        }
      }
      free(formatted_line);
    }
  }

  // Remove trailing newlines
  while (current_output_position > 0 && formatted_result[current_output_position - 1] == '\n') {
    current_output_position--;
  }
  formatted_result[current_output_position] = '\0';

  // Clean up original content and line array
  free(file_content);
  for (int i = 0; i < actual_line_count; i++) {
    free(lines_array[i]);
  }
  free(lines_array);

  // Write formatted result back to file
  FILE* output_file = fopen(filename, "w");
  if (!output_file) {
    fprintf(stderr, "Error: Cannot write to file\n");
    free(formatted_result);
    return 1;
  }

  fputs(formatted_result, output_file);
  fclose(output_file);
  free(formatted_result);

  printf("File successfully formatted: %s\n", filename);
  return 0;
}