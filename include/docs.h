#ifndef XYL_DOCS_H
#define XYL_DOCS_H

#include "compiler.h"
#include <stdbool.h>

// Forward declarations
typedef struct doc_comment doc_comment_t;
typedef struct doc_param doc_param_t;
typedef struct doc_function doc_function_t;
typedef struct doc_variable doc_variable_t;
typedef struct doc_class doc_class_t;
typedef struct doc_module doc_module_t;

// Documentation comment structure
struct doc_comment {
  char *content;       // The comment text
  size_t length;       // Length of content
  int line;            // Line number where comment appears
  doc_comment_t *next; // For multiple comment lines
};

// Parameter documentation
struct doc_param {
  char *name;         // Parameter name
  type_hint_t type;   // Parameter type hint
  doc_comment_t *doc; // Parameter documentation
  doc_param_t *next;  // Next parameter
};

// Function documentation
struct doc_function {
  char *name;                // Function name
  doc_comment_t *doc;        // Function documentation
  doc_param_t *params;       // Parameter list
  type_hint_t return_type;   // Return type hint
  doc_comment_t *return_doc; // Return value documentation
  char *class_name;          // If method, which class it belongs to
  int line;                  // Line number of function declaration
  bool is_method;            // True if this is a class method
  doc_function_t *next;      // Next function in module
};

// Variable documentation
struct doc_variable {
  char *name;           // Variable name
  type_hint_t type;     // Variable type hint
  doc_comment_t *doc;   // Variable documentation
  int line;             // Line number of variable declaration
  doc_variable_t *next; // Next variable in module
};

// Class documentation
struct doc_class {
  char *name;              // Class name
  doc_comment_t *doc;      // Class documentation
  doc_function_t *methods; // Class methods
  doc_variable_t *fields;  // Class fields
  int line;                // Line number of class declaration
  doc_class_t *next;       // Next class in module
};

// Module documentation
struct doc_module {
  char *name;                // Module name (usually filename)
  char *path;                // Full file path
  doc_comment_t *doc;        // Module-level documentation
  doc_function_t *functions; // Functions in module
  doc_variable_t *variables; // Variables in module
  doc_class_t *classes;      // Classes in module
};

// Documentation generation output formats
typedef enum {
  DOC_FORMAT_MARKDOWN,
  DOC_FORMAT_HTML,
  DOC_FORMAT_JSON
} doc_format_t;

// Documentation generation options
typedef struct {
  doc_format_t format;  // Output format
  bool include_private; // Include private members
  bool generate_index;  // Generate index/navigation
  char *output_dir;     // Output directory
  char *title;          // Documentation title
  char *theme;          // Theme for HTML output
} doc_options_t;

// Function declarations
doc_comment_t *doc_comment_new(const char *content, size_t length, int line);
void doc_comment_free(doc_comment_t *comment);
doc_comment_t *doc_comment_append(doc_comment_t *head,
                                  doc_comment_t *new_comment);

doc_param_t *doc_param_new(const char *name, type_hint_t type,
                           doc_comment_t *doc);
void doc_param_free(doc_param_t *param);

doc_function_t *doc_function_new(const char *name, int line);
void doc_function_free(doc_function_t *func);

doc_variable_t *doc_variable_new(const char *name, type_hint_t type, int line);
void doc_variable_free(doc_variable_t *var);

doc_class_t *doc_class_new(const char *name, int line);
void doc_class_free(doc_class_t *cls);

doc_module_t *doc_module_new(const char *name, const char *path);
void doc_module_free(doc_module_t *module);

// Documentation extraction
doc_module_t *extract_docs_from_source(const char *source, const char *path);

// Documentation generation
bool generate_docs(doc_module_t *module, const doc_options_t *options);
bool generate_docs_markdown(doc_module_t *module, const char *output_path);
bool generate_docs_html(doc_module_t *module, const doc_options_t *options);

// Utility functions
char *doc_format_type_hint(const type_hint_t *hint);
char *doc_escape_html(const char *text);
char *doc_escape_markdown(const char *text);

#endif
