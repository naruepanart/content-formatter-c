#include <stdio.h>
#include <windows.h>

int main() {
  static const char* exts[] = {".jpg", ".jpeg", ".png", ".gif", ".bmp",  ".webp", ".tiff", ".svg", ".mp4", ".mov",
                               ".avi", ".mkv",  ".flv", ".wmv", ".webm", ".mpeg", ".mpg",  ".3gp", NULL};

  WIN32_FIND_DATAA f;
  HANDLE h = FindFirstFileA("*", &f);
  if (h == INVALID_HANDLE_VALUE)
    return 1;

  LARGE_INTEGER counter;
  QueryPerformanceCounter(&counter);
  unsigned int seed = counter.LowPart;

  char new_name[37];  // 8 hex + max 5 char ext + null = 14, but some exts are longer
  do {
    if (f.dwFileAttributes & (FILE_ATTRIBUTE_DIRECTORY | FILE_ATTRIBUTE_SYSTEM))
      continue;

    const char* name = f.cFileName;
    const char* ext = NULL;

    // Find extension
    for (const char* p = name; *p; ++p) {
      if (*p == '.')
        ext = p;
    }
    if (!ext)
      continue;

    // Check extension
    int found = 0;
    for (const char** e = exts; *e; ++e) {
      const char *a = ext, *b = *e;
      while (*a && *b && (*a == *b || (*a | 32) == (*b | 32)))
        ++a, ++b;
      if (!*a && !*b) {
        found = 1;
        break;
      }
    }
    if (!found)
      continue;

    // Xorshift PRNG - zero overhead
    seed ^= seed << 13;
    seed ^= seed >> 17;
    seed ^= seed << 5;

    // Generate random name and move
    sprintf(new_name, "%08x%s", seed, ext);
    if (MoveFileA(name, new_name)) {
      puts(new_name);
    }
  } while (FindNextFileA(h, &f));

  FindClose(h);
  return 0;
}